#pragma once
#ifndef __DSY_MIDIUSBTRANSPORT_H__
#define __DSY_MIDIUSBTRANSPORT_H__

#include "hid/usb.h"
#include "sys/system.h"
#include "util/ringbuffer.h"

namespace daisy
{
/** @brief USB Transport for MIDI 
 *  @ingroup midi
 */
class MidiUsbTransport
{
  public:
    // MidiUsbTransport() {}
    ~MidiUsbTransport() {}

    struct Config
    {
        enum Periph
        {
            INTERNAL = 0,
            EXTERNAL
        };

        Periph periph;
    };

    void Init(Config config);

    void    StartRx();
    size_t  Readable();
    uint8_t Rx();
    bool    RxActive();
    void    FlushRx();
    void    Tx(uint8_t* buffer, size_t size);

    class Impl;

    MidiUsbTransport() : pimpl_(nullptr) {}
    MidiUsbTransport(const MidiUsbTransport& other) = default;
    MidiUsbTransport& operator=(const MidiUsbTransport& other) = default;

  private:
    Impl* pimpl_;
};

} // namespace daisy

#endif // __DSY_MIDIUSBTRANSPORT_H__