/* GlossButton.cpp ************************************************************
Author:    Paul Watt
Date:      8/11/2011 11:03:21 AM
Purpose:   
Copyright 2011 Paul Watt
*******************************************************************************/
/* Includes ******************************************************************/
#include "../stdafx.h"
#include "GlossButton.h"
#include "BitBlender.h"

namespace article
{
/* Forward Declarations ******************************************************/
void DrawGlossBtn(HDC hdc, 
                  const RECT &rc, 
                  COLORREF c1, 
                  COLORREF c2,
                  COLORREF hiliteColor,
                  COLORREF outlineColor,
                  BYTE translucency);


/* Default Constructor *******************************************************/
GlossButton::GlossButton()
{
  m_colors.m_face       = ::GetSysColor(COLOR_BTNFACE);
  m_colors.m_shadow     = ::GetSysColor(COLOR_BTNSHADOW);
  m_colors.m_highlight  = ::GetSysColor(COLOR_3DLIGHT);
  m_colors.m_focus      = ::GetSysColor(COLOR_ACTIVECAPTION);
  m_colors.m_focusShadow= ::GetSysColor(COLOR_ACTIVECAPTION);
  m_colors.m_disabled   = ::GetSysColor(COLOR_INACTIVECAPTION);
}

/*****************************************************************************/
void GlossButton::GetColorProfile(ColorProfile& profile) const
{
  profile = m_colors;
}

/*****************************************************************************/
void GlossButton::SetColorProfile(const ColorProfile& profile)
{
  m_colors = profile;
}

/*****************************************************************************/
COLORREF GlossButton::GetForeColor() const
{
  return m_foreColor;
}

/*****************************************************************************/
void GlossButton::SetForeColor(COLORREF color)
{
  m_foreColor = color;
}

/*****************************************************************************/
HBITMAP GlossButton::GetImage()
{
  return m_hImage;
}

/*****************************************************************************/
HBITMAP GlossButton::SetImage(HBITMAP hNewImage)
{
  HBITMAP oldBmp = m_hImage;
  m_hImage = hNewImage;

  return oldBmp;
}

/*****************************************************************************/
void GlossButton::Copy_(const GlossButton& rhs)
{
  // Avoid copy to self.
  if (this == &rhs)
    return;

  // Release existing resources.
  Destroy_();

  // Copy the values from rhs into this object.
  m_colors      = rhs.m_colors;

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
void GlossButton::OnDrawBtn(DRAWITEMSTRUCT& drawInfo)
{
  HDC hdc = drawInfo.hDC;
  RECT rc = drawInfo.rcItem;
  SIZE sz = article::GetRectSize(rc);

  article::AutoSaveDC checkpoint(hdc);

  // Select the colors to paint based on current state.
  COLORREF c1;
  COLORREF c2;
  COLORREF hiliteClr  = k_black;
  COLORREF outlineClr = k_gray;

  BYTE transparency = k_alpha40;
  if (BST_PUSHED & ::SendMessage(drawInfo.hwndItem, BM_GETSTATE, 0, 0))
  {
    c1 = m_colors.m_focus;
    c2 = m_colors.m_shadow;
    outlineClr = m_colors.m_focusShadow;
    // Almost entirely remove the reflection when the button is pressed.
    transparency = 0x10;
  }
  else if (drawInfo.itemState == ODS_FOCUS)
  {
    c1 = m_colors.m_focus;
    c2 = m_colors.m_focusShadow;
    hiliteClr = RGB(0x20, 0xE0, 0xFF);
  }
  else if (drawInfo.itemState == ODS_DISABLED)
  {
    c1 = m_colors.m_disabled;
    c2 = m_colors.m_shadow;
  }
  else if (drawInfo.itemState == ODS_HOTLIGHT)
  {
    c1 = m_colors.m_highlight;
    c2 = m_colors.m_focus;
  }
  else
  {
    c1 = m_colors.m_face;
    c2 = m_colors.m_shadow;
  }

  // Draw onto a memory buffer to reduce flicker.
  article::MemDCBuffer memBuf(hdc, sz.cx, sz.cy);

  //!!! I am changing the view port origin of the DC I am passing into the
  //    parent window, so it will erase the background with the background
  //    that is suppose to be underneath this control.
  //    This will give a greater effect of transparency.
  // Have the parent erase the background for the area under this button.
  RECT rcParent;
  ::GetWindowRect(::GetParent(drawInfo.hwndItem), &rcParent);
  POINT pt    = {drawInfo.rcItem.left - rcParent.left, drawInfo.rcItem.top - rcParent.top};
  POINT curPt = {0,0};
  ::SetViewportOrgEx(memBuf, pt.x, pt.y, &curPt);
  ::SendMessage(::GetParent(drawInfo.hwndItem), WM_ERASEBKGND, WPARAM((HDC)memBuf), 0);
  // Restore the original origin.
  ::SetViewportOrgEx(memBuf, curPt.x, curPt.y, NULL);


  // Draw the Btn.
  DrawGlossBtn( memBuf, 
                rc, 
                c1,
                c2,
                hiliteClr,
                outlineClr,
                transparency);

  // Decorate the btn with the image or text.
  if (m_hImage)
  {
    HDC hImageDC = ::CreateCompatibleDC(hdc);
    ::SelectObject(hImageDC, m_hImage);
    COLORREF clrTransparent = ::GetPixel(hImageDC, 0, 0);

    article::MemDCBuffer testDc(hdc);
    AutoBrush gb(::CreateSolidBrush(m_foreColor));
    ::FillRect(testDc, &rc, (HBRUSH)gb);
    ::SelectObject(testDc, gb);
    ::BitBlt(testDc, 0, 0, sz.cx, sz.cy,
              hImageDC, 0, 0, SRCAND);

    BITMAP bm;
    ::GetObject(m_hImage, sizeof(BITMAP), &bm);
    POINT pt = {(sz.cx - bm.bmWidth) / 2, (sz.cy - bm.bmHeight) / 2};
    ::GdiTransparentBlt(memBuf, pt.x, pt.y, bm.bmWidth, bm.bmHeight,
                        testDc, 0, 0, bm.bmWidth, bm.bmHeight, 
                        clrTransparent);
    ::DeleteDC(hImageDC);
  }
  else
  {
    // Setup to draw the text.
    // Use the font selected into the original DC.
    ::SetTextColor(memBuf, ::GetTextColor(hdc));

    HFONT hFont = (HFONT)::SelectObject(hdc, ::GetStockObject(SYSTEM_FIXED_FONT));
    ::SelectObject(memBuf, hFont);
    ::SetBkMode(memBuf, TRANSPARENT);
    ::SetTextColor(hdc, m_foreColor);
    
    // Calculate the location of the text
    // Reduce the width to allow space for the borders.
    RECT textArea = rc;
    textArea.left  += 2;
    textArea.right -= 2;
    textArea.bottom = textArea.top + 5;

    int len = ::GetWindowTextLength(drawInfo.hwndItem);
    std::vector<TCHAR> textBuffer(len+1);
    ::GetWindowText(drawInfo.hwndItem, &textBuffer[0], len+1);

    ::DrawText(memBuf,&textBuffer[0], len, &textArea, DT_CENTER | DT_CALCRECT | DT_WORDBREAK);
    ::OffsetRect(&textArea, -2, 0);

    SIZE szRc   = article::GetRectSize(rc);
    SIZE szArea = article::GetRectSize(textArea);
    ::OffsetRect(&textArea, (szRc.cx-szArea.cx)/2, (szRc.cy-szArea.cy)/2);
    // Draw the text.
    ::DrawText(memBuf, &textBuffer[0], len, &textArea, DT_CENTER | DT_WORDBREAK);
  }

  // Flush the captured writes from the 
  // double buffer to the output DC.
  memBuf.Flush(hdc);
}


/******************************************************************************
Date:       8/11/2011
Purpose:    
Parameters: hdc
            rc
            c1
            c2
            hiliteClr
            translucency
*******************************************************************************/
void DrawGlossBtn( HDC hdc, 
                   const RECT& rc, 
                   COLORREF c1, 
                   COLORREF c2,
                   COLORREF hiliteClr,
                   COLORREF outlineClr,
                   BYTE translucency)
{
  // Stack managed object to restore the state of the DC.
  article::AutoSaveDC snapShot(hdc);

  SIZE sz = article::GetRectSize(rc);
  RECT rcIn = rc;
 
  // Create the base layer
  article::MemDCBuffer memBuf(hdc, sz.cx, sz.cy);
  ::FillRect(memBuf, &rc, (HBRUSH)::GetStockObject(WHITE_BRUSH));

  article::AutoRgn outlineRgn(::CreateRoundRectRgn( rcIn.left, rcIn.top,
                                                    rcIn.right, rcIn.bottom,
                                                    7, 7));
  ::FrameRgn(memBuf, outlineRgn, (HBRUSH)::GetStockObject(BLACK_BRUSH), 2, 2);

  // Create face of the button.
  ::InflateRect(&rcIn, -1, -1);
  article::AutoRgn innerRgn  (::CreateRoundRectRgn( rcIn.left, rcIn.top,
                                                    rcIn.right, rcIn.bottom,
                                                    7, 7));


  article::AutoBrush grayBrush(::CreateSolidBrush(outlineClr));

  ::SelectClipRgn(memBuf, innerRgn);
  RECT shadow = rc;
  shadow.top = shadow.top + (sz.cy / 2) - 3;
  article::RectGradient(memBuf, shadow, c1, c2, true);
  ::SelectClipRgn(memBuf, NULL);
  ::FrameRgn(memBuf, innerRgn, grayBrush, 1, 1);

  memBuf.Flush(hdc);

  // If the hi-lite color is not Black, paint the highlight ring.
  if (k_black != hiliteClr)
  {
    // TODO: Revisit this concept.
  }


  //// Create the reflection.
  //article::MemDCBuffer reflBuf(hdc, sz.cx, sz.cy/2);

  //// Adjustment values to help place the reflection.
  //int quarter   = sz.cy / 4;
  //int eighth    = sz.cy / 8;
  //int tenth     = sz.cy / 10;

  // Draw the elliptical reflection.
  RECT refBounds; 
  refBounds.left  = rcIn.right - sz.cx/2;
  refBounds.top   = rcIn.top + sz.cy/2;
  refBounds.right = rcIn.left + sz.cx;
  refBounds.bottom= refBounds.top + sz.cy;

  //article::AutoRgn reflRgn(::CreateEllipticRgn( refBounds.left, refBounds.top, 
  //                                              refBounds.right, refBounds.bottom));
  //::SelectClipRgn(memBuf, reflRgn);
  //article::RectGradient(memBuf, refBounds, k_cpDarkOrange, k_white, true, 0, COLOR16(0xFF));

  //// Blend the reflection.

  //BLENDFUNCTION bfRef = article::GetBlendFn(translucency, false);
  //::GdiAlphaBlend(hdc, rc.left, rc.top, sz.cx, sz.cy/2,
  //                memBuf, 0, 0, sz.cx, sz.cy/2, bfRef);

}


}
