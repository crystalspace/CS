/*
    Copyright (C) 1998,1999 by Jorrit Tyberghein
    Written by Xavier Planet.
    Overhauled and re-engineered by Eric Sunshine <sunshine@sunshineco.com>

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
class BMessage;
class ITextureHandle;

extern const IID IID_IBeLibSystemDriver;

interface IBeLibSystemDriver : public IUnknown
{
  STDMETHOD (ProcessUserEvent) (BMessage*) PURE;
  STDMETHOD (SetMouseCursor) (int shape, ITextureHandle*) PURE;
};

#endif // __IBE_H__
