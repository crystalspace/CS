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

#ifndef __IBEOS_H__
#define __IBEOS_H__

#include "cscom/com.h"


extern const IID IID_IBeLibSystemDriver;

typedef void (*BeKeyboardHandler) (void *param, int Key, bool Down);
typedef void (*BeMouseHandler) (void *param, int Button, int Down,
  int x, int y, int ShiftFlags);
typedef void (*BeFocusHandler) (void *param, int Enable);
typedef void (*LoopCallback) (void *param);

interface IBeLibSystemDriver : public IUnknown
{
  /// Get user settings
  STDMETHOD (GetSettings) (int &SimDepth, bool &UseSHM, bool &HardwareCursor) PURE;
  /// Get Unix-specific keyboard event handler routine
  STDMETHOD (GetKeyboardHandler) (BeKeyboardHandler &Handler, void *&Parm) PURE;
  /// Get Unix-specific mouse event handler routine
  STDMETHOD (GetMouseHandler) (BeMouseHandler &Handler, void *&Parm) PURE;
  /// Get Unix-specific focus change event handler routine
  STDMETHOD (GetFocusHandler) (BeFocusHandler &Handler, void *&Parm) PURE;
  /// Set a callback that gets called from inside the main event loop
  STDMETHOD (SetLoopCallback) (LoopCallback Callback, void *Param) PURE;
};

#endif // __IBE_H__
