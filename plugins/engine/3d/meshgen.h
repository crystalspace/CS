/*
    Copyright (C) 2005 by Jorrit Tyberghein

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

#ifndef __CS_MESHGEN_H__
#define __CS_MESHGEN_H__

#include "csutil/csobject.h"
#include "csutil/array.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"
#include "csgeom/box.h"
#include "iengine/mesh.h"
#include "iengine/meshgen.h"

/**
 * Geometry implementation for the mesh generator.
 */
class csMeshGeneratorGeometry : public scfImplementation1<
	csMeshGeneratorGeometry, iMeshGeneratorGeometry>
{
private:
  csRefArray<iMeshFactoryWrapper> factories;
  csArray<float> maxdistances;
  float radius;
  float density;

public:
  csMeshGeneratorGeometry ();
  virtual ~csMeshGeneratorGeometry () { }

  virtual void AddFactory (iMeshFactoryWrapper* factory, float maxdist);
  virtual size_t GetFactoryCount () const { return factories.Length (); }
  virtual void RemoveFactory (size_t idx);
  virtual iMeshFactoryWrapper* GetFactory (size_t idx)
  {
    return factories[idx];
  }
  virtual float GetMaximumDistance (size_t idx)
  {
    return maxdistances[idx];
  }
  virtual void SetRadius (float radius);
  virtual float GetRadius () const { return radius; }
  virtual void SetDensity (float density);
  virtual float GetDensity () const { return density; }
};

/**
 * Mapping implementation for the mesh generator.
 */
class csMeshGeneratorMapping : public scfImplementationExt1<
	csMeshGeneratorMapping, csObject, iMeshGeneratorMapping>
{
private:
  csRef<iMeshWrapper> mesh;
  csBox3 samplebox;

public:
  csMeshGeneratorMapping ();
  virtual ~csMeshGeneratorMapping () { }

  virtual void SetMeshWrapper (iMeshWrapper* mesh);
  virtual iMeshWrapper* GetMeshWrapper () { return mesh; }
  virtual void SetSampleBox (const csBox3& box) { samplebox = box; }
  virtual const csBox3& GetSampleBox () const { return samplebox; }
};

/**
 * The mesh generator.
 */
class csMeshGenerator : public scfImplementation1<
	csMeshGenerator, iMeshGenerator>
{
private:
  csRefArray<csMeshGeneratorGeometry> geometries;
  csRefArray<csMeshGeneratorMapping> mappings;

public:
  csMeshGenerator ();
  virtual ~csMeshGenerator () { }

  virtual iObject *QueryObject () { return (iObject*)this; }
  virtual iMeshGeneratorGeometry* CreateGeometry ();
  virtual size_t GetGeometryCount () const { return geometries.Length (); }
  virtual iMeshGeneratorGeometry* GetGeometry (size_t idx)
  {
    return geometries[idx];
  }
  virtual void RemoveGeometry (size_t idx);
  virtual iMeshGeneratorMapping* CreateMapping ();
  virtual size_t GetMappingCount () const { return mappings.Length (); }
  virtual iMeshGeneratorMapping* GetMapping (size_t idx)
  {
    return mappings[idx];
  }
  virtual void RemoveMapping (size_t idx);
};

#endif // __CS_MESHGEN_H__

