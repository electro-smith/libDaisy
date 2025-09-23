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

    /**
     * Send MIDI data using cable 0
     * @param buffer MIDI message bytes
     * @param size Size of the MIDI message
     */
    void Tx(uint8_t* buffer, size_t size);

    /**
     * Send MIDI data to a specific virtual cable (port)
     * @param cable_num Cable number (0-15) representing the virtual MIDI port
     * @param buffer MIDI message bytes
     * @param size Size of the MIDI message
     */
    void Tx(uint8_t cable_num, uint8_t* buffer, size_t size);

    /**
     * Process any received MIDI messages
     * This should be called regularly (e.g., in the main loop or a timer handler)
     * to handle incoming MIDI data through TinyUSB
     */
    void ProcessRx();

    class Impl;

    MidiUsbTransport() : pimpl_(nullptr) {}
    ~MidiUsbTransport() {}
    MidiUsbTransport(const MidiUsbTransport& other)            = default;
    MidiUsbTransport& operator=(const MidiUsbTransport& other) = default;

  private:
    Impl* pimpl_;
};

} // namespace daisy

#endif // __DSY_MIDIUSBTRANSPORT_H__
