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

#include "crystalspace.h"

#include "scene.h"
#include "lighter.h"

using namespace CrystalSpace;

namespace lighter
{
  Scene::Scene ()
  {
    
  }

  void Scene::AddFile (const char* directory)
  {
    //ugly check to see that file isn't loaded twice
    for (unsigned int i = 0; i < sceneFiles.GetSize (); i++)
    {
      if (sceneFiles[i].directory.Compare (directory))
      {
        return;
      }
    }

    // Now setup loading of it
    LoadedFile newFile;
    newFile.directory = directory;
    sceneFiles.Push (newFile);
  }

  bool Scene::LoadFiles ()
  {
    if (sceneFiles.GetSize () == 0) 
      return globalLighter->Report ("No files to load!");


    for (unsigned int i = 0; i < sceneFiles.GetSize (); i++)
    {
      //Change path
      csStringArray paths;
      paths.Push ("/lev/");
      if (!globalLighter->vfs->ChDirAuto (sceneFiles[i].directory, &paths, 0, "world"))
        return globalLighter->Report ("Error setting directory '%s'!", 
          sceneFiles[i].directory.GetData());

      // Load it
      csRef<iFile> buf = globalLighter->vfs->Open ("world", VFS_FILE_READ);
      if (!buf) return globalLighter->Report ("Error opening file 'world'!");

      csRef<iDocument> doc = globalLighter->docSystem->CreateDocument ();
      const char* error = doc->Parse (buf, true);
      if (error != 0)
      {
        return globalLighter->Report ("Document system error: %s!", error);
      }

      sceneFiles[i].document = doc;
      sceneFiles[i].rootNode = doc->GetRoot ();

      // Pass it to the loader
      iBase *res;
      if (!globalLighter->loader->Load (sceneFiles[i].rootNode, res))
        return globalLighter->Report ("Error loading file 'world'!"); 
    }

    return true;
  }

  bool Scene::SaveFiles ()
  {
    if (sceneFiles.GetSize () == 0) 
      return globalLighter->Report ("No files to save!");

    for (unsigned int i = 0; i < sceneFiles.GetSize (); i++)
    {
      //Traverse the DOM, save any factory we encounter
      SaveSceneToDom (sceneFiles[i].rootNode, &sceneFiles[i]);

      //Change path
      csStringArray paths;
      paths.Push ("/lev/");
      if (!globalLighter->vfs->ChDirAuto (sceneFiles[i].directory, &paths, 0, "world"))
        return globalLighter->Report ("Error setting directory '%s'!", 
          sceneFiles[i].directory.GetData());

      // Save it
      csRef<iFile> buf = globalLighter->vfs->Open ("world", VFS_FILE_WRITE);
      if (!buf) return globalLighter->Report ("Error opening file 'world'!");

      const char *err = sceneFiles[i].document->Write (buf);
      if (err)
        return globalLighter->Report ("Error writing file 'world': %s", err);

    }

    return true;
  }

  bool Scene::ParseEngine ()
  {
    int i=0;
 
    iSectorList *sectorList = globalLighter->engine->GetSectors ();
    for (i = 0; i < sectorList->GetCount (); i++)
    {
      ParseSector (sectorList->Get (i));
    }

    return true;
  }

  void Scene::ParseSector (iSector *sector)
  {
    if (!sector) return;

    // Setup a sector struct
    Sector* radSector = new Sector;
    radSector->sectorName = sector->QueryObject ()->GetName ();
    sectors.Put (radSector->sectorName, radSector);

    // Parse all meshes (should have selector later!)
    iMeshList *meshList = sector->GetMeshes ();
    int i;
    for (i = 0; i < meshList->GetCount (); i++)
    {
      if (!ParseMesh (radSector, meshList->Get (i)))
        globalLighter->Report ("Error parsing mesh in sector '%s'!", 
          radSector->sectorName.GetData ());
    }
  }

  RadObject* Scene::ParseMesh (Sector *sector, iMeshWrapper *mesh)
  {
    if (!sector || !mesh) return 0;

    // Get the factory
    RadObjectFactory *factory = ParseMeshFactory (mesh->GetFactory ());
    if (!factory) return 0;

    // Construct a new mesh
    RadObject* obj = factory->CreateObject ();
    if (!obj) return 0;

    obj->ParseMesh (mesh);

    // Save it
    sector->allObjects.Put (obj->meshName, obj);

    return obj;
  }

  RadObjectFactory* Scene::ParseMeshFactory (iMeshFactoryWrapper *factory)
  {
    if (!factory) return 0;

    RadObjectFactory *radFact = 0;
    // Check for duplicate
    csString factName= factory->QueryObject ()->GetName ();
    radFact = radFactories.Get (factName, 0);
    if (radFact) return radFact;

    csRef<iFactory> ifact = scfQueryInterface<iFactory> (
      factory->GetMeshObjectFactory ()->GetMeshObjectType());

    const char* type = ifact->QueryClassID ();

    if (!strcasecmp (type, "crystalspace.mesh.object.genmesh"))
    {
      // Genmesh
      radFact = new RadObjectFactory_Genmesh ();
    }
    radFact->ParseFactory (factory);
    radFactories.Put (radFact->factoryName, radFact);

    return radFact;
  }

  // Helper variable
  csArray<csString> texturesToSave;

  void Scene::SaveSceneToDom (iDocumentNode* r, LoadedFile* fileInfo)
  {
    csRef <iDocumentNode> worldRoot = r->GetNode ("world");

    csRef<iDocumentNodeIterator> it = worldRoot->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> node = it->Next ();
      if (node->GetType () != CS_NODE_ELEMENT) continue;
      
      const char* nodeName = node->GetValue ();

      if (!strcasecmp (nodeName, "meshfact"))
      {
        SaveMeshFactoryToDom (node, fileInfo);
      }
      else if (!strcasecmp (nodeName, "sector"))
      {
        SaveSectorToDom (node, fileInfo);
      }
    }

    // And now save the textures
    // see if we have any <textures> node
    csRef<iDocumentNode> texturesNode = worldRoot->GetNode ("textures");
    if (!texturesNode)
    {
      //Create one
      texturesNode = worldRoot->CreateNodeBefore (CS_NODE_ELEMENT);
      texturesNode->SetValue ("textures");
    }

    for (unsigned int i = 0; i < texturesToSave.GetSize (); i++)
    {
      csString textureFile = texturesToSave[i];
      csString textureName = GetTextureNameFromFilename (textureFile);
      csRef<iDocumentNode> textureNode = 
        texturesNode->CreateNodeBefore (CS_NODE_ELEMENT);
      textureNode->SetValue ("texture");
      textureNode->SetAttribute ("name", textureName.GetData ());
      
      csRef<iDocumentNode> fileNode = 
        textureNode->CreateNodeBefore (CS_NODE_ELEMENT);
      fileNode->SetValue ("file");

      csRef<iDocumentNode> filenameNode =
        fileNode->CreateNodeBefore (CS_NODE_TEXT);
      filenameNode->SetValue (textureFile.GetData ());
    }

    DocumentHelper::RemoveDuplicateChildren(texturesNode, 
      texturesNode->GetNodes ("texture"),
      DocumentHelper::NodeAttributeCompare("name"));
  }

  void Scene::SaveMeshFactoryToDom (iDocumentNode* factNode, LoadedFile* fileInfo)
  {
    // Save a single factory to the dom
    csString name = factNode->GetAttributeValue ("name");
    csRef<RadObjectFactory> radFact = radFactories.Get (name, 0);
    if (radFact)
    {
      // We do have one
      radFact->SaveFactory (factNode);
    }

    // Check if we have any child factories
    csRef<iDocumentNodeIterator> it = factNode->GetNodes ("meshfact");
    while (it->HasNext ())
    {
      csRef<iDocumentNode> node = it->Next ();
      SaveMeshFactoryToDom (node, fileInfo);
    }
  }

  void Scene::SaveSectorToDom (iDocumentNode* sectorNode, LoadedFile* fileInfo)
  {
    csString name = sectorNode->GetAttributeValue ("name");
    csRef<Sector> sector = sectors.Get (name, 0);
    if (sector)
    {
      //ok, have the sector, try to save all objects
      csRef<iDocumentNodeIterator> it = sectorNode->GetNodes ("meshobj");
      while (it->HasNext ())
      {
        csRef<iDocumentNode> node = it->Next ();
        SaveMeshObjectToDom (node, sector, fileInfo);
      }
    }
  }

  void Scene::SaveMeshObjectToDom (iDocumentNode *objNode, Sector* sect, LoadedFile* fileInfo)
  {
    // Save the mesh
    csString name = objNode->GetAttributeValue ("name");
    csRef<RadObject> radObj = sect->allObjects.Get (name, 0);
    if (radObj)
    {
      // We do have one
      radObj->SaveMesh (objNode);

      // Remove any old lightmap svs
      objNode->RemoveNodes (DocumentHelper::FilterDocumentNodeIterator (
        objNode->GetNodes ("shadervar"),
        DocumentHelper::NodeAttributeRegexpTest ("name", "tex lightmap.*")));
      
      // Save the names of the lightmaps
      LightmapPtrDelArray &lightmaps = radObj->GetLightmaps ();
      for (unsigned int i = 0; i < lightmaps.GetSize (); i++)
      {
        csString textureFilename;
        // Save the lightmap
        {
          csRef<iDataBuffer> buf =  globalLighter->vfs->ExpandPath (
            fileInfo->directory.GetData (), true);
          textureFilename = buf->GetData ();
          textureFilename += "lm_";
          textureFilename += radObj->meshName;
          textureFilename += "_";
          textureFilename += i;
          textureFilename += ".png";
        }
        lightmaps[i]->SaveLightmap (textureFilename);

        texturesToSave.Push (textureFilename);

        //add a sv to the meshnode
        csRef<iDocumentNode> svNode = objNode->CreateNodeBefore (CS_NODE_ELEMENT);
        svNode->SetValue ("shadervar");
        csString svName = "tex lightmap";
        if (i!=0)
        {
          svName += " "; svName += i;
        }
        svNode->SetAttribute ("name", svName.GetData ());
        svNode->SetAttribute ("type", "texture");
        
        csString textureName = GetTextureNameFromFilename (textureFilename);
        csRef<iDocumentNode> svValNode = svNode->CreateNodeBefore (CS_NODE_TEXT);
        svValNode->SetValue (textureName.GetData ());
      }
    }

    // Check if we have any child factories
    csRef<iDocumentNodeIterator> it = objNode->GetNodes ("meshobj");
    while (it->HasNext ())
    {
      csRef<iDocumentNode> node = it->Next ();
      SaveMeshObjectToDom (node, sect, fileInfo);
    }
  }

  csString Scene::GetTextureNameFromFilename (csString file)
  {
    csString out (file);
    out.ReplaceAll ("\\", "_"); //replace bad characters
    out.ReplaceAll ("/", "_"); 
    out.ReplaceAll (" ", "_"); 
    out.ReplaceAll (".", "_"); 
    return out;
  }
}
