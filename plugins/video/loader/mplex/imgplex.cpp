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
#include "iutil/plugin.h"
#include "iutil/stringarray.h"
#include "csgfx/csimage.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"

#define IMGPLEX_CLASSNAME "crystalspace.graphic.image.io.multiplex"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE(csMultiplexImageIO);
  SCF_IMPLEMENTS_INTERFACE(iImageIO);
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent);
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (csMultiplexImageIO::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY(csMultiplexImageIO);


csMultiplexImageIO::csMultiplexImageIO (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csMultiplexImageIO::~csMultiplexImageIO ()
{
  for (int i=0; i < list.Length (); i++)
    ((iImageIO*)list.Get (i))->DecRef ();
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

bool csMultiplexImageIO::Initialize (iObjectRegistry *object_reg)
{
  if (object_reg)
  {
    csRef<iPluginManager> plugin_mgr (
    	CS_QUERY_REGISTRY (object_reg, iPluginManager));

    csRef<iStringArray> classlist =
      iSCF::SCF->QueryClassList ("crystalspace.graphic.image.io.");
    int const nmatches = classlist.IsValid() ? classlist->Length() : 0;
    if (nmatches != 0)
    {
      int i;
      for (i = 0; i < nmatches; i++)
      {
	char const* classname = classlist->Get(i);
        if (strcasecmp (classname, IMGPLEX_CLASSNAME))
        {
	  csRef<iImageIO> plugin (
	  	CS_LOAD_PLUGIN (plugin_mgr, classname, iImageIO));
	  if (plugin)
	  {
	    // remember the plugin
	    list.Push (plugin);
	    // and load its description, since we gonna return it on request
	    StoreDesc (plugin->GetDescription ());
	    plugin->IncRef ();	// Avoid smart pointer release.
	  }
        }
      }
    }
    return true;
  }
  return false;
}

void csMultiplexImageIO::StoreDesc (
	const csImageIOFileFormatDescriptions& format)
{
  // add the formats coming in to our ever growing list
  int i;
  for (i=0; i < format.Length (); i++)
    formats.Push (format[i]);
}

const csImageIOFileFormatDescriptions& csMultiplexImageIO::GetDescription ()
{
  return formats;
}

csPtr<iImage> csMultiplexImageIO::Load (uint8* iBuffer, uint32 iSize,
  int iFormat)
{
  int i;
  for (i=0; i<list.Length(); i++)
  {
    iImageIO *pIO = (iImageIO*)list.Get(i);
    csRef<iImage> img (pIO->Load(iBuffer, iSize, iFormat));
    if (img)
      return csPtr<iImage> (img);
  }
  return 0;
}

/**
 * Set global image dithering option.<p>
 * By default this option is disabled. If you enable it, all images will
 * be dithered both after loading and after mipmapping/scaling. This will
 * affect all truecolor->paletted image conversions.
 */
void csMultiplexImageIO::SetDithering (bool iEnable)
{
  //extern bool csImage_dither;
  //csImage_dither = iEnable;
}

csPtr<iDataBuffer> csMultiplexImageIO::Save (
  iImage *image, iImageIO::FileFormatDescription *format,
  const char* extraoptions)
{
  int i;
  for (i=0; i<list.Length(); i++)
  {
    iImageIO *pIO = (iImageIO*)list.Get(i);
    csRef<iDataBuffer> buf (pIO->Save(image, format, extraoptions));
    if (buf)
      return csPtr<iDataBuffer> (buf);
  }
  return 0;
}

csPtr<iDataBuffer> csMultiplexImageIO::Save (iImage *image, const char *mime,
  const char* extraoptions)
{
  int i;
  for (i=0; i<list.Length(); i++)
  {
    iImageIO *pIO = (iImageIO*)list.Get(i);
    csRef<iDataBuffer> buf (pIO->Save(image, mime, extraoptions));
    if (buf)
      return csPtr<iDataBuffer> (buf);
  }
  return 0;
}
