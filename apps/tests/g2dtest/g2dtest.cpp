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

#define CS_SYSDEF_PROVIDE_ALLOCA
#define CS_SYSDEF_PROVIDE_SOFTWARE2D
#include "cssysdef.h"
#include "cssys/sysfunc.h"
#include "csutil/csvector.h"
#include "csutil/rng.h"
#include "cstool/initapp.h"
#include "ivaria/reporter.h"
#include "qint.h"
#include "qsqrt.h"

#include "iutil/vfs.h"
#include "iutil/plugin.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivideo/graph2d.h"
#include "ivideo/fontserv.h"
#include "csutil/cmdhelp.h"
#include "iutil/cmdline.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"

CS_IMPLEMENT_APPLICATION

#define APP_TITLE	"Graphics canvas plugin test"

class G2DTestSystemDriver;
G2DTestSystemDriver* Sys;

class G2DTestSystemDriver
{
  // Application states
  enum appState
  {
    stStartup,
    stContextInfo,
    stWindowFixed,
    stWindowResize,
    stBackBufferON,
    stBackBufferOFF,
    stTestLineDraw,
    stTestLinePerf,
    stTestTextDraw,
    stTestTextDraw2,
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
  int lastkey, lastkey2, lastkey3, lastkey4;
  // Switch backbuffer while waiting for a key
  bool SwitchBB;
  // Current font
  iFont *font;
  // Event Outlet
  iEventOutlet* EventOutlet;

public:
  iGraphics2D *myG2D;
  iObjectRegistry* object_reg;

public:
  G2DTestSystemDriver (int argc, char* argv[]);
  virtual ~G2DTestSystemDriver ();
  void SetupFrame ();
  void FinishFrame ();
  bool HandleEvent (iEvent &Event);

private:
  void SetFont (const char *fontID);

  void EnterState (appState newstate, int arg = 0);
  void LeaveState ();

  int MakeColor (int r, int g, int b);
  void WriteCentered (int mode, int dy, int fg, int bg, char *format, ...);

  void ResizeContext ();

  void DrawStartupScreen ();
  void DrawContextInfoScreen ();
  void DrawWindowScreen ();
  void DrawWindowResizeScreen ();
  void DrawBackBufferText ();
  void DrawBackBufferON ();
  void DrawBackBufferOFF ();
  void DrawLineTest ();
  void DrawLinePerf ();
  void DrawTextTest ();
  void DrawTextTest2 ();
};

G2DTestSystemDriver::G2DTestSystemDriver (int argc, char* argv[])
{
  state_sptr = 0;
  EnterState (stStartup);
  SwitchBB = false;
  font = NULL;
  pfmt_init = false;

  object_reg = csInitializer::CreateEnvironment (argc, argv);

  if (!csInitializer::SetupConfigManager (object_reg, NULL))
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

  iEventQueue* q = CS_QUERY_REGISTRY(object_reg, iEventQueue);
  if (q != 0)
  {
    EventOutlet = q->GetEventOutlet();
    EventOutlet->IncRef();
    q->DecRef ();
  }
}

G2DTestSystemDriver::~G2DTestSystemDriver ()
{
  if (EventOutlet != 0)
    EventOutlet->DecRef();
  if (font != 0)
    font->DecRef();
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
    black = 0;
  }

  if (state_sptr == 0)
  {
    EventOutlet->Broadcast (cscmdQuit);
    return;
  }

  appState curstate = state [state_sptr - 1];
  switch (curstate)
  {
    case stStartup:
    case stContextInfo:
    case stWindowFixed:
    case stWindowResize:
    case stBackBufferON:
    case stBackBufferOFF:
    case stTestLineDraw:
    case stTestLinePerf:
    case stTestTextDraw:
    case stTestTextDraw2:
    {
      if (!myG2D->BeginDraw ())
        break;

      myG2D->Clear (black);
      LeaveState ();
      switch (curstate)
      {
        case stStartup:
          DrawStartupScreen ();
//EnterState (stTestTextDraw);
//break;
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
          EnterState (stTestLineDraw);
          if (myG2D->DoubleBuffer (false))
          {
            DrawBackBufferOFF ();
            SwitchBB = true;
            EnterState (stWaitKey);
          }
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
            ;//EnterState ();
          else
            EnterState (stTestTextDraw2);
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
          myG2D->Print (NULL);
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
  myG2D->FinishDraw ();
  myG2D->Print (NULL);
}

bool G2DTestSystemDriver::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevBroadcast:
      switch (Event.Command.Code)
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
    case csevKeyDown:
      if (state_sptr)
        switch (state [state_sptr - 1])
        {
          case stWaitKey:
            lastkey = Event.Key.Char;
            break;
          case stTestLinePerf:
            lastkey2 = Event.Key.Char;
            break;
          case stTestTextDraw:
            lastkey3 = Event.Key.Char;
            break;
          case stTestTextDraw2:
            lastkey4 = Event.Key.Char;
            break;
          default:
            break;
        }
      break;
  }
  return false;
}

int G2DTestSystemDriver::MakeColor (int r, int g, int b)
{
  if (!pfmt.PalEntries)
    return ((r >> (8 - pfmt.RedBits)) << pfmt.RedShift)
         | ((g >> (8 - pfmt.GreenBits)) << pfmt.GreenShift)
         | ((b >> (8 - pfmt.BlueBits)) << pfmt.BlueShift);

  // In paletted mode this is easy since we have a uniform 3-3-2 palette
  return ((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6);
}

void G2DTestSystemDriver::SetFont (const char *iFontID)
{
  if (font) font->DecRef ();
  iFontServer *fs = myG2D->GetFontServer ();
  font = fs->LoadFont (iFontID);
}

void G2DTestSystemDriver::WriteCentered (int mode, int dy, int fg, int bg,
  char *format, ...)
{
  char text [1024];
  va_list arg;

  va_start (arg, format);
  vsprintf (text, format, arg);
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

  myG2D->Write (font, x, y, fg, bg, text);
}

void G2DTestSystemDriver::DrawStartupScreen ()
{
  myG2D->DrawBox (20, 20, myG2D->GetWidth () - 40, myG2D->GetHeight () - 40, blue);

  SetFont (CSFONT_ITALIC);
  WriteCentered (0, -20, white, -1, "WELCOME");
  SetFont (CSFONT_LARGE);
  WriteCentered (0,   0, white, -1, "to graphics canvas plugin");
  WriteCentered (0, +20, white, -1, "test application");

  SetFont (CSFONT_COURIER);
  WriteCentered (2, 0, green, -1, "please wait five seconds");
}

void G2DTestSystemDriver::DrawContextInfoScreen ()
{
  SetFont (CSFONT_LARGE);

  WriteCentered (0,-16*3, white, -1, "Some information about graphics context");
  WriteCentered (0,-16*2, gray,  -1, "Screen size: %d x %d", myG2D->GetWidth (), myG2D->GetHeight ());
  char pixfmt [50];
  if (pfmt.PalEntries)
    sprintf (pixfmt, "%d colors (Indexed)", pfmt.PalEntries);
  else
    sprintf (pixfmt, "R%dG%dB%d", pfmt.RedBits, pfmt.GreenBits, pfmt.BlueBits);
  WriteCentered (0,-16*1, gray,  -1, "Pixel format: %d BPP, %s", pfmt.PixelBytes * 8, pixfmt);

  if (pfmt.PalEntries)
    sprintf (pixfmt, "not available");
  else
    sprintf (pixfmt, "R[%08lX] G[%08lX] B[%08lX] ", pfmt.RedMask, pfmt.GreenMask, pfmt.BlueMask);
  WriteCentered (0, 16*0, gray,  -1, "R/G/B masks: %s", pixfmt);

  WriteCentered (0, 16*1, gray,  -1, "More than one backbuffer available: %s",
    myG2D->GetDoubleBufferState () ? "yes" : "no");
  int MinX, MinY, MaxX, MaxY;
  myG2D->GetClipRect (MinX, MinY, MaxX, MaxY);
  WriteCentered (0, 16*2, gray,  -1, "Current clipping rectangle: %d,%d - %d,%d", MinX, MinY, MaxX, MaxY);

  SetFont (CSFONT_COURIER);
  WriteCentered (2, 0, green, -1, "press any key to continue");
}

void G2DTestSystemDriver::DrawWindowScreen ()
{
  SetFont (CSFONT_LARGE);
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

  SetFont (CSFONT_COURIER);
  WriteCentered (2, 0, green, -1, "press any key to continue");
}

void G2DTestSystemDriver::DrawWindowResizeScreen ()
{
  DrawWindowScreen ();

  myG2D->AllowResize (true);
  myG2D->DrawBox (0, myG2D->GetHeight () / 2 + 16, myG2D->GetWidth (), 16 * 4, blue);
  SetFont (CSFONT_LARGE);

  WriteCentered (0, 16*1, white, -1, "Now resizing should be enabled. Try to resize the");
  WriteCentered (0, 16*2, white, -1, "window: you should be either unable to do it (if");
  WriteCentered (0, 16*3, white, -1, "canvas driver does not support resize) or see");
  WriteCentered (0, 16*4, white, -1, "the current window size in top-right corner.");

  SetFont (CSFONT_COURIER);
  WriteCentered (2, 0, green, -1, "press any key to continue");
}

void G2DTestSystemDriver::ResizeContext ()
{
  if (!myG2D->BeginDraw ())
    return;

  myG2D->Clear (black);
  DrawWindowResizeScreen ();

  char text [50];
  sprintf (text, "Canvas [%d x %d]", myG2D->GetWidth (), myG2D->GetHeight ());
  SetFont (CSFONT_LARGE);
  int fw, fh;
  font->GetDimensions (text, fw, fh);
  int x = myG2D->GetWidth () - fw;
  myG2D->Write (font, x, 0, red, -1, text);

  myG2D->FinishDraw ();
  myG2D->Print (NULL);
}

void G2DTestSystemDriver::DrawBackBufferText ()
{
  SetFont (CSFONT_ITALIC);
  WriteCentered (0,-16*5, white, -1, "DOUBLE BACK BUFFER TEST");
  SetFont (CSFONT_LARGE);
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
  myG2D->Print (NULL);

  if (!myG2D->BeginDraw ())
    return;
  myG2D->Clear (red);
  DrawBackBufferText ();
}

void G2DTestSystemDriver::DrawBackBufferOFF ()
{
  myG2D->Clear (white);
  myG2D->FinishDraw ();
  myG2D->Print (NULL);
  if (!myG2D->BeginDraw ())
    return;

  myG2D->Clear (black);

  SetFont (CSFONT_ITALIC);
  WriteCentered (0,-16*3, white, -1, "SINGLE BACK BUFFER TEST");
  SetFont (CSFONT_LARGE);
  WriteCentered (0,-16*1, gray,  -1, "Now graphics canvas is in single-backbuffer mode");
  WriteCentered (0, 16*0, gray,  -1, "You should not see any flickering now; if this text");
  WriteCentered (0, 16*1, gray,  -1, "flickers, this means that current canvas plugin has");
  WriteCentered (0, 16*2, gray,  -1, "wrong support for single-backbuffer mode.");
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

  SetFont (CSFONT_ITALIC);
  WriteCentered (0,-16*5, white, -1, "LINE DRAWING TEST");
  SetFont (CSFONT_LARGE);
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
  SetFont (CSFONT_ITALIC);
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

  SetFont (CSFONT_LARGE);
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
      x2 = QInt (x2 - x1); y2 = QInt (y2 - y1);
      pix_count += qsqrt (x2 * x2 + y2 * y2);
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

  SetFont (CSFONT_ITALIC);
  WriteCentered (0,-16*7, white, -1, "TEXT DRAWING TEST");

  SetFont (CSFONT_LARGE);
  WriteCentered (0,-16*5,   blue,    -1, "This is blue text with transparent background");
  WriteCentered (0,-16*4,  green,  blue, "This is green text on blue background");
  WriteCentered (0,-16*3, yellow,  gray, "Yellow text on gray background");
  WriteCentered (0,-16*2,    red, black, "Red text on black background");
  WriteCentered (0,-16*1,  black, white, "Black text on white background");

  SetFont (CSFONT_COURIER);
  int sx = 0, sy = h / 2 + 48, sw = w, sh = h / 2 - 48;
  myG2D->DrawBox (sx, sy, sw, sh, dsteel);
  const char *text = "Crystal Space rulez";
  int tw, th;
  font->GetDimensions (text, tw, th);
  int cc = strlen (text);

  // Test text drawing performance for 1/4 seconds
  int colors [4] = { red, green, blue, yellow };
  sx += 20; sw -= 40 + tw;
  sy += 10; sh -= 20 + th;
  csRandomGen rng (csGetTicks ());
  csTicks start_time = csGetTicks (), delta_time;
  int char_count = 0;
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
  SetFont (CSFONT_LARGE);
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

  SetFont (CSFONT_ITALIC);
  WriteCentered (0,-16*4, white, -1, "TEXT DRAWING TEST");

  SetFont (CSFONT_LARGE);
  WriteCentered (0,-16*2,   gray, black, "This is a benchmark of text drawing");
  WriteCentered (0,-16*1,   gray, black, "with transparent background");

  SetFont (CSFONT_COURIER);
  int sx = 0, sy = h / 2 + 48, sw = w, sh = h / 2 - 48;
  myG2D->DrawBox (sx, sy, sw, sh, dsteel);
  const char *text = "Crystal Space rulez";
  int tw,th;
  font->GetDimensions (text, tw, th);
  int cc = strlen (text);

  // Test text drawing performance for 1/4 seconds
  int colors [4] = { red, green, blue, yellow };
  sx += 20; sw -= 40 + tw;
  sy += 10; sh -= 20 + th;
  csRandomGen rng (csGetTicks ());
  csTicks start_time = csGetTicks (), delta_time;
  int char_count = 0;
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
  SetFont (CSFONT_LARGE);
  WriteCentered (0, 16*1, green, black, " Performance: %20.1f characters/second ", perf);
}

static bool G2DEventHandler (iEvent& ev)
{
  if (ev.Type == csevBroadcast && ev.Command.Code == cscmdProcess)
  {
    Sys->SetupFrame ();
    return true;
  }
  else if (ev.Type == csevBroadcast && ev.Command.Code == cscmdFinalProcess)
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

  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  iCommandLineParser* cmdline = CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser);

  System.myG2D = CS_QUERY_REGISTRY (object_reg, iGraphics2D);
  // Now load the canvas plugin
  if (!System.myG2D)
  {
    const char *canvas = cmdline->GetOption ("canvas");
    if (!canvas || !*canvas)
      canvas = CS_SOFTWARE_2D_DRIVER;
    else if (strncmp ("crystalspace.", canvas, 13))
    {
      char *tmp = (char *)alloca (strlen (canvas) + 25);
      strcpy (tmp, "crystalspace.graphics2d.");
      strcat (tmp, canvas);
      canvas = tmp;
    }
    System.myG2D = CS_LOAD_PLUGIN (plugin_mgr, canvas, iGraphics2D);
    if (!object_reg->Register (System.myG2D, "iGraphics2D"))
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	  "crystalspace.application.g2dtest",
	  "Unable to register canvas!");
      return -1;
    }
  }
  plugin_mgr->DecRef ();
  cmdline->DecRef ();

  if (!System.myG2D)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.g2dtest",
	"Unable to load canvas driver!");
    return -1;
  }

  if (!csInitializer::OpenApplication (object_reg))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.g2dtest",
        "Unable to open drivers!");
    return -1;
  }

  if (!System.myG2D->Open ())
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.g2dtest",
        "Unable to open graphics context!");
    return -1;
  }

  csDefaultRunLoop(object_reg);
  System.myG2D->Close();
  System.myG2D->DecRef ();
  return 0;
}
