/*
    Crystal Space Windowing System: dialog window class
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
#include "csws/csdialog.h"
#include "csws/csapp.h"
#include "csws/cswsutil.h"
#include "csws/sdefault.h"
#include "qint.h"

void csDefaultDialogSkin::Initialize (csApp *iApp, csSkin *Parent)
{
  (void)iApp;
  Parent->Load (Back, "Dialog", "Background");
}

void csDefaultDialogSkin::Deinitialize ()
{
  Back.Free ();
}

void csDefaultDialogSkin::Draw (csComponent &This)
{
#define This ((csDialog &)This)
  switch (This.GetFrameStyle ())
  {
    case csdfsNone:
      break;
    case csdfsHorizontal:
      This.Line (0, 0, This.bound.Width (), 0, CSPAL_DIALOG_2LIGHT3D);
      This.Line (0, 1, This.bound.Width (), 1, CSPAL_DIALOG_LIGHT3D);
      This.Line (0, This.bound.Height () - 2, This.bound.Width (),
         This.bound.Height () - 2, CSPAL_DIALOG_DARK3D);
      This.Line (0, This.bound.Height () - 1, This.bound.Width (),
        This.bound.Height () - 1, CSPAL_DIALOG_2DARK3D);
      break;
    case csdfsVertical:
      This.Line (0, 0, 0, This.bound.Height (), CSPAL_DIALOG_2LIGHT3D);
      This.Line (1, 0, 1, This.bound.Height (), CSPAL_DIALOG_LIGHT3D);
      This.Line (This.bound.Width () - 2, 0, This.bound.Width () - 2,
        This.bound.Height (), CSPAL_DIALOG_DARK3D);
      This.Line (This.bound.Width () - 1, 0, This.bound.Width () - 1,
        This.bound.Height (), CSPAL_DIALOG_2DARK3D);
      break;
    case csdfsAround:
      This.Rect3D (0, 0, This.bound.Width (), This.bound.Height (),
        CSPAL_DIALOG_2DARK3D, CSPAL_DIALOG_2LIGHT3D);
      This.Rect3D (1, 1, This.bound.Width () - 1, This.bound.Height () - 1,
        CSPAL_DIALOG_DARK3D, CSPAL_DIALOG_LIGHT3D);
      break;
    case csdfsBitmap:
      break;
  } /* endswitch */

  if (This.GetAlpha () >= 255)
    return;

  int bw, bh;
  This.GetBorderSize (bw, bh);
  if ((Back.GetType () == csbgNone)
   && (Back.GetColor () != CSPAL_DIALOG_BACKGROUND))
  {
    float r, g, b;
    csGetRGB (This.GetColor (CSPAL_DIALOG_BACKGROUND), This.app, r, g, b);
    csRGBcolor rgb (QInt (r * 255.9), QInt (g * 255.9), QInt (b * 255.9));
    Back.SetColor (CSPAL_DIALOG_BACKGROUND);
    Back.SetColor (0, rgb);
    Back.SetColor (1, rgb);
    Back.SetColor (2, rgb);
    Back.SetColor (3, rgb);
    Back.SetType (csbgNone);
  }


// Draw the background only if this is NOT a Bitmap-style frame
if (This.GetFrameStyle() != csdfsBitmap)
{

   int orgx = 0, orgy = 0;
   // If dialog has title, suppose it is the top-level window
   // thus we'll align the texture with the dialog; otherwise
   // we'll align the texture to parent's top-left corner.
   if (!This.GetText ())
   {
     orgx = -This.bound.xmin;
     orgy = -This.bound.ymin;
  }
   Back.Draw (This, bw, bh, This.bound.Width () - 2 * bw,
     This.bound.Height () - 2 * bh, orgx, orgy, This.GetAlpha ());
  }
  // Draw the bitmap-style frame
  else
  {
  	This.Pixmap(This.GetFrameBitmap(), 0,0, This.GetAlpha());
  	This.Pixmap(This.GetOverlayBitmap(), 0,0, This.GetOverlayAlpha());
  }
#undef This
}

void csDefaultDialogSkin::SetBorderSize (csDialog &This)
{
  int bw = 0, bh = 0;
  switch (This.GetFrameStyle ())
  {
    case csdfsNone:
      bw = 0; bh = 0;
      break;
    case csdfsHorizontal:
      bw = 0; bh = 2;
      break;
    case csdfsVertical:
      bw = 2; bh = 0;
      break;
    case csdfsAround:
      bw = 2; bh = 2;
      break;
    default:
      return;
  } /* endswitch */
  This.SetBorderSize (bw, bh);
}
