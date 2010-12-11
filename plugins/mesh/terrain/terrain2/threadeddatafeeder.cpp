/*
  Copyright (C) 2006 by Kapoulkine Arseny
                2007 by Marten Svanfeldt

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

#include "cssysdef.h"

#include "csgeom/csrect.h"
#include "csgfx/imagemanipulate.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/refarr.h"
#include "csutil/threadjobqueue.h"
#include "csutil/threading/mutex.h"

#include "iengine/material.h"
#include "igraphic/image.h"
#include "imap/loader.h"
#include "imesh/terrain2.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"

#include "threadeddatafeeder.h"
#include "feederhelper.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{
SCF_IMPLEMENT_FACTORY (csTerrainThreadedDataFeeder)

struct ThreadedFeederData : public csRefCount
{
  ThreadedFeederData () : haveValidData (false)
  {
  }

  CS::Threading::Mutex dataMutex;

  csRef<iJob> loaderJob;

  csDirtyAccessArray<float> heightmapData;
  csDirtyAccessArray<csVector3> normalmapData;
  csArray<csDirtyAccessArray<unsigned char> > materialmapData;

  csString heightmapSource, normalmapSource, materialmapSource, heightmapFormat;
  float heightScale, heightOffset;

  csArray<csString> alphaMapsSources;
  csRefArray<iImage> alphaMaps;

  bool smoothHeightmap;

  unsigned int gridWidth, gridHeight, materialMapWidth, materialMapHeight;
  size_t materialMapCount;

  bool haveValidData;
};

class ThreadedFeederJob : public scfImplementation1<ThreadedFeederJob, iJob>
{
public:
  ThreadedFeederJob (ThreadedFeederData* data, iLoader* loader,
      iObjectRegistry* reg)
    : scfImplementationType (this), data (data), loader (loader), objReg (reg)
  {

  }

  virtual void Run()
  {
    if (!data || !loader)
      return;

    data->heightmapData.SetSize (data->gridWidth * data->gridHeight);

    float* h_data = data->heightmapData.GetArray ();

    HeightFeederParser mapReader (data->heightmapSource, 
      data->heightmapFormat, loader, objReg);
    mapReader.Load (h_data, data->gridWidth, data->gridHeight, data->gridWidth, 
      data->heightScale, data->heightOffset);

    if (data->smoothHeightmap)
    {
      SmoothHeightmap (h_data, data->gridWidth, data->gridHeight, data->gridWidth);
    }

    if (!data->normalmapSource.IsEmpty ())
    {
      data->normalmapData.SetSize (data->gridWidth * data->gridHeight);

      csVector3* n_data = data->normalmapData.GetArray ();

      NormalFeederParser nmapReader (data->normalmapSource, loader, objReg);
      nmapReader.Load (n_data, data->gridWidth, data->gridHeight, data->gridWidth);
    }

    csRef<iImage> material = loader->LoadImage (
	data->materialmapSource.GetDataSafe (), CS_IMGFMT_PALETTED8);

    if (!material) return;

    if ((unsigned int)material->GetWidth () != data->materialMapWidth ||
        (unsigned int)material->GetHeight () != data->materialMapHeight)
    {
      material = csImageManipulate::Rescale (material,
        data->materialMapWidth, data->materialMapHeight);
    }

    data->materialmapData.SetSize (data->materialMapCount);

    CS_ASSERT (data->materialMapCount < 255);

    const unsigned char* materialmap = (const unsigned char*)
      material->GetImageData ();

    size_t materialMapSize = data->materialMapWidth *  data->materialMapHeight;

    for (size_t i = 0; i < data->materialMapCount; ++i)
    {
      data->materialmapData[i].SetSize (materialMapSize);

      unsigned char* m_data = data->materialmapData[i].GetArray ();

      for (unsigned int y = 0; y < data->materialMapHeight; ++y)
      {
        for (unsigned int x = 0; x < data->materialMapWidth; ++x)
        {
          *m_data++ = (*materialmap++ == i) ? 255 : 0;
        }
      }
    }

    for (size_t i = 0; i < data->alphaMapsSources.GetSize (); ++i)
    {
      csRef<iImage> img = loader->LoadImage (
        data->alphaMapsSources[i].GetDataSafe (), CS_IMGFMT_ANY);

      data->alphaMaps.Push (img);
    }

    data->haveValidData = true;  
  }
  
private:
  ThreadedFeederData* data;
  csRef<iLoader> loader;
  iObjectRegistry* objReg;
};


csTerrainThreadedDataFeeder::csTerrainThreadedDataFeeder (iBase* parent)
  : scfImplementationType (this, parent)
{
  jobQueue.AttachNew (new CS::Threading::ThreadedJobQueue);
}

csTerrainThreadedDataFeeder::~csTerrainThreadedDataFeeder ()
{
}

bool csTerrainThreadedDataFeeder::PreLoad (iTerrainCell* cell)
{
  // Setup a new job iten and enqueue it
  csTerrainSimpleDataFeederProperties* properties = 
    (csTerrainSimpleDataFeederProperties*)cell->GetFeederProperties ();

  if (!loader || !properties)
    return false;

  if (properties->heightmapSource.IsEmpty () ||
    properties->materialmapSource.IsEmpty ())
    return false;

  // Check if there is any existing state associated with it
  csRef<ThreadedFeederData> data = (ThreadedFeederData*)cell->GetFeederData ();

  if (data)
  {
    // We have one, check if it is running etc
    if (data->loaderJob)
      return true; //Already enqueued
  }
  else
  {
    data.AttachNew (new ThreadedFeederData);
    cell->SetFeederData (data);
  }
  
  // Setup job
  data->heightmapSource = properties->heightmapSource;
  data->normalmapSource = properties->normalmapSource;
  data->materialmapSource = properties->materialmapSource;
  data->gridWidth = cell->GetGridWidth ();
  data->gridHeight = cell->GetGridHeight ();
  data->materialMapWidth = cell->GetMaterialMapWidth ();
  data->materialMapHeight = cell->GetMaterialMapHeight ();
  data->materialMapCount = cell->GetTerrain ()->GetMaterialPalette ().GetSize ();
  data->heightScale = cell->GetSize ().y;
  data->heightOffset = properties->heightOffset;
  data->smoothHeightmap = properties->smoothHeightmap;
  
  for (size_t i = 0; i < properties->alphaMaps.GetSize (); ++i)
  {
    data->alphaMapsSources.Push (properties->alphaMaps[i].alphaSource);
  }

  csRef<ThreadedFeederJob> job;
  job.AttachNew (new ThreadedFeederJob (data, loader, objectReg));

  data->loaderJob = job;
  jobQueue->Enqueue (job);

  return true;
}

bool csTerrainThreadedDataFeeder::Load (iTerrainCell* cell)
{
  // Check if there is any existing state associated with it
  csRef<ThreadedFeederData> data = (ThreadedFeederData*)cell->GetFeederData ();
  csTerrainSimpleDataFeederProperties* properties = 
    (csTerrainSimpleDataFeederProperties*)cell->GetFeederProperties ();

  if (data && data->loaderJob)
  {
    // PreLoad was called earlier, so let the thread finish and upload data. 
    // We can't do it in the thread because of thread-safeness issues (context 
    // access from the main thread only)
    jobQueue->PullAndRun (data->loaderJob);
    data->loaderJob = 0;

    if (!data->haveValidData)
      return false; //Failed

    // Copy over data
    csLockedHeightData heightData = cell->LockHeightData (csRect (0, 0,
      data->gridWidth, data->gridHeight));

    float* src_data = data->heightmapData.GetArray ();

    for (unsigned int y = 0; y < data->gridHeight; ++y)
    {
      memcpy (heightData.data, src_data, data->gridWidth * sizeof(float));
      heightData.data += heightData.pitch;
      src_data += data->gridWidth;
    }

    cell->UnlockHeightData ();

    if (!data->normalmapSource.IsEmpty ())
    {
      csLockedNormalData normalData = cell->LockNormalData (csRect (0, 0,
        data->gridWidth, data->gridHeight));

      csVector3* src_ndata = data->normalmapData.GetArray ();

      for (unsigned int y = 0; y < data->gridHeight; ++y)
      {
        memcpy (normalData.data, src_ndata, data->gridWidth * sizeof(csVector3));
        normalData.data += normalData.pitch;
        src_ndata += data->gridWidth;
      }

      cell->UnlockNormalData ();
    }
    else
    {
      cell->RecalculateNormalData ();
    }

    for (size_t m = 0; m < data->materialmapData.GetSize (); ++m)
    {
      cell->SetMaterialMask ((uint)m, data->materialmapData[m].GetArray (),
        data->materialMapWidth, data->materialMapHeight);
    }       

    // Setup any alphamaps/materials
    for (size_t i = 0; i < properties->alphaMaps.GetSize (); ++i)
    {
      iMaterialWrapper* mat = engine->FindMaterial (
        properties->alphaMaps[i].material);

      iImage* alphaMap = data->alphaMaps[i];
      if (mat && alphaMap)
      {
        cell->SetAlphaMask (mat, alphaMap);
      }
    }

    // Remove the data
    cell->SetFeederData (0);

    // Finished
    return true;
  }

  // No pre-load, just load normally
  return csTerrainSimpleDataFeeder::Load (cell);
}

}
CS_PLUGIN_NAMESPACE_END(Terrain2)
