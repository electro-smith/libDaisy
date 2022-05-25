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
    void MidiToUsbSingle(uint8_t* buffer, size_t length);

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
            size_t  remaining_bytes = *length - i;
            uint8_t packet_length   = remaining_bytes > 4 ? 4 : remaining_bytes;
            midi_usb_handle.UsbToMidi(buffer + i, packet_length);
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

void MidiUsbTransport::Impl::MidiToUsbSingle(uint8_t* buffer, size_t size)
{
    if(size == 0)
        return;

    // Channel voice messages
    if((buffer[0] & 0xF0) != 0xF0)
    {
        // Check message validity
        if((buffer[0] & 0xF0) == 0xC0 || (buffer[0] & 0xF0) == 0xD0)
        {
            if(size != 2)
                return; // error
        }
        else
        {
            if(size != 3)
                return; //error
        }

        // CIN is the same as status byte for channel voice messages
        tx_buffer_[tx_ptr_ + 0] = (buffer[0] & 0xF0) >> 4;
        tx_buffer_[tx_ptr_ + 1] = buffer[0];
        tx_buffer_[tx_ptr_ + 2] = buffer[1];
        tx_buffer_[tx_ptr_ + 3] = size == 3 ? buffer[2] : 0;

        tx_ptr_ += 4;
    }
    else // buffer[0] & 0xF0 == 0xF0 aka System common or realtime
    {
        if(0xF2 == buffer[0])
        // three byte message
        {
            if(size != 3)
                return; // error

            tx_buffer_[tx_ptr_ + 0] = 0x03;
            tx_buffer_[tx_ptr_ + 1] = buffer[0];
            tx_buffer_[tx_ptr_ + 2] = buffer[1];
            tx_buffer_[tx_ptr_ + 3] = buffer[2];

            tx_ptr_ += 4;
        }
        if(0xF1 == buffer[0] || 0xF3 == buffer[0])
        // two byte messages
        {
            if(size != 2)
                return; // error

            tx_buffer_[tx_ptr_ + 0] = 0x02;
            tx_buffer_[tx_ptr_ + 1] = buffer[0];
            tx_buffer_[tx_ptr_ + 2] = buffer[1];
            tx_buffer_[tx_ptr_ + 3] = 0;

            tx_ptr_ += 4;
        }
        else if(0xF4 <= buffer[0])
        // one byte message
        {
            if(size != 1)
                return; // error

            tx_buffer_[tx_ptr_ + 0] = 0x05;
            tx_buffer_[tx_ptr_ + 1] = buffer[0];
            tx_buffer_[tx_ptr_ + 2] = 0;
            tx_buffer_[tx_ptr_ + 3] = 0;

            tx_ptr_ += 4;
        }
        else // sysex
        {
            size_t i = 0;
            // Sysex messages are split up into several 4 bytes packets
            // first ones use CIN 0x04
            // but packet containing the SysEx stop byte use a different CIN
            for(i = 0; i + 3 < size; i += 3, tx_ptr_ += 4)
            {
                tx_buffer_[tx_ptr_]     = 0x04;
                tx_buffer_[tx_ptr_ + 1] = buffer[i];
                tx_buffer_[tx_ptr_ + 2] = buffer[i + 1];
                tx_buffer_[tx_ptr_ + 3] = buffer[i + 2];
            }

            // Fill CIN for terminating bytes
            // 0x05 for 1 remaining byte
            // 0x06 for 2
            // 0x07 for 3
            tx_buffer_[tx_ptr_] = 0x05 + (size - i - 1);
            tx_ptr_++;
            for(; i < size; ++i, ++tx_ptr_)
                tx_buffer_[tx_ptr_] = buffer[i];
            for(; (tx_ptr_ % 4) != 0; ++tx_ptr_)
                tx_buffer_[tx_ptr_] = 0;
        }
    }
}

void MidiUsbTransport::Impl::MidiToUsb(uint8_t* buffer, size_t size)
{
    // We'll assume your message starts with a status byte!
    size_t status_index = 0;
    while(status_index < size)
    {
        // Search for next status byte or end
        size_t next_status = status_index;
        for(size_t j = status_index + 1; j < size; j++)
        {
            if(buffer[j] & 0x80)
            {
                next_status = j;
                break;
            }
        }
        if(next_status == status_index)
        {
            // Either we're at the end or it's malformed
            next_status = size;
        }
        MidiToUsbSingle(buffer + status_index, next_status - status_index);
        status_index = next_status;
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