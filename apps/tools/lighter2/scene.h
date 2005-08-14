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

namespace lighter
{
  struct Sector : public csRefCount
  {
  public:
    RadObjectHash allObjects;

    csString sectorName;
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

  protected:
    
    // Rad factories
    RadObjectFactoryHash radFactories;
 
    // All sectors
    SectorHash sectors;

    struct LoadedFile
    {
      csRef<iDocumentNode> rootNode;
      csRef<iDocument> document;
      csString directory; //VFS name, full path
    };

    // All files loaded into scene
    csArray<LoadedFile> sceneFiles;

    // Save functions
    void SaveSceneToDom (iDocumentNode* root, LoadedFile* fileInfo);
    void SaveMeshFactoryToDom (iDocumentNode* factNode, LoadedFile* fileInfo);
    void SaveSectorToDom (iDocumentNode* sectorNode, LoadedFile* fileInfo);
    void SaveMeshObjectToDom (iDocumentNode *objNode, Sector* sect, LoadedFile* fileInfo);
    csString GetTextureNameFromFilename (csString file);
    
    // Load functions
    void ParseSector (iSector *sector);
    RadObject* ParseMesh (Sector *sector, iMeshWrapper *mesh);
    RadObjectFactory* ParseMeshFactory (iMeshFactoryWrapper *factory);

  };
}

#endif
