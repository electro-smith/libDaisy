#pragma once

#include "AbstractMenu.h"

namespace daisy
{
/** @brief A menu page for small screens
 *  @author jelliesen
 *  @ingroup ui
 * 
 *  This class builds upon the menu logic of AbstractMenu and adds 
 *  drawing routines that are suitable for small screens.
 * 
 *  By default, it will paint to the canvas returned by 
 *  `UI::GetPrimaryOneBitGraphicsDisplayId()`. It can also be 
 *  configured to paint to a different canvas.
 * 
 *  Each item will occupy the entire display.
 *  FullScreenItemMenu uses the LookAndFeel system to draw draw the
 *  items. This means that you can create your own graphics design 
 *  by creating your own LookAndFeel based on the OneBitGraphicsLookAndFeel
 *  class and apply that either globally (UI::SetOneBitGraphicsLookAndFeel())
 *  or to this page only (UiPage::SetOneBitGraphicsLookAndFeel()).
 */
class FullScreenItemMenu : public AbstractMenu
{
  public:
    /** Call this to initialize the menu. It's okay to re-initialize a 
     *  FullScreenItemMenu multiple times, even while it's displayed on the UI.
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
    void Init(const AbstractMenu::ItemConfig* items,
              uint16_t                        numItems,
              AbstractMenu::Orientation       orientation
              = AbstractMenu::Orientation::leftRightSelectUpDownModify,
              bool allowEntering = true);

    /** Call this to change which canvas this menu will draw to. The canvas
     *  must be a `OneBitGraphicsDisplay`, e.g. the `OledDisplay` class.
     *  If `canvasId == UI::invalidCanvasId` then this menu will draw to the
     *  canvas returned by `UI::GetPrimaryOneBitGraphicsDisplayId()`. This
     *  is also the default behaviour.
     */
    void SetOneBitGraphicsDisplayToDrawTo(uint16_t canvasId);

    // inherited from UiPage
    void Draw(const UiCanvasDescriptor& canvas) override;

  private:
    uint16_t canvasIdToDrawTo_ = UI::invalidCanvasId;

    //////////////////////////////////////////////////////////////////////
    // Drawing routines
    //////////////////////////////////////////////////////////////////////
    virtual int  GetTopRowHeight(int displayHeight) const;
    virtual void DrawTextItem(OneBitGraphicsDisplay& display,
                              bool                   isVertical,
                              uint16_t               selectedItemIdx,
                              uint16_t               numItems,
                              const char*            itemText) const;
    virtual void DrawCheckboxItem(OneBitGraphicsDisplay& display,
                                  bool                   isVertical,
                                  uint16_t               selectedItemIdx,
                                  uint16_t               numItems,
                                  const char*            itemText,
                                  const bool& isCheckboxTicked) const;
    virtual void DrawValueItem(OneBitGraphicsDisplay& display,
                               bool                   isVertical,
                               uint16_t               selectedItemIdx,
                               uint16_t               numItems,
                               const char*            itemText,
                               const MappedValue&     value,
                               bool                   isEditing) const;
    virtual void DrawOpenUiPageItem(OneBitGraphicsDisplay& display,
                                    bool                   isVertical,
                                    uint16_t               selectedItemIdx,
                                    uint16_t               numItems,
                                    const char*            itemText) const;

    virtual void      DrawCloseMenuItem(OneBitGraphicsDisplay& display,
                                        bool                   isVertical,
                                        uint16_t               selectedItemIdx,
                                        uint16_t               numItems,
                                        const char*            itemText) const;
    virtual void      DrawTopRow(OneBitGraphicsDisplay& display,
                                 bool                   isVertical,
                                 int                    currentIndex,
                                 int                    numItemsTotal,
                                 const char*            text,
                                 Rectangle              rect,
                                 bool                   isSelected) const;
    virtual Rectangle DrawUDArrowsAndGetRemRect(OneBitGraphicsDisplay& display,
                                                Rectangle topRowRect,
                                                bool      upAvailable,
                                                bool      downAvailable) const;
    virtual Rectangle DrawLRArrowsAndGetRemRect(OneBitGraphicsDisplay& display,
                                                Rectangle topRowRect,
                                                bool      leftAvailable,
                                                bool      rightAvailable) const;

    virtual void    DrawValueText(OneBitGraphicsDisplay& display,
                                  bool                   isVertical,
                                  const char*            text,
                                  Rectangle              rect,
                                  bool                   isBeingEdited) const;
    virtual FontDef GetValueFont(const char*      textToDraw,
                                 const Rectangle& availableSpace) const;

    virtual FontDef GetNameFont(const char*      textToDraw,
                                const Rectangle& availableSpace) const;
};
} // namespace daisy
