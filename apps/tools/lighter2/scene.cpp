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
#include "kdtree.h"
#include "statistics.h"
#include "object_genmesh.h"

using namespace CS;

namespace lighter
{

  void Sector::Initialize ()
  {
    // Initialize objects
    ObjectHash::GlobalIterator objIt = allObjects.GetIterator ();
    while (objIt.HasNext ())
    {
      csRef<Object> obj = objIt.Next ();
      obj->Initialize ();
    }
  }

  void Sector::PrepareLighting ()
  {
    // Prepare lighting of objects
    ObjectHash::GlobalIterator objIt = allObjects.GetIterator ();
    while (objIt.HasNext ())
    {
      csRef<Object> obj = objIt.Next ();
      obj->PrepareLighting ();
    }
  }

  void Sector::BuildKDTree ()
  {
    // Build KD-tree
    ObjectHash::GlobalIterator objIt = allObjects.GetIterator ();
    KDTreeBuilder builder;
    kdTree = builder.BuildTree (objIt);
  }

  //-------------------------------------------------------------------------

  Scene::Scene () : lightmapPostProc (this)
  {
  }

  Scene::~Scene ()
  {
    PDLightmapsHash::GlobalIterator pdlIt = pdLightmaps.GetIterator();
    while (pdlIt.HasNext())
    {
      LightmapPtrDelArray* lm = pdlIt.Next();
      delete lm;
    }
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

  bool Scene::LoadFiles (Statistics::SubProgress& progress)
  {
    progress.SetProgress (0);

    if (sceneFiles.GetSize () == 0) 
      return globalLighter->Report ("No files to load!");

    const float div = 100.0f / sceneFiles.GetSize ();

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
      // ChDirAuto() may have created an automount, use that...
      sceneFiles[i].directory = globalLighter->vfs->GetCwd ();

      // Pass it to the loader
      iBase *res;
      if (!globalLighter->loader->Load (sceneFiles[i].rootNode, res))
        return globalLighter->Report ("Error loading file 'world'!");

      // Parses meshes from engine
      ParseEngine ();

      progress.IncProgress (div);
    }

    /* We have turned everything needed into our own objects which keep
       refs to the engine objects as needed. Hence instruct engine to
       release everything it holds. */
    globalLighter->engine->DeleteAll();

    progress.SetProgress (100);
    return true;
  }

  bool Scene::SaveWorldFactories (/*Statistics::SubProgress& progress*/)
  {
    //progress.SetProgress (0);

    if (sceneFiles.GetSize () == 0)
    {
      globalLighter->Report ("No files to save!");
      return false;
    }

    float fileProgress = 100.0f / sceneFiles.GetSize ();
    for (size_t i = 0; i < sceneFiles.GetSize (); i++)
    {
      //Traverse the DOM, save any factory we encounter
      SaveSceneFactoriesToDom (sceneFiles[i].rootNode, &sceneFiles[i]);

      //progress.IncProgress (fileProgress);
    }

    //progress.SetProgress (100);
    return true;
  }

  bool Scene::SaveWorldMeshes (/*Statistics::SubProgress& progress*/)
  {
    //progress.SetProgress (0);

    if (sceneFiles.GetSize () == 0)
    {
      globalLighter->Report ("No files to save!");
      return false;
    }

    float fileProgress = 100.0f / sceneFiles.GetSize ();
    for (size_t i = 0; i < sceneFiles.GetSize (); i++)
    {
      //Traverse the DOM, save any mesh we encounter
      SaveSceneMeshesToDom (sceneFiles[i].rootNode, &sceneFiles[i]);

      //progress.IncProgress (fileProgress);
    }

    //progress.SetProgress (100);
    return true;
  }

  bool Scene::FinishWorldSaving (/*Statistics::SubProgress& progress*/)
  {
    //progress.SetProgress (0);

    if (sceneFiles.GetSize () == 0)
    {
      globalLighter->Report ("No files to save!");
      return false;
    }

    float fileProgress = 100.0f / sceneFiles.GetSize ();
    for (size_t i = 0; i < sceneFiles.GetSize (); i++)
    {
      //Change path
      csStringArray paths;
      paths.Push ("/lev/");
      if (!globalLighter->vfs->ChDirAuto (sceneFiles[i].directory, &paths, 0, "world"))
      {
        globalLighter->Report ("Error setting directory '%s'!", 
          sceneFiles[i].directory.GetData());
        return false;
      }

      // Save it
      csRef<iFile> buf = globalLighter->vfs->Open (".lighter2.world", VFS_FILE_WRITE);
      if (!buf) 
      {
        globalLighter->Report (
          "Error opening file '.lighter2.world' for writing!");
        return false;
      }

      const char *err = sceneFiles[i].document->Write (buf);
      if (err)
      {
        globalLighter->Report ("Error writing file '.lighter2.world': %s", err);
        return false;
      }

      // Release document, not needed any more.
      sceneFiles[i].document.Invalidate();
      sceneFiles[i].rootNode.Invalidate();

      //progress.IncProgress (fileProgress);
    }

    // Ensure no archives are cached in memory (for memory consumption reasons)
    if (!globalLighter->vfs->Sync())
    {
      globalLighter->Report ("Could not synchronize VFS.");
      return false;
    }

    //progress.SetProgress (100);
    return true;
  }

  bool Scene::ApplyWorldChanges (Statistics::SubProgress& progress)
  {
    progress.SetProgress (0);

    if (sceneFiles.GetSize () == 0)
    {
      globalLighter->Report ("No files to save!");
      return false;
    }

    float fileProgress = 100.0f / sceneFiles.GetSize ();
    for (size_t i = 0; i < sceneFiles.GetSize (); i++)
    {
      //Change path
      csStringArray paths;
      paths.Push ("/lev/");
      if (!globalLighter->vfs->ChDirAuto (sceneFiles[i].directory, &paths, 0, "world"))
      {
        globalLighter->Report ("Error setting directory '%s'!", 
          sceneFiles[i].directory.GetData());
        return false;
      }

      // Read temp file...
      csRef<iDataBuffer> buf = globalLighter->vfs->ReadFile (".lighter2.world", false);
      if (!buf) 
      {
        globalLighter->Report ("Error reading file '.lighter2.world'!");
        return false;
      }

      // ... write to 'world' ...
      if (!globalLighter->vfs->WriteFile ("world", buf->GetData(), 
        buf->GetSize()))
      {
        globalLighter->Report ("Error writing to 'world'!");
        return false;
      }

      // ... and remove the backup.
      buf.Invalidate();
      if (!globalLighter->vfs->DeleteFile (".lighter2.world"))
      {
        globalLighter->Report ("Error deleting file '.lighter2.world'!");
        return false;
      }

      // Also clear out genmesh cache.
      {
        static const char gmCacheDir[] = "cache/genmesh_lm";
        csRef<iStringArray> cacheFiles = 
          globalLighter->vfs->FindFiles (csString (gmCacheDir) + "/*");
        if (cacheFiles.IsValid())
        {
          for (size_t i = 0; i < cacheFiles->GetSize(); i++)
            globalLighter->vfs->DeleteFile (cacheFiles->Get (i));
        }
        globalLighter->vfs->DeleteFile (gmCacheDir);
      }

      progress.IncProgress (fileProgress);
    }

    progress.SetProgress (100);
    return true;
  }

  static const char lightmapLibraryName[] = "lightmaps.cslib";

  bool Scene::SaveLightmaps (Statistics::SubProgress& progress)
  {
    progress.SetProgress (0);

    if (sceneFiles.GetSize () == 0)
    {
      globalLighter->Report ("No files to save!");
      return false;
    }

    float fileProgress = 100.0f / sceneFiles.GetSize ();
    for (size_t i = 0; i < sceneFiles.GetSize (); i++)
    {
      //Change path
      csStringArray paths;
      paths.Push ("/lev/");
      if (!globalLighter->vfs->ChDirAuto (sceneFiles[i].directory, &paths, 0, "world"))
      {
        globalLighter->Report ("Error setting directory '%s'!", 
          sceneFiles[i].directory.GetData());
        return false;
      }

      csRef<iDocument> doc = globalLighter->docSystem->CreateDocument ();
      csRef<iFile> buf = globalLighter->vfs->Open (lightmapLibraryName, 
        VFS_FILE_READ);
      if (buf.IsValid())
      {
        // Open lightmaps lib if it exists...
        const char* err = doc->Parse (buf);
        if (err != 0)
        {
          globalLighter->Report ("Error parsing file '%s': %s", 
            lightmapLibraryName, err);
          return false;
        }
      }
      else
      {
        // ... create empty otherwise
        csRef<iDocumentNode> root = doc->CreateRoot ();
        csRef<iDocumentNode> libNode = root->CreateNodeBefore (
          CS_NODE_ELEMENT);
        libNode->SetValue ("library");
      }
      SaveLightmapsToDom (doc->GetRoot (), &sceneFiles[i]);

      // Save it
      buf.Invalidate();
      buf = globalLighter->vfs->Open (lightmapLibraryName, VFS_FILE_WRITE);
      if (!buf) 
      {
        globalLighter->Report ("Error opening file '%s' for writing!", 
          lightmapLibraryName);
        return false;
      }

      const char *err = doc->Write (buf);
      if (err)
      {
        globalLighter->Report ("Error writing file '%s': %s", lightmapLibraryName, err);
        return false;
      }

      // Finally delete any leftover lightmap files
      CleanOldLightmaps (&sceneFiles[i]);

      progress.IncProgress (fileProgress);
    }

    progress.SetProgress (100);

    return true;
  }

  bool Scene::SaveMeshesPostLighting (Statistics::SubProgress& progress)
  {
    progress.SetProgress (0);

    if (sceneFiles.GetSize () == 0)
    {
      globalLighter->Report ("No files to save!");
      return false;
    }

    float fileProgress = 100.0f / sceneFiles.GetSize ();
    for (size_t i = 0; i < sceneFiles.GetSize (); i++)
    {
      //Change path
      csStringArray paths;
      paths.Push ("/lev/");
      if (!globalLighter->vfs->ChDirAuto (sceneFiles[i].directory, &paths, 0, "world"))
      {
        globalLighter->Report ("Error setting directory '%s'!", 
          sceneFiles[i].directory.GetData());
        return false;
      }

      size_t meshObjNum = sceneFiles[i].fileObjects.GetSize();
      if (meshObjNum == 0)
      {
        progress.IncProgress (fileProgress);
        continue;
      }
      float meshProgress = fileProgress / meshObjNum;
      for (size_t o = 0; o < meshObjNum; o++)
      {
        Object* obj = sceneFiles[i].fileObjects[o];
        obj->SaveMeshPostLighting (this);
        progress.IncProgress (meshProgress);
      }
      
    }

    progress.SetProgress (100);

    return true;
  }

  Lightmap* Scene::GetLightmap (uint lightmapID, Light* light)
  {
    if (!light || !light->IsPDLight ())
      return lightmaps[lightmapID];

    light = light->GetOriginalLight();
    LightmapPtrDelArray* pdLights = pdLightmaps.Get (light, 0);
    if (pdLights == 0)
    {
      pdLights = new LightmapPtrDelArray;
      for (size_t i = 0; i < lightmaps.GetSize(); i++)
      {
        Lightmap* lm = new Lightmap (lightmaps[i]->GetWidth(), 
          lightmaps[i]->GetHeight());
        lm->Initialize ();
        pdLights->Push (lm);
      }
      pdLightmaps.Put (light, pdLights);
    }
    return pdLights->Get (lightmapID);
  }

  csArray<LightmapPtrDelArray*> Scene::GetAllLightmaps ()
  {
    csArray<LightmapPtrDelArray*> allLms;
    allLms.Push (&lightmaps);

    PDLightmapsHash::GlobalIterator pdlIt = pdLightmaps.GetIterator();
    while (pdlIt.HasNext())
    {
      LightmapPtrDelArray* lm = pdlIt.Next();
      allLms.Push (lm);
    }

    return allLms;
  }

  bool Scene::ParseEngine (/*Statistics::SubProgress& progress*/)
  {
    //progress.SetProgress (0);

    // Parse sectors
    iSectorList *sectorList = globalLighter->engine->GetSectors ();
    for (int i = 0; i < sectorList->GetCount (); i++)
    {
      ParseSector (sectorList->Get (i));
    }

    // Parse portals
    SectorOrigSectorHash::GlobalIterator sectIt = originalSectorHash.GetIterator ();
    while (sectIt.HasNext ())
    {
      Sector* sector;
      csPtrKey<iSector> srcSector;

      sector = sectIt.Next (srcSector);
      ParsePortals (srcSector, sector);
    }

    // Propagate light in sectors
    sectIt.Reset ();
    while (sectIt.HasNext ())
    {
      Sector* sector = sectIt.Next ();
      
      LightRefArray tmpArray = sector->allNonPDLights;
      LightRefArray::Iterator lid = tmpArray.GetIterator ();
      while (lid.HasNext ())
      {
        Light* l = lid.Next ();
        if (l->IsRealLight ())
          PropagateLight (l, l->GetFrustum ());
      }

      tmpArray = sector->allPDLights;
      lid = tmpArray.GetIterator ();
      while (lid.HasNext ())
      {
        Light* l = lid.Next ();
        
        if (l->IsRealLight ())
          PropagateLight (l, l->GetFrustum ());
      }
    }

    // Map mesh objects loaded from each scene file to it
    for (size_t i = 0; i < sceneFiles.GetSize (); i++)
    {
      csRef<iDocumentNode> worldNode = 
        sceneFiles[i].rootNode->GetNode ("world");

      csRef<iDocumentNodeIterator> sectorNodeIt = 
        worldNode->GetNodes ("sector");
      while (sectorNodeIt->HasNext())
      {
        csRef<iDocumentNode> sectorNode = sectorNodeIt->Next ();
        const char* sectorName = sectorNode->GetAttributeValue ("name");
        if (!sectorName) continue;
        const Sector* sector = sectors.Get (sectorName, (Sector*)0);
        if (!sector) continue;

        csRef<iDocumentNodeIterator> meshObjNodeIt = 
          sectorNode->GetNodes ("meshobj");
        while (meshObjNodeIt->HasNext())
        {
          csRef<iDocumentNode> meshObjNode = meshObjNodeIt->Next ();
          const char* meshName = meshObjNode->GetAttributeValue ("name");
          if (!meshName) continue;

          Object* obj = sector->allObjects.Get (meshName, (Object*)0);
          if (!obj) continue;
          sceneFiles[i].fileObjects.Push (obj);
        }
      }
    }

    //progress.SetProgress (100);
    return true;
  }

  void Scene::ParseSector (iSector *sector)
  {
    if (!sector) return;

    // Setup a sector struct
    const char* sectorName = sector->QueryObject ()->GetName ();
    csRef<Sector> radSector = sectors.Get (sectorName, 0);
    if (radSector == 0)
    {
      radSector.AttachNew (new Sector (this));
      radSector->sectorName = sectorName;
      sectors.Put (radSector->sectorName, radSector);
      originalSectorHash.Put (sector, radSector);
    }

    // Parse all meshes (should have selector later!)
    iMeshList *meshList = sector->GetMeshes ();
    for (int i = meshList->GetCount (); i-- > 0;)
    {
      iMeshWrapper* mesh = meshList->Get (i);
      csRef<Object> obj;
      bool isPortal = mesh->GetPortalContainer() != 0;
      if (ParseMesh (radSector, mesh, obj) == Failure)
      {
        if (!isPortal)
          globalLighter->Report ("Error parsing mesh '%s' in sector '%s'!", 
            mesh->QueryObject()->GetName(), radSector->sectorName.GetData ());
      }
      // Mesh is parsed, release resources
      if (!isPortal) meshList->Remove (i);
    }

    // Parse all lights (should have selector later!)
    iLightList *lightList = sector->GetLights ();
    for (int i = lightList->GetCount (); i-- > 0;)
    {
      iLight *light = lightList->Get (i);
      if (light->GetDynamicType() == CS_LIGHT_DYNAMICTYPE_DYNAMIC) 
      {
        lightList->Remove (i);
        continue;
      }

      bool isPD = light->GetDynamicType() == CS_LIGHT_DYNAMICTYPE_PSEUDO;

      // Atm, only point light
      csRef<PointLight> intLight;
      intLight.AttachNew (new PointLight (radSector));

      intLight->SetPosition (light->GetMovable ()->GetFullPosition ());
      intLight->SetColor (isPD ? csColor (1.0f) : light->GetColor ());

      intLight->SetAttenuation (light->GetAttenuationMode (),
        light->GetAttenuationConstants ());
      intLight->SetPDLight (isPD);
      intLight->SetLightID (light->GetLightID());

      intLight->SetRadius (light->GetCutoffDistance ());

      if (isPD)
        radSector->allPDLights.Push (intLight);
      else
        radSector->allNonPDLights.Push (intLight);

      lightList->Remove (i);
    }
  }

  void Scene::ParsePortals (iSector *srcSect, Sector* sector)
  {
    // Parse portals
    const csSet<csPtrKey<iMeshWrapper> >& allPortals = srcSect->GetPortalMeshes ();
    csSet<csPtrKey<iMeshWrapper> >::GlobalIterator it = allPortals.GetIterator ();

    while (it.HasNext ())
    {
      iMeshWrapper *portalMesh = it.Next ();
      iPortalContainer *portalCont = portalMesh->GetPortalContainer ();

      for (int i = 0; i < portalCont->GetPortalCount (); ++i)
      {
        iPortal* portal = portalCont->GetPortal (i);
        portal->CompleteSector (0);

        iSector* destSector = portal->GetSector ();
        if (!destSector)
          continue;

        csRef<Portal> lightPortal; lightPortal.AttachNew (new Portal);
        lightPortal->sourceSector = sector;
        lightPortal->destSector = originalSectorHash.Get (destSector, 0);
        lightPortal->portalPlane = portal->GetWorldPlane ();
        
        if (portal->GetFlags ().Check (CS_PORTAL_WARP))
        {
          //Compute wrapping
          portal->ObjectToWorld (portalMesh->GetMovable ()->GetFullTransform (),
            lightPortal->wrapTransform);
        }

        // Get vertices, in world space
        int* vi = portal->GetVertexIndices ();
        const csVector3* vertices = portal->GetWorldVertices ();
        for (int j = 0; j < portal->GetVertexIndicesCount (); ++j)
        {
          lightPortal->worldVertices.Push (vertices[vi[j]]);
        }

        sector->allPortals.Push (lightPortal);
      }
    }
    // Remove the remaining portal meshes
    srcSect->GetMeshes()->RemoveAll();
  }

  void Scene::PropagateLight (Light* light, const csFrustum& lightFrustum)
  {
    //Propagate light through all portals in its current sector, setup proxy-lights in targets
    Sector* sourceSector = light->GetSector ();
    const csVector3& lightCenter = light->GetPosition ();
    const csBox3& lightBB = light->GetBoundingBox ();

    PortalRefArray::Iterator it = sourceSector->allPortals.GetIterator ();
    while (it.HasNext ())
    {
      Portal* portal = it.Next ();
      
      if (portal->portalPlane.Classify (lightCenter) < -0.01f && //light in front of portal
        true) //csIntersect3::BoxPlane (lightBB, portal->portalPlane)) //light at least cuts portal plane
      {
        const csVector3& origin = lightFrustum.GetOrigin ();
        CS_ALLOC_STACK_ARRAY(csVector3, tmpVertices, portal->worldVertices.GetSize ());
        for (size_t i = 0; i < portal->worldVertices.GetSize (); ++i)
        {
          tmpVertices[i] = portal->worldVertices[i] - origin;
        }

        csRef<csFrustum> newFrustum = lightFrustum.Intersect (
          tmpVertices, portal->worldVertices.GetSize ());

        if (newFrustum && !newFrustum->IsEmpty ())
        {
          //Have something left to push through, use that          
          newFrustum->Transform (&portal->wrapTransform);

          // Now, setup our proxy light
          csRef<ProxyLight> proxyLight;
          proxyLight.AttachNew (new ProxyLight (portal->destSector, light, *newFrustum, 
            portal->wrapTransform, portal->portalPlane));
          proxyLight->SetPosition (newFrustum->GetOrigin ());
          
          if (proxyLight->IsPDLight ())
            portal->destSector->allPDLights.Push (proxyLight);
          else
            portal->destSector->allNonPDLights.Push (proxyLight);

          PropagateLight (proxyLight, proxyLight->GetFrustum ());
        }
      }
    }
  }

  Scene::MeshParseResult Scene::ParseMesh (Sector *sector, 
                                           iMeshWrapper *mesh,
                                           csRef<Object>& obj)
  {
    if (!sector || !mesh) return Failure;

    if (sector->allObjects.Contains (mesh->QueryObject()->GetName()))
    {
      globalLighter->Report (
        "A mesh named '%s' already exists in sector '%s'",
        mesh->QueryObject()->GetName(), sector->sectorName.GetData());
      return Success;
    }

    // Get the factory
    csRef<ObjectFactory> factory;
    MeshParseResult parseFact = ParseMeshFactory (mesh->GetFactory (), factory);
    if (parseFact != Success) return parseFact;
    if (!factory) return Failure;

    // Construct a new mesh
    obj = factory->CreateObject ();
    if (!obj) return Failure;

    obj->ParseMesh (mesh);

    // Save it
    sector->allObjects.Put (obj->meshName, obj);

    return Success;
  }

  Scene::MeshParseResult Scene::ParseMeshFactory (iMeshFactoryWrapper *factory,
                                                  csRef<ObjectFactory>& radFact)
  {
    if (!factory) return Failure;

    // Check for duplicate
    csString factName = factory->QueryObject ()->GetName ();
    radFact = radFactories.Get (factName, (ObjectFactory*)0);
    if (radFact) return Success;

    csRef<iFactory> ifact = scfQueryInterface<iFactory> (
      factory->GetMeshObjectFactory ()->GetMeshObjectType());

    const char* type = ifact->QueryClassID ();

    if (!strcasecmp (type, "crystalspace.mesh.object.genmesh"))
    {
      // Genmesh
      radFact.AttachNew (new ObjectFactory_Genmesh ());
    }
    else
      return NotAGenMesh;
    radFact->ParseFactory (factory);
    radFactories.Put (radFact->factoryName, radFact);

    return Success;
  }

  void Scene::CollectDeleteTextures (iDocumentNode* textureNode,
                                     csSet<csString>& filesToDelete)
  {
    csRef<iDocumentNode> fileNode = textureNode->GetNode ("file");
    if (fileNode.IsValid())
      filesToDelete.Add (fileNode->GetContentsValue());
    csRef<iDocumentNode> typeNode = textureNode->GetNode ("type");
    if (typeNode.IsValid())
    {
      const char* typeStr = typeNode->GetContentsValue();
      if (typeStr && (strcmp (typeStr,
          "crystalspace.texture.loader.pdlight") == 0))
      {
        csRef<iDocumentNode> paramsNode = textureNode->GetNode ("params");
        if (paramsNode.IsValid())
        {
          csRef<iDocumentNodeIterator> mapNodes = 
            paramsNode->GetNodes ("map");
          while (mapNodes->HasNext())
          {
            csRef<iDocumentNode> mapNode = mapNodes->Next();
            if (mapNode->GetType() != CS_NODE_ELEMENT) continue;
            const char* mapName = mapNode->GetContentsValue();
            if (mapName != 0)
              filesToDelete.Add (mapName);
          }
        }
      }
    }
  }

  void Scene::BuildLightmapTextureList (csStringArray& texturesToSave)
  {
    for (size_t i = 0; i < lightmaps.GetSize (); i++)
    {
      csString textureFilename;
      textureFilename = "lightmaps/";
      textureFilename += (uint)i;
      textureFilename += ".png";
      lightmaps[i]->SetFilename (textureFilename);

      texturesToSave.Push (lightmaps[i]->GetTextureName());
    }
  }

  void Scene::CleanOldLightmaps (LoadedFile* fileInfo)
  {
    csVfsDirectoryChanger chdir (globalLighter->vfs);
    chdir.ChangeTo (fileInfo->directory);

    csSet<csString>::GlobalIterator iter = 
      fileInfo->texFileNamesToDelete.GetIterator();
    while (iter.HasNext ())
    {
      globalLighter->vfs->DeleteFile (iter.Next());
    }
  }

  void Scene::SaveSceneFactoriesToDom (iDocumentNode* r, LoadedFile* fileInfo)
  {
    csSet<csString> savedFactories;
    
    csRef <iDocumentNode> worldRoot = r->GetNode ("world");

    csRef<iDocumentNodeIterator> it = worldRoot->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> node = it->Next ();
      if (node->GetType () != CS_NODE_ELEMENT) continue;
      
      const char* nodeName = node->GetValue ();

      if (!strcasecmp (nodeName, "library"))
      {
        const char* libFile = node->GetAttributeValue ("file");
        if (!libFile) libFile = node->GetContentsValue();
        if ((libFile != 0) && (strcmp (libFile, lightmapLibraryName) != 0))
          HandleLibraryNode (savedFactories, node, fileInfo);
      }
      else if (!strcasecmp (nodeName, "meshfact"))
      {
        SaveMeshFactoryToDom (savedFactories, node, fileInfo);
      }
    }
  }

  void Scene::SaveSceneMeshesToDom (iDocumentNode* r, LoadedFile* fileInfo)
  {
    bool hasLightmapsLibrary = false;
    csStringArray texturesToSave;
    
    BuildLightmapTextureList (texturesToSave);

    csRef <iDocumentNode> worldRoot = r->GetNode ("world");

    csRef<iDocumentNodeIterator> it = worldRoot->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> node = it->Next ();
      if (node->GetType () != CS_NODE_ELEMENT) continue;
      
      const char* nodeName = node->GetValue ();

      if (!strcasecmp (nodeName, "library"))
      {
        const char* libFile = node->GetAttributeValue ("file");
        if (!libFile) libFile = node->GetContentsValue();
        if ((libFile != 0) && (strcmp (libFile, lightmapLibraryName) == 0))
          hasLightmapsLibrary = true;
      }
      else if (!strcasecmp (nodeName, "sector"))
      {
        SaveSectorToDom (node, fileInfo);
      }
    }

    // see if we have any <textures> node
    csRef<iDocumentNode> texturesNode = worldRoot->GetNode ("textures");
    if (texturesNode)
    {
      csSet<csString> texFileNamesToDelete;
      // Clean out old lightmap textures
      {
        csRefArray<iDocumentNode> nodesToDelete;
        csRef<iDocumentNodeIterator> it = texturesNode->GetNodes ("texture");
        while (it->HasNext())
        {
          csRef<iDocumentNode> child = it->Next();
          if (child->GetType() != CS_NODE_ELEMENT) continue;

          const char* name = child->GetAttributeValue ("name");
          if ((name != 0) && fileInfo->texturesToClean.Contains (name))
          {
            CollectDeleteTextures (child, texFileNamesToDelete);
            nodesToDelete.Push (child);
          }
        }
        for (size_t i = 0; i < nodesToDelete.GetSize(); i++)
          texturesNode->RemoveNode (nodesToDelete[i]);

        CleanOldLightmaps (fileInfo);
      }
    }

    // Generate a <library> node for the external lightmaps library
    if (!hasLightmapsLibrary)
    {
      const char* const createNodeBeforeNames[] = { "materials", "meshfact", "sector" };

      csRef<iDocumentNode> createBefore;
      for (size_t n = 0; n < sizeof(createNodeBeforeNames)/sizeof(const char*); 
        n++)
      {
        createBefore = worldRoot->GetNode (createNodeBeforeNames[n]);
        if (createBefore.IsValid ()) break;
      }
      csRef<iDocumentNode> libraryNode = 
        worldRoot->CreateNodeBefore (CS_NODE_ELEMENT, createBefore);
      libraryNode->SetValue ("library");

      csRef<iDocumentNode> libraryContentsNode = 
        libraryNode->CreateNodeBefore (CS_NODE_TEXT);
      libraryContentsNode->SetValue (lightmapLibraryName);
    }
  }

  bool Scene::SaveSceneLibrary (csSet<csString>& savedFactories, 
                                const char* libFile, LoadedFile* fileInfo)
  {
    csRef<iFile> buf = globalLighter->vfs->Open (libFile, VFS_FILE_READ);
    if (!buf) 
    {
      globalLighter->Report ("Error opening file '%s'!", libFile);
      return false;
    }

    csRef<iDocument> doc = globalLighter->docSystem->CreateDocument ();
    const char* error = doc->Parse (buf, true);
    if (error != 0)
    {
      globalLighter->Report ("Document system error: %s!", error);
      return false;
    }

    csRef<iDocumentNode> docRoot = doc->GetRoot();

    csRef<iDocumentNode> libRoot = docRoot->GetNode ("library");
    if (!libRoot)
    {
      globalLighter->Report ("'%s' is not a library", libFile);
      return false;
    }

    csRef<iDocumentNodeIterator> it = libRoot->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> node = it->Next ();
      if (node->GetType () != CS_NODE_ELEMENT) continue;
      
      const char* nodeName = node->GetValue ();

      if (!strcasecmp (nodeName, "library"))
      {
        HandleLibraryNode (savedFactories, node, fileInfo);
      }
      else if (!strcasecmp (nodeName, "meshfact"))
      {
        SaveMeshFactoryToDom (savedFactories, node, fileInfo);
      }
    }

    buf = globalLighter->vfs->Open (libFile, VFS_FILE_WRITE);
    if (!buf) 
    {
      globalLighter->Report ("Error opening file '%s' for writing!", libFile);
      return false;
    }
    error = doc->Write (buf);
    if (error != 0)
    {
      globalLighter->Report ("Document system error: %s!", error);
      return false;
    }

    return true;
  }

  void Scene::HandleLibraryNode (csSet<csString>& savedFactories, 
                                 iDocumentNode* node, LoadedFile* fileInfo)
  {
    const char* file = node->GetAttributeValue ("file");
    if (file)
    {
      csVfsDirectoryChanger changer (globalLighter->vfs);
      const char* path = node->GetAttributeValue ("path");

      if (path)
      {
        changer.PushDir ();
        globalLighter->vfs->ChDir (path);
      }
      SaveSceneLibrary (savedFactories, file, fileInfo);
    }
    else
    {
      SaveSceneLibrary (savedFactories, node->GetContentsValue (), fileInfo);
    }
  }

  void Scene::SaveMeshFactoryToDom (csSet<csString>& savedObjects, 
                                    iDocumentNode* factNode, LoadedFile* fileInfo)
  {
    // Save a single factory to the dom
    csString name = factNode->GetAttributeValue ("name");
    if (savedObjects.Contains (name))
    {
      globalLighter->Report (
        "A factory named '%s' already exists",
        name.GetData());
      return;
    }
    csRef<ObjectFactory> radFact = radFactories.Get (name, 
      (ObjectFactory*)0);
    if (radFact)
    {
      // We do have one
      radFact->SaveFactory (factNode);
      savedObjects.AddNoTest (name);
    }

    // Check if we have any child factories
    csRef<iDocumentNodeIterator> it = factNode->GetNodes ("meshfact");
    while (it->HasNext ())
    {
      csRef<iDocumentNode> node = it->Next ();
      SaveMeshFactoryToDom (savedObjects, node, fileInfo);
    }
  }

  void Scene::SaveSectorToDom (iDocumentNode* sectorNode, LoadedFile* fileInfo)
  {
    csString name = sectorNode->GetAttributeValue ("name");
    csRef<Sector> sector = sectors.Get (name, (Sector*)0);
    if (sector)
    {
      //ok, have the sector, try to save all objects
      csSet<csString> savedObjects;
      csRef<iDocumentNodeIterator> it = sectorNode->GetNodes ("meshobj");
      while (it->HasNext ())
      {
        csRef<iDocumentNode> node = it->Next ();
        SaveMeshObjectToDom (savedObjects, node, sector, fileInfo);
      }
    }
  }

  void Scene::SaveMeshObjectToDom (csSet<csString>& savedObjects, 
                                   iDocumentNode *objNode, 
                                   Sector* sect, LoadedFile* fileInfo)
  {
    // Save the mesh
    csString name = objNode->GetAttributeValue ("name");
    if (savedObjects.Contains (name))
      /* Already emitted an "ignoring duplicate mesh" earlier, so just skip 
         saving here. */
      return;
    csRef<Object> radObj = sect->allObjects.Get (name, (Object*)0);
    if (radObj)
    {
      // We do have one
      radObj->StripLightmaps (fileInfo->texturesToClean);
      radObj->SaveMesh (sect, objNode);
      radObj->FreeNotNeededForLighting ();
      savedObjects.AddNoTest (name);

      // Remove any old lightmap svs
      objNode->RemoveNodes (DocumentHelper::FilterDocumentNodeIterator (
        objNode->GetNodes ("shadervar"),
        DocumentHelper::NodeAttributeRegexpTest ("name", "tex lightmap.*")));
    }

    // Check if we have any child factories
    csRef<iDocumentNodeIterator> it = objNode->GetNodes ("meshobj");
    while (it->HasNext ())
    {
      csRef<iDocumentNode> node = it->Next ();
      SaveMeshObjectToDom (savedObjects, node, sect, fileInfo);
    }
  }

  struct SaveTexture
  {
    csString filename;
    csString texname;
    csStringArray pdLightmapFiles;
    csStringArray pdLightIDs;
  };

  void Scene::SaveLightmapsToDom (iDocumentNode* r, LoadedFile* fileInfo)
  {
    csArray<SaveTexture> texturesToSave;
    csStringSet pdlightNums;
    for (unsigned int i = 0; i < lightmaps.GetSize (); i++)
    {
      const char* textureFilename = lightmaps[i]->GetFilename ();
      // Save the lightmap
      {
        // Texture file name is relative to world file
        csVfsDirectoryChanger chdir (globalLighter->vfs);
        chdir.ChangeTo (fileInfo->directory);
      #ifndef DUMP_NORMALS
        lightmapPostProc.ApplyAmbient (lightmaps[i]);
        lightmapPostProc.ApplyExposure (lightmaps[i]);
      #endif
        lightmaps[i]->SaveLightmap (textureFilename);
      }
      SaveTexture savetex;
      savetex.filename = textureFilename;
      savetex.texname = lightmaps[i]->GetTextureName();

      PDLightmapsHash::GlobalIterator pdlIt = pdLightmaps.GetIterator();
      while (pdlIt.HasNext())
      {
        csPtrKey<Light> key;
        LightmapPtrDelArray* lm = pdlIt.Next(key);
        if (lm->Get (i)->IsNull()) continue;

        csString lmID (key->GetLightID ().HexString());
        csString textureFilename = "lightmaps/";
        textureFilename += i;
        textureFilename += "_";
        textureFilename += pdlightNums.Request (lmID);
        textureFilename += ".png";

        {
          // Texture file name is relative to world file
          csVfsDirectoryChanger chdir (globalLighter->vfs);
          chdir.ChangeTo (fileInfo->directory);
        #ifndef DUMP_NORMALS
          lightmapPostProc.ApplyExposure (lm->Get (i));
        #endif
          lm->Get (i)->SaveLightmap (textureFilename);
        }
        savetex.pdLightmapFiles.Push (textureFilename);
        savetex.pdLightIDs.Push (lmID);
      }

      texturesToSave.Push (savetex);
    }

    csRef <iDocumentNode> libRoot = r->GetNode ("library");

    // And now save the textures
    // see if we have any <textures> node
    csRef<iDocumentNode> texturesNode = libRoot->GetNode ("textures");
    if (!texturesNode)
    {
      csRef<iDocumentNode> firstNode;
      csRef<iDocumentNodeIterator> it = libRoot->GetNodes ();
      if (it->HasNext ())
        firstNode = it->Next ();

      //Create one
      texturesNode = libRoot->CreateNodeBefore (CS_NODE_ELEMENT, firstNode);
      texturesNode->SetValue ("textures");
    }

    for (unsigned int i = 0; i < texturesToSave.GetSize (); i++)
    {
      const SaveTexture& textureToSave = texturesToSave[i];

      csRef<iDocumentNode> textureNode;
      {
        csRef<iDocumentNodeIterator> textureNodes = 
          texturesNode->GetNodes ("texture");
        while (textureNodes->HasNext())
        {
          csRef<iDocumentNode> texNode = textureNodes->Next();
          if (texNode->GetType() != CS_NODE_ELEMENT) continue;
          const char* texName = texNode->GetAttributeValue ("name");
          if (texName 
            && (strcmp (texName, textureToSave.texname.GetData ()) == 0))
          {
            textureNode = texNode;
            break;
          }
        }
      }
      if (!textureNode.IsValid())
      {
        textureNode = texturesNode->CreateNodeBefore (CS_NODE_ELEMENT);
        textureNode->SetValue ("texture");
      }
      else
      {
        CollectDeleteTextures (textureNode, fileInfo->texFileNamesToDelete);
        textureNode->RemoveNodes ();
      }
      textureNode->SetAttribute ("name", textureToSave.texname.GetData ());
           
      csRef<iDocumentNode> classNode = 
        textureNode->CreateNodeBefore (CS_NODE_ELEMENT);
      classNode->SetValue ("class");
      csRef<iDocumentNode> classContNode = 
        classNode->CreateNodeBefore (CS_NODE_TEXT);
      classContNode->SetValue ("lightmap");

      csRef<iDocumentNode> fileNode = 
        textureNode->CreateNodeBefore (CS_NODE_ELEMENT);
      fileNode->SetValue ("file");

      csRef<iDocumentNode> filenameNode =
        fileNode->CreateNodeBefore (CS_NODE_TEXT);
      filenameNode->SetValue (textureToSave.filename.GetData ());
      fileInfo->texFileNamesToDelete.Delete (textureToSave.filename);

      csRef<iDocumentNode> mipmapNode = 
        textureNode->CreateNodeBefore (CS_NODE_ELEMENT);
      mipmapNode->SetValue ("mipmap");

      csRef<iDocumentNode> mipmapContents =
        mipmapNode->CreateNodeBefore (CS_NODE_TEXT);
      mipmapContents->SetValue ("no");

      if (textureToSave.pdLightmapFiles.GetSize() > 0)
      {
        csRef<iDocumentNode> typeNode = 
          textureNode->CreateNodeBefore (CS_NODE_ELEMENT, fileNode);
        typeNode->SetValue ("type");
        csRef<iDocumentNode> typeContent = 
          typeNode->CreateNodeBefore (CS_NODE_TEXT, 0);
        typeContent->SetValue ("crystalspace.texture.loader.pdlight");

        csRef<iDocumentNode> pdlightParamsNode = 
          textureNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
        pdlightParamsNode->SetValue ("params");

        for (size_t p = 0; p < textureToSave.pdLightmapFiles.GetSize(); p++)
        {
          csRef<iDocumentNode> mapNode = 
            pdlightParamsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
          mapNode->SetValue ("map");
          mapNode->SetAttribute ("lightid", textureToSave.pdLightIDs[p]);

          csRef<iDocumentNode> mapContents = 
            mapNode->CreateNodeBefore (CS_NODE_TEXT, 0);
          mapContents->SetValue (textureToSave.pdLightmapFiles[p]);
          fileInfo->texFileNamesToDelete.Delete (textureToSave.pdLightmapFiles[p]);
        }
      }

      fileInfo->texturesToClean.Delete (textureToSave.texname.GetData ());
    }

    // Clean out old lightmap textures
    {
      csRefArray<iDocumentNode> nodesToDelete;
      csRef<iDocumentNodeIterator> it = texturesNode->GetNodes ("texture");
      while (it->HasNext())
      {
        csRef<iDocumentNode> child = it->Next();
        if (child->GetType() != CS_NODE_ELEMENT) continue;

        const char* name = child->GetAttributeValue ("name");
        if ((name != 0) && fileInfo->texturesToClean.Contains (name))
        {
          CollectDeleteTextures (child, fileInfo->texFileNamesToDelete);
          nodesToDelete.Push (child);
        }
      }
      for (size_t i = 0; i < nodesToDelete.GetSize(); i++)
        texturesNode->RemoveNode (nodesToDelete[i]);
    }

    DocumentHelper::RemoveDuplicateChildren(texturesNode, 
      texturesNode->GetNodes ("texture"),
      DocumentHelper::NodeAttributeCompare("name"));
  }

  //-------------------------------------------------------------------------

  Scene::LightingPostProcessor::LightingPostProcessor (Scene* scene) : scene (scene)
  {
  }

  void Scene::LightingPostProcessor::ApplyExposure (Lightmap* lightmap)
  {
    // 0.5 to account for the fact that the shader does *2
    //lightmap->ApplyExposureFunction (1.8f, 0.5f);
    lightmap->ApplyScaleClampFunction (0.5f, 1.0f);
  }

  void Scene::LightingPostProcessor::ApplyExposure (csColor* colors, size_t numColors)
  {
    // @@@ ATM shader does *not* do *2 for vertex lighting
    //LightmapPostProcess::ApplyExposureFunction(colors, numColors, 1.8f, 1.0f);
    //LightmapPostProcess::ApplyScaleClampFunction (colors, numColors, 1.0f, 1.0f);
  }
  
  void Scene::LightingPostProcessor::ApplyAmbient (Lightmap* lightmap)
  {
    //if (!<indirect lighting enabled>)
    {
      csColor amb;
      globalLighter->engine->GetAmbientLight (amb);
      lightmap->AddAmbientTerm (amb);
    }
  }

  void Scene::LightingPostProcessor::ApplyAmbient (csColor* colors, size_t numColors)
  {
    //if (!<indirect lighting enabled>)
    {
      csColor amb;
      globalLighter->engine->GetAmbientLight (amb);
      LightmapPostProcess::AddAmbientTerm (colors, numColors, amb);
    }
  }

}
