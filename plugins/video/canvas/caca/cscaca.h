/*
    Colour ASCII art rendering support for Crystal Space 3D library
    Copyright (C) 2005 by dr.W.C.A. Wijngaards
    Based on aalib canvas written by Andrew Zabolotny.

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

#ifndef __CS_CSCACA_H__
#define __CS_CSCACA_H__

#include "csutil/scf.h"
#include "csplugincommon/canvas/graph2d.h"
#include <caca.h>
#include "iutil/event.h"

/**
 * The Colour Ascii Art driver. This is a cross-platform graphics driver which
 * implements drawing using characters and (depending on platform)
 * different intensities.
 */
class csGraphics2DCaca : public csGraphics2D, public iEventPlug
{
  /// Use native mouse cursor, if possible?
  bool HardwareCursor;
  /// The event outlet
  csRef<iEventOutlet> EventOutlet;
  /// the caca bitmap context with pixel format etc.
  struct caca_bitmap* caca_context;

  /// return cooked code for raw keyboard event
  int MapKey(int raw);

public:
  SCF_DECLARE_IBASE_EXT(csGraphics2D);

  csGraphics2DCaca (iBase *iParent);
  virtual ~csGraphics2DCaca ();

  virtual bool Initialize (iObjectRegistry *object_reg);
  virtual bool Open ();
  virtual void Close ();

  virtual void Print (csRect const* area = 0);
  virtual bool BeginDraw ();
  virtual void FinishDraw ();

  virtual bool DoubleBuffer(bool) {return false;}
  virtual void SetTitle (const char* title);

  //------------------------ iEventPlug interface ---------------------------//
  virtual unsigned GetPotentiallyConflictingEvents ()
  { return CSEVTYPE_Keyboard | CSEVTYPE_Mouse; }
  virtual unsigned QueryEventPriority (unsigned /*iType*/)
  { return 1; }
};

#endif // __CS_CSCACA_H__
