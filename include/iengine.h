/*
    Crystal Space 3D engine
    Copyright (C) 2000 by Jorrit Tyberghein
  
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

#ifndef __IENGINE_H__
#define __IENGINE_H__

#include "csutil/scf.h"
#include "iplugin.h"

struct iWorld;

SCF_VERSION (iEngine, 0, 0, 1);

/**
 * This interface represents the engine as a loadable plugin.
 */
struct iEngine : public iPlugIn
{
  /// Initialize.
  virtual bool Initialize (iSystem *iSys) = 0;
  /// Get pointer to the world.
  virtual iWorld *GetWorld () = 0;
};

#endif // __IENGINE_H__
