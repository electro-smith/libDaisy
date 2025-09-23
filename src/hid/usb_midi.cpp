#include "system.h"
#include "usbd_cdc.h"
#include "usbh_midi.h"
#include "hid/usb_midi.h"
#include <cassert>
#include "tusb.h"

extern "C"
{
    extern USBH_HandleTypeDef hUsbHostHS;
}

#define pUSB_Host &hUsbHostHS

using namespace daisy;

class MidiUsbTransport::Impl
{
  public:
    void Init(Config config);

    void StartRx(MidiRxParseCallback callback, void* context)
    {
        FlushRx();
        rx_active_      = true;
        parse_callback_ = callback;
        parse_context_  = context;
    }

    bool RxActive() { return rx_active_; }
    void FlushRx()
    { /* No need for explicit flushing with TinyUSB */
    }

    void Tx(uint8_t* buffer, size_t size);
    void Tx(uint8_t cable_num, uint8_t* buffer, size_t size);

    void ProcessRx(); // Process any pending received MIDI packets

  private:
    Config config_;

    static constexpr size_t kBufferSize = 1024;
    bool                    rx_active_;
    MidiRxParseCallback     parse_callback_;
    void*                   parse_context_;

    // For handling Running Status in MIDI messages
    uint8_t last_status_byte_;
    bool    running_status_enabled_;
};

// Global Impl
static MidiUsbTransport::Impl midi_usb_handle;

void MidiUsbTransport::Impl::Init(Config config)
{
    config_                 = config;
    rx_active_              = false;
    last_status_byte_       = 0;
    running_status_enabled_ = true;

    // TinyUSB initialization should already be done
    // in the UsbHandle::Init() function
}

void MidiUsbTransport::Impl::Tx(uint8_t* buffer, size_t size)
{
    // Always use cable 0 when not specified
    Tx(0, buffer, size);
}

void MidiUsbTransport::Impl::Tx(uint8_t cable_num, uint8_t* buffer, size_t size)
{
    if(!tud_midi_mounted())
    {
        return;
    }

    // Ensure cable number is valid (0-15)
    cable_num = cable_num & 0x0F;

    // We'll handle raw MIDI messages here, and encode them for USB MIDI
    if(size == 0 || buffer == nullptr)
    {
        return;
    }

    // For basic Channel Voice messages (the most common case)
    if((buffer[0] & 0x80) && (buffer[0] & 0xF0) != 0xF0)
    {
        uint8_t status_byte = buffer[0];
        uint8_t channel     = buffer[0] & 0x0F;
        uint8_t msg_type    = buffer[0] & 0xF0;

        // Determine message size based on status byte
        size_t msg_size = 3; // Most common case for note on/off, CC, etc.

        if(msg_type == 0xC0 || msg_type == 0xD0)
        {
            msg_size = 2; // Program change and Channel pressure
        }

        // Ensure we have enough bytes for the full message
        if(size >= msg_size)
        {
            if(msg_size == 3)
            {
                tud_midi_n_stream_write(0, cable_num, buffer, 3);
            }
            else if(msg_size == 2)
            {
                tud_midi_n_stream_write(0, cable_num, buffer, 2);
            }
        }
    }
    // System Common and System Real-Time messages
    else if((buffer[0] & 0xF0) == 0xF0)
    {
        // SysEx message - special handling required
        if(buffer[0] == 0xF0)
        {
            // Find end of SysEx
            size_t sysex_size = 1;
            while(sysex_size < size && buffer[sysex_size] != 0xF7)
            {
                sysex_size++;
            }
            // Include the end marker
            if(sysex_size < size && buffer[sysex_size] == 0xF7)
            {
                sysex_size++;
            }

            // TinyUSB has a helper function to send SysEx
            tud_midi_n_stream_write(0, cable_num, buffer, sysex_size);
        }
        // Other system messages
        else if(buffer[0] == 0xF1 || buffer[0] == 0xF3)
        {
            // 2-byte messages (Time Code, Song Select)
            if(size >= 2)
            {
                tud_midi_n_stream_write(0, cable_num, buffer, 2);
            }
        }
        else if(buffer[0] == 0xF2)
        {
            // 3-byte message (Song Position)
            if(size >= 3)
            {
                tud_midi_n_stream_write(0, cable_num, buffer, 3);
            }
        }
        else
        {
            // Single-byte messages (realtime)
            tud_midi_n_stream_write(0, cable_num, buffer, 1);
        }
    }
}

void MidiUsbTransport::Impl::ProcessRx()
{
    if(!rx_active_ || !parse_callback_)
    {
        return;
    }

    // Check if there's data available
    if(!tud_midi_available())
    {
        return;
    }

    // Buffer to store multiple MIDI messages
    uint8_t rx_buffer[kBufferSize];
    size_t  total_bytes = 0;

    // Read and process all available packets
    while(tud_midi_available() && total_bytes < kBufferSize)
    {
        uint8_t packet[4];

        // Read the full USB MIDI packet (always 4 bytes)
        if(tud_midi_packet_read(packet))
        {
            uint8_t cable      = packet[0] >> 4;
            uint8_t code_index = packet[0] & 0x0F;

            // Skip invalid code indices (0x0, 0x1)
            if(code_index == 0x0 || code_index == 0x1)
            {
                continue;
            }

            // The number of valid bytes in the packet depends on the code index
            // Convert from USB-MIDI packet to regular MIDI message
            uint8_t bytes_to_copy;

            switch(code_index)
            {
                case 0x2: // 2-byte System Common messages like MTC, Song Select
                case 0xC: // Program Change
                case 0xD: // Channel Pressure
                    bytes_to_copy = 2;
                    break;

                case 0x3: // 3-byte System Common messages like Song Position
                case 0x4: // SysEx starts or continues
                case 0x5: // Single-byte System Common, or SysEx ends with one byte
                case 0x6: // SysEx ends with two bytes
                case 0x7: // SysEx ends with three bytes
                case 0x8: // Note Off
                case 0x9: // Note On
                case 0xA: // Poly-Key Press
                case 0xB: // Control Change
                case 0xE: // Pitch Bend Change
                    bytes_to_copy = 3;
                    break;

                case 0xF: // Single Byte message (like realtime)
                    bytes_to_copy = 1;
                    break;

                default: bytes_to_copy = 0; break;
            }

            // Copy valid bytes to our buffer
            for(uint8_t i = 0; i < bytes_to_copy && total_bytes < kBufferSize;
                i++)
            {
                rx_buffer[total_bytes++]
                    = packet[i + 1]; // Skip the cable nibble/code index byte
            }
        }
        else
        {
            break; // Error reading packet
        }
    }

    // If we collected any data, pass it to the callback
    if(total_bytes > 0 && parse_callback_)
    {
        parse_callback_(rx_buffer, total_bytes, parse_context_);
    }
}

////////////////////////////////////////////////
// MidiUsbTransport -> MidiUsbTransport::Impl
////////////////////////////////////////////////

void MidiUsbTransport::Init(MidiUsbTransport::Config config)
{
    pimpl_ = &midi_usb_handle;
    pimpl_->Init(config);
}

void MidiUsbTransport::StartRx(MidiRxParseCallback callback, void* context)
{
    pimpl_->StartRx(callback, context);
}

bool MidiUsbTransport::RxActive()
{
    return pimpl_->RxActive();
}

void MidiUsbTransport::FlushRx()
{
    // Noop with TinyUSB - flushing happens automatically
}

void MidiUsbTransport::Tx(uint8_t* buffer, size_t size)
{
    pimpl_->Tx(buffer, size);
}

void MidiUsbTransport::Tx(uint8_t cable_num, uint8_t* buffer, size_t size)
{
    pimpl_->Tx(cable_num, buffer, size);
}

void MidiUsbTransport::ProcessRx()
{
    pimpl_->ProcessRx();
}
