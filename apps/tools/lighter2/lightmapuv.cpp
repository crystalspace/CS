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

#include "common.h"
#include "lightmapuv.h"

namespace lighter
{

  // Very simple layouter.. just map "flat" on the lightmap
  bool SimpleUVLayouter::LayoutUVOnPrimitives (RadPrimitiveArray &prims, 
    LightmapPtrDelArray& lightmaps)
  {
    // Layout every primitive by itself
    unsigned int i;
    for (i = 0; i < prims.GetSize (); i++)
    {
      RadPrimitive &prim = prims[i];
      bool lmCoordsGood = false;
      int its = 0; //number of iterations
      
      csVector2 minuv, maxuv, uvSize;
      while (!lmCoordsGood && its < 5)
      {
        // Compute lightmapping
        prim.SetLightmapMapping (globalSettings.uTexelPerUnit / (1<<its), 
                                 globalSettings.vTexelPerUnit / (1<<its));

        // Compute uv-size  
        prim.ComputeMinMaxUV (minuv, maxuv);
        uvSize = maxuv-minuv;
        if (uvSize.x < globalSettings.maxLightmapU &&
            uvSize.y < globalSettings.maxLightmapV)
        {
          lmCoordsGood = true;
        }
        its++;
      } 

      if (lmCoordsGood)
      {
        // Ok, resonable size, lets try to find a LM for it
        csRect lmArea; 
        int lmID;
        bool res;
        res = AllocLightmap (lightmaps, uvSize.x, uvSize.y, lmArea, lmID);
        if (!res) continue; 

        lightmaps[lmID]->SetMaxUsedUV (lmArea.xmax, lmArea.ymax);

        // Ok, now remap our coords
        csVector2 uvRemap = csVector2(lmArea.xmin, lmArea.ymin) - minuv;
        prim.RemapUVs (uvRemap);

        prim.SetLightmapID (lmID);
      }
    }

    // Set the size of the lightmaps..
    for (i = 0; i < lightmaps.GetSize (); i++)
    {
      Lightmap *lm = lightmaps[i];
      // Calculate next pow2 size
      uint newWidth = csFindNearestPowerOf2 (lm->GetMaxUsedU ());
      uint newHeight = csFindNearestPowerOf2 (lm->GetMaxUsedV ());
      lm->SetSize (newWidth, newHeight);
    }

    return true;
  }

  bool SimpleUVLayouter::AllocLightmap (LightmapPtrDelArray& lightmaps, 
    int u, int v, csRect &lightmapArea, int &lightmapID)
  {
    // Now see if we can get some space in the already allocated lightmaps
    csSubRect *rect;
    for (unsigned int i = 0; i < lightmaps.GetSize (); i++)
    {
      rect = lightmaps[i]->GetAllocator ().Alloc (u,v,lightmapArea);
      if (rect)
      {
        //We have a space, use it
        lightmapID = i;
        return true;
      }
    }

    // Still here, need a new lightmap
    Lightmap *newL = new Lightmap (globalSettings.maxLightmapU,
                                   globalSettings.maxLightmapV);
    lightmaps.Push (newL);

    rect = newL->GetAllocator ().Alloc (u,v,lightmapArea);
    if (rect)
    {
      //We have a space, use it
      lightmapID = (int)lightmaps.GetSize () - 1;
      return true;
    }

    return false;
  }

}
