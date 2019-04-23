/* BLMisc.h *******************************************************************
Author:   Paul Watt
Date:     12/28/2011
Purpose:  
Copyright 2011 Paul Watt
*******************************************************************************/
#ifndef BLMISC_H_INCLUDED
#define BLMISC_H_INCLUDED
/* Includes ******************************************************************/
#include "compiler.h"
#include <algorithm>

#if !defined(_WIN32)
struct POINT
{
  int32_t x;
  int32_t y;
};

struct RECT
{
  int32_t left;
  int32_t top;
  int32_t right;
  int32_t bottom;
};

struct SIZE
{
  int32_t cx;
  int32_t cy;
};

#endif


namespace pbl
{

/* Battery Definitions *******************************************************/
namespace battery
{
typedef byte_t   BatteryType;

const BatteryType k_notConnected  = 0;
const BatteryType k_deviceWired   = 1;
const BatteryType k_alkaline      = 2;
const BatteryType k_nimh          = 3;
const BatteryType k_unknown       = 4;

} // namespace battery


/* Forward Declarations ******************************************************/
class BLSize;
class BLPoint;
class BLRect;

/* Class **********************************************************************
Purpose:  A general purpose size object.
          Based on the SIZE structure defined in WIN32.
*******************************************************************************/
class BLSize 
  : public SIZE
{
public:
  /* Construction **************************************************************/
  BLSize()                                      { cx = 0;
                                                  cy = 0;
                                                }

	BLSize(int32_t width, int32_t height)         { cx = width;
                                                  cy = height;
                                                }

	BLSize(SIZE initSize)                         { cx = initSize.cx;
                                                  cy = initSize.cy;
                                                }
	
  BLSize(POINT initPt)                          { cx = initPt.x;
                                                  cy = initPt.y;
                                                }

  /* Operations ****************************************************************/
  bool operator ==(SIZE &size) const            {	return cx == size.cx 
                                                      && cy == size.cy;
	                                              }

  bool operator !=(SIZE &size) const            { return !(*this == size);}

	void operator +=(SIZE &size)	                { cx += size.cx;
		                                              cy += size.cy;
	                                              }

  void operator -=(SIZE &size)                  { cx -= size.cx;
		                                              cy -= size.cy;
	                                              }

  BLSize operator +(SIZE &size) const           {	return BLSize(cx + size.cx, 
                                                                cy + size.cy);
                                                }

	BLSize operator -(SIZE &size) const           { return BLSize(cx - size.cx, 
                                                                cy - size.cy);
                                                }

	BLSize operator -() const                     { return BLSize(-cx, -cy);}


  BLPoint operator +(POINT &point) const;
	BLPoint operator -(POINT &point) const;
  BLRect operator +(const RECT& rect) const;
	BLRect operator -(const RECT& rect) const;
};


/* Class **********************************************************************
Purpose:  A general purpose point object with integers.
          Based on the POINT structure defined in WIN32.
*******************************************************************************/
class BLPoint 
  : public POINT
{
public:
  /* Construction **************************************************************/
  BLPoint()                                     { x = 0;
                                                  y = 0;
                                                }

  BLPoint(int initX, int initY)                 { x = initX;
                                                  y = initY;
                                                }

  BLPoint(const POINT &initPt)                  { x = initPt.x;
                                                  y = initPt.y;
                                                }

  BLPoint(const SIZE &initSize)                 { x = initSize.cx;
                                                  y = initSize.cy;
                                                }

  /* Operations ****************************************************************/
	void Offset(int32_t xOffset, int32_t yOffset)	{	x += xOffset;
                                                  y += yOffset;
                                                }

	void Offset(POINT& point)                     { x += point.x;
		                                              y += point.y;
	                                              }

	void Offset(SIZE &size)                       { x += size.cx;
		                                              y += size.cy;
	                                              }

	bool operator ==(const POINT &point) const	  {	return ( x == point.x 
                                                        && y == point.y);
	                                              }

  bool operator !=(const POINT &point) const    { return !(*this == point);}

  void operator +=(const SIZE &size)            { x += size.cx;
		                                              y += size.cy;
	                                              }

	void operator -=(const SIZE &size)            { x -= size.cx;
		                                              y -= size.cy;
	                                              }

  void operator +=(const POINT &point)          { x += point.x;
		                                              y += point.y;
	                                              }

	void operator -=(const POINT &point)          { x -= point.x;
		                                              y -= point.y;
	                                              }


	BLPoint operator +(const SIZE &size) const    {	return BLPoint( x + size.cx, 
                                                                  y + size.cy);
                                                }

	BLPoint operator -(const SIZE &size) const    {	return BLPoint( x - size.cx, 
                                                                  y - size.cy);
	                                              }

	BLPoint operator -() const                  	{	return BLPoint(-x, -y);}

	BLPoint operator +(const POINT &point) const  {	return BLPoint( x + point.x, 
                                                                  y + point.y);
	                                              }

	BLSize operator -(const POINT &point) const   {	return BLSize(x - point.x, 
                                                                y - point.y);
	                                              }

  BLRect operator +(const RECT& rect) const;
	BLRect operator -(const RECT& rect) const;
};

/* Class **********************************************************************
Purpose:  A general purpose rectangle object with integers.
          Based on the RECT structure defined in WIN32.
*******************************************************************************/
class BLRect 
  : public RECT
{
public:
  /* Construction ************************************************************/
  BLRect()                                      { left    = 0;
                                                  top     = 0;
                                                  right   = 0;
                                                  bottom  = 0; 
                                                }

  /* Copy Constructor ********************************************************/
  BLRect(const RECT& rhs)                       { left    = rhs.left;
                                                  top     = rhs.top;
                                                  right   = rhs.right;
                                                  bottom  = rhs.bottom; 
                                                }

  /* Assignment Operator *****************************************************/
  BLRect& operator =(const RECT& rhs)           { if (this != &rhs)
                                                  { left  = rhs.left;
                                                    top   = rhs.top;
                                                    right = rhs.right;
                                                    bottom= rhs.bottom;
                                                  }

                                                  return *this;
                                                }

  /***************************************************************************/
  BLRect(int l, int t, int r, int b)            { left    = l;
                                                  top     = t;
                                                  right   = r;
                                                  bottom  = b; 
                                                }

  /***************************************************************************/
	BLRect(const POINT &point, const SIZE &size)  { left    = point.x;
                                                  top     = point.y;
                                                  right   = left + size.cx;
                                                  bottom  = top  + size.cy; 
                                                }

  /***************************************************************************/
	BLRect(const POINT &upperLeft, const POINT &lowerRight)
                                                { left    = upperLeft.x;
                                                  top     = upperLeft.y;
                                                  right   = lowerRight.x;
                                                  bottom  = lowerRight.y; 
                                                }

  /* Status ******************************************************************/
	int32_t Width() const                         { return right - left;}

	int32_t Height() const	                      { return bottom - top;}

	BLSize Size() const                           { return BLSize(Width(), 
                                                                Height());
	                                              }

  BLPoint UpperLeft() const                     { return BLPoint(left, top); }

	BLPoint LowerRight() const                    { return BLPoint(right, bottom); }

  BLPoint CenterPoint() const                   { return BLPoint((left + right) / 2, 
                                                                 (top + bottom) / 2);
	                                              }

	bool IsEmpty() const                          {	return   left == right    
                                                        && top  == bottom;
                                                }

	bool IsNull() const                           {	return   left   == 0
                                                        && right  == 0
                                                        && top    == 0
                                                        && bottom == 0;
                                                }  

	bool PtInRect(const POINT &pt) const          { return    pt.x > left
                                                        &&  pt.x < right
                                                        &&  pt.y > top
                                                        &&  pt.y < bottom;
                                                }
  
  /* Operations **************************************************************/
  bool operator ==(const RECT& rect) const      { return left   == rect.left
                                                      && top    == rect.top
                                                      && right  == rect.right
                                                      && bottom == rect.bottom;
                                                }

  bool operator !=(const RECT& rect) const      { return !(*this == rect); }

  BLRect& operator +=(const POINT &point)       { return Offset(point); }
  BLRect& operator +=(const SIZE &size)         { return Offset(size); }
  BLRect& operator +=(const RECT &rect)         { return Inflate( rect.left,
                                                                  rect.top,
                                                                  rect.right,
                                                                  rect.bottom); 
                                                }

  BLRect& operator -=(const POINT &point)       { return Offset(-BLPoint(point)); }
  BLRect& operator -=(const SIZE &size)         { return Offset(-BLSize(size)); }
  BLRect& operator -=(const RECT &rect)         { return SetDifference(*this,rect); }

  BLRect& operator &=(const RECT &rect)         { return SetIntersection(*this,rect); }
  BLRect& operator |=(const RECT &rect)         { return SetUnion(*this,rect); }

  /* Methods *****************************************************************/
  void Clear()                                  { left  = 0;
                                                  top   = 0;
                                                  right = 0;
                                                  bottom= 0;
                                                }

  BLRect& Inflate(int32_t x, int32_t y)         { left  -= x;
                                                  right += x;
                                                  top   -= y;
                                                  bottom+= y;
                                                  return *this;
                                                }

  BLRect& Inflate(const SIZE &size)             { left  -= size.cx;
                                                  right += size.cx;
                                                  top   -= size.cy;
                                                  bottom+= size.cy;
                                                  return *this;
                                                }

  BLRect& Inflate(int32_t l, int32_t t,
                  int32_t r, int32_t b)         { left  -= l;
                                                  right += r;
                                                  top   -= t;
                                                  bottom+= b;
                                                  return *this;
                                                }

  BLRect& Deflate(int32_t x, int32_t y)         { return Inflate(-x,-y); }

  BLRect& Deflate(const SIZE &size)             { return Inflate(-BLSize(size.cx, size.cy)); }

  BLRect& Deflate(int32_t l, int32_t t,
                  int32_t r, int32_t b)         { return Inflate(-l,-t,-r,-b); }

  BLRect& Offset(int32_t x, int32_t y)          { return Inflate(-x,-y,x,y); }
  BLRect& Offset(const SIZE& size)              { return Offset(size.cx, size.cy); }
  BLRect& Offset(const POINT& pt)               { return Offset(pt.x, pt.y); }

  BLRect& Normalize()                           { if (left > right)
                                                    std::swap(left, right);
                                                  if (top > bottom)
                                                    std::swap(top, bottom);
                                                }

	void MoveToY(int32_t y)                       {	bottom = Height() + y;
		                                              top = y;
	                                              }

	void MoveToX(int32_t x)	                      {	right = Width() + x;
		                                              left = x;
	                                              }

	void MoveTo(int32_t x, int32_t y)	            { MoveToX(x);
		                                              MoveToY(y);
	                                              }

	void MoveTo(const POINT &pt)                  {	MoveToX(pt.x);
		                                              MoveToY(pt.y);
	                                              }

	BLRect& SetIntersection(const BLRect &lhs, 
                          const BLRect &rhs);
	BLRect& SetUnion       (const BLRect &lhs, 
                          const BLRect &rhs);
	BLRect& SetDifference  (const BLRect &lhs, 
                          const BLRect &rhs);
};

/* Binary BLRect Operations **************************************************/
/*****************************************************************************/
inline
BLRect operator +(const RECT &lhs, const SIZE &rhs)
{
	BLRect rect(lhs);
  return rect.Offset(rhs);
}

/*****************************************************************************/
inline
BLRect operator -(const RECT &lhs, const SIZE &rhs)
{
	BLRect rect(lhs);
  return rect.Offset(-BLSize(rhs));
}

/*****************************************************************************/
inline
BLRect operator +(const RECT &lhs, const POINT &rhs)
{
	BLRect rect(lhs);
  return rect.Offset(rhs);
}

/*****************************************************************************/
inline
BLRect operator -(const RECT &lhs, const POINT &rhs)
{
	BLRect rect(lhs);
  return rect.Offset(-BLPoint(rhs));
}

/*****************************************************************************/
inline
BLRect operator +(const RECT &lhs, const RECT &rhs)
{
	BLRect rect(lhs);
	return rect += rhs;
}

/*****************************************************************************/
inline
BLRect operator -(const RECT &lhs, const RECT &rhs)
{
	BLRect rect(lhs);
  return rect -= rhs;
}

/*****************************************************************************/
inline
BLRect operator &(const RECT &lhs, const RECT& rhs)
{
	BLRect rect(lhs);
  return rect &= rhs;
}

/*****************************************************************************/
inline
BLRect operator |(const RECT &lhs, const RECT& rhs)
{
	BLRect rect(lhs);
  return rect |= rhs;
}

/*****************************************************************************/
inline
BLPoint BLSize::operator +(POINT &point) const        
{ 
  return BLPoint(cx + point.x, 
                 cy + point.y);
}

/*****************************************************************************/
inline
BLPoint BLSize::operator -(POINT &point) const        
{ 
  return BLPoint(cx - point.x, 
                 cy - point.y);
}

/*****************************************************************************/
inline
BLRect BLSize::operator +(const RECT& rect) const     
{ 
  RECT retVal   = rect;
  retVal.right += cx;
  retVal.bottom+= cy;
  return retVal;
}

/*****************************************************************************/
inline
BLRect BLSize::operator -(const RECT& rect) const     
{ 
  RECT retVal   = rect;
  retVal.right -= cx;
  retVal.bottom-= cy;
  return retVal;
}

/*****************************************************************************/
inline
BLRect BLPoint::operator +(const RECT& rect) const
{ 
  RECT retVal   = rect;
  retVal.right += x;
  retVal.bottom+= y;
  return retVal;
}

/*****************************************************************************/
inline
BLRect BLPoint::operator -(const RECT& rect) const     
{ 
  RECT retVal   = rect;
  retVal.right -= x;
  retVal.bottom-= y;
  return retVal;
}

/******************************************************************************
Date:       12/28/2011
Purpose:    Set this rectangle as the intersection of the two input rectangles.
Parameters: lhs[in]: Rectangle on the left side of the equation.
            rhs[in]: Rectangle on the left side of the equation.
Return:     A reference to this rectangle.
*******************************************************************************/
inline
BLRect& BLRect::SetIntersection(const BLRect &lhs, 
                                const BLRect &rhs)	  
{	
  this->Clear();
#if defined(_WIN32)                            
  ::IntersectRect(this, &lhs, &rhs);
#else
# error SetIntersection not implemented for this platform, add implementation
#endif
  return *this;
}

/******************************************************************************
Date:       12/28/2011
Purpose:    Set this rectangle as the union of the two input rectangles.
Parameters: lhs[in]: Rectangle on the left side of the equation.
            rhs[in]: Rectangle on the left side of the equation.
Return:     A reference to this rectangle.
*******************************************************************************/
inline
BLRect& BLRect::SetUnion(const BLRect &lhs, 
                         const BLRect &rhs)	  
{	
  this->Clear();
#if defined(_WIN32)                            
  ::UnionRect(this, &lhs, &rhs);
#else
# error SetUnion not implemented for this platform, add implementation
#endif
  return *this;
}

/******************************************************************************
Date:       12/28/2011
Purpose:    Set this rectangle as the difference of the rhs from lhs.
Parameters: lhs[in]: Rectangle on the left side of the equation.
            rhs[in]: Rectangle on the left side of the equation.
Return:     A reference to this rectangle.
*******************************************************************************/
inline
BLRect& BLRect::SetDifference(const BLRect &lhs, 
                              const BLRect &rhs)	  
{	
  this->Clear();

  left    = lhs.left    - rhs.left;
  top     = lhs.top     - rhs.top;
  right   = lhs.right   - rhs.right;
  bottom  = lhs.bottom  - rhs.bottom;

  return *this;
}

} // namespace pbl

typedef pbl::BLSize           BLSize;
typedef pbl::BLPoint          BLPoint;
typedef pbl::BLRect           BLRect;

#endif
