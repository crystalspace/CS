/*
    DOS Allegro support for Crystal Space 3D library
    Copyright (C) 1999 by Dan Bogdanov <dan@pshg.edu.ee>
    Modified for full Allegro by Burton Radons <loth@pacificcoast.net>

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

#ifndef __ALLEGRO_H__
#define __ALLEGRO_H__

#include <allegro.h>
#include "csutil/scf.h"
#include "video/canvas/common/graph2d.h"
#include "iutil/event.h"

/**
 * Allegro 2D graphics driver.
 */
class csGraphics2DAlleg : public csGraphics2D, public iEventPlug
{
  /// Palette has been changed?
  bool PaletteChanged;
  int keydown [128];
  int x_mouse;
  int y_mouse; /* Previously recorded mouse position */
  int button;
  BITMAP *bitmap;
  bool scale; /* Bitmap is not screen size, scale it */
  // The event outlet
  iEventOutlet *EventOutlet;
  // Hook keyboard and mouse?
  bool hook_kbd, hook_mouse;
  // Are the keyboard/mouse hooks active?
  bool kbd_hook_active, mouse_hook_active;

public:
  SCF_DECLARE_IBASE_EXT(csGraphics2D);

  csGraphics2DAlleg (iBase *iParent);
  virtual ~csGraphics2DAlleg ();

  void Report (int severity, const char* msg, ...);

  virtual bool Initialize (iObjectRegistry *object_reg);
  virtual bool Open();
  virtual void Close(void);

  virtual void Print (csRect *area = NULL);

  virtual void SetRGB(int i, int r, int g, int b);
  virtual bool HandleEvent (iEvent &/*Event*/);

  virtual bool BeginDraw () { return (Memory != NULL); }
  virtual bool DoubleBuffer (bool /*Enable*/) { return true; }
  virtual bool GetDoubleBufferState () { return true; }

  virtual bool PerformExtensionV (char const* command, va_list);
  /**
   * Blit between a bitmap and one twice it's size.
   * sw,sh are in source dimensions.
   */
  virtual void DoubleBlit (BITMAP *src, BITMAP *dst, int sw, int sh);

  //------------------------ iEventPlug interface ---------------------------//

  virtual unsigned GetPotentiallyConflictingEvents ()
  { return CSEVTYPE_Keyboard | CSEVTYPE_Mouse; }
  virtual unsigned QueryEventPriority (unsigned /*iType*/)
  { return 50; }
  virtual void EnableEvents (unsigned iType, bool iEnable);

private:
  void FillEvents ();
  void printf_Enable (bool Enable);
};

#endif // __ALLEGRO_H__
