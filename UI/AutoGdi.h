/* AutoGDI.h ******************************************************************
Author:    Paul Watt
Date:      7/21/2011 11:10:01 PM
Purpose:   A few GDI helper objects and functions for use with C++ win32 GDI 
           development in the absence of MFC or WTL.
Copyright 2011 Paul Watt
*******************************************************************************/
#ifndef AutoObj_H_INCLUDED
#define AutoObj_H_INCLUDED

/* Includes ******************************************************************/
#include <windows.h>

namespace article
{
/* Class Definitions *********************************************************/
// These objects are defined below to allow object cleanup on the stack.
// AutoBitmap   Manages HBITMAP handles
// AutoBrush    Manages HBRUSH handles
// AutoFont     Manages HFONT handles
// AutoPen      Manages HPEN handles
// AutoRgn      Manages HRGN handles 
// AutoPalette  Manages HPALETTE handles

// AutoSaveDC   Takes a snapshot of the current DC.  Restores when leaves scope.

// MemDCBuffer Creates and manages a memory DC buffer for double buffer painting.

/* Utility Functions *********************************************************/
/******************************************************************************
Date:       8/14/2011
Purpose:    Calculates the Height and Width of a rectangle.
Parameters: rc[in]: A RECT struct where left <= right and top <= bottom.
Return:     Returns a SIZE struct with the calculated height and width.
*******************************************************************************/
inline
SIZE GetRectSize(const RECT& rc) 
{
  SIZE sz = {rc.right - rc.left, rc.bottom - rc.top};
  return sz;
}

/******************************************************************************
Date:       8/14/2011
Purpose:    Calculates the center point of a rectangle.
Parameters: rc[in]: A RECT struct where left <= right and top <= bottom.
            sz[in]: Size has already been calculated for the rectangle.
Return:     Returns the center point of the rectangle.
*******************************************************************************/
inline
POINT GetRectCenter(const RECT& rc, const SIZE& sz) 
{
  POINT pt = {rc.left + sz.cx/2, rc.top + sz.cy/2};
  return pt;
}

/******************************************************************************
Date:       8/14/2011
Purpose:    Calculates the center point of a rectangle.
Parameters: rc[in]: A RECT struct where left <= right and top <= bottom.
Return:     Returns the center point of the rectangle.
*******************************************************************************/
inline
POINT GetRectCenter(const RECT& rc) 
{
  return GetRectCenter(rc, GetRectSize(rc));
}

/* GDI HANDLE Resource Management Object **************************************
Purpose:    This class provides the template to cleanup any of the GDI objects
            that should be freed with a call to ::DeleteObject.
            This object is meant to be created on the stack.  When the object
            goes out of scope, it will automatically clean up the GDI resources.

Note:       This object is only intended to be declared on the stack.
Do not declare this object with "static"
******************************************************************************/
template <typename T>
class AutoObj
{
public:
  T   handle;

  /* Default Constructor *****************************************************/
  // Use Attach to assign a handle to an empty Auto object.
  AutoObj ()  
    : handle(NULL)          { }

  /* explicit Handle constructor *********************************************/
  // Wouldn't it be a bitch for the compiler to convert one of your
  // GDI Object Handles into one of these objects (secretly), 
  // and destroy it without telling you.  That's why this is defined explicit.
  explicit
    AutoObj (T in)  
    : handle(in)            { }

  /* Destructor **************************************************************/
  ~AutoObj()                { ::DeleteObject(handle);}

  /* GDI Object Handle Conversion Operator ***********************************/
  operator T() const        { return handle;} 

  /* Public ******************************************************************
  Purpose:    Attach an unmanaged handle to this auto-object 
              for resource management.
  Parameters: in[in]: The input handle to be managed.
  Return:     If the object is not currently managing a handle, the input
              handle will become managed by this object and true is returned.
              If false is returned, this object is already managing a handle.
  ***************************************************************************/
  bool Attach(T in)         { if (NULL == handle)
                                handle = in;
                              return handle == in;
                            }

  /* Public ******************************************************************
  Purpose:    Detach a managed handle from this auto-object.
              The caller then takes responsibility for resource management
              of the GDI Object.
  Return:     The managed handle is returned.
              If there is currently no handle, then NULL will be returned.
  ***************************************************************************/
  T Detach()                { T retVal = handle;
                              handle = NULL;
                              return retVal;
                            }

private:
  // Make this object non-copyable by hiding these functions"
  AutoObj(const AutoObj&);            // Copy Constructor
  AutoObj& operator=(const AutoObj&); // Assignment Operator
  // Prohibit some other nonsensical functions.
  bool Attach(T in) const;            
  T    Detach()     const;
};


/* DC Snapshot manager ********************************************************
Purpose:    Takes a snapshot of the current DC state.  When the object goes
            out of scope, it will restore the context of the DC.

            The restoration can be forced sooner by calling Restore.

Note:       This object is only intended to be declared on the stack.
            Do not declare this object with "static"
******************************************************************************/
class AutoSaveDC
{
public:
  /* Construction ************************************************************/
  explicit
    AutoSaveDC(HDC hdc)     
    : m_hdc(hdc)            { m_ctx = ::SaveDC(hdc);}
   ~AutoSaveDC()            { Restore();}

 /* Methods ******************************************************************/
 void Restore()             { if (0 != m_ctx)
                              {
                                ::RestoreDC(m_hdc, m_ctx);
                                m_ctx = 0;
                                m_hdc = 0;
                              }
                            }
private:
  /* Members *****************************************************************/
  HDC m_hdc;                // HDC to restore
  int m_ctx;                // Saved context

  /* Methods *****************************************************************/
  // No default constructor.
  AutoSaveDC();
  // Make this object non-copyable by hiding these functions"
  AutoSaveDC(const AutoSaveDC&);            // Copy Constructor
  AutoSaveDC& operator=(const AutoSaveDC&); // Assignment Operator
};

/* Class **********************************************************************
Purpose:    Implements a double-buffer from an input DC Handle.
            A size should be specified to indicate the size of the backup buffer.
Construction:            
            However, if the size is omitted, and a Memory DC is passed in, 
            the constructor will attempt to create a duplicate buffer the 
            size of the buffer selected into the input Memory DC.

            If the size is omitted, and the input DC is not a Memory DC, all
            input directed to this object will simply write directly to the 
            original DC.

IsBuffered: Return true if this object maintains a double buffer, false otherwise.
Flush:      Writes the data from the backup buffer into the specified buffer.
            
*****************************************************************************/
class MemDCBuffer
{
public:
  /* Construction ************************************************************/
  explicit 
    MemDCBuffer(HDC hdc)                       
    : m_buffer(BufferInit_(hdc))            { }

    MemDCBuffer(HDC hdc, int cx, int cy)   
    : m_buffer(BufferInit_(hdc, cx, cy))    { }

   ~MemDCBuffer()                          { if (IsBuffered())
                                                ::DeleteDC(m_memDC);
                                            }

  /* Operators ***************************************************************/
  operator HDC()                            { return m_memDC;}

  /* Status ******************************************************************/
  bool IsBuffered() const                   { return NULL != (HBITMAP)m_buffer
                                                  && m_isSelected;
                                            }

  /* Methods *****************************************************************/
  void Flush(HDC hdc)                       { Flush(hdc, 0, 0, m_bm.bmWidth, m_bm.bmHeight);}
  void Flush(HDC hdc, const RECT& rc)       { Flush(hdc, 
                                                    rc.left, rc.top, 
                                                    rc.right - rc.left, rc.bottom - rc.top);
                                            }
  void Flush( HDC hdc, 
              int x, int y, 
              int cx, int cy)               { ::BitBlt( hdc, x, y, cx, cy,
                                                        m_memDC, 0, 0, SRCCOPY);
                                            }

  HBITMAP BorrowImage()                     { (HBITMAP)::SelectObject(m_memDC, m_hOldBmp);
                                              m_isSelected = false;
                                              return m_buffer;
                                            }
  void    ReturnImage()                     { ::SelectObject(m_memDC, m_buffer);
                                              m_isSelected = true;
                                            }

private:
  /* Members *****************************************************************/
  HDC               m_memDC;
  AutoObj<HBITMAP>  m_buffer;
  BITMAP            m_bm;
  HBITMAP           m_hOldBmp;
  bool              m_isSelected;

  /* Methods *****************************************************************/
  HBITMAP BufferInit_(HDC hdc, int cx=0, int cy=0)
  { 
    if ( 0 == cx 
      || 0 == cy)
    {
      AutoObj<HBITMAP> tempBmp(::CreateCompatibleBitmap(hdc, 1, 1));
      // Attempt to create a duplicate buffer assuming
      // the specified hdc is a memory DC.
      m_hOldBmp = (HBITMAP)::SelectObject(hdc, tempBmp);
      if (NULL == m_hOldBmp)
      {
        // This is not a memory DC.
        // Double buffering will not be performed.
        m_memDC = hdc;
        m_isSelected = false;
        return NULL;    
      }

      // Get the dimensions of the existing bitmap.
      ::GetObject(m_hOldBmp, sizeof(BITMAP), &m_bm);
      cx = m_bm.bmWidth;
      cy = m_bm.bmHeight;
    }

    m_memDC = ::CreateCompatibleDC(hdc);
    HBITMAP hBitmapBuffer = ::CreateCompatibleBitmap(hdc, cx, cy);
    ::GetObject(hBitmapBuffer, sizeof(BITMAP), &m_bm);
    m_hOldBmp = (HBITMAP)::SelectObject(m_memDC, hBitmapBuffer);
    m_isSelected = true;

    return hBitmapBuffer;
  }

  /* Prohibited Access functions *********************************************/
  MemDCBuffer();
  MemDCBuffer(const MemDCBuffer&);
  MemDCBuffer& operator=(const MemDCBuffer&);
};

/******************************************************************************
Date:       8/27/2011
Purpose:    Makes a copy of the specified bitmap which will be compatible with
            the supplied DC.
Parameters: hdc[in]: DC to make the new bitmap compatible with.
            hBmp[in]: BITMAP handle, must not be selected into any memDC.
Return:     A new bitmap handle that contains a copy of hBmp.
            The caller is responsible to call ::DeleteObject on this handle.
*******************************************************************************/
inline
HBITMAP CopyBitmap(HDC hdc, HBITMAP hBmp)
{
  HDC memInDC  = ::CreateCompatibleDC(hdc);
  HDC memOutDC = ::CreateCompatibleDC(hdc);

  BITMAP bm;
  ::GetObject(hBmp, sizeof(BITMAP), &bm);
  HBITMAP hOutputBmp = ::CreateCompatibleBitmap(hdc, bm.bmWidth, bm.bmHeight);

  ::SelectObject(memInDC, hBmp);
  ::SelectObject(memOutDC, hOutputBmp);

  ::BitBlt(memOutDC, 0, 0, bm.bmWidth, bm.bmHeight,
           memInDC, 0, 0, SRCCOPY);

  ::DeleteDC(memInDC);
  ::DeleteDC(memOutDC);

  return hOutputBmp;
}

/* Typedefs ******************************************************************/
typedef AutoObj<HBITMAP>    AutoBitmap;
typedef AutoObj<HBRUSH>     AutoBrush;
typedef AutoObj<HFONT>      AutoFont;
typedef AutoObj<HPEN>       AutoPen;
typedef AutoObj<HRGN>       AutoRgn;
typedef AutoObj<HPALETTE>   AutoPalette;

} // namespace article

#endif
