#include "AbstractMenu.h"
#include "util/FixedCapStr.h"

namespace daisy
{
void AbstractMenu::SelectItem(uint16_t itemIdx)
{
    if(itemIdx >= numItems_)
        return;
    selectedItemIdx_ = itemIdx;
    isEditing_       = false;
}

// inherited from UiPage
bool AbstractMenu::OnOkayButton(uint8_t numberOfPresses, bool isRetriggering)
{
    (void)(isRetriggering); // silence unused variable warning

    if(numberOfPresses < 1)
        return true;

    if(allowEntering_ && CanItemBeEnteredForEditing(selectedItemIdx_))
    {
        isEditing_ = !isEditing_;
    }
    else
    {
        isEditing_ = false;
        TriggerItemAction(selectedItemIdx_);
    }
    return true;
}

bool AbstractMenu::OnCancelButton(uint8_t numberOfPresses, bool isRetriggering)
{
    (void)(isRetriggering); // silence unused variable warning

    if(numberOfPresses < 1)
        return true;

    if(isEditing_)
        isEditing_ = false;
    else
        Close();
    return true;
}

bool AbstractMenu::OnArrowButton(ArrowButtonType arrowType,
                                 uint8_t         numberOfPresses,
                                 bool            isRetriggering)
{
    (void)(isRetriggering); // silence unused variable warning

    if(numberOfPresses < 1)
        return true;

    if(orientation_ == Orientation::leftRightSelectUpDownModify)
    {
        if(arrowType == ArrowButtonType::down)
            ModifyItemValue(selectedItemIdx_, -1, 0, isFuncButtonDown_);
        else if(arrowType == ArrowButtonType::up)
            ModifyItemValue(selectedItemIdx_, 1, 0, isFuncButtonDown_);
        else if(isEditing_)
        {
            if(arrowType == ArrowButtonType::left)
                ModifyItemValue(selectedItemIdx_, -1, 0, isFuncButtonDown_);
            else if(arrowType == ArrowButtonType::right)
                ModifyItemValue(selectedItemIdx_, 1, 0, isFuncButtonDown_);
        }
        else
        {
            if((arrowType == ArrowButtonType::left) && (selectedItemIdx_ > 0))
                selectedItemIdx_--;
            else if((arrowType == ArrowButtonType::right)
                    && (selectedItemIdx_ < numItems_ - 1))
                selectedItemIdx_++;
        }
    }
    else
    // orientation_ == Orientation::upDownSelectLeftRightModify
    {
        if(arrowType == ArrowButtonType::left)
            ModifyItemValue(selectedItemIdx_, -1, 0, isFuncButtonDown_);
        else if(arrowType == ArrowButtonType::right)
            ModifyItemValue(selectedItemIdx_, 1, 0, isFuncButtonDown_);
        else if(isEditing_)
        {
            if(arrowType == ArrowButtonType::down)
                ModifyItemValue(selectedItemIdx_, -1, 0, isFuncButtonDown_);
            else if(arrowType == ArrowButtonType::up)
                ModifyItemValue(selectedItemIdx_, 1, 0, isFuncButtonDown_);
        }
        else
        {
            if((arrowType == ArrowButtonType::up) && (selectedItemIdx_ > 0))
                selectedItemIdx_--;
            else if((arrowType == ArrowButtonType::down)
                    && (selectedItemIdx_ < numItems_ - 1))
                selectedItemIdx_++;
        }
    }
    return true;
}

bool AbstractMenu::OnFunctionButton(uint8_t numberOfPresses,
                                    bool    isRetriggering)
{
    (void)(isRetriggering); // silence unused variable warning

    isFuncButtonDown_ = numberOfPresses > 0;
    return true;
}

bool AbstractMenu::OnMenuEncoderTurned(int16_t  turns,
                                       uint16_t stepsPerRevolution)
{
    // edit value
    if(isEditing_)
        ModifyItemValue(
            selectedItemIdx_, turns, stepsPerRevolution, isFuncButtonDown_);
    else
    // scroll through menu
    {
        int16_t result = selectedItemIdx_ + turns;
        selectedItemIdx_
            = (result < 0) ? 0
                           : ((result >= numItems_) ? numItems_ - 1 : result);
    }
    return true;
}

bool AbstractMenu::OnValueEncoderTurned(int16_t  turns,
                                        uint16_t stepsPerRevolution)
{
    ModifyItemValue(
        selectedItemIdx_, turns, stepsPerRevolution, isFuncButtonDown_);
    return true;
}

bool AbstractMenu::OnValuePotMoved(float newPosition)
{
    ModifyItemValue(selectedItemIdx_, newPosition, isFuncButtonDown_);
    return true;
}

void AbstractMenu::OnShow()
{
    isEditing_        = false;
    isFuncButtonDown_ = false;
    selectedItemIdx_  = 0;
}

void AbstractMenu::Init(const ItemConfig* items,
                        uint16_t          numItems,
                        Orientation       orientation,
                        bool              allowEntering)
{
    orientation_   = orientation;
    items_         = items;
    numItems_      = numItems;
    allowEntering_ = allowEntering;

    selectedItemIdx_  = 0;
    isEditing_        = false;
    isFuncButtonDown_ = false;
}

bool AbstractMenu::CanItemBeEnteredForEditing(uint16_t itemIdx)
{
    if(itemIdx >= numItems_)
        return false;

    const auto& item = items_[itemIdx];
    const auto  type = item.type;
    switch(type)
    {
        case ItemType::callbackFunctionItem:
        case ItemType::checkboxItem:
        case ItemType::closeMenuItem:
        case ItemType::openUiPageItem: return false;
        case ItemType::valueItem: return true;
        case ItemType::customItem:
            return item.asCustomItem.itemObject->CanBeEnteredForEditing();
        default: return false;
    }
}
void AbstractMenu::ModifyItemValue(uint16_t itemIdx,
                                   int16_t  increments,
                                   uint16_t stepsPerRevolution,
                                   bool     isFunctionButtonPressed)
{
    if(itemIdx >= numItems_)
        return;

    const auto& item = items_[itemIdx];
    const auto  type = item.type;
    switch(type)
    {
        case ItemType::callbackFunctionItem: break;
        case ItemType::checkboxItem:
            *item.asCheckboxItem.valueToModify = (increments > 0);
            break;
        case ItemType::closeMenuItem: break;
        case ItemType::openUiPageItem: break;
        case ItemType::valueItem:
            item.asMappedValueItem.valueToModify->Step(increments,
                                                       isFunctionButtonPressed);
            break;
        case ItemType::customItem:
            item.asCustomItem.itemObject->ModifyValue(
                increments, stepsPerRevolution, isFunctionButtonPressed);
            break;
    }
}
void AbstractMenu::ModifyItemValue(uint16_t itemIdx,
                                   float    valueSliderPosition0To1,
                                   bool     isFunctionButtonPressed)
{
    if(itemIdx >= numItems_)
        return;

    const auto& item = items_[itemIdx];
    const auto  type = item.type;
    switch(type)
    {
        case ItemType::callbackFunctionItem: break;
        case ItemType::checkboxItem:
            *item.asCheckboxItem.valueToModify
                = (valueSliderPosition0To1 >= 0.5f);
            break;
        case ItemType::closeMenuItem: break;
        case ItemType::openUiPageItem: break;
        case ItemType::valueItem:
            item.asMappedValueItem.valueToModify->SetFrom0to1(
                valueSliderPosition0To1);
            break;
        case ItemType::customItem:
            item.asCustomItem.itemObject->ModifyValue(valueSliderPosition0To1,
                                                      isFunctionButtonPressed);
            break;
    }
}

void AbstractMenu::TriggerItemAction(uint16_t itemIdx)
{
    if(itemIdx >= numItems_)
        return;

    const auto& item = items_[itemIdx];
    const auto  type = item.type;
    switch(type)
    {
        case ItemType::callbackFunctionItem:
            item.asCallbackFunctionItem.callbackFunction(
                item.asCallbackFunctionItem.context);
            break;
        case ItemType::checkboxItem:
            *item.asCheckboxItem.valueToModify
                = !*item.asCheckboxItem.valueToModify;
            break;
        case ItemType::closeMenuItem: Close(); break;
        case ItemType::openUiPageItem:
            if(auto* ui = GetParentUI())
                ui->OpenPage(*item.asOpenUiPageItem.pageToOpen);
            break;
        case ItemType::valueItem:
            // no "on enter" action
            break;
        case ItemType::customItem:
            item.asCustomItem.itemObject->OnOkayButton();
            break;
    }
}

} // namespace daisy