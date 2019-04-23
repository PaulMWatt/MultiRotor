/* PaintUtility.h *************************************************************
Author:    Paul Watt
Date:      8/10/2011 4:32:19 PM
Purpose:   
Copyright 2011 Paul Watt
*******************************************************************************/
#ifndef PAINTUTILITY_H_INCLUDED
#define PAINTUTILITY_H_INCLUDED
#include <utility>
#include <vector>

namespace article
{

typedef std::pair<COLORREF, RECT>     ColorRect;
typedef std::vector<ColorRect>        ColorRectArray;


/* Functors ******************************************************************/
class PaintColorRect
{
public:
  HDC m_hdc;
  PaintColorRect(HDC hdc) 
    : m_hdc(hdc)            { }

  /****************************************************************************
  Purpose: Paints the specified color, in the enclosed rectangle with a 
           black border.
  *****************************************************************************/
  void operator()(const ColorRect& clrRect) 
  {
    const RECT &rc = clrRect.second;
    ::Rectangle(m_hdc, rc.left - 1,  rc.top - 1, rc.right + 1, rc.bottom + 1);
    ::FillRect (m_hdc, &rc, article::AutoBrush(::CreateSolidBrush(clrRect.first)));
  }
};

}

#endif
