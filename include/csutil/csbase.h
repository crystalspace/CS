/*
  Crystal Space Windowing System: base class interface
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

#ifndef __CSBASE_H__
#define __CSBASE_H__

#include "cscom/com.h"
#include "css/cssdefs.h"

/**
 * Base class, all other CrystalSpace classes should be derived from this.
 */
extern const GUID IID_IBase;

/// Interface for csBase
interface IBase : public IUnknown
{
};

/// Class csBase
class csBase : public IBase
{
public:
  ///
  virtual ~csBase () {}

  DEFAULT_COM (Base);
};

#endif // __CSBASE_H__
