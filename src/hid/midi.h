#pragma once
#ifndef DSY_MIDI_H
#define DSY_MIDI_H

// TODO: make this adjustable
#define SYSEX_BUFFER_LEN 1024

#include <stdint.h>
#include <stdlib.h>
#include "per/uart.h"
#include "util/ringbuffer.h"
#include "hid/MidiEvent.h"

namespace daisy
{
/** @addtogroup external 
    @{ 
*/

class MidiUartTransport
{
  public:
    MidiUartTransport() {}
    ~MidiUartTransport() {}

    inline void    Init(UartHandler::Config config) { uart_.Init(config); }
    inline void    StartRx() { uart_.StartRx(); }
    inline size_t  Readable() { return uart_.Readable(); }
    inline uint8_t Rx() { return uart_.PopRx(); }
    inline bool    RxActive() { return uart_.RxActive(); }
    inline void    FlushRx() { uart_.FlushRx(); }
    inline void    Tx(uint8_t* buff, size_t size) { uart_.PollTx(buff, size); }

  private:
    UartHandler uart_;
};


/** 
    @brief Simple MIDI Handler \n 
    Parses bytes from an input into valid MidiEvents. \n 
    The MidiEvents fill a FIFO queue that the user can pop messages from.
    @author shensley
    @date March 2020
*/
template <typename Transport>
class MidiHandler
{
  public:
    MidiHandler() {}
    ~MidiHandler() {}

    struct Config
    {
        typename Transport::Config transport_config;
    };

    /** Initializes the MidiHandler 
    \param in_mode Input mode
    \param out_mode Output mode
     */
    void Init(Config config);

    /** Starts listening on the selected input mode(s). MidiEvent Queue will begin to fill, and can be checked with */
    void StartReceive();

    /** Start listening */
    void Listen();

    /** Feed in bytes to state machine from a queue.
    Populates internal FIFO queue with MIDI Messages
    For example with uart:
    midi.Parse(uart.PopRx());
    \param byte &
    */
    void Parse(uint8_t byte);

    /** Checks if there are unhandled messages in the queue 
    \return True if there are events to be handled, else false.
     */
    bool HasEvents() const { return event_q_.readable(); }


    /** Pops the oldest unhandled MidiEvent from the internal queue
    \return The event to be handled
     */
    MidiEvent PopEvent() { return event_q_.Read(); }

    /** SendMessage
    Send raw bytes as message
    */
    void SendMessage(uint8_t* bytes, size_t size);

  private:
    enum ParserState
    {
        ParserEmpty,
        ParserHasStatus,
        ParserHasData0,
        ParserSysEx,
    };
    UartHandler                uart_;
    ParserState                pstate_;
    MidiEvent                  incoming_message_;
    RingBuffer<MidiEvent, 256> event_q_;
    uint32_t                   last_read_; // time of last byte
    MidiMessageType            running_status_;
    Config                     config_;

    void ClearSysExBuffer(MidiEvent* event);
};

/** @} */

using MidiUartHandler = MidiHandler<MidiUartTransport>;
} // namespace daisy
#endif
