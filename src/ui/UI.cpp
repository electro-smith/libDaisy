#include "UI.h"
#include "../sys/system.h"

namespace daisy
{
void UiPage::Close()
{
    if(parent_ != nullptr)
        parent_->ClosePage(*this);
}

// =========================================================================

// =========================================================================

UI::UI() {}

void UI::Init(UiEventQueue&                             inputQueue,
              const SpecialControlIds&                  specialControlIds,
              std::initializer_list<UiCanvasDescriptor> canvases,
              uint16_t primaryOneBitGraphicsDisplayId)
{
    isMuted_                        = false;
    queueEvents_                    = false;
    eventQueue_                     = &inputQueue;
    specialControlIds_              = specialControlIds;
    canvases_                       = decltype(canvases_)(canvases);
    primaryOneBitGraphicsDisplayId_ = primaryOneBitGraphicsDisplayId;

    for(int i = 0; i < kMaxNumCanvases; i++)
        lastUpdateTimes_[i] = 0;
}

UI::~UI()
{
    while(pages_.GetNumElements() > 0)
        ClosePage(*pages_[pages_.GetNumElements() - 1]);
}

void UI::Process()
{
    const auto currentTimeInMs = System::GetNow();

    // handle user input
    if(!isMuted_)
    {
        while(!eventQueue_->IsQueueEmpty())
        {
            // clear next event if screen is off
            for(uint32_t i = 0; i < canvases_.GetNumElements(); i++)
            {
                if(canvases_[i].screenSaverOn)
                {
                    eventQueue_->GetAndRemoveNextEvent();
                    canvases_[i].screenSaverOn = false;
                    break;
                }
            }

            lastEventTime_        = currentTimeInMs;
            UiEventQueue::Event e = eventQueue_->GetAndRemoveNextEvent();
            if(e.type != UiEventQueue::Event::EventType::invalid)
            {
                ProcessEvent(e);
                for(int32_t i = pages_.GetNumElements() - 1; i >= 0; i--)
                    pages_[i]->OnUserInteraction();
            }
        }
    }
    else if(!queueEvents_)
    {
        // drop events
        while(!eventQueue_->IsQueueEmpty())
            eventQueue_->GetAndRemoveNextEvent();
    }

    // redraw canvases
    for(uint32_t i = 0; i < canvases_.GetNumElements(); i++)
    {
        if(canvases_[i].screenSaverTimeOut == 0
           || currentTimeInMs - lastEventTime_
                  < canvases_[i].screenSaverTimeOut)
        {
            const uint32_t timeDiff = currentTimeInMs - lastUpdateTimes_[i];
            if(timeDiff > canvases_[i].updateRateMs_)
                RedrawCanvas(i, currentTimeInMs);
        }
        else
        { // turn off oled
            canvases_[i].clearFunction_(canvases_[i]);
            canvases_[i].flushFunction_(canvases_[i]);
            canvases_[i].screenSaverOn = true;
        }
    }
}

/** Call this to temporarily disable processing of user input, e.g.
 *  while a project is loading. If queueEvents==true, all user input
 *  that happens while muted will be queued up and processed when the
 *  mute state is removed. If queueEvents==false, all user input that
 *  happens while muted will be discarded.
 */
void UI::Mute(bool shouldBeMuted, bool queueEvents)
{
    isMuted_     = shouldBeMuted;
    queueEvents_ = queueEvents;
}

/** Adds a new UIPage on the top of the stack of UI pages without
 *  taking ownership of the object.
 */
void UI::OpenPage(UiPage& page)
{
    if(page.parent_ != nullptr)
        return;
    if(pages_.IsFull())
        return;

    pages_.PushBack(&page);
    page.parent_ = this;
    page.OnShow();

    // was there a page below?
    if(pages_.GetNumElements() > 1)
        // Remove focus
        pages_[pages_.GetNumElements() - 2]->OnFocusLost();
    page.OnFocusGained();
}

/** Called to close a page: */
void UI::ClosePage(UiPage& page)
{
    if(page.parent_ != this)
        return;

    // find page index
    int pageIndex = -1;
    for(int32_t i = pages_.GetNumElements() - 1; i >= 0; i--)
    {
        if(pages_[i] == &page)
        {
            pageIndex = i;
            break;
        }
    }
    if(pageIndex < 0)
        return;

    if(pageIndex == int(pages_.GetNumElements()) - 1)
    {
        // this is the topmost page, give focus to the page below
        page.OnFocusLost();
        if(pageIndex >= 1)
            pages_[pageIndex - 1]->OnFocusGained();
    }

    // remove from page stack
    pages_.Remove(pageIndex);

    // close the page
    page.OnHide();
    page.parent_ = nullptr;
}

void UI::ProcessEvent(const UiEventQueue::Event& e)
{
    switch(e.type)
    {
        case UiEventQueue::Event::EventType::buttonPressed:
            ForwardToButtonHandler(e.asButtonPressed.id,
                                   e.asButtonPressed.numSuccessivePresses,
                                   e.asButtonPressed.isRetriggering);
            break;
        case UiEventQueue::Event::EventType::buttonReleased:
            ForwardToButtonHandler(e.asButtonReleased.id, 0, false);
            break;
        case UiEventQueue::Event::EventType::encoderTurned:
            for(int32_t i = pages_.GetNumElements() - 1; i >= 0; i--)
            {
                const auto id          = e.asEncoderTurned.id;
                const auto increments  = e.asEncoderTurned.increments;
                const auto stepsPerRev = e.asEncoderTurned.stepsPerRev;
                if(id == specialControlIds_.menuEncoderId)
                {
                    if(pages_[i]->OnMenuEncoderTurned(increments, stepsPerRev))
                        break;
                }
                else if(id == specialControlIds_.valueEncoderId)
                {
                    if(pages_[i]->OnValueEncoderTurned(increments, stepsPerRev))
                        break;
                }
                else if(pages_[i]->OnEncoderTurned(id, increments, stepsPerRev))
                    break;
            }
            break;
        case UiEventQueue::Event::EventType::encoderActivityChanged:
            for(int32_t i = pages_.GetNumElements() - 1; i >= 0; i--)
            {
                const auto id = e.asEncoderActivityChanged.id;
                const auto activityType
                    = e.asEncoderActivityChanged.newActivityType;
                const bool IsActive
                    = activityType == UiEventQueue::Event::ActivityType::active;
                if(id == specialControlIds_.menuEncoderId)
                {
                    if(pages_[i]->OnMenuEncoderActivityChanged(IsActive))
                        break;
                }
                else if(id == specialControlIds_.valueEncoderId)
                {
                    if(pages_[i]->OnValueEncoderActivityChanged(IsActive))
                        break;
                }
                else if(pages_[i]->OnEncoderActivityChanged(id, IsActive))
                    break;
            }
            break;
        case UiEventQueue::Event::EventType::potMoved:
            for(int32_t i = pages_.GetNumElements() - 1; i >= 0; i--)
            {
                const auto id          = e.asPotMoved.id;
                const auto newPosition = e.asPotMoved.newPosition;
                if(id == specialControlIds_.valuePotId)
                {
                    if(pages_[i]->OnValuePotMoved(newPosition))
                        break;
                }
                else if(pages_[i]->OnPotMoved(id, newPosition))
                    break;
            }
            break;
        case UiEventQueue::Event::EventType::potActivityChanged:
            for(int32_t i = pages_.GetNumElements() - 1; i >= 0; i--)
            {
                const auto id = e.asPotActivityChanged.id;
                const auto activityType
                    = e.asPotActivityChanged.newActivityType;
                const bool IsActive
                    = activityType == UiEventQueue::Event::ActivityType::active;
                if(id == specialControlIds_.valuePotId)
                {
                    if(pages_[i]->OnValuePotActivityChanged(IsActive))
                        break;
                }
                else if(pages_[i]->OnPotActivityChanged(id, IsActive))
                    break;
            }
            break;
        case UiEventQueue::Event::EventType::invalid:
        default: break;
    }
}

void UI::RedrawCanvas(uint8_t index, uint32_t currentTimeInSysticks)
{
    UiCanvasDescriptor& canvas = canvases_[index];

    // find the bottom most page to draw, then draw the pages upwards from there
    int firstToDraw;
    for(firstToDraw = int(pages_.GetNumElements()) - 1; firstToDraw >= 0;
        firstToDraw--)
    {
        if(pages_[firstToDraw]->IsOpaque(canvas))
            break;
    }

    // all pages are transparent - start with the page on the bottom
    if(firstToDraw < 0)
        firstToDraw = 0;

    // clear canvas
    canvas.clearFunction_(canvas);

    // draw pages
    for(uint32_t i = firstToDraw; i < pages_.GetNumElements(); i++)
    {
        pages_[i]->Draw(canvas);
    }

    // flush canvas to the hardware
    canvas.flushFunction_(canvas);
    lastUpdateTimes_[index] = currentTimeInSysticks;
}

void UI::ForwardToButtonHandler(const uint16_t buttonID,
                                const uint8_t  numberOfPresses,
                                bool           isRetriggering)
{
    if(buttonID == specialControlIds_.okBttnId)
    {
        for(int i = int(pages_.GetNumElements()) - 1; i >= 0; i--)
        {
            if(pages_[i]->OnOkayButton(numberOfPresses, isRetriggering))
                break;
        }
    }
    else if(buttonID == specialControlIds_.cancelBttnId)
    {
        for(int i = int(pages_.GetNumElements()) - 1; i >= 0; i--)
        {
            if(pages_[i]->OnCancelButton(numberOfPresses, isRetriggering))
                break;
        }
    }
    else if(buttonID == specialControlIds_.funcBttnId)
    {
        for(int i = int(pages_.GetNumElements()) - 1; i >= 0; i--)
        {
            if(pages_[i]->OnFunctionButton(numberOfPresses, isRetriggering))
                break;
        }
    }
    else if(buttonID == specialControlIds_.leftBttnId)
    {
        for(int i = int(pages_.GetNumElements()) - 1; i >= 0; i--)
        {
            if(pages_[i]->OnArrowButton(
                   ArrowButtonType::left, numberOfPresses, isRetriggering))
                break;
        }
    }
    else if(buttonID == specialControlIds_.rightBttnId)
    {
        for(int i = int(pages_.GetNumElements()) - 1; i >= 0; i--)
        {
            if(pages_[i]->OnArrowButton(
                   ArrowButtonType::right, numberOfPresses, isRetriggering))
                break;
        }
    }
    else if(buttonID == specialControlIds_.upBttnId)
    {
        for(int i = int(pages_.GetNumElements()) - 1; i >= 0; i--)
        {
            if(pages_[i]->OnArrowButton(
                   ArrowButtonType::up, numberOfPresses, isRetriggering))
                break;
        }
    }
    else if(buttonID == specialControlIds_.downBttnId)
    {
        for(int i = int(pages_.GetNumElements()) - 1; i >= 0; i--)
        {
            if(pages_[i]->OnArrowButton(
                   ArrowButtonType::down, numberOfPresses, isRetriggering))
                break;
        }
    }
    else
    {
        for(int i = int(pages_.GetNumElements()) - 1; i >= 0; i--)
        {
            if(pages_[i]->OnButton(buttonID, numberOfPresses, isRetriggering))
                break;
        }
    }
}

} // namespace daisy