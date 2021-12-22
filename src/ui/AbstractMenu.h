#pragma once

#include "hid/disp/display.h"
#include "util/MappedValue.h"
#include "UI.h"

namespace daisy
{
/** @brief Base class for complex menus.
 *  @author jelliesen
 *  @ingroup ui
 * 
 * This is the base class for any form of UiPage that displays a menu with multiple items.
 * It handles all the logic behind a menu (selecting items, editing items, etc.) but doesn't
 * implement any form of drawing. Implement your own drawing routines by overriding
 * UiPage::Draw() or use `FullScreenItemMenu`
 * 
 * Vaious types of items can be added to the menu, e.g. 
 * - Checkbox items to toggle something on/off,
 * - Generic action items that call a function when activated,
 * - Value editing items for editing int/float/enum values,
 * - Close items that close the menu when activated (useful when no cancel button is available),
 * - Custom items that do whatever you want them to do, by providing a CustomItem object that 
 *   handles the item-specific functionality.
 * 
 * The Abstract Menu can work with a wide variety of physical controls, here are a couple 
 * of combinations that are possible:
 * - 3 buttons: Left/Right to select and edit items, Ok to activate or enter/leave editing mode
 * - 5 buttons: Up/Down to select, Ok to activate/enter/leave, Cancel to close menu, function 
 *              button to use coarse step size when editing
 * - 6 buttons: Left/Right to select, Up/Down to quickly edit selected items, Ok/Cancel to enter/leave
 * - 1 encoder with pushbutton
 * - 1 encoder for selecting items, another one for editing their values
 * - 1 encoder for selecting items and a value slider potentiometer for editing
 * - ... any other combination of the above
 * 
 * These are the controls that the AbstractMenu will react to and their associated function:
 * - Left/Right buttons: Select items when `Orientation::leftRightSelectUpDownModify`,
 *                       directly edit value of the selected item when 
 *                       `Orientation::upDownSelectLeftRightModify`.
 * - Up/Down buttons: Select items when `Orientation::upDownSelectLeftRightModify`,
 *                    directly edit value of the selected item when 
 *                    `Orientation::leftRightSelectUpDownModify`.
 * - Ok button: Activate the action of the selected item (if it has an action, e.g. 
 *              `ItemType::closeMenuItem`) otherwise enter/leave editing mode where the 
 *              arrow buttons used for selection will now edit the value instead (only 
 *              possible if `allowEntering` is set to `true`).
 * - Cancel button: Closes the menu page or leaves editing mode
 * - Menu encoder: Selects items; edits item value when in editing mode
 * - Value Encoder: Edits value of selected item
 * - Value potentiometer/slider: Edits value of selected item
 * - Function button: Uses an alternate step size when modifying the value with encoders
 *                    or buttons while pressed
 */
class AbstractMenu : public UiPage
{
  public:
    /** Controls which buttons are used to navigate back and forth between the menu 
     *  items (selection buttons) and which buttons can be used to modify their value 
     *  directly without pressing the enter button first (modify buttons; these don't
     *  have to be available).
     *  @see AbstractMenuPage
     */
    enum class Orientation
    {
        /** left/right buttons => selection buttons, up/down => value buttons */
        leftRightSelectUpDownModify,
        /** up/down buttons => selection buttons, left/right => value buttons */
        upDownSelectLeftRightModify,
    };

    /** The types of entries that can be added to the menu. */
    enum class ItemType
    {
        /** Displays a text and calls a callback function when activated with the enter button */
        callbackFunctionItem,
        /** Displays a name and a checkbox. When selected, the modify keys will allow
         *  to change the value directly. Pressing the enter button toggles the value. */
        checkboxItem,
        /** Displays a name and a value (with unit) from a MappedValue. When selected, the modify keys will allow
         *  to change the value directly. Pressing the enter button allows to change the value with
         *  the selection buttons as well. */
        valueItem,
        /** Displays a name and opens another UiPage when selected. */
        openUiPageItem,
        /** Displays a text and closes the menu page when selected. This is useful when no cancel 
         *  button is available to close a menu and return to the page below. */
        closeMenuItem,
        /** A custom item. @see CustomItem */
        customItem,
    };

    /** Base class for a custom menu item */
    class CustomItem
    {
      public:
        virtual ~CustomItem() {}

        /** Draws the item to a OneBitGraphicsDisplay.
         * @param display           The display to draw to
         * @param currentIndex      The index in the menu
         * @param numItemsTotal     The total number of items in the menu
         * @param boundsToDrawIn    The Rectangle to draw the item into
         * @param isEditing         True if the enter button was pressed and the value is being edited directly.
         */
        virtual void Draw(OneBitGraphicsDisplay& display,
                          int                    currentIndex,
                          int                    numItemsTotal,
                          Rectangle              boundsToDrawIn,
                          bool                   isEditing)
            = 0;

        /** Returns true, if this item can be entered for direct editing of the value. */
        virtual bool CanBeEnteredForEditing() const { return false; }

        /** Called when the encoder of the buttons are used to modify the value. */
        virtual void ModifyValue(int16_t  increments,
                                 uint16_t stepsPerRevolution,
                                 bool     isFunctionButtonPressed)
        {
            (void)(increments);              // silence unused variable warnings
            (void)(stepsPerRevolution);      // silence unused variable warnings
            (void)(isFunctionButtonPressed); // silence unused variable warnings
        };

        /** Called when the value slider is used to modify the value. */
        virtual void ModifyValue(float valueSliderPosition0To1,
                                 bool  isFunctionButtonPressed)
        {
            (void)(valueSliderPosition0To1); // silence unused variable warnings
            (void)(isFunctionButtonPressed); // silence unused variable warnings
        };

        /** Called when the okay button is pressed (and CanBeEnteredForEditing() returns false). */
        virtual void OnOkayButton(){};
    };

    struct ItemConfig
    {
        /** The type of item */
        ItemType type = ItemType::closeMenuItem;
        /** The name/text to display */
        const char* text = "";

        /** additional properties that depend on the value of `type` */
        union
        {
            /** Properties for type == ItemType::callbackFunctionItem */
            struct
            {
                void (*callbackFunction)(void* context);
                void* context;
            } asCallbackFunctionItem;

            /** Properties for type == ItemType::checkboxItem */
            struct
            {
                /** The variable to modify. */
                bool* valueToModify;
            } asCheckboxItem;

            /** Properties for type == ItemType::valueItem */
            struct
            {
                /** The variable to modify. */
                MappedValue* valueToModify;
            } asMappedValueItem;

            /** Properties for type == ItemType::openUiPageItem */
            struct
            {
                /** The UiPage to open when the okay button is pressed.
                 *  The object must stay alive longer than the MenuPage, 
                 *  e.g. as a global variable. */
                UiPage* pageToOpen;
            } asOpenUiPageItem;

            /** Properties for type == ItemType::customItem */
            struct
            {
                /** The CustomItem to display. The object provided here must 
                 *  stay alive longer than the MenuPage, e.g. as a global variable. */
                CustomItem* itemObject;
            } asCustomItem;
        };
    };

    AbstractMenu() = default;
    virtual ~AbstractMenu() override {}

    uint16_t          GetNumItems() const { return numItems_; }
    const ItemConfig& GetItem(uint16_t itemIdx) const
    {
        return items_[itemIdx];
    }
    void    SelectItem(uint16_t itemIdx);
    int16_t GetSelectedItemIdx() const { return selectedItemIdx_; }

    // inherited from UiPage
    bool OnOkayButton(uint8_t numberOfPresses, bool isRetriggering) override;
    bool OnCancelButton(uint8_t numberOfPresses, bool isRetriggering) override;
    bool OnArrowButton(ArrowButtonType arrowType,
                       uint8_t         numberOfPresses,
                       bool            isRetriggering) override;
    bool OnFunctionButton(uint8_t numberOfPresses,
                          bool    isRetriggering) override;
    bool OnMenuEncoderTurned(int16_t  turns,
                             uint16_t stepsPerRevolution) override;
    bool OnValueEncoderTurned(int16_t  turns,
                              uint16_t stepsPerRevolution) override;
    bool OnValuePotMoved(float newPosition) override;
    void OnShow() override;

  protected:
    /** Call this from your child class to initialize the menu. It's okay to
     *  re-initialize an AbstractMene multiple times, even while it's displayed
     *  on the UI.
     * @param items             An array of ItemConfig that determine which items are 
     *                          available in the menu.
     * @param numItems          The number of items in the `items` array.
     * @param orientation       Controls which pair of arrow buttons are used for 
     *                          selection / editing
     * @param allowEntering     Globally controls if the Ok button can enter items 
     *                          for editing. If you have a physical controls that can edit 
     *                          selected items directly (value slider, a second arrow button 
     *                          pair, value encoder) you can set this to false, otherwise you
     *                          set it to true so that the controls used for selecting items 
     *                          can now also be used to edit the values.
     */
    void Init(const ItemConfig* items,
              uint16_t          numItems,
              Orientation       orientation,
              bool              allowEntering);

    /** Returns the state of the function button. */
    bool IsFunctionButtonDown() const { return isFuncButtonDown_; }

    /** The orientation of the menu. This is used to determine 
     *  which function the arrow keys will be assigned to. */
    Orientation orientation_ = Orientation::upDownSelectLeftRightModify;
    /** A list of items to include in the menu. */
    const ItemConfig* items_ = nullptr;
    /** The number of items in `items_` */
    uint16_t numItems_ = 0;
    /** The currently selected item index */
    int16_t selectedItemIdx_ = -1;
    /** If true, the menu allows "entering" an item to modify 
     *  its value with the encoder / selection buttons. */
    bool allowEntering_ = true;
    /** If true, the currently selected item index is 
     *  "entered" so that it can be edited with the encoder/
     *  selection buttons. */
    bool isEditing_ = false;

  private:
    AbstractMenu(const AbstractMenu& other) = delete;
    AbstractMenu& operator=(const AbstractMenu& other) = delete;

    bool CanItemBeEnteredForEditing(uint16_t itemIdx);
    void ModifyItemValue(uint16_t itemIdx,
                         int16_t  increments,
                         uint16_t stepsPerRevolution,
                         bool     isFunctionButtonPressed);
    void ModifyItemValue(uint16_t itemIdx,
                         float    valueSliderPosition0To1,
                         bool     isFunctionButtonPressed);
    void TriggerItemAction(uint16_t itemIdx);

    bool isFuncButtonDown_ = false;
};


} // namespace daisy