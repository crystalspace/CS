/*
    SDL 2d canvas for Crystal Space (header)
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

#ifndef __CS_SDL2D_H__
#define __CS_SDL2D_H__

#include "csutil/scf.h"
#include "csplugincommon/canvas/graph2d.h"
#include "iutil/event.h"

#include <SDL.h>
#include <SDL_mutex.h>

/// SDL canvas.
class csGraphics2DSDL : public scfImplementationExt1<csGraphics2DSDL,
						       csGraphics2D, 
						       iEventPlug>
{
public:
  SDL_Surface *screen;
  int size_mem;
  int shutdown;
  unsigned char *membuffer;
  SDL_mutex *th_lock;
  csRef<iEventOutlet> EventOutlet;

private:
  virtual int translate_key(SDL_Event *ev);
  /// fixup: to keep library in memory.
  void fixlibrary();

public:
  csGraphics2DSDL (iBase *iParent);
  virtual ~csGraphics2DSDL ();

  virtual bool Initialize (iObjectRegistry *object_reg);
  virtual bool Open ();
  virtual void Close ();

  void Report (int severity, const char* msg, ...);

  /// Necessary to access framebuffer
  virtual bool BeginDraw();
  virtual void FinishDraw();

  virtual void Print (csRect const* area = 0);
  virtual void SetRGB (int i, int r, int g, int b);

  /// Called on every frame by system driver
  virtual bool HandleEvent (iEvent &Event);

  virtual bool SetMousePosition (int x, int y);
  virtual bool SetMouseCursor (csMouseCursorID iShape);

  virtual unsigned GetPotentiallyConflictingEvents ()
    { return CSEVTYPE_Keyboard | CSEVTYPE_Mouse; }

  virtual unsigned QueryEventPriority (unsigned /*iType*/)
    { return 150; }
};

#endif // __CS_SDL2D_H__
