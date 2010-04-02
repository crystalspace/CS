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

#include "common.h"

#include "scene.h"
#include "lighter.h"
#include "kdtree.h"
#include "statistics.h"
#include "object_genmesh.h"
#include "object_terrain2.h"

#include "photonmap.h"
#include "irradiancecache.h"

#include <functional>

using namespace CS;

namespace lighter
{
  Sector::~Sector()
  {
    delete kdTree;
    if (photonMap != NULL)
    {
      delete photonMap;
    }
  }

  void Sector::Initialize (Statistics::Progress& progress)
  {
    progress.SetProgress (0);

    size_t u, updateFreq;
    float progressStep;

    u = updateFreq = progress.GetUpdateFrequency (allObjects.GetSize());
    progressStep = updateFreq * (1.0f / allObjects.GetSize());

    // Initialize objects
    ObjectHash::GlobalIterator objIt = allObjects.GetIterator ();
    while (objIt.HasNext ())
    {
      csRef<Object> obj = objIt.Next ();
      obj->Initialize (this);
      if (--u == 0)
      {
        progress.IncProgress (progressStep);
        u = updateFreq;
      }
    }
    
    scene->PropagateLights (this);

    progress.SetProgress (1);
  }

  void Sector::PrepareLighting (Statistics::Progress& progress)
  {
    progress.SetProgress (0);

    size_t u, updateFreq;
    float progressStep;

    u = updateFreq = progress.GetUpdateFrequency (allObjects.GetSize());
    progressStep = updateFreq * (1.0f / allObjects.GetSize());

    // Prepare lighting of objects
    ObjectHash::GlobalIterator objIt = allObjects.GetIterator ();
    while (objIt.HasNext ())
    {
      csRef<Object> obj = objIt.Next ();
      obj->PrepareLighting ();
      if (--u == 0)
      {
        progress.IncProgress (progressStep);
        u = updateFreq;
      }
    }

    progress.SetProgress (1);
  }

  void Sector::BuildKDTree (Statistics::Progress& progress)
  {
    // Build KD-tree
    ObjectHash::GlobalIterator objIt = allObjects.GetIterator ();
    KDTreeBuilder builder;
    kdTree = builder.BuildTree (objIt, progress);
  }

  void Sector::InitPhotonMap()
  { 
    if(photonMap == NULL)
    {
      photonMap = new PhotonMap(
        2 * globalConfig.GetIndirectProperties ().numPhotons);
    }
  }

  void Sector::AddPhoton(const csColor power, const csVector3 pos,
      const csVector3 dir )
  {
    if(photonMap == NULL)
    {
      photonMap = new PhotonMap(
        2 * globalConfig.GetIndirectProperties ().numPhotons);
    }

    const float fPower[3] = { power.red, power.green, power.blue };
    const float fPos[3] = { pos.x, pos.y, pos.z };
    const float fDir[3] = { dir.x, dir.y, dir.z };

    photonMap->Store(fPower, fPos, fDir);
  }

  void Sector::ScalePhotons(const float scale)
  {
    if(photonMap != NULL) photonMap->ScalePhotonPower(scale);
  }

  size_t Sector::GetPhotonCount()
  {
    if(photonMap != NULL) return photonMap->GetPhotonCount();
    return 0;
  }

  void Sector::BalancePhotons(Statistics::ProgressState& prog)
  {
    if(photonMap != NULL)
    {
      // First, initialize the irradiance cache
      delete irradianceCache;
      irradianceCache = new IrradianceCache(
        photonMap->GetBBoxMin(), photonMap->GetBBoxMax());

      // Balance the photons
      photonMap->Balance(prog);
    }
  }

  bool Sector::SampleIRCache(const csVector3 point, const csVector3 normal,
                    csColor &irrad)
  {
    if(irradianceCache == NULL) return false;

    // Check cache to see if we can reuse old results
    float* result = new float[3];
    float fPos[3] = { point.x, point.y, point.z };
    float fNorm[3] = { normal.x, normal.y, normal.z };
    
    if(irradianceCache->EstimateIrradiance(fPos, fNorm, result))
    {
      irrad.Set(result[0], result[1], result[2]);
      delete [] result;
      return true;
    }

    return false;
  }

  csColor Sector::SamplePhoton(const csVector3 point, const csVector3 normal,
                    const float searchRad)
  {
    if(photonMap == NULL) return csColor(0, 0, 0);

    // Local copy of the global options
    const static size_t densitySamples =
      globalConfig.GetIndirectProperties ().maxDensitySamples;

    // Sample the photon map
    float result[3] = {0, 0, 0};
    float fPos[3] = { point.x, point.y, point.z };
    float fNorm[3] = { normal.x, normal.y, normal.z };
    photonMap->IrradianceEstimate(result, fPos, fNorm, searchRad, densitySamples);

    // Return result as a csColor
    return csColor(result[0], result[1], result[2]);
  }

  void Sector::AddToIRCache(const csVector3 point, const csVector3 normal,
                      const csColor irrad, const float mean)
  {
    float fPow[3] = { irrad.red, irrad.green, irrad.blue };
    float fPos[3] = { point.x, point.y, point.z };
    float fNorm[3] = { normal.x, normal.y, normal.z };

    // Add sample to the irradiance cache for reuse
    if(irradianceCache == NULL)
    {
      irradianceCache = new IrradianceCache(
        photonMap->GetBBoxMin(), photonMap->GetBBoxMax());
    }

    irradianceCache->Store(fPos, fNorm, fPow, mean);
  }

  void Sector::SavePhotonMap(const char* filename)
  {
    if(photonMap != NULL) photonMap->SaveToFile(filename);
  }

  //-------------------------------------------------------------------------

  Scene::Scene () : lightmapPostProc (this)
  {
  }

  Scene::~Scene ()
  {
    Statistics::Progress progDummy (0, 0);
    CleanLightingData (progDummy);
  }
  
  void Scene::CleanUp (Statistics::Progress& progress)
  {
    progress.SetProgress (0);
    
    Statistics::Progress sectorsProgress (0, 1, &progress);
    Statistics::Progress filesProgress (0, 1, &progress);
    
    float progressStep = 1.0f/sectors.GetSize();
    sectorsProgress.SetProgress (0);
    SectorHash::GlobalIterator sectorIt = sectors.GetIterator();
    while (sectorIt.HasNext())
    {
      sectors.DeleteElement (sectorIt);
      sectorsProgress.IncProgress (progressStep);
    }
    sectorsProgress.SetProgress (1);
        
    progressStep = 1.0f/sceneFiles.GetSize();
    filesProgress.SetProgress (0);
    while (sceneFiles.GetSize() > 0)
    {
      sceneFiles.DeleteIndex (sceneFiles.GetSize()-1);
    }
    filesProgress.SetProgress (1);
    
    progress.SetProgress (1);
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

  bool Scene::LoadFiles (Statistics::Progress& progress)
  {
    progress.SetProgress (0);

    Statistics::Progress filesProgress (0, 9, &progress);
    Statistics::Progress allProgress (0, 1, &progress);

    if (sceneFiles.GetSize () == 0) 
      return globalLighter->Report ("No files to load!");

    const float div = 1.0f / sceneFiles.GetSize ();

    for (unsigned int i = 0; i < sceneFiles.GetSize (); i++)
    {
      // Change path
      csStringArray paths;
      paths.Push ("/lev/");
      if (!globalLighter->vfs->ChDirAuto (sceneFiles[i].directory, &paths, 0, "world"))
        return globalLighter->Report ("Error setting directory '%s'!", 
          sceneFiles[i].directory.GetData());

      sceneFiles[i].fileName = "world";
      // Load it
      csRef<iFile> buf = globalLighter->vfs->Open (
        sceneFiles[i].fileName, VFS_FILE_READ);
      if (!buf)
      {
        // Check if the file was part of the path.
        csString directory = sceneFiles[i].directory;
        csString filename = sceneFiles[i].directory;
        size_t start = filename.FindLast('\\')+1;
        directory = directory.Slice(0, start);
        filename = filename.Slice(start);

        if (!globalLighter->vfs->ChDirAuto (directory, &paths, 0, filename))
        {
          return globalLighter->Report ("Error setting directory '%s'!", 
            sceneFiles[i].directory.GetData());
        }

        buf = globalLighter->vfs->Open (filename, VFS_FILE_READ);
        if(buf)
        {
          sceneFiles[i].directory = directory;
          sceneFiles[i].fileName = filename;
        }
        else
        {
          return globalLighter->Report ("Error opening file '%s'!",
            sceneFiles[i].fileName.GetData());
        }
      }
      
      csRef<iDocument> doc = globalLighter->docSystem->CreateDocument ();
      const char* error = doc->Parse (buf, true);
      if (error != 0)
      {
        return globalLighter->Report ("Document system error: %s!", error);
      }

      // Try to extract the "level name" from the given path
      const size_t maxPartLen = sceneFiles[i].directory.Length()+1;
      CS_ALLOC_STACK_ARRAY(char, pathDir, maxPartLen);
      CS_ALLOC_STACK_ARRAY(char, pathFile, maxPartLen);
      csSplitPath (sceneFiles[i].directory, pathDir, maxPartLen,
        pathFile, maxPartLen);
      if ((strlen (pathFile) == 0) || (strcmp (pathFile, "world") == 0))
      {
        // Impractical name, try again one level higher
        csString tmp (pathDir);
        csSplitPath (tmp, pathDir, maxPartLen, pathFile, maxPartLen);
      }
      bool defaultLevelName = true;
      if (strlen (pathFile) == 0)
      {
        // No name, use some random number
        union
        {
          uint16 ui16[sizeof(uint)/sizeof(uint16)];
          uint ui;
        } tag;
        csRandomGen rng;
        for (size_t j = 0; j < sizeof(uint)/sizeof(uint16); j++)
        {
          tag.ui16[j] = rng.Get ((uint16)~0);
        }
        sceneFiles[i].levelName.Format ("%x", tag.ui);
      }
      else
      {
        // Use found file name as level name
        sceneFiles[i].levelName = pathFile;
        // Strip off extension
        size_t dot = sceneFiles[i].levelName.FindLast ('.');
        if (dot != (size_t)-1)
          sceneFiles[i].levelName.Truncate (dot);
        defaultLevelName = false;
      }
      // Read lighter2.cfg in map dir
      {
        csRef<csConfigFile> cfgFile;
        cfgFile.AttachNew (new csConfigFile ("lighter2.cfg", globalLighter->vfs));
        sceneFiles[i].sceneConfig.Initialize (cfgFile);
      }
      // Read <level>.cfg in map dir
      if (!defaultLevelName)
      {
        csString cfgname;
        cfgname.Format ("%s.cfg", sceneFiles[i].levelName.GetData());
        csRef<csConfigFile> cfgFile;
        cfgFile.AttachNew (new csConfigFile (cfgname, globalLighter->vfs));
        sceneFiles[i].sceneConfig.Initialize (cfgFile);
      }

      sceneFiles[i].SetDocument (doc);
      // ChDirAuto() may have created an automount, use that...
      sceneFiles[i].directory = globalLighter->vfs->GetCwd ();

      // Pass it to the loader
      csLoadResult rc;
      rc = globalLighter->loader->Load (sceneFiles[i].GetDocument()->GetRoot(), 0, false,
        sceneFiles[i].sceneConfig.GetLighterProperties().checkDupes);
      if (!rc.success)
        return globalLighter->Report ("Error loading file 'world'!");

      // Parses meshes from engine
      Statistics::Progress* progEngine = filesProgress.CreateProgress (div);
      ParseEngine (&sceneFiles[i], *progEngine);
      delete progEngine;

      filesProgress.SetProgress (i * div);
    }

    ParseEngineAll (allProgress);

    /* We have turned everything needed into our own objects which keep
       refs to the engine objects as needed. Hence instruct engine to
       release everything it holds. */
    globalLighter->engine->DeleteAll();

    progress.SetProgress (1);
    return true;
  }

  bool Scene::SaveWorldFactories (Statistics::Progress& progress)
  {
    progress.SetProgress (0);

    if (sceneFiles.GetSize () == 0)
    {
      globalLighter->Report ("No files to save!");
      return false;
    }

    float fileProgress = 1.0f / sceneFiles.GetSize ();
    for (size_t i = 0; i < sceneFiles.GetSize (); i++)
    {
      //Traverse the DOM, save any factory we encounter
      Statistics::Progress* progFile = progress.CreateProgress (fileProgress);
      SaveSceneFactoriesToDom (&sceneFiles[i], *progFile);
      delete progFile;
    }

    progress.SetProgress (1);
    return true;
  }

  bool Scene::SaveWorldMeshes (Statistics::Progress& progress)
  {
    progress.SetProgress (0);

    if (sceneFiles.GetSize () == 0)
    {
      globalLighter->Report ("No files to save!");
      return false;
    }

    float fileProgress = 1.0f / sceneFiles.GetSize ();
    for (size_t i = 0; i < sceneFiles.GetSize (); i++)
    {
      //Traverse the DOM, save any mesh we encounter
      Statistics::Progress* progFile = progress.CreateProgress (fileProgress);
      SaveSceneMeshesToDom (&sceneFiles[i], *progFile);
      delete progFile;
    }

    progress.SetProgress (1);
    return true;
  }

  bool Scene::FinishWorldSaving (Statistics::Progress& progress)
  {
    progress.SetProgress (0);

    if (sceneFiles.GetSize () == 0)
    {
      globalLighter->Report ("No files to save!");
      return false;
    }

    float fileProgress = 1.0f / sceneFiles.GetSize ();
    for (size_t i = 0; i < sceneFiles.GetSize (); i++)
    {
      if (!sceneFiles[i].IsChanged()) continue;
        
      //Change path
      if (!globalLighter->vfs->ChDir (sceneFiles[i].directory))
      {
        globalLighter->Report ("Error setting directory '%s'!", 
          sceneFiles[i].directory.GetData());
        return false;
      }

      // Save it
      csString tempfile;
      tempfile.Format (".lighter2.%s.world", 
        sceneFiles[i].levelName.GetData());
      csRef<iFile> buf = globalLighter->vfs->Open (tempfile, VFS_FILE_WRITE);
      if (!buf) 
      {
        globalLighter->Report (
          "Error opening file '%s' for writing!", tempfile.GetData());
        return false;
      }

      const char *err = sceneFiles[i].GetDocument()->Write (buf);
      if (err)
      {
        globalLighter->Report ("Error writing file '%s': %s", 
          tempfile.GetData(), err);
        return false;
      }

      // Release document, not needed any more.
      sceneFiles[i].SetDocument (0);

      progress.IncProgress (fileProgress);
    }

    // Ensure no archives are cached in memory (for memory consumption reasons)
    if (!globalLighter->vfs->Sync())
    {
      globalLighter->Report ("Could not synchronize VFS.");
      return false;
    }

    progress.SetProgress (1);
    return true;
  }

  bool Scene::ApplyWorldChanges (Statistics::Progress& progress)
  {
    progress.SetProgress (0);

    if (sceneFiles.GetSize () == 0)
    {
      globalLighter->Report ("No files to save!");
      return false;
    }

    float fileProgress = 1.0f / sceneFiles.GetSize ();
    for (size_t i = 0; i < sceneFiles.GetSize (); i++)
    {
      Statistics::Progress* progFile = progress.CreateProgress (fileProgress);
      if (!sceneFiles[i].IsChanged()) 
      {
        progFile->SetProgress (1);
        delete progFile;
        continue;
      }

      //Change path
      if (!globalLighter->vfs->ChDir (sceneFiles[i].directory))
      {
        globalLighter->Report ("Error setting directory '%s'!", 
          sceneFiles[i].directory.GetData());
        return false;
      }

      // Read temp file...
      csString tempfile;
      tempfile.Format (".lighter2.%s.world", 
        sceneFiles[i].levelName.GetData());
      csRef<iDataBuffer> buf = globalLighter->vfs->ReadFile (tempfile, false);
      if (!buf) 
      {
        globalLighter->Report ("Error reading file '%s'!", tempfile.GetData());
        return false;
      }

      buf = SaveDebugData (sceneFiles[i], buf, *progFile);

      // ... write to 'world' ...
      if (!globalLighter->vfs->WriteFile (sceneFiles[i].fileName, buf->GetData(), 
        buf->GetSize()))
      {
        globalLighter->Report ("Error writing to '%s'!",
          sceneFiles[i].fileName.GetData());
        return false;
      }

      // ... and remove the backup.
      buf.Invalidate();
      if (!globalLighter->vfs->DeleteFile (tempfile))
      {
        globalLighter->Report ("Error deleting file '%s'!", 
          tempfile.GetData());
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

      progFile->SetProgress (1);
      delete progFile;
    }

    progress.SetProgress (1);
    return true;
  }
    
  bool Scene::GenerateSpecularDirectionMaps (Statistics::Progress& progress)
  {
    if (globalConfig.GetLighterProperties().specularDirectionMaps)
    {
      progress.SetProgress (0);
      float fileProgress = 1.0f / sceneFiles.GetSize ();
      for (size_t i = 0; i < sceneFiles.GetSize (); i++)
      {
        Statistics::Progress* progFile = progress.CreateProgress (fileProgress);
        progFile->SetProgress (0);
        GenerateSpecularDirectionMaps (&(sceneFiles[i]), *progFile);
        progFile->SetProgress (1);
        delete progFile;
  
	progress.SetProgress (i * fileProgress);
      }
  
      progress.SetProgress (1);
    }
    return true;
  }

  static const char lightmapLibraryName[] = "lightmaps.cslib";

  bool Scene::SaveLightmaps (Statistics::Progress& progress)
  {
    progress.SetProgress (0);

    if (sceneFiles.GetSize () == 0)
    {
      globalLighter->Report ("No files to save!");
      return false;
    }

    float fileProgress = 1.0f / sceneFiles.GetSize ();
    for (size_t i = 0; i < sceneFiles.GetSize (); i++)
    {
      if (!sceneFiles[i].IsChanged()) continue;

      //Change path
      if (!globalLighter->vfs->ChDir (sceneFiles[i].directory))
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
        doc = EnsureChangeable (doc);
      }
      else
      {
        // ... create empty otherwise
        csRef<iDocumentNode> root = doc->CreateRoot ();
        csRef<iDocumentNode> libNode = root->CreateNodeBefore (
          CS_NODE_ELEMENT);
        libNode->SetValue ("library");
      }
      Statistics::Progress* progLightmaps = progress.CreateProgress (
        fileProgress * 0.95f);
      SaveLightmapsToDom (doc->GetRoot (), &sceneFiles[i], *progLightmaps);
      delete progLightmaps;

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

      progress.SetProgress (i * fileProgress);
    }

    progress.SetProgress (1);

    return true;
  }

  bool Scene::SaveMeshesPostLighting (Statistics::Progress& progress)
  {
    progress.SetProgress (0);

    if (sceneFiles.GetSize () == 0)
    {
      globalLighter->Report ("No files to save!");
      return false;
    }

    float fileProgress = 1.0f / sceneFiles.GetSize ();
    for (size_t i = 0; i < sceneFiles.GetSize (); i++)
    {
      //Change path
      if (!globalLighter->vfs->ChDir (sceneFiles[i].directory))
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

      size_t updateFreq, u;
      float progressStep;

      Statistics::Progress* progMesh = progress.CreateProgress (fileProgress);

      u = updateFreq = progMesh->GetUpdateFrequency (meshObjNum);
      progressStep = updateFreq * (1.0f / meshObjNum);

      progMesh->SetProgress (0);
      for (size_t o = 0; o < meshObjNum; o++)
      {
        Object* obj = sceneFiles[i].fileObjects[o];
        obj->SaveMeshPostLighting (this);
        if (--u == 0)
        {
          progMesh->IncProgress (progressStep);
          u = updateFreq;
        }
      }
      progMesh->SetProgress (1);
      delete progMesh;
    }

    progress.SetProgress (1);

    return true;
  }

  void Scene::CleanLightingData (Statistics::Progress& progress)
  {
    static const int cleanupSteps = 7;
    float progressStep = 1.0f/cleanupSteps;
  
    progress.SetProgress (0);
    PDLightmapsHash::GlobalIterator pdlIt = pdLightmaps.GetIterator();
    while (pdlIt.HasNext())
    {
      LightmapPtrDelArray* lm = pdlIt.Next();
      delete lm;
    }
    pdLightmaps.DeleteAll ();
    progress.SetProgress (1*progressStep);

    lightmaps.DeleteAll ();
    progress.SetProgress (2*progressStep);
    radFactories.DeleteAll ();
    progress.SetProgress (3*progressStep);
    radMaterials.DeleteAll ();
    progress.SetProgress (4*progressStep);
    originalSectorHash.DeleteAll ();
    progress.SetProgress (5*progressStep);
    directionMaps[0].DeleteAll ();
    progress.SetProgress (6*progressStep);
    directionMaps[1].DeleteAll ();
    
    progress.SetProgress (1);
  }

  Lightmap* Scene::GetLightmap (uint lightmapID, size_t subLightmapNum, 
                                Light* light)
  {
    size_t realLmNum = lightmapID;
    if (globalConfig.GetLighterProperties().directionalLMs)
      realLmNum += (lightmaps.GetSize()/4) * subLightmapNum;
    if (!light || !light->IsPDLight ())
      return lightmaps[realLmNum];

    light = light->GetOriginalLight();
    LightmapPtrDelArray* pdLights = pdLightmaps.Get (light, 0);
    if (pdLights == 0)
    {
      pdLights = new LightmapPtrDelArray;
      pdLights->SetSize (lightmaps.GetSize ());
 
      pdLightmaps.Put (light, pdLights);
    }

    Lightmap* lm = pdLights->Get (realLmNum);
    if (!lm)
    {
      lm = new Lightmap (lightmaps[realLmNum]->GetWidth(), 
        lightmaps[realLmNum]->GetHeight());
      lm->Grow (lightmaps[realLmNum]->GetWidth(), 
        lightmaps[realLmNum]->GetHeight());
      pdLights->Get (realLmNum) = lm;
    }
    return lm;
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
  
  iTextureWrapper* Scene::GetSpecDirectionMapTexture (uint ID, int subNum)
  {
    DirectionMapTextures& tex = directionMapTextures.GetExtend (ID);
    if (tex.t[subNum] == 0)
    {
      tex.t[subNum] = globalLighter->engine->CreateBlackTexture (
        Lightmap::GetTextureNameFromFilename (
          GetDirectionMapFilename (ID, subNum)),
        1, 1, 0, CS_TEXTURE_3D);
    }
    return tex.t[subNum];
  }

  bool Scene::ParseEngine (LoadedFile* fileInfo, Statistics::Progress& progress)
  {
    progress.SetProgress (0);

    // Parse sectors

    // Map mesh objects loaded from each scene file to it
    iSectorList *sectorList = globalLighter->engine->GetSectors ();
    csRef<iDocumentNode> worldNode = 
      fileInfo->GetDocument()->GetRoot()->GetNode ("world");

    globalStats.scene.numSectors = sectorList->GetCount ();

    if (worldNode.IsValid ())
    {
      csRef<iDocumentNodeIterator> sectorNodeIt = 
        worldNode->GetNodes ("sector");
      float progressStep = 1.0f / sectorNodeIt->GetEndPosition ();
      while (sectorNodeIt->HasNext())
      {
        progress.SetProgress (
          progressStep * sectorNodeIt->GetNextPosition ());

        csRef<iDocumentNode> sectorNode = sectorNodeIt->Next ();
        const char* sectorName = sectorNode->GetAttributeValue ("name");
        if (!sectorName) continue;

        csRef<iSector> engSector = sectorList->FindByName (sectorName);
        if (!engSector.IsValid ()) continue;

        Statistics::Progress* progSector = 
          progress.CreateProgress (progressStep);
        ParseSector (fileInfo, engSector, *progSector);
        delete progSector;

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
          fileInfo->fileObjects.Push (obj);
        }
      }
    }
    else
    {
      // Not a world file, so just parse meshfacts.
      iMeshFactoryList* mflist = globalLighter->engine->GetMeshFactories();
      for(int m=0; m<mflist->GetCount(); ++m)
      {
        csRef<ObjectFactory> factory;
        ParseMeshFactory(fileInfo, mflist->Get(m), factory);
      }
    }

    progress.SetProgress (1);
    return true;
  }

  bool Scene::ParseEngineAll (Statistics::Progress& progress)
  {
    progress.SetProgress (0);

    Statistics::Progress portalProgress (0, 1, &progress);

    // Parse materials
    iMaterialList* matList = globalLighter->engine->GetMaterialList ();
    for (int i = 0; i < matList->GetCount (); i++)
    {
      ParseMaterial (matList->Get (i));
    }

    // Parse portals
    portalProgress.SetProgress (0);
    SectorOrigSectorHash::GlobalIterator sectIt = originalSectorHash.GetIterator ();
    while (sectIt.HasNext ())
    {
      Sector* sector;
      csPtrKey<iSector> srcSector;

      sector = sectIt.Next (srcSector);
      ParsePortals (srcSector, sector);
    }
    portalProgress.SetProgress (1);

    progress.SetProgress (1);
    return true;
  }

  void Scene::ParseSector (LoadedFile* fileInfo,iSector *sector, 
                           Statistics::Progress& progress)
  {
    if (!sector) 
    {
      progress.SetProgress (1);
      return;
    }

    progress.SetProgress (0);

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

    size_t u, updateFreq;
    float progressStep;

    // Parse all meshes (should have selector later!)
    iMeshList *meshList = sector->GetMeshes ();
    globalStats.scene.numObjects += meshList->GetCount ();

    u = updateFreq = progress.GetUpdateFrequency (meshList->GetCount ());
    progressStep = updateFreq * (1.0f / meshList->GetCount ());

    for (int i = meshList->GetCount (); i-- > 0;)
    {
      iMeshWrapper* mesh = meshList->Get (i);
      csRef<Object> obj;
      bool isPortal = mesh->GetPortalContainer() != 0;
      if (ParseMesh (fileInfo, radSector, mesh, obj) == mpFailure)
      {
        if (!isPortal)
          globalLighter->Report ("Error parsing mesh '%s' in sector '%s'!", 
            mesh->QueryObject()->GetName(), radSector->sectorName.GetData ());
      }
      // Mesh is parsed, release resources
      if (!isPortal) meshList->Remove (i);

      if (--u == 0)
      {
        progress.IncProgress (progressStep);
        u = updateFreq;
      }
    }

    csSet<csString> lightNames;
    // Parse all lights (should have selector later!)
    iLightList *lightList = sector->GetLights ();
    globalStats.scene.numLights = lightList->GetCount ();

    for (int i = lightList->GetCount (); i-- > 0;)
    {
      iLight *light = lightList->Get (i);
      if (light->GetDynamicType() == CS_LIGHT_DYNAMICTYPE_DYNAMIC) 
      {
        lightList->Remove (i);
        continue;
      }

      const char* lightName = light->QueryObject()->GetName ();
      if (lightNames.Contains (lightName))
      {
        globalLighter->Report (
          "A light named '%s' already exists in sector '%s'",
          lightName, sectorName);
        lightList->Remove (i);
        continue;
      }
      lightNames.AddNoTest (lightName);

      bool isPD = light->GetDynamicType() == CS_LIGHT_DYNAMICTYPE_PSEUDO;

      // Force to realistic attenuation if requested
      if(globalConfig.GetLighterProperties ().forceRealistic)
      {
        light->SetAttenuationMode( CS_ATTN_REALISTIC );
      }

      // Scale light power to help avoid dark scenes when forcing realistic att
      if(globalConfig.GetLighterProperties ().lightPowerScale != 1.0)
      {
        light->SetColor( light->GetColor()*
          globalConfig.GetLighterProperties ().lightPowerScale);
      }

      // IneQuation was here
      csRef<Light> intLight;

      switch (light->GetType ()) {
        // directional light
        case CS_LIGHT_DIRECTIONAL:
          {
            csRef<DirectionalLight> dirLight;
            dirLight.AttachNew (new DirectionalLight (radSector));

            dirLight->SetRadius (light->GetDirectionalCutoffRadius ());
            dirLight->SetLength (light->GetCutoffDistance ());

            // light's Z axis
            dirLight->SetDirection (light->GetMovable ()->GetFullTransform ().GetO2T ().Row3 ());

            intLight = dirLight;
          }
          break;

        // spotlight
        case CS_LIGHT_SPOTLIGHT:
          {
            csRef<SpotLight> spotLight;
            spotLight.AttachNew (new SpotLight (radSector));

            spotLight->SetRadius (light->GetCutoffDistance ());

            float in, out;
            light->GetSpotLightFalloff (in, out);
            spotLight->SetFalloff (in, out);

            // light's Z axis
            spotLight->SetDirection (light->GetMovable ()->GetFullTransform ().GetO2T ().Row3 ());

            intLight = spotLight;
          }
          break;

        // by default assume point light
        case CS_LIGHT_POINTLIGHT:
        default:
          {
            csRef<PointLight> pointLight;
            pointLight.AttachNew (new PointLight (radSector));

            pointLight->SetRadius (light->GetCutoffDistance ());

            intLight = pointLight;
          }
          break;
      }

      intLight->SetPosition (light->GetMovable ()->GetFullPosition ());
      intLight->SetColor (isPD ? csColor (1.0f) : light->GetColor ());

      intLight->SetAttenuation (light->GetAttenuationMode (),
        light->GetAttenuationConstants ());

      intLight->SetPDLight (isPD);
      intLight->SetLightID (light->GetLightID());
      intLight->SetName (lightName);

      if (isPD)
        radSector->allPDLights.Push (intLight);
      else
        radSector->allNonPDLights.Push (intLight);

      lightList->Remove (i);
    }

    progress.SetProgress (1);
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

  void Scene::PropagateLights (Sector* sector)
  {
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
  
  void Scene::PropagateLight (Light* light, const csFrustum& lightFrustum, 
                              PropageState& state)
  {
    //Propagate light through all portals in its current sector, setup proxy-lights in targets
    Sector* sourceSector = light->GetSector ();
    const csVector3& lightCenter = light->GetPosition ();

    PortalRefArray::Iterator it = sourceSector->allPortals.GetIterator ();
    while (it.HasNext ())
    {
      Portal* portal = it.Next ();
      if (state.seenPortals.Contains (portal)) continue;
      state.seenPortals.AddNoTest (portal);
      
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

          PropagateLight (proxyLight, proxyLight->GetFrustum (), state);
        }
      }
    }
  }

  static bool IsDebugMesh (const char* name)
  {
    const char debugPrefix[] = "__lighter2_debug__";

    return strncmp (name, debugPrefix, sizeof(debugPrefix) - 1) == 0;
  }
  
  Scene::MeshParseResult Scene::ParseMesh (LoadedFile* fileInfo,
                                           Sector *sector, 
                                           iMeshWrapper *mesh,
                                           csRef<Object>& obj)
  {
    if (!sector || !mesh) return mpFailure;

    const char* meshName = mesh->QueryObject()->GetName();
    // Ignore meshes created as debugging aids
    if (IsDebugMesh (meshName))
      return mpSuccess;

    if (sector->allObjects.Contains (meshName))
    {
      globalLighter->Report (
        "A mesh named '%s' already exists in sector '%s'",
        meshName, sector->sectorName.GetData());
      return mpSuccess;
    }

    // Get the factory
    csRef<ObjectFactory> factory;
    MeshParseResult parseFact = ParseMeshFactory (fileInfo, mesh->GetFactory (), 
      factory);
    if (parseFact != mpSuccess) return parseFact;
    if (!factory) return mpFailure;

    // Construct a new mesh
    obj = factory->CreateObject ();
    if (!obj) return mpFailure;

    obj->ParseMesh (mesh);
    obj->StripLightmaps (fileInfo->texturesToClean);

    // Save it
    sector->allObjects.Put (obj->meshName, obj);

    return mpSuccess;
  }

  Scene::MeshParseResult Scene::ParseMeshFactory (LoadedFile* fileInfo, 
                                                  iMeshFactoryWrapper *factory,
                                                  csRef<ObjectFactory>& radFact)
  {
    if (!factory) return mpFailure;

    // Check for duplicate
    csString factName = factory->QueryObject ()->GetName ();
    // Ignore meshes created as debugging aids
    if (IsDebugMesh (factName))
      return mpSuccess;

    radFact = radFactories.Get (factName, (ObjectFactory*)0);
    if (radFact) return mpSuccess;

    csRef<iFactory> ifact = scfQueryInterface<iFactory> (
      factory->GetMeshObjectFactory ()->GetMeshObjectType());

    const char* type = ifact->QueryClassID ();

    if (!strcasecmp (type, "crystalspace.mesh.object.genmesh"))
    {
      // Genmesh
      radFact.AttachNew (new ObjectFactory_Genmesh (fileInfo->sceneConfig));
    }
    else if (!strcasecmp (type, "crystalspace.mesh.object.terrain2"))
    {
      // Terrain2
      radFact.AttachNew (new ObjectFactory_Terrain2 (fileInfo->sceneConfig));
    }
    else
      return mpNotAGenMesh;
    radFact->ParseFactory (factory);
    if (!IsObjectFromBaseDir (factory->QueryObject(), fileInfo->directory))
    {
      radFact->noModify = true;
    }
    radFactories.Put (radFact->factoryName, radFact);

    return mpSuccess;
  }
    
  bool Scene::ParseMaterial (iMaterialWrapper* material)
  {
    RadMaterial radMat;
    
    // No material properties from key-value-pairs yet
#if 0
    csRef<iObjectIterator> objiter = 
      material->QueryObject ()->GetIterator();
    while (objiter->HasNext())
    {
      iObject* obj = objiter->Next();
      csRef<iKeyValuePair> kvp = 
        scfQueryInterface<iKeyValuePair> (obj);
      if (kvp.IsValid() && (strcmp (kvp->GetKey(), "lighter2") == 0))
      {
      }
    }
#endif
    
    csRef<iShaderVariableContext> matSVC = 
      scfQueryInterface<iShaderVariableContext> (material->GetMaterial());
    csRef<csShaderVariable> svTex =
      matSVC->GetVariable (globalLighter->svStrings->Request ("tex diffuse"));
    if (svTex.IsValid())
    {
      iTextureWrapper* texwrap = 0;
      svTex->GetValue (texwrap);
      if (texwrap != 0)
      {
        iImage* teximg = texwrap->GetImageFile ();
        if (teximg != 0)
        {
          if (teximg->GetFormat() & CS_IMGFMT_ALPHA)
            radMat.ComputeFilterImage (teximg);
        }
      }
    }
    
    radMaterials.Put (material->QueryObject()->GetName(), radMat);
    return true;
  }

  iDocument* Scene::LoadedFile::GetDocumentChangeable ()
  {
    if (!document.IsValid()) return 0;
    if (docChangeable)
      return document;

    SetDocument (Scene::EnsureChangeable (document));
    CS_ASSERT(docChangeable);
    return document;
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

  void Scene::BuildLightmapTextureList (LoadedFile* fileInfo,
                                        csStringArray& texturesToSave)
  {
    size_t realNumLMs = lightmaps.GetSize();
    if (globalConfig.GetLighterProperties().directionalLMs)
      realNumLMs /= 4;
    for (size_t i = 0; i < lightmaps.GetSize (); i++)
    {
      csString textureFilename;
      size_t subNum = i / realNumLMs;
      if (subNum == 0)
        textureFilename.Format ("lightmaps/%s_%zu.png",
          fileInfo->levelName.GetData(), i);
      else
        textureFilename.Format ("lightmaps/%s_%zu_d%zu.png",
          fileInfo->levelName.GetData(), i % realNumLMs, subNum);
      lightmaps[i]->SetFilename (textureFilename);

      texturesToSave.Push (lightmaps[i]->GetTextureName());
    
      if (globalConfig.GetLighterProperties().specularDirectionMaps)
      {
	textureFilename.Format ("lightmaps/%s_%zu_sd%%d",
	  fileInfo->levelName.GetData(), i);
	directionMapBaseNames.Push (textureFilename);
	for (int x = 0; x < specDirectionMapCount; x++)
	  texturesToSave.Push (GetDirectionMapFilename (i, x));
      }
    }
  }
  
  csString Scene::GetDirectionMapFilename (uint ID, int subNum) const
  {
    return csString().Format (directionMapBaseNames[ID], subNum)
      .Append (".dds");
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

  void Scene::SaveSceneFactoriesToDom (LoadedFile* fileInfo,
                                       Statistics::Progress& progress)
  {
    progress.SetProgress (0);

    csSet<csString> savedFactories;
    
    csRef<iDocumentNode> rootNode = 
      fileInfo->GetDocumentChangeable()->GetRoot();
    csRef <iDocumentNode> worldRoot = rootNode->GetNode ("world");
    if (!worldRoot)
    {
      worldRoot = rootNode->GetNode ("library");
    }
    if (!worldRoot) return;

    csRef<iDocumentNodeIterator> it = worldRoot->GetNodes ();
    float nodeStep = 1.0f / it->GetEndPosition ();

    while (it->HasNext ())
    {
      csRef<iDocumentNode> node = it->Next ();
      size_t nextPos = it->GetNextPosition ();
      if (node->GetType () == CS_NODE_ELEMENT)
      {
        const char* nodeName = node->GetValue ();

        if (!strcasecmp (nodeName, "library"))
        {
          const char* libFile = node->GetAttributeValue ("file");
          if (!libFile) libFile = node->GetContentsValue();
          if ((libFile != 0) && (strcmp (libFile, lightmapLibraryName) != 0))
          {
            Statistics::Progress* progLibrary = 
              progress.CreateProgress (nodeStep);
            HandleLibraryNode (savedFactories, node, fileInfo, *progLibrary,
              false);
            delete progLibrary;
          }
        }
        else if (!strcasecmp (nodeName, "meshfact"))
        {
          SaveResult saveRes = SaveMeshFactoryToDom (savedFactories, node, 
            fileInfo);
          if (saveRes == svRemoveItem)
            worldRoot->RemoveNode (node);
        }
      }
      progress.SetProgress (nextPos * nodeStep);
    }

    progress.SetProgress (1);
  }

  void Scene::SaveSceneMeshesToDom (LoadedFile* fileInfo,
                                    Statistics::Progress& progress)
  {
    Statistics::Progress sectorProgress (0, 90, &progress);
    Statistics::Progress texturesProgress (0, 9, &progress);
    Statistics::Progress lightmapsProgress (0, 1, &progress);

    progress.SetProgress (0);

    bool hasLightmapsLibrary = false;
    csStringArray texturesToSave;
    
    BuildLightmapTextureList (fileInfo, texturesToSave);

    csRef <iDocumentNode> worldRoot = 
      fileInfo->GetDocumentChangeable()->GetRoot()->GetNode ("world");
    if (!worldRoot.IsValid ()) return;

    sectorProgress.SetProgress (0);
    csRef<iDocumentNodeIterator> it = worldRoot->GetNodes ();
    float nodeStep = 1.0f / it->GetEndPosition ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> node = it->Next ();
      size_t nextPos = it->GetNextPosition();
      if (node->GetType () == CS_NODE_ELEMENT) 
      {
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
          Statistics::Progress* progSector = sectorProgress.CreateProgress (
            nodeStep);
          SaveSectorToDom (node, fileInfo, *progSector);
          delete progSector;
        }
      }
      sectorProgress.SetProgress (nextPos * nodeStep);
    }
    sectorProgress.SetProgress (1);

    texturesProgress.SetProgress (0);
    // see if we have any <textures> node
    csRef<iDocumentNode> texturesNode = worldRoot->GetNode ("textures");
    if (texturesNode)
    {
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
            texturesNode->RemoveNode (child);
          }
        }

        CleanOldLightmaps (fileInfo);
      }
    }
    texturesProgress.SetProgress (1);

    // Generate a <library> node for the external lightmaps library
    if (!hasLightmapsLibrary)
    {
      lightmapsProgress.SetProgress (0);
      const char* const createNodeBeforeNames[] = { "textures", "materials", "library", "meshfact", "sector" };

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

      lightmapsProgress.SetProgress (1);
    }

    progress.SetProgress (1);
  }

  bool Scene::SaveSceneLibrary (csSet<csString>& savedFactories, 
                                const char* libFile, LoadedFile* fileInfo,
                                Statistics::Progress& progress, bool noModify)
  {
    csRef<iFile> buf = globalLighter->vfs->Open (libFile, VFS_FILE_READ);
    if (!buf) 
    {
      globalLighter->Report ("Error opening file '%s'!", libFile);
      progress.SetProgress (1);
      return false;
    }

    csRef<iDocument> doc = globalLighter->docSystem->CreateDocument ();
    const char* error = doc->Parse (buf, true);
    if (error != 0)
    {
      globalLighter->Report ("Document system error: %s!", error);
      progress.SetProgress (1);
      return false;
    }

    csRef<iDocumentNode> docRoot = doc->GetRoot();

    csRef<iDocumentNode> libRoot = docRoot->GetNode ("library");
    if (!libRoot)
    {
      globalLighter->Report ("'%s' is not a library", libFile);
      progress.SetProgress (1);
      return false;
    }

    progress.SetProgress (0);
    csRef<iDocumentNodeIterator> it = libRoot->GetNodes ();
    float nodeStep = 1.0f / it->GetEndPosition ();

    while (it->HasNext ())
    {
      csRef<iDocumentNode> node = it->Next ();
      size_t nextPos = it->GetNextPosition ();
      if (node->GetType () == CS_NODE_ELEMENT) 
      {
        const char* nodeName = node->GetValue ();

        if (!strcasecmp (nodeName, "library"))
        {
          Statistics::Progress* progLibrary = 
            progress.CreateProgress (nodeStep);
          HandleLibraryNode (savedFactories, node, fileInfo, *progLibrary,
            noModify);
          delete progLibrary;
        }
        else if (!strcasecmp (nodeName, "meshfact"))
        {
          SaveResult saveRes = SaveMeshFactoryToDom (savedFactories, node, 
            fileInfo);
          if (saveRes == svRemoveItem)
            libRoot->RemoveNode (node);
        }
      }
      progress.SetProgress (nextPos * nodeStep);
    }

    if (!noModify)
    {
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
    }
    progress.SetProgress (1);

    return true;
  }

  void Scene::HandleLibraryNode (csSet<csString>& savedFactories, 
                                 iDocumentNode* node, LoadedFile* fileInfo,
                                 Statistics::Progress& progress, bool noModify)
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
      SaveSceneLibrary (savedFactories, file, fileInfo, progress,
        noModify || !(IsFilenameFromBaseDir (file, fileInfo->directory)));
    }
    else
    {
      file = node->GetContentsValue ();
      SaveSceneLibrary (savedFactories, file, fileInfo, progress,
        noModify || !(IsFilenameFromBaseDir (file, fileInfo->directory)));
    }
  }

  Scene::SaveResult Scene::SaveMeshFactoryToDom (csSet<csString>& savedObjects, 
                                                 iDocumentNode* factNode, 
                                                 LoadedFile* fileInfo)
  {
    // Save a single factory to the dom
    csString name = factNode->GetAttributeValue ("name");
    if (savedObjects.Contains (name))
    {
      globalLighter->Report (
        "A factory named '%s' already exists",
        name.GetData());
      return svFailure;
    }

    // Clean out debugging aid meshes (will be saved later)
    if (IsDebugMesh (name))
    {
      fileInfo->SetChanged (true);
      return svRemoveItem;
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
      SaveResult saveRes = SaveMeshFactoryToDom (savedObjects, node, fileInfo);
      if (saveRes == svRemoveItem)
      {
        factNode->RemoveNode (node);
      }
    }

    fileInfo->SetChanged (true);
    return svSuccess;
  }

  void Scene::SaveSectorToDom (iDocumentNode* sectorNode, LoadedFile* fileInfo,
                               Statistics::Progress& progress)
  {
    progress.SetProgress (0);
    csString name = sectorNode->GetAttributeValue ("name");
    csRef<Sector> sector = sectors.Get (name, (Sector*)0);
    if (sector)
    {
      //ok, have the sector, try to save all objects
      csSet<csString> savedObjects;
      csRef<iDocumentNodeIterator> it = sectorNode->GetNodes ("meshobj");
      float nodeStep = 1.0f / it->GetEndPosition ();
      while (it->HasNext ())
      {
        csRef<iDocumentNode> node = it->Next ();
        size_t nextPos = it->GetNextPosition ();
        SaveResult saveRes = SaveMeshObjectToDom (savedObjects, node, sector,
          fileInfo);
        if (saveRes == svRemoveItem)
          sectorNode->RemoveNode (node);
        progress.SetProgress (nextPos * nodeStep);
      }
    }
    progress.SetProgress (1);
  }

  Scene::SaveResult Scene::SaveMeshObjectToDom (csSet<csString>& savedObjects, 
                                                iDocumentNode *objNode, 
                                                Sector* sect, 
                                                LoadedFile* fileInfo)
  {
    // Save the mesh
    csString name = objNode->GetAttributeValue ("name");
    if (IsDebugMesh (name))
    {
      fileInfo->SetChanged (true);
      return svRemoveItem;
    }
    if (savedObjects.Contains (name))
      /* Already emitted an "ignoring duplicate mesh" earlier, so just skip 
         saving here. */
      return svSuccess;
    csRef<Object> radObj = sect->allObjects.Get (name, (Object*)0);
    if (radObj)
    {
      csRef<iMeshWrapper> meshwrap = radObj->GetMeshWrapper();
      // We do have one
      CS_ASSERT(radObj->GetSector() == sect);
      radObj->SaveMesh (objNode);
      radObj->FreeNotNeededForLighting ();
      savedObjects.AddNoTest (name);

      // Remove any old svs...
      objNode->RemoveNodes (DocSystem::FilterDocumentNodeIterator (
        objNode->GetNodes ("shadervar"),
        DocSystem::NodeAttributeRegexpTest ("name", "tex lightmap.*")));
        
      // ...and add current one
      iShaderVariableContext* meshSVs = meshwrap->GetSVContext ();
      CS::ShaderVarName lightmapName (globalLighter->svStrings, "tex lightmap");
      csRef<csShaderVariable> lightmapSV = meshSVs->GetVariable (lightmapName);
      if (lightmapSV.IsValid())
      {
        csRef<iDocumentNode> shadervarNode = objNode->CreateNodeBefore (
          CS_NODE_ELEMENT, 0);
	shadervarNode->SetValue ("shadervar");
	csRef<iSyntaxService> synsrv = 
	  csQueryRegistry<iSyntaxService> (globalLighter->objectRegistry);
	synsrv->WriteShaderVar (shadervarNode, *lightmapSV);
      }
    }

    // Check if we have any child factories
    csRef<iDocumentNodeIterator> it = objNode->GetNodes ("meshobj");
    while (it->HasNext ())
    {
      csRef<iDocumentNode> node = it->Next ();
      SaveResult saveRes = SaveMeshObjectToDom (savedObjects, node, sect, 
        fileInfo);
      if (saveRes == svRemoveItem)
        objNode->RemoveNode (node);
    }

    fileInfo->SetChanged (true);
    return svSuccess;
  }
  
  struct InfluencePair
  {
    Light* light;
    const LightInfluences* infl;
    float totalIntensity;
    int subMap;
    
    InfluencePair (Light* light, const LightInfluences* infl)
     : light (light), infl (infl), subMap (-1)
    {
      ScopedSwapLock<LightInfluences> l (*infl);
      totalIntensity = infl->GetTotalIntensity();
    }
    
    struct SortByIntensity :
      public std::binary_function<InfluencePair, InfluencePair, bool>
    {
      bool operator() (const InfluencePair& a, const InfluencePair& b) const
      {
        return a.totalIntensity > b.totalIntensity;
      }
    };
  };
  
  void Scene::GenerateSpecularDirectionMaps (LoadedFile* fileInfo,
                                             Statistics::Progress& progress)
  {
    Statistics::Progress objProgress (0, 90, &progress);
    Statistics::Progress normProgress (0, 1, &progress);
    
    progress.SetProgress (0);
    
    // Create 'final' direction maps
    size_t realNumLMs = lightmaps.GetSize ();
    if (globalConfig.GetLighterProperties().directionalLMs)
      realNumLMs /= 4;
    for (size_t m = 0; m < realNumLMs; m++)
    {
      directionMaps[0].Push (new DirectionMap (*(lightmaps[m])));
      directionMaps[1].Push (new DirectionMap (*(lightmaps[m])));
    }
    
    objProgress.SetProgress (0);
    float progressStep = 1.0f/fileInfo->fileObjects.GetSize();
    for (size_t o = 0; o < fileInfo->fileObjects.GetSize(); o++)
    {
      Statistics::Progress* primsProgress =
        objProgress.CreateProgress (progressStep);
    
      Object* obj = fileInfo->fileObjects[o];
    
      size_t u, updateFreq;
      float primProgressStep;
  
      u = updateFreq = primsProgress->GetUpdateFrequency (
        obj->GetPrimitiveGroupNum());
      primProgressStep =
        updateFreq * (1.0f / obj->GetPrimitiveGroupNum());
    
      for (size_t pg = 0; pg < obj->GetPrimitiveGroupNum(); pg++)
      {
        // get all influences for primgroup
        csArray<Light*> influencingLights (obj->GetLightsAffectingGroup (pg));
        csArray<InfluencePair> allInfluences;
        allInfluences.SetCapacity (influencingLights.GetSize());
        for (size_t l = 0; l < influencingLights.GetSize(); l++)
        {
          LightInfluences* infl = &(obj->GetLightInfluences (pg,
            influencingLights[l]));
          allInfluences.Push (InfluencePair (influencingLights[l], infl));
        }
        if (allInfluences.GetSize() == 0) continue;
        
        InfluencePair::SortByIntensity sortPred;
        allInfluences.Sort (sortPred);
        
        uint w = allInfluences[0].infl->GetWidth();
        const size_t numDirections = w * allInfluences[0].infl->GetHeight();
        csVector3* hunk = (csVector3*)cs_malloc (4 * numDirections * sizeof (csVector3));
        csVector3* finalDirectionMap[2];
        csVector3* tempDirectionMap[2];
        for (int i = 0; i < 2; i++)
        {
          finalDirectionMap[i] = hunk + i*numDirections;
          tempDirectionMap[i] = hunk + (i+2)*numDirections;
	}
	memset (hunk, 0, 4*numDirections * sizeof (csVector3));
        
        for (size_t i = 0; i < allInfluences.GetSize(); i++)
        {
          InfluencePair& ip = allInfluences[i];
          CS_ASSERT(w == ip.infl->GetWidth());
          
          float impact1 = 0, impact2 = 0;
          // choose sub-direction map with lowest 'impact'
          for (uint y = 0; y < ip.infl->GetHeight(); y++)
          {
	    for (uint x = 0; x < w; x++)
	    {
	      if (ip.infl->GetIntensityForLocalCoord (x, y) <
	          globalConfig.GetLMProperties().blackThreshold)
	        continue;
	      const csVector3& d = ip.infl->GetDirectionForLocalCoord (x, y);
	      
	      uint coord = x+w*y;
	      /* 'Impact' actually measures how much intensity we have to 
	         share a direction map point with */
	      csVector3 oldVec0 = tempDirectionMap[0][coord];
	      csVector3 newVec0 = oldVec0 + d;
	      if (oldVec0.IsZero()) oldVec0 = newVec0;
	      oldVec0.Normalize();
	      newVec0.Normalize();
	      csVector3 oldVec1 = tempDirectionMap[1][coord];
	      csVector3 newVec1 = oldVec1 + d;
	      if (oldVec1.IsZero()) oldVec1 = newVec1;
	      oldVec1.Normalize();
	      newVec1.Normalize();
	      impact1 += 1.0f - csClamp (newVec0 * oldVec0, 1.0f, 0.0f);
	      impact2 += 1.0f - csClamp (newVec1 * oldVec1, 1.0f, 0.0f);
	      tempDirectionMap[0][coord] = newVec0;
	      tempDirectionMap[1][coord] = newVec1;
	    }
          }
          // record choice of sub-direction map
          if (impact1 <= impact2)
          {
            ip.subMap = 0;
            memcpy (finalDirectionMap[0], tempDirectionMap[0],
              numDirections * sizeof (csVector3));
            memcpy (tempDirectionMap[1], finalDirectionMap[1],
              numDirections * sizeof (csVector3));
          }
          else
          {
            ip.subMap = 1;
            memcpy (tempDirectionMap[0], finalDirectionMap[0],
              numDirections * sizeof (csVector3));
            memcpy (finalDirectionMap[1], tempDirectionMap[1],
              numDirections * sizeof (csVector3));
          }
        }
        
        for (size_t i = 0; i < allInfluences.GetSize(); i++)
        {
          // add onto final direction maps
          InfluencePair& ip = allInfluences[i];
          
          DirectionMap& dirMap = *(directionMaps[ip.subMap].Get (
            obj->GetPrimitiveGroupLightmap (pg)));
          dirMap.AddFromLightInfluences (*(ip.infl));
        }
        
        cs_free (hunk);
        
	if (--u == 0)
	{
	  primsProgress->IncProgress (primProgressStep);
	  u = updateFreq;
	}
      }
      
      primsProgress->SetProgress (1);
      delete primsProgress;
      objProgress.SetProgress (progressStep * o);
    }
    
    normProgress.SetProgress (0);
    progressStep = 0.5f/directionMaps[0].GetSize();
    // Postprocess (normalize) 'final' direction maps
    for (size_t d = 0; d < directionMaps[0].GetSize(); d++)
    {
      directionMaps[0].Get(d)->Normalize();
      normProgress.SetProgress ((2*d)*progressStep);
      directionMaps[1].Get(d)->Normalize();
      normProgress.SetProgress ((2*d+1)*progressStep);
    }
  
    progress.SetProgress (1);
  }
  
  void Scene::SaveSpecularDirectionMaps (LoadedFile* fileInfo,
                                         csStringArray& filenames,
                                         csStringArray& textureNames)
  {
    for (size_t d = 0; d < directionMaps[0].GetSize(); d++)
    {
      CS_ASSERT(specDirectionMapCount == 3); // This method needs adjustments if
					     // specDirectionMapCount changes
					     
      uint width = directionMaps[0].Get (d)->GetWidth();
      uint height = directionMaps[0].Get (d)->GetHeight();
					     
      void* data[specDirectionMapCount];
      const size_t pixelArraySize = width*height;
  
      csRGBpixel *pixelData[specDirectionMapCount];
      for (int i = 0; i < specDirectionMapCount; i++)
      {
	data[i] = cs_malloc (pixelArraySize * sizeof (csRGBpixel));
	pixelData[i] = (csRGBpixel*)data[i];
      }
      
      ScopedSwapLock<DirectionMap> l1 (*(directionMaps[0].Get (d)));
      ScopedSwapLock<DirectionMap> l2 (*(directionMaps[1].Get (d)));
      const csVector3* map0 = directionMaps[0].Get (d)->GetDirections();
      const csVector3* map1 = directionMaps[1].Get (d)->GetDirections();
      for (uint y = 0; y < height; y++)
      {
	for (uint x = 0; x < width; x++)
	{
	  uint i = y*width+x;
		  
	  csRGBpixel inflRGB1, inflRGB2;
	  csVector3 d1 = map0[i];
	  csVector3 d2 = map1[i];
	  if (!d1.IsZero())
	  {
	    inflRGB1.red = csClamp ((int) ((d1.x+1) * 127.5f), 255, 0);
	    inflRGB1.green = csClamp ((int) ((d1.y+1) * 127.5f), 255, 0);
	    inflRGB1.blue = csClamp ((int) ((d1.z+1) * 127.5f), 255, 0);
	  }
	  else
	  {
	    inflRGB1.red = 128;
	    inflRGB1.green = 128;
	    inflRGB1.blue = 0;
	  }
	  
	  if (!d2.IsZero())
	  {
	    inflRGB2.red = csClamp ((int)((d2.x+1) * 127.5f), 255, 0);
	    inflRGB2.green = csClamp ((int) ((d2.y+1) * 127.5f), 255, 0);
	    inflRGB2.blue = csClamp ((int) ((d2.z+1) * 127.5f), 255, 0);
	  }
	  else
	  {
	    inflRGB2.red = 128;
	    inflRGB2.green = 128;
	    inflRGB2.blue = 0;
	  }
	  
	  (pixelData[0])[i].Set (0, inflRGB1.red, 0, inflRGB1.green);
	  (pixelData[1])[i].Set (0, inflRGB2.red, 0, inflRGB2.green);
	  (pixelData[2])[i].Set (0, inflRGB1.blue, 0, inflRGB2.blue);
	}
      }
      
      // make new images
      for (int i = 0; i < specDirectionMapCount; i++)
      {
	csRef<iImage> img;
	img.AttachNew (new csImageMemory (width, height, pixelData[i], false,
	  CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA));
    
	/* @@@ Ideal would be a format such as ATI2N/LATC, but as long as that
	  is not supported ... */
	csRef<iDataBuffer> imgData = globalLighter->imageIO->Save (img,
	  "image/dds", "format=dxt5");
	csString fname (GetDirectionMapFilename (d, i));
	csRef<iFile> file = globalLighter->vfs->Open (fname, VFS_FILE_WRITE);
	if (file)
	{
	  file->Write (imgData->GetData (), imgData->GetSize ());
	  file->Flush ();
	}
	cs_free (data[i]);
	
	filenames.Push (fname);
	textureNames.Push (Lightmap::GetTextureNameFromFilename (fname));
      }
    }
  }

  const char* Scene::GetSolidColorFile (LoadedFile* fileInfo, 
					  const csColor& col)
  {
    int r = int (col.red * 255.99f);
    int g = int (col.green * 255.99f);
    int b = int (col.blue * 255.99f);
    csString colorStr;
    colorStr.Format ("lightmaps/%s_%02x%02x%02x.png", 
      fileInfo->levelName.GetData(), r, g, b);
    if (!solidColorFiles.Contains (colorStr))
    {
      csRef<iImage> img;
      img.AttachNew (new csImageMemory (1, 1));
      ((csRGBpixel*)img->GetImageData())->Set (r, g, b);
      csRef<iDataBuffer> imgData = globalLighter->imageIO->Save (img, "image/png");
      csRef<iFile> file = globalLighter->vfs->Open (colorStr, VFS_FILE_WRITE);
      if (file)
      {
        file->Write (imgData->GetData (), imgData->GetSize ());
      }
    }
    fileInfo->texFileNamesToDelete.Delete (colorStr);
    return solidColorFiles.Register (colorStr);
  }

  struct SaveTexture
  {
    csString filename;
    csString texname;
    csStringArray pdLightmapFiles;
    csArray<Light*> pdLights;
    Lightmap* lm;
    bool isSolid;
    csColor solidColor;
    
    SaveTexture() : lm (0), isSolid (false) {}
  };

  void Scene::SaveLightmapsToDom (iDocumentNode* r, LoadedFile* fileInfo,
                                  Statistics::Progress& progress)
  {
    Statistics::Progress filesProgress (0, 90, &progress);
    Statistics::Progress texturesProgress (0, 7, &progress);
    Statistics::Progress cleanupProgress (0, 1, &progress);

    progress.SetProgress (0);

    csRef<iSyntaxService> synsrv = 
      csQueryRegistry<iSyntaxService> (globalLighter->objectRegistry);

    size_t u, updateFreq;
    float progressStep;

    u = updateFreq = filesProgress.GetUpdateFrequency (lightmaps.GetSize());
    progressStep = updateFreq * (1.0f / lightmaps.GetSize());

    filesProgress.SetProgress (0);
    csArray<SaveTexture> texturesToSave;
    csStringSet pdlightNums;
    size_t realNumLMs = lightmaps.GetSize ();
    if (globalConfig.GetLighterProperties().directionalLMs)
      realNumLMs /= 4;
    for (unsigned int i = 0; i < lightmaps.GetSize (); i++)
    {
      SaveTexture savetex;
      size_t subNum = i / realNumLMs;
      Lightmap* lm = lightmaps[i];
      savetex.lm = lm;
    #ifndef DUMP_NORMALS
      lightmapPostProc.ApplyAmbient (lightmaps[i]);
      lightmapPostProc.ApplyExposure (lightmaps[i]);
    #endif
      savetex.isSolid = lm->IsOneColor (
        globalConfig.GetLMProperties().blackThreshold, 
        savetex.solidColor);
      if (!savetex.isSolid)
      {
        const char* textureFilename = lm->GetFilename ();
        // Save the lightmap
        // Texture file name is relative to world file
        csVfsDirectoryChanger chdir (globalLighter->vfs);
        chdir.ChangeTo (fileInfo->directory);
        lm->SaveLightmap (textureFilename);
        savetex.filename = textureFilename;
      }
      savetex.texname = lm->GetTextureName();

      PDLightmapsHash::GlobalIterator pdlIt = pdLightmaps.GetIterator();
      while (pdlIt.HasNext())
      {
        csPtrKey<Light> key;
        LightmapPtrDelArray* lm = pdlIt.Next(key);
        if (!lm->Get (i) || 
            lm->Get (i)->IsNull (
              globalConfig.GetLMProperties().blackThreshold,
              globalConfig.GetLMProperties().grayPDMaps))
        {
          continue;
        }

        csString lmID (key->GetLightID ().HexString());
        csString textureFilename;
        if (subNum == 0)
          textureFilename.Format ("lightmaps/%s_%u_%u.png",
            fileInfo->levelName.GetData(), i,
	      static_cast<uint>(pdlightNums.Request (lmID)));
        else
          textureFilename.Format ("lightmaps/%s_%zu_%u_d%zu.png", 
            fileInfo->levelName.GetData(), i % realNumLMs, 
	      static_cast<uint>(pdlightNums.Request (lmID)), subNum);

        {
          // Texture file name is relative to world file
          csVfsDirectoryChanger chdir (globalLighter->vfs);
          chdir.ChangeTo (fileInfo->directory);
        #ifndef DUMP_NORMALS
          lightmapPostProc.ApplyExposure (lm->Get (i));
        #endif
          lm->Get (i)->SaveLightmap (textureFilename,
            globalConfig.GetLMProperties().grayPDMaps);
        }
        savetex.pdLightmapFiles.Push (textureFilename);
        savetex.pdLights.Push (key);
      }

      texturesToSave.Push (savetex);

      if (--u == 0)
      {
        filesProgress.IncProgress (progressStep);
        u = updateFreq;
      }
    }
    if (globalConfig.GetLighterProperties().specularDirectionMaps)
    {
      csStringArray filenames, textureNames;
      SaveSpecularDirectionMaps (fileInfo, filenames, textureNames);
      for (size_t j = 0; j < filenames.GetSize(); j++)
      {
	SaveTexture specsavetex;
	specsavetex.filename = filenames[j];
	specsavetex.texname = textureNames[j];
	texturesToSave.Push (specsavetex);
      }
    }
    filesProgress.SetProgress (1);

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
    else
    {
      // Node exists already; clean out
      csRef<iDocumentNodeIterator> it = texturesNode->GetNodes ();
      while (it->HasNext ())
      {
        csRef<iDocumentNode> node = it->Next();
        if (node->GetType() != CS_NODE_ELEMENT) continue;
        CollectDeleteTextures (node, fileInfo->texFileNamesToDelete);
      }

      texturesNode->RemoveNodes ();
    }

    u = updateFreq = texturesProgress.GetUpdateFrequency (
      texturesToSave.GetSize());
    progressStep = updateFreq * (1.0f / texturesToSave.GetSize());

    texturesProgress.SetProgress (0);
    for (unsigned int i = 0; i < texturesToSave.GetSize (); i++)
    {
      const SaveTexture& textureToSave = texturesToSave[i];

      csRef<iDocumentNode> textureNode;
      textureNode = texturesNode->CreateNodeBefore (CS_NODE_ELEMENT);
      textureNode->SetValue ("texture");
      textureNode->SetAttribute ("name", textureToSave.texname.GetData ());
           
      csRef<iDocumentNode> classNode = 
        textureNode->CreateNodeBefore (CS_NODE_ELEMENT);
      classNode->SetValue ("class");
      csRef<iDocumentNode> classContNode = 
        classNode->CreateNodeBefore (CS_NODE_TEXT);
      classContNode->SetValue ("lightmap");

      csRef<iDocumentNode> fileNode;
      const char* filename = 0;
      if (textureToSave.isSolid)
      {
        if (textureToSave.pdLightmapFiles.GetSize() == 0)
        {
          filename = GetSolidColorFile (fileInfo, textureToSave.solidColor);
        }
        else
        {
          csRef<iDocumentNode> sizeNode = 
            textureNode->CreateNodeBefore (CS_NODE_ELEMENT);
          sizeNode->SetValue ("size");
          sizeNode->SetAttributeAsInt ("width", textureToSave.lm->GetWidth());
          sizeNode->SetAttributeAsInt ("height", textureToSave.lm->GetHeight());
        }
      }
      else
      {
        filename = textureToSave.filename.GetData ();
        fileInfo->texFileNamesToDelete.Delete (textureToSave.filename);
      }

      if (filename != 0)
      {
        fileNode = textureNode->CreateNodeBefore (CS_NODE_ELEMENT);
        fileNode->SetValue ("file");

        csRef<iDocumentNode> filenameNode =
          fileNode->CreateNodeBefore (CS_NODE_TEXT);
        filenameNode->SetValue (filename);
      }

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

        if (textureToSave.isSolid)
        {
          csRef<iDocumentNode> baseColorNode = 
            pdlightParamsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
          baseColorNode->SetValue ("basecolor");
          synsrv->WriteColor (baseColorNode, textureToSave.solidColor);
        }

        for (size_t p = 0; p < textureToSave.pdLightmapFiles.GetSize(); p++)
        {
          csRef<iDocumentNode> mapNode = 
            pdlightParamsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
          mapNode->SetValue ("map");
          mapNode->SetAttribute ("lightsector", 
            textureToSave.pdLights[p]->GetSector()->sectorName);
          mapNode->SetAttribute ("lightname", 
            textureToSave.pdLights[p]->GetName());

          csRef<iDocumentNode> mapContents = 
            mapNode->CreateNodeBefore (CS_NODE_TEXT, 0);
          mapContents->SetValue (textureToSave.pdLightmapFiles[p]);
          fileInfo->texFileNamesToDelete.Delete (textureToSave.pdLightmapFiles[p]);
        }
      }

      fileInfo->texturesToClean.Delete (textureToSave.texname.GetData ());

      if (--u == 0)
      {
        texturesProgress.IncProgress (progressStep);
        u = updateFreq;
      }
    }
    texturesProgress.SetProgress (1);

    cleanupProgress.SetProgress (0);
    DocSystem::RemoveDuplicateChildren(texturesNode, 
      texturesNode->GetNodes ("texture"),
      DocSystem::NodeAttributeCompare("name"));
    cleanupProgress.SetProgress (1);

    progress.SetProgress (1);
  }
    
  csPtr<iDataBuffer> Scene::SaveDebugData (LoadedFile& fileInfo, 
                                           iDataBuffer* sourceData, 
                                           Statistics::Progress& progress)
  {
    if (!globalLighter->rayDebug.IsEnabled())
    {
      return csPtr<iDataBuffer> (csRef<iDataBuffer> (sourceData));
    }

    csRef<iDocument> doc (globalLighter->docSystem->CreateDocument ());
    const char* err = doc->Parse (sourceData);
    if (err != 0)
    {
      globalLighter->Report ("Error re-parsing '%s': %s", 
        fileInfo.levelName.GetData(), err);
      return csPtr<iDataBuffer> (csRef<iDataBuffer> (sourceData));
    }

    float sectorProgress = 1.0f / sectors.GetSize ();

    csRef<iDocument> newDoc (globalLighter->docSystem->CreateDocument ());
    csRef<iDocumentNode> srcRoot (doc->GetRoot ());
    csRef<iDocumentNode> newRoot (newDoc->CreateRoot ());

    csRef<iDocumentNodeIterator> topIt (srcRoot->GetNodes ());
    while (topIt->HasNext ())
    {
      csRef<iDocumentNode> node = topIt->Next ();
      csRef<iDocumentNode> newNode = 
        newRoot->CreateNodeBefore (node->GetType());
      newNode->SetValue (node->GetValue ());
      CS::DocSystem::CloneAttributes (node, newNode);

      csRef<iDocumentNodeIterator> nodeIt (node->GetNodes ());
      while (nodeIt->HasNext ())
      {
        bool doDupe = true;
        csRef<iDocumentNode> child = nodeIt->Next ();
        if (child->GetType() == CS_NODE_ELEMENT)
        {
          const char* val = child->GetValue ();
          if (strcmp (val, "sector") == 0)
          {
            Statistics::Progress* progSector = 
              progress.CreateProgress (sectorProgress);

            const char* sectorName = child->GetAttributeValue ("name");
            if (!sectorName) continue;
            Sector* sector = sectors.Get (sectorName, (Sector*)0);
            if (!sector) continue;

            Statistics::Progress* progAppend;
            progAppend = progSector->CreateProgress (0.9f);
            globalLighter->rayDebug.AppendMeshFactories (sector, newNode, 
              *progAppend);
            delete progAppend;
            
            doDupe = false;
            csRef<iDocumentNode> newChild = 
              newNode->CreateNodeBefore (child->GetType());
            CS::DocSystem::CloneNode (child, newChild);
            progAppend = progSector->CreateProgress (0.1f);
            globalLighter->rayDebug.AppendMeshObjects (sector, newChild, 
              *progAppend);
            delete progAppend;

            globalLighter->rayDebug.FreeInfo (sector);

            progSector->SetProgress (1);
            delete progSector;
          }
        }
        if (doDupe)
        {
          csRef<iDocumentNode> newChild = 
            newNode->CreateNodeBefore (child->GetType());
          CS::DocSystem::CloneNode (child, newChild);
        }
      }
    }

    csMemFile mf;
    err = newDoc->Write (&mf);
    if (err != 0)
    {
      globalLighter->Report ("Error re-writing '%s': %s", 
        fileInfo.levelName.GetData(), err);
      return csPtr<iDataBuffer> (csRef<iDataBuffer> (sourceData));
    }
    return mf.GetAllData ();
  }
    
  iCollection* Scene::GetCollection (iObject* obj)
  {
    csRef<iCollectionArray> collections = globalLighter->engine->GetCollections ();
    for (size_t i = 0; i < collections->GetSize(); i++)
    {
      iCollection* collection = collections->Get (i);
      if (collection->IsParentOf (obj)) return collection;
    }
    return 0;
  }
  
  bool Scene::IsObjectFromBaseDir (iObject* obj, const char* baseDir)
  {
    iCollection* collection = GetCollection (obj);
    if (collection == 0) return true;
    
    csRef<iSaverFile> saverFile (CS::GetChildObject<iSaverFile> (collection->QueryObject()));
    if (saverFile.IsValid()) return true;
    
    return strncmp (saverFile->GetFile(), baseDir, strlen (baseDir)) == 0;
  }
  
  bool Scene::IsFilenameFromBaseDir (const char* filename, const char* baseDir)
  {
    const size_t fnLen = strlen (filename);
    CS_ALLOC_STACK_ARRAY(char, fnDir, fnLen+1);
    CS_ALLOC_STACK_ARRAY(char, fnName, fnLen+1);
    csSplitPath (filename, fnDir, fnLen+1, fnName, fnLen+1);
    
    globalLighter->vfs->PushDir ();
    globalLighter->vfs->ChDir (fnDir);
    bool ret = strncmp (globalLighter->vfs->GetCwd(), baseDir, strlen (baseDir)) == 0;
    globalLighter->vfs->PopDir ();
    return ret;
  }

  csRef<iDocument> Scene::EnsureChangeable (iDocument* doc)
  {
    int changeable = doc->Changeable ();
    if (changeable == CS_CHANGEABLE_YES)
      return doc;

    csRef<iDocument> newDoc;
    if (changeable == CS_CHANGEABLE_NEWROOT)
      newDoc = doc;
    else
      newDoc = globalLighter->docSystem->CreateDocument ();

    csRef<iDocumentNode> oldRoot = doc->GetRoot ();
    csRef<iDocumentNode> newRoot = newDoc->CreateRoot ();
    CS::DocSystem::CloneNode (oldRoot, newRoot);

    return newDoc;
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
    //LightmapPostProcess::ApplyExposureFunction(colors, numColors, 1.8f, 0.5f);
    LightmapPostProcess::ApplyScaleClampFunction (colors, numColors, 0.5f, 1.0f);
  }
  
  void Scene::LightingPostProcessor::ApplyAmbient (Lightmap* lightmap)
  {
    if (globalConfig.GetLighterProperties ().indirectLightEngine == LIGHT_ENGINE_NONE &&
        globalConfig.GetLighterProperties ().globalAmbient)
    {
      csColor amb;
      globalLighter->engine->GetAmbientLight (amb);
      lightmap->AddAmbientTerm (amb);
    }
  }

  void Scene::LightingPostProcessor::ApplyAmbient (csColor* colors, size_t numColors)
  {
    if (globalConfig.GetLighterProperties ().indirectLightEngine == LIGHT_ENGINE_NONE &&
        globalConfig.GetLighterProperties ().globalAmbient)
    {
      csColor amb;
      globalLighter->engine->GetAmbientLight (amb);
      LightmapPostProcess::AddAmbientTerm (colors, numColors, amb);
    }
  }

}