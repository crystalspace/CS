/*
    SDL 2d canvas for Crystal Space (source)
    Copyright (C) 2000 by George Yohng <yohng@drivex.dosware.8m.com>

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
#include "sdl2d.h"
#include "csgeom/csrect.h"
#include "csutil/csinput.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"

#include <SDL.h>
#include <SDL_mutex.h>
#include <SDL_thread.h>

CS_IMPLEMENT_PLUGIN

// If you want to enable "drawing thread" functionality. This
// includes NICE ALPHA mouse cursors for ANY SDL mode.

// HOWEVER, IT BRINGS A SLOWDOWN. DO NOT APPLY THIS, UNLESS YOUR MACHINE
// IS FAST ENOUGH. BUT IF YOUR MACHINE IS FAST, THIS OPTION IS RECOMMENDED.
// YOU CAN APPLY IT SAFELY - THE SLOWDOWN DOES NOT DEPEND ON SCENE COMPLEXITY.

// The slowdown is caused by copying buffer screen THREE TIMES. However,
// if you want to apply some nice 2d effects later to this driver, this
// is the necessary thing.

#define SHADE_BUF 0

//fixup:
//  For dlopen::dladdr
#ifdef CS_USE_GNU_DLADDR
#    ifndef __USE_GNU
#        define __USE_GNU 1
#    endif
#    include <dlfcn.h>
#endif

SCF_IMPLEMENT_FACTORY (csGraphics2DSDL)


SCF_IMPLEMENT_IBASE_EXT (csGraphics2DSDL)
  SCF_IMPLEMENTS_INTERFACE (iEventPlug)
SCF_IMPLEMENT_IBASE_EXT_END

#if SHADE_BUF
#include "util/img.inc"
static SDL_Surface
*Sarrow_tga=0,
*Scross_tga=0,
*Shglass1_tga=0,
*Shglass2_tga=0,
*Slrarrow_tga=0,
*Smagnify_tga=0,
*Smove_tga=0,
*Spen_tga=0,
*Ss1arrow_tga=0,
*Ss2arrow_tga=0,
*Sstop_tga=0,
*Sudarrow_tga=0,
*Scurrent=0;
static int cursorNo;

static int hotspot[12][2]=
{{0,0},    {9,9},    {16,16},  {0,0},  {14,14},  {13,14},  {14,14},
 {15,14},  {14,15},  {10,10},  {10,11}} ;

static int Sinitialized=0;
#define INIT_SF(name)\
  S##name = SDL_CreateRGBSurface (\
      SDL_SRCALPHA, 32,32,32,0xFF0000, 0xFF00, 0xFF, 0xFF000000\
  );\
  SDL_LockSurface(S##name);\
  memcpy(S##name->pixels,name,4096);\
  SDL_UnlockSurface(S##name);

#define DEINIT_SF(name)\
  SDL_FreeSurface (\
      S##name\
  );

static void init_surfaces()
{
  if (Sinitialized)
    { Sinitialized++;return; }

  INIT_SF(arrow_tga)
  INIT_SF(cross_tga)
  INIT_SF(hglass1_tga)
  INIT_SF(hglass2_tga)
  INIT_SF(lrarrow_tga)
  INIT_SF(magnify_tga)
  INIT_SF(move_tga)
  INIT_SF(pen_tga)
  INIT_SF(s1arrow_tga)
  INIT_SF(s2arrow_tga)
  INIT_SF(stop_tga)
  INIT_SF(udarrow_tga)

  Sinitialized=1;
}

static void deinit_surfaces()
{
  if (--Sinitialized)
    return;

  DEINIT_SF(arrow_tga)
  DEINIT_SF(cross_tga)
  DEINIT_SF(hglass1_tga)
  DEINIT_SF(hglass2_tga)
  DEINIT_SF(lrarrow_tga)
  DEINIT_SF(magnify_tga)
  DEINIT_SF(move_tga)
  DEINIT_SF(pen_tga)
  DEINIT_SF(s1arrow_tga)
  DEINIT_SF(s2arrow_tga)
  DEINIT_SF(stop_tga)
  DEINIT_SF(udarrow_tga)
}

static int drawing_thread(void *_owner)
{
  csGraphics2DSDL *owner = (csGraphics2DSDL *)_owner;

  owner->shutdown=0;

  while(owner->is_open)
  {
    SDL_LockMutex(owner->th_lock);
    SDL_LockSurface(owner->screen);
    memcpy(owner->screen->pixels,owner->membuffer,owner->size_mem);
    SDL_UnlockSurface(owner->screen);
    SDL_UnlockMutex(owner->th_lock);
    if (Scurrent)
    {
      SDL_Rect dst;
      int x,y;

      SDL_GetMouseState(&x, &y);

      dst.x = (Sint16)x-hotspot[cursorNo][0];
      dst.y = (Sint16)y-hotspot[cursorNo][1];
      dst.w = dst.h = 32;

      SDL_BlitSurface(Scurrent, 0, owner->screen, &dst);

      if (Scurrent == Shglass1_tga)
        Scurrent = Shglass2_tga; else
      if (Scurrent == Shglass2_tga)
        Scurrent = Shglass1_tga;
    }
    SDL_Flip (owner->screen);
  }

  owner->shutdown=1;
  return 0;
}
#endif

// csGraphics2DSDL functions
csGraphics2DSDL::csGraphics2DSDL(iBase *iParent) : csGraphics2D (iParent)
{
  EventOutlet = 0;
}

void csGraphics2DSDL::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (rep)
    rep->ReportV (severity, "crystalspace.canvas.sdl", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

//fixup:
//  This function increases reference counter for sdl2d.so. This is
//  necessary to keep all dependent libraries in memory. For example,
//  sdl2d.so may load pthread.so. Thus, pthread sets "hook-on-exit",
//  but the code of pthread.so becomes unavailable on program exit
//  (after sdl2d.so is unloaded), and program ends with "Segmentation
//  Fault". Increasing reference counter (via dlopen) fixes this problem
//  - the library sdl2d.so with all dependent libraries stays open until
//  the main program exits. Of course, this is not the best solution,
//  that's why the function is marked "fixup".

#if defined(CS_HAVE_RTLD_NOW)
#define CS_SDL2D_DLOPEN_MODE RTLD_NOW
#else
#define CS_SDL2D_DLOPEN_MODE RTLD_LAZY
#endif

void csGraphics2DSDL::fixlibrary()
{
#if defined(CS_USE_GNU_DLADDR)
    Dl_info dlip;

    dladdr((const void*)sdl2d_scfInitialize,&dlip);
    dlopen(dlip.dli_fname,CS_SDL2D_DLOPEN_MODE);

    Report (CS_REPORTER_SEVERITY_NOTIFY, "Library %s locked.",dlip.dli_fname);
#elif defined(CS_PLATFORM_WIN32)
	Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "SDL generic Win32 support by Crystal Space Development Team.");
#else
    Report (CS_REPORTER_SEVERITY_NOTIFY,
              "WARNING: Your operating system is not tested\n"
              "         yet with sdl2d video driver!");
#endif
}

bool csGraphics2DSDL::Initialize (iObjectRegistry *object_reg)
{
    if (!csGraphics2D::Initialize (object_reg))
      return false;

    Memory = 0;

    // SDL Starts here

    Report (CS_REPORTER_SEVERITY_NOTIFY, "Crystal Space SDL version.");

    //fixup:
    //make library persistent
    fixlibrary();

    Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Defaults to %dx%dx%d resolution.", Width, Height, Depth);

    Memory = 0;

#if SHADE_BUF
    cursorNo = csmcNone;
#endif

    switch (Depth)
    {
      case 8:
        pfmt.RedMask = pfmt.GreenMask = pfmt.BlueMask = pfmt.AlphaMask = 0;
        pfmt.PalEntries = 256;
        pfmt.PixelBytes = 1;
        break;
      case 15:
        pfmt.RedMask   = 0x1f << 10;
        pfmt.GreenMask = 0x1f << 5;
        pfmt.BlueMask  = 0x1f;
        pfmt.AlphaMask = 0;
        pfmt.PalEntries = 0;
        pfmt.PixelBytes = 2;
        break;
      case 16:
        pfmt.RedMask   = 0x1f << 11;
        pfmt.GreenMask = 0x3f << 5;
        pfmt.BlueMask  = 0x1f;
        pfmt.AlphaMask = 0;
        pfmt.PalEntries = 0;
        pfmt.PixelBytes = 2;
        break;
      case 32:
        pfmt.RedMask = 0xff << 16;
        pfmt.GreenMask = 0xff << 8;
        pfmt.BlueMask = 0xff;
        pfmt.AlphaMask = 0xff << 24;
        pfmt.PalEntries = 0;
        pfmt.PixelBytes = 4;
        break;
      default:
        Report (CS_REPORTER_SEVERITY_ERROR, "Pixel depth %d not supported",
	  Depth);
    }

    return true;
}

csGraphics2DSDL::~csGraphics2DSDL(void)
{
    // Destroy your graphic interface
    Memory = 0;
    Close();
}

bool csGraphics2DSDL::Open()
{
  if (is_open) return true;

  // Open your graphic interface
  if (!csGraphics2D::Open ()) return false;

  if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE) < 0)
  {
    Report (CS_REPORTER_SEVERITY_ERROR,
      "Couldn't initialize SDL: %s", SDL_GetError());
    return false;
  }

  screen = SDL_SetVideoMode(Width,Height,Depth,SDL_SWSURFACE);
  if (screen == 0) {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't set %dx%dx%d video mode: %s",
                               Width, Height, Depth, SDL_GetError());
    return false;
  }

  SDL_WM_SetCaption(win_title, 0);
  SDL_EnableKeyRepeat(250, 30);
  SDL_ShowCursor(1);

#if SHADE_BUF
  cursorNo = csmcNone;
  *((char **)&Memory) =
    new char[(size_mem=Width*Height*screen->format->BytesPerPixel)+128];
  *((char **)&membuffer) =
    new char[(size_mem=Width*Height*screen->format->BytesPerPixel)+128];
  init_surfaces();
  Scurrent = 0;
  th_lock = SDL_CreateMutex();
  SDL_CreateThread(drawing_thread, (void *)this);
#else
  *((void **)&Memory) = screen->pixels;
#endif

  switch (Depth)
  {
    case 8:
      pfmt.RedMask = pfmt.GreenMask = pfmt.BlueMask = pfmt.AlphaMask = 0;
      pfmt.PalEntries = 256; pfmt.PixelBytes = 1;
      break;

    case 15:
    case 16:
      pfmt.RedMask   = screen->format->Rmask;
      pfmt.GreenMask = screen->format->Gmask;
      pfmt.BlueMask  = screen->format->Bmask;
      pfmt.AlphaMask = screen->format->Amask;
      pfmt.PalEntries = 0;
      pfmt.PixelBytes = screen->format->BytesPerPixel;

      _DrawPixel = DrawPixel16;
      _GetPixelAt = GetPixelAt16;
      break;

    case 24:
    case 32:
      pfmt.RedMask   = screen->format->Rmask;
      pfmt.GreenMask = screen->format->Gmask;
      pfmt.BlueMask  = screen->format->Bmask;
      pfmt.AlphaMask = screen->format->Amask;
      pfmt.PalEntries = 0;
      pfmt.PixelBytes = screen->format->BytesPerPixel;

      _DrawPixel = DrawPixel32;
      _GetPixelAt = GetPixelAt32;
      break;
    default:
      Report(CS_REPORTER_SEVERITY_ERROR,"Pixel depth %d not supported",Depth);
  }

  pfmt.complete ();
  Clear(0);

  csRef<iEventQueue> q = CS_QUERY_REGISTRY(object_reg, iEventQueue);
  if (q != 0)
  {
    q->RegisterListener (scfiEventHandler, CSMASK_Nothing);
    if (!EventOutlet.IsValid())
      EventOutlet = q->CreateEventOutlet (this);
  }
  return true;
}

void csGraphics2DSDL::Close(void)
{
  if (!is_open) return;
  // Close your graphic interface

#if SHADE_BUF
  while(!shutdown)
    SDL_Delay(1);
#endif

  SDL_Quit();
  csGraphics2D::Close ();

#if SHADE_BUF
  delete[] ((char *)Memory);
  delete[] ((char *)membuffer);
  SDL_DestroyMutex(th_lock);
#endif
  Memory = 0;
}

struct keyconv_t
{
  SDLKey key_sdl;
  int key_cs;
};

int csGraphics2DSDL::translate_key (SDL_Event *ev)
{
  switch(ev->key.keysym.sym)
  {
#define I case //////////////////////////////////////
#define l : return //////////////////////////////////
#define J ; /////////////////////////////////////////
      I SDLK_TAB         l CSKEY_TAB               J
      I SDLK_ESCAPE      l CSKEY_ESC               J
      I SDLK_RETURN      l CSKEY_ENTER             J
      I SDLK_KP_ENTER    l CSKEY_ENTER             J
      I SDLK_CLEAR       l CSKEY_BACKSPACE         J
      I SDLK_BACKSPACE   l CSKEY_BACKSPACE         J

      I SDLK_UP          l CSKEY_UP                J
      I SDLK_KP8         l CSKEY_UP                J
      I SDLK_DOWN        l CSKEY_DOWN              J
      I SDLK_KP2         l CSKEY_DOWN              J
      I SDLK_LEFT        l CSKEY_LEFT              J
      I SDLK_KP4         l CSKEY_LEFT              J
      I SDLK_RIGHT       l CSKEY_RIGHT             J
      I SDLK_KP6         l CSKEY_RIGHT             J

      I SDLK_PAGEUP      l CSKEY_PGUP              J
      I SDLK_PAGEDOWN    l CSKEY_PGDN              J
      I SDLK_INSERT      l CSKEY_INS               J
      I SDLK_DELETE      l CSKEY_DEL               J
      I SDLK_HOME        l CSKEY_HOME              J
      I SDLK_END         l CSKEY_END               J

      I SDLK_LSHIFT      l CSKEY_SHIFT             J
      I SDLK_RSHIFT      l CSKEY_SHIFT             J

      I SDLK_LCTRL       l CSKEY_CTRL              J
      I SDLK_RCTRL       l CSKEY_CTRL              J

      I SDLK_LALT        l CSKEY_ALT               J
      I SDLK_RALT        l CSKEY_ALT               J
      I SDLK_LMETA       l CSKEY_ALT               J
      I SDLK_RMETA       l CSKEY_ALT               J

      I SDLK_KP_PLUS     l CSKEY_PADPLUS           J
      I SDLK_KP_MINUS    l CSKEY_PADMINUS          J
      I SDLK_KP_MULTIPLY l CSKEY_PADMULT           J
      I SDLK_KP_DIVIDE   l CSKEY_PADDIV            J
      I SDLK_KP5         l CSKEY_CENTER            J

      I SDLK_F1          l CSKEY_F1                J
      I SDLK_F2          l CSKEY_F2                J
      I SDLK_F3          l CSKEY_F3                J
      I SDLK_F4          l CSKEY_F4                J
      I SDLK_F5          l CSKEY_F5                J
      I SDLK_F6          l CSKEY_F6                J
      I SDLK_F7          l CSKEY_F7                J
      I SDLK_F8          l CSKEY_F8                J
      I SDLK_F9          l CSKEY_F9                J
      I SDLK_F10         l CSKEY_F10               J
      I SDLK_F11         l CSKEY_F11               J
      I SDLK_F12         l CSKEY_F12               J

      default            l                       //J
          (((int)(ev->key.keysym.sym))<256)?     //J
            (int)(ev->key.keysym.sym):           //J
            -1                                     J
#undef I ////////////////////////////////////////////
#undef l ////////////////////////////////////////////
#undef J ////////////////////////////////////////////
  }
}

bool csGraphics2DSDL::HandleEvent (iEvent&)
{
  SDL_Event ev;
  while ( SDL_PollEvent(&ev) )
  {
    switch (ev.type)
    {
      case SDL_KEYDOWN:
      case SDL_KEYUP:
      {
          int key  = translate_key(&ev);
          int down = (ev.type == SDL_KEYDOWN);

          if (key >= 0)
            EventOutlet->Key (key, 0, down);

          break;
      }
      case SDL_MOUSEMOTION:
      {
          EventOutlet->Mouse (0, false, ev.motion.x, ev.motion.y);
          break;
      }
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
      {
          int btn = (ev.button.button==1)?1:
                    (ev.button.button==2)?3:
                    (ev.button.button==3)?2:
                                          0;

          if (btn)
            EventOutlet->Mouse ( btn, (bool)(ev.type==SDL_MOUSEBUTTONDOWN),
                                 ev.button.x, ev.button.y);
      }
    }
  }
  return false;
}

void csGraphics2DSDL::Print (csRect const* area)
{
  (void) area;

#if SHADE_BUF
  SDL_LockMutex(th_lock);
  memcpy(membuffer,Memory,size_mem);
  SDL_UnlockMutex(th_lock);
#else
  if ((!area)||
       ((area->xmin==0)&&(area->xmax==Width)&&
        (area->ymin==0)&&(area->ymax==Height)))
    SDL_Flip(screen);
  else
    SDL_UpdateRect(
      screen, area->xmin, area->ymin, area->Width (), area->Height ());
#endif
}

void csGraphics2DSDL::SetRGB(int i, int r, int g, int b)
{
  SDL_Color c;

  memset(&c,0,sizeof(c));

  c.r = r;
  c.g = g;
  c.b = b;

  SDL_SetColors(screen,&c,i,1);
}

bool csGraphics2DSDL::BeginDraw()
{
  if (!Memory)
    return false;
  if (!csGraphics2D::BeginDraw ())
    return false;
#if !SHADE_BUF
  SDL_LockSurface (screen);
#endif
  return true;
}
void csGraphics2DSDL::FinishDraw()
{
#if !SHADE_BUF
  SDL_UnlockSurface (screen);
#endif
  csGraphics2D::FinishDraw ();
}

bool csGraphics2DSDL::SetMousePosition (int x, int y)
{
  SDL_WarpMouse ((Uint16)x,(Uint16)y);
  return true;
}

bool csGraphics2DSDL::SetMouseCursor (csMouseCursorID iShape)
{
  if (iShape == csmcNone)
  {
#if SHADE_BUF
    cursorNo = csmcNone;
#endif
    SDL_ShowCursor (0);
    return true;
  }
  else
  {
#if SHADE_BUF
#define I (iShape == (cursorNo =
#define l ))?
#define J :
    Scurrent =
    I csmcArrow     l Sarrow_tga    J
    I csmcLens      l Smagnify_tga  J
    I csmcCross     l Scross_tga    J
    I csmcPen       l Spen_tga      J
    I csmcMove      l Smove_tga     J
    I csmcSizeNWSE  l Ss1arrow_tga  J
    I csmcSizeNESW  l Ss2arrow_tga  J
    I csmcSizeNS    l Sudarrow_tga  J
    I csmcSizeEW    l Slrarrow_tga  J
    I csmcStop      l Sstop_tga     J
    I csmcWait      l Shglass1_tga  J
    I 12            l 0          J
                      0;

    if (cursorNo >= 12) cursorNo = 0;
    if (cursorNo < 0) cursorNo = 0;
#undef I
#undef l
#undef J
      return !! Scurrent;
#else
    //fixup:
    SDL_ShowCursor (0);
    return false;
#endif
  }
}
