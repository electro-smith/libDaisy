#include "FullScreenItemMenuLookAndFeelMethods.h"

namespace daisy
{
int FullScreenItemMenuLookAndFeelMethods::FSIM_GetTopRowHeight(
    int displayHeight) const
{
    return displayHeight / 2;
}

void FullScreenItemMenuLookAndFeelMethods::FSIM_DrawTextItem(
    OneBitGraphicsDisplay& display,
    bool                   isVertical,
    uint16_t               selectedItemIdx,
    uint16_t               numItems,
    const char*            itemText) const
{
    auto       remainingBounds = display.GetBounds();
    const auto topRowHeight = FSIM_GetTopRowHeight(remainingBounds.GetHeight());
    const auto topRowRect   = remainingBounds.RemoveFromTop(topRowHeight);
    FSIM_DrawTopRow(display,
                    isVertical,
                    selectedItemIdx,
                    numItems,
                    itemText,
                    topRowRect,
                    true);
}

void FullScreenItemMenuLookAndFeelMethods::FSIM_DrawCheckboxItem(
    OneBitGraphicsDisplay& display,
    bool                   isVertical,
    uint16_t               selectedItemIdx,
    uint16_t               numItems,
    const char*            itemText,
    const bool&            isCheckboxTicked) const
{
    auto       remainingBounds = display.GetBounds();
    const auto topRowHeight = FSIM_GetTopRowHeight(remainingBounds.GetHeight());
    const auto topRowRect   = remainingBounds.RemoveFromTop(topRowHeight);
    FSIM_DrawTopRow(display,
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

void FullScreenItemMenuLookAndFeelMethods::FSIM_DrawValueItem(
    OneBitGraphicsDisplay& display,
    bool                   isVertical,
    uint16_t               selectedItemIdx,
    uint16_t               numItems,
    const char*            itemText,
    const MappedValue&     value,
    bool                   isEditing) const
{
    auto       remainingBounds = display.GetBounds();
    const auto topRowHeight = FSIM_GetTopRowHeight(remainingBounds.GetHeight());
    const auto topRowRect   = remainingBounds.RemoveFromTop(topRowHeight);
    FSIM_DrawTopRow(display,
                    isVertical,
                    selectedItemIdx,
                    numItems,
                    itemText,
                    topRowRect,
                    !isEditing);

    // draw the value
    FixedCapStr<20> valueStr;
    value.AppentToString(valueStr);
    FSIM_DrawValueText(
        display, isVertical, valueStr, remainingBounds, isEditing);
}

void FullScreenItemMenuLookAndFeelMethods::FSIM_DrawOpenUiPageItem(
    OneBitGraphicsDisplay& display,
    bool                   isVertical,
    uint16_t               selectedItemIdx,
    uint16_t               numItems,
    const char*            itemText) const
{
    auto       remainingBounds = display.GetBounds();
    const auto topRowHeight = FSIM_GetTopRowHeight(remainingBounds.GetHeight());
    const auto topRowRect   = remainingBounds.RemoveFromTop(topRowHeight);
    FSIM_DrawTopRow(display,
                    isVertical,
                    selectedItemIdx,
                    numItems,
                    itemText,
                    topRowRect,
                    true);

    FSIM_DrawValueText(display, isVertical, "...", remainingBounds, false);
}

void FullScreenItemMenuLookAndFeelMethods::FSIM_DrawCloseMenuItem(
    OneBitGraphicsDisplay& display,
    bool                   isVertical,
    uint16_t               selectedItemIdx,
    uint16_t               numItems,
    const char*            itemText) const
{
    auto       remainingBounds = display.GetBounds();
    const auto topRowHeight = FSIM_GetTopRowHeight(remainingBounds.GetHeight());
    const auto topRowRect   = remainingBounds.RemoveFromTop(topRowHeight);
    FSIM_DrawTopRow(display,
                    isVertical,
                    selectedItemIdx,
                    numItems,
                    itemText,
                    topRowRect,
                    true);

    FSIM_DrawValueText(display, isVertical, "...", remainingBounds, false);
}


void FullScreenItemMenuLookAndFeelMethods::FSIM_DrawTopRow(
    OneBitGraphicsDisplay& display,
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
            rect = FSIM_DrawLRArrowsAndGetRemRect(
                display, rect, hasPrev, hasNext);
        else
            rect = FSIM_DrawUDArrowsAndGetRemRect(
                display, rect, hasPrev, hasNext);
    }
    const auto font = FSIM_GetValueFont(text, rect);
    display.WriteStringAligned(text, font, rect, Alignment::centered, true);
}

Rectangle FullScreenItemMenuLookAndFeelMethods::FSIM_DrawUDArrowsAndGetRemRect(
    OneBitGraphicsDisplay& display,
    Rectangle              topRowRect,
    bool                   upAvailable,
    bool                   downAvailable) const
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

Rectangle FullScreenItemMenuLookAndFeelMethods::FSIM_DrawLRArrowsAndGetRemRect(
    OneBitGraphicsDisplay& display,
    Rectangle              topRowRect,
    bool                   leftAvailable,
    bool                   rightAvailable) const
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

void FullScreenItemMenuLookAndFeelMethods::FSIM_DrawValueText(
    OneBitGraphicsDisplay& display,
    bool                   isVertical,
    const char*            text,
    Rectangle              rect,
    bool                   isBeingEdited) const
{
    if(isBeingEdited)
    {
        if(!isVertical)
            rect = FSIM_DrawLRArrowsAndGetRemRect(display, rect, true, true);
        else
            rect = FSIM_DrawUDArrowsAndGetRemRect(display, rect, true, true);
    }

    const auto font = FSIM_GetValueFont(text, rect);
    display.WriteStringAligned(text, font, rect, Alignment::centered, true);
}


FontDef FullScreenItemMenuLookAndFeelMethods::FSIM_GetValueFont(
    const char*      textToDraw,
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

FontDef FullScreenItemMenuLookAndFeelMethods::FSIM_GetNameFont(
    const char*      textToDraw,
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
