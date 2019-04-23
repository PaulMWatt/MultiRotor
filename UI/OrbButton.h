/* OrbButton.h ****************************************************************
Author:     Paul Watt
Date:       8/11/2011 9:42:50 AM
Purpose:    Simple implementation of an Orb Button that demonstrates image
            composition with the functions in msimg32.dll.

            This class has not dependencies on MFC or WTL.
            However, it should be fairly easy to port to one of those 
            frameworks, and you are welcome to do so.

            This button supports the following states and actions:
              - Enabled/Disabled
              - Focus/Non-Focused
              - Default Button
              - Checked/Unchecked
              
            The button supports both text and images, however, the button will
            always be a circle and therefore is not conducive to support more
            than a few letters based on the size of the button and font.
            The text and image can be dynamically changed.

            Only one color is required to be specified for the button.
            The default colors will be the system colors.
            All of the other colors will be derived.  However, these colors
            can be dynamically configured through the class:
              Orb Color
              Shadow Color
              Highlight Color
              Focus Color

Usage:      1) Define the button in the dialog resource editor, 
               use the BS_OWNERDRAW style, and attach the button handle
               in the WM_CREATE message of the parent window.

            Or

            2) Use the CreateButton member function to create an instance of
               this button on a specified parent window. 

Copyright 2011 Paul Watt
*******************************************************************************/
#ifndef ORBBUTTON_H_INCLUDED
#define ORBBUTTON_H_INCLUDED
/* Includes ******************************************************************/
#include <string>
#include "AutoGdi.h"
#include "ui_def.h"

namespace article
{

// If you would like to use some other string object,
// define the type in the USER_STRING_TYPE macro.
#ifndef USER_STRING_TYPE
# ifdef _UNICODE
#   define USER_STRING_TYPE std::wstring
# else
#   define USER_STRING_TYPE std::string
# endif
#endif

/* Typedefs ****************************************************************/
typedef USER_STRING_TYPE StringType;

/* Class **********************************************************************
Purpose:  Implementation for an owner-drawn button using composition with the 
          techniques demonstrated in the Code Project article:
            ""
******************************************************************************/
class OrbButton
{
public:
  /* ColorProfile struct *******************************************************/
  struct ColorProfile
  {
    COLORREF    m_orb;
    COLORREF    m_shadow;
    COLORREF    m_highlight;
    COLORREF    m_focus;
    COLORREF    m_disabledOrb;
  };

  /* Construction ************************************************************/
    OrbButton();
    OrbButton(const OrbButton& rhs)             { Copy_(rhs);}
    ~OrbButton()                                { Destroy_();}

   OrbButton& operator=(const OrbButton& rhs)   { Copy_(rhs);
                                                  return *this;
                                                }

  /* Configuration ***********************************************************/
  void GetColorProfile(ColorProfile& profile) const;
  void SetColorProfile(const ColorProfile& profile);

  StringType  GetText();
  void        SetText(const StringType& btnText);

  HBITMAP     GetImage();
  HBITMAP     SetImage(HBITMAP hNewImage);

  void EnableFocus(bool isAllowFocus)
  {
    m_isAllowFocus = isAllowFocus; 
  }

  void EnableClick(bool isAllowClick)
  {
    m_isAllowClick = isAllowClick;
  }

  /* Message Handlers ********************************************************/
  void OnDrawBtn(DRAWITEMSTRUCT& drawInfo);

private:
  /* Members *****************************************************************/
  ColorProfile m_colors;

  StringType  m_text;
  HBITMAP     m_hImage;

  bool m_isAllowFocus;
  bool m_isAllowClick;

  /* Methods *****************************************************************/
  void Copy_(const OrbButton& rhs);
  void Destroy_()                               { if (NULL != m_hImage)
                                                  {
                                                    ::DeleteObject(m_hImage);
                                                    m_hImage = NULL;
                                                  }
                                                }
};

} // namespace article


  const article::OrbButton::ColorProfile k_redOrb =
  {
    k_redBase,
    k_deepRed,
    k_white,
    k_red,
    k_darkGray
  };

  const article::OrbButton::ColorProfile k_greenOrb =
  {
    k_greenBase,
    k_deepGreen,
    k_white,
    k_green,
    k_darkGray
  };

  const article::OrbButton::ColorProfile k_blueOrb =
  {
    k_blueBase,
    k_deepBlue,
    k_white,
    k_blue,
    k_darkGray
  };

  const article::OrbButton::ColorProfile k_grayOrb =
  {
    k_darkGray,
    k_black,
    k_white,
    k_white,
    k_darkGray
  };


  const article::OrbButton::ColorProfile k_propGreenOrb =
  {
    k_cpDarkGreen,
    k_black,
    k_white,
    k_cpLightGreen,
    k_darkGray
  };

  const article::OrbButton::ColorProfile k_propOrangeOrb =
  {
    k_cpDarkOrange,
    k_black,
    k_white,
    k_cpLightOrange,
    k_darkGray
  };



#endif
