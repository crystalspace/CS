/*
    Crystal Space Windowing System: CSWS palette definition
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "sysdef.h"
#include "csws/cswspal.h"
#include "csws/cscomp.h"

// Application class default palette
static int palette_csApp[] =
{
  cs_Color_Gray_D			// Application workspace
};

// Button class default palette
static int palette_csButton[] =
{
  cs_Color_Gray_L,			// button background
  cs_Color_Black,			// default button frame
  cs_Color_Gray_D,			// 3D border dark
  cs_Color_White,			// 3D border light
  cs_Color_Black,			// button text
  cs_Color_Gray_D			// button disabled text
};

// Dialog class default palette
static int palette_csDialog[] =
{
  cs_Color_Gray_L,			// dialog background
  cs_Color_Gray_D,			// 3D border dark
  cs_Color_White,			// 3D border light
  cs_Color_Black,			// 2nd level 3D border dark
  cs_Color_Gray_D			// 2nd level 3D border light
};

// Input line class default palette
static int palette_csInputLine[] =
{
  cs_Color_Gray_L,			// input line background
  cs_Color_White,			// background for csifsThickRect
  cs_Color_Gray_D,			// 3D border dark
  cs_Color_White,			// 3D border light
  cs_Color_Black,			// 2nd level 3D border dark
  cs_Color_Gray_D,			// 2nd level 3D border light
  cs_Color_Black,			// input line text
  cs_Color_Cyan_D,			// input line selection background
  cs_Color_White			// input line selected text
};

// Menu item class default palette
static int palette_csMenuItem[] =
{
  cs_Color_Gray_L,			// Menu item unselected background
  cs_Color_Cyan_D,			// Selection bar background
  cs_Color_Black,			// Unselected text
  cs_Color_White,			// Selected text
  cs_Color_Gray_D,			// Disabled text
  cs_Color_Gray_D,			// 3D separator dark
  cs_Color_White,			// 3D separator light
};

// Menu class default palette
static int palette_csMenu[] =
{
  cs_Color_Gray_L,			// Menu background
  cs_Color_Gray_D,			// 3D border dark
  cs_Color_White,			// 3D border light
  cs_Color_Black,			// 3D border 2nd level dark
  cs_Color_Gray_D,			// 3D border 2nd level light
};

// Static class default palette
static int palette_csStatic[] =
{
  cs_Color_Gray_D,			// static component background
  cs_Color_Cyan_D,			// static component text / inactive
  cs_Color_Cyan_M,			// static component text / active
  cs_Color_Gray_D,			// 3D border dark
  cs_Color_White			// 3D border light
};

// Title bar class default palette
static int palette_csTitleBar[] =
{
  cs_Color_Gray_D,			// 3D border dark
  cs_Color_White,			// 3D border light
  cs_Color_Cyan_D,			// passive title background
  cs_Color_Gray_M,			// passive title text
  cs_Color_Black,			// passive title 3D bars dark
  cs_Color_Cyan_M,			// passive title 3D bars light
  cs_Color_Cyan_M,			// active title background
  cs_Color_White,			// active title text
  cs_Color_Cyan_D,			// active title 3D bars dark
  cs_Color_Cyan_L			// active title 3D bars light
};

// Window class default palette
static int palette_csWindow[] =
{
  cs_Color_Gray_D,			// 3D border dark
  cs_Color_White,			// 3D border light
  cs_Color_Black,			// 3D border 2nd level dark
  cs_Color_Gray_D,			// 3D border 2nd level light
  cs_Color_Gray_L,			// border normal
};

// List box item class default palette
static int palette_csListBoxItem[] =
{
  cs_Color_Gray_L,			// Unselected listbox item background
  cs_Color_Cyan_D,			// Selected listbox item background
  cs_Color_Cyan_L,			// Thin rectangle around selected item
  cs_Color_Gray_D,			// Disabled text
  cs_Color_Black,			// Unselected normal text
  cs_Color_White,			// Selected normal text
  cs_Color_Cyan_D,			// Unselected emphasized text
  cs_Color_Cyan_L			// Selected emphasized text
};

// Scroll bar class default palette
static int palette_csScrollBar[] =
{
  cs_Color_Gray_L,			// Scroll bar background
  cs_Color_Gray_M,			// Scroll bar selected background
  cs_Color_Gray_D,			// 3D border dark
  cs_Color_White			// 3D border light
};

// Pointers to all standard palettes
csPaletteExport cswsPalette[] =
{
  { palette_csApp,         sizeof (palette_csApp)         / sizeof (int) },
  { palette_csButton,      sizeof (palette_csButton)      / sizeof (int) },
  { palette_csDialog,      sizeof (palette_csDialog)      / sizeof (int) },
  { palette_csInputLine,   sizeof (palette_csInputLine)   / sizeof (int) },
  { palette_csMenuItem,    sizeof (palette_csMenuItem)    / sizeof (int) },
  { palette_csMenu,        sizeof (palette_csMenu)        / sizeof (int) },
  { palette_csStatic,      sizeof (palette_csStatic)      / sizeof (int) },
  { palette_csTitleBar,    sizeof (palette_csTitleBar)    / sizeof (int) },
  { palette_csWindow,      sizeof (palette_csWindow)      / sizeof (int) },
  { palette_csListBoxItem, sizeof (palette_csListBoxItem) / sizeof (int) },
  { palette_csScrollBar,   sizeof (palette_csScrollBar)   / sizeof (int) }
};
