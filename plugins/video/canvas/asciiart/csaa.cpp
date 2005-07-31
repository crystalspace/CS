/*
    ASCII art rendering support for Crystal Space 3D library
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Andrew Zabolotny <bit@eltech.ru>

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

#include <stdarg.h>
#include <limits.h>
#include "cssysdef.h"
#include "csaa.h"
#include "csgeom/csrect.h"
#include "csutil/cfgacc.h"
#include "csutil/sysfunc.h"
#include "iutil/cfgfile.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "csqint.h"
#include "csgfx/rgbpixel.h"

//---------------------------------------------------------- csGraphics2DAA ---

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGraphics2DAA)


SCF_IMPLEMENT_IBASE_EXT (csGraphics2DAA)
  SCF_IMPLEMENTS_INTERFACE (iEventPlug)
SCF_IMPLEMENT_IBASE_EXT_END

csGraphics2DAA::csGraphics2DAA (iBase *iParent) : csGraphics2D (iParent)
{
  context = 0;
  EventOutlet = 0;
  Memory = 0;
}

csGraphics2DAA::~csGraphics2DAA (void)
{
  Close ();
}

bool csGraphics2DAA::Initialize (iObjectRegistry *object_reg)
{
  if (!csGraphics2D::Initialize (object_reg))
    return false;

  csConfigAccess config;
  config.AddConfig(object_reg, "/config/asciiart.cfg");
  config.AddConfig(object_reg, "/config/video.cfg");

  // Load settings from config file and setup the aa_defparams structure
  HardwareCursor = config->GetBool ("Video.SystemMouseCursor", true);

  aa_defparams.width =
  aa_defparams.recwidth = config->GetInt ("Video.ASCII.Console.Width", 80);
  aa_defparams.height =
  aa_defparams.recheight = config->GetInt ("Video.ASCII.Console.Height", 25);
  const char *font = config->GetStr ("Video.ASCII.Console.Font", "vga16");
  int i;
  for (i = 0; aa_fonts [i]; i++)
    if ((strcasecmp (font, aa_fonts [i]->name) == 0)
     || (strcasecmp (font, aa_fonts [i]->shortname) == 0))
      aa_defparams.font = aa_fonts [i];

#define SETFLAG(s, m)                           \
  if (config->GetBool (s, true))                \
    aa_defparams.supported |= (m);              \
  else                                          \
    aa_defparams.supported &= ~(m);

  SETFLAG ("Video.ASCII.Console.Normal", AA_NORMAL_MASK);
  SETFLAG ("Video.ASCII.Console.Dim", AA_DIM_MASK);
  SETFLAG ("Video.ASCII.Console.Bright", AA_BOLD_MASK);
  SETFLAG ("Video.ASCII.Console.BoldFont", AA_BOLDFONT_MASK);
  SETFLAG ("Video.ASCII.Console.Reverse", AA_REVERSE_MASK);
  SETFLAG ("Video.ASCII.Console.All", AA_ALL);
  SETFLAG ("Video.ASCII.Console.EightBit", AA_EIGHT);
//  SETFLAG ("Video.ASCII.Console.GenFont", AA_GENFONT);
//  if (aa_defparams.supported & AA_GENFONT)
//    aa_defparams.supported |= AA_EXTENDED;

#undef SETFLAG

  aa_defrenderparams.inversion = config->GetBool
        ("Video.ASCII.Rendering.Inverse", false);
  const char *dither = config->GetStr ("Video.ASCII.Rendering.Dither", "none");
  if (strcasecmp (dither, "none") == 0)
    aa_defrenderparams.dither = AA_NONE;
  else if (strcasecmp (dither, "floyd-steinberg") == 0)
    aa_defrenderparams.dither = AA_FLOYD_S;
  else if (strcasecmp (dither, "error-distribution") == 0)
    aa_defrenderparams.dither = AA_ERRORDISTRIB;
  aa_defrenderparams.randomval = config->GetInt
        ("Video.ASCII.Rendering.RandomDither", 0);
  aa_defrenderparams.bright = config->GetInt
        ("Video.ASCII.Rendering.Bright", 1);
  aa_defrenderparams.contrast = config->GetInt
        ("Video.ASCII.Rendering.Contrast", 1);

  Depth = 32;
  pfmt.RedMask   = 0xff << 24;
  pfmt.GreenMask = 0xff << 16;
  pfmt.BlueMask  = 0xff << 8;
  pfmt.AlphaMask = 0x00;
  pfmt.PalEntries = 0;
  pfmt.PixelBytes = 4;
  pfmt.complete ();

  csRef<iEventQueue> q = CS_QUERY_REGISTRY(object_reg, iEventQueue);
  if (q != 0)
  {
    if (!EventOutlet.IsValid())
      EventOutlet = q->CreateEventOutlet (this);
  }
  return true;
}

bool csGraphics2DAA::Open ()
{
  if (is_open) return true;
  context = aa_autoinit (&aa_defparams);
  if (context == 0)
  {
    csPrintf ("Cannot initialize AA-lib. Sorry\n");
    return false;
  }
  Width = aa_imgwidth (context);
  Height = aa_imgheight (context);
  Memory = new unsigned char[ pfmt.PixelBytes*Width*Height ];

  aa_autoinitkbd (context, AA_SENDRELEASE);
  aa_autoinitmouse (context, AA_MOUSEALLMASK);

  aa_hidecursor (context);

  return csGraphics2D::Open ();
}

void csGraphics2DAA::Close ()
{
  if (!is_open) return;
  if (!context)
    return;
  if(Memory) delete[] Memory;
  Memory = 0;
  aa_showcursor (context);
  aa_uninitmouse (context);
  aa_uninitkbd (context);
  aa_close (context);
  context = 0;
}

static int oldmousebut = 0;
static int oldmousex = -1, oldmousey = -1;
static bool shift_state = false, ctrl_state = false, alt_state = false;

void csGraphics2DAA::Print (csRect const* area)
{
  int sx1, sx2, sy1, sy2;
  if (area)
  {
    float xc = float (aa_scrwidth (context)) / Width;
    float yc = float (aa_scrheight (context)) / Height;

    sx1 = csQround (area->xmin * xc);
    sx2 = csQround (area->xmax * xc);
    sy1 = csQround (area->ymin * yc);
    sy2 = csQround (area->ymax * yc);
  }
  else
  {
    sx1 = 0;
    sx2 = aa_scrwidth (context);
    sy1 = 0;
    sy2 = aa_scrheight (context);
  }

  // render Memory image to aa context imagebuffer
  for(int y=0; y<Height; ++y)
    for(int x=0; x<Width; ++x)
      context->imagebuffer[y*Width + x] = 
	((csRGBpixel*)Memory)[y*Width+x].Luminance();
  aa_render (context, &aa_defrenderparams, sx1, sy1, sx2, sy2);
  aa_flush (context);

  /* Get all events from keyboard and mouse and put them into system queue */
  int event;
  while ((event = aa_getevent (context, 0)) != AA_NONE)
    switch (event)
    {
      case AA_MOUSE:
      {
        int x, y, b;
        aa_getmouse (context, &x, &y, &b);
        x = (x * Width) / aa_scrwidth (context);
        y = (y * Height) / aa_scrheight (context);
        if ((oldmousex != x)
         || (oldmousey != y))
        {
          EventOutlet->Mouse (0, 0, x, y);
          oldmousex = x; oldmousey = y;
        }
        if (oldmousebut != b)
        {
          static int but [3] = { 1, 3, 2 };
		  int i;
          for (i = 0; i <= 2; i++)
            if ((oldmousebut ^ b) & (1 << i))
              EventOutlet->Mouse (but [i], (b & (1 << i)) != 0, x, y);
          oldmousebut = b;
        }
        break;
      }
      case AA_UNKNOWN:
      case AA_RESIZE:
        break;
      default:
      {
        bool down = (event & AA_RELEASE) == 0;
        event &= ~AA_RELEASE;
        switch (event)
        {
          case AA_UP:		event = CSKEY_UP;	break;
          case AA_DOWN:		event = CSKEY_DOWN;	break;
          case AA_LEFT:		event = CSKEY_LEFT;	break;
          case AA_RIGHT:	event = CSKEY_RIGHT;	break;
          case AA_BACKSPACE:	event = CSKEY_BACKSPACE;break;
          case AA_ESC:		event = CSKEY_ESC;	break;
/*          case AA_INS:		event = CSKEY_INS;	break;
          case AA_DEL:		event = CSKEY_DEL;	break;
          case AA_HOME:		event = CSKEY_HOME;	break;
          case AA_END:		event = CSKEY_END;	break;
          case AA_PGUP:		event = CSKEY_PGUP;	break;
          case AA_PGDN:		event = CSKEY_PGDN;	break;
          case AA_SHIFT:	event = CSKEY_SHIFT;	break;
          case AA_CTRL:		event = CSKEY_CTRL;	break;
          case AA_ALT:		event = CSKEY_ALT;	break;
          case AA_F1:		event = CSKEY_F1;	break;
          case AA_F2:		event = CSKEY_F2;	break;
          case AA_F3:		event = CSKEY_F3;	break;
          case AA_F4:		event = CSKEY_F4;	break;
          case AA_F5:		event = CSKEY_F5;	break;
          case AA_F6:		event = CSKEY_F6;	break;
          case AA_F7:		event = CSKEY_F7;	break;
          case AA_F8:		event = CSKEY_F8;	break;
          case AA_F9:		event = CSKEY_F9;	break;
          case AA_F10:		event = CSKEY_F10;	break;
          case AA_F11:		event = CSKEY_F11;	break;
          case AA_F12:		event = CSKEY_F12;	break;*/
          case '\n':		event = CSKEY_ENTER;	break;
        }
        if (event == CSKEY_SHIFT)
          shift_state = down;
        if (event == CSKEY_ALT)
          alt_state = down;
        if (event == CSKEY_CTRL)
          ctrl_state = down;
        EventOutlet->Key (event, 0, down);
      }
    }
}

void csGraphics2DAA::SetRGB (int i, int r, int g, int b)
{
  aa_setpalette (palette, i, r, g, b);
  csGraphics2D::SetRGB (i, r, g, b);
}

bool csGraphics2DAA::BeginDraw ()
{
  csGraphics2D::BeginDraw ();
  return true;
}

void csGraphics2DAA::FinishDraw ()
{
  csGraphics2D::FinishDraw ();
}

bool csGraphics2DAA::SetMousePosition (int x, int y)
{
  return false;
}

bool csGraphics2DAA::SetMouseCursor (csMouseCursorID iShape)
{
  if (!HardwareCursor)
  {
    aa_hidemouse (context);
    return false;
  } /* endif */

  switch (iShape)
  {
    case csmcNone:
      aa_hidemouse (context);
      return true;
    case csmcArrow:
      aa_showmouse (context);
      return true;
    default:
      aa_hidemouse (context);
      return false;
  } /* endswitch */
}
