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

#ifndef __ITHING_H__
#define __ITHING_H__

#include "csutil/scf.h"

class csVector3;
class csMatrix3;
class csThing;
struct iSector;

SCF_VERSION (iThing, 0, 0, 2);

/**
 * This is the generalized interface to Things.<p>
 * A thing is a 3D model, in most cases non-moveable, which is mostly used
 * for details. That is, iSector objects outlines the basic bounds of the
 * room (e.g. walls) and iThing's are the inside details (tables, columns,
 * paintings and so on).
 */
struct iThing : public iBase
{
  /// Used by the engine to retrieve internal sector object (ugly)
  virtual csThing *GetPrivateObject () = 0;

  /// Get the movable for this thing.
  virtual iMovable* GetMovable () = 0;
  /// Update the thing after doing a move.
  virtual void UpdateMove () = 0;
};

#endif
