/*
  Copyright (C) 2010 Alexandru - Teodor Voicu
      Faculty of Automatic Control and Computer Science of the "Politehnica"
      University of Bucharest
      http://csite.cs.pub.ro/index.php/en/

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <cssysdef.h>

#include "furmesh.h"
#include "furdata.h"

CS_PLUGIN_NAMESPACE_BEGIN(FurMesh)
{
  /********************
  *  csTextureRGBA
  ********************/

  csTextureRGBA::csTextureRGBA ()
  {
    handle = 0;
    data = 0;
  }

  csTextureRGBA::csTextureRGBA (int width, int height)
  {
    this->width = width;
    this->height = height;
    handle = 0;
    data = 0;
  }

  uint8 csTextureRGBA::Get(int x, int y, int channel) const 
  {
    // Compute position in a texture with 4 channels
    int pos = 4 * ( x + y * width ) + channel;

    if (pos >= 4 * width * height || pos < 0)
      return 0;

    return data[ pos ];
  }

  void csTextureRGBA::Set(int x, int y, int channel, uint8 value)
  {
    int pos = 4 * ( x + y * width ) + channel;

    if (pos >= 4 * width * height || pos < 0)
      return;

    data[ pos ] = value;
  }

  bool csTextureRGBA::Read()
  {
    CS::StructuredTextureFormat readbackFmt 
      (CS::TextureFormatStrings::ConvertStructured ("abgr8"));

    csRef<iDataBuffer> buf = handle->Readback(readbackFmt);
    databuf = buf;

    if (!databuf)
      return false;

    handle->GetOriginalDimensions(width, height);
    data = databuf->GetUint8();

    if (!data)
      return false;

    return true;
  }

  void csTextureRGBA::Write()
  {
    // Two calls to Blit function, because he rectangle passed must lie 
    // completely inside the texture
    handle->Blit(0, 0, width, height / 2, data);
    handle->Blit(0, height / 2, width, height / 2, data + (width * height * 2));
  }

  bool csTextureRGBA::Create(iGraphics3D* g3d)
  {
    if (!g3d)
      return false;

    // Create a default texture
    handle = g3d->GetTextureManager()->CreateTexture(width, height, csimg2D, 
      "abgr8", CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_NOFILTER);

    if(!handle)
      return false;

    return true;
  }

  void csTextureRGBA::SaveImage
    (iObjectRegistry* object_reg, const char* texname) const
  {
    csRef<iImageIO> imageio = csQueryRegistry<iImageIO> (object_reg);
    csRef<iVFS> VFS = csQueryRegistry<iVFS> (object_reg);

    if(!data)
    {
      csPrintfErr ("Bad data buffer!\n");
      return;
    }

    csRef<iImage> image;
    image.AttachNew(new csImageMemory (width, height, data, false,
      CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA));

    if(!image.IsValid())
    {
      csPrintfErr ("Error creating image\n");
      return;
    }

    csPrintf ("Saving %zu KB of data.\n", 
      csImageTools::ComputeDataSize (image) / 1024);

    csRef<iDataBuffer> db = imageio->Save (image, "image/png", "progressive");
    
    if (db)
    {
      if (!VFS->WriteFile (texname, (const char*)db->GetData (), db->GetSize ()))
      {
        csPrintfErr ("Failed to write file %s!", CS::Quote::Single (texname));
        return;
      }
    }
    else
    {
      csPrintfErr ("Failed to save png image for basemap!");
      return;
    }	    
  }

  /********************
  *  csFurData
  ********************/

  size_t csFurData::GetControlPointsCount(float controlPointsLOD) const
  {
    if (controlPointsCount == 0)
      return 0;

    if (0.0f <= controlPointsLOD && controlPointsLOD <= 0.33f)
      return 2;
    else if (controlPointsLOD < 0.67f)
      return csMax ( (size_t)(controlPointsCount / 2), (size_t)2);
    else if (controlPointsLOD <= 1.0f)
      return controlPointsCount;

    return 0;
  }

  void csFurData::Clear()
  {
    // Free control points
    if (controlPointsCount)
      delete controlPoints;
  }

  /********************
  *  csFurStrand
  ********************/

  void csFurStrand::SetUV( const csArray<csGuideFur> &guideFurs,
    const csArray<csGuideFurLOD> &guideFursLOD )
  {
    csVector2 strandUV(0);

    for ( size_t j = 0 ; j < GUIDE_HAIRS_COUNT ; j ++ )
      if (guideHairsRef[j].index < guideFurs.GetSize() )
        strandUV += guideHairsRef[j].distance * 
          guideFurs.Get(guideHairsRef[j].index).uv;
      else
        strandUV += guideHairsRef[j].distance * 
          guideFursLOD.Get(guideHairsRef[j].index - guideFurs.GetSize()).uv;

    uv = strandUV;
  }

  void csFurStrand::Generate( size_t controlPointsCount,
    const csArray<csGuideFur> &guideFurs, 
    const csArray<csGuideFurLOD> &guideFursLOD )
  {
    // Allocate control points
    this -> controlPointsCount = controlPointsCount;

    controlPoints = new csVector3[ controlPointsCount ];

    for ( size_t i = 0 ; i < controlPointsCount ; i ++ )
    {
      controlPoints[i] = csVector3(0);

      for ( size_t j = 0 ; j < GUIDE_HAIRS_COUNT ; j ++ )
        if ( guideHairsRef[j].index < guideFurs.GetSize() )
          controlPoints[i] += guideHairsRef[j].distance *
            guideFurs.Get(guideHairsRef[j].index).controlPoints[i];
        else
          controlPoints[i] += guideHairsRef[j].distance *
            guideFursLOD.Get(guideHairsRef[j].index - 
            guideFurs.GetSize()).controlPoints[i];
    }
  }

  void csFurStrand::Update( const csArray<csGuideFur> &guideFurs,
    const csArray<csGuideFurLOD> &guideFursLOD, float controlPointsLOD)
  {
    for ( size_t i = 0 ; i < GetControlPointsCount(controlPointsLOD); i++ )
    {
      controlPoints[i] = csVector3(0);

      for ( size_t j = 0 ; j < GUIDE_HAIRS_COUNT ; j ++ )
        if ( guideHairsRef[j].index < guideFurs.GetSize() )
          controlPoints[i] += guideHairsRef[j].distance * 
            (guideFurs.Get(guideHairsRef[j].index).controlPoints[i]);
        else
          controlPoints[i] += guideHairsRef[j].distance * (guideFursLOD.Get
            (guideHairsRef[j].index - guideFurs.GetSize()).controlPoints[i]);
    }
  }

  void csFurStrand::SetGuideHairsRefs(const csTriangle& triangle, csRandomGen *rng)
  {
    float bA, bB, bC; // barycentric coefficients

    bA = rng->Get();
    bB = rng->Get() * (1 - bA);
    bC = 1 - bA - bB;

    guideHairsRef[0].distance = bA;
    guideHairsRef[0].index = triangle.a;
    guideHairsRef[1].distance = bB;
    guideHairsRef[1].index = triangle.b;
    guideHairsRef[2].distance = bC;
    guideHairsRef[2].index = triangle.c;
  }

  /********************
  *  csGuideFur
  ********************/

  void csGuideFur::Generate (size_t controlPointsCount, float distance,
    const csVector3& pos, const csVector3& direction)
  {
    this->controlPointsCount = controlPointsCount;

    controlPoints = new csVector3[ controlPointsCount ];

    for ( size_t j = 0 ; j < controlPointsCount ; j ++ )
      controlPoints[j] = pos + j * distance * direction;
  }

}
CS_PLUGIN_NAMESPACE_END(FurMesh)

