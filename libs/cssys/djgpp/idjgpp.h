/*
    DOS support for Crystal Space 3D library
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by David N. Arnold <derek_arnold@fuse.net>
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

#ifndef __IDJGPP_H__
#define __IDJGPP_H__

#include "cscom/com.h"

extern const IID IID_IDosSystemDriver;

interface IDosSystemDriver : public IUnknown
{
  /// Enable or disable text-mode CsPrintf
  STDMETHOD (EnablePrintf) (bool Enable) PURE;
  /// Set mouse position since mouse driver is part of system driver
  STDMETHOD (SetMousePosition) (int x, int y) PURE;
};

#endif // __IDJGPP_H__
