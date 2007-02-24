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

#ifndef __SCENE_H__
#define __SCENE_H__

#include "radobject.h"
#include "kdtree.h"

namespace lighter
{
  class KDTree;

  // A lightsource
  class Light : public csRefCount
  {
  public:
    csVector3 position;
    csColor color;
    csLightAttenuationMode attenuation;
    csVector3 attenuationConsts;
    bool pseudoDynamic;
    csMD5::Digest lightId;

    csColor freeEnergy;

    csBox3 boundingBox;
  };
  typedef csRefArray<Light> LightRefArray;

  class Scene;

  // Representation of sector in our local setup
  class Sector : public csRefCount
  {
  public:
    Sector (Scene* scene)
      : kdTree (0), scene (scene)
    {}

    ~Sector ()
    {
      delete kdTree;
    }

    // Initialize any extra data in the sector
    void Initialize ();

    // All objects in sector
    RadObjectHash allObjects;

    // All lightsources
    LightRefArray allLights;

    // KD-tree of all primitives in sector
    KDTree *kdTree;

    // Sector-name
    csString sectorName;

    Scene* scene;
  };
  typedef csHash<csRef<Sector>, csString> SectorHash;

  class Scene
  {
  public:
    Scene ();

    // Add a file for later loading
    void AddFile (const char* directory);

    // Load all files, and parse the loaded info
    bool LoadFiles ();

    // Save all files we've loaded. Will save any changed factory and mesh
    bool SaveFiles ();

    // Parse in our scene from the engine
    bool ParseEngine ();

    // Data access
    inline RadObjectFactoryHash& GetFactories () 
    { return radFactories; }

    inline const RadObjectFactoryHash& GetFactories () const
    { return radFactories; }

    inline SectorHash& GetSectors () { return sectors; }

    inline const SectorHash& GetSectors () const { return sectors; }

    const LightmapPtrDelArray& GetLightmaps () const 
    { return lightmaps; }

    LightmapPtrDelArray& GetLightmaps () 
    { return lightmaps; }

    Lightmap* GetLightmap (uint lightmapID, Light* light);
    csArray<LightmapPtrDelArray*> GetAllLightmaps ();
  protected:
    
    // Rad factories
    RadObjectFactoryHash radFactories;
 
    // All sectors
    SectorHash sectors;

    LightmapPtrDelArray lightmaps;
    typedef csHash<LightmapPtrDelArray*, csPtrKey<Light> > PDLightmapsHash;
    PDLightmapsHash pdLightmaps;

    struct LoadedFile
    {
      csRef<iDocumentNode> rootNode;
      csRef<iDocument> document;
      csString directory; //VFS name, full path
    };

    // All files loaded into scene
    csArray<LoadedFile> sceneFiles;
    // Helper variable
    struct SaveTexture
    {
      csString filename;
      csString texname;
      csStringArray pdLightmapFiles;
      csStringArray pdLightIDs;
    };
    csArray<SaveTexture> texturesToSave;
    csSet<csString> texturesToClean;

    // Save functions
    void CleanOldLightmaps (LoadedFile* fileInfo, 
      const csStringArray& texFileNames);
    void SaveSceneToDom (iDocumentNode* root, LoadedFile* fileInfo);
    void SaveMeshFactoryToDom (iDocumentNode* factNode, LoadedFile* fileInfo);
    void SaveSectorToDom (iDocumentNode* sectorNode, LoadedFile* fileInfo);
    void SaveMeshObjectToDom (iDocumentNode *objNode, Sector* sect, LoadedFile* fileInfo);
    
    // Load functions
    void ParseSector (iSector *sector);
    enum MeshParseResult
    {
      Failure, Success, NotAGenMesh
    };
    MeshParseResult ParseMesh (Sector *sector, iMeshWrapper *mesh);
    MeshParseResult ParseMeshFactory (iMeshFactoryWrapper *factory, 
      RadObjectFactory*& radFact);

  };
}

#endif
