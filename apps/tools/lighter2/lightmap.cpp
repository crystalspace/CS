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

#define Byte z_Byte	/* Kludge to avoid conflicting typedef in zconf.h */
#include <zlib.h>
#undef Byte

#include "common.h"

#include "lightmap.h"
#include "lighter.h"

namespace lighter
{
  LightmapCache* globalLMCache;

  Lightmap::Lightmap (uint width, uint height)
    : data (0), width (0), height (0), maxUsedU (0), maxUsedV (0), 
    lightmapAllocator (csRect (0,0,1,1)), texture (0)
  {
    Grow (width, height);
  }

  Lightmap::~Lightmap ()
  {
    globalLMCache->RemoveLM (this);

    delete data;
  }

  void Lightmap::AddAmbientTerm (const csColor amb)
  {
    LightmapCacheLock l (this);
    LightmapPostProcess::AddAmbientTerm (data->colorArray,
      data->colorArraySize, amb);
  }

  void Lightmap::ApplyExposureFunction (float expConstant, float expMax)
  {
    LightmapCacheLock l (this);
    LightmapPostProcess::ApplyExposureFunction (data->colorArray,
      data->colorArraySize,  expConstant, expMax);
  }

  void Lightmap::SaveLightmap (const csString& fname)
  {
    LightmapCacheLock l (this);

    filename = fname;
    //write it out

#ifndef DUMP_NORMALS
    // 0.5 to account for the fact that the shader does *2
    ApplyExposureFunction (1.8f, 0.5f); 
#endif

    // first we downsample to LDR csRGBpixel RGBA
    csRGBpixel *pixelData = new csRGBpixel[width*height];
    for (uint i = 0; i < data->colorArraySize; i++)
    {
      csColor &c = data->colorArray[i];
      c.Clamp (1.0f,1.0f,1.0f); //just make sure we don't oversaturate below
      pixelData[i].red = (uint) (c.red * 255.0f);
      pixelData[i].green = (uint) (c.green * 255.0f);
      pixelData[i].blue = (uint) (c.blue * 255.0f);
    }

    // make an image
    csRef<iImage> img;
    img.AttachNew (new csImageMemory (width, height, pixelData));
    csRef<iDataBuffer> imgData = globalLighter->imageIO->Save (img, "image/png");
    csRef<iFile> file = globalLighter->vfs->Open (fname, VFS_FILE_WRITE);
    if (file)
    {
      file->Write (imgData->GetData (), imgData->GetSize ());
      file->Flush ();
    }
    delete[] pixelData;
  }

  void Lightmap::FixupLightmap (const LightmapMask& mask)
  {
    LightmapCacheLock l (this);
    csColor* lmData = data->colorArray;
    const float* mmData = mask.maskData.GetArray ();

    const size_t size = data->colorArraySize;

    for (uint j = 0; j < size; j++, lmData++, mmData++)
    {
      if (*mmData < FLT_EPSILON || *mmData >= 1.0f) continue;

      *lmData *= (1.0f / *mmData);
    }

    // Reset
    lmData = data->colorArray;
    mmData = mask.maskData.GetArray ();

    for (uint v = 0; v < height; v++)
    {
      // now scan over the row
      for (uint u = 0; u < width; u++)
      {
        const uint idx = v*width+u;

        // Only try to fix non-masked
        if (mmData[idx]>0) continue;

        uint count = 0;
        csColor newColor (0.0f,0.0f,0.0f);

        // We have a row above to use
        if (v > 0)
        {
          // We have a column to the left
          if (u > 0 && mmData[(v-1)*width+(u-1)] > FLT_EPSILON) newColor += lmData[(v-1)*width+(u-1)], count++;
          if (mmData[(v-1)*width+(u)] > FLT_EPSILON) newColor += lmData[(v-1)*width+(u)], count++;
          if (u < width-1 && mmData[(v-1)*width+(u+1)] > FLT_EPSILON) newColor += lmData[(v-1)*width+(u+1)], count++;
        }

        //current row
        if (u > 0 && mmData[v*width+(u-1)] > FLT_EPSILON) newColor += lmData[v*width+(u-1)], count++;
        if (u < width-1 && mmData[v*width+(u+1)] > FLT_EPSILON) newColor += lmData[v*width+(u+1)], count++;

        // We have a row below
        if (v < (height-1))
        {
          if (u > 0 && mmData[(v+1)*width+(u-1)] > FLT_EPSILON) newColor += lmData[(v+1)*width+(u-1)], count++;
          if (mmData[(v+1)*width+(u)] > FLT_EPSILON) newColor += lmData[(v+1)*width+(u)], count++;
          if (u < width-1 && mmData[(v+1)*width+(u+1)] > FLT_EPSILON) newColor += lmData[(v+1)*width+(u+1)], count++;
        }

        if (count > 0) 
        {
          newColor *= (1.0f/count);
          lmData[idx] = newColor;
        }
      }
    }
  }

  iTextureWrapper* Lightmap::GetTexture()
  {
    if (texture == 0)
    {
      texture = globalLighter->engine->CreateBlackTexture (
        GetTextureName(), 1, 1, 0, CS_TEXTURE_3D);
    }
    return texture;
  }

  bool Lightmap::IsNull ()
  {
    LightmapCacheLock l (this);

    for (uint i = 0; i < data->colorArraySize; i++)
    {
      const csColor &c = data->colorArray[i];
      if (!c.IsBlack ())
        return false;
    }
    return true;
  }

  csString Lightmap::GetTextureNameFromFilename (const csString& file)
  {
    csString out (file);
    out.ReplaceAll ("\\", "_"); //replace bad characters
    out.ReplaceAll ("/", "_"); 
    out.ReplaceAll (" ", "_"); 
    out.ReplaceAll (".", "_"); 
    return out;
  }

  //-------------------------------------------------------------------------

  void LightmapPostProcess::AddAmbientTerm (csColor* colors, 
                                            size_t numColors, 
                                            const csColor amb)
  {
    for (uint i = 0; i < numColors; i++)
    {
      colors[i] += amb;
    }
  }

  void LightmapPostProcess::ApplyExposureFunction (csColor* colors, 
                                                   size_t numColors, 
                                                   float expConstant, 
                                                   float expMax)
  {
    for (uint i = 0; i < numColors; i++)
    {
      csColor &c = colors[i];
      c.red = expMax * (1 - expf (-c.red * expConstant));
      c.green = expMax * (1 - expf (-c.green * expConstant));
      c.blue = expMax * (1 - expf (-c.blue * expConstant));
    }
  }

  //-------------------------------------------------------------------------

  LightmapCache::LightmapCache (size_t maxSize)
    : maxCacheSize (maxSize / sizeof(csColor)), currentCacheSize (0),
    currentUnlockTime (1)
  {

  }

  LightmapCache::~LightmapCache ()
  {
  }

  void LightmapCache::Initialize ()
  {
    int maxSize = globalLighter->configMgr->GetInt ("lighter2.lmcachesize", 
      200*1024*1024);

    globalLMCache = new LightmapCache (maxSize);

  }

  void LightmapCache::CleanUp ()
  {
    delete globalLMCache;
    globalLMCache = 0;
  }

  void LightmapCache::LockLM (Lightmap* lm)
  {
    LMEntry *e = lmCache.Get (lm, 0);

    if (!e)
    {
      // Allocate and setup a new entry
      e = new LMEntry;
      e->lm = lm;
      lmCache.Put (lm, e);
    }
    else
    {
      unlockedCacheEntries.Delete (e);

      // Swap it in
      if (!e->data && !SwapIn (e))
      {
        //Reallocate? Assert?
        CS_ASSERT(0);
      }
    }    

    if (!e->data)
    {
      size_t physSize = lm->GetWidth () * lm->GetHeight ();
      e->data = AllocateData (physSize);
    }

    lm->SetData (e->data);    
  }

  void LightmapCache::UnlockLM (Lightmap* lm)
  {
    LMEntry *e = lmCache.Get (lm, 0);
    CS_ASSERT (e != 0);

    unlockedCacheEntries.Add (e);
    e->lastUnlockTime = currentUnlockTime++;
  }

  void LightmapCache::UnlockAll ()
  {
    LMCacheType::GlobalIterator git = lmCache.GetIterator ();

    while (git.HasNext ())
    {
      LMEntry* e = git.Next ();

      if (!unlockedCacheEntries.In (e))
      {
        unlockedCacheEntries.AddNoTest (e);
        e->lastUnlockTime = currentUnlockTime;
      }
    }

    currentUnlockTime++;
  }

  void LightmapCache::RemoveLM (Lightmap* lm)
  {
    LMEntry *e = lmCache.Get (lm, 0);

    if (e)
    {
      unlockedCacheEntries.Delete (e);
      csString tmpFileName = GetLMFileName (e->lm);
      globalLighter->vfs->DeleteFile (tmpFileName);
    }
  }

  bool LightmapCache::SwapOut (LMEntry* e)
  {
    // If nothing to swap out, nothing to do
    if (e->data == 0)
      return false;

    csString tmpFileName = GetLMFileName (e->lm);

    csRef<iFile> file = globalLighter->vfs->Open (tmpFileName, VFS_FILE_WRITE);

    if (!file.IsValid ())
      return false;

    // Write a smallish header
    file->Write ("L2LM", 4);
    file->Write ((const char*)&e->data->colorArraySize, sizeof (size_t));

    // Write the raw data
    z_stream zs;
    memset (&zs, 0, sizeof(z_stream));

    zs.next_in = (z_Byte*)e->data->colorArray;
    zs.avail_in = (uInt)(e->data->colorArraySize*sizeof(csColor));

    if (deflateInit (&zs, 1) != Z_OK)
      return false;

    size_t compressBufferSize = 128*1024;
    CS_ALLOC_STACK_ARRAY(z_Byte, compressBuffer, compressBufferSize);

    while (true)
    {
      zs.next_out = compressBuffer;
      zs.avail_out = (uInt)compressBufferSize;

      int rc = deflate (&zs, Z_FINISH);
      size_t size = compressBufferSize - zs.avail_out;

      if (file->Write ((const char*)compressBuffer, size) != size)
      {
        deflateEnd (&zs);
        return false;
      }

      if (rc == Z_STREAM_END)
        break;
    }
    deflateEnd (&zs);

    //file->Write ((const char*)e->data->colorArray, 
    //  e->data->colorArraySize*sizeof (csColor));

    // Mark it as swapped out too..
    unlockedCacheEntries.Delete (e);
    
    // Delete the memory (@@TODO: pool it)
    currentCacheSize -= e->data->colorArraySize;

    delete e->data;
    e->lm->SetData (0);
    e->data = 0;  

    return true;
  }

  bool LightmapCache::SwapIn (LMEntry* e)
  {
    csString tmpFileName = GetLMFileName (e->lm);

    csRef<iFile> file = globalLighter->vfs->Open (tmpFileName, VFS_FILE_READ);
    if (!file.IsValid ())
      return false;

    char signBuffer [4];
    file->Read (signBuffer, 4);

    // Bail on wrong header
    if (signBuffer[0] != 'L' || signBuffer[1] != '2' ||
      signBuffer[2] != 'L' || signBuffer[3] != 'M')
      return false;

    // Read size
    size_t lmSize;
    file->Read ((char*)&lmSize, sizeof (size_t));

    // Allocate
    e->data = AllocateData (lmSize);

    //file->Read ((char*)e->data->colorArray,
    //  e->data->colorArraySize*sizeof (csColor));
    z_stream zs;
    memset (&zs, 0, sizeof(z_stream));

    zs.next_out = (z_Byte*)e->data->colorArray;
    zs.avail_out = (uInt)(e->data->colorArraySize*sizeof(csColor));

    if (inflateInit (&zs) != Z_OK)
      return false;

    size_t compressBufferSize = 128*1024;
    CS_ALLOC_STACK_ARRAY(z_Byte, compressBuffer, compressBufferSize);

    while (true)
    {
      size_t readSize = file->Read ((char*)compressBuffer, compressBufferSize);

      zs.next_in = compressBuffer;
      zs.avail_in = (uInt)readSize;

      int rc = inflate (&zs, Z_FINISH);

      if (rc == Z_STREAM_END)
        break;
    }


    return true;
  }



  void LightmapCache::FreeMemory (size_t targetSize)
  {
    // Walk through the unlocked set of entries, swap them out until we have enough free memory
    // size_t targetSize = currentCacheSize - desiredAmount;

    csArray<LMEntry*> sortedList;

    UnlockedEntriesType::GlobalIterator git = unlockedCacheEntries.GetIterator ();
    while (git.HasNext ())
    {
      LMEntry* e = git.Next ();
      sortedList.InsertSorted (e, LMEntryAgeCompare);
    }

    csArray<LMEntry*>::Iterator sit = sortedList.GetIterator ();
    while (targetSize < currentCacheSize && sit.HasNext ())
    {
      LMEntry* e = sit.Next ();

      SwapOut (e);
    }
  }

  LightmapData* LightmapCache::AllocateData (size_t size)
  {
    if (currentCacheSize + size > maxCacheSize)
    {
      FreeMemory (maxCacheSize - size);
    }

    LightmapData* lmD = new LightmapData;
    lmD->colorArray = new csColor[size];
    lmD->colorArraySize = size;

    memset (lmD->colorArray, 0, lmD->colorArraySize * sizeof (csColor));

    currentCacheSize += size;

    return lmD;
  }

  csString LightmapCache::GetLMFileName (Lightmap* lm)
  {
    csString tmp;
    tmp << "/tmp/lighter2/lm" << (uintptr_t)lm << ".tmp";
    return tmp;
  }
}
