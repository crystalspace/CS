/*
    A test program for Crystal Space canvas plugins.
    Copyright (C) 1999 Andrew Zabolotny <bit@eltech.ru>

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

#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "csutil/csuctransform.h"
#include "csutil/randomgen.h"
#include "csutil/csstring.h"
#include "cstool/initapp.h"
#include "ivaria/reporter.h"
#include "csqint.h"
#include "csqsqrt.h"
#include "csgeom/polyaa.h"
#include "csgeom/vector2.h"

#include "iutil/cfgmgr.h"
#include "iutil/vfs.h"
#include "iutil/plugin.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivideo/graph2d.h"
#include "ivideo/natwin.h"
#include "ivideo/fontserv.h"
#include "ivideo/custcursor.h"
#include "csutil/cmdhelp.h"
#include "csutil/util.h"
#include "iutil/cmdline.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "csutil/event.h"

CS_IMPLEMENT_APPLICATION

#define APP_TITLE	"Graphics canvas plugin test"

class G2DTestSystemDriver;
G2DTestSystemDriver* Sys;

#include "teststrings.h"

class G2DTestSystemDriver
{
  // Application states
  enum appState
  {
    stInit,
    stStartup,
    stContextInfo,
    stWindowFixed,
    stWindowResize,
    stCustomCursor,
    stAlphaTest,
    stBackBufferON,
    stBackBufferOFF,
    stTestUnicode1,
    stTestUnicode2,
    stTestFreetype,
    stTestLineDraw,
    stTestLinePerf,
    stTestTextDraw,
    stTestTextDraw2,
    stPixelClipTest,
    stLineClipTest,
    stBoxClipTest,
    stFontClipTest,
    stBlitTest,
    stPause,
    stWaitKey
  };

  // Application state (stored in a stack fashion, topmost is active state)
  appState state [20];
  // State stack top pointer
  int state_sptr;

  // Pixel format
  csPixelFormat pfmt;
  bool pfmt_init;
  // Timer
  int timer;
  // some handy colors
  int white, yellow, green, red, blue, black, gray, dsteel;
  // Last pressed key
  int lastkey, lastkey2, lastkey3, lastkey4, lastkey5, lastkey6, lastkey7, 
    lastkey8, lastkey9;
  // Switch backbuffer while waiting for a key
  bool SwitchBB;
  // Current font
  csRef<iFont> font;
  csRef<iFont> fontLarge;
  csRef<iFont> fontItalic;
  csRef<iFont> fontCourier;
  csRef<iFont> fontSmall;
  csRef<iImage> blitTestImage;
  csRef<iImage> alphaBlitImage;
  // Event Outlet
  iEventOutlet* EventOutlet;

public:
  csRef<iGraphics2D> myG2D;
  csRef<iGraphics3D> myG3D;
  iObjectRegistry* object_reg;
  csRef<iCursor> cursorPlugin;

public:
  G2DTestSystemDriver (int argc, char* argv[]);
  virtual ~G2DTestSystemDriver ();
  void SetupFrame ();
  void FinishFrame ();
  bool HandleEvent (iEvent &Event);

private:
  csPtr<iFont> GetFont (const char *fontID, int size = 10);
  void SetFont (iFont* font);

  void EnterState (appState newstate, int arg = 0);
  void LeaveState ();

  int MakeColor (int r, int g, int b, int a = 255);
  void WriteCentered (int mode, int dy, int fg, int bg, const char *format, ...);
  void WriteCenteredWrapped (int mode, int dy, int &h, int fg, int bg, 
    const char *format, ...);

  void ResizeContext ();

  void SetCustomCursor ();
  void SetNormalCursor ();

  void DrawStartupScreen ();
  void DrawContextInfoScreen ();
  void DrawWindowScreen ();
  void DrawWindowResizeScreen ();
  void DrawCustomCursorScreen ();
  void DrawAlphaTestScreen ();
  void DrawBackBufferText ();
  void DrawBackBufferON ();
  void DrawBackBufferOFF ();
  void DrawUnicodeTest1 ();
  void DrawUnicodeTest2 ();
  void DrawFreetypeTest ();
  void DrawLineTest ();
  void DrawLinePerf ();
  void DrawTextTest ();
  void DrawTextTest2 ();

  void PixelClipTest ();
  void LineClipTest  ();
  void BoxClipTest ();
  void FontClipTest ();

  void BlitTest ();

  void DrawClipRect(int x, int y, int w, int h);
};

G2DTestSystemDriver::G2DTestSystemDriver (int argc, char* argv[])
{
  state_sptr = 0;
  EnterState (stInit);
  SwitchBB = false;
  pfmt_init = false;

  object_reg = csInitializer::CreateEnvironment (argc, argv);

  if (!csInitializer::SetupConfigManager (object_reg, "/config/g2dtest.cfg"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.g2dtest",
        "Unable to init app!");
    exit (0);
  }

  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
  	CS_REQUEST_FONTSERVER,
	CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.g2dtest",
        "Unable to init app!");
    exit (0);
  }

  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (q != 0)
  {
    EventOutlet = q->GetEventOutlet();
    EventOutlet->IncRef();
  }

}

G2DTestSystemDriver::~G2DTestSystemDriver ()
{
  if (EventOutlet != 0)
    EventOutlet->DecRef();
  font = 0;
  fontItalic = 0;
  fontCourier = 0;
  fontLarge = 0;
  fontSmall = 0;
  blitTestImage = 0;
  alphaBlitImage = 0;
  cursorPlugin = 0;
  csInitializer::DestroyApplication (object_reg);
}

void G2DTestSystemDriver::EnterState (appState newstate, int arg)
{
  state [state_sptr++] = newstate;
  switch (newstate)
  {
    case stPause:
      timer = csGetTicks () + arg;
      break;
    case stTestLinePerf:
      lastkey2 = 0;
      break;
    case stTestTextDraw:
      lastkey3 = 0;
      break;
    case stTestTextDraw2:
      lastkey4 = 0;
      break;
    case stPixelClipTest:
      lastkey5 = 0;
      break;
    case stLineClipTest:
      lastkey6 = 0;
      break;
    case stBoxClipTest:
      lastkey7 = 0;
      break;
    case stFontClipTest:
      lastkey8 = 0;
      break;
    case stCustomCursor:
      lastkey9 = 0;
      break;
    case stWaitKey:
      lastkey = 0;
      break;
    default:
      break;
  }
}

void G2DTestSystemDriver::LeaveState ()
{
  state_sptr--;
}

void G2DTestSystemDriver::SetupFrame ()
{
  if (!pfmt_init)
  {
    pfmt_init = true;
    pfmt = *myG2D->GetPixelFormat ();
    white = MakeColor (255, 255, 255);
    yellow = MakeColor (255, 255, 0);
    green = MakeColor (0, 255, 0);
    red = MakeColor (255, 0, 0);
    blue = MakeColor (0, 0, 255);
    gray = MakeColor (128, 128, 128);
    dsteel = MakeColor (80, 100, 112);
    black = MakeColor (0, 0, 0);
  }

  if (state_sptr == 0)
  {
    EventOutlet->Broadcast (cscmdQuit);
    return;
  }

  appState curstate = state [state_sptr - 1];
  switch (curstate)
  {
    case stInit:
    case stStartup:
    case stContextInfo:
    case stWindowFixed:
    case stWindowResize:
    case stCustomCursor:
    case stAlphaTest:
    case stBackBufferON:
    case stBackBufferOFF:
    case stTestUnicode1:
    case stTestUnicode2:
    case stTestFreetype:
    case stTestLineDraw:
    case stTestLinePerf:
    case stTestTextDraw:
    case stTestTextDraw2:
    case stPixelClipTest:
    case stLineClipTest:
    case stBoxClipTest:
    case stFontClipTest:
    case stBlitTest:
    {
      if (!myG3D->BeginDraw (CSDRAW_2DGRAPHICS))
        break;

      myG2D->Clear (black);
      LeaveState ();
      switch (curstate)
      {
        case stInit:
	  fontLarge = GetFont (CSFONT_LARGE);
	  fontItalic = GetFont (CSFONT_ITALIC);
	  fontCourier = GetFont (CSFONT_COURIER);
	  fontSmall = GetFont (CSFONT_SMALL);
	  {
	    csRef<iVFS> vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
	    csRef<iImageIO> iio = CS_QUERY_REGISTRY (object_reg,
	      iImageIO);
	    if (vfs.IsValid () && iio.IsValid ())
	    {
	      csRef<iFile> testFile = vfs->Open ("/lib/g2dtest/up.png", 
		VFS_FILE_READ);
	      if (testFile.IsValid ())
	      {
		csRef<iDataBuffer> fileData = testFile->GetAllData ();
		blitTestImage = iio->Load (fileData, CS_IMGFMT_TRUECOLOR 
		  | CS_IMGFMT_ALPHA);
	      }
	      testFile = vfs->Open ("/lib/std/cslogo2.png", VFS_FILE_READ);
	      if (testFile.IsValid ())
	      {
		csRef<iDataBuffer> fileData = testFile->GetAllData ();
		alphaBlitImage = iio->Load (fileData, CS_IMGFMT_TRUECOLOR |
		  CS_IMGFMT_ALPHA);
	      }
	    }
	  }
	  EnterState (stStartup);
	  break;
        case stStartup:
          DrawStartupScreen ();
	  EnterState (stContextInfo);
          EnterState (stPause, 5000);
          break;
        case stContextInfo:
          DrawContextInfoScreen ();
          EnterState (stWindowFixed);
          EnterState (stWaitKey);
          break;
        case stWindowFixed:
          DrawWindowScreen ();
          EnterState (stWindowResize);
          EnterState (stWaitKey);
          break;
        case stWindowResize:
          DrawWindowResizeScreen ();
          EnterState (stCustomCursor);
          EnterState (stWaitKey);
          break;
	case stCustomCursor:
          DrawCustomCursorScreen ();
	  SetCustomCursor ();
	  if (lastkey9)
            EnterState (stAlphaTest);
	  else
            EnterState (stCustomCursor);
          break;
	case stAlphaTest:
          SetNormalCursor ();
          DrawAlphaTestScreen ();
          EnterState (stBackBufferON);
          EnterState (stWaitKey);
          break;
        case stBackBufferON:
          myG2D->AllowResize (false);
          EnterState (stBackBufferOFF);
          if (myG2D->DoubleBuffer (true))
          {
            DrawBackBufferON ();
            SwitchBB = true;
            EnterState (stWaitKey);
          }
          break;
        case stBackBufferOFF:
          EnterState (stTestUnicode1);
	  //EnterState (stPixelClipTest);
          if (myG2D->DoubleBuffer (false))
          {
            DrawBackBufferOFF ();
            SwitchBB = true;
            EnterState (stWaitKey);
          }
          break;
	case stTestUnicode1:
	  DrawUnicodeTest1 ();
          EnterState (stTestUnicode2);
          EnterState (stWaitKey);
          break;
	case stTestUnicode2:
	  DrawUnicodeTest2 ();
          EnterState (stTestFreetype);
          EnterState (stWaitKey);
          break;
	case stTestFreetype:
	  DrawFreetypeTest ();
          EnterState (stTestLineDraw);
          EnterState (stWaitKey);
          break;
        case stTestLineDraw:
          DrawLineTest ();
          EnterState (stTestLinePerf);
          EnterState (stWaitKey);
          break;
        case stTestLinePerf:
          DrawLinePerf ();
          if (lastkey2)
            EnterState (stTestTextDraw);
          else
            EnterState (stTestLinePerf);
          break;
        case stTestTextDraw:
          DrawTextTest ();
          if (lastkey3)
            EnterState (stTestTextDraw2);
          else
            EnterState (stTestTextDraw);
          break;
        case stTestTextDraw2:
          DrawTextTest2 ();
          if (lastkey4)
            EnterState (stPixelClipTest);
          else
            EnterState (stTestTextDraw2);
          break;
        case stPixelClipTest:
          PixelClipTest ();
          if (lastkey5)
          {
            myG2D->SetClipRect(0,0,myG2D->GetWidth(), myG2D->GetHeight());
            EnterState (stLineClipTest);
          }
          else
            EnterState (stPixelClipTest);
          break;
        case stLineClipTest:
          LineClipTest ();
          if (lastkey6)
          {
            myG2D->SetClipRect(0,0,myG2D->GetWidth(), myG2D->GetHeight());
            EnterState (stBoxClipTest);
          }
          else
            EnterState (stLineClipTest);
          break;
        case stBoxClipTest:
          BoxClipTest ();
          if (lastkey7)
          {
            myG2D->SetClipRect(0,0,myG2D->GetWidth(), myG2D->GetHeight());
	    EnterState (stFontClipTest);
          }
          else
            EnterState (stBoxClipTest);
          break;
        case stFontClipTest:
          FontClipTest ();
          if (lastkey8)
	    EnterState (stBlitTest);
          else
            EnterState (stFontClipTest);
          break;
	case stBlitTest:
	  BlitTest ();
          EnterState (stWaitKey);
          break;
        default:
          break;
      }
      break;
    }
    case stPause:
      if (int (csGetTicks () - timer) > 0)
        LeaveState ();
      else
        csSleep (1);
      break;
    case stWaitKey:
      if (lastkey)
      {
        LeaveState ();
        SwitchBB = false;
      }
      else
      {
        if (SwitchBB)
        {
          myG2D->Print (0);
          csSleep (200);
        }
        else
          csSleep (1);
      }
      break;
  }
}

void G2DTestSystemDriver::FinishFrame ()
{
  myG3D->FinishDraw ();
  myG3D->Print (0);
}

bool G2DTestSystemDriver::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevBroadcast:
      switch (csCommandEventHelper::GetCode(&Event))
      {
        case cscmdSystemOpen:
          if (myG2D)
          {
            // Create a uniform palette: r(3)g(3)b(2)
            int r,g,b;
            for (r = 0; r < 8; r++)
              for (g = 0; g < 8; g++)
                for (b = 0; b < 4; b++)
                  myG2D->SetRGB (r * 32 + g * 4 + b, r * 32, g * 32, b * 64);
            break;
          }
        case cscmdContextResize:
          if (myG2D)
          {
            ResizeContext ();
            break;
          }
      }
      break;
    case csevKeyboard:
      if (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown)
      {
	if (state_sptr)
	  switch (state [state_sptr - 1])
	  {
	    case stWaitKey:
	      lastkey = csKeyEventHelper::GetCookedCode (&Event);
	      break;
	    case stTestLinePerf:
	      lastkey2 = csKeyEventHelper::GetCookedCode (&Event);
	      break;
	    case stTestTextDraw:
	      lastkey3 = csKeyEventHelper::GetCookedCode (&Event);
	      break;
	    case stTestTextDraw2:
	      lastkey4 = csKeyEventHelper::GetCookedCode (&Event);
	      break;
	    case stPixelClipTest:
	      lastkey5 = csKeyEventHelper::GetCookedCode (&Event);
	      break;
	    case stLineClipTest:
	      lastkey6 = csKeyEventHelper::GetCookedCode (&Event);
	      break;
	    case stBoxClipTest:
	      lastkey7 = csKeyEventHelper::GetCookedCode (&Event);
	      break;
	    case stFontClipTest:
	      lastkey8 = csKeyEventHelper::GetCookedCode (&Event);
	      break;
	    case stCustomCursor:
	      lastkey9 = csKeyEventHelper::GetCookedCode (&Event);
	      break;
	    default:
	      break;
	  }
      }
      break;
  }
  return false;
}

int G2DTestSystemDriver::MakeColor (int r, int g, int b, int a)
{
  /*if (!pfmt.PalEntries)
    return ((r >> (8 - pfmt.RedBits)) << pfmt.RedShift)
         | ((g >> (8 - pfmt.GreenBits)) << pfmt.GreenShift)
         | ((b >> (8 - pfmt.BlueBits)) << pfmt.BlueShift)
         | ((a >> (8 - pfmt.AlphaBits)) << pfmt.AlphaShift);

  // In paletted mode this is easy since we have a uniform 3-3-2 palette
  return ((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6);*/
  return myG2D->FindRGB (r, g, b, a);
}

csPtr<iFont> G2DTestSystemDriver::GetFont (const char *fontID, int size)
{
  iFontServer *fs = myG2D->GetFontServer ();
  return (fs->LoadFont (fontID, size));
}

void G2DTestSystemDriver::SetFont (iFont* font)
{
  G2DTestSystemDriver::font = font;
}

void G2DTestSystemDriver::WriteCentered (int mode, int dy, int fg, int bg,
  const char *format, ...)
{
  if (!font) return;

  csString text;
  va_list arg;

  va_start (arg, format);
  text.FormatV (format, arg);
  va_end (arg);

  int fw, fh;
  font->GetDimensions (text, fw, fh);

  int x = (myG2D->GetWidth () - fw) / 2;
  int y = 0;

  switch (mode)
  {
    case 0: // centered by Y
      y = dy + myG2D->GetHeight () / 2;
      break;
    case 1: // from top
      y = dy;
      break;
    case 2: // from bottom
      y = dy + (myG2D->GetHeight () - 1 - fh);
      break;
  }

  myG2D->Write (font, x, y + fh - font->GetDescent(), fg, bg, text, 
    CS_WRITE_BASELINE);
}

void G2DTestSystemDriver::WriteCenteredWrapped (int mode, int dy, int &h, 
						int fg, int bg, 
						const char *format, ...)
{
  if (!font) return;

  csString text;
  va_list arg;

  va_start (arg, format);
  text.FormatV (format, arg);
  va_end (arg);

  int y = 0, w = myG2D->GetWidth ();
  int fW, fH;
  font->GetMaxSize (fW, fH);

  switch (mode)
  {
    case 0: // centered by Y
      y = dy + myG2D->GetHeight () / 2;
      break;
    case 1: // from top
      y = dy;
      break;
    case 2: // from bottom
      y = dy + (myG2D->GetHeight () - 1 - fH);
      break;
  }

  h = 0;

  int sW, sH;
  font->GetDimensions (" ", sW, sH);

  // break text so that it completely fits onto the screen.
  int lw = -sW;
  int maxLH = fH;
  char* line = csStrNew (text);
  char* p = line;
  csString drawLine;

  while (p && *p)
  {
    char* space = strchr (p, ' ');
    if (space != 0)
      *space = 0;
    int tW, tH;
    font->GetDimensions (p, tW, tH);
    if (lw + tW + sW >= w)
    {
      WriteCentered (1, y + h, fg, bg, (drawLine.GetData ()) + 1);
      drawLine.Clear ();
      drawLine << ' ' << p;
      //p = space + 1;
      lw = 0;
      h += maxLH;
      maxLH = MAX(fH, tH);
    }
    else
    {
      lw += tW + sW;
      drawLine << ' ' << p;
      maxLH = MAX(maxLH, tH);
    }
    if (space != 0) p = space + 1; else p = 0;
  }
  WriteCentered (1, y + h, fg, bg, (drawLine.GetData ()) + 1);
  h += maxLH;
  delete[] line;
}

void G2DTestSystemDriver::DrawStartupScreen ()
{
  myG2D->DrawBox (20, 20, myG2D->GetWidth () - 40, myG2D->GetHeight () - 40, blue);

  SetFont (fontItalic);
  WriteCentered (0, -20, white, -1, "WELCOME");
  SetFont (fontLarge);
  WriteCentered (0,   0, white, -1, "to graphics canvas plugin");
  WriteCentered (0, +20, white, -1, "test application");

  SetFont (fontCourier);
  WriteCentered (2, 0, green, -1, "please wait five seconds");

}

void G2DTestSystemDriver::DrawContextInfoScreen ()
{
  SetFont (fontLarge);

  WriteCentered (0,-16*3, white, -1, "Some information about graphics context");
  WriteCentered (0,-16*2, gray,  -1, "Screen size: %d x %d", myG2D->GetWidth (), myG2D->GetHeight ());
  csString pixfmt;
  if (pfmt.PalEntries)
    pixfmt.Format ("%d colors (Indexed)", pfmt.PalEntries);
  else
    pixfmt.Format ("R%dG%dB%dA%d", pfmt.RedBits, pfmt.GreenBits, pfmt.BlueBits, 
      pfmt.AlphaBits);
  WriteCentered (0,-16*1, gray,  -1, "Pixel format: %d BPP, %s", pfmt.PixelBytes * 8, 
    pixfmt.GetData());

  if (pfmt.PalEntries)
    pixfmt = "not available";
  else
    pixfmt.Format (
      "R[%08" PRIX32 "] "
      "G[%08" PRIX32 "] "
      "B[%08" PRIX32 "] "
      "A[%08" PRIX32 "]",
      pfmt.RedMask, pfmt.GreenMask, pfmt.BlueMask, pfmt.AlphaMask);
  WriteCentered (0, 16*0, gray,  -1, "R/G/B/A masks: %s", pixfmt.GetData());

  WriteCentered (0, 16*1, gray,  -1, "More than one backbuffer available: %s",
    myG2D->GetDoubleBufferState () ? "yes" : "no");
  int MinX, MinY, MaxX, MaxY;
  myG2D->GetClipRect (MinX, MinY, MaxX, MaxY);
  WriteCentered (0, 16*2, gray,  -1, "Current clipping rectangle: %d,%d - %d,%d", MinX, MinY, MaxX, MaxY);

  SetFont (fontCourier);
  WriteCentered (2, 0, green, -1, "press any key to continue");
}

void G2DTestSystemDriver::DrawWindowScreen ()
{
  SetFont (fontLarge);
  WriteCentered (0,-16*4, white, -1, "If you're running in windowed mode, you should");
  WriteCentered (0,-16*3, white, -1, "see a black window with white and green letters.");
  WriteCentered (0,-16*2, white, -1, "The window's title should read:");
  WriteCentered (0,-16*1, green, -1, APP_TITLE);

// By default context resizing should be disabled
//myG2D->AllowResize (false);
  WriteCentered (0, 16*1, white, -1, "Try to resize this window, you should either be");
  WriteCentered (0, 16*2, white, -1, "unable to do it, or the window contents should");
  WriteCentered (0, 16*3, white, -1, "rescale along width window (e.g. the resolution");
  WriteCentered (0, 16*4, white, -1, "should remain constant).");

  SetFont (fontCourier);
  WriteCentered (2, 0, green, -1, "press any key to continue");
}

void G2DTestSystemDriver::DrawWindowResizeScreen ()
{
  DrawWindowScreen ();

  myG2D->AllowResize (true);
  myG2D->DrawBox (0, myG2D->GetHeight () / 2 + 16, myG2D->GetWidth (), 16 * 4, blue);
  SetFont (fontLarge);

  WriteCentered (0, 16*1, white, -1, "Now resizing should be enabled. Try to resize the");
  WriteCentered (0, 16*2, white, -1, "window: you should be either unable to do it (if");
  WriteCentered (0, 16*3, white, -1, "canvas driver does not support resize) or see");
  WriteCentered (0, 16*4, white, -1, "the current window size in top-right corner.");

  SetFont (fontCourier);
  WriteCentered (2, 0, green, -1, "press any key to continue");
}

void G2DTestSystemDriver::ResizeContext ()
{
  if (!myG2D->BeginDraw ())
    return;

  myG2D->Clear (black);
  DrawWindowResizeScreen ();

  csString text;
  text.Format ("Canvas [%d x %d]", myG2D->GetWidth (), myG2D->GetHeight ());
  SetFont (fontLarge);
  int fw, fh;
  font->GetDimensions (text, fw, fh);
  int x = myG2D->GetWidth () - fw;
  myG2D->Write (font, x, 0, red, -1, text);

  myG2D->FinishDraw ();
  myG2D->Print (0);
}

void G2DTestSystemDriver::SetCustomCursor ()
{
  if (cursorPlugin)
  {
    cursorPlugin->SwitchCursor ("Hand");
  }
}

void G2DTestSystemDriver::SetNormalCursor ()
{
  myG2D->SetMouseCursor (csmcArrow);
}

void G2DTestSystemDriver::DrawBackBufferText ()
{
  SetFont (fontItalic);
  WriteCentered (0,-16*5, white, -1, "DOUBLE BACK BUFFER TEST");
  SetFont (fontLarge);
  WriteCentered (0,-16*3, gray,  -1, "Now graphics canvas is in double-backbuffer mode");
  WriteCentered (0,-16*2, gray,  -1, "You should see how background quickly switches");
  WriteCentered (0,-16*1, gray,  -1, "between yellow and red colors.");

  WriteCentered (0, 16*0, white, -1, "At the same time the text should stay still.");
  WriteCentered (0, 16*1, gray,  -1, "If all these statements are correct, then the");
  WriteCentered (0, 16*2, gray,  -1, "current canvas plugin have correctly implemented");
  WriteCentered (0, 16*3, gray,  -1, "double-backbuffer support.");

  WriteCentered (0, 16*5, green, -1, "BACK BUFFER NUMBER %d", myG2D->GetPage ());
}

void G2DTestSystemDriver::DrawBackBufferON ()
{
  myG2D->Clear (yellow);
  DrawBackBufferText ();
  myG2D->FinishDraw ();
  myG2D->Print (0);

  if (!myG2D->BeginDraw ())
    return;
  myG2D->Clear (red);
  DrawBackBufferText ();
}

void G2DTestSystemDriver::DrawBackBufferOFF ()
{
  myG2D->Clear (white);
  myG2D->FinishDraw ();
  myG2D->Print (0);
  if (!myG2D->BeginDraw ())
    return;

  myG2D->Clear (black);

  SetFont (fontItalic);
  WriteCentered (0,-16*3, white, -1, "SINGLE BACK BUFFER TEST");
  SetFont (fontLarge);
  WriteCentered (0,-16*1, gray,  -1, "Now graphics canvas is in single-backbuffer mode");
  WriteCentered (0, 16*0, gray,  -1, "You should not see any flickering now; if this text");
  WriteCentered (0, 16*1, gray,  -1, "flickers, this means that current canvas plugin has");
  WriteCentered (0, 16*2, gray,  -1, "wrong support for single-backbuffer mode.");
}

void G2DTestSystemDriver::DrawCustomCursorScreen ()
{
  int w = myG2D->GetWidth ();
  int h = myG2D->GetHeight ();
  myG2D->SetClipRect(0,0,w,h);
  myG2D->DrawBox(0,0,w,h, dsteel);

  SetFont (fontItalic);
  int tpos = -h / 2;
  WriteCentered (0, tpos, white, -1, "CUSTOM MOUSE CURSOR");

  SetFont (fontLarge);
  WriteCentered (0, tpos + 16*2, black,  -1, "If your current canvas supports custom mouse cursors");
  WriteCentered (0, tpos + 16*3, black,  -1, "you shouldn't see your systems default cursor now.");
}

void G2DTestSystemDriver::DrawAlphaTestScreen ()
{
  int w = myG2D->GetWidth ();
  int h = myG2D->GetHeight ();
  myG2D->SetClipRect(0,0,w,h);
  myG2D->DrawBox(0,0,w,h, dsteel);

  SetFont (fontItalic);
  WriteCentered (1, 1, white, -1, "ALPHA COLOR TEST");

  SetFont (fontLarge);
  WriteCentered (1, 16*2, black, -1, "If your current canvas is in 32-bit mode, you should");
  WriteCentered (1, 16*3, black, -1, "see various text and geometry at various transparencies.");

  myG2D->DrawBox (190, 80, 50, 100, black);
  myG2D->DrawBox (20, 100, 150, 75, myG2D->FindRGB (205, 0, 125, 200));
  myG2D->DrawBox (120, 100, 100, 50, myG2D->FindRGB (120, 50, 50, 100));
  myG2D->DrawLine (30, 110, 120, 60, myG2D->FindRGB (255, 128, 128, 128));
  myG2D->DrawLine (120, 60, 70, 120, myG2D->FindRGB (128, 255, 128, 128));
  myG2D->DrawLine (70, 120, 30, 110, myG2D->FindRGB (128, 128, 255, 128));

  if (alphaBlitImage.IsValid ())
  {
    myG2D->Blit (20, 160, alphaBlitImage->GetWidth (), alphaBlitImage->GetHeight (), 
      (unsigned char*)alphaBlitImage->GetImageData ());
  }

  myG2D->Write (font, 50, 140, myG2D->FindRGB (255, 255, 255, 100), -1,
    "Here is some partially transparent text");
  myG2D->Write (font, 50, 150, myG2D->FindRGB (0, 0, 255, 150), -1,
    "overlaying partially transparent boxes.");

  csString str;
  int i;
  int y = 140;
  int tw, th;
  font->GetMaxSize (tw, th);
  for (i = 0; i < 6; i++)
  {
    const uint8 alpha = (i * 51);
    str.Format ("FG has alpha %" PRIu8 , alpha);
    myG2D->Write (font, 320, y, MakeColor (255, 255, 255, alpha), 
      black, str);
    y += th;
    str.Format ("BG has alpha %" PRIu8, alpha);
    myG2D->Write (font, 320, y, white, MakeColor (0, 0, 0, alpha), 
      str);
    y += th;
  }
}

void G2DTestSystemDriver::DrawUnicodeTest1 ()
{
  int w = myG2D->GetWidth ();
  int h = myG2D->GetHeight ();
  myG2D->SetClipRect(0,0,w,h);
  myG2D->DrawBox(0,0,w,h, dsteel);

  SetFont (fontItalic);
  int tpos = -h / 2;
  WriteCentered (0, tpos, white, -1, "UNICODE TEST 1");

  SetFont (fontLarge);
  WriteCentered (0, tpos + 16*2, black,  -1, "Below you see the equivalent of \"Quick brown fox\"");
  WriteCentered (0, tpos + 16*3, black,  -1, "in several languages.");
  WriteCentered (0, tpos + 16*4, black,  -1, "In the ideal case, all characters should be displayed.");
  WriteCentered (0, tpos + 16*5, black,  -1, "If you see a box in some places, a particular");
  WriteCentered (0, tpos + 16*6, black,  -1, "character is not available in the font.");

  int y = tpos + 16*8;
  int i = 0;
  while (quickBrownFox[i] != 0)
  {
    int fW, fH;
    SetFont (fontCourier);
    WriteCentered (0, y, yellow, -1, quickBrownFox[i + 1]);
    font->GetDimensions (quickBrownFox[i + 1], fW, fH);
    y += fH;

    SetFont (fontLarge);
    font->GetMaxSize (fW, fH);
    int h;
    WriteCenteredWrapped (0, y, h, white, -1, quickBrownFox[i]);
    y += h + fH;
    i += 2;
  }

  SetFont (fontCourier);
  WriteCentered (2, 0, green, -1, "press any key to continue");
}

void G2DTestSystemDriver::DrawUnicodeTest2 ()
{
  int w = myG2D->GetWidth ();
  int h = myG2D->GetHeight ();
  myG2D->SetClipRect(0,0,w,h);
  myG2D->DrawBox(0,0,w,h, dsteel);

  SetFont (fontItalic);
  int tpos = -h / 2;
  WriteCentered (0, tpos, white, -1, "UNICODE TEST 2");

  SetFont (fontLarge);
  WriteCentered (0, tpos + 16*2, black,  -1, "Below you see some translations for \"I can eat glass\".");
  WriteCentered (0, tpos + 16*3, black,  -1, "In the ideal case, all characters should be displayed.");
  WriteCentered (0, tpos + 16*4, black,  -1, "If you see a box in some places, a particular");
  WriteCentered (0, tpos + 16*5, black,  -1, "character is not available in the font.");

  int y = tpos + 16*7;
  int i = 0;
  while (iCanEatGlass[i] != 0)
  {
    int fW, fH;
    SetFont (fontCourier);
    WriteCentered (0, y, yellow, -1, iCanEatGlass[i + 1]);
    font->GetDimensions (iCanEatGlass[i + 1], fW, fH);
    y += fH;

    SetFont (fontLarge);
    font->GetMaxSize (fW, fH);
    int h;
    WriteCenteredWrapped (0, y, h, white, -1, iCanEatGlass[i]);
    y += h + fH;
    i += 2;
  }

  SetFont (fontCourier);
  WriteCentered (2, 0, green, -1, "press any key to continue");
}

void G2DTestSystemDriver::DrawFreetypeTest ()
{
  const char* fontFaces[] = {"DejaVuSans", "DejaVuSansBoldOblique", 
    "DejaVuSansMono", "DejaVuSerif", 0};
  const int fontSizes[] = {4, 8, 12, 24, 0};

  int w = myG2D->GetWidth ();
  int h = myG2D->GetHeight ();
  myG2D->SetClipRect(0,0,w,h);
  myG2D->DrawBox(0,0,w,h, dsteel);

  SetFont (fontItalic);
  int tpos = -h / 2;
  WriteCentered (0, tpos, white, -1, "FREETYPE2 PLUGIN TEST");

  SetFont (fontLarge);
  WriteCentered (0, tpos + 16*2, black,  -1, 
    "If the FreeType2 plugin was built and activated in the");
  WriteCentered (0, tpos + 16*3, black,  -1, 
    "g2dtest.cfg file (it is by default), you should see text");
  WriteCentered (0, tpos + 16*4, black,  -1, 
    "in various faces and sizes below.");

  csRefArray<iFont> fonts;
  // The used fonts are all kept until the end of this function, to provide
  // some more "stress" on the font cache.

  int y = tpos + 16*7;
  int i = 0;
  while (fontFaces[i] != 0)
  {
    csString str;
    int j = 0;
    while (fontSizes[j] != 0)
    {
      int fW, fH;
      csRef<iFont> font = GetFont (fontFaces[i], fontSizes[j]);
      if (font)
      {
	fonts.Push (font);
	SetFont (font);
	str.Clear ();
        str << fontFaces[i] << ", Size " << fontSizes[j];
        WriteCentered (0, y, yellow, -1, str.GetData ());
        font->GetDimensions (str.GetData (), fW, fH);
        y += fH + 4;
      }
      j++;
    }
    i++;
  }

  SetFont (fontCourier);
  WriteCentered (2, 0, green, -1, "press any key to continue");
}

void G2DTestSystemDriver::DrawLineTest ()
{
#if 0

  // some tests for some special kinds of lines
  myG2D->DrawLine (0, 0, 200, 200, yellow);
  myG2D->DrawLine (0, 0, 205, 200, red);
  myG2D->DrawLine (0, 0, 195, 200, green);

  int w = myG2D->GetWidth ();
  myG2D->DrawLine (0, 250, w / 3, 250, red);
  myG2D->DrawLine (w / 3, 250.1, w * 2 / 3, 250.1, red);
  myG2D->DrawLine (w * 2 / 3, 250.99, w, 250.99, red);

  myG2D->DrawLine (200, myG2D->GetHeight () - 200, 0, myG2D->GetHeight (), blue);

  myG2D->DrawLine (81, 221, 519, 221, white);

#else

  SetFont (fontItalic);
  WriteCentered (0,-16*5, white, -1, "LINE DRAWING TEST");
  SetFont (fontLarge);
  WriteCentered (0,-16*3, gray,  -1, "At the top of the screen you should see a sinusoid,");
  WriteCentered (0,-16*2, gray,  -1, "each point on sinusoid should be connected with the");
  WriteCentered (0,-16*1, gray,  -1, "top-left corner of the canvas.");

  float py = -1;
  int a;
  for (a = 0; a <= myG2D->GetWidth (); a += 8)
  {
    float angle = float (a) / 30;
    float y = int (80 + sin (angle) * 60);
    if (py > 0)
      myG2D->DrawLine (a - 8, py, a, y, red);
    myG2D->DrawLine (0, 0, a, y, yellow);
    py = y;
  }

  WriteCentered (0, 16*1, gray,  -1, "At the bottom of the screen you should see several");
  WriteCentered (0, 16*2, gray,  -1, "lines interruped by a white pixel in the middle.");

  int w = myG2D->GetWidth ();
  float x = (w / 2) + 0.5;
  float y = myG2D->GetHeight () - 50.5;
  myG2D->DrawPixel (int(x), int(y), white);
  myG2D->DrawLine (0, y - 0.5, x - 0.5, y - 0.5, red);
  myG2D->DrawLine (x + 0.5, y + 0.49, w, y + 0.49, red);

  // Compute the slope for a line that is going through (x,y)
  float y1 = y - 5;
  float y2 = y + 5;
  float dy = float (y2 - y1) / float (w);
  float y11 = y1 + (x - 0.5 ) * dy;
  float y12 = y1 + (x + 0.5) * dy;
  myG2D->DrawLine (0, y1, x - 0.5, y11, blue);
  myG2D->DrawLine (x + 0.5, y12, w, y2, blue);

  myG2D->DrawLine (x, y - 20, x, y - 0.5, gray);
  myG2D->DrawLine (x, y + 0.5, x, y + 20, gray);

  WriteCentered (0, 16*4, gray,  -1, "A little above you should see four adjanced horizontal");
  WriteCentered (0, 16*5, gray,  -1, "lines of blue, green, red and yellow colors.");

  myG2D->DrawLine (0, y - 43 - 0.5,  w - 0.9, y - 43,        blue);
  myG2D->DrawLine (0, y - 42 + 0.49, w + 0.9, y - 42,        green);
  myG2D->DrawLine (0, y - 41,        w,       y - 41 - 0.5,  red);
  myG2D->DrawLine (0, y - 40,        w - 0.5, y - 40 + 0.49, yellow);
#endif
}

void G2DTestSystemDriver::DrawLinePerf ()
{
  SetFont (fontItalic);
  WriteCentered (0,-16*4, white, -1, "LINE SLOPE AND PERFORMANCE TEST");

  int w2 = myG2D->GetWidth () / 2;
  int colors [4] = { red, green, blue, yellow };
  int a;
  for (a = 0; a < 360; a += 5)
  {
    float angle = (a * TWO_PI) / 360.0;
    float x = w2 + 80 * cos (angle);
    float y = 100 + 80 * sin (angle);
    myG2D->DrawLine (w2, 100, x, y, colors [a & 3]);
  }

  // Compute the size for the random lines box
  int sx = 0;
  int sw = myG2D->GetWidth ();
  int sy = myG2D->GetHeight () / 2;
  int sh = sy;
  myG2D->DrawBox (sx, sy + 16, sw, sh - 16, dsteel);

  SetFont (fontLarge);
  WriteCentered (0,-16*2, gray,  -1, "Above this text you should see a uniformly hashed circle,");
  WriteCentered (0,-16*1, gray,  -1, "while below you should see some random lines, and the");
  WriteCentered (0, 16*0, gray,  -1, "measured line drawing performance in pixels per second.");

  // Test line drawing performance for 1/4 seconds
  sx += 20; sw -= 40;
  sy += 30; sh -= 40;
  csRandomGen rng (csGetTicks ());
  csTicks start_time = csGetTicks (), delta_time;
  float pix_count = 0;
  do
  {
    for (a = 0; a < 5000; a++)
    {
      float x1 = sx + rng.Get () * sw;
      float y1 = sy + rng.Get () * sh;
      float x2 = sx + rng.Get () * sw;
      float y2 = sy + rng.Get () * sh;
      myG2D->DrawLine (x1, y1, x2, y2, colors [rng.Get (4)]);
      x2 = csQint (x2 - x1); y2 = csQint (y2 - y1);
      pix_count += csQsqrt (x2 * x2 + y2 * y2);
    }
    myG2D->PerformExtension ("flush");
    delta_time = csGetTicks () - start_time;
  } while (delta_time < 500);
  pix_count = pix_count * (1000.0 / delta_time);
  WriteCentered (0, 16*1, green, black, " Performance: %20.1f pixels/second ", pix_count);
}

void G2DTestSystemDriver::DrawTextTest ()
{
  // Draw a grid of lines so that transparent text background will be visible
  int w = myG2D->GetWidth ();
  int h = myG2D->GetHeight ();
  int i;
  for (i = 0; i < w; i += 4)
  {
    myG2D->DrawLine (float(i), 0.0f, float(i) + 50.0f, float(h), dsteel);
    myG2D->DrawLine (float(w - i), 0.0f, float(w - i) - 50.0f, float(h), dsteel);
  }

  SetFont (fontItalic);
  WriteCentered (0,-16*7, white, -1, "TEXT DRAWING TEST");

  SetFont (fontLarge);
  WriteCentered (0,-16*5,   blue,    -1, "This is blue text with transparent background");
  WriteCentered (0,-16*4,  green,  blue, "This is green text on blue background");
  WriteCentered (0,-16*3, yellow,  gray, "Yellow text on gray background");
  WriteCentered (0,-16*2,    red, black, "Red text on black background");
  WriteCentered (0,-16*1,  black, white, "Black text on white background");

  SetFont (fontCourier);
  int sx = 0, sy = h / 2 + 48, sw = w, sh = h / 2 - 48;
  myG2D->DrawBox (sx, sy, sw, sh, dsteel);
  const char *text = "Crystal Space rulez";
  int tw, th;
  font->GetDimensions (text, tw, th);
  size_t cc = strlen (text);

  // Test text drawing performance for 1/4 seconds
  int colors [4] = { red, green, blue, yellow };
  sx += 20; sw -= 40 + tw;
  sy += 10; sh -= 20 + th;
  csRandomGen rng (csGetTicks ());
  csTicks start_time = csGetTicks (), delta_time;
  size_t char_count = 0;
  do
  {
    for (i = 0; i < 2000; i++)
    {
      float x = sx + rng.Get () * sw;
      float y = sy + rng.Get () * sh;
      myG2D->Write (font, int(x), int(y), colors [rng.Get (4)], black, text);
      char_count += cc;
    }
    myG2D->PerformExtension ("flush");
    delta_time = csGetTicks () - start_time;
  } while (delta_time < 500);
  float perf = char_count * (1000.0f / delta_time);
  SetFont (fontLarge);
  WriteCentered (0, 16*1, green, black, " Performance: %20.1f characters/second ", perf);
}

void G2DTestSystemDriver::DrawTextTest2 ()
{
  // Draw a grid of lines so that transparent text background will be visible
  int w = myG2D->GetWidth ();
  int h = myG2D->GetHeight ();
  int x;
  for (x = 0; x < w; x += 4)
  {
    myG2D->DrawLine (float(x), 0.0f, float(x) + 50.0f, float(h), dsteel);
    myG2D->DrawLine (float(w - x), 0.0f, float(w - x) - 50.0f, float(h), dsteel);
  }

  SetFont (fontItalic);
  WriteCentered (0,-16*4, white, -1, "TEXT DRAWING TEST");

  SetFont (fontLarge);
  WriteCentered (0,-16*2,   gray, black, "This is a benchmark of text drawing");
  WriteCentered (0,-16*1,   gray, black, "with transparent background");

  SetFont (fontCourier);
  int sx = 0, sy = h / 2 + 48, sw = w, sh = h / 2 - 48;
  myG2D->DrawBox (sx, sy, sw, sh, dsteel);
  const char *text = "Crystal Space rulez";
  int tw,th;
  font->GetDimensions (text, tw, th);
  size_t cc = strlen (text);

  // Test text drawing performance for 1/4 seconds
  int colors [4] = { red, green, blue, yellow };
  sx += 20; sw -= 40 + tw;
  sy += 10; sh -= 20 + th;
  csRandomGen rng (csGetTicks ());
  csTicks start_time = csGetTicks (), delta_time;
  size_t char_count = 0;
  do
  {
	int i;
    for (i = 0; i < 2000; i++)
    {
      float x = sx + rng.Get () * sw;
      float y = sy + rng.Get () * sh;
      myG2D->Write (font, int(x), int(y), colors [rng.Get (4)], -1, text);
      char_count += cc;
    }
    myG2D->PerformExtension ("flush");
    delta_time = csGetTicks () - start_time;
  } while (delta_time < 500);
  float perf = char_count * (1000.0f / delta_time);
  SetFont (fontLarge);
  WriteCentered (0, 16*1, green, black, " Performance: %20.1f characters/second ", perf);
}


void G2DTestSystemDriver::PixelClipTest ()
{
  int w = myG2D->GetWidth ();
  int h = myG2D->GetHeight ();
  int sx = w/4, sy = h / 2 + 60, sw = w/2, sh = h / 4 - 60;
  myG2D->SetClipRect(0,0,w,h);
  myG2D->DrawBox(0,0,w,h, dsteel);
  


  SetFont (fontItalic);
  WriteCentered (0,16*-12, white, -1, "PIXEL CLIP TEST");

  SetFont (fontLarge);
  WriteCentered (0,16*-10,  black, dsteel, "This will test if pixel clipping is being done properly");
  
  WriteCentered (0,16*-8,   black, dsteel, "For each of the following clip tests we will be drawing");
  WriteCentered (0,16*-7,   black, dsteel, "a 1 pixel wide green rectangle with a 1 pixel wide red rectangle");
  WriteCentered (0,16*-6,   black, dsteel, "inside of it.");

  WriteCentered (0,16*-4,   black, dsteel, "The clipping rectangle has been set so the red rectangle is");
  WriteCentered (0,16*-3,   black, dsteel, "inside the clipping region. If any of the lines of the red rectangle");
  WriteCentered (0,16*-2,   black, dsteel, "are solid (not drawn over) then the clipping region is cutting off too much");
  
  WriteCentered (0,16*0,   black, dsteel, "The green rectangle is outside the clipping region. If any of the lines");
  WriteCentered (0,16*1,   black, dsteel, "of the green rectangle are being drawn over then the clipping region is");
  WriteCentered (0,16*2,   black, dsteel, "not clipping enough.");
  

  SetFont (fontCourier);

  DrawClipRect(sx, sy, sw, sh);

  myG2D->SetClipRect(sx + 1, sy + 1, sx + sw, sy + sh);
  

  // Test random pixel drawing
  csRandomGen rng (csGetTicks ());
  csTicks start_time = csGetTicks (), delta_time;


  // widen the range where we try to draw pixels
  sx -= 10;
  sy -= 10;
  sw += 20;
  sh += 20;

  do
  {
	int i;
    for (i = 0; i < 1000; i++)
    {
      int x = int(sx + rng.Get () * sw);
      int y = int(sy + rng.Get () * sh);
      myG2D->DrawPixel(x,y,black);
    }
    delta_time = csGetTicks () - start_time;
  } while (delta_time < 100);
}


void G2DTestSystemDriver::DrawClipRect(int sx, int sy, int sw, int sh)
{
  myG2D->DrawLine (sx, sy, sx + sw, sy, green);
  myG2D->DrawLine (sx, sy + sh, sx + sw, sy + sh, green);
  myG2D->DrawLine (sx, sy, sx, sy + sh, green);
  myG2D->DrawLine (sx + sw, sy, sx + sw, sy + sh, green);
  myG2D->DrawLine (sx+1, sy+1, sx + sw-1, sy+1, red);
  myG2D->DrawLine (sx+1, sy + sh-1, sx + sw-1, sy + sh-1, red);
  myG2D->DrawLine (sx+1, sy+1, sx+1, sy + sh-1, red);
  myG2D->DrawLine (sx + sw - 1, sy+1, sx + sw - 1, sy + sh-1, red);
}


void G2DTestSystemDriver::LineClipTest ()
{
  int w = myG2D->GetWidth ();
  int h = myG2D->GetHeight ();
    myG2D->SetClipRect(0,0,w,h);
  myG2D->DrawBox(0,0,w,h, dsteel);


  SetFont (fontItalic);
  WriteCentered (0,-16*4, white, -1, "LINE CLIP TEST");

  SetFont (fontLarge);
  WriteCentered (0,-16*1,  black, dsteel, "This will test if line clipping is being done properly");
  WriteCentered (0,0,   black, dsteel, "You should see 3 thin green rectangles below with black");
  WriteCentered (0,16*1,   black, dsteel, "inside each. Like before we want no black on the green while the");
  WriteCentered (0,16*2,   black, dsteel, "red should be covered. The first box is drawing horizontal lines, ");
  WriteCentered (0,16*3,   black, dsteel, "the second, vertical lines, and the third, random diagonal lines.");
  

  SetFont (fontCourier);
  int sx1 = w/7, sx2 = 3*sx1, sx3 = 5*sx1, sy = h / 2 + 60, sw = w/7, sh = h / 4 - 60;
  DrawClipRect(sx1, sy, sw, sh);
  DrawClipRect(sx2, sy, sw, sh);
  DrawClipRect(sx3, sy, sw, sh);



  // Test random pixel drawing
  csRandomGen rng (csGetTicks ());
  csTicks start_time = csGetTicks (), delta_time;


  // widen the range where we try to draw pixels
  int sx1_big = sx1 - 10;
  int sx2_big = sx2 - 10;
  int sx3_big = sx3 - 10;
  int sy_big = sy - 10;
  int sw_big = sw + 20;
  int sh_big = sh + 20;

  do
  {
	int i;

    myG2D->SetClipRect(sx1 + 1, sy + 1, sx1 + sw, sy + sh);
    for (i = 0; i < 10; i++)
    {
      float x1 = sx1_big + rng.Get () * sw_big;
      float x2 = sx1_big + rng.Get () * sw_big;
      float y = sy_big + rng.Get () * sh_big;
      myG2D->DrawLine(x1,y,x2,y,black);
    }

    myG2D->SetClipRect(sx2 + 1, sy + 1, sx2 + sw, sy + sh);
    for (i = 0; i < 100; i++)
    {
      float x = sx2_big + rng.Get () * sw_big;
      float y1 = sy_big + rng.Get () * sh_big;
      float y2 = sy_big + rng.Get () * sh_big;
      myG2D->DrawLine(x,y1,x,y2,black);
    }

    myG2D->SetClipRect(sx3 + 1, sy + 1, sx3 + sw, sy + sh);
    for (i = 0; i < 100; i++)
    {
      float x1 = sx3_big + rng.Get () * sw_big;
      float y1 = sy_big + rng.Get () * sh_big;
      float x2 = sx3_big + rng.Get () * sw_big;
      float y2 = sy_big + rng.Get () * sh_big;
      myG2D->DrawLine(x1,y1,x2,y2,black);
    }
    delta_time = csGetTicks () - start_time;
  } while (delta_time < 100);

} 

void G2DTestSystemDriver::BoxClipTest()
{
  int w = myG2D->GetWidth ();
  int h = myG2D->GetHeight ();
  int sx = w/4, sy = h / 2 + 60, sw = w/2, sh = h / 4 - 60;
  myG2D->SetClipRect(0,0,w,h);
  myG2D->DrawBox(0,0,w,h, dsteel);
  
  SetFont (fontItalic);
  WriteCentered (0,-16*4, white, -1, "BOX CLIP TEST");

  SetFont (fontLarge);
  WriteCentered (0,-16*3,  black, dsteel, "This will test if box clipping is being done properly");
  WriteCentered (0,-16*2,  black, dsteel, "You should see a thin green rectangle below");

  WriteCentered (0,16*0,   black, dsteel, "Again all the black should be contained inside the green");
  WriteCentered (0,16*1,   black, dsteel, "rectangle. The red rectangle should not be visible.");
  

  SetFont (fontCourier);

  DrawClipRect(sx, sy, sw, sh);

  myG2D->SetClipRect(sx + 1, sy + 1, sx + sw, sy + sh);
  

  // Test random box drawing
  csRandomGen rng (csGetTicks ());
  csTicks start_time = csGetTicks (), delta_time;


  // widen the range where we try to draw
  sx -= 10;
  sy -= 10;
  sw += 20;
  sh += 20;

  do
  {
	int i;
    for (i = 0; i < 1000; i++)
    {
      int x = int(sx + rng.Get () * sw);
      int y = int(sy + rng.Get () * sh);
      int width = int(rng.Get () * sw);
      int height = int(rng.Get () * sh);
      myG2D->DrawBox(x,y,width,height,black);
    }
    delta_time = csGetTicks () - start_time;
  } while (delta_time < 100);

}

void G2DTestSystemDriver::FontClipTest()
{
  int w = myG2D->GetWidth ();
  int h = myG2D->GetHeight ();
  int sx = w/10, sy = h / 2 + 60, sw = sx * 2, sh = h / 6 - 60;
  int sx1 = sx * 1, sx2 = sx * 4, sx3 = sx * 7;
  myG2D->SetClipRect(0,0,w,h);
  myG2D->DrawBox(0,0,w,h, dsteel);
  
  SetFont (fontItalic);
  WriteCentered (0,-16*8, white, -1, "FONT CLIP TEST");

  SetFont (fontLarge);
  WriteCentered (0,-16*7, black, dsteel, "This will test if font clipping is being done properly");
  WriteCentered (0,-16*6, black, dsteel, "You should see three thin green rectangles below");

  WriteCentered (0,-16*4, black, dsteel, "Again all the black should be contained inside the first");
  WriteCentered (0,-16*3, black, dsteel, "green rectangle. The red rectangle should not be visible.");
  
  WriteCentered (0,-16*1, black, dsteel, "The second and third green rectangles shouldn't be crossed as well,");
  WriteCentered (0, 16*0, black, dsteel, "the text should only overdraw the red rectangle. Additionally,");
  WriteCentered (0, 16*1, black, dsteel, "all the text should look the same (well, except for the parts cut off.)");

  SetFont (fontCourier);

  const char* testText = "CrystalSpace";
  int fW, fH;

  font->GetDimensions (testText, fW, fH);

  int fX = -fW /2, fY = -fH / 2;

  DrawClipRect(sx2, sy, sw, sh);
  myG2D->SetClipRect(sx2 + 1, sy + 1, sx2 + sw, sy + sh);

  myG2D->Write (font, sx2 + fX,          sy + fY,          black, -1, testText);
  myG2D->Write (font, sx2 + sw / 2 + fX, sy + fY,          black, -1, testText);
  myG2D->Write (font, sx2 + sw + fX,     sy + fY,          black, -1, testText);

  myG2D->Write (font, sx2 + fX,          sy + sh / 2 + fY, black, -1, testText);
  myG2D->Write (font, sx2 + sw / 2 + fX, sy + sh / 2 + fY, black, -1, testText);
  myG2D->Write (font, sx2 + sw + fX,     sy + sh / 2 + fY, black, -1, testText);

  myG2D->Write (font, sx2 + fX,          sy + sh + fY,     black, -1, testText);
  myG2D->Write (font, sx2 + sw / 2 + fX, sy + sh + fY,     black, -1, testText);
  myG2D->Write (font, sx2 + sw + fX,     sy + sh + fY,     black, -1, testText);

  myG2D->SetClipRect(0,0,w,h);
  DrawClipRect(sx3, sy, sw, sh);
  myG2D->SetClipRect(sx3 + 1, sy + 1, sx3 + sw, sy + sh);

  myG2D->Write (font, sx3 + fX,          sy + fY,          black, blue, testText);
  myG2D->Write (font, sx3 + sw / 2 + fX, sy + fY,          black, blue, testText);
  myG2D->Write (font, sx3 + sw + fX,     sy + fY,          black, blue, testText);

  myG2D->Write (font, sx3 + fX,          sy + sh / 2 + fY, black, blue, testText);
  myG2D->Write (font, sx3 + sw / 2 + fX, sy + sh / 2 + fY, black, blue, testText);
  myG2D->Write (font, sx3 + sw + fX,     sy + sh / 2 + fY, black, blue, testText);

  myG2D->Write (font, sx3 + fX,          sy + sh + fY,     black, blue, testText);
  myG2D->Write (font, sx3 + sw / 2 + fX, sy + sh + fY,     black, blue, testText);
  myG2D->Write (font, sx3 + sw + fX,     sy + sh + fY,     black, blue, testText);

  myG2D->SetClipRect(0,0,w,h);
  DrawClipRect(sx1, sy, sw, sh);
  myG2D->SetClipRect(sx1 + 1, sy + 1, sx1 + sw, sy + sh);

  // Test random text drawing
  csRandomGen rng (csGetTicks ());
  csTicks start_time = csGetTicks (), delta_time;

  // widen the range where we try to draw
  sx -= fW;
  sy -= fH;
  sw += fW * 2;
  sh += fH * 2;

  do
  {
    int i;
    for (i = 0; i < 1000; i++)
    {
      int x = int(sx + rng.Get () * sw);
      int y = int(sy + rng.Get () * sh);
      myG2D->Write (font, x, y, black, blue, testText);
    }
    delta_time = csGetTicks () - start_time;
  } while (delta_time < 100);
}

void G2DTestSystemDriver::BlitTest ()
{
  int w = myG2D->GetWidth ();
  int h = myG2D->GetHeight ();
  myG2D->SetClipRect(0,0,w,h);
  myG2D->DrawBox(0,0,w,h, dsteel);
  
  SetFont (fontItalic);
  WriteCentered (0,-16*8, white, -1, "BLIT() TEST");

  SetFont (fontLarge);
  WriteCentered (0,-16*7, black, dsteel, "This will test whether iGraphics2D->Blit() works correctly");
  WriteCentered (0,-16*6, black, dsteel, "on this canvas.");

  WriteCentered (0,-16*4, black, dsteel, "You should an image of an arrow and the word \"up\".");
  WriteCentered (0,-16*3, black, dsteel, "It is surrounded by a green rectangle, and the image");
  WriteCentered (0,-16*2, black, dsteel, "itself has a black border. No red should be visible");
  WriteCentered (0,-16*1, black, dsteel, "and the border has to be complete, too.");

  if (blitTestImage.IsValid ())
  {
    const int imW = blitTestImage->GetWidth ();
    const int imH = blitTestImage->GetHeight ();
    const int bx = (w - imW) / 2;
    const int by = h / 2;
    DrawClipRect (bx, by, imW + 1, imH + 1);
    myG2D->Blit (bx + 1, by + 1, imW, imH, 
      (unsigned char*)blitTestImage->GetImageData ());
  }
}

static bool G2DEventHandler (iEvent& ev)
{
  if (ev.Type == csevBroadcast && csCommandEventHelper::GetCode(&ev) == cscmdProcess)
  {
    Sys->SetupFrame ();
    return true;
  }
  else if (ev.Type == csevBroadcast && csCommandEventHelper::GetCode(&ev) == cscmdFinalProcess)
  {
    Sys->FinishFrame ();
    return true;
  }
  else
  {
    return Sys ? Sys->HandleEvent (ev) : false;
  }
}

int main (int argc, char *argv[])
{
  G2DTestSystemDriver System (argc, argv);
  Sys = &System;
  iObjectRegistry* object_reg = System.object_reg;

  if (!csInitializer::SetupEventHandler (object_reg, G2DEventHandler))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.g2dtest",
        "Unable to init app!");
    return false;
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper::Help (object_reg);
    exit (0);
  }

  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));
  csRef<iCommandLineParser> cmdline (CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser));

  System.myG3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  // Now load the renderer plugin
  if (!System.myG3D)
  {
    csString canvas = cmdline->GetOption ("video");
    if (!canvas || !*canvas)
      canvas = "crystalspace.graphics3d.software"; //CS_SOFTWARE_2D_DRIVER;
    else if (strncmp ("crystalspace.", canvas, 13))
    {
      canvas = "crystalspace.graphics3d." + canvas;
    }
    System.myG3D = CS_LOAD_PLUGIN (plugin_mgr, canvas, iGraphics3D);
    if (!object_reg->Register (System.myG3D, "iGraphics3D"))
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	  "crystalspace.application.g2dtest",
	  "Unable to register renderer!");
      return -1;
    }
  }

  if (!System.myG3D)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.g2dtest",
	"Unable to load canvas driver!");
    return -1;
  }
  System.myG2D = System.myG3D->GetDriver2D ();
    
  if (!csInitializer::OpenApplication (object_reg))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.g2dtest",
        "Unable to open drivers!");
    return -1;
  }
  
  System.cursorPlugin = CS_QUERY_REGISTRY(object_reg, iCursor);
  if (System.cursorPlugin)
  {
    csRef<iConfigManager> cfg (CS_QUERY_REGISTRY (object_reg, iConfigManager));
    if (System.cursorPlugin->Setup (System.myG3D))
    {
      System.cursorPlugin->ParseConfigFile ((iConfigManager*)cfg);
    }
    else
      System.cursorPlugin = 0;
  }

  iNativeWindow* nw = System.myG2D->GetNativeWindow ();
  if (nw) nw->SetTitle (APP_TITLE);

  csDefaultRunLoop(object_reg);

  System.myG2D = 0;
  System.myG3D->Close();
  System.myG3D = 0;
  plugin_mgr = 0;
  cmdline = 0;

  return 0;
}

