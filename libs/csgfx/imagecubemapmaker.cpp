/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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
#include "csutil/csstring.h"
#include "csgfx/imagecubemapmaker.h"
#include "csgfx/xorpat.h"

CS_LEAKGUARD_IMPLEMENT (csImageCubeMapMaker);

SCF_IMPLEMENT_IBASE (csImageCubeMapMaker)
  SCF_IMPLEMENTS_INTERFACE (iImage)
SCF_IMPLEMENT_IBASE_END

csImageCubeMapMaker::csImageCubeMapMaker() : csImageBase(), manualName (false)
{
  SCF_CONSTRUCT_IBASE(0);
}

csImageCubeMapMaker::csImageCubeMapMaker (iImage* source) 
  : csImageBase(), manualName (false)
{
  SCF_CONSTRUCT_IBASE(0);

  if (source != 0)
  {
    for (uint i = 0; i < source->HasSubImages() + 1; i++)
      cubeImages[i] = source->GetSubImage (i);
  }
  UpdateName();
}

csImageCubeMapMaker::csImageCubeMapMaker (iImage* posX, iImage* negX, 
					  iImage* posY, iImage* negY, 
					  iImage* posZ, iImage* negZ) 
  : csImageBase(), manualName (false)
{
  SCF_CONSTRUCT_IBASE(0);

  cubeImages[0] = posX;
  cubeImages[1] = negX;
  cubeImages[2] = posY;
  cubeImages[3] = negY;
  cubeImages[4] = posZ;
  cubeImages[5] = negZ;
  UpdateName();
}

csImageCubeMapMaker::~csImageCubeMapMaker ()
{
  SCF_DESTRUCT_IBASE();
}

void csImageCubeMapMaker::CheckImage (int index)
{
  if (cubeImages[index] == 0)
    cubeImages[index] = csCreateXORPatternImage (128, 128, 7);
}

void csImageCubeMapMaker::UpdateName ()
{
  csString newName;
  newName.Format ("%s:%s:%s:%s:%s:%s",
    cubeImages[0].IsValid() ? cubeImages[0]->GetName() : "",
    cubeImages[1].IsValid() ? cubeImages[1]->GetName() : "",
    cubeImages[2].IsValid() ? cubeImages[2]->GetName() : "",
    cubeImages[3].IsValid() ? cubeImages[3]->GetName() : "",
    cubeImages[4].IsValid() ? cubeImages[4]->GetName() : "",
    cubeImages[5].IsValid() ? cubeImages[5]->GetName() : "");
  delete[] fName;
  fName = csStrNew (newName);
}

const void* csImageCubeMapMaker::GetImageData ()
{
  CheckImage (0);
  return cubeImages[0]->GetImageData();
}

int csImageCubeMapMaker::GetWidth () const
{
  return cubeImages[0].IsValid() ? cubeImages[0]->GetWidth() : 128;
}

int csImageCubeMapMaker::GetHeight () const
{
  return cubeImages[0].IsValid() ? cubeImages[0]->GetHeight() : 128;
}

void csImageCubeMapMaker::SetName (const char *iName)
{
  delete[] fName;
  fName = csStrNew (iName);
  manualName = true;
}

int csImageCubeMapMaker::GetFormat () const
{
  return cubeImages[0].IsValid() ? 
    cubeImages[0]->GetFormat() : CS_IMGFMT_TRUECOLOR;
}

const csRGBpixel* csImageCubeMapMaker::GetPalette ()
{
  CheckImage (0);
  return cubeImages[0]->GetPalette();
}

const uint8* csImageCubeMapMaker::GetAlpha ()
{
  CheckImage (0);
  return cubeImages[0]->GetAlpha();
}

uint csImageCubeMapMaker::HasMipmaps () const
{
  return cubeImages[0].IsValid() ? cubeImages[0]->HasMipmaps() : 0;
}

csRef<iImage> csImageCubeMapMaker::GetMipmap (uint num)
{
  CheckImage (0);
  return cubeImages[0]->GetMipmap (num);
}

const char* csImageCubeMapMaker::GetRawFormat() const
{
  return cubeImages[0].IsValid() ? 
    cubeImages[0]->GetRawFormat() : 0;
}

csRef<iDataBuffer> csImageCubeMapMaker::GetRawData() const
{
  csRef<iDataBuffer> d;
  if (cubeImages[0].IsValid())
    d = cubeImages[0]->GetRawData();
  return d;
}

csRef<iImage> csImageCubeMapMaker::GetSubImage (uint num)
{
  if (num == 0)
    return this;
  else if ((num > 0) && (num < NUM_FACES))
  {
    CheckImage (num);
    return cubeImages[num];
  }
  else
    return 0;
}

void csImageCubeMapMaker::SetSubImage (uint num, iImage* image)
{
  if ((num >= 0) && (num < NUM_FACES))
  {
    cubeImages[num] = image;
    if (!manualName) UpdateName();
  }
}
