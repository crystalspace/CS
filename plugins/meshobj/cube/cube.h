/*
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

#ifndef _CUBE_H_
#define _CUBE_H_

#include "imeshobj.h"

/**
 * Cube version of mesh object.
 */
class csCubeMeshObject : public iMeshObject
{
private:

public:
  /// Constructor.
  csCubeMeshObject ();

  /// Destructor.
  virtual ~csCubeMeshObject ();

public:
  ///------------------------ iMeshObject implementation ------------------------
  DECLARE_IBASE;

  /// Draw.
  virtual bool Draw (iRenderView* rview);
};

/**
 * Factory for cubes.
 */
class csCubeMeshObjectFactory : public iMeshObjectFactory
{
private:

public:
  /// Constructor.
  csCubeMeshObjectFactory (iBase*);

  /// Destructor.
  virtual ~csCubeMeshObjectFactory ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  ///------------------------ iMeshObjectFactory implementation ------------------------
  DECLARE_IBASE;

  /// Draw.
  virtual iMeshObject* NewInstance ();
};

#endif // _CUBE_H_

