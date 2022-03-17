#pragma once
#include <stdint.h>
#include "../util/FIFO.h"
#include "../util/scopedirqblocker.h"

namespace daisy
{
/** @brief A queue that holds user input events in the UI system.
 *  @author jelliesen
 *  @ingroup ui 
 * 
 * A queue that holds user interface events such as button presses or encoder turns.
 * The queue can be filled from hardware drivers and read from a UI object.
 * Access to the queue is protected by a ScopedIrqBlocker - that means it's safe to add
 * events from interrupt handlers.
 */
class UiEventQueue
{
  public:
    /** A button ID used to indicate an invalid or non existing button. */
    static constexpr uint16_t invalidButtonId = UINT16_MAX;

    /** An encoder ID used to indicate an invalid or non existing encoder. */
    static constexpr uint16_t invalidEncoderId = UINT16_MAX;

    /** A potentiometer ID used to indicate an invalid or non existing potentiometer. */
    static constexpr uint16_t invalidPotId = UINT16_MAX;

    /** An event in the queue */
    struct __attribute__((packed)) Event
    {
        /** The type of event */
        enum class EventType : uint8_t
        {
            /** An invalid event. Returned to indicate that no events are left in the queue. */
            invalid,
            /** A button was pressed. */
            buttonPressed,
            /** A button was released. */
            buttonReleased,
            /** An encoder was turned. */
            encoderTurned,
            /** The user has started or stopped turning an encoder. */
            encoderActivityChanged,
            /** A potentiometer was moved. */
            potMoved,
            /** The user has started or stopped moving a potentiometer. */
            potActivityChanged,
        };

        /** Used to indicate if a control is currently being used. */
        enum class ActivityType : uint8_t
        {
            /** The control is not in use at the moment. */
            inactive,
            /** The control is actively used at the moment. */
            active
        };

        /** The type of event that this Event object represents. */
        EventType type;

        union
        {
            struct __attribute__((packed))
            {
                /** The unique ID of the button that was pressed. */
                uint16_t id;
                /** The number of successive button presses (e.g. double click). */
                uint16_t numSuccessivePresses;
                /** True if the event was generated because a button was retriggered 
                 *  automatically while being held down. */
                bool isRetriggering;
            } asButtonPressed;
            struct __attribute__((packed))
            {
                /** The unique ID of the button that was released. */
                uint16_t id;
            } asButtonReleased;
            struct __attribute__((packed))
            {
                /** The unique ID of the encoder that was turned. */
                uint16_t id;
                /** The number of increments detected. */
                int16_t increments;
                /** The total number of increments per revolution. */
                uint16_t stepsPerRev;
            } asEncoderTurned;
            struct __attribute__((packed))
            {
                /** The unique ID of the encoder that is affected. */
                uint16_t id;
                /** The new activity type. */
                ActivityType newActivityType;
            } asEncoderActivityChanged;
            struct __attribute__((packed))
            {
                /** The unique ID of the pot that was moved. */
                uint16_t id;
                /** The new position of the pot. */
                float newPosition;
            } asPotMoved;
            struct __attribute__((packed))
            {
                /** The unique ID of the pot that is affected. */
                uint16_t id;
                /** The new activity type. */
                ActivityType newActivityType;
            } asPotActivityChanged;
        };
    };

    UiEventQueue() {}
    ~UiEventQueue() {}

    /** Adds a Event::EventType::buttonPressed event to the queue. */
    void AddButtonPressed(uint16_t buttonID,
                          uint16_t numSuccessivePresses,
                          bool     isRetriggering = false)
    {
        Event e;
        e.type               = Event::EventType::buttonPressed;
        e.asButtonPressed.id = buttonID;
        e.asButtonPressed.numSuccessivePresses = numSuccessivePresses;
        e.asButtonPressed.isRetriggering       = isRetriggering;
        ScopedIrqBlocker sIrqBl;
        events_.PushBack(e);
    }

    /** Adds a Event::EventType::buttonReleased event to the queue. */
    void AddButtonReleased(uint16_t buttonID)
    {
        Event m;
        m.type                = Event::EventType::buttonReleased;
        m.asButtonReleased.id = buttonID;
        ScopedIrqBlocker sIrqBl;
        events_.PushBack(m);
    }

    /** Adds a Event::EventType::encoderTurned event to the queue. */
    void AddEncoderTurned(uint16_t encoderID,
                          int16_t  increments,
                          uint16_t stepsPerRev)
    {
        Event e;
        e.type                        = Event::EventType::encoderTurned;
        e.asEncoderTurned.id          = encoderID;
        e.asEncoderTurned.increments  = increments;
        e.asEncoderTurned.stepsPerRev = stepsPerRev;
        ScopedIrqBlocker sIrqBl;
        events_.PushBack(e);
    }

    /** Adds a Event::EventType::encoderActivityChanged event to the queue. */
    void AddEncoderActivityChanged(uint16_t encoderId, bool isActive)
    {
        Event e;
        e.type = Event::EventType::encoderActivityChanged;
        e.asEncoderActivityChanged.id = encoderId;
        e.asEncoderActivityChanged.newActivityType
            = isActive ? Event::ActivityType::active
                       : Event::ActivityType::inactive;
        ScopedIrqBlocker sIrqBl;
        events_.PushBack(e);
    }

    /** Adds a Event::EventType::potMoved event to the queue. */
    void AddPotMoved(uint16_t potId, float newPosition)
    {
        Event e;
        e.type                   = Event::EventType::potMoved;
        e.asPotMoved.id          = potId;
        e.asPotMoved.newPosition = newPosition;
        ScopedIrqBlocker sIrqBl;
        events_.PushBack(e);
    }

    /** Adds a Event::EventType::potActivityChanged event to the queue. */
    void AddPotActivityChanged(uint16_t potId, bool isActive)
    {
        Event e;
        e.type                    = Event::EventType::potActivityChanged;
        e.asPotActivityChanged.id = potId;
        e.asPotActivityChanged.newActivityType
            = isActive ? Event::ActivityType::active
                       : Event::ActivityType::inactive;
        ScopedIrqBlocker sIrqBl;
        events_.PushBack(e);
    }

    /** Removes and returns an event from the queue. */
    Event GetAndRemoveNextEvent()
    {
        ScopedIrqBlocker sIrqBl;
        if(events_.IsEmpty())
        {
            Event m;
            m.type = Event::EventType::invalid;
            return m;
        }
        else
        {
            return events_.PopFront();
        }
    }

    /** Returns true, if the queue is empty. */
    bool IsQueueEmpty()
    {
        ScopedIrqBlocker sIrqBl;
        return events_.IsEmpty();
    }

  private:
    FIFO<Event, 256> events_;
};

} // namespace daisy