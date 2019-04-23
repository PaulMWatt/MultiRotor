/* GlossButton.h **************************************************************
Author:     Paul Watt
Date:       8/11/2011 9:42:50 AM
Purpose:    


Copyright 2011 Paul Watt
*******************************************************************************/
#ifndef GLOSSBUTTON_H_INCLUDED
#define GLOSSBUTTON_H_INCLUDED
/* Includes ******************************************************************/
#include <string>
#include "AutoGdi.h"
#include "ui_def.h"

namespace article
{

/* Class **********************************************************************
Purpose:  Implementation for an owner-drawn button using composition with the 
          techniques demonstrated in the Code Project article:
            ""
******************************************************************************/
class GlossButton
{
public:

  /* ColorProfile ***************************************************************
  Purpose:  A simple struct to hold a set of colors to conveniently associate with 
            gloss buttons.
  *******************************************************************************/
  struct ColorProfile
  {
    COLORREF    m_face;
    COLORREF    m_shadow;
    COLORREF    m_highlight;
    COLORREF    m_focus;
    COLORREF    m_focusShadow;
    COLORREF    m_disabled;
  };

  /* Construction ************************************************************/
    GlossButton();
    GlossButton(const GlossButton& rhs)           { Copy_(rhs);}
    ~GlossButton()                                { Destroy_();}

   GlossButton& operator=(const GlossButton& rhs) { Copy_(rhs);
                                                    return *this;
                                                  }

  void GetColorProfile(ColorProfile& profile) const;
  void SetColorProfile(const ColorProfile& profile);

  COLORREF GetForeColor() const;
  void     SetForeColor(COLORREF color);

  /* Configuration ***********************************************************/
  HBITMAP     GetImage();
  HBITMAP     SetImage(HBITMAP hNewImage);

  /* Message Handlers ********************************************************/
  void OnDrawBtn(DRAWITEMSTRUCT& drawInfo);

  /* Members *****************************************************************/
  ColorProfile m_colors;
  COLORREF     m_foreColor;

  HBITMAP     m_hImage;

private:
  void Copy_(const GlossButton& rhs);
  void Destroy_()                               { if (NULL != m_hImage)
                                                  {
                                                    ::DeleteObject(m_hImage);
                                                    m_hImage = NULL;
                                                  }
                                                }
};

/* Predefined Color Profiles *************************************************/
const GlossButton::ColorProfile k_silverProfile = 
{
  k_lightGray,              // m_face
  k_lightPurple,            // m_shadow
  k_white,                  // m_highlight
  k_white,                  // m_focus
  k_hilightPurple,          // m_focusShadow
  k_gray                    // m_disabled
};

const GlossButton::ColorProfile k_redProfile = 
{
  k_lightGray,              // m_face
  k_redBase,                // m_shadow
  k_white,                  // m_highlight
  k_white,                  // m_focus
  k_redBase,                // m_focusShadow
  k_gray                    // m_disabled
};

const GlossButton::ColorProfile k_greenProfile = 
{
  k_lightGray,              // m_face
  k_deepGreen,            // m_shadow
  k_white,                  // m_highlight
  k_white,                  // m_focus
  k_deepGreen,          // m_focusShadow
  k_gray                    // m_disabled
};




} // namespace article

#endif
