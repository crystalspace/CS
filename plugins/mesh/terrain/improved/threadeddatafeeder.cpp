/*
    Copyright (C) 2006 by Kapoulkine Arseny

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

#include "iterrain/terraincell.h"

#include "csgeom/csrect.h"

#include "igraphic/image.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"

#include "csutil/refarr.h"
#include "csutil/dirtyaccessarray.h"

#include "csgfx/imagemanipulate.h"

#include "iengine/material.h"

#include "iengine/engine.h"

#include "imap/loader.h"

#include "csutil/threadjobqueue.h"

#include "threadeddatafeeder.h"

#include "iterrain/terrainsystem.h"

CS_PLUGIN_NAMESPACE_BEGIN(ImprovedTerrain)
{

SCF_IMPLEMENT_FACTORY (csTerrainThreadedDataFeeder)

class csTerrainFeedJob : public scfImplementation1<csTerrainFeedJob, iJob>
{
  iTerrainCell* cell;
  csTerrainFeederData* feed_data;
  iObjectRegistry* object_reg;
public:
  csTerrainFeedJob(iTerrainCell* cell, csTerrainFeederData* data,
	  iObjectRegistry* object_reg)
    : scfImplementationType (this)
  {
    this->cell = cell;
    this->feed_data = data;
    this->object_reg = object_reg;
	
	static int counter = 0;
	++counter;

	if (counter > 100)
	{
		counter = 0;
	}
  }
  
  virtual void Run()
  {
    int width = cell->GetGridWidth ();
    int height = cell->GetGridHeight ();

    csRef<iLoader> loader = csQueryRegistry<iLoader> (object_reg);
  
    csRef<iImage> map = loader->LoadImage (feed_data->heightmap_source,
      CS_IMGFMT_PALETTED8);

    if (!map) return;
 
    if (map->GetWidth () != width || map->GetHeight () != height)
    {
      map = csImageManipulate::Rescale (map, width, height);
    }

    feed_data->height_width = width;
    feed_data->height_height = height;

    feed_data->height_data = new float[width * height];

    float* h_data = feed_data->height_data;
  
    const unsigned char* imagedata = (const unsigned char*)map->GetImageData ();

    float size_z = cell->GetSize ().z;

    for (int y = 0; y < height; ++y)
    {
      for (int x = 0; x < width; ++x)
      {
        float xd = float(x - width/2) / width;
        float yd = float(y - height/2) / height;

        *h_data++ = *imagedata++ / 255.0f * size_z;
      }
    }
  
    csRef<iImage> material = loader->LoadImage (feed_data->mmap_source,
      CS_IMGFMT_PALETTED8);
    
    if (!material) return;

    if (material->GetWidth () != cell->GetMaterialMapWidth () ||
      material->GetHeight () != cell->GetMaterialMapHeight ())
      material = csImageManipulate::Rescale (material,
        cell->GetMaterialMapWidth (), cell->GetMaterialMapHeight ());
   
    int mwidth = material->GetWidth ();
    int mheight = material->GetHeight ();

    feed_data->material_width = mwidth;
    feed_data->material_height = mheight;

    size_t mcount = cell->GetTerrain ()->GetMaterialPalette ().GetSize ();

    feed_data->material_data.SetSize (mcount);

    CS_ASSERT (mcount < 255);

    for (unsigned char i = 0; i < mcount; ++i)
    {
      const unsigned char* materialmap = (const unsigned char*)
	    material->GetImageData ();

      unsigned char* m_data =
        (feed_data->material_data[i] = new unsigned char[mwidth * mheight]);

      for (int y = 0; y < mheight; ++y)
      {
        for (int x = 0; x < mwidth; ++x)
        {
          *m_data++ = (*materialmap++ == i) ? 255 : 0;
        }
      }
    }

    feed_data->result = true;
  }
};

csTerrainThreadedDataFeeder::csTerrainThreadedDataFeeder (iBase* parent)
  : csTerrainSimpleDataFeeder(parent)
{
}

csTerrainThreadedDataFeeder::~csTerrainThreadedDataFeeder ()
{
  if (job.IsValid ())
    job_queue->Unqueue (job);
}

bool csTerrainThreadedDataFeeder::PreLoad (iTerrainCell* cell)
{
  feed_data.heightmap_source = heightmap_source;
  feed_data.mmap_source = mmap_source;

  job.AttachNew (new csTerrainFeedJob(cell, &feed_data, object_reg));
  job_queue->Enqueue (job);

  return true;
}

bool csTerrainThreadedDataFeeder::Load (iTerrainCell* cell)
{
  if (job)
  {
    feed_data.heightmap_source = heightmap_source;
    feed_data.mmap_source = mmap_source;

    // PreLoad was called earlier, so let the thread finish
    // and upload data (we can't do it in the thread because
    // of OpenGL issues (context access from the main thread
    // only)
    job_queue->PullAndRun (job);

    if (!feed_data.result) return false;
    
    csLockedHeightData data = cell->LockHeightData (
      csRect(0, 0, feed_data.height_width, feed_data.height_height));

    float* src_data = feed_data.height_data;
    
    for (unsigned int y = 0; y < feed_data.height_height; ++y)
    {
      memcpy (data.data, src_data, feed_data.height_width * sizeof(float));
      data.data += data.pitch;
      src_data += feed_data.height_width;
    }

    cell->UnlockHeightData ();

    for (unsigned int m = 0; m < feed_data.material_data.GetSize (); ++m)
      cell->SetMaterialMask (m, feed_data.material_data[m],
        feed_data.material_width, feed_data.material_height);

    return true;
  }
  else return csTerrainSimpleDataFeeder::Load (cell);
}

bool csTerrainThreadedDataFeeder::Initialize (iObjectRegistry* object_reg)
{
  this->object_reg = object_reg;
  
  static const char queue_tag[] = "crystalspace.jobqueue.threadeddatafeeder";
  job_queue = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg, queue_tag, iJobQueue);
  if (!job_queue.IsValid ())
  {
    job_queue.AttachNew (new csThreadJobQueue ());
    object_reg->Register (job_queue, queue_tag);
  }

  return true;
}

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)
