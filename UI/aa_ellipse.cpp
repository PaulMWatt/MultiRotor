/* aa_ellipse.cpp *************************************************************
Author:    Paul Watt
Date:      8/30/2011
Purpose:   
Copyright 2004 Paul Watt
*******************************************************************************/
/* Includes ******************************************************************/
#include "../stdafx.h"
#include <cmath>
#include "BitBlender.h"
#include "ui_def.h"

using namespace article;
/* Forward Declarations ******************************************************/
bool AAAngleArc(HDC,const POINT&, const POINT &, double, double);


/******************************************************************************
Date:       8/30/2011
Purpose:    
Parameters: hdc
            start
            end
Return:     true 
            false 
*******************************************************************************/
bool AAEllipse(HDC hdc, const POINT &start, const POINT &end)
{
  // Organize the parameters to make always calculate
  // with an upper-left and lower-right points.
  POINT p1 = start;
  POINT p2 = end;

  if (p1.x > p2.x)
    std::swap(p1.x, p2.x);

  if (p1.y > p2.y)
    std::swap(p1.y, p2.y);

  // Offset the Upper-Left point by one pixel to keep the drawing within
  // the requested bounds. dimensions in by one pixel.
  p1.x += 1;
  //p1.y += 1;

  // Calculate a radius and center point for the curve.
  LONG width  = p2.x - p1.x;
  LONG height = p2.y - p1.y;

  POINT radius;
  radius.x = width / 2;
  radius.y = height/ 2;

  POINT center;
  center.x = p1.x + radius.x;
  center.y = p1.y + radius.y;

  // Paint the arc with the full circumference of the circle.
  return AAAngleArc(hdc, center, radius, 0, 360);
}

/* Global *********************************************************************
Author:		Paul Watt
Date:		5/11/2004
Purpose:	Calculates the ratio of distribution for a circle passing between
			two pixels.
Parameters:	rad[in]: The radius of the circle.
			Y[in]: THe point Y on the circle.
Return:		THe Ratio is returned.
******************************************************************************/
double D(double rad, int Y)
{
	double result = std::sqrt((rad*rad) - (Y*Y));

	return std::ceil(result) - result;
}

/* Global *********************************************************************
Author:		Paul Watt
Date:		5/11/2004
Purpose:	Calculates the distribution of color between two pixels on a circle.
Parameters:	T[in]: The ratio of where the circle passes between the two pixels.
			first[in]: THe first color;
			second[in]: The second color.
			flip[in]: Indicates the colors should be swapped.
Return:		The new color that is calculated from the distribution.
******************************************************************************/
inline USHORT H_Distribute(double T, USHORT first, USHORT second, bool flip)
{
	SHORT range = first - second;

	if ((range > 0 && !flip) || (range < 0 && flip))
	{
		return USHORT(abs(range) * (1-T) + min(first, second));
	}
	else
	{
		return USHORT(abs(range) * (T) + min(first, second));
	}
}


/* Global *********************************************************************
Author:		Paul Watt
Date:		5/11/2004
Purpose:	Function that evenly distributes the intensity of color to the 
			pixels based on what is currently held in the DC image and the 
			parameters passed in.
Parameters:	hdc[in]: The DC to draw on.
			cur[in]: THe current point to draw on.
			T[in]: A ratio value that determines how to distribute the intensity
				between the two pixels that will be painted.
			pen[in]: The color of the pen to use when painting the circle points.
			flip[in]: Indicates if the distribution should be flipped.  This
				occurs during the duplication of points.
			isHorizontal[in]: Indicates the distribution should be adjusted 
				vertically rather than horizontally.
Return:		If the function succeeds thrn TRUE will be returned otherwise FALSE.
******************************************************************************/
inline BOOL H_ColorCircle(HDC hdc, POINT &cur, double T, COLORREF pen, bool flip, bool isHorizontal)
{
	COLORREF P1;
	COLORREF clr = ::GetPixel(hdc, cur.x, cur.y);
	P1 = RGB(H_Distribute(T, GetRValue(pen), GetRValue(clr), flip),
			 H_Distribute(T, GetGValue(pen), GetGValue(clr), flip),
			 H_Distribute(T, GetBValue(pen), GetBValue(clr), flip));

	::SetPixel(hdc, cur.x, cur.y, P1);

	COLORREF P2;
	if (isHorizontal)
	{
		clr = ::GetPixel(hdc, cur.x, cur.y-1);
	}
	else
	{
		clr = ::GetPixel(hdc, cur.x-1, cur.y);
	}

	P2 = RGB(H_Distribute(T, GetRValue(pen), GetRValue(clr), !flip),
			 H_Distribute(T, GetGValue(pen), GetGValue(clr), !flip),
			 H_Distribute(T, GetBValue(pen), GetBValue(clr), !flip));

	if (isHorizontal)
	{
		::SetPixel(hdc, cur.x, cur.y-1, P2);
	}
	else
	{
		::SetPixel(hdc, cur.x-1, cur.y, P2);
	}

	return TRUE;
}

/* Global *********************************************************************
Author:		Paul Watt
Date:		5/10/2004
Purpose:	This function will paint out an anti-aliased arc segment of a circle
			based on the start angle and sweep angle specified.
Parameters:	hdc[in]: The device content in which the angle will be painted.
			X[in]: The X coordinate of the arcs center.
			Y[in]: THe Y coordinate of the arcs center.
			radius[in]: A positive value that specifies the circles radius in
				logical units.
			startAngle[in]: Specifies the start angle in degrees relative to
				the x-axis.
			sweepAngle[in]: Specifies the sweep and in degrees relative to the
				start angle.
Return:		IF the function succeeds then the return value is non-zero, otherwise
			zero will be returned.

Note:		Currently this function only supports MM_TEXT mode.
******************************************************************************/
bool AAAngleArc(HDC hdc, const POINT& ctr, const POINT &rad, double theta, double phi)
{
  //C: Calculate start pos (x, y, angle 0).
  POINT start = {ctr.x + rad.x, ctr.y};
  //C: Convert the input angles into radians.
  double radStart = theta * (k_degToRad);
  double radEnd   = radStart + (phi * (k_degToRad));
  //C: Insure the start angle is less than the end angle.
  if (radStart > radEnd)
  {
    double temp = radStart;
    radStart = radEnd;
    radEnd = temp;
  }
  //C: Insure both angles are within 0-2k_pi
  while (radStart > (k_2pi))
  {
    radStart -= (k_2pi);
  }

  while (radStart < 0)
  {
    radStart += (k_2pi);
  }

  while (radEnd > (k_2pi))
  {
    radEnd -= (k_2pi);
  }

  while (radEnd < 0)
  {
    radEnd += (k_2pi);
  }

  //C: Get the pen color from the HDC.
  LOGPEN		pen;
  COLORREF	penColor;
  HPEN		hPen = (HPEN)::GetCurrentObject(hdc, OBJ_PEN);
  if (!::GetObject(hPen, sizeof(LOGPEN), &pen))
  {
    penColor = RGB(0,0,0);
  }
  else
  {	
    penColor = pen.lopnColor;
  }

  POINT cur;
  cur.x = rad.x;
  cur.y = 0;

  double T = 0;
  //C: Handle special case pixels.
  if (0 >= radStart && (k_2pi) <= radEnd)
  {
    ::SetPixel(hdc, ctr.x + rad.x, ctr.y, penColor);
  }

  if (k_pi_2 >= radStart && k_pi_2 <= radEnd)
  {
    //		::SetPixel(hdc, ctr.x, ctr.y + rad.x, penColor);
  }

  if (k_pi >= radStart && k_pi <= radEnd)
  {
    //		::SetPixel(hdc, ctr.x - rad.x - 1, ctr.y, penColor);
  }

  if ((k_3pi_2) >= radStart && (k_3pi_2) <= radEnd)
  {
    //		::SetPixel(hdc, ctr.x, ctr.y - rad.x - 1, penColor);
  }

  while (cur.x > cur.y)
  {
    double ratio = D(rad.x, cur.y);
    if (ratio < T)
    {
      cur.x--;
    }

    T = ratio;
    //C: Calculate the current angle for octant I.
    double radCurrent = std::atan2((double)cur.y, cur.x);
    //C: Process each of the 8 octants.
    int		octant;
    POINT	pos;
    double  radAdjust;
    bool	flip;
    bool	isHorizontal;
    for (octant = 1; octant <= 8; octant++)
    {
      switch (octant)
      {
      case 1:
        pos.x = cur.x;
        pos.y = cur.y;
        radAdjust = radCurrent;
        flip = false;
        isHorizontal = false;
        break;
      case 2:
        pos.x = cur.y;
        pos.y = cur.x;
        radAdjust = (k_pi_2) - radCurrent;
        flip = false;
        isHorizontal = true;
        break;
      case 3:
        pos.x = -cur.y - 1;
        pos.y = cur.x;
        radAdjust = (k_pi_2) + radCurrent;
        flip = false;
        isHorizontal = true;
        break;
      case 4:
        pos.x = -cur.x;
        pos.y =  cur.y;
        radAdjust = k_pi - radCurrent;
        flip = true;
        isHorizontal = false;
        break;
      case 5:
        pos.x = -cur.x;
        pos.y = -cur.y;
        radAdjust = k_pi + radCurrent;
        flip = true;
        isHorizontal = false;
        break;
      case 6:
        pos.x = -cur.y - 1;
        pos.y = -cur.x + 1;
        radAdjust = (k_3pi_2) - radCurrent;
        flip = true;
        isHorizontal = true;
        break;
      case 7:
        pos.x = cur.y;
        pos.y = -cur.x + 1;
        radAdjust = (k_3pi_2) + radCurrent;
        flip = true;
        isHorizontal = true;
        break;
      case 8:
        pos.x =  cur.x;
        pos.y = -cur.y;
        radAdjust = (k_2pi) - radCurrent;
        flip = false;
        isHorizontal = false;
        break;
      }
      //C: If the point on the circle in the current octant is within the
      //   range to be painted, color the pixel.
      if (radAdjust > radStart && radAdjust < radEnd)
      {
        pos.x += ctr.x;
        pos.y += ctr.y;
        H_ColorCircle(hdc, pos, T, penColor, flip, isHorizontal);
      }
    }

    cur.y++;
  }

  return TRUE;
}

