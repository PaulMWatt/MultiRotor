#pragma once


#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#define _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX

#include <algorithm>
using std::min;
using std::max;

// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


/* Constants *****************************************************************/
/* Common Color Definitions **************************************************/
const COLORREF k_white          = RGB(0xFF, 0xFF, 0xFF);
const COLORREF k_lightGray      = RGB(0xED, 0xED, 0xED);
const COLORREF k_gray           = RGB(0xC0, 0xC0, 0xC0);
const COLORREF k_darkGray       = RGB(0x80, 0x80, 0x80);
const COLORREF k_black          = RGB(0x00, 0x00, 0x00);
const COLORREF k_cpDarkOrange   = RGB(0xFF, 0x99, 0x00);
const COLORREF k_cpLightOrange  = RGB(0xFF, 0xE2, 0xA8);
const COLORREF k_cpDarkGreen    = RGB(0x48, 0x8E, 0x00);
const COLORREF k_cpLightGreen   = RGB(0x76, 0xAB, 0x40);
const COLORREF k_red2           = RGB(0xFF, 0x1A, 0x00);
const COLORREF k_red            = RGB(0xFF, 0x00, 0x00);
const COLORREF k_green          = RGB(0x00, 0xFF, 0x00);
const COLORREF k_blue           = RGB(0x00, 0x00, 0xFF);
const COLORREF k_lightPurple    = RGB(0x70, 0x70, 0xA0);
const COLORREF k_hilightPurple  = RGB(0x80, 0x80, 0xC0);
const COLORREF k_redBase        = RGB(0xFF, 0x40, 0x60);
const COLORREF k_deepRed        = RGB(0x60, 0x00, 0x00);
const COLORREF k_greenBase      = RGB(0x30, 0xC0, 0x40);
const COLORREF k_deepGreen      = RGB(0x00, 0x60, 0x00);
const COLORREF k_blueBase       = RGB(0x40, 0x80, 0xFF);
const COLORREF k_deepBlue       = RGB(0x00, 0x00, 0x60);

const COLORREF k_blueGlass      = RGB(0x40,0x50,0x70);
const COLORREF k_blueGlassEdge  = RGB(0x30,0x40,0x55);

// Color Palette colors.
const COLORREF k_brightRed      = RGB(237,28,36);
const COLORREF k_brightYellow   = RGB(255,242,0);
const COLORREF k_yellowGreen    = RGB(168,230,29);
const COLORREF k_brightGreen    = RGB(34,217,76);
const COLORREF k_brightCyan     = RGB(100,183,239);
const COLORREF k_indigo         = RGB(47,54,153);
const COLORREF k_violet         = RGB(111,49,152);

const COLORREF k_softRed        = RGB(255,163,177);
const COLORREF k_softOrange     = RGB(245,228,156);
const COLORREF k_softGreen      = RGB(211,249,188);
const COLORREF k_periwinkle     = RGB(112,154,209);
const COLORREF k_lavender       = RGB(0xC0, 0xC0, 0xE0);




