/*
    DSS image file format support for CrystalSpace 3D library
    Copyright (C) 2003 by Matze Braun <matze@braunis.de>

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

#include "ivaria/reporter.h"

#include "dds.h"
#include "ddsloader.h"

#define DDS_MIME "image/dds"

static iImageIO::FileFormatDescription formatlist[1] =
{
  {DDS_MIME, "RGBA", CS_IMAGEIO_LOAD}
};

csDDSImageIO::csDDSImageIO (iBase* parent)
{
  SCF_CONSTRUCT_IBASE(parent);
  int const formatcount =
    sizeof(formatlist)/sizeof(iImageIO::FileFormatDescription);
  for (int i=0; i < formatcount; i++)
    formats.Push (&formatlist[i]);
}

csDDSImageIO::~csDDSImageIO ()
{
  SCF_DESTRUCT_IBASE();
}

bool csDDSImageIO::Initialize (iObjectRegistry* objreg)
{
  object_reg = objreg;
  return true;
}

const csImageIOFileFormatDescriptions& csDDSImageIO::GetDescription ()
{
  return formats;
}

void csDDSImageIO::SetDithering (bool)
{
}

csPtr<iImage> csDDSImageIO::Load (uint8* buffer, size_t size, int format)
{
  dds::Loader* loader = new dds::Loader;

  loader->SetSource (buffer, size);
  if (!loader->IsDDS())                       
  {
    delete loader;
    return 0;
  }

  csDDSImageFile* image = new csDDSImageFile (object_reg, format);
  if (!image->Load (loader))
  {
    delete loader;
    return 0;
  }

  delete loader;
  return csPtr<iImage> (image);
}

csPtr<iDataBuffer> csDDSImageIO::Save (iImage* image,
    iImageIO::FileFormatDescription* format, const char* options)
{
  return 0;
}

csPtr<iDataBuffer> csDDSImageIO::Save (iImage* image, const char* mime,
				       const char* options)
{
  return 0;
}

//---------------------------------------------------------------------------

csDDSImageFile::csDDSImageFile (iObjectRegistry* object_reg, int format)
  : csImageFile (format), mipmaps(0), mipmapcount(0)
{
  csDDSImageFile::object_reg = object_reg;
}

csDDSImageFile::~csDDSImageFile ()
{
}

bool csDDSImageFile::Load (dds::Loader* loader)
{
  set_dimensions (loader->GetWidth(), loader->GetHeight());
  /*if (loader->GetBytesPerPixel() != 4)
  {
    Report (CS_REPORTER_SEVERITY_WARNING, 
      "DDS loader only supports 32 bit images at the moment.");
    return false;
  }*/
  csRGBpixel* img = loader->LoadImage ();
  if (!img)
    return false;
  convert_rgba (img);

  mipmapcount = loader->GetMipmapCount () - 1;
  for (int i=0;i<mipmapcount;i++)
  {
    csRGBpixel* img = loader->LoadMipmap(i);
    if (!img)
      return false;
    csDDSImageFile* image = new csDDSImageFile (object_reg, Format);
    int newW = loader->GetWidth() >> (i+1);
    newW = MAX(newW, 1);
    int newH = loader->GetHeight() >> (i+1);
    newH = MAX(newH, 1);
    image->set_dimensions (newW, newH);
    image->convert_rgba(img);
    mipmaps.Push (image);
  }

  return true;
}

csPtr<iImage> csDDSImageFile::MipMap (int step, csRGBpixel* transp)
{
  if (step == 0 || step > mipmapcount || transp)
    return csImageFile::MipMap (step, transp);
  return csPtr<iImage> (mipmaps[step-1]);
}

int csDDSImageFile::HasMipmaps ()
{
  return mipmapcount;
}

void csDDSImageFile::Report (int severity, const char* msg, ...)
{
  va_list argv;
  va_start (argv, msg);
  csReportV (object_reg, severity, "crystalspace.graphic.image.io.dds", msg, 
    argv);
  va_end (argv);
}

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csDDSImageIO)
  SCF_IMPLEMENTS_INTERFACE (iImageIO)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY(csDDSImageIO);
