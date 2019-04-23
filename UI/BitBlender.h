/* BitBlender.h ***************************************************************
Author      Paul Watt
Date:       7/21/2011
Purpose:    Bit Blender is a collection of helper functions that simplifies the 
            use of GradientFill and AlphaBlend.  These functions also extend
            the capabilities of the original API to allow radial gradients, 
            arbitrary gradients at an angle, and alpha-blend support.

Copyright 2011 Paul Watt
******************************************************************************/
#ifndef BITBLENDER_H_INCLUDED
#define BITBLENDER_H_INCLUDED

/* Includes ******************************************************************/
#include <windows.h>
#include <algorithm>
#include <vector>

namespace article
{                     

/* Constants *****************************************************************/
/* Constants *****************************************************************/
const double k_pi         = 3.1415926535897932384626433832795;
const double k_pi_2       = 0.5 * k_pi;
const double k_pi_4       = 0.25* k_pi;
const double k_3pi_2      = 1.5 * k_pi;
const double k_2pi        = 2.0 * k_pi;

const double k_degToRad   = k_pi / 180.0;
const double k_RadToDeg   = 180.0 / k_pi;


// These constants are common levels of translucency.
const BYTE k_opaque       = 0xFF;
const BYTE k_transparent  = 0x00;

const BYTE k_alpha100     = k_opaque;       //100%, 0xFF == 255
const BYTE k_alpha80      = 0xCC;           // 80%, 0xCC == 204
const BYTE k_alpha75      = 0xC0;           // 75%, 0xC0 == 192
const BYTE k_alpha60      = 0x99;           // 60%, 0x99 == 153
const BYTE k_alpha50      = 0x80;           // 50%, 0x80 == 128
const BYTE k_alpha40      = 0x66;           // 40%, 0x66 == 102
const BYTE k_alpha25      = 0x40;           // 25%, 0x40 == 64
const BYTE k_alpha20      = 0x33;           // 20%, 0x33 == 51
const BYTE k_alpha0       = k_transparent;  //  0%, 0x00 == 0

/* Utilities *****************************************************************/
inline COLOR16 ToColor16(BYTE byte)   { return byte << 8;}
inline COLOR16 RVal16(COLORREF color) { return ToColor16(GetRValue(color));}
inline COLOR16 GVal16(COLORREF color) { return ToColor16(GetGValue(color));}
inline COLOR16 BVal16(COLORREF color) { return ToColor16(GetBValue(color));}


/* Functions *****************************************************************/
bool RectGradient(HDC,const RECT&,COLORREF,COLORREF,BOOL,BYTE,BYTE);
bool RectGradient(HDC,const RECT&,COLORREF,COLORREF,BOOL);
bool RadialGradient(HDC,int,int,int,COLORREF,COLORREF,size_t,BYTE,BYTE);
bool RadialGradient(HDC,int,int,int,COLORREF,COLORREF,size_t);
bool AngularGradient(HDC,const RECT&,double,COLORREF,COLORREF,BYTE,BYTE);
bool AngularGradient(HDC,const RECT&,double,COLORREF,COLORREF);

bool CombineAlphaChannel(HDC, HBITMAP, HBITMAP);

void GetColorDiff(COLORREF, COLORREF, int&, int&, int&);

BLENDFUNCTION GetBlendFn(BYTE, bool);

/* Bit Manipulation Functions ************************************************/
bool PreBlendAlphaBitmap(HDC, HBITMAP);
bool ShiftColorChannelsLeft (HDC,HBITMAP);
bool ShiftColorChannelsRight(HDC,HBITMAP);
bool InvertBitmap(HDC, HBITMAP);
bool ColorToGrayscale(HDC, HBITMAP);
bool GrayscaleToColor(HDC, HBITMAP, COLORREF);

/******************************************************************************
Date:       8/26/2011
Purpose:    Template that will extract the pixels from a bitmap into a DIB, and
            call a specified function on each bit.
            The bitmap manipulated with this function must be 32-bit color depth.
Parameters: hdc[in]: DC to be compatible with.
            hBmp[in]: Bitmap to shift the color channels for.
            fnT[in]: Functor that will modify a 32-bit number however is desired.

*******************************************************************************/
template <typename fnT>
bool ManipulateDIBits(HDC hdc, HBITMAP hBmp, fnT fn)
{
  // Attempt to extract the BITMAP from the current DC.
  BITMAP bm;
  ::GetObject(hBmp, sizeof(BITMAP), &bm);

  // zero the memory for the bitmap info 
  BITMAPINFO bmi;
  ZeroMemory(&bmi, sizeof(BITMAPINFO));

  // setup bitmap info  
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = bm.bmWidth;
  bmi.bmiHeader.biHeight = bm.bmHeight;
  bmi.bmiHeader.biPlanes = bm.bmPlanes;
  bmi.bmiHeader.biBitCount = bm.bmBitsPixel;
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biSizeImage = bm.bmWidth * bm.bmHeight * 4;

  std::vector<UINT32> bits;
  bits.resize(bm.bmWidth * bm.bmHeight, 0x0);
  if (!::GetDIBits(hdc, hBmp, 0, bm.bmHeight, (void**)&bits[0], &bmi, DIB_RGB_COLORS))
    return false;

  // Perform the manipulation.
  std::for_each(bits.begin(), bits.end(), fn);

  // Transfer the data back to the input device.
  return 0
      != ::SetDIBits(hdc, hBmp, 0, bm.bmHeight, (void**)&bits[0], &bmi, DIB_RGB_COLORS);
}

/* Inline Function Implementations *******************************************/
/******************************************************************************
Date:       7/21/2011
Purpose:    Helper function to simplify defining and drawing a gradient fill
            in a rectangular area.  This function will create all of the 
            internal structures for each call to GradientFill.  
            If the same gradient definition will be used repeatedly, !!! will
            provide better performance.
Parameters: hDC[in]:  The device context to write to.
            rect[in]: The rectangle coordinates to fill with the gradient.
            c1[in]: The color to use at the start of the gradient.
            c2[in]: The color to use at the end of the gradient.
            isVertical[in]: Indicates if the gradient fill should transition
              vertically in the rectangle.  If this value is false, horizontal
              will be used.                

              All fills will be defined from left to right, or top to bottom.
Return:     true  If the function succeeds
            false If an error occurs and the function fails.
*******************************************************************************/
inline 
bool RectGradient(HDC hDC,
                  const RECT &rc, 
                  COLORREF c1, 
                  COLORREF c2,
                  BOOL isVertical)
{
  return RectGradient(hDC, rc, c1, c2, isVertical, k_opaque, k_opaque);
}

/******************************************************************************
Date:       7/22/2011
Purpose:    Creates a radial gradient.
            The gradient is approximated by breaking up the 
            circle into triangles, and using the Win32 GradientFill call
            to fill all of the individual triangles.
Parameters: hDC[in]:  The device context to write to.
            x[in]: The x coordinate of the center of the gradient.
            y[in]: The y coordinate of the center of the gradient.
            r[in]: The radius of the gradient fill.
            c1[in]: The color to use at the start of the gradient.
            c2[in]: The color to use at the end of the gradient.
            segments[in]: The number of segments to break the circle into.
              The default number of segments is 16.  
              3 is the absolute minimum, which will result in a triangle.
Return:     true  If the function succeeds
            false If an error occurs and the function fails.
*******************************************************************************/
inline 
bool RadialGradient(HDC hdc, 
                    int x, 
                    int y, 
                    int r,
                    COLORREF c1,
                    COLORREF c2,
                    size_t segments
                    )
{
  return RadialGradient(hdc, x, y, r, c1, c2, segments, k_opaque, k_opaque);
}

/******************************************************************************
Date:       7/25/2011
Purpose:    Creates a linear gradient fill directed along an arbitrary angle.
Parameters: hDC[in]:  The device context to write to.
            rect[in]: The rectangle coordinates to fill with the gradient.
            angle[in]: The angle in radians 
            c1[in]: The color to use at the start of the gradient.
            c2[in]: The color to use at the end of the gradient.
Return:     true  If the function succeeds
            false If an error occurs and the function fails.
*******************************************************************************/
inline 
bool AngularGradient(
  HDC hdc,
  const RECT &rc,
  double angle,
  COLORREF c1,
  COLORREF c2
  )
{
  return AngularGradient(hdc, rc, angle, c1, c2, k_opaque, k_opaque);
}

/******************************************************************************
Date:       7/25/2011
Purpose:    Calculates the difference in color between the two colors.
            The difference is calculated individually for each color channel.
Parameters: c1[in]: Start color
            c2[in]: End color
Return:     The difference in each color is returned.
*******************************************************************************/
inline
void GetColorDiff(COLORREF c1, COLORREF c2, int &red, int &green, int &blue)
{
  // Not sure what negative colors will do yet.  
  // Keep a higher level of accuracy until the results are known.
  red    = GetRValue(c2) - GetRValue(c1);
  green  = GetGValue(c2) - GetGValue(c1);
  blue   = GetBValue(c2) - GetBValue(c1);
}

/******************************************************************************
Date:       8/7/2011
Purpose:    Helper function to easily initialize a blend function for alpha blends.
Parameters: globalAlpha[in]: Indicates a global alpha constant, 0-255.
            hasSrcAlpha[in]: True indicates the source image has alpha info,
              this will set the AC_SRC_ALPHA flag.
              Otherwise false will clear this flag.
Return:    An initialized BLENDFUNCTION object is returned for use in GdiAlphaBlend.
*******************************************************************************/
inline 
BLENDFUNCTION GetBlendFn(BYTE globalAlpha, bool hasSrcAlpha)
{
  BLENDFUNCTION bf = {AC_SRC_OVER, BYTE(0), globalAlpha, hasSrcAlpha ? AC_SRC_ALPHA : BYTE(0)};
  return bf;
}

/* Functors ******************************************************************/
/* Class **********************************************************************
Purpose:  Functor to multiply individual channels with a specified alpha value.
*******************************************************************************/
struct MultiplyAlpha
{
  void operator()(UINT32 &elt) const
  {
    UINT32 val = elt;
    BYTE alpha = (BYTE)(val >> 24);
    double factor = alpha / 255.0;
    elt = ((alpha) << 24)
      | (BYTE(GetRValue(elt) * factor) ) 
      | (BYTE(GetGValue(elt) * factor) << 8)
      | (BYTE(GetBValue(elt) * factor) << 16);
  }
};

/* Class **********************************************************************
Purpose:  Functor to shift all color channels left.
*******************************************************************************/
struct ShiftLeft
{
  void operator()(UINT32 &elt) const
  {
    elt <<= 8;
  }
};

/* Class **********************************************************************
Purpose:  Functor to shift all color channels right.
*******************************************************************************/
struct ShiftRight
{
  void operator()(UINT32 &elt) const
  {
    elt >>= 8;
  }
};

/* Class **********************************************************************
Purpose:  Functor to invert the bits for the current pixel.
*******************************************************************************/
struct Invert
{
  void operator()(UINT32 &elt) const
  {
    elt = ~elt;
  }
};

/* Class **********************************************************************
Purpose:  Functor to convert a color image into grayscale.
          This grayscale conversion is based on relative color intensity to 
          create a more natural looking image compared to the original.
              Intensity = (Red    * 0.299) 
                        + (Green  * 0.587) 
                        + (Blue   * 0.114)
*******************************************************************************/
struct ToGrayscale
{
  void operator()(UINT32 &elt) const
  {
    // Leave the alpha channel unchanged.
    BYTE intensity = BYTE( (GetRValue(elt) * 0.299)
                         + (GetGValue(elt) * 0.587)
                         + (GetBValue(elt) * 0.114) * 255);
    elt = (elt & 0xFF000000)
        | intensity << 16
        | intensity << 8
        | intensity;
  }
};

/* Class **********************************************************************
Purpose:  Functor to convert a grayscale image to an image with 256 levels of 
          a particular color.
*******************************************************************************/
struct Colorize
{
  Colorize(COLORREF color)
  {
    rVal = GetRValue(color);
    gVal = GetGValue(color);
    bVal = GetBValue(color);
  }  

  void operator()(UINT32 &elt) const
  {
    // Leave the alpha channel unchanged.
    // Get the intensity level for only one of the channels.
    // If a proper grayscale image were passed in, R = G = B.
    float intensity = GetRValue(elt) / 255.0f;
    elt = (elt & 0xFF000000)
        | ((BYTE(intensity * rVal)) << 16) 
        | ((BYTE(intensity * gVal)) << 8)
        |  (BYTE(intensity * bVal));
    //elt = (elt & 0x000000FF)
    //    | ((BYTE(intensity * rVal)) << 16) 
    //    | ((BYTE(intensity * gVal)) << 8)
    //    |  (BYTE(intensity * bVal));
  }

  BYTE rVal;
  BYTE gVal;
  BYTE bVal;
};

/******************************************************************************
Date:       8/5/2011
Purpose:    Pre-multiplies the alpha channel for each pixel in the bitmap.
Parameters: hdc[in]: A DC to the device the bitmap is compatible with.
            hBmp[in]: Handle to the BITMAP to have its alpha channel pre-blended.
              This handle cannot be selected into a DC when it is passed into
              this function.
Return:     true  The function was successful, the pre-blend completed.
            false The function failed, the bitmap is unchanged.
*******************************************************************************/
inline
bool PreBlendAlphaBitmap(HDC hdc, HBITMAP hBmp)
{
  return ManipulateDIBits(hdc, hBmp, MultiplyAlpha());
}

/******************************************************************************
Date:       8/26/2011
Purpose:    Shifts all of the color channels to the left one channel.
            The image passed in must be a 32-bit color bitmap.
Parameters: hdc[in]: DC to be compatible with.
            hBmp[in]: Bitmap to shift the color channels for.
Return:     true  The function was successful, the channels shifted left.
            false The function failed, the bitmap is unchanged.
*******************************************************************************/
inline
bool ShiftColorChannelsLeft(HDC hdc,HBITMAP hBmp)
{
  return ManipulateDIBits(hdc, hBmp, ShiftLeft());
}

/******************************************************************************
Date:       8/26/2011
Purpose:    Shifts all of the color channels to the right one channel.
            The image passed in must be a 32-bit color bitmap.
Parameters: hdc[in]: DC to be compatible with.
            hBmp[in]: Bitmap to shift the color channels for.
*******************************************************************************/
inline
bool ShiftColorChannelsRight(HDC hdc,HBITMAP hBmp)
{
  return ManipulateDIBits(hdc, hBmp, ShiftRight());
}

/******************************************************************************
Date:       8/26/2011
Purpose:    Inverts all of the colors of an image.  Every bit is flipped.
Parameters: hdc[in]: DC to be compatible with.
            hBmp[in]: Bitmap to shift the color channels for.
*******************************************************************************/
inline 
bool InvertBitmap(HDC hdc, HBITMAP hBmp)
{
  // The functor exists to perform the operation this way.
  // return ManipulateDIBits(hdc, hBmp, Invert());

  // However, this ROP3 method in BitBlt is implemented in hardware:
  BITMAP bm;
  ::GetObject(hBmp, sizeof(BITMAP), &bm);

  HDC memDC = ::CreateCompatibleDC(hdc);
  ::SelectObject(memDC, hBmp);
  ::BitBlt(memDC, 0, 0, bm.bmWidth, bm.bmHeight,
           0, 0, 0, DSTINVERT);
  ::DeleteDC(memDC);
}

/******************************************************************************
Date:       8/26/2011
Purpose:    Converts the input color image to a grayscale image calculated
            from color intensity.
Parameters: hdc[in]: DC to be compatible with.
            hBmp[in]: Bitmap to convert to grayscale.
*******************************************************************************/
inline
bool ColorToGrayscale(HDC hdc,HBITMAP hBmp)
{
  return ManipulateDIBits(hdc, hBmp, ToGrayscale());
}

/******************************************************************************
Date:       8/27/2011
Purpose:    Converts a grayscale image into an image with color.
            This function is not symmetric with the ColorToGrayscale function.
            It does not add color based on an intensity calculation, rather it
            applies the specified color scaled to the current gray pixel.

            The image will still appear monochromatic, except with a shade of color.
Parameters: hdc[in]: DC to be compatible with.
            hBmp[in]: Bitmap to convert to grayscale.
            color[in]: The color to convert the image to.
*******************************************************************************/
inline
bool GrayscaleToColor(HDC hdc, HBITMAP hBmp, COLORREF color)
{
  Colorize colorize(color);
  return ManipulateDIBits(hdc, hBmp, colorize);
}

} // namespace article

#endif // BITBLENDER_H_INCLUDED

