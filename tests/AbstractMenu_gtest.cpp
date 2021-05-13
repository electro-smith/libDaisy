#include <gtest/gtest.h>
#include "ui/AbstractMenu.h"
#include "util/MappedValue.h"
#include <vector>

using namespace daisy;

/** Exposes some of the internals of AbstractMenu to make them 
 *  accessible from unit tests.
 */
class ExposedAbstractMenu : public AbstractMenu
{
  public:
    void AddCloseItemsAndInit(Orientation orientation
                              = Orientation::leftRightSelectUpDownModify,
                              int  numItemsToAdd = 2,
                              bool allowEntering = true)
    {
        // add a bunch of "close" items.
        itemConfigs_.resize(numItemsToAdd);
        for(auto&& item : itemConfigs_)
        {
            item.type = AbstractMenu::ItemType::closeMenuItem;
            item.text = "close";
        }
        AbstractMenu::Init(
            itemConfigs_.data(), numItemsToAdd, orientation, allowEntering);
    }

    void AddCallbackItemsAndInit(Orientation orientation
                                 = Orientation::leftRightSelectUpDownModify,
                                 int  numItemsToAdd = 2,
                                 bool allowEntering = true)
    {
        // add a bunch of "callback" items.
        itemConfigs_.resize(2);
        for(auto&& item : itemConfigs_)
        {
            item.type = AbstractMenu::ItemType::callbackFunctionItem;
            item.text = "callback";
            item.asCallbackFunctionItem.callbackFunction
                = &callbackItemCallbackFunction;
            // we use the context to pass a reference to this object so that the
            // callback comes back to this object.
            item.asCallbackFunctionItem.context = this;
        }
        AbstractMenu::Init(
            itemConfigs_.data(), numItemsToAdd, orientation, allowEntering);
    }

    void AddCheckboxItemsAndInit(Orientation orientation
                                 = Orientation::leftRightSelectUpDownModify,
                                 int  numItemsToAdd = 2,
                                 bool allowEntering = true)
    {
        // add a bunch of "checkbox" items.
        itemConfigs_.resize(numItemsToAdd);
        for(auto&& item : itemConfigs_)
        {
            item.type = AbstractMenu::ItemType::checkboxItem;
            item.text = "checkbox";
            item.asCheckboxItem.valueToModify = &checkboxItemValue_;
        }
        AbstractMenu::Init(
            itemConfigs_.data(), numItemsToAdd, orientation, allowEntering);
    }

    void AddValueItemsAndInit(Orientation orientation
                              = Orientation::leftRightSelectUpDownModify,
                              int  numItemsToAdd = 2,
                              bool allowEntering = true)
    {
        // add a bunch of "MappedValue" items.
        itemConfigs_.resize(numItemsToAdd);
        for(auto&& item : itemConfigs_)
        {
            item.type = AbstractMenu::ItemType::valueItem;
            item.text = "value";
            item.asMappedValueItem.valueToModify = &mappedIntValue_;
        }
        AbstractMenu::Init(
            itemConfigs_.data(), numItemsToAdd, orientation, allowEntering);
    }

    void AddOpenUiPageItemAndInit(UiPage& pageToOpen)
    {
        itemConfigs_.resize(1);
        itemConfigs_[0].type = AbstractMenu::ItemType::openUiPageItem;
        itemConfigs_[0].text = "Open Page";
        itemConfigs_[0].asOpenUiPageItem.pageToOpen = &pageToOpen;

        AbstractMenu::Init(itemConfigs_.data(),
                           1,
                           Orientation::leftRightSelectUpDownModify,
                           true);
    }

    void AddCustomItemAndInit(Orientation orientation
                              = Orientation::leftRightSelectUpDownModify)
    {
        itemConfigs_.resize(1);
        itemConfigs_[0].type = AbstractMenu::ItemType::customItem;
        itemConfigs_[0].text = "";
        itemConfigs_[0].asCustomItem.itemObject = &customItem_;

        AbstractMenu::Init(itemConfigs_.data(), 1, orientation, true);
    }

    AbstractMenu::Orientation GetOrientation() { return orientation_; }
    bool                      AllowsEntering() { return allowEntering_; }
    bool                      IsEnteredForEditing() { return isEditing_; }
    bool IsFunctionButtonDown() { return AbstractMenu::IsFunctionButtonDown(); }

    void Draw(const UiCanvasDescriptor& /* canvas */) override {}

    static void callbackItemCallbackFunction(void* context)
    {
        auto& testObject                   = *((ExposedAbstractMenu*)(context));
        testObject.callbackFunctionCalled_ = true;
    }

    /** A MappedIntValue that exposes calls to its functions so 
     *  that unit tests can observer when the Abstractmenu calls them.
    */
    class ExposedMappedIntValue : public MappedIntValue
    {
      public:
        ExposedMappedIntValue() : MappedIntValue(0, 10, 5, 2, 1) {}

        void SetFrom0to1(float normalizedValue0to1) override
        {
            setFrom0to1Called_          = true;
            valuePassedIntoSetFrom0to1_ = normalizedValue0to1;
        }
        void Step(int16_t numStepsUp, bool useCoarseStepSize) override
        {
            stepCalled_                      = true;
            numStepsUpPassedIntoStep_        = numStepsUp;
            useCoarseStepSizePassedIntoStep_ = useCoarseStepSize;
        }

        bool  setFrom0to1Called_          = false;
        float valuePassedIntoSetFrom0to1_ = 0.0f;

        bool    stepCalled_                      = false;
        int16_t numStepsUpPassedIntoStep_        = 0;
        bool    useCoarseStepSizePassedIntoStep_ = false;
    };

    class UnitTestCustomItem : public AbstractMenu::CustomItem
    {
      public:
        void Draw(OneBitGraphicsDisplay&, int, int, Rectangle, bool) override {}
        bool canBeEnteredForEditing_ = false;
        bool CanBeEnteredForEditing() const override
        {
            return canBeEnteredForEditing_;
        }

        bool     modifySteppedCalled_                       = false;
        int16_t  incrementsPassedIntoModifyStepped_         = 0;
        uint16_t stepsPerRevolutionPassedIntoModifyStepped_ = 0;
        bool     isFuncButtonDownPassedIntoModifyStepped_   = false;
        void     ModifyValue(int16_t  increments,
                             uint16_t stepsPerRevolution,
                             bool     isFunctionButtonPressed) override
        {
            modifySteppedCalled_                       = true;
            incrementsPassedIntoModifyStepped_         = increments;
            stepsPerRevolutionPassedIntoModifyStepped_ = stepsPerRevolution;
            isFuncButtonDownPassedIntoModifyStepped_ = isFunctionButtonPressed;
        };

        bool  modifyContinuousCalled_                     = false;
        float valuePassedIntoModifyContinuous             = 0.0f;
        bool  isFuncButtonDownPassedIntoModifyContinuous_ = false;
        void  ModifyValue(float valueSliderPosition0To1,
                          bool  isFunctionButtonPressed) override
        {
            modifyContinuousCalled_         = true;
            valuePassedIntoModifyContinuous = valueSliderPosition0To1;
            isFuncButtonDownPassedIntoModifyContinuous_
                = isFunctionButtonPressed;
        };

        bool onOkayButtonCalled_ = false;
        void OnOkayButton() override { onOkayButtonCalled_ = true; }
    };

    std::vector<AbstractMenu::ItemConfig> itemConfigs_;
    bool                                  callbackFunctionCalled_ = false;
    bool                                  checkboxItemValue_      = false;
    ExposedMappedIntValue                 mappedIntValue_;
    UnitTestCustomItem                    customItem_;
};

TEST(ui_AbstractMenu, a_stateAfterConstruction)
{
    ExposedAbstractMenu menu;

    EXPECT_EQ(menu.GetNumItems(), 0);
    EXPECT_EQ(menu.GetSelectedItemIdx(), -1);
}

TEST(ui_AbstractMenu, b_stateAfterInit)
{
    // initializes the menu with two items,
    // checks the state after initialisation.

    ExposedAbstractMenu menu;

    {
        // initialize the menu with 2 items
        const int kNumItems = 2;
        menu.AddCloseItemsAndInit(
            AbstractMenu::Orientation::leftRightSelectUpDownModify,
            kNumItems,
            true);

        EXPECT_EQ(menu.GetNumItems(), kNumItems);
        EXPECT_EQ(menu.GetSelectedItemIdx(), 0);
        EXPECT_EQ(menu.GetOrientation(),
                  AbstractMenu::Orientation::leftRightSelectUpDownModify);
        EXPECT_EQ(menu.AllowsEntering(), true);
        EXPECT_EQ(menu.IsFunctionButtonDown(), false);
    }

    {
        // initialize the same menu again, this time with 4 items and
        // different setings
        const int kNumItems = 4;
        menu.AddCloseItemsAndInit(
            AbstractMenu::Orientation::upDownSelectLeftRightModify,
            kNumItems,
            false);

        EXPECT_EQ(menu.GetNumItems(), kNumItems);
        EXPECT_EQ(menu.GetSelectedItemIdx(), 0);
        EXPECT_EQ(menu.GetOrientation(),
                  AbstractMenu::Orientation::upDownSelectLeftRightModify);
        EXPECT_EQ(menu.AllowsEntering(), false);
        EXPECT_EQ(menu.IsFunctionButtonDown(), false);
    }
}

TEST(ui_AbstractMenu, c_selectWithButtons)
{
    // initializes the menu with some items,
    // selects them with the arrow buttons

    ExposedAbstractMenu menu;
    {
        // init with left & right as the select buttons
        menu.AddCloseItemsAndInit(
            AbstractMenu::Orientation::leftRightSelectUpDownModify, 4, true);
        EXPECT_EQ(menu.GetSelectedItemIdx(), 0);

        // select an item manually
        menu.SelectItem(2);
        EXPECT_EQ(menu.GetSelectedItemIdx(), 2);

        // press the right button
        menu.OnArrowButton(ArrowButtonType::right, 1, false);
        EXPECT_EQ(menu.GetSelectedItemIdx(), 3);
        // press the right button again
        menu.OnArrowButton(ArrowButtonType::right, 1, false);
        EXPECT_EQ(menu.GetSelectedItemIdx(),
                  3); // we already were at the last item!

        // select an item manually
        menu.SelectItem(1);
        EXPECT_EQ(menu.GetSelectedItemIdx(), 1);

        // press the left button
        menu.OnArrowButton(ArrowButtonType::left, 1, false);
        EXPECT_EQ(menu.GetSelectedItemIdx(), 0);
        // press the left button again
        menu.OnArrowButton(ArrowButtonType::left, 1, false);
        EXPECT_EQ(menu.GetSelectedItemIdx(),
                  0); // we already were at the first item!

        // up and down buttons are not configured to change the selection!
        menu.SelectItem(1);
        menu.OnArrowButton(ArrowButtonType::up, 1, false);
        EXPECT_EQ(menu.GetSelectedItemIdx(), 1);
        menu.OnArrowButton(ArrowButtonType::down, 1, false);
        EXPECT_EQ(menu.GetSelectedItemIdx(), 1);
    }
    {
        // repeat the same test, this time with up & down buttons
        menu.AddCloseItemsAndInit(
            AbstractMenu::Orientation::upDownSelectLeftRightModify, 4, true);
        EXPECT_EQ(menu.GetSelectedItemIdx(), 0);

        // select an item manually
        menu.SelectItem(2);
        EXPECT_EQ(menu.GetSelectedItemIdx(), 2);

        // press the down button
        menu.OnArrowButton(ArrowButtonType::down, 1, false);
        EXPECT_EQ(menu.GetSelectedItemIdx(), 3);
        // press the down button again
        menu.OnArrowButton(ArrowButtonType::down, 1, false);
        EXPECT_EQ(menu.GetSelectedItemIdx(),
                  3); // we already were at the last item!

        // select an item manually
        menu.SelectItem(1);
        EXPECT_EQ(menu.GetSelectedItemIdx(), 1);

        // press the up button
        menu.OnArrowButton(ArrowButtonType::up, 1, false);
        EXPECT_EQ(menu.GetSelectedItemIdx(), 0);
        // press the up button again
        menu.OnArrowButton(ArrowButtonType::up, 1, false);
        EXPECT_EQ(menu.GetSelectedItemIdx(),
                  0); // we already were at the first item!

        // left and right buttons are not configured to change the selection!
        menu.SelectItem(1);
        menu.OnArrowButton(ArrowButtonType::left, 1, false);
        EXPECT_EQ(menu.GetSelectedItemIdx(), 1);
        menu.OnArrowButton(ArrowButtonType::right, 1, false);
        EXPECT_EQ(menu.GetSelectedItemIdx(), 1);
    }
}

TEST(ui_AbstractMenu, d_selectWithEncoder)
{
    // initializes the menu with some items,
    // selects them with the menu encoder

    ExposedAbstractMenu menu;

    // init with left & right as the select buttons
    menu.AddCloseItemsAndInit(
        AbstractMenu::Orientation::leftRightSelectUpDownModify, 4, true);
    EXPECT_EQ(menu.GetSelectedItemIdx(), 0);

    menu.OnMenuEncoderTurned(1, 12);
    EXPECT_EQ(menu.GetSelectedItemIdx(), 1);
    menu.OnMenuEncoderTurned(10, 12);
    EXPECT_EQ(menu.GetSelectedItemIdx(), 3); // clamped to the last item

    menu.OnMenuEncoderTurned(-1, 12);
    EXPECT_EQ(menu.GetSelectedItemIdx(), 2);
    menu.OnMenuEncoderTurned(-10, 12);
    EXPECT_EQ(menu.GetSelectedItemIdx(), 0); // clamped to the first item

    // the value encoder should not change the selection
    menu.SelectItem(1);
    menu.OnValueEncoderTurned(-10, 12);
    EXPECT_EQ(menu.GetSelectedItemIdx(), 1);
    menu.OnValueEncoderTurned(10, 12);
    EXPECT_EQ(menu.GetSelectedItemIdx(), 1);
}

TEST(ui_AbstractMenu, e_allowEntering)
{
    // initializes the menu with value items
    // and checks if the menu allows or disallows
    // entering when okay button is pressed.

    ExposedAbstractMenu menu;

    // allow entering
    menu.AddValueItemsAndInit(
        AbstractMenu::Orientation::leftRightSelectUpDownModify, 2, true);

    EXPECT_TRUE(menu.AllowsEntering());
    EXPECT_FALSE(menu.IsEnteredForEditing());
    // enter editing mode
    menu.OnOkayButton(1, false);
    EXPECT_TRUE(menu.IsEnteredForEditing());
    // leave editing mode
    menu.OnOkayButton(1, false);
    EXPECT_FALSE(menu.IsEnteredForEditing());

    // Init again, this time don't allow entering
    menu.AddValueItemsAndInit(
        AbstractMenu::Orientation::leftRightSelectUpDownModify, 2, false);

    EXPECT_FALSE(menu.AllowsEntering());
    EXPECT_FALSE(menu.IsEnteredForEditing());
    // enter editing mode is forbidden
    menu.OnOkayButton(1, false);
    EXPECT_FALSE(menu.IsEnteredForEditing());
}

TEST(ui_AbstractMenu, f_closeItem)
{
    // initializes the menu with close items
    // and checks if the menu is closed when okay button is pressed

    ExposedAbstractMenu menu;

    menu.AddCloseItemsAndInit();

    // to be able to observe if the Close() funciton was called,
    // we must first add the menu page to a UI.
    UiEventQueue queue;
    UI           ui;
    ui.Init(queue, UI::SpecialControlIds{}, {}); // total dummy...
    ui.OpenPage(menu);

    // are we correctly added to the UI?
    EXPECT_EQ(menu.GetParentUI(), &ui);
    EXPECT_TRUE(menu.IsActive());

    // press the okay button
    menu.OnOkayButton(1, false);

    // menu should now be closed
    EXPECT_EQ(menu.GetParentUI(), nullptr);
    EXPECT_FALSE(menu.IsActive());
}

TEST(ui_AbstractMenu, g_callbackItem)
{
    // initializes the menu with callback items
    // and checks if the callback is called correctly.

    ExposedAbstractMenu menu;
    menu.AddCallbackItemsAndInit();

    // press the Okay buton => callback item is called
    EXPECT_FALSE(menu.callbackFunctionCalled_);
    menu.OnOkayButton(1, false);
    EXPECT_TRUE(menu.callbackFunctionCalled_);
}


TEST(ui_AbstractMenu, h_checkboxItem)
{
    // initializes the menu with checkbox items
    // and checks if they modify the value properly

    ExposedAbstractMenu menu;
    menu.AddCheckboxItemsAndInit(
        AbstractMenu::Orientation::leftRightSelectUpDownModify);

    ////////////////////////////////////////////////////////////////////
    // Modify the value with the ok button
    ////////////////////////////////////////////////////////////////////

    // press the Okay buton => value is toggled
    EXPECT_FALSE(menu.checkboxItemValue_);
    menu.OnOkayButton(1, false);
    EXPECT_TRUE(menu.checkboxItemValue_);
    menu.OnOkayButton(1, false);
    EXPECT_FALSE(menu.checkboxItemValue_);

    ////////////////////////////////////////////////////////////////////
    // Modify the value with the arrow buttons => horizontal orientation
    ////////////////////////////////////////////////////////////////////

    // up always sets the value to true, down always sets the value to false.
    menu.OnArrowButton(ArrowButtonType::up, 1, false);
    EXPECT_TRUE(menu.checkboxItemValue_);
    menu.OnArrowButton(ArrowButtonType::up, 1, false);
    EXPECT_TRUE(menu.checkboxItemValue_); // no toggling!
    menu.OnArrowButton(ArrowButtonType::down, 1, false);
    EXPECT_FALSE(menu.checkboxItemValue_);
    menu.OnArrowButton(ArrowButtonType::down, 1, false);
    EXPECT_FALSE(menu.checkboxItemValue_); // no toggling!

    ////////////////////////////////////////////////////////////////////
    // Modify the value with the arrow buttons => vertical orientation
    ////////////////////////////////////////////////////////////////////

    menu.AddCheckboxItemsAndInit(
        AbstractMenu::Orientation::upDownSelectLeftRightModify);

    // press the buttons that are used for modifying the value
    // right always sets the value to true, left always sets the value to false.
    menu.OnArrowButton(ArrowButtonType::right, 1, false);
    EXPECT_TRUE(menu.checkboxItemValue_);
    menu.OnArrowButton(ArrowButtonType::right, 1, false);
    EXPECT_TRUE(menu.checkboxItemValue_); // no toggling!
    menu.OnArrowButton(ArrowButtonType::left, 1, false);
    EXPECT_FALSE(menu.checkboxItemValue_);
    menu.OnArrowButton(ArrowButtonType::left, 1, false);
    EXPECT_FALSE(menu.checkboxItemValue_); // no toggling!

    ////////////////////////////////////////////////////////////////////
    // Modify the value with the value encoder
    ////////////////////////////////////////////////////////////////////

    // clockwise: set value to true
    menu.OnValueEncoderTurned(1, 12);
    EXPECT_TRUE(menu.checkboxItemValue_);
    menu.OnValueEncoderTurned(1, 12);
    EXPECT_TRUE(menu.checkboxItemValue_); // no toggling!

    // counter clockwise: set value to false
    menu.OnValueEncoderTurned(-1, 12);
    EXPECT_FALSE(menu.checkboxItemValue_);
    menu.OnValueEncoderTurned(-1, 12);
    EXPECT_FALSE(menu.checkboxItemValue_); // no toggling!

    ////////////////////////////////////////////////////////////////////
    // Modify the value with the value potentiometer
    ////////////////////////////////////////////////////////////////////

    // Checkbox value should be true if pot >= 0.5
    menu.OnValuePotMoved(1.0f);
    EXPECT_TRUE(menu.checkboxItemValue_);
    menu.OnValuePotMoved(0.0f);
    EXPECT_FALSE(menu.checkboxItemValue_);
    menu.OnValuePotMoved(0.5f);
    EXPECT_TRUE(menu.checkboxItemValue_);
    menu.OnValuePotMoved(0.4999f);
    EXPECT_FALSE(menu.checkboxItemValue_);
}

TEST(ui_AbstractMenu, i_valueItem)
{
    // initializes the menu with value items
    // and checks if they modify the value properly

    ExposedAbstractMenu menu;
    menu.AddValueItemsAndInit(
        AbstractMenu::Orientation::leftRightSelectUpDownModify);

    ////////////////////////////////////////////////////////////////////
    // Modify the value with the arrow buttons => horizontal orientation
    ////////////////////////////////////////////////////////////////////

    // press the up button
    menu.OnArrowButton(ArrowButtonType::up, 1, false);
    EXPECT_TRUE(menu.mappedIntValue_.stepCalled_);
    EXPECT_FALSE(menu.mappedIntValue_.useCoarseStepSizePassedIntoStep_);
    EXPECT_EQ(menu.mappedIntValue_.numStepsUpPassedIntoStep_, 1);
    menu.mappedIntValue_.stepCalled_ = false; // reset flag

    // use coarse grid size when func button is down
    menu.OnFunctionButton(1, false);
    menu.OnArrowButton(ArrowButtonType::up, 1, false);
    EXPECT_TRUE(menu.mappedIntValue_.stepCalled_);
    EXPECT_TRUE(menu.mappedIntValue_.useCoarseStepSizePassedIntoStep_);
    EXPECT_EQ(menu.mappedIntValue_.numStepsUpPassedIntoStep_, 1);
    menu.mappedIntValue_.stepCalled_ = false; // reset flag
    menu.OnFunctionButton(0, false);

    // press down button
    menu.OnArrowButton(ArrowButtonType::down, 1, false);
    EXPECT_TRUE(menu.mappedIntValue_.stepCalled_);
    EXPECT_FALSE(menu.mappedIntValue_.useCoarseStepSizePassedIntoStep_);
    EXPECT_EQ(menu.mappedIntValue_.numStepsUpPassedIntoStep_, -1);
    menu.mappedIntValue_.stepCalled_ = false; // reset flag

    // use coarse grid size when func button is down
    menu.OnFunctionButton(1, false);
    menu.OnArrowButton(ArrowButtonType::down, 1, false);
    EXPECT_TRUE(menu.mappedIntValue_.stepCalled_);
    EXPECT_TRUE(menu.mappedIntValue_.useCoarseStepSizePassedIntoStep_);
    EXPECT_EQ(menu.mappedIntValue_.numStepsUpPassedIntoStep_, -1);
    menu.mappedIntValue_.stepCalled_ = false; // reset flag
    menu.OnFunctionButton(0, false);

    ////////////////////////////////////////////////////////////////////
    // Modify the value with the arrow buttons => vertical orientation
    ////////////////////////////////////////////////////////////////////
    menu.AddValueItemsAndInit(
        AbstractMenu::Orientation::upDownSelectLeftRightModify);

    // press the right button
    menu.OnArrowButton(ArrowButtonType::right, 1, false);
    EXPECT_TRUE(menu.mappedIntValue_.stepCalled_);
    EXPECT_FALSE(menu.mappedIntValue_.useCoarseStepSizePassedIntoStep_);
    EXPECT_EQ(menu.mappedIntValue_.numStepsUpPassedIntoStep_, 1);
    menu.mappedIntValue_.stepCalled_ = false; // reset flag

    // use coarse grid size when func button is down
    menu.OnFunctionButton(1, false);
    menu.OnArrowButton(ArrowButtonType::right, 1, false);
    EXPECT_TRUE(menu.mappedIntValue_.stepCalled_);
    EXPECT_TRUE(menu.mappedIntValue_.useCoarseStepSizePassedIntoStep_);
    EXPECT_EQ(menu.mappedIntValue_.numStepsUpPassedIntoStep_, 1);
    menu.mappedIntValue_.stepCalled_ = false; // reset flag
    menu.OnFunctionButton(0, false);

    // press left button
    menu.OnArrowButton(ArrowButtonType::left, 1, false);
    EXPECT_TRUE(menu.mappedIntValue_.stepCalled_);
    EXPECT_FALSE(menu.mappedIntValue_.useCoarseStepSizePassedIntoStep_);
    EXPECT_EQ(menu.mappedIntValue_.numStepsUpPassedIntoStep_, -1);
    menu.mappedIntValue_.stepCalled_ = false; // reset flag

    // use coarse grid size when func button is down
    menu.OnFunctionButton(1, false);
    menu.OnArrowButton(ArrowButtonType::left, 1, false);
    EXPECT_TRUE(menu.mappedIntValue_.stepCalled_);
    EXPECT_TRUE(menu.mappedIntValue_.useCoarseStepSizePassedIntoStep_);
    EXPECT_EQ(menu.mappedIntValue_.numStepsUpPassedIntoStep_, -1);
    menu.mappedIntValue_.stepCalled_ = false; // reset flag
    menu.OnFunctionButton(0, false);

    ////////////////////////////////////////////////////////////////////
    // Modify the value with the value encoder
    ////////////////////////////////////////////////////////////////////

    // rotate the value encoder clockwise: increment value with
    // fine step size
    menu.OnValueEncoderTurned(4, 12);
    EXPECT_FALSE(menu.mappedIntValue_.setFrom0to1Called_);
    EXPECT_TRUE(menu.mappedIntValue_.stepCalled_);
    EXPECT_FALSE(menu.mappedIntValue_.useCoarseStepSizePassedIntoStep_);
    EXPECT_EQ(menu.mappedIntValue_.numStepsUpPassedIntoStep_, 4);
    menu.mappedIntValue_.stepCalled_ = false; // reset flag

    // rotate the value encoder counter clockwise with func
    // button pressed: decrement value with coarse step size
    menu.OnFunctionButton(
        1, false); // press function button to use coarse step size
    menu.OnValueEncoderTurned(-4, 12);
    EXPECT_FALSE(menu.mappedIntValue_.setFrom0to1Called_);
    EXPECT_TRUE(menu.mappedIntValue_.stepCalled_);
    EXPECT_TRUE(menu.mappedIntValue_.useCoarseStepSizePassedIntoStep_);
    EXPECT_EQ(menu.mappedIntValue_.numStepsUpPassedIntoStep_, -4);
    menu.mappedIntValue_.stepCalled_ = false; // reset flag
    menu.OnFunctionButton(
        0, false); // release function button to use fine step size

    ////////////////////////////////////////////////////////////////////
    // Modify the value with the menu encoder when entered for editing
    ////////////////////////////////////////////////////////////////////

    menu.OnOkayButton(1, false);

    // rotate the menu encoder clockwise: increment value with
    // fine step size
    menu.OnMenuEncoderTurned(4, 12);
    EXPECT_FALSE(menu.mappedIntValue_.setFrom0to1Called_);
    EXPECT_TRUE(menu.mappedIntValue_.stepCalled_);
    EXPECT_FALSE(menu.mappedIntValue_.useCoarseStepSizePassedIntoStep_);
    EXPECT_EQ(menu.mappedIntValue_.numStepsUpPassedIntoStep_, 4);
    menu.mappedIntValue_.stepCalled_ = false; // reset flag

    // rotate the value encoder counter clockwise with func
    // button pressed: decrement value with coarse step size
    menu.OnFunctionButton(
        1, false); // press function button to use coarse step size
    menu.OnMenuEncoderTurned(-4, 12);
    EXPECT_FALSE(menu.mappedIntValue_.setFrom0to1Called_);
    EXPECT_TRUE(menu.mappedIntValue_.stepCalled_);
    EXPECT_TRUE(menu.mappedIntValue_.useCoarseStepSizePassedIntoStep_);
    EXPECT_EQ(menu.mappedIntValue_.numStepsUpPassedIntoStep_, -4);
    menu.mappedIntValue_.stepCalled_ = false; // reset flag

    menu.OnOkayButton(1, false); // leave editing mode

    ////////////////////////////////////////////////////////////////////
    // Modify the value potentiometer
    ////////////////////////////////////////////////////////////////////

    menu.OnValuePotMoved(0.753f);
    EXPECT_TRUE(menu.mappedIntValue_.setFrom0to1Called_);
    EXPECT_FALSE(menu.mappedIntValue_.stepCalled_);
    EXPECT_EQ(menu.mappedIntValue_.valuePassedIntoSetFrom0to1_, 0.753f);
    menu.mappedIntValue_.setFrom0to1Called_ = false; // reset flag

    // should work exactly the same when in editing mode
    menu.OnOkayButton(1, false); // enter editing mode
    menu.OnValuePotMoved(0.123f);
    EXPECT_TRUE(menu.mappedIntValue_.setFrom0to1Called_);
    EXPECT_FALSE(menu.mappedIntValue_.stepCalled_);
    EXPECT_EQ(menu.mappedIntValue_.valuePassedIntoSetFrom0to1_, 0.123f);
    menu.mappedIntValue_.setFrom0to1Called_ = false; // reset flag
    menu.OnOkayButton(1, false);                     // leave editing mode
}

TEST(ui_AbstractMenu, j_openUiPageItem)
{
    // initializes the menu with a open UiPage items
    // and checks if the callback is called correctly.

    ExposedAbstractMenu menu;
    ExposedAbstractMenu subMenuToOpen;

    menu.AddOpenUiPageItemAndInit(subMenuToOpen);

    // The openUiPageItem opens the page on the UI that the menu
    // is being displayed on. The openUiPageItem can only work
    // it there's a parent UI => We must first add the menu page
    // to a UI.
    UiEventQueue queue;
    UI           ui;
    ui.Init(queue, UI::SpecialControlIds{}, {}); // total dummy...
    ui.OpenPage(menu);

    // press the Okay buton => page is opened on the parent UI
    EXPECT_FALSE(subMenuToOpen.IsActive());
    menu.OnOkayButton(1, false);
    EXPECT_TRUE(subMenuToOpen.IsActive()); // sub menu was opened
}

TEST(ui_AbstractMenu, k_customItemEditable)
{
    // initializes the menu with a custom item
    // and checks if it calls the correct functions

    ExposedAbstractMenu menu;
    // add custom item
    menu.AddCustomItemAndInit(
        AbstractMenu::Orientation::leftRightSelectUpDownModify);
    // configure custom item to be editable
    menu.customItem_.canBeEnteredForEditing_ = true;

    ////////////////////////////////////////////////////////////////////
    // Modify the value with the arrow buttons => horizontal orientation
    ////////////////////////////////////////////////////////////////////

    // press the up button
    menu.OnArrowButton(ArrowButtonType::up, 1, false);
    EXPECT_TRUE(menu.customItem_.modifySteppedCalled_);
    EXPECT_FALSE(menu.customItem_.isFuncButtonDownPassedIntoModifyStepped_);
    EXPECT_EQ(menu.customItem_.incrementsPassedIntoModifyStepped_, 1);
    EXPECT_EQ(menu.customItem_.stepsPerRevolutionPassedIntoModifyStepped_,
              0); // 0 to indicate that source was no encoder
    menu.customItem_.modifySteppedCalled_ = false; // reset flag

    // with func button down
    menu.OnFunctionButton(1, false);
    menu.OnArrowButton(ArrowButtonType::up, 1, false);
    EXPECT_TRUE(menu.customItem_.modifySteppedCalled_);
    EXPECT_TRUE(menu.customItem_.isFuncButtonDownPassedIntoModifyStepped_);
    EXPECT_EQ(menu.customItem_.incrementsPassedIntoModifyStepped_, 1);
    EXPECT_EQ(menu.customItem_.stepsPerRevolutionPassedIntoModifyStepped_,
              0); // 0 to indicate that source was no encoder
    menu.customItem_.modifySteppedCalled_ = false; // reset flag
    menu.OnFunctionButton(0, false);

    // press the down button
    menu.OnArrowButton(ArrowButtonType::down, 1, false);
    EXPECT_TRUE(menu.customItem_.modifySteppedCalled_);
    EXPECT_FALSE(menu.customItem_.isFuncButtonDownPassedIntoModifyStepped_);
    EXPECT_EQ(menu.customItem_.incrementsPassedIntoModifyStepped_, -1);
    EXPECT_EQ(menu.customItem_.stepsPerRevolutionPassedIntoModifyStepped_,
              0); // 0 to indicate that source was no encoder
    menu.customItem_.modifySteppedCalled_ = false; // reset flag

    // with func button down
    menu.OnFunctionButton(1, false);
    menu.OnArrowButton(ArrowButtonType::down, 1, false);
    EXPECT_TRUE(menu.customItem_.modifySteppedCalled_);
    EXPECT_TRUE(menu.customItem_.isFuncButtonDownPassedIntoModifyStepped_);
    EXPECT_EQ(menu.customItem_.incrementsPassedIntoModifyStepped_, -1);
    EXPECT_EQ(menu.customItem_.stepsPerRevolutionPassedIntoModifyStepped_,
              0); // 0 to indicate that source was no encoder
    menu.customItem_.modifySteppedCalled_ = false; // reset flag
    menu.OnFunctionButton(0, false);

    ////////////////////////////////////////////////////////////////////
    // Modify the value with the arrow buttons => vertical orientation
    ////////////////////////////////////////////////////////////////////
    menu.AddCustomItemAndInit(
        AbstractMenu::Orientation::upDownSelectLeftRightModify);

    // press the right button
    menu.OnArrowButton(ArrowButtonType::right, 1, false);
    EXPECT_TRUE(menu.customItem_.modifySteppedCalled_);
    EXPECT_FALSE(menu.customItem_.isFuncButtonDownPassedIntoModifyStepped_);
    EXPECT_EQ(menu.customItem_.incrementsPassedIntoModifyStepped_, 1);
    EXPECT_EQ(menu.customItem_.stepsPerRevolutionPassedIntoModifyStepped_,
              0); // 0 to indicate that source was no encoder
    menu.customItem_.modifySteppedCalled_ = false; // reset flag

    // with func button down
    menu.OnFunctionButton(1, false);
    menu.OnArrowButton(ArrowButtonType::right, 1, false);
    EXPECT_TRUE(menu.customItem_.modifySteppedCalled_);
    EXPECT_TRUE(menu.customItem_.isFuncButtonDownPassedIntoModifyStepped_);
    EXPECT_EQ(menu.customItem_.incrementsPassedIntoModifyStepped_, 1);
    EXPECT_EQ(menu.customItem_.stepsPerRevolutionPassedIntoModifyStepped_,
              0); // 0 to indicate that source was no encoder
    menu.customItem_.modifySteppedCalled_ = false; // reset flag
    menu.OnFunctionButton(0, false);

    // press the left button
    menu.OnArrowButton(ArrowButtonType::left, 1, false);
    EXPECT_TRUE(menu.customItem_.modifySteppedCalled_);
    EXPECT_FALSE(menu.customItem_.isFuncButtonDownPassedIntoModifyStepped_);
    EXPECT_EQ(menu.customItem_.incrementsPassedIntoModifyStepped_, -1);
    EXPECT_EQ(menu.customItem_.stepsPerRevolutionPassedIntoModifyStepped_,
              0); // 0 to indicate that source was no encoder
    menu.customItem_.modifySteppedCalled_ = false; // reset flag

    // with func button down
    menu.OnFunctionButton(1, false);
    menu.OnArrowButton(ArrowButtonType::left, 1, false);
    EXPECT_TRUE(menu.customItem_.modifySteppedCalled_);
    EXPECT_TRUE(menu.customItem_.isFuncButtonDownPassedIntoModifyStepped_);
    EXPECT_EQ(menu.customItem_.incrementsPassedIntoModifyStepped_, -1);
    EXPECT_EQ(menu.customItem_.stepsPerRevolutionPassedIntoModifyStepped_,
              0); // 0 to indicate that source was no encoder
    menu.customItem_.modifySteppedCalled_ = false; // reset flag
    menu.OnFunctionButton(0, false);

    ////////////////////////////////////////////////////////////////////
    // Modify the value encoder
    ////////////////////////////////////////////////////////////////////

    // rotate the value encoder clockwise with function button released
    menu.OnValueEncoderTurned(4, 12);
    EXPECT_TRUE(menu.customItem_.modifySteppedCalled_);
    EXPECT_FALSE(menu.customItem_.isFuncButtonDownPassedIntoModifyStepped_);
    EXPECT_EQ(menu.customItem_.incrementsPassedIntoModifyStepped_, 4);
    EXPECT_EQ(menu.customItem_.stepsPerRevolutionPassedIntoModifyStepped_, 12);
    menu.customItem_.modifySteppedCalled_ = false; // reset flag

    // rotate the value encoder counter clockwise with function button pressed
    menu.OnFunctionButton(1, false); // press function button
    menu.OnValueEncoderTurned(-4, 12);
    EXPECT_TRUE(menu.customItem_.modifySteppedCalled_);
    EXPECT_TRUE(menu.customItem_.isFuncButtonDownPassedIntoModifyStepped_);
    EXPECT_EQ(menu.customItem_.incrementsPassedIntoModifyStepped_, -4);
    EXPECT_EQ(menu.customItem_.stepsPerRevolutionPassedIntoModifyStepped_, 12);
    menu.customItem_.modifySteppedCalled_ = false; // reset flag
    menu.OnFunctionButton(0, false);               // release function button

    // The value encoder still works when in editing mode
    menu.OnOkayButton(1, false); // enter editing mode
    menu.OnValueEncoderTurned(4, 12);
    EXPECT_TRUE(menu.customItem_.modifySteppedCalled_);
    EXPECT_FALSE(menu.customItem_.isFuncButtonDownPassedIntoModifyStepped_);
    EXPECT_EQ(menu.customItem_.incrementsPassedIntoModifyStepped_, 4);
    EXPECT_EQ(menu.customItem_.stepsPerRevolutionPassedIntoModifyStepped_, 12);
    menu.customItem_.modifySteppedCalled_ = false; // reset flag
    menu.OnOkayButton(1, false);                   // leave editing mode

    ////////////////////////////////////////////////////////////////////
    // Modify the menu encoder
    ////////////////////////////////////////////////////////////////////

    menu.OnOkayButton(1, false); // enter editing mode

    // rotate the menu encoder clockwise with function button released
    menu.OnMenuEncoderTurned(4, 12);
    EXPECT_TRUE(menu.customItem_.modifySteppedCalled_);
    EXPECT_FALSE(menu.customItem_.isFuncButtonDownPassedIntoModifyStepped_);
    EXPECT_EQ(menu.customItem_.incrementsPassedIntoModifyStepped_, 4);
    EXPECT_EQ(menu.customItem_.stepsPerRevolutionPassedIntoModifyStepped_, 12);
    menu.customItem_.modifySteppedCalled_ = false; // reset flag

    // rotate the menu encoder counter clockwise with function
    // button pressed
    menu.OnFunctionButton(1, false); // press function button
    menu.OnMenuEncoderTurned(-4, 12);
    EXPECT_TRUE(menu.customItem_.modifySteppedCalled_);
    EXPECT_TRUE(menu.customItem_.isFuncButtonDownPassedIntoModifyStepped_);
    EXPECT_EQ(menu.customItem_.incrementsPassedIntoModifyStepped_, -4);
    EXPECT_EQ(menu.customItem_.stepsPerRevolutionPassedIntoModifyStepped_, 12);
    menu.customItem_.modifySteppedCalled_ = false; // reset flag
    menu.OnFunctionButton(0, false);               // release function button

    menu.OnOkayButton(1, false); // leave editing mode

    ////////////////////////////////////////////////////////////////////
    // Modify the value potentiometer
    ////////////////////////////////////////////////////////////////////

    // edit value with the value pot when not in editing mode
    menu.customItem_.modifyContinuousCalled_ = false;
    menu.OnValuePotMoved(0.753f);
    EXPECT_TRUE(menu.customItem_.modifyContinuousCalled_);
    EXPECT_EQ(menu.customItem_.valuePassedIntoModifyContinuous, 0.753f);
    menu.customItem_.isFuncButtonDownPassedIntoModifyContinuous_ = false;
    menu.customItem_.modifyContinuousCalled_ = false; // reset flag
    // repeat with function button pressed
    menu.OnFunctionButton(1, false);
    menu.OnValuePotMoved(0.53f);
    EXPECT_TRUE(menu.customItem_.modifyContinuousCalled_);
    EXPECT_EQ(menu.customItem_.valuePassedIntoModifyContinuous, 0.53f);
    menu.customItem_.isFuncButtonDownPassedIntoModifyContinuous_ = true;
    menu.customItem_.modifyContinuousCalled_ = false; // reset flag
    menu.OnFunctionButton(0, false);                  // release function button

    // edit value with the value pot works the same when in editing mode
    menu.OnOkayButton(1, false); // enter editing mode
    menu.customItem_.modifyContinuousCalled_ = false;
    menu.OnValuePotMoved(0.753f);
    EXPECT_TRUE(menu.customItem_.modifyContinuousCalled_);
    EXPECT_EQ(menu.customItem_.valuePassedIntoModifyContinuous, 0.753f);
    menu.customItem_.isFuncButtonDownPassedIntoModifyContinuous_ = false;
    menu.customItem_.modifyContinuousCalled_ = false; // reset flag
    // repeat with function button pressed
    menu.OnFunctionButton(1, false);
    menu.OnValuePotMoved(0.53f);
    EXPECT_TRUE(menu.customItem_.modifyContinuousCalled_);
    EXPECT_EQ(menu.customItem_.valuePassedIntoModifyContinuous, 0.53f);
    menu.customItem_.isFuncButtonDownPassedIntoModifyContinuous_ = true;
    menu.customItem_.modifyContinuousCalled_ = false; // reset flag
    menu.OnOkayButton(1, false);                      // leave editing mode

    // Okay button doesn't trigger the item action because the item is
    // editable and the okay button enters editing mode.
    menu.customItem_.onOkayButtonCalled_ = false; // reset flag
    menu.OnOkayButton(1, false);
    EXPECT_FALSE(menu.customItem_.onOkayButtonCalled_);
}

TEST(ui_AbstractMenu, l_customItemNonEditable)
{
    ExposedAbstractMenu menu;
    // add custom item
    menu.AddCustomItemAndInit(
        AbstractMenu::Orientation::leftRightSelectUpDownModify);
    // configure custom item to NOT be editable
    menu.customItem_.canBeEnteredForEditing_ = false;

    // Okay button triggers the item action because the item is
    // not editable and can't be entered for editing mode.
    menu.customItem_.onOkayButtonCalled_ = false; // reset flag
    menu.OnOkayButton(1, false);
    EXPECT_TRUE(menu.customItem_.onOkayButtonCalled_);
    EXPECT_FALSE(menu.IsEnteredForEditing());
}

TEST(ui_AbstractMenu, l_leaveEditingMode)
{
    // initializes the menu with a custom item
    // and checks if editing mode can be left with both the
    // OK and Cancel buttons

    ExposedAbstractMenu menu;
    // add custom item
    menu.AddCustomItemAndInit(
        AbstractMenu::Orientation::leftRightSelectUpDownModify);
    // configure custom item to be editable
    menu.customItem_.canBeEnteredForEditing_ = true;

    // to be able to observe if the Close() funciton was called,
    // we must first add the menu page to a UI.
    UiEventQueue queue;
    UI           ui;
    ui.Init(queue, UI::SpecialControlIds{}, {}); // total dummy...
    ui.OpenPage(menu);

    // are we correctly added to the UI?
    EXPECT_EQ(menu.GetParentUI(), &ui);
    EXPECT_TRUE(menu.IsActive());

    // enter editing mode
    menu.OnOkayButton(1, false);
    EXPECT_TRUE(menu.IsEnteredForEditing());
    // leave with the enter button
    menu.OnOkayButton(1, false);
    EXPECT_FALSE(menu.IsEnteredForEditing());
    // enter editing mode
    menu.OnOkayButton(1, false);
    EXPECT_TRUE(menu.IsEnteredForEditing());
    // leave with the cancel button
    menu.OnCancelButton(1, false);
    EXPECT_FALSE(menu.IsEnteredForEditing());
    EXPECT_TRUE(menu.IsActive()); // we didn't close the menu!
}

TEST(ui_AbstractMenu, m_closeMenu)
{
    // initializes the menu with a custom item
    // and checks if menu can be closed with the cancel button

    ExposedAbstractMenu menu;
    menu.AddCustomItemAndInit(
        AbstractMenu::Orientation::leftRightSelectUpDownModify);

    // to be able to observe if the Close() funciton was called,
    // we must first add the menu page to a UI.
    UiEventQueue queue;
    UI           ui;
    ui.Init(queue, UI::SpecialControlIds{}, {}); // total dummy...
    ui.OpenPage(menu);

    // are we correctly added to the UI?
    EXPECT_EQ(menu.GetParentUI(), &ui);
    EXPECT_TRUE(menu.IsActive());

    // close menu with the cancel button
    menu.OnCancelButton(1, false);
    EXPECT_FALSE(menu.IsActive());
}