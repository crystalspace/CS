/*
    Copyright (C) 2000 by Jorrit Tyberghein

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
#include "csutil/scf.h"
#include "csutil/csstring.h"
#include "video/canvas/ps2d/ps2d.h"
#include "video/canvas/common/scancode.h"
#include "csutil/csrect.h"
#include "isys/isystem.h"

#include "SDL/SDL.h"
#include <fakegl.h>

IMPLEMENT_FACTORY (csGraphics2Dps2)

EXPORT_CLASS_TABLE (ps2d)
  EXPORT_CLASS (csGraphics2Dps2, "crystalspace.graphics2d.ps2d",
    "Playstation 2 2D graphics driver (ps2d) for Crystal Space")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE (csGraphics2Dps2)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics2D)
  IMPLEMENTS_INTERFACE (iEventPlug)
IMPLEMENT_IBASE_END

csGraphics2Dps2::csGraphics2Dps2 (iBase *iParent) :
  csGraphics2D ()
{
  CONSTRUCT_IBASE (iParent);

  EventOutlet = NULL;
}

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 224
#define PI (3.141592f)

#define VIRTUAL_WIDTH 640
#define VIRTUAL_HEIGHT 480

unsigned char framebuffer[VIRTUAL_WIDTH*VIRTUAL_HEIGHT];

unsigned char *csGraphics2Dps2::GetPixelAt (int x, int y) {
  return &framebuffer[x+(y*VIRTUAL_WIDTH)];
}

bool csGraphics2Dps2::Initialize (iSystem *pSystem)
{
  printf("csGraphics2Dps2::Initialize\n");

  if (!csGraphics2D::Initialize (pSystem))
    return false;

  // Tell system driver to call us on every frame
  System->CallOnEvents (this, CSMASK_Nothing);
  // Create the event outlet
  EventOutlet = System->CreateEventOutlet (this);

  return true;
}

csGraphics2Dps2::~csGraphics2Dps2(void)
{
  Close();
  if (EventOutlet)
    EventOutlet->DecRef ();
}

bool pdglSetCrystalSpace();
bool csGraphics2Dps2::Open(const char *Title)
{
  CsPrintf (MSG_INITIALIZATION, "Crystal Space PS2 Driver.\n");

  SDL_Init(255);
  pdglSetCrystalSpace();

  // Open your graphic interface
  if (!csGraphics2D::Open (Title))
    return false;

  return true;
}

void csGraphics2Dps2::Close ()
{
  csGraphics2D::Close ();
}

bool csGraphics2Dps2::BeginDraw ()
{
  csGraphics2D::BeginDraw ();

  return true;
}

void csGraphics2Dps2::FinishDraw ()
{
  csGraphics2D::FinishDraw ();
}

bool csGraphics2Dps2::PerformExtension (const char *iCommand, ...) {
  return true;
}

void csGraphics2Dps2::Print (csRect* /*area*/) {
//	printf("print\n");

    SDL_GL_SwapBuffers();
}

void csGraphics2Dps2::DrawLine (float x1, float y1, float x2, float y2, int color) {
/*
    glBegin(GL_LINE);
    glDisable(GL_TEXTURE_2D);
    glColor4ub(255,255,255,128);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
*/
}

void csGraphics2Dps2::Write (int x, int y, int fg, int bg, const char *text) {
//	printf("write\n");
}

void csGraphics2Dps2::Clear (int color) {
//	printf("clear\n");
/*	for(int i=0; i<VIRTUAL_WIDTH*VIRTUAL_HEIGHT; i++) {
		framebuffer[i]=color;
	}*/
    memset(framebuffer, color, VIRTUAL_WIDTH*VIRTUAL_HEIGHT);
}

void csGraphics2Dps2::SetRGB (int i, int r, int g, int b) {
  csGraphics2D::SetRGB (i, r, g, b);

//  printf("setrgb\n");
}

bool csGraphics2Dps2::SetMousePosition (int x, int y)
{
  return true;
}

bool csGraphics2Dps2::SetMouseCursor (csMouseCursorID iShape)
{
  return true;
}

int translate_key (SDL_Event *ev)
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

bool csGraphics2Dps2::HandleEvent (iEvent &/*Event*/)
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
            EventOutlet->Key (key, -1, down);

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

