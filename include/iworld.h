/*
    Crystal Space 3D engine
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

#ifndef __IWORLD_H__
#define __IWORLD_H__

#include "csutil/scf.h"
#include "iplugin.h"

class csWorld;

SCF_VERSION (iWorld, 0, 0, 1);

/**
 * This interface is supposed to be exposed to plugins and other
 * SCF modules that need access to the world internals.
 */
struct iWorld : public iPlugIn
{
  /// KLUDGE: this shouldn't be used when iWorld interface will be complete
  virtual csWorld *GetCsWorld () = 0;
  /// Query the format to load textures (usually this depends on texture manager)
  virtual int GetTextureFormat () = 0;
};

#endif // __IWORLD_H__
