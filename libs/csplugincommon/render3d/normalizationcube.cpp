/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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

#include "csgfx/imagecubemapmaker.h"
#include "csgfx/memimage.h"

#include "csplugincommon/render3d/normalizationcube.h"

void csNormalizationCubeAccessor::FillNormalizationMapSide (
  unsigned char *normdata, int size, 
  int xx, int xy, int xo,
  int yx, int yy, int yo,
  int zx, int zy, int zo)
{
  const float halfSize = (float)size / 2.0f;
  for (int y=0; y < size; y++)
  {
    float yv = ((float)y + 0.5f) / halfSize - 1.0f;
    for (int x=0; x < size; x++)
    {
      float xv = ((float)x + 0.5f) / halfSize - 1.0f;
      csVector3 norm = csVector3 (
        xo + xv*xx + yv*xy, 
        yo + xv*yx + yv*yy, 
        zo + xv*zx + yv*zy);
      norm.Normalize ();
      *normdata++ = (unsigned char)(127.5f + norm.x*127.5f);
      *normdata++ = (unsigned char)(127.5f + norm.y*127.5f);
      *normdata++ = (unsigned char)(127.5f + norm.z*127.5f);
      *normdata++ = 0;
    }
  }
}

csNormalizationCubeAccessor::csNormalizationCubeAccessor (
  iTextureManager* txtmgr, int sideSize)
  : scfImplementationType (this), normalizeCubeSize (sideSize), txtmgr (txtmgr)
{
}

csNormalizationCubeAccessor::~csNormalizationCubeAccessor ()
{
}

void csNormalizationCubeAccessor::PreGetValue (csShaderVariable *variable)
{
  if (!texture.IsValid ())
  {
    if (txtmgr.IsValid ())
    {
      csRef<csImageCubeMapMaker> cubeMaker;
      cubeMaker.AttachNew (new csImageCubeMapMaker ());
      cubeMaker->SetName (0);

      // Positive X
      unsigned char *normdata = 
	new unsigned char[normalizeCubeSize*normalizeCubeSize*4];
      FillNormalizationMapSide (normdata, normalizeCubeSize,  0,  0,  1,
							      0, -1,  0,
							    -1,  0,  0);
      csRef<iImage> img = csPtr<iImage> (new csImageMemory (
	normalizeCubeSize, normalizeCubeSize, normdata, true,
	CS_IMGFMT_TRUECOLOR));
      cubeMaker->SetSubImage (0, img);

      // Negative X
      normdata = new unsigned char[normalizeCubeSize*normalizeCubeSize*4];
      FillNormalizationMapSide (normdata, normalizeCubeSize,  0,  0, -1,
							      0, -1,  0,
							      1,  0,  0);
      img = csPtr<iImage> (new csImageMemory (
	normalizeCubeSize, normalizeCubeSize, normdata, true, 
	CS_IMGFMT_TRUECOLOR));
      cubeMaker->SetSubImage (1, img);

      // Positive Y
      normdata = new unsigned char[normalizeCubeSize*normalizeCubeSize*4];
      FillNormalizationMapSide (normdata, normalizeCubeSize,  1,  0,  0,
							      0,  0,  1,
							      0,  1,  0);
      img = csPtr<iImage> (new csImageMemory (
	normalizeCubeSize, normalizeCubeSize, normdata, true, 
	CS_IMGFMT_TRUECOLOR));
      cubeMaker->SetSubImage (2, img);

      // Negative Y
      normdata = new unsigned char[normalizeCubeSize*normalizeCubeSize*4];
      FillNormalizationMapSide (normdata, normalizeCubeSize,  1,  0,  0,
							      0,  0, -1,
							      0, -1,  0);
      img = csPtr<iImage> (new csImageMemory (
	normalizeCubeSize, normalizeCubeSize, normdata, true, 
	CS_IMGFMT_TRUECOLOR));
      cubeMaker->SetSubImage (3, img);

      // Positive Z
      normdata = new unsigned char[normalizeCubeSize*normalizeCubeSize*4];
      FillNormalizationMapSide (normdata, normalizeCubeSize,  1,  0,  0,
							      0, -1,  0,
							      0,  0,  1);
      img = csPtr<iImage> (new csImageMemory (
	normalizeCubeSize, normalizeCubeSize, normdata, true, 
	CS_IMGFMT_TRUECOLOR));
      cubeMaker->SetSubImage (4, img);

      // Negative Z
      normdata = new unsigned char[normalizeCubeSize*normalizeCubeSize*4];
      FillNormalizationMapSide (normdata, normalizeCubeSize, -1,  0,  0,
							      0, -1,  0,
							      0,  0, -1);
      img = csPtr<iImage> (new csImageMemory (
	normalizeCubeSize, normalizeCubeSize, normdata, true, 
	CS_IMGFMT_TRUECOLOR));
      cubeMaker->SetSubImage (5, img);

      texture = txtmgr->RegisterTexture (
	cubeMaker, CS_TEXTURE_3D | CS_TEXTURE_CLAMP | CS_TEXTURE_NOMIPMAPS);
      texture->SetTextureClass ("lookup");
      texture->Precache ();
    }
  }
  variable->SetValue (texture);
}
