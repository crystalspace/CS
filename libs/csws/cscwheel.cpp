/*
    Crystal Space Windowing System: color wheel class
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
#include "csws/cscwheel.h"
#include "csws/csapp.h"
#include "csws/cswsaux.h"
#include "csqsqrt.h"

#define COLORWHEEL_TEXTURE_NAME	"csws::ColorWheel"

static int cwref = 0;
static csPixmap *cwspr = 0;

csColorWheel::csColorWheel (csComponent *iParent) : csStatic (iParent, csscsBitmap)
{
  h = s = 0;
  trackmouse = false;
  cwref++;
  if (app)
  {
    // If color wheel image is not loaded, load it
    if (!cwspr)
      cwspr = new csSimplePixmap (app->GetTexture (
        COLORWHEEL_TEXTURE_NAME), 0, 0, 128, 128);
  } /* endif */
  Bitmap = cwspr;
  SetSuggestedSize (0, 0);
}

csColorWheel::~csColorWheel ()
{
  if (--cwref == 0)
  {
    delete cwspr;
    cwspr = 0;
  }
  Bitmap = 0;
}

bool csColorWheel::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevMouseDown:
    case csevMouseMove:
      if (trackmouse || (Event.Type == csevMouseDown))
      {
        if (!trackmouse)
        {
          app->CaptureMouse (this);
          trackmouse = true;
        }
        float xc = (float) bound.Width () / 2;
        float yc = (float) bound.Height () / 2;
        float ns = csQsqrt ((Event.Mouse.x - xc)*(Event.Mouse.x - xc) + 
          (Event.Mouse.y - yc)*(Event.Mouse.y - yc)) / xc;
        if (ns > 1) ns = 1;
        float nh = (float) atan2 (yc - Event.Mouse.y, Event.Mouse.x - xc) / TWO_PI;
        if (nh < 0) nh += 1;
        if ((ns != s) || (nh != h))
        {
          s = ns; h = nh;
          parent->SendCommand (cscmdColorWheelChanged, (intptr_t)this);
        }
        return true;
      }
    case csevMouseUp:
      if (trackmouse)
      {
        app->CaptureMouse (0);
        trackmouse = false;
      }
      return true;
  }
  return csStatic::HandleEvent (Event);
}

void csColorWheel::SetHS (float iH, float iS)
{
  h = iH; s = iS;
  Invalidate ();
}

void csColorWheel::Draw ()
{
  csStatic::Draw ();
  float xc = (float) bound.Width () / 2;
  float yc = (float) bound.Height () / 2;
  int x = int (xc * (1 + s * cos (h * TWO_PI)));
  int y = int (yc * (1 - s * sin (h * TWO_PI)));
  if (x >= bound.Width ()) x = bound.Width () - 1;
  if (y >= bound.Height ()) y = bound.Height () - 1;
  Line ((float) x, 0, (float) x, (float) bound.Height (), CSPAL_STATIC_LIGHT3D);
  Line (0, (float) y, (float) bound.Width (), (float) y, CSPAL_STATIC_LIGHT3D);
}
