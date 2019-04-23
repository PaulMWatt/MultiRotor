/* OrbButton.cpp **************************************************************
Author:    Paul Watt
Date:      8/11/2011 11:03:21 AM
Purpose:   
Copyright 2011 Paul Watt
*******************************************************************************/
/* Includes ******************************************************************/
#include "../stdafx.h"
#include "OrbButton.h"
#include "BitBlender.h"
#include "aa_ellipse.h"
#include "ui_def.h"


namespace article
{
/* Forward Declarations ******************************************************/
void DrawOrb( HDC hdc, 
              const POINT &ctr, 
              int radius, 
              COLORREF c1, 
              COLORREF c2,
              BYTE translucency);


/* Default Constructor *******************************************************/
OrbButton::OrbButton()
  : m_isAllowFocus(true)
  , m_isAllowClick(true)
{
  m_colors.m_orb        = ::GetSysColor(COLOR_BTNFACE);
  m_colors.m_shadow     = ::GetSysColor(COLOR_BTNSHADOW);
  m_colors.m_highlight  = ::GetSysColor(COLOR_3DLIGHT);
  m_colors.m_focus      = ::GetSysColor(COLOR_ACTIVECAPTION);
  m_colors.m_disabledOrb= ::GetSysColor(COLOR_INACTIVECAPTION);
}

/*****************************************************************************/
void OrbButton::GetColorProfile(ColorProfile& profile) const
{
  profile = m_colors;
}

/*****************************************************************************/
void OrbButton::SetColorProfile(const ColorProfile& profile)
{
  m_colors = profile;
}


/*****************************************************************************/
StringType OrbButton::GetText()
{
  return m_text;
}

/*****************************************************************************/
void OrbButton::SetText(const StringType& btnText)
{
  m_text = btnText;
}

/*****************************************************************************/
HBITMAP OrbButton::GetImage()
{
  return m_hImage;
}

/*****************************************************************************/
HBITMAP OrbButton::SetImage(HBITMAP hNewImage)
{
  HBITMAP oldBmp = m_hImage;
  m_hImage = hNewImage;

  return oldBmp;
}

/*****************************************************************************/
void OrbButton::Copy_(const OrbButton& rhs)
{
  // Avoid copy to self.
  if (this == &rhs)
    return;

  // Release existing resources.
  Destroy_();

  // Copy the values from rhs into this object.
  m_colors.m_orb          = rhs.m_colors.m_orb;
  m_colors.m_shadow       = rhs.m_colors.m_shadow;
  m_colors.m_highlight    = rhs.m_colors.m_highlight;
  m_colors.m_focus        = rhs.m_colors.m_focus;
  m_colors.m_disabledOrb  = rhs.m_colors.m_disabledOrb;
  m_text                  = rhs.m_text;

  // Only copy the bitmap if one has been assigned.
  if (rhs.m_hImage)
  {
    BITMAP bm;
    ::GetObject(rhs.m_hImage, sizeof(BITMAP), &bm);

    HDC hdc     = ::GetDC(NULL);
    HDC hSrcDC  = ::CreateCompatibleDC(hdc);
    ::SelectObject(hSrcDC, rhs.m_hImage) ;
      
    HDC hDstDC  = ::CreateCompatibleDC(hdc);
    m_hImage = ::CreateCompatibleBitmap(hdc, bm.bmWidth, bm.bmHeight);
    ::SelectObject(hDstDC, m_hImage) ;

    ::BitBlt( hDstDC, 0, 0, bm.bmWidth, bm.bmHeight,
              hSrcDC, 0, 0, SRCCOPY);

    ::DeleteDC(hSrcDC);
    ::DeleteDC(hDstDC);
    ::ReleaseDC(NULL, hdc);
  }
  else
  {
    m_hImage = NULL;
  }
}

/*****************************************************************************/
void OrbButton::OnDrawBtn(DRAWITEMSTRUCT& drawInfo)
{
  HDC hdc = drawInfo.hDC;
  RECT rc = drawInfo.rcItem;

  int width = rc.right - rc.left;
  int height= rc.bottom - rc.top;

  int diameter  = min(width, height);
  int radius    = diameter / 2;
  POINT ctr     = {width / 2, height / 2};

  // Select the colors to paint based on current state.
  COLORREF c1;
  COLORREF c2;

  BYTE transparency = k_alpha40;
  if ( m_isAllowClick
    && BST_PUSHED & ::SendMessage(drawInfo.hwndItem, BM_GETSTATE, 0, 0))
  {
    c1 = m_colors.m_focus;
    c2 = m_colors.m_orb;
    // Almost entirely remove the reflection when the button is pressed.
    transparency = 0x10;
  }
  else if ( m_isAllowFocus
         && drawInfo.itemState == ODS_FOCUS)
  {
    c1 = m_colors.m_focus;
    c2 = m_colors.m_orb;
  }
  else if (drawInfo.itemState == ODS_DISABLED)
  {
    c1 = m_colors.m_disabledOrb;
    c2 = m_colors.m_shadow;
  }
  else if (drawInfo.itemState == ODS_HOTLIGHT)
  {
    c1 = k_red;
    c2 = k_white;
  }
  else
  {
    c1 = m_colors.m_orb;
    c2 = m_colors.m_shadow;

  }

  // Draw onto a memory buffer to reduce flicker.
  article::MemDCBuffer memBuf(hdc, width, height);

  // Have the parent erase the background.
  ::SendMessage(::GetParent(drawInfo.hwndItem), WM_ERASEBKGND, WPARAM((HDC)memBuf), 0);

  // Draw the Orb.
  DrawOrb(memBuf, 
          ctr, 
          radius, 
          c1,
          c2,
          transparency);

  // Decorate the orb with the image or text.
  if (m_hImage)
  {
    HDC hImageDC = ::CreateCompatibleDC(hdc);
    ::SelectObject(hImageDC, m_hImage);
    COLORREF clrTransparent = ::GetPixel(hImageDC, 0, 0);

    BITMAP bm;
    ::GetObject(m_hImage, sizeof(BITMAP), &bm);
    POINT pt = {(width - bm.bmWidth) / 2, (height - bm.bmHeight) / 2};
    ::GdiTransparentBlt(memBuf, pt.x, pt.y, bm.bmWidth, bm.bmHeight,
                        hImageDC, 0, 0, bm.bmWidth, bm.bmHeight, 
                        clrTransparent);
    ::DeleteDC(hImageDC);
  }
  else
  {
    HFONT orbFont = (HFONT)::SendMessage(drawInfo.hwndItem, WM_GETFONT, 0, 0);

    ::SelectObject(memBuf, orbFont);
    ::SetBkMode(memBuf, TRANSPARENT);

    ::SetTextColor(memBuf, m_colors.m_shadow);
    ::DrawText(memBuf, m_text.c_str(), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  }

  // Flush the captured writes from the 
  // double buffer to the output DC.
  memBuf.Flush(hdc);
}


/******************************************************************************
Date:       8/11/2011
Purpose:    
Parameters: hdc
            ctr
            radius
            c1
            c2
            translucency
*******************************************************************************/
void DrawOrb(HDC hdc, 
             const POINT &ctr, 
             int radius, 
             COLORREF c1, 
             COLORREF c2,
             BYTE translucency)
{
  // Stack managed object to restore the state of the DC.
  article::AutoSaveDC dcCtx(hdc);

  const int k_diameter = radius * 2;
  RECT rc = {ctr.x - radius, ctr.y - radius, ctr.x + radius, ctr.y +radius};


  HDC hMemDC = ::CreateCompatibleDC(hdc);

  // Create the base layer
  article::AutoRgn orbRgn(::CreateEllipticRgn(rc.left, rc.top+1, rc.right+1, rc.bottom+1));
  ::SelectClipRgn(hdc, orbRgn);
  article::RadialGradient(hdc, ctr.x, ctr.y, radius+1, c1, c2, 32);

  // Create the reflection.
  article::AutoBitmap refl(::CreateCompatibleBitmap(hdc, k_diameter, radius));
  ::SelectObject(hMemDC, refl);

  // Adjustment values to help place the reflection.
  int quarter   = radius / 4;
  int eighth    = radius / 8;
  int tenth     = radius / 10;
  int refWidth  = radius - quarter;
  int refHeight = refWidth / 2;

  // Draw the elliptical reflection.
  RECT refBounds; 
  refBounds.left  = quarter;
  refBounds.top   = tenth;
  refBounds.right = k_diameter - refBounds.left;
  refBounds.bottom= refBounds.top + radius - tenth;

  article::AutoRgn reflRgn(::CreateEllipticRgn( refBounds.left, refBounds.top, 
                                                refBounds.right, refBounds.bottom));
  ::SelectClipRgn(hMemDC, reflRgn);
    article::RectGradient(hMemDC, refBounds, k_black, k_white, true, 0, 0xff);

  // Blend the reflection.
  BLENDFUNCTION bfRef = article::GetBlendFn(translucency, true);
  ::GdiAlphaBlend(hdc, rc.left, rc.top, k_diameter, radius,
                  hMemDC, 0, 0, k_diameter, radius, bfRef);

  ::SelectClipRgn(hdc, NULL);
  // Outline the orb with an anti-aliased circle.
  LOGPEN lp;
  lp.lopnColor    = c2;
  lp.lopnStyle    = PS_SOLID;
  lp.lopnWidth.x  = 1;
  lp.lopnWidth.y  = 1;

  AutoPen outliner(::CreatePenIndirect(&lp));
  ::SelectObject(hdc, outliner);

  POINT ul = {rc.left, rc.top};
  POINT lr = {rc.right, rc.bottom};
  AAEllipse(hdc, ul, lr);

  ::DeleteDC(hMemDC);
}


}
