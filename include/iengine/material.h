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

#ifndef __IENGINE_MATERIAL_H__
#define __IENGINE_MATERIAL_H__

#include "csutil/scf.h"

class csMaterialWrapper;
struct iMaterial;

SCF_VERSION (iMaterialWrapper, 0, 0, 4);

/**
 * This class represents a material wrapper which holds
 * the mapping between a material in the engine and a material
 * in the 3D rasterizer.
 */
struct iMaterialWrapper : public iBase
{
  /// @@@Ugly.
  virtual csMaterialWrapper* GetPrivateObject () = 0;
  /// Get the material handle.
  virtual iMaterialHandle* GetMaterialHandle () = 0;
  /// Visit this material.
  virtual void Visit () = 0;
  /// Get the original material.
  virtual iMaterial* GetMaterial () = 0;
};

#endif // __IENGINE_MATERIAL_H__
