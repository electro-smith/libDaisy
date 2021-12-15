#include "system.h"
#include "usbd_cdc.h"
#include "hid/usb_midi.h"
#include <cassert>

using namespace daisy;

class MidiUsbTransport::Impl
{
  public:
    void Init(Config config);

    void    StartRx() { rx_active_ = true; }
    size_t  Readable() { return rx_buffer_.readable(); }
    uint8_t Rx() { return rx_buffer_.Read(); }
    bool    RxActive() { return rx_active_; }
    void    FlushRx() { rx_buffer_.Flush(); }
    void    Tx(uint8_t* buffer, size_t size);

    void UsbToMidi(uint8_t* buffer, uint8_t length);
    void MidiToUsb(uint8_t* buffer, size_t length);

  private:
    /** USB Handle for CDC transfers 
         */
    UsbHandle usb_handle_;
    Config    config_;

    static constexpr size_t kBufferSize = 1024;
    bool                    rx_active_;
    // This corresponds to 256 midi messages
    RingBuffer<uint8_t, kBufferSize> rx_buffer_;

    // simple, self-managed buffer
    uint8_t tx_buffer_[kBufferSize];
    size_t  tx_ptr_;

    // MIDI message size determined by the
    // code index number. You can find this
    // table in the MIDI USB spec 1.0
    const uint8_t code_index_size_[16]
        = {3, 3, 2, 3, 3, 1, 2, 3, 3, 3, 3, 3, 2, 2, 3, 1};

    const uint8_t midi_message_size_[16]
        = {0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 1, 1, 2, 0};

    // Masks to check for message type, and byte content
    const uint8_t kStatusByteMask     = 0x80;
    const uint8_t kMessageMask        = 0x70;
    const uint8_t kDataByteMask       = 0x7F;
    const uint8_t kSystemCommonMask   = 0xF0;
    const uint8_t kChannelMask        = 0x0F;
    const uint8_t kRealTimeMask       = 0xF8;
    const uint8_t kSystemRealTimeMask = 0x07;
};

// Global Impl
static MidiUsbTransport::Impl midi_usb_handle;

void ReceiveCallback(uint8_t* buffer, uint32_t* length)
{
    if(midi_usb_handle.RxActive())
    {
        for(uint16_t i = 0; i < *length; i += 4)
        {
            uint8_t packet_length = *length - i >= 4 ? 4 : *length - i;
            midi_usb_handle.UsbToMidi(buffer, packet_length);
        }
    }
}

void MidiUsbTransport::Impl::Init(Config config)
{
    // Borrowed from logger
    /** this implementation relies on the fact that UsbHandle class has no member variables and can be shared
     * assert this statement:
     */
    // static_assert(1u == sizeof(MidiUsbTransport::Impl::usb_handle_), "UsbHandle is not static");

    // This tells the USB middleware to send out MIDI descriptors instead of CDC
    usbd_mode = USBD_MODE_MIDI;
    config_   = config;

    UsbHandle::UsbPeriph periph = UsbHandle::FS_INTERNAL;
    if(config_.periph == Config::EXTERNAL)
        periph = UsbHandle::FS_EXTERNAL;

    usb_handle_.Init(periph);

    rx_active_ = false;
    System::Delay(10);
    usb_handle_.SetReceiveCallback(ReceiveCallback, periph);
}

void MidiUsbTransport::Impl::Tx(uint8_t* buffer, size_t size)
{
    MidiToUsb(buffer, size);
    if(config_.periph == Config::EXTERNAL)
        usb_handle_.TransmitExternal(tx_buffer_, tx_ptr_);
    else
        usb_handle_.TransmitInternal(tx_buffer_, tx_ptr_);
    tx_ptr_ = 0;
}

void MidiUsbTransport::Impl::UsbToMidi(uint8_t* buffer, uint8_t length)
{
    // A length of less than four in the buffer indicates
    // a garbled message, since USB MIDI packets usually*
    // require 4 bytes per message
    if(length < 4)
        return;

    // Right now, Daisy only supports a single cable, so we don't
    // need to extract that value from the upper nibble
    uint8_t code_index = buffer[0] & 0xF;
    if(code_index == 0x0 || code_index == 0x1 || code_index == 0xF)
    {
        // 0x0 and 0x1 are reserved codes, and if they come up,
        // there's probably been an error. *0xF indicates data is
        // sent one byte at a time, rather than in packets of four.
        // This functionality could be supported later.
        return;
    }

    // Only writing as many bytes as necessary
    for(uint8_t i = 0; i < code_index_size_[code_index]; i++)
    {
        if(rx_buffer_.writable() > 0)
            rx_buffer_.Write(buffer[1 + i]);
        else
        {
            rx_active_ = false; // disable on overflow
            break;
        }
    }
}

void MidiUsbTransport::Impl::MidiToUsb(uint8_t* buffer, size_t size)
{
    size_t i             = 0;
    int    currentStatus = buffer[i] & 0xF0;
    while(i < size)
    {
        if(currentStatus > 0x7 && currentStatus < 0xF)
        {
            // This is the code index number in the given ^ state
            tx_buffer_[tx_ptr_++] = currentStatus >> 4;
            for(int j = 0; j < midi_message_size_[currentStatus]; j++)
            {
                // If the user tries to send an invalid message or
                // the buffer overflows, then we exit. This does
                // not account for user-generated running messages!
                if(i < size && tx_ptr_ < kBufferSize)
                    tx_buffer_[tx_ptr_++] = buffer[i++];
                else
                    break;
            }

            // Filling extra bytes with zero
            for(int j = 0; j < 3 - midi_message_size_[currentStatus]; j++)
            {
                if(tx_ptr_ < kBufferSize)
                    tx_buffer_[tx_ptr_++] = 0;
                else
                    break;
            }
        }
        else
        {
            // Scan for valid MIDI message statuses
            currentStatus = buffer[i++] & 0xF0;
        }
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

void MidiUsbTransport::StartRx()
{
    pimpl_->StartRx();
}

size_t MidiUsbTransport::Readable()
{
    return pimpl_->Readable();
}

uint8_t MidiUsbTransport::Rx()
{
    return pimpl_->Rx();
}

bool MidiUsbTransport::RxActive()
{
    return pimpl_->RxActive();
}

void MidiUsbTransport::FlushRx()
{
    pimpl_->FlushRx();
}

void MidiUsbTransport::Tx(uint8_t* buffer, size_t size)
{
    pimpl_->Tx(buffer, size);
}