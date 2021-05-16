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
    const auto& laf  = GetOneBitGraphicsLookAndFeel();
    bool isVertical  = orientation_ == Orientation::upDownSelectLeftRightModify;
    switch(type)
    {
        case ItemType::callbackFunctionItem:
            laf.FSIM_DrawTextItem(
                display, isVertical, selectedItemIdx_, numItems_, item.text);
            break;
        case ItemType::checkboxItem:
            laf.FSIM_DrawCheckboxItem(display,
                                      isVertical,
                                      selectedItemIdx_,
                                      numItems_,
                                      item.text,
                                      *item.asCheckboxItem.valueToModify);
            break;
        case ItemType::valueItem:
            laf.FSIM_DrawValueItem(display,
                                   isVertical,
                                   selectedItemIdx_,
                                   numItems_,
                                   item.text,
                                   *item.asMappedValueItem.valueToModify,
                                   isEditing_);
            break;
        case ItemType::openUiPageItem:
            laf.FSIM_DrawOpenUiPageItem(
                display, isVertical, selectedItemIdx_, numItems_, item.text);
            break;
        case ItemType::closeMenuItem:
            laf.FSIM_DrawCloseMenuItem(
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

} // namespace daisy