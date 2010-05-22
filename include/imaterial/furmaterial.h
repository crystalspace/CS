/*
  Copyright (C) 2010 Alexandru - Teodor Voicu

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __FUR_INTERF_H__
#define __FUR_INTERF_H__

#include <csutil/scf.h>

#include <ivideo/material.h>
#include <ivaria/view.h>
#include <iengine/sector.h>

#include "crystalspace.h"

struct iFurMaterial;

class csVector3;
class csColor4;

struct iFurMaterialType : public virtual iBase
{
  SCF_INTERFACE (iFurMaterialType, 1, 0, 0);

  virtual void ClearFurMaterials () = 0;
  virtual void RemoveFurMaterial (const char *name,iFurMaterial* furMaterial) = 0;
  virtual iFurMaterial* CreateFurMaterial (const char *name) = 0;
  virtual iFurMaterial* FindFurMaterial (const char *name) const = 0;
};

/**
 * This is the API for our plugin. It is recommended
 * that you use better comments than this one in a
 * real situation.
 */
struct iFurMaterial : public virtual iMaterial 
{
  SCF_INTERFACE (iFurMaterial, 1, 0, 0);
  /// Do something.
  virtual void DoSomething (int param, const csVector3&) = 0;
  /// Get something.
  virtual int GetSomething () const = 0;

  /// Generate geometry
  virtual void GenerateGeometry (iView* view, iSector *room, int controlPoints, 
	int numberOfStrains, float length) = 0;
  virtual void GenerateGeometry (iView* view, iSector *room, 
	csRefArray<iBulletSoftBody> hairStrands) = 0;
};
#endif // __FUR_INTERF_H__
