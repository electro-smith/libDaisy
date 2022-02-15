#pragma once
#ifndef DSY_MIDI_DISTRIBUTOR_H
#define DSY_MIDI_DISTRIBUTOR_H

#include "hid/MidiEvent.h"
#include "hid/midi.h"

namespace daisy
{
/** @brief   Base class for anything that wants to send and receive midi
 *  @ingroup midi
*/
class MidiEventListener
{
  public:
    virtual ~MidiEventListener() {}

    virtual void OnNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {}
    virtual void OnNoteOff(uint8_t channel, uint8_t note) {}
};

/** @brief Manage a group of listeners and call callbacks as appropriate
 *  @ingroup midi
*/
template <size_t kMaxNumListeners = 16>
class MidiDistributor
{
  public:
    MidiDistributor() {}
    ~MidiDistributor() {}

    struct ListenerStack
    {
        void AddListener(MidiEventListener l)
        {
            if(idx < (kMaxNumListeners - 1))
            {
                idx++;
                listeners[idx] = l;
            }
        }

        void RemoveListener()
        {
            if(idx > 0)
            {
                idx--;
            }
        }

        void NoteOns(uint8_t channel, uint8_t note, uint8_t velocity){
          for(int i = 0; i < idx; i++){
            listeners[i].OnNoteOn(channel, note, velocity);
          }
        }

        void NoteOffs(uint8_t channel, uint8_t note){
          for(int i = 0; i < idx; i++){
            listeners[i].OnNoteOff(channel, note);
          }
        }

      private:
        MidiEventListener listeners[kMaxNumListeners];
        int               idx;
    };

    void AddListener(MidiEventListener& listener)
    {
        listeners_.PushListener(listener);
    }

    void RemoveListener(MidiEventListener& listener)
    {
        listeners_.PopListener();
    }

    void DistributeEvent(MidiEvent ev)
    {
        switch(ev.type)
        {
            case MidiMessageType::NoteOn:
            {
                NoteOnEvent parsed = ev.AsNoteOn();
                listeners_.NoteOns(parsed.channel, parsed.note, parsed.velocity);
            }
            break;
            case MidiMessageType::NoteOff:
            {
                NoteOffEvent parsed = ev.AsNoteOff();
                listeners_.NoteOffs(parsed.channel, parsed.note);
            }
            break;
                // more types handled here ...
        }
    }

  private:
    template <typename... MethodArgs, typename... Args>
    void
    CallListeners(void (MidiEventListener::*callbackFunction)(MethodArgs...),
                  Args&&... args)
    {
        // for(size_t i = 0; i < listeners_.GetNumElements(); i++)
        //     (listeners_[i].*callbackFunction)(args...);
    }

    ListenerStack listeners_;
    // Stack<MidiEventListener&, kMaxNumListeners> listeners_;
};

} // namespace daisy
#endif