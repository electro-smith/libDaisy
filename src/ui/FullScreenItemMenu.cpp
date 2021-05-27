#include "FullScreenItemMenu.h"

namespace daisy
{
void FullScreenItemMenu::Init(const AbstractMenu::ItemConfig* items,
                              uint16_t                        numItems,
                              AbstractMenu::Orientation       orientation,
                              bool                            allowEntering)
{
    AbstractMenu::Init(items, numItems, orientation, allowEntering);
}

void FullScreenItemMenu::SetOneBitGraphicsDisplayToDrawTo(uint16_t canvasId)
{
    canvasIdToDrawTo_ = canvasId;
}

void FullScreenItemMenu::Draw(const UiCanvasDescriptor& canvas)
{
    // no items or out of bounds??!
    if((selectedItemIdx_ < 0) || (selectedItemIdx_ >= numItems_))
        return;

    // Find out if this canvas is one we should draw to.
    if(canvasIdToDrawTo_ == UI::invalidCanvasId)
    {
        // We're configured to use the UIs default canvas.
        auto* ui = GetParentUI();
        if(!ui)
            // No parent UI?! How are we supposed to find out what cannvas to draw to?
            return;

        if(ui->GetPrimaryOneBitGraphicsDisplayId() != canvas.id_)
            // This is not the default canvas! Don't draw here.
            return;
    }
    else if(canvasIdToDrawTo_ != canvas.id_)
        // we're configured to draw to a specific canvas, but not this one.
        return;

    // If we end uo here, this canvas is the one we should draw to.
    OneBitGraphicsDisplay& display = *(OneBitGraphicsDisplay*)(canvas.handle_);

    // make the current LookAndFeel draw the item
    const auto& item = items_[selectedItemIdx_];
    const auto  type = item.type;
    bool isVertical  = orientation_ == Orientation::upDownSelectLeftRightModify;
    switch(type)
    {
        case ItemType::callbackFunctionItem:
            DrawTextItem(
                display, isVertical, selectedItemIdx_, numItems_, item.text);
            break;
        case ItemType::checkboxItem:
            DrawCheckboxItem(display,
                             isVertical,
                             selectedItemIdx_,
                             numItems_,
                             item.text,
                             *item.asCheckboxItem.valueToModify);
            break;
        case ItemType::valueItem:
            DrawValueItem(display,
                          isVertical,
                          selectedItemIdx_,
                          numItems_,
                          item.text,
                          *item.asMappedValueItem.valueToModify,
                          isEditing_);
            break;
        case ItemType::openUiPageItem:
            DrawOpenUiPageItem(
                display, isVertical, selectedItemIdx_, numItems_, item.text);
            break;
        case ItemType::closeMenuItem:
            DrawCloseMenuItem(
                display, isVertical, selectedItemIdx_, numItems_, item.text);
            break;
        case ItemType::customItem:
            item.asCustomItem.itemObject->Draw(display,
                                               selectedItemIdx_,
                                               numItems_,
                                               display.GetBounds(),
                                               isEditing_);
            break;
    }
}

//////////////////////////////////////////////////////////////////////
// Drawing routines
//////////////////////////////////////////////////////////////////////

int FullScreenItemMenu::GetTopRowHeight(int displayHeight) const
{
    return displayHeight / 2;
}

void FullScreenItemMenu::DrawTextItem(OneBitGraphicsDisplay& display,
                                      bool                   isVertical,
                                      uint16_t               selectedItemIdx,
                                      uint16_t               numItems,
                                      const char*            itemText) const
{
    auto       remainingBounds = display.GetBounds();
    const auto topRowHeight    = GetTopRowHeight(remainingBounds.GetHeight());
    const auto topRowRect      = remainingBounds.RemoveFromTop(topRowHeight);
    DrawTopRow(display,
               isVertical,
               selectedItemIdx,
               numItems,
               itemText,
               topRowRect,
               true);
}

void FullScreenItemMenu::DrawCheckboxItem(OneBitGraphicsDisplay& display,
                                          bool                   isVertical,
                                          uint16_t    selectedItemIdx,
                                          uint16_t    numItems,
                                          const char* itemText,
                                          const bool& isCheckboxTicked) const
{
    auto       remainingBounds = display.GetBounds();
    const auto topRowHeight    = GetTopRowHeight(remainingBounds.GetHeight());
    const auto topRowRect      = remainingBounds.RemoveFromTop(topRowHeight);
    DrawTopRow(display,
               isVertical,
               selectedItemIdx,
               numItems,
               itemText,
               topRowRect,
               true);

    // draw the checkbox
    auto checkboxBounds = remainingBounds.WithSizeKeepingCenter(12, 12);
    display.DrawRect(checkboxBounds, true, false);
    if(isCheckboxTicked)
        display.DrawRect(checkboxBounds.Reduced(3), true, true);
}

void FullScreenItemMenu::DrawValueItem(OneBitGraphicsDisplay& display,
                                       bool                   isVertical,
                                       uint16_t               selectedItemIdx,
                                       uint16_t               numItems,
                                       const char*            itemText,
                                       const MappedValue&     value,
                                       bool                   isEditing) const
{
    auto       remainingBounds = display.GetBounds();
    const auto topRowHeight    = GetTopRowHeight(remainingBounds.GetHeight());
    const auto topRowRect      = remainingBounds.RemoveFromTop(topRowHeight);
    DrawTopRow(display,
               isVertical,
               selectedItemIdx,
               numItems,
               itemText,
               topRowRect,
               !isEditing);

    // draw the value
    FixedCapStr<20> valueStr;
    value.AppentToString(valueStr);
    DrawValueText(display, isVertical, valueStr, remainingBounds, isEditing);
}

void FullScreenItemMenu::DrawOpenUiPageItem(OneBitGraphicsDisplay& display,
                                            bool                   isVertical,
                                            uint16_t    selectedItemIdx,
                                            uint16_t    numItems,
                                            const char* itemText) const
{
    auto       remainingBounds = display.GetBounds();
    const auto topRowHeight    = GetTopRowHeight(remainingBounds.GetHeight());
    const auto topRowRect      = remainingBounds.RemoveFromTop(topRowHeight);
    DrawTopRow(display,
               isVertical,
               selectedItemIdx,
               numItems,
               itemText,
               topRowRect,
               true);

    DrawValueText(display, isVertical, "...", remainingBounds, false);
}

void FullScreenItemMenu::DrawCloseMenuItem(OneBitGraphicsDisplay& display,
                                           bool                   isVertical,
                                           uint16_t    selectedItemIdx,
                                           uint16_t    numItems,
                                           const char* itemText) const
{
    auto       remainingBounds = display.GetBounds();
    const auto topRowHeight    = GetTopRowHeight(remainingBounds.GetHeight());
    const auto topRowRect      = remainingBounds.RemoveFromTop(topRowHeight);
    DrawTopRow(display,
               isVertical,
               selectedItemIdx,
               numItems,
               itemText,
               topRowRect,
               true);

    DrawValueText(display, isVertical, "...", remainingBounds, false);
}


void FullScreenItemMenu::DrawTopRow(OneBitGraphicsDisplay& display,
                                    bool                   isVertical,
                                    int                    currentIndex,
                                    int                    numItemsTotal,
                                    const char*            text,
                                    Rectangle              rect,
                                    bool                   isSelected) const
{
    const bool hasPrev = currentIndex > 0;
    const bool hasNext = currentIndex < numItemsTotal - 1;
    // draw the arrows
    if(isSelected)
    {
        if(!isVertical)
            rect = DrawLRArrowsAndGetRemRect(display, rect, hasPrev, hasNext);
        else
            rect = DrawUDArrowsAndGetRemRect(display, rect, hasPrev, hasNext);
    }
    const auto font = GetValueFont(text, rect);
    display.WriteStringAligned(text, font, rect, Alignment::centered, true);
}

Rectangle
FullScreenItemMenu::DrawUDArrowsAndGetRemRect(OneBitGraphicsDisplay& display,
                                              Rectangle              topRowRect,
                                              bool upAvailable,
                                              bool downAvailable) const
{
    auto upArrowRect = topRowRect.RemoveFromLeft(9).WithSizeKeepingCenter(9, 5);
    auto downArrowRect
        = topRowRect.RemoveFromRight(9).WithSizeKeepingCenter(9, 5);

    if(upAvailable)
    {
        for(int16_t y = upArrowRect.GetBottom() - 1; y >= upArrowRect.GetY();
            y--)
        {
            display.DrawLine(
                upArrowRect.GetX(), y, upArrowRect.GetRight(), y, true);

            upArrowRect = upArrowRect.Reduced(1, 0);
            if(upArrowRect.IsEmpty())
                break;
        }
    }
    if(downAvailable)
    {
        for(int16_t y = downArrowRect.GetY(); y < upArrowRect.GetBottom(); y++)
        {
            display.DrawLine(
                downArrowRect.GetX(), y, downArrowRect.GetRight(), y, true);

            downArrowRect = downArrowRect.Reduced(1, 0);
            if(downArrowRect.IsEmpty())
                break;
        }
    }

    return topRowRect;
}

Rectangle
FullScreenItemMenu::DrawLRArrowsAndGetRemRect(OneBitGraphicsDisplay& display,
                                              Rectangle              topRowRect,
                                              bool leftAvailable,
                                              bool rightAvailable) const
{
    auto leftArrowRect
        = topRowRect.RemoveFromLeft(9).WithSizeKeepingCenter(5, 9).Translated(
            0, -1);
    auto rightArrowRect
        = topRowRect.RemoveFromRight(9).WithSizeKeepingCenter(5, 9).Translated(
            0, -1);

    if(leftAvailable)
    {
        for(int16_t x = leftArrowRect.GetRight() - 1; x >= leftArrowRect.GetX();
            x--)
        {
            display.DrawLine(
                x, leftArrowRect.GetY(), x, leftArrowRect.GetBottom(), true);

            leftArrowRect = leftArrowRect.Reduced(0, 1);
            if(leftArrowRect.IsEmpty())
                break;
        }
    }
    if(rightAvailable)
    {
        for(int16_t x = rightArrowRect.GetX(); x < rightArrowRect.GetRight();
            x++)
        {
            display.DrawLine(
                x, rightArrowRect.GetY(), x, rightArrowRect.GetBottom(), true);

            rightArrowRect = rightArrowRect.Reduced(0, 1);
            if(rightArrowRect.IsEmpty())
                break;
        }
    }

    return topRowRect;
}

void FullScreenItemMenu::DrawValueText(OneBitGraphicsDisplay& display,
                                       bool                   isVertical,
                                       const char*            text,
                                       Rectangle              rect,
                                       bool isBeingEdited) const
{
    if(isBeingEdited)
    {
        if(!isVertical)
            rect = DrawLRArrowsAndGetRemRect(display, rect, true, true);
        else
            rect = DrawUDArrowsAndGetRemRect(display, rect, true, true);
    }

    const auto font = GetValueFont(text, rect);
    display.WriteStringAligned(text, font, rect, Alignment::centered, true);
}


FontDef FullScreenItemMenu::GetValueFont(const char*      textToDraw,
                                         const Rectangle& availableSpace) const
{
    (void)(textToDraw); // ignore unused variable warning
    if(availableSpace.GetHeight() < 10)
        return Font_6x8;
    else if(availableSpace.GetHeight() < 18)
        return Font_7x10;
    else
        return Font_11x18;
}

FontDef FullScreenItemMenu::GetNameFont(const char*      textToDraw,
                                        const Rectangle& availableSpace) const
{
    (void)(textToDraw); // ignore unused variable warning
    if(availableSpace.GetHeight() < 10)
        return Font_6x8;
    else if(availableSpace.GetHeight() < 18)
        return Font_7x10;
    else
        return Font_11x18;
}

} // namespace daisy