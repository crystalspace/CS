/*
    Copyright (C) 2009 by Marten Svanfeldt, Mike Gist

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

#include "heightmapproc.h"

CS_IMPLEMENT_APPLICATION

HeightMapProc::HeightMapProc (iObjectRegistry* object_reg) : object_reg(object_reg)
{
  csInitializer::RequestPlugins (object_reg, CS_REQUEST_IMAGELOADER, CS_REQUEST_NULL3D, CS_REQUEST_VFS,
    CS_REQUEST_ENGINE, CS_REQUEST_PLUGIN("crystalspace.level.threadedloader", iThreadedLoader),
    CS_REQUEST_END);

  clp = csQueryRegistry<iCommandLineParser>(object_reg);
  vfs = csQueryRegistry<iVFS>(object_reg);
  engine = csQueryRegistry<iEngine>(object_reg);
  imageio = csQueryRegistry<iImageIO>(object_reg);
  loader = csQueryRegistry<iThreadedLoader>(object_reg);
}

HeightMapProc::~HeightMapProc()
{
}

void HeightMapProc::Run()
{
  if (csCommandLineHelper::CheckHelp(object_reg))
  {
    csCommandLineHelper::Help(object_reg);
    return;
  }

  ReadHeightmap();
  if(image.IsValid())
  {
    ProcessHeightmap();
    WriteHeightmap();
  }
}

void HeightMapProc::ReadHeightmap()
{
  const char* path = clp->GetOption("map");
  if(!path)
  {
    printf("No heightmap passed!\n");
    return;
  }

  csRef<iThreadReturn> ret = loader->LoadImageWait(vfs->GetCwd(), path, CS_IMGFMT_ANY);
  image = scfQueryInterfaceSafe<iImage>(ret->GetResultRefPtr());

  if(!image.IsValid())
  {
    printf("Passed heightmap invalid!\n");
    return;
  }
}

void HeightMapProc::ProcessHeightmap()
{
  width = image->GetWidth();
  height = image->GetHeight();

  // Start with a rescale.
  const char* nW = clp->GetOption("width");
  const char* nH = clp->GetOption("height");

  if(nW && nH && strcmp(nW, nH) == 0)
  {
    int newWidth = atoi(nW);
    int newHeight = atoi(nH);
    if(width != newWidth && height != newHeight)
    {
      width = newWidth;
      height = newHeight;
      image = csImageManipulate::Rescale(image, newWidth, newHeight);
    }
  }

  // Create buffer and load heightmap data.
  heightBuffer = new float[width*height];
  float* heightData = heightBuffer;

  if((image->GetFormat () & CS_IMGFMT_MASK) == CS_IMGFMT_TRUECOLOR)
  {
    const csRGBpixel *data = (csRGBpixel*)image->GetImageData();

    for (int y = 0; y < height; ++y)
    {
      for (int x = 0; x < width; ++x)
      {          
        const csRGBpixel& p = *data++;

        unsigned int h;
        h = p.red; h <<= 8;
        h |= p.green; h <<= 8;
        h |= p.blue;

        *heightData++ = (float)h;
      }
    }
  }
  else if ((image->GetFormat () & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8)
  {
    unsigned char *data = (unsigned char*)image->GetImageData ();
    const csRGBpixel *palette = image->GetPalette ();

    for (int y = 0; y < height; ++y)
    {
      for (int x = 0; x < width; ++x)
      {   
        const unsigned char p = *data++;
        const int h = palette[p].Intensity (); 

        *heightData++ = float(h*0x010101);
      }
    }
  }

  SmoothHeightmap();
}

void HeightMapProc::SmoothHeightmap()
{
  // What should this be?
  size_t pitch = width;

  // We need a temporary buffer for smoothing into
  float* tempBuffer = (float*)cs_malloc(width * height * sizeof(float));

  for (size_t passes = 0; passes < 2; ++passes)
  {
    // Smooth into tempBuffer

    // Copy first and last row
    for (int col = 0; col < width; ++col)
    {
      tempBuffer[0*width + col] = heightBuffer[0*pitch + col];
      tempBuffer[(height - 1)*width + col] = heightBuffer[(height - 1)*pitch + col];
    }

    // Handle interior
    for (int row = 1; row < height - 1; ++row)
    {
      // First column
      tempBuffer[row*width + 0] = heightBuffer[row*pitch + 0];

      // Rest
      for (int col = 1; col < width - 1; ++col)
      {
        float result = 0;

        result = 0.25f * heightBuffer[row*pitch + col - 1] +
          0.50f * heightBuffer[row*pitch + col + 0] +
          0.25f * heightBuffer[row*pitch + col + 1];

        tempBuffer[row*width + col] = result;
      }

      // Last column
      tempBuffer[row*width + width - 1] = heightBuffer[row*pitch + width - 1];
    }

    // Smooth back into the height buffer
    for (int row = 1; row < height - 1; ++row)
    {
      // Interior
      for (int col = 1; col < width - 1; ++col)
      {
        float result = 0;

        result = 0.25f * tempBuffer[(row - 1)*width + col] +
          0.50f * tempBuffer[(row + 0)*width + col] +
          0.25f * tempBuffer[(row + 1)*width + col];

        heightBuffer[row*pitch + col] = result;
      }      
    }
  }

  cs_free (tempBuffer);
}

void HeightMapProc::WriteHeightmap()
{
  csRef<csImageMemory> heightmap_img;
  heightmap_img.AttachNew (new csImageMemory(width, height));
  csRGBpixel* heightPixels = (csRGBpixel*)(heightmap_img->GetImageData ());

  for (int x = 0 ; x < height ; x++)
  {
    for (int z = 0 ; z < width ; z++)
    {
      int h = (int)heightBuffer[x*width + z];
      int r = h & 16711680; r >>= 16;
      int g = h & 65280; g >>= 8;
      int b = h & 255;

      (heightPixels++)->Set (r, g, b);
    }
  }

  csRef<iDataBuffer> db = imageio->Save (heightmap_img, "image/png", "progressive");
  if (db)
  {
    vfs->WriteFile (clp->GetOption("map"), (const char*)db->GetData(), db->GetSize());
  }
}

int main(int argc, char *argv[])
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment(argc, argv);

  HeightMapProc* hmp = new HeightMapProc(object_reg);
  hmp->Run();

  delete hmp;

  csInitializer::DestroyApplication(object_reg);

  return 0;
}
