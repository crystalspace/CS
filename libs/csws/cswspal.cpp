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

#include "cssysdef.h"
#include "csqint.h"
#include "csws/cswspal.h"
#include "csws/cscomp.h"
#include "csws/csapp.h"
#include "csws/cswsutil.h"
#include "csws/csskin.h"

// Application class default palette
static int palette_csApp [] =
{
  cs_Color_Gray_D			// Application workspace
};

// Button class default palette
static int palette_csButton [] =
{
  cs_Color_Gray_L,			// button background
  cs_Color_Black,			// default button frame
  cs_Color_Gray_D,			// 3D border dark
  cs_Color_White,			// 3D border light
  cs_Color_Black,			// button text
  cs_Color_Gray_D			// button disabled text
};

// Dialog class default palette
static int palette_csDialog [] =
{
  cs_Color_Gray_L,			// dialog background
  cs_Color_Gray_D,			// 3D border dark
  cs_Color_White,			// 3D border light
  cs_Color_Black,			// 2nd level 3D border dark
  cs_Color_Gray_D			// 2nd level 3D border light
};

// Input line class default palette
static int palette_csInputLine [] =
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
static int palette_csMenuItem [] =
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
static int palette_csMenu [] =
{
  cs_Color_Gray_L,			// Menu background
  cs_Color_Gray_D,			// 3D border dark
  cs_Color_White,			// 3D border light
  cs_Color_Black,			// 3D border 2nd level dark
  cs_Color_Gray_D,			// 3D border 2nd level light
};

// Static class default palette
static int palette_csStatic [] =
{
  cs_Color_Gray_D,			// static component background
  cs_Color_Cyan_D,			// static component text / inactive
  cs_Color_Cyan_M,			// static component text / active
  cs_Color_Gray_D,			// static component text / disabled
  cs_Color_Gray_D,			// 3D border dark
  cs_Color_White			// 3D border light
};

// Title bar class default palette
static int palette_csTitleBar [] =
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
static int palette_csWindow [] =
{
  cs_Color_Gray_D,			// 3D border dark
  cs_Color_White,			// 3D border light
  cs_Color_Black,			// 3D border 2nd level dark
  cs_Color_Gray_D,			// 3D border 2nd level light
  cs_Color_Gray_L,			// border normal
};

// List box class default palette
static int palette_csListBox [] =
{
  cs_Color_Gray_L,			// List box background
  cs_Color_White,			// background for csifsThickRect
  cs_Color_Gray_D,			// 3D border dark
  cs_Color_White,			// 3D border light
  cs_Color_Black,			// 2nd level 3D border dark
  cs_Color_Gray_D,			// 2nd level 3D border light
};

// List box item class default palette
static int palette_csListBoxItem [] =
{
  cs_Color_Gray_L,			// Unselected listbox item background
  cs_Color_Cyan_D,			// Selected listbox item background
  cs_Color_Gray_D,			// Disabled text
  cs_Color_Black,			// Unselected normal text
  cs_Color_White,			// Selected normal text
  cs_Color_Cyan_D,			// Unselected emphasized text
  cs_Color_Cyan_L			// Selected emphasized text
};

// Scroll bar class default palette
static int palette_csScrollBar [] =
{
  cs_Color_Gray_L,			// Scroll bar background
  cs_Color_Gray_M,			// Scroll bar selected background
  cs_Color_Gray_D,			// 3D border dark
  cs_Color_White			// 3D border light
};

// Notebook class default palette
static int palette_csNotebook [] =
{
  cs_Color_Gray_L,			// Notebook background
  cs_Color_Gray_M,			// Unselected tab background
  cs_Color_Black,			// The text on unselected tabs
  cs_Color_Black,			// The most dark color of unselected tab border
  cs_Color_Gray_D,			// The second color of unselected tab border
  cs_Color_Gray_L,			// The third color of unselected tab border
  cs_Color_Gray_L,			// Selected tab background
  cs_Color_Cyan_D,			// The text on selected tab
  cs_Color_Black,			// The most dark color of selected tab border
  cs_Color_Gray_D,			// The second color of selected tab border
  cs_Color_White,			// The third color of selected tab border
  cs_Color_Cyan_D,			// The information text
  cs_Color_Gray_D,			// 3D border dark
  cs_Color_White,			// 3D border light
  cs_Color_Black,			// 3D border very dark
  cs_Color_Gray_M			// 3D border middle light
};

// GridCell class default palette
static int palette_csGridCell [] =
{
  cs_Color_White,			// cell background
  cs_Color_Black,			// cell border foreground color
  cs_Color_White,			// cell border background color
  cs_Color_Black,			// selected cell background
  cs_Color_White,			// selected cell border foreground color
  cs_Color_Black,			// selected cell border background color
  cs_Color_Black,			// Data foreground color
  cs_Color_White,			// Data background color
  cs_Color_White,			// selected Data foreground color
  cs_Color_Black			// selected Data background color
};

// Grid class default palette
static int palette_csGridView [] =
{
  cs_Color_Gray_L,			// Grid background
  cs_Color_Gray_M,			// 3D border dark
  cs_Color_Gray_M,			// 3D border light
  cs_Color_Green_M,			// 3D border dark selected
  cs_Color_Red_L			// 3D border light selected
};

// Splitter class default palette
static int palette_csSplitter [] =
{
  cs_Color_Cyan_M,			// Splitter inactive background
  cs_Color_Cyan_L,			// Splitter inactive light 3D color
  cs_Color_Cyan_D,			// Splitter inactive dark 3D color
  cs_Color_Red_M,			// Splitter active background
  cs_Color_Red_L,			// Splitter active light 3D color
  cs_Color_Red_D,			// Splitter active dark 3D color
};

// Treecontrol class default palette
static int palette_csTreeBox [] =
{
  cs_Color_Gray_L,			// input line background
  cs_Color_White,			// background for csifsThickRect
  cs_Color_Gray_D,			// 3D border dark
  cs_Color_White,			// 3D border light
  cs_Color_Black,			// 2nd level 3D border dark
  cs_Color_Gray_D			// 2nd level 3D border light
};

// Tree item class default palette
static int palette_csTreeItem [] =
{
  cs_Color_Gray_L,			// Unselected tree item background
  cs_Color_Cyan_D,			// Selected tree item background
  cs_Color_Gray_D,			// Disabled text
  cs_Color_Black,			// Unselected normal text
  cs_Color_White,			// Selected normal text
  cs_Color_Cyan_D,			// Unselected emphasized text
  cs_Color_Cyan_L,			// Selected emphasized text
  cs_Color_Red_D			// Lines connecting children
};

// Hint class default palette
static int palette_csHint [] =
{
  cs_Color_Lemon,			// Hint background
  cs_Color_Black, 			// Hint text
  cs_Color_Brown_M  			// Hint border
};

// Pointers to all standard palettes
static csPaletteExport defaultPalette [] =
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
  { palette_csListBox,     sizeof (palette_csListBox)     / sizeof (int) },
  { palette_csListBoxItem, sizeof (palette_csListBoxItem) / sizeof (int) },
  { palette_csScrollBar,   sizeof (palette_csScrollBar)   / sizeof (int) },
  { palette_csNotebook,    sizeof (palette_csNotebook)    / sizeof (int) },
  { palette_csGridCell,    sizeof (palette_csGridCell)    / sizeof (int) },
  { palette_csGridView,    sizeof (palette_csGridView)    / sizeof (int) },
  { palette_csSplitter,    sizeof (palette_csSplitter)    / sizeof (int) },
  { palette_csTreeItem,    sizeof (palette_csTreeItem)    / sizeof (int) },
  { palette_csTreeBox,     sizeof (palette_csTreeBox)     / sizeof (int) },
  { palette_csHint,        sizeof (palette_csHint)        / sizeof (int) }
};

csPaletteExport *cswsPalette = defaultPalette;
int cswsPaletteSize = sizeof (defaultPalette) / sizeof (csPaletteExport);
static int **savedPalette = 0;

int csRegisterPalette (int *Palette, int Size)
{
  int index = cswsPaletteSize++;
  if (cswsPalette == defaultPalette)
  {
    // We're called for the first time, so we have to allocate a new palette
    cswsPalette = (csPaletteExport *)malloc (
      sizeof (csPaletteExport) * cswsPaletteSize);
    memcpy (cswsPalette, &defaultPalette, index * sizeof (csPaletteExport));
  }
  else
    cswsPalette = (csPaletteExport *)realloc (cswsPalette,
      sizeof (csPaletteExport) * cswsPaletteSize);
  cswsPalette [index].Palette = Palette;
  cswsPalette [index].Size = Size;
  return index;
}

void csResetPalette()
{
 if (cswsPalette != defaultPalette)
 {
   free(cswsPalette);
 }
 cswsPalette = defaultPalette;
}

static void csSavePalette ()
{
  if (savedPalette)
    return;
  savedPalette = new int * [cswsPaletteSize];
  int i;
  for (i = 0; i < cswsPaletteSize; i++)
  {
    savedPalette [i] = new int [cswsPalette [i].Size];
    memcpy (savedPalette [i], cswsPalette [i].Palette,
      cswsPalette [i].Size * sizeof (int));
  }
}

static void csFreePalette ()
{
  if (!savedPalette)
    return;
  int i;
  for (i = 0; i < cswsPaletteSize; i++)
    delete [] savedPalette [i];
  delete [] savedPalette;
  savedPalette = 0;
}

static void csRestorePalette ()
{
  if (!savedPalette)
    return;
  int i;
  for (i = 0; i < cswsPaletteSize; i++)
    memcpy (cswsPalette [i].Palette, savedPalette [i],
      cswsPalette [i].Size * sizeof (int));
}

void csSetColorScheme (csApp *iApp, csColorScheme &Scheme)
{
  if (!&Scheme)
  {
    csRestorePalette ();
    csFreePalette ();
  }
  else
  {
    csSavePalette ();

    // Get the HLS for scheme's base tone
    float sch_r, sch_g, sch_b;
    csGetRGB (Scheme.BaseTone, iApp, sch_r, sch_g, sch_b);
    float sch_h, sch_l, sch_s;
    csRGB2HLS (sch_r, sch_g, sch_b, sch_h, sch_l, sch_s);

    // Scheme color,contrast and blend values as floating-point values
    float sch_color = Scheme.Color / 100.;
    float sch_contrast = -Scheme.Contrast / 100.;
    float sch_blend = Scheme.Blend / 100.;

	int i, j;
    for (i = 0; i < cswsPaletteSize; i++)
    {
      int *pal = savedPalette [i];
      for (j = 0; j < cswsPalette [i].Size; j++)
      {
        float r, g, b;
        csGetRGB (pal [j], iApp, r, g, b);
        float h, l, s;
        csRGB2HLS (r, g, b, h, l, s);

        if (sch_s > 1/32.)
          h = h + (sch_h - h) * sch_color;
        l = l + (sch_l - l) * sch_contrast;
        s = s + (sch_s - s) * sch_blend;

        if (h < 0.) h += 1.; if (h > 1.) h -= 1.;
        if (l < 0.) l = 0.; if (l > 1.) l = 1.;
        if (s < 0.) s = 0.; if (s > 1.) s = 1.;

        csHLS2RGB (h, l, s, r, g, b);

        cswsPalette [i].Palette [j] =
          iApp->FindColor (csQint (r * 255), csQint (g * 255), csQint (b * 255));
      }
    }
  }

  iApp->Invalidate (true);
  iApp->SendBroadcast (cscmdColorSchemeChanged);
}
