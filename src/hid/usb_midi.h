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
    typedef void (*MidiRxParseCallback)(uint8_t* data,
                                        size_t   size,
                                        void*    context);

    struct Config
    {
        enum Periph
        {
            INTERNAL = 0,
            EXTERNAL,
            HOST
        };

        Periph periph;

        /**
         * When sending MIDI messages immediately back-to-back in user code,
         * sometimes the USB CDC driver is still "busy".
         *
         * This option configures the number of times to retry a Tx after
         * delaying for 100 microseconds (default = 3 retries).
         *
         * If you set this to zero, Tx will not retry so the attempt will block
         * for slightly less time, but transmit can fail if the Tx state is busy.
         */
        uint8_t tx_retry_count;

        Config() : periph(INTERNAL), tx_retry_count(3) {}
    };

    void Init(Config config);

    void StartRx(MidiRxParseCallback callback, void* context);
    bool RxActive();
    void FlushRx();
    void Tx(uint8_t* buffer, size_t size);

    class Impl;

    MidiUsbTransport() : pimpl_(nullptr) {}
    ~MidiUsbTransport() {}
    MidiUsbTransport(const MidiUsbTransport& other) = default;
    MidiUsbTransport& operator=(const MidiUsbTransport& other) = default;

  private:
    Impl* pimpl_;
};

} // namespace daisy

#endif // __DSY_MIDIUSBTRANSPORT_H__
