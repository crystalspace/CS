/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Contributions made by Ivan Avramovic <ivan@avramovic.com>

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
#include "imgplex.h"
#include "isys/system.h"
#include "iutil/strvec.h"
#include "csgfx/csimage.h"

#define MY_CLASSNAME "crystalspace.graphic.image.io.multiplex"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE(csMultiplexImageIO);
  SCF_IMPLEMENTS_INTERFACE(iImageIO);
  SCF_IMPLEMENTS_INTERFACE(iPlugIn);
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_FACTORY(csMultiplexImageIO);

SCF_EXPORT_CLASS_TABLE (imgplex)
  SCF_EXPORT_CLASS (csMultiplexImageIO,
    MY_CLASSNAME, "Image file format multiplex plug-in.")
SCF_EXPORT_CLASS_TABLE_END

csMultiplexImageIO::csMultiplexImageIO (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
}

csMultiplexImageIO::~csMultiplexImageIO ()
{
  for (int i=0; i < list.Length (); i++)
    ((iImageIO*)list.Get (i))->DecRef ();
}

bool csMultiplexImageIO::Initialize (iSystem *pSystem)
{
  if (pSystem)
  {
    pSystem->Printf(CS_MSG_INITIALIZATION,
      "Initializing image loading multiplexer...\n"
      "  Looking for image loader modules:\n");

    iStrVector* classlist =
      iSCF::SCF->QueryClassList ("crystalspace.graphic.image.io.");
    int const nmatches = classlist->Length();
    if (nmatches != 0)
    {
      for (int i = 0; i < nmatches; i++)
      {
	char const* classname = classlist->Get(i);
        if (strcasecmp (classname, MY_CLASSNAME))
        {
	  pSystem->Printf(CS_MSG_INITIALIZATION,"  %s\n",classname);
	  iImageIO *plugin = CS_LOAD_PLUGIN (pSystem, classname, NULL, iImageIO);
	  if (plugin)
	  {
	    // remember the plugin
	    list.Push (plugin);
	    // and load its description, since we gonna return it on request
	    StoreDesc (plugin->GetDescription ());
	  }
        }
      }
    }
    classlist->DecRef();
    return (list.Length() > 0);
  }
  return false;
}

void csMultiplexImageIO::StoreDesc (const csVector& format)
{
  // add the formats coming in to our ever growing list
  int i;
  for (i=0; i < format.Length (); i++)
    formats.Push ((csSome)format.Get (i));
}

const csVector& csMultiplexImageIO::GetDescription ()
{
  return formats;
}

iImage *csMultiplexImageIO::Load (UByte* iBuffer, ULong iSize, int iFormat)
{
  int i;
  for (i=0; i<list.Length(); i++)
  {
    iImageIO *pIO = (iImageIO*)list.Get(i);
    iImage *img = pIO->Load(iBuffer, iSize, iFormat);
    if (img) return img;
  }
  return NULL;
}

/**
 * Set global image dithering option.<p>
 * By default this option is disabled. If you enable it, all images will
 * be dithered both after loading and after mipmapping/scaling. This will
 * affect all truecolor->paletted image conversions.
 */
void csMultiplexImageIO::SetDithering (bool iEnable)
{
  extern bool csImage_dither;
  csImage_dither = iEnable;
}

iDataBuffer *csMultiplexImageIO::Save (
  iImage *image, iImageIO::FileFormatDescription *format)
{
  int i;
  for (i=0; i<list.Length(); i++)
  {
    iImageIO *pIO = (iImageIO*)list.Get(i);
    iDataBuffer *buf = pIO->Save(image, format);
    if (buf) return buf;
  }
  return NULL;
}

iDataBuffer *csMultiplexImageIO::Save (iImage *image, const char *mime)
{
  int i;
  for (i=0; i<list.Length(); i++)
  {
    iImageIO *pIO = (iImageIO*)list.Get(i);
    iDataBuffer *buf = pIO->Save(image, mime);
    if (buf) return buf;
  }
  return NULL;
}
