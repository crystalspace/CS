/*
    Copyright (C) 2003 by Jorrit Tyberghein

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

#ifndef __CS_MESHLOD_H__
#define __CS_MESHLOD_H__

#include "csutil/scf.h"
#include "csutil/array.h"
#include "iengine/lod.h"
#include "iengine/mesh.h"

struct iMeshWrapper;

/**
 * This class is used to represent the static lod levels of a
 * hierarchical mesh.
 */
class csStaticLODMesh : public iLODControl
{
private:
  /**
   * There are 10 static lod levels. Every lod level can contain
   * several meshes.
   */
  csArray<iMeshWrapper*> meshes_for_lod[CS_STATIC_LOD_LEVELS];

public:
  SCF_DECLARE_IBASE;

  /// constructor
  csStaticLODMesh ();
  virtual ~csStaticLODMesh ();

  virtual void SetLOD (float m, float a);
  virtual void GetLOD (float& m, float& a) const;
  virtual int GetLODPolygonCount (float lod) const;

  csArray<iMeshWrapper*>& GetMeshesForLOD (int lod)
  {
    return meshes_for_lod[lod];
  }
};

#endif // __CS_MESHLOD_H__

