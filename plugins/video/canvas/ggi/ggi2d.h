/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#ifndef __CS_GGI2D_H__
#define __CS_GGI2D_H__

#include "csutil/scf.h"
#include "cs2d/common/graph2d.h"
#include "cssys/unix/iunix.h"

#include <ggi/ggi.h>

/// GGI version.
class csGraphics2DGGI : public csGraphics2D
{
  /// Pointer to OS-specific interface
  iUnixSystemDriver* UnixSystem;

  /// This routine is called once per event loop
  static void ProcessEvents (void *Param);

//--  /// Check for a key.
//--  void checkkey (int scancode, bool shift, bool alt, bool ctrl);
//--  /// Handle keys.
//--  void handle_keys (bool shift, bool alt, bool ctrl);
//--  ///
//--  int translate_scancode (int scancode);
//--  bool remember_key[256];
//--  bool button1_pressed, button2_pressed, button3_pressed;

  ///
  virtual bool PerformExtension (char* args);

private:
  ggi_visual_t vis;
  ggi_mode vis_mode;
  int display_width, display_height;

  virtual int translate_key(ggi_event *ev);

public:
  DECLARE_IBASE;

  csGraphics2DGGI (iBase *iParent);
  virtual ~csGraphics2DGGI ();

  virtual bool Initialize (iSystem *pSystem);
  virtual bool Open (const char *Title);
  virtual void Close ();

  virtual bool BeginDraw () { return (Memory != NULL); }

  virtual void Print (csRect *area = NULL);
  virtual void SetRGB (int i, int r, int g, int b);
};

#endif // __CS_GGI2D_H__
