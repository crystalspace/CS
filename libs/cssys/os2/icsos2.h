/*
  OS/2 support for Crystal Space 3D library
  Copyright (C) 1998 by Jorrit Tyberghein
  Written by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __ICSOS2_H__
#define __ICSOS2_H__

#include "cscom/com.h"

extern const IID IID_IOS2SystemDriver;

interface IOS2SystemDriver : public IUnknown
{
  /// Get user settings
  STDMETHOD (GetSettings) (int &WindowX, int &WindowY,
    int &WindowWidth, int &WindowHeight, bool &HardwareCursor) PURE;
  /// Put a keyboard event into event queue
  STDMETHOD (KeyboardEvent) (int ScanCode, bool Down) PURE;
  /// Put a mouse event into event queue
  STDMETHOD (MouseEvent) (int Button, int Down, int x, int y, int ShiftFlags) PURE;
  /// Put a focus change event into event queue
  STDMETHOD (FocusEvent) (bool Enable) PURE;
  /// Handle application termination event
  STDMETHOD (TerminateEvent) () PURE;
};

#endif // __ICSOS2_H__
