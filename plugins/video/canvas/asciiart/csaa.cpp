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
#include "sysdef.h"
#include "cscom/com.h"
#include "csgeom/csrect.h"
#include "csinput/csevent.h"
#include "csutil/inifile.h"
#include "isystem.h"
#include "qint.h"

#include "csaa.h"

//------------------------------------------------------ Loading/Maintenance ---

static unsigned int gRefCount = 0;
static DllRegisterData gRegData =
{
  &CLSID_AAGraphics2D,
  "crystalspace.graphics2d.asciiart",
  "Crystal Space 2D graphics driver using Ascii Art library"
};

#ifdef CS_STATIC_LINKED

void AARegister ()
{
  static csGraphics2DFactoryAA gG2DFactory;
  gRegData.pClass = &gG2DFactory;
  csRegisterServer (&gRegData);
}

void AAUnregister ()
{
  csUnregisterServer (&gRegData);
}

#else

// Initialization entry point
STDAPI DllInitialize ()
{
  csCoInitialize (0);
  gRegData.szInProcServer = "asciiart.dll";
  return TRUE;
}

void STDAPICALLTYPE ModuleRelease ()
{
  gRefCount--;
}

void STDAPICALLTYPE ModuleAddRef ()
{
  gRefCount++;
}

// return S_OK if it's ok to unload us now.
STDAPI DllCanUnloadNow ()
{
  return gRefCount ? S_FALSE : S_OK;
}

// used to get a COM class object from us.
STDAPI DllGetClassObject (REFCLSID rclsid, REFIID riid, void** ppv)
{
  static csGraphics2DFactoryAA gG2DFactory;
  if (rclsid == CLSID_AAGraphics2D)
    return gG2DFactory.QueryInterface (riid, ppv);

  // if we get here, rclsid is a class we don't implement
  *ppv = NULL;
  return CLASS_E_CLASSNOTAVAILABLE;
}

// Called by regsvr
STDAPI DllRegisterServer ()
{
  return csRegisterServer (&gRegData);
}

// Called by regsvr
STDAPI DllUnregisterServer ()
{
  return csUnregisterServer (&gRegData);
}

#endif // CS_STATIC_LINKED

//---------------------------------------------------- csGraphics2DAAFactory ---

IMPLEMENT_UNKNOWN_NODELETE (csGraphics2DFactoryAA)

BEGIN_INTERFACE_TABLE (csGraphics2DFactoryAA)
  IMPLEMENTS_INTERFACE (IGraphics2DFactory)
END_INTERFACE_TABLE ()

STDMETHODIMP csGraphics2DFactoryAA::CreateInstance (REFIID riid,
  ISystem* piSystem, void**ppv)
{
  if (!piSystem)
  {
    *ppv = 0;
    return E_INVALIDARG;
  }

  CHK (csGraphics2DAA *pNew = new csGraphics2DAA (piSystem));
  if (!pNew)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }
		
  return pNew->QueryInterface (riid, ppv);
}

STDMETHODIMP csGraphics2DFactoryAA::LockServer (COMBOOL bLock)
{
  if (bLock)
    gRefCount++;
  else
    gRefCount--;

  return S_OK;
}

//----------------------------------------------------------- csGraphics2DAA ---

BEGIN_INTERFACE_TABLE (csGraphics2DAA)
  IMPLEMENTS_COMPOSITE_INTERFACE_EX (IGraphics2D, XGraphics2D)
  IMPLEMENTS_COMPOSITE_INTERFACE_EX (IGraphicsInfo, XGraphicsInfo)
END_INTERFACE_TABLE ()

IMPLEMENT_UNKNOWN_NODELETE (csGraphics2DAA)

csGraphics2DAA::csGraphics2DAA (ISystem* piSystem) :
  csGraphics2D (piSystem)
{
  System = piSystem;
  config = new csIniFile ("asciiart.cfg");

  // Load settings from config file and setup the aa_defparams structure
  System->ConfigGetYesNo ("VideoDriver", "SYS_MOUSE_CURSOR", HardwareCursor, true);

  aa_defparams.width =
  aa_defparams.recwidth = config->GetInt ("Console", "Width", 80);
  aa_defparams.height =
  aa_defparams.recheight = config->GetInt ("Console", "Height", 25);
  char *font = config->GetStr ("Console", "Font", "vga16");
  for (int i = 0; aa_fonts [i]; i++)
    if ((strcasecmp (font, aa_fonts [i]->name) == 0)
     || (strcasecmp (font, aa_fonts [i]->shortname) == 0))
      aa_defparams.font = aa_fonts [i];

#define SETFLAG(s, m)				\
  if (config->GetYesNo ("Console", s, true))	\
    aa_defparams.supported |= m;		\
  else						\
    aa_defparams.supported &= ~m;

  SETFLAG ("Normal",	AA_NORMAL_MASK);
  SETFLAG ("Dim",	AA_DIM_MASK);
  SETFLAG ("Bright",	AA_BOLD_MASK);
  SETFLAG ("BoldFont",	AA_BOLDFONT_MASK);
  SETFLAG ("Reverse",	AA_REVERSE_MASK);
  SETFLAG ("All",	AA_ALL);
  SETFLAG ("EightBit",	AA_EIGHT);

#undef SETFLAG

  aa_defrenderparams.inversion = config->GetYesNo ("Rendering", "Inverse", false);
  char *dither = config->GetStr ("Rendering", "Dither", "none");
  if (strcasecmp (dither, "none") == 0)
    aa_defrenderparams.dither = 0;
  else if (strcasecmp (dither, "floyd-steinberg") == 0)
    aa_defrenderparams.dither = AA_FLOYD_S;
  else if (strcasecmp (dither, "error-distribution") == 0)
    aa_defrenderparams.dither = AA_ERRORDISTRIB;
  aa_defrenderparams.randomval = config->GetInt ("Rendering", "RandomDither", 0);
  aa_defrenderparams.bright = config->GetInt ("Rendering", "Bright", 1);
  aa_defrenderparams.contrast = config->GetInt ("Rendering", "Contrast", 1);
}

csGraphics2DAA::~csGraphics2DAA (void)
{
  Close ();
  if (config)
    delete config;
}

void csGraphics2DAA::Initialize ()
{
  csGraphics2D::Initialize ();

  Depth = 8;
  pfmt.PalEntries = 256;
  pfmt.PixelBytes = 1;
  pfmt.RedMask    = 0xff;
  pfmt.GreenMask  = 0xff;
  pfmt.BlueMask   = 0xff;
  complete_pixel_format ();
}

bool csGraphics2DAA::Open (char *Title)
{
  context = aa_autoinit (&aa_defparams);
  if (context == NULL)
  {
    SysPrintf (MSG_FATAL_ERROR, "Cannot initialize AA-lib. Sorry\n");
    return false;
  }
  Width = aa_imgwidth (context);
  Height = aa_imgheight (context);

  aa_autoinitkbd (context, AA_SENDRELEASE);
  aa_autoinitmouse (context, AA_MOUSEALLMASK);

  aa_hidecursor (context);

  return csGraphics2D::Open (Title);
}

void csGraphics2DAA::Close ()
{
  if (!context)
    return;
  aa_showcursor (context);
  aa_uninitmouse (context);
  aa_uninitkbd (context);
  aa_close (context);
  context = NULL;
}

static int oldmousebut = 0;
static int oldmousex = -1, oldmousey = -1;
static bool shift_state = false, ctrl_state = false, alt_state = false;

void csGraphics2DAA::Print (csRect *area)
{
  int sx1, sx2, sy1, sy2;
  if (area)
  {
    float xc = float (aa_scrwidth (context)) / Width;
    float yc = float (aa_scrheight (context)) / Height;

    sx1 = QRound (area->xmin * xc);
    sx2 = QRound (area->xmax * xc);
    sy1 = QRound (area->ymin * yc);
    sy2 = QRound (area->ymax * yc);
  }
  else
  {
    sx1 = 0;
    sx2 = aa_scrwidth (context);
    sy1 = 0;
    sy2 = aa_scrheight (context);
  }

  aa_renderpalette (context, palette, &aa_defrenderparams, sx1, sy1, sx2, sy2);
  aa_flush (context);

  /* Get all events from keyboard and mouse and put them into the system queue */
  int event;
  int shiftmask =
    (shift_state ? CSMASK_SHIFT : 0) |
    (ctrl_state ? CSMASK_CTRL : 0) |
    (alt_state ? CSMASK_ALT : 0);
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
          System->QueueMouseEvent (0, 0, x, y, shiftmask);
          oldmousex = x; oldmousey = y;
        }
        if (oldmousebut != b)
        {
          static int but [3] = { 1, 3, 2 };
          for (int i = 0; i <= 2; i++)
            if ((oldmousebut ^ b) & (1 << i))
              System->QueueMouseEvent (but [i], (b & (1 << i)) != 0, x, y, shiftmask);
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
          case AA_INS:		event = CSKEY_INS;	break;
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
          case AA_F12:		event = CSKEY_F12;	break;
          case '\n':		event = CSKEY_ENTER;	break;
        }
        if (event == CSKEY_SHIFT)
          shift_state = down;
        if (event == CSKEY_ALT)
          alt_state = down;
        if (event == CSKEY_CTRL)
          ctrl_state = down;
        System->QueueKeyEvent (event, down);
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
  Memory = aa_image (context);
  return true;
}

void csGraphics2DAA::FinishDraw ()
{
  Memory = NULL;
}

bool csGraphics2DAA::SetMousePosition (int x, int y)
{
  return false;
}

bool csGraphics2DAA::SetMouseCursor (int iShape, ITextureHandle *hBitmap)
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

#if 0
void csGraphics2DAA::MouseHandlerStub (void *Self, int Button, int Down,
  int x, int y, int ShiftFlags)
{
  csGraphics2DAA *This = (csGraphics2DAA *)Self;
  if (!This)
    return;
  int ww = This->dW->WindowWidth (), wh = This->dW->WindowHeight ();
  if ((ww <= 0) || (wh <= 0))
    return;

  int fwidth, fheight;
  This->System->GetWidthSetting (fwidth);
  This->System->GetHeightSetting (fheight);

  x = (x * fwidth) / ww;
  y = ((wh - 1 - y) * fheight) / wh;

  This->OS2System->MouseEvent (Button, Down, x, y,
    (ShiftFlags & dkf_SHIFT ? CSMASK_SHIFT : 0) |
    (ShiftFlags & dkf_CTRL ? CSMASK_CTRL : 0) |
    (ShiftFlags & dkf_ALT ? CSMASK_ALT : 0));
}

void csGraphics2DAA::KeyboardHandlerStub (void *Self, unsigned char ScanCode,
  int Down, unsigned char RepeatCount, int ShiftFlags)
{
  csGraphics2DAA *This = (csGraphics2DAA *)Self;
  if (!This)
    return;
  This->OS2System->KeyboardEvent (ScanCode, Down);
}

void csGraphics2DAA::FocusHandlerStub (void *Self, bool Enable)
{
  csGraphics2DAA *This = (csGraphics2DAA *)Self;
  This->OS2System->FocusEvent (Enable);
}

void csGraphics2DAA::TerminateHandlerStub (void *Self)
{
  csGraphics2DAA *This = (csGraphics2DAA *)Self;
  This->OS2System->TerminateEvent ();
}
#endif
