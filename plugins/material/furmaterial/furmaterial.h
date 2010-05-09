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

#ifndef __FUR_MATERIAL_H__
#define __FUR_MATERIAL_H__

#include <iutil/comp.h>
#include <csgeom/vector3.h>
#include <imaterial/furmaterial.h>

#include "csutil/scf_implementation.h"

struct iObjectRegistry;

class FurMaterialType : public scfImplementation2<FurMaterialType,iFurMaterialType,iComponent>
{
private:
  iObjectRegistry* object_reg;
  csHash<csRef<iFurMaterial>, csString> furMaterialHash;
public:
  FurMaterialType (iBase* parent);
  virtual ~FurMaterialType ();

  // From iComponent	
  virtual bool Initialize (iObjectRegistry*);
  
  // From iFurMaterialType
  virtual void ClearFurMaterials ();
  virtual void RemoveFurMaterial (const char *name, iFurMaterial* furMaterial);
  virtual iFurMaterial* CreateFurMaterial (const char *name);
  virtual iFurMaterial* FindFurMaterial (const char *name) const;
};

/**
* This is the implementation for our API and
* also the implementation of the plugin.
*/
class FurMaterial : public scfImplementation2<FurMaterial,iFurMaterial,iComponent>
{
private:
  iObjectRegistry* object_reg;
  csVector3 store_v;

public:
  FurMaterial (iBase* parent);
  virtual ~FurMaterial ();

  // From iComponent.
  virtual bool Initialize (iObjectRegistry*);

  // From iFurMaterial.
  virtual void DoSomething (int param, const csVector3&);
  virtual int GetSomething () const;

  virtual void SetLength (float len);
  virtual float GetLength () const;

  virtual void SetColor (const csColor4& color);
  virtual csColor4 GetColor () const;
};

#endif // __FUR_MATERIAL_H__
