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

#ifndef __IUNIX_H__
#define __IUNIX_H__

#include "cscom/com.h"

extern const IID IID_IUnixSystemDriver;

typedef void (*LoopCallback) (void *param);

interface IUnixSystemDriver : public IUnknown
{
  /// Get user settings
  STDMETHOD (GetSettings) (int &SimDepth, bool &UseSHM, bool &HardwareCursor) PURE;
  /// Set a callback that gets called from inside the main event loop
  STDMETHOD (SetLoopCallback) (LoopCallback Callback, void *Param) PURE;
  /// Put a keyboard event into event queue
  STDMETHOD (KeyboardEvent) (int Key, bool Down) PURE;
  /// Put a mouse event into event queue
  STDMETHOD (MouseEvent) (int Button, int Down, int x, int y, int ShiftFlags) PURE;
  /// Put a focus change event into event queue
  STDMETHOD (FocusEvent) (int Enable) PURE;
};

#endif // __IUNIX_H__
