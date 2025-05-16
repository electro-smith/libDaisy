#pragma once
#include <initializer_list>
#include "UiEventQueue.h"
#include "../util/Stack.h"

namespace daisy
{
class UI;

/** @brief The type of arrow button in the UI system.
 *  @author jelliesen
 *  @ingroup ui
 */
enum class ArrowButtonType
{
    /** The left arrow button. */
    left = 0,
    /** The right arrow button. */
    right,
    /** The up arrow button. */
    up,
    /** The down arrow button. */
    down
};

/** @brief A descriptor for a canvas in the UI system.
 *  @author jelliesen
 *  @ingroup ui
 * 
 *  A descriptor for a generic display / led / output device 
 *  that's used in the UI system.
 */
struct UiCanvasDescriptor
{
    /** An id number to tell apart various types of canvases that 
     *  are used concurrently in your system. */
    uint8_t id_;

    /** A pointer to some object that allows to draw to the canvas. 
     *  In your UI pages, you will use the id_ to identify which canvas this is, 
     *  and then cast this pointer to whatever object it represents, e.g. OledDisplay.
     */
    void* handle_;

    /** The desired update rate in ms */
    uint32_t updateRateMs_;

    /** The desired timeout in ms before a display will shut off. 
     *  This defaults to 0, which will keep the display on all the time.
     *  Nonzero values are useful for displays that can suffer from burn-in,
     *  such as OLEDs. 
     */
    uint32_t screenSaverTimeOut = 0;

    bool screenSaverOn = false;

    /** A function to clear the display before the UiPages are drawn. */
    using ClearFuncPtr = void (*)(const UiCanvasDescriptor& canvasToClear);
    ClearFuncPtr clearFunction_;

    /** A function to call when all UIPages have finished the drawing procedure 
     *  and the results can be flushed out to the device.
     */
    using FlushFuncPtr = void (*)(const UiCanvasDescriptor& canvasToFlush);
    FlushFuncPtr flushFunction_;
};

class OneBitGraphicsLookAndFeel;

/** @brief The base class for a page in the UI system.
 *  @author jelliesen
 *  @ingroup ui
 */
class UiPage
{
  public:
    UiPage() : parent_(nullptr) {}

    virtual ~UiPage() {}

    /** Returns true, if the page fills the entire canvas. A canvas can
     *  be individual leds, text displays, alphanumeric displays, graphics
     *  displays, etc. The UI class will use this to determine if underlying pages
     *  must be drawn before this page.
     */
    virtual bool IsOpaque(const UiCanvasDescriptor& display)
    {
        (void)(display); // silence unused variable warnings
        return true;
    }

    /** Returns true if the page is currently active on a UI - it may not be visible, though. */
    bool IsActive() { return parent_ != nullptr; }

    /** Called on any user input event, after the respective callback has completed.
     * OnUserInteraction will be invoked for all pages in the page stack and can be used to 
     * track general user activity. */
    virtual void OnUserInteraction() {}

    /** Closes the current page. This calls the parent UI and asks it to Remove this page
     *  from the page stack.
     */
    void Close();

    /** Called when the okay button is pressed or released.
     *  @param numberOfPresses     Holds the number of successive button presses.
     *                          It will be 1 on the first call and increasing by 1
     *                          with each successive call. A button down event is
     *                          signaled by numberOfButtonPresses == 0.
     *  @param isRetriggering   True if the button is auto-retriggering (due to being 
     *                          held down)
     *  @returns    false, if you want the event to be passed on to the page below.
     */
    virtual bool OnOkayButton(uint8_t numberOfPresses, bool isRetriggering)
    {
        (void)(numberOfPresses); // silence unused variable warnings
        (void)(isRetriggering);  // silence unused variable warnings
        return true;
    }

    /** Called when the cancel button is pressed or released.
     *  @param numberOfPresses     Holds the number of successive button presses.
     *                          It will be 1 on the first call and increasing by 1
     *                          with each successive call. A button down event is
     *                          signaled by numberOfButtonPresses == 0.
     *  @param isRetriggering   True if the button is auto-retriggering (due to being 
     *                          held down)
     *  @returns    false, if you want the event to be passed on to the page below.
     */
    virtual bool OnCancelButton(uint8_t numberOfPresses, bool isRetriggering)
    {
        (void)(numberOfPresses); // silence unused variable warnings
        (void)(isRetriggering);  // silence unused variable warnings
        return true;
    }

    /** Called when an arrow button is pressed or released.
     *  @param arrowType         The arrow button affected.
     *  @param numberOfPresses     Holds the number of successive button presses.
     *                          It will be 1 on the first call and increasing by 1
     *                          with each successive call. A button down event is
     *                          signaled by numberOfButtonPresses == 0.
     *  @param isRetriggering   True if the button is auto-retriggering (due to being 
     *                          held down)
     *  @returns    false, if you want the event to be passed on to the page below.
     */
    virtual bool OnArrowButton(ArrowButtonType arrowType,
                               uint8_t         numberOfPresses,
                               bool            isRetriggering)
    {
        (void)(arrowType);       // silence unused variable warnings
        (void)(numberOfPresses); // silence unused variable warnings
        (void)(isRetriggering);  // silence unused variable warnings
        return true;
    }

    /** Called when the function button is pressed or released.
     *  @param numberOfPresses     Holds the number of successive button presses.
     *                          It will be 1 on the first call and increasing by 1
     *                          with each successive call. A button down event is
     *                          signaled by numberOfButtonPresses == 0.
     *  @param isRetriggering   True if the button is auto-retriggering (due to being 
     *                          held down)
     *  @returns    false, if you want the event to be passed on to the page below.
     */
    virtual bool OnFunctionButton(uint8_t numberOfPresses, bool isRetriggering)
    {
        (void)(numberOfPresses); // silence unused variable warnings
        (void)(isRetriggering);  // silence unused variable warnings
        return true;
    }

    /** Called when any button is pressed or released that is not an arrow button,
     *  the function button or the okay / cancel buttons.
     *  @param buttonID         The ID of the affected button.
     *  @param numberOfPresses     Holds the number of successive button presses.
     *                          It will be 1 on the first call and increasing by 1
     *                          with each successive call. A button down event is
     *                          signaled by numberOfButtonPresses == 0.
     *  @param isRetriggering   True if the button is auto-retriggering (due to being 
     *                          held down)
     *  @returns    false, if you want the event to be passed on to the page below.
     */
    virtual bool
    OnButton(uint16_t buttonID, uint8_t numberOfPresses, bool isRetriggering)
    {
        (void)(buttonID);        // silence unused variable warnings
        (void)(numberOfPresses); // silence unused variable warnings
        (void)(isRetriggering);  // silence unused variable warnings
        return true;
    }

    /** Called when the menu encoder is turned.
     *  @param turns                 The number of increments, positive is clockwise.
     *  @param stepsPerRevolution    The total number of increments per revolution on this encoder.
     *  @returns    false, if you want the event to be passed on to the page below.
     */
    virtual bool OnMenuEncoderTurned(int16_t turns, uint16_t stepsPerRevolution)
    {
        (void)(turns);              // silence unused variable warnings
        (void)(stepsPerRevolution); // silence unused variable warnings
        return true;
    }

    /** Called when the menu encoder is turned.
     *  @param turns                 The number of increments, positive is clockwise.
     *  @param stepsPerRevolution    The total number of increments per revolution on this encoder.
     *  @returns    false, if you want the event to be passed on to the page below.
     */
    virtual bool OnValueEncoderTurned(int16_t  turns,
                                      uint16_t stepsPerRevolution)
    {
        (void)(turns);              // silence unused variable warnings
        (void)(stepsPerRevolution); // silence unused variable warnings
        return true;
    }

    /** Called when an encoder is turned that is not the menu encoder or the value encoder.
     *  @param encoderID            The ID of the affected encoder.
     *  @param turns                 The number of increments, positive is clockwise.
     *  @param stepsPerRevolution    The total number of increments per revolution on this encoder.
     *  @returns    false, if you want the event to be passed on to the page below.
     */
    virtual bool OnEncoderTurned(uint16_t encoderID,
                                 int16_t  turns,
                                 uint16_t stepsPerRevolution)
    {
        (void)(encoderID);          // silence unused variable warnings
        (void)(turns);              // silence unused variable warnings
        (void)(stepsPerRevolution); // silence unused variable warnings
        return true;
    }

    /** Called when the user starts or stops turning the menu encoder.
     *  @param isCurrentlyActive    True, if the user currently moves this encoder.
     *  @returns    false, if you want the event to be passed on to the page below.
     */
    virtual bool OnMenuEncoderActivityChanged(bool isCurrentlyActive)
    {
        (void)(isCurrentlyActive); // silence unused variable warnings
        return true;
    }

    /** Called when the user starts or stops turning the value encoder.
     *  @param isCurrentlyActive    True, if the user currently moves this encoder.
     *  @returns    false, if you want the event to be passed on to the page below.
     */
    virtual bool OnValueEncoderActivityChanged(bool isCurrentlyActive)
    {
        (void)(isCurrentlyActive); // silence unused variable warnings
        return true;
    }

    /** Called when the user starts or stops turning an encoder that is not the menu 
     *  encoder or the value encoder.
     *  @param encoderID            The ID of the affected encoder.
     *  @param isCurrentlyActive    True, if the user currently moves this encoder.
     *  @returns    false, if you want the event to be passed on to the page below.
     */
    virtual bool OnEncoderActivityChanged(uint16_t encoderID,
                                          bool     isCurrentlyActive)
    {
        (void)(encoderID);         // silence unused variable warnings
        (void)(isCurrentlyActive); // silence unused variable warnings
        return true;
    }

    /** Called when the value potentiometer is turned.
     *  @param newPosition             The new position in the range 0 .. 1
     *  @returns    false, if you want the event to be passed on to the page below.
     */
    virtual bool OnValuePotMoved(float newPosition)
    {
        (void)(newPosition); // silence unused variable warnings
        return true;
    }

    /** Called when a potentiometer is turned that's not the
     *  value potentiometer.
     *  @param potID                The ID of the affected potentiometer.
     *  @param newPosition             The new position in the range 0 .. 1
     *  @returns    false, if you want the event to be passed on to the page below.
     */
    virtual bool OnPotMoved(uint16_t potID, float newPosition)
    {
        (void)(potID);       // silence unused variable warnings
        (void)(newPosition); // silence unused variable warnings
        return true;
    }

    /** Called when the user starts or stops turning the value potentiometer.
     *  @param isCurrentlyActive    True, if the user currently moves this potentiometer.
     *  @returns    false, if you want the event to be passed on to the page below.
     */
    virtual bool OnValuePotActivityChanged(bool isCurrentlyActive)
    {
        (void)(isCurrentlyActive); // silence unused variable warnings
        return true;
    }

    /** Called when the user starts or stops turning a potentiometer that's not the
     *  value potentiometer.
     *  @param potID                The ID of the affected potentiometer.
     *  @param isCurrentlyActive    True, if the user currently moves this potentiometer.
     *  @returns    false, if you want the event to be passed on to the page below.
     */
    virtual bool OnPotActivityChanged(uint16_t potID, bool isCurrentlyActive)
    {
        (void)(potID);             // silence unused variable warnings
        (void)(isCurrentlyActive); // silence unused variable warnings
        return true;
    }

    /** Called when the page is added to the UI. */
    virtual void OnShow(){};

    /** Called when the page is removed from the UI. */
    virtual void OnHide(){};

    /** Called when the page becomes the topmost page in the page stack. */
    virtual void OnFocusGained(){};

    /** Called when the page is no longer the topmost page in the page stack. */
    virtual void OnFocusLost(){};

    /** Called to make the UIPage repaint everything on a canvas. 
     *  Check the ID to determine which display this corresponds to.
     *  Cast the handle to the corresponding type and do your draw operations on it.
     */
    virtual void Draw(const UiCanvasDescriptor& canvas) = 0;

    /** Returns a reference to the parent UI object, or nullptr if not added to any UI at the moment. */
    UI* GetParentUI() { return parent_; }
    /** Returns a reference to the parent UI object, or nullptr if not added to any UI at the moment. */
    const UI* GetParentUI() const { return parent_; }

  private:
    friend class UI;
    UI* parent_;
};

/** @brief A generic UI system
 *  @author jelliesen
 *  @ingroup ui
 *
 *  This system allows you to create complex and dynamic user interfaces 
 *  with menus, pages and dialogs. It holds a stack of pages. Each page 
 *  can react to user input on buttons, potentiometers, and encoders
 *  while drawing to one or multiple displays, leds or other output devices.
 * 
 *  User input is consumed from a UiEventQueue and distributed to the pages
 *  from the top down. If a page doesn't handle an event, it will be 
 *  forwarded to the next page below.
 * 
 *  Pages are drawn from the bottom up. Multiple abstract canvases can be 
 *  used for the drawing, where each canvas could be a graphics display, 
 *  LEDs, alphanumeric displays, etc. The UI system makes sure that drawing 
 *  is executed with a constant refresh rate that can be individually 
 *  specified for each canvas.
 */
class UI
{
  public:
    UI();
    ~UI();

    /** Contains information about the control IDs used for special functions
     *  such as arrow buttons, okay/cancel, function buttons, value sliders, etc.
     *  If such a control is available, set the corresponding variable to 
     *  the control ID that's used when events are pushed to the UiEventQueue.
     *  If such a control is not available, use UiEventQueue::invalidButtonId,
     *  UiEventQueue::invalidEncoderId or UiEventQueue::invalidPotId.
     */
    struct SpecialControlIds
    {
        uint16_t funcBttnId   = UiEventQueue::invalidButtonId;
        uint16_t okBttnId     = UiEventQueue::invalidButtonId;
        uint16_t cancelBttnId = UiEventQueue::invalidButtonId;
        uint16_t upBttnId     = UiEventQueue::invalidButtonId;
        uint16_t downBttnId   = UiEventQueue::invalidButtonId;
        uint16_t leftBttnId   = UiEventQueue::invalidButtonId;
        uint16_t rightBttnId  = UiEventQueue::invalidButtonId;
        /** navigates through menu selections */
        uint16_t menuEncoderId = UiEventQueue::invalidEncoderId;
        /** changes the value of selected menu items */
        uint16_t valueEncoderId = UiEventQueue::invalidEncoderId;
        /** changes the value of selected menu items (= old school "value slider") */
        uint16_t valuePotId = UiEventQueue::invalidPotId;
    };

    /** Use this to denote a nonexistent / invalid canvas ID */
    static constexpr uint16_t invalidCanvasId = uint16_t(-1);

    /** Initializes the UI.
     *  @param inputQueue           The UiEventQueue to read user input events from.
     *  @param specialControlIds    Information about the control IDs used for special buttons/encoders/pots.
     *  @param canvases             A list of UiCanvasDescriptor that define which canvases to use.
     *  @param primaryOneBitGraphicsDisplayId 
     *                              The ID of a OneBitGraphicsDisplay canvas that should be used as the 
     *                              main display. Menus will draw to this canvas. If no such dispaly is available,
     *                              use Ui::invalidCanvasId.
     * 
     */
    void Init(UiEventQueue&                             inputQueue,
              const SpecialControlIds&                  specialControlIds,
              std::initializer_list<UiCanvasDescriptor> canvases,
              uint16_t primaryOneBitGraphicsDisplayId = invalidCanvasId);

    /** Call this regularly to allow processing user input, redraw canvases
     *  and do other "housekeeping" work. This is best done from a low priority context,
     *  ideally from your main() idle loop.
     **/
    void Process();

    /** Call this to temporarily disable processing of user input, e.g.
     *  while a project is loading. If queueEvents==true, all user input
     *  that happens while muted will be queued up and processed when the
     *  mute state is removed. If queueEvents==false, all user input that
     *  happens while muted will be discarded.
     */
    void Mute(bool shouldBeMuted, bool queueEvents = false);

    /** Adds a new UIPage on the top of the stack of UI pages without
     *  taking ownership of the object. The new page is set to be visible.
     *  The page must stay alive until it was removed from the UI.
     *  It's best to have each page statically allocated as a global variable.
     */
    void OpenPage(UiPage& page);

    /** Called to close a page. */
    void ClosePage(UiPage& page);

    /** If this UI has a canvas that uses a OneBitGraphicsDisplay AND this canvas should be used 
     *  as the main display for menus, etc. then this function returns the canvas ID of this display.
     *  If no such canvas exists, this function returns UI::invalidCanvasId.
     */
    uint16_t GetPrimaryOneBitGraphicsDisplayId() const
    {
        return primaryOneBitGraphicsDisplayId_;
    }

    /** Returns the button IDs, encoder IDs and pot IDs used for special functions. */
    SpecialControlIds GetSpecialControlIds() const
    {
        return specialControlIds_;
    }

  private:
    bool                                       isMuted_;
    bool                                       queueEvents_;
    static constexpr int                       kMaxNumPages    = 32;
    static constexpr int                       kMaxNumCanvases = 8;
    Stack<UiPage*, kMaxNumPages>               pages_;
    Stack<UiCanvasDescriptor, kMaxNumCanvases> canvases_;
    uint32_t          lastUpdateTimes_[kMaxNumCanvases];
    uint32_t          lastEventTime_;
    UiEventQueue*     eventQueue_;
    SpecialControlIds specialControlIds_;
    uint16_t          primaryOneBitGraphicsDisplayId_ = invalidCanvasId;

    // internal
    void RemovePage(UiPage* page);
    void AddPage(UiPage* p);
    void ProcessEvent(const UiEventQueue::Event& m);
    void RedrawCanvas(uint8_t index, uint32_t currentTimeInMs);
    void ForwardToButtonHandler(uint16_t buttonID,
                                uint8_t  numberOfPresses,
                                bool     isRetriggering);
    void RebuildPageVisibilities();
};

} // namespace daisy
