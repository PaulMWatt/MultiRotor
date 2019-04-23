/* BitBlender.cpp *************************************************************
Author:    Paul Watt
Date:      7/21/2011 10:35:29 PM
Purpose:   
Copyright 2011 Paul Watt
*******************************************************************************/
#include "../stdafx.h"
#include <cmath>

#include "BitBlender.h"
#include "AutoGdi.h"

/* Forward Declarations ******************************************************/
namespace // anonymous
{

/* Forward Declarations ******************************************************/
bool AngularGradient_(HDC,
                      const RECT&,
                      double,
                      COLORREF,
                      COLORREF,
                      BYTE,
                      BYTE
                      );

bool SegmentedRadialGradient_(HDC hdc, 
                              const POINT& ctr, 
                              int radius,
                              double startAngle,
                              COLORREF c1,
                              COLORREF c2,
                              size_t segments,
                              BYTE alpha1,
                              BYTE alpha2
                              );

} // namespace anonymous


namespace article
{

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
            alpha1[in]: Starting alpha level to associate with the gradient.
            alpha2[in]: Ending alpha level to associate with the gradient.

Return:     true  If the function succeeds
            false If an error occurs and the function fails.
*******************************************************************************/
bool RectGradient(
  HDC hDC,
  const RECT &rc, 
  COLORREF c1, 
  COLORREF c2,
  BOOL isVertical,
  BYTE alpha1,
  BYTE alpha2
  )
{
  TRIVERTEX v[2] =
  {
    {rc.left,  rc.top,    RVal16(c1), GVal16(c1), BVal16(c1), ToColor16(alpha1)},  
    {rc.right, rc.bottom, RVal16(c2), GVal16(c2), BVal16(c2), ToColor16(alpha2)}
  };

  GRADIENT_RECT topGradient;
  topGradient.UpperLeft = 0;
  topGradient.LowerRight= 1;

  BOOL result = ::GdiGradientFill(hDC, 
                                  v, 
                                  2, 
                                  &topGradient, 
                                  1, 
                                  isVertical
                                  ? GRADIENT_FILL_RECT_V
                                  : GRADIENT_FILL_RECT_H);

  return FALSE != result;
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
            alpha1[in]: Starting alpha level to associate with the gradient.
            alpha2[in]: Ending alpha level to associate with the gradient.
Return:     true  If the function succeeds
            false If an error occurs and the function fails.
*******************************************************************************/
bool RadialGradient(
  HDC hdc, 
  int x, 
  int y, 
  int r,
  COLORREF c1,
  COLORREF c2,
  size_t segments,
  BYTE alpha1,
  BYTE alpha2
  )
{
  POINT pt = {x,y};
  return SegmentedRadialGradient_(hdc, pt, r, 0.0, c1, c2, segments, alpha1, alpha2);
}

/******************************************************************************
Date:       7/25/2011
Purpose:    Creates a linear gradient fill directed along an arbitrary angle.
Parameters: hDC[in]:  The device context to write to.
            rect[in]: The rectangle coordinates to fill with the gradient.
            angle[in]: The angle in radians 
            c1[in]: The color to use at the start of the gradient.
            c2[in]: The color to use at the end of the gradient.
            alpha1[in]: Starting alpha level to associate with the gradient.
            alpha2[in]: Ending alpha level to associate with the gradient.
Return:     true  If the function succeeds
            false If an error occurs and the function fails.
*******************************************************************************/
bool AngularGradient(
  HDC hdc,
  const RECT &rc,
  double angle,
  COLORREF c1,
  COLORREF c2,
  BYTE alpha1,
  BYTE alpha2
  )
{
  // Immediately test for the axis aligned angles, and call the simpler version.
  if ( 0      == angle
    || k_pi_2 == angle
    || k_pi   == angle
    || k_3pi_2== angle)
  {
    // Determine the direction of the gradient.
    bool isVertical = !(0 == angle || k_pi == angle);
    // The colors will need to be swapped for the 2nd half of the circle.
    if (angle >= k_pi)
    {
      std::swap(c1, c2);
      std::swap(alpha1, alpha2);
    }

    return RectGradient(hdc, rc, c1, c2, isVertical, alpha1, alpha2);
  }

  return AngularGradient_(hdc, rc, angle, c1, c2, alpha1, alpha2);
}

/******************************************************************************
Date:       8/27/2011
Purpose:    Combines two images together. The source image will be used as is, 
            and the red channel from the alpha image will be blended into the 
            alpha channel of the src image.
Parameters: hdc[in]:  DC which the bitmaps are compatible with.
            hImg[in/out]: Image to receive the alpha channel.
            hAlpha[in]: Image to create an alpha channel from.
*******************************************************************************/
bool CombineAlphaChannel(HDC hdc, HBITMAP hImg, HBITMAP hAlpha)
{
  // Get Bitmap info.
  BITMAP bmImg;
  BITMAP bmAlpha;
  ::GetObject(hImg,   sizeof(BITMAP), &bmImg);
  ::GetObject(hAlpha, sizeof(BITMAP), &bmAlpha);

  RECT rc = {0,0,bmAlpha.bmWidth,bmAlpha.bmHeight};

  // Create a working bitmap buffer.
  MemDCBuffer imageDC(hdc, bmAlpha.bmWidth, bmAlpha.bmHeight);

  // Create a memory DC to manipulate the input bitmaps.
  HDC memDC = ::CreateCompatibleDC(hdc);
  ::SelectObject(memDC, hAlpha);
  
  // Remove the Green and Blue channels from the alpha image.
  ::FillRect(imageDC, &rc, AutoBrush(::CreateSolidBrush(RGB(0xFF,0,0))));
  ::BitBlt(imageDC, 0, 0, bmAlpha.bmWidth, bmAlpha.bmHeight,
            memDC, 0, 0, SRCAND);

  // Shift the channel.
  ShiftColorChannelsLeft(hdc, imageDC.BorrowImage());
  // Return the buffer to the working buffer.
  imageDC.ReturnImage();

  // Add the alpha channel to the input image.
  ::SelectObject(memDC, hImg);
  ::BitBlt( memDC, 0, 0, bmImg.bmWidth, bmImg.bmHeight,
            imageDC, 0, 0, SRCPAINT);

  ::DeleteDC(memDC);

  return true;
}

} // namespace article

/* Local Declarations ********************************************************/
namespace // anonymous
{

/******************************************************************************
Date:       8/10/2011
Purpose:    Creates a linear gradient fill directed along an arbitrary angle
            inside of any axis aligned rectangle.
Parameters: hDC[in]:  The device context to write to.
            rect[in]: The rectangle coordinates to fill with the gradient.
            angle[in]: The angle in radians 
            c1[in]: The color to use at the start of the gradient.
            c2[in]: The color to use at the end of the gradient.
            alpha1[in]: Starting alpha level to associate with the gradient.
            alpha2[in]: Ending alpha level to associate with the gradient.
Return:     true  If the function succeeds
            false If an error occurs and the function fails.
*******************************************************************************/
bool AngularGradient_(
  HDC hdc,
  const RECT& rc,
  double angle,
  COLORREF c1,
  COLORREF c2,
  BYTE alpha1,
  BYTE alpha2
  )
{
  using namespace article;

  SIZE sz = article::GetRectSize(rc);
  // The quadrant and rotation offset are used to make corrections for (+/-).
  double quad = (angle / (k_pi / 2));
  int offset = int(ceil(quad) - 1);

  // Determine a ratio of intersection points for 
  // the LL, and UR corners of the provided rectangle.  
  // This will allow the other colors to be derived.
  double cosTheta = sz.cx * cos(angle);
  double sinTheta = sz.cy * sin(angle);

  double len = fabs(cosTheta) + fabs(sinTheta);
  double r1  = fabs(cosTheta / len);
  double r2  = fabs(sinTheta / len);

  // Derive 4 color points from the original 2 colors, and the provided angle.
  // Calculate the total color distance between c1 and c2 for all channels.
  int rDiff = 0;
  int gDiff = 0;
  int bDiff = 0;

  article::GetColorDiff(c1, c2, rDiff, gDiff, bDiff);
  COLOR16 alphaDiff = ToColor16(alpha2 - alpha1);

  // Calculate the other colors by multiplying the color diff with each ratio.
  // There may be two or four colors depending on the type of shape.
  COLORREF cC[2];
  USHORT   alpha[2];
  cC[1] = RGB(GetRValue(c1) + (rDiff * r1),
              GetGValue(c1) + (gDiff * r1),
              GetBValue(c1) + (bDiff * r1));
  cC[0] = RGB(GetRValue(c1) + (rDiff * r2),
              GetGValue(c1) + (gDiff * r2),
              GetBValue(c1) + (bDiff * r2));

  // Perform the same set steps for the alpha channel
  // until each vertex is defined.
  alpha[0] = alpha1 + USHORT(alphaDiff * r1);
  alpha[1] = alpha1 + USHORT(alphaDiff * r2);

  // As the angle's start point changes quadrants, the colors will need to
  // rotate around the vertices to match the starting position.
  if (0 == (offset % 2))
  {
    std::swap(cC[0], cC[1]);
    std::swap(alpha[0], alpha[1]);
  }

  offset = std::abs(offset - 4) % 4;
  COLORREF clr[4] = { c1, cC[0], c2, cC[1]};
  std::rotate(clr, clr + offset, clr + 4);

  USHORT   alphaV[4] = { alpha1, alpha[0], alpha2, alpha[1]};
  std::rotate(alphaV, alphaV + offset, alphaV + 4);

  // Populate the data points.
  TRIVERTEX corners[4] =
  {
    {rc.left,  rc.top,    RVal16(clr[0]), GVal16(clr[0]), BVal16(clr[0]), alphaV[0]},  
    {rc.right, rc.top,    RVal16(clr[1]), GVal16(clr[1]), BVal16(clr[1]), alphaV[1]},
    {rc.right, rc.bottom, RVal16(clr[2]), GVal16(clr[2]), BVal16(clr[2]), alphaV[2]},
    {rc.left,  rc.bottom, RVal16(clr[3]), GVal16(clr[3]), BVal16(clr[3]), alphaV[3]}
  };

  // Create the mesh definitions for the square with only 2 polygons.
  const GRADIENT_TRIANGLE sqGradient[2] =
  {
    {0, 2, 3},
    {0, 1, 2}
  };

  BOOL result = ::GdiGradientFill(hdc, 
                                  corners, 
                                  4, 
                                  (PVOID)sqGradient, 
                                  2, 
                                  GRADIENT_FILL_TRIANGLE);

  return FALSE != result;
}

/******************************************************************************
Date:       7/22/2011
Purpose:    Creates a radial gradient.
            The gradient is approximated by breaking up the 
            circle into triangles, and using the Win32 GradientFill call
            to fill all of the individual triangles.
Parameters: hDC[in]:  The device context to write to.
            ctr[in]: The center point of the gradient.
            radius[in]: The radius length of the gradient fill.
            startAngle[in]: An offset angle to start the gradient segmentation.
              This angle is defined in radians.
            c1[in]: The color to use at the start of the gradient.
            c2[in]: The color to use at the end of the gradient.
            segments[in]: The number of segments to break the circle into.
              The default number of segments is 16.  
              3 is the absolute minimum, which will result in a triangle.
            alpha1[in]: Starting alpha level to associate with the gradient.
            alpha2[in]: Ending alpha level to associate with the gradient.
Return:     true  If the function succeeds
            false If an error occurs and the function fails.
*******************************************************************************/
bool SegmentedRadialGradient_(
  HDC hdc, 
  const POINT& ctr, 
  int radius,
  double startAngle,
  COLORREF c1,
  COLORREF c2,
  size_t segments,
  BYTE alpha1,
  BYTE alpha2
  )
{
  // Less than 3 segments cannot create anything useful with this algorithm.
  if (segments < 3)
  {
    return false;
  }

  // Allocate space for both the vertex and mesh definitions.
  const size_t      k_vertexCount = segments + 2;
  TRIVERTEX         *pVertex = new TRIVERTEX[k_vertexCount];
  GRADIENT_TRIANGLE *pMesh   = new GRADIENT_TRIANGLE[segments];

  // Populate the first point as the center point of the radial gradient.
  // This point will appear in every triangle mesh that is rendered.
  pVertex[0].x      = ctr.x;
  pVertex[0].y      = ctr.y;
  pVertex[0].Red    = article::RVal16(c1); 
  pVertex[0].Green  = article::GVal16(c1); 
  pVertex[0].Blue   = article::BVal16(c1);
  pVertex[0].Alpha  = article::ToColor16(alpha1);

  // Pre-calculate the radial offset around the circle for each segment.
  double segOffset = 1.0 / segments;
  double curOffset = startAngle + segOffset;

  // Populate each vertex and mesh segment.
  for (size_t index = 1; index < k_vertexCount; ++index)
  {
    // Populate the current vertex
    double angle = article::k_2pi * curOffset;
    pVertex[index].x      = ctr.x + static_cast<LONG>(radius * cos(angle));
    pVertex[index].y      = ctr.y + static_cast<LONG>(radius * sin(angle));
    pVertex[index].Red    = article::RVal16(c2); 
    pVertex[index].Green  = article::GVal16(c2); 
    pVertex[index].Blue   = article::BVal16(c2);
    pVertex[index].Alpha  = article::ToColor16(alpha2);

    // Update the next segment;
    curOffset += segOffset;
  }

  // Populate the set of mesh segments.
  for (size_t meshIndex = 0; meshIndex < segments; ++meshIndex)
  {
    pMesh[meshIndex].Vertex1 = 0; // This will be the first vertex for every triangle.
    pMesh[meshIndex].Vertex2 = meshIndex + 1; 
    pMesh[meshIndex].Vertex3 = meshIndex + 2; 
  }

  // Self correction from the loop assignment.  
  // This will correctly set the last vertex to be within a valid range.
  // The last vertex, in the last mesh, will share a vertex with the first mesh.
  // The actual vertex that is shared:
  //    pMesh[0].Vertex2
  pMesh[segments - 1].Vertex3 = 1;

  // All of that setup, and only one call to GradientFill is required.
  BOOL retVal = ::GdiGradientFill(hdc, 
                                  pVertex,
                                  k_vertexCount,
                                  pMesh,
                                  segments,
                                  GRADIENT_FILL_TRIANGLE);

  // Release resources.
  delete[] pMesh;
  pMesh = NULL;

  delete[] pVertex;
  pVertex = NULL;

  return FALSE != retVal;
}

} // namespace anonymous

