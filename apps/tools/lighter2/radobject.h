/*
  Copyright (C) 2005 by Marten Svanfeldt

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

#ifndef __RADOBJECT_H__
#define __RADOBJECT_H__

#include "radprimitive.h"
#include "lightmap.h"

namespace lighter
{
  class LightmapUVLayouter;
  class RadObject;

  /**
   * Baseclass for RadObject factories.
   * Needs to be subclassed to provide an interface to a specific mesh
   * object type to be able to import/export geometry data.
   */
  class RadObjectFactory : public csRefCount
  {
  public:
    RadObjectFactory ();

    // Compute lightmap coordinates for this factory. Returns true on success
    virtual bool ComputeLightmapUV (LightmapUVLayouter* layoutEngine);

    // Get a new object
    virtual RadObject* CreateObject ();

    // Parse data
    virtual void ParseFactory (iMeshFactoryWrapper *factory);

    // Write out the data again
    virtual void SaveFactory (iDocumentNode *node);

    // Name of the factory
    csString factoryName;
  protected:
    // All faces, untransformed
    RadPrimitiveArray allPrimitives;

    // Size of the required lightmaps
    LightmapPtrDelArray lightmapTemplates;

    // Factory created from
    iMeshFactoryWrapper *factoryWrapper;

    // String identifying the saver plugin. Should be set from derived
    // classes
    csString saverPluginName;

    friend class RadObject;
  };
  typedef csRefArray<RadObjectFactory> RadObjectFactoryRefArray;
  typedef csHash<csRef<RadObjectFactory>, csString> RadObjectFactoryHash;

  /**
  * A single object in the computational scene.
  * Each object exists in a single spot in a single sector, but there
  * might be more instances created from a single factory in different places.
  */
  class RadObject : public csRefCount
  {
  public:
    // Construct a new RadObject from a RadObjectFactory and transform
    RadObject (RadObjectFactory* object);

    // Initialize the RadObject from factory and wrapper. Call only after
    // constructor and ParseMesh have been called
    virtual void Initialize ();

    // Parse data
    virtual void ParseMesh (iMeshWrapper *wrapper);

    // Write out the data again
    virtual void SaveMesh (iDocumentNode *node);


    // Accessor 
    const LightmapPtrDelArray& GetLightmaps () const 
    { return lightmaps; }

    LightmapPtrDelArray& GetLightmaps () 
    { return lightmaps; }

    const RadPrimitiveArray& GetPrimitives () const
    { return allPrimitives; }

    RadPrimitiveArray& GetPrimitives ()
    { return allPrimitives; }
    

    // Name
    csString meshName;
  protected:
    // All faces, already transformed
    RadPrimitiveArray allPrimitives;

    // Lightmaps associated with this mesh
    LightmapPtrDelArray lightmaps;

    // Factory we where created from
    RadObjectFactory* factory;

    // Reference to the mesh
    iMeshWrapper *meshWrapper;

    friend class  RadObjectFactory;
  };
  typedef csRefArray<RadObject> RadObjectRefArray;
  typedef csHash<csRef<RadObject>, csString> RadObjectHash;

  //////////////////////////////////////////////////////////////////////////
  // SPECIFIC VERISONS
  //////////////////////////////////////////////////////////////////////////
  
  //////////////   Genmesh     /////////////////////////////////////////////
  class RadObjectFactory_Genmesh : public RadObjectFactory
  {
  public:
    RadObjectFactory_Genmesh ();

    // Parse data
    virtual void ParseFactory (iMeshFactoryWrapper *factory);

    // Write out the data again
    virtual void SaveFactory (iDocumentNode *node);
  protected:

    // Extra data saved
    csVector3 *normals;
  };

}

#endif
