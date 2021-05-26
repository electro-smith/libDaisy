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

/** 
    @brief Simple MIDI Handler \n 
    Parses bytes from an input into valid MidiEvents. \n 
    The MidiEvents fill a FIFO queue that the user can pop messages from.
    @author shensley
    @date March 2020
*/
class MidiHandler
{
  public:
    MidiHandler() {}
    ~MidiHandler() {}
    /** Input and Output can be configured separately
    Multiple Input modes can be selected by OR'ing the values.
    */
    enum MidiInputMode
    {
        INPUT_MODE_NONE    = 0x00, /**< & */
        INPUT_MODE_UART1   = 0x01, /**< & */
        INPUT_MODE_USB_INT = 0x02, /**< & */
        INPUT_MODE_USB_EXT = 0x04, /**< & */
    };
    /** Output mode */
    enum MidiOutputMode
    {
        OUTPUT_MODE_NONE    = 0x00, /**< & */
        OUTPUT_MODE_UART1   = 0x01, /**< & */
        OUTPUT_MODE_USB_INT = 0x02, /**< & */
        OUTPUT_MODE_USB_EXT = 0x04, /**< & */
    };


    /** Initializes the MidiHandler 
    \param in_mode Input mode
    \param out_mode Output mode
     */
    void Init(MidiInputMode in_mode, MidiOutputMode out_mode);

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
    MidiInputMode              in_mode_;
    MidiOutputMode             out_mode_;
    UartHandler                uart_;
    ParserState                pstate_;
    MidiEvent                  incoming_message_;
    RingBuffer<MidiEvent, 256> event_q_;
    uint32_t                   last_read_; // time of last byte
    MidiMessageType            running_status_;

    void ClearSysExBuffer(MidiEvent* event);
};

/** @} */
} // namespace daisy
#endif
