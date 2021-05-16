#pragma once
#include "hid/disp/display.h"
#include "hid/disp/graphics_common.h"
#include "util/MappedValue.h"

namespace daisy
{
/** Implements drawing routines for the `FullScreenItemMenu`. */
class FullScreenItemMenuLookAndFeelMethods
{
  public:
    virtual ~FullScreenItemMenuLookAndFeelMethods() {}

    virtual void FSIM_DrawTextItem(OneBitGraphicsDisplay& display,
                                   bool                   isVertical,
                                   uint16_t               selectedItemIdx,
                                   uint16_t               numItems,
                                   const char*            itemText) const;
    virtual void FSIM_DrawCheckboxItem(OneBitGraphicsDisplay& display,
                                       bool                   isVertical,
                                       uint16_t               selectedItemIdx,
                                       uint16_t               numItems,
                                       const char*            itemText,
                                       const bool& isCheckboxTicked) const;
    virtual void FSIM_DrawValueItem(OneBitGraphicsDisplay& display,
                                    bool                   isVertical,
                                    uint16_t               selectedItemIdx,
                                    uint16_t               numItems,
                                    const char*            itemText,
                                    const MappedValue&     value,
                                    bool                   isEditing) const;
    virtual void FSIM_DrawOpenUiPageItem(OneBitGraphicsDisplay& display,
                                         bool                   isVertical,
                                         uint16_t               selectedItemIdx,
                                         uint16_t               numItems,
                                         const char*            itemText) const;
    virtual void FSIM_DrawCloseMenuItem(OneBitGraphicsDisplay& display,
                                        bool                   isVertical,
                                        uint16_t               selectedItemIdx,
                                        uint16_t               numItems,
                                        const char*            itemText) const;

  private:
    virtual void FSIM_DrawTopRow(OneBitGraphicsDisplay& display,
                                 bool                   isVertical,
                                 int                    currentIndex,
                                 int                    numItemsTotal,
                                 const char*            text,
                                 Rectangle              rect,
                                 bool                   isSelected) const;
    virtual Rectangle
    FSIM_DrawUDArrowsAndGetRemRect(OneBitGraphicsDisplay& display,
                                   Rectangle              topRowRect,
                                   bool                   upAvailable,
                                   bool                   downAvailable) const;
    virtual Rectangle
                 FSIM_DrawLRArrowsAndGetRemRect(OneBitGraphicsDisplay& display,
                                                Rectangle              topRowRect,
                                                bool                   leftAvailable,
                                                bool                   rightAvailable) const;
    virtual void FSIM_DrawValueText(OneBitGraphicsDisplay& display,
                                    bool                   isVertical,
                                    const char*            text,
                                    Rectangle              rect,
                                    bool                   isBeingEdited) const;
    virtual int  FSIM_GetTopRowHeight(int displayHeight) const;

    virtual FontDef FSIM_GetValueFont(const char*      textToDraw,
                                      const Rectangle& availableSpace) const;
    virtual FontDef FSIM_GetNameFont(const char*      textToDraw,
                                     const Rectangle& availableSpace) const;
};

} // namespace daisy
