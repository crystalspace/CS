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
#include "iutil/strvec.h"
#include "csgfx/csimage.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "csutil/util.h"

#define MY_CLASSNAME "crystalspace.graphic.image.io.multiplex"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE(csMultiplexImageIO);
  SCF_IMPLEMENTS_INTERFACE(iImageIO);
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent);
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (csMultiplexImageIO::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY(csMultiplexImageIO);

SCF_EXPORT_CLASS_TABLE (imgplex)
  SCF_EXPORT_CLASS (csMultiplexImageIO,
    MY_CLASSNAME, "Image file format multiplex plug-in.")
SCF_EXPORT_CLASS_TABLE_END

csMultiplexImageIO::csMultiplexImageIO (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);

  global_dither = false;
}

csMultiplexImageIO::~csMultiplexImageIO ()
{
  if (classlist) classlist->DeleteAll ();
  classlist = NULL;
  plugin_mgr = NULL;
}

bool csMultiplexImageIO::Initialize (iObjectRegistry *object_reg)
{
  if (object_reg)
  {
    plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);

    // query registry for image plugins...
    classlist = csPtr<iStrVector> (
      iSCF::SCF->QueryClassList ("crystalspace.graphic.image.io."));
    // but don't load them yet

    return true;
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

bool csMultiplexImageIO::LoadNextPlugin ()
{
  csRef<iImageIO> plugin;
  while (classlist && !plugin)
  {
    char const* classname = NULL;
    do
    {
      if (classname) classlist->Delete (0);
      if (classlist->Length() == 0)
      {
	classlist = NULL;
	plugin_mgr = NULL;
	return false;
      }
      classname = classlist->Get(0);
    } while (!strcasecmp (classname, MY_CLASSNAME));
    
    plugin = CS_LOAD_PLUGIN (plugin_mgr, classname, iImageIO);
    if (plugin)
    {
      plugin->SetDithering (global_dither);
      // remember the plugin
      list.Push (plugin);
      // and load its description, since we gonna return it on request
      StoreDesc (plugin->GetDescription ());
    }
    classlist->Delete (0);
  }
  return true;
}

const csVector& csMultiplexImageIO::GetDescription ()
{
  // need all plugins.
  while (LoadNextPlugin()); 
  return formats;
}

void csMultiplexImageIO::SetDithering (bool iEnable)
{
  global_dither = iEnable;
  for (int i = 0; i < list.Length (); i++)
    list[i]->SetDithering (global_dither);
}

csPtr<iImage> csMultiplexImageIO::Load (uint8* iBuffer, uint32 iSize, int iFormat)
{
  bool consecutive = false; // set to true if we searched the list completely.
  do
  {
    int i;
    for (i=list.Length(); (i--)>0; ) 
      // i is decremented after comparison but before we use it below;
      //  hence it goes from list.Length()-1 to 0
    {
      csRef<iImageIO> pIO = (iImageIO*)list.Get(i);
      csRef<iImage> img (pIO->Load(iBuffer, iSize, iFormat));
      if (img)
      {
	/*
	  move used plugin to the bottom of the list.
	  the idea is that some formats are used more
	  commonly than other formats and that those
	  plugins are asked first. 
	 */
	if ((list.Length()-i) > 4)
	  // keep a 'top 4'; no need to shuffle the list
	  // when a plugin is already one of the first asked
	{
	  list.Push (pIO);
	  list.Delete (i);
	}
	return csPtr<iImage> (img);
      }
      // if we just loaded a plugin only check that.
      if (consecutive) break;
    }
    consecutive = true;
  } while (LoadNextPlugin());
  return NULL;
}

csPtr<iDataBuffer> csMultiplexImageIO::Save (
  iImage *image, iImageIO::FileFormatDescription *format,
  const char* extraoptions)
{
  // same algortihm as in Load()
  bool consecutive = false; 
  do
  {
    int i;
    for (i=list.Length(); (i--)>0; ) 
    {
      csRef<iImageIO> pIO = (iImageIO*)list.Get(i);
      csRef<iDataBuffer> buf (pIO->Save(image, format, extraoptions));
      if (buf)
      {
	if ((list.Length()-i) > 4)
	{
	  list.Push (pIO);
	  list.Delete (i);
	}
	return csPtr<iDataBuffer> (buf);
      }
      // if we just loaded a plugin only check that.
      if (consecutive) break;
    }
    consecutive = true;
  } while (LoadNextPlugin());
  return NULL;
}

csPtr<iDataBuffer> csMultiplexImageIO::Save (iImage *image, const char *mime,
  const char* extraoptions)
{
  // same algortihm as in Load()
  bool consecutive = false; 
  do
  {
    int i;
    for (i=list.Length(); (i--)>0; ) 
    {
      csRef<iImageIO> pIO = (iImageIO*)list.Get(i);
      csRef<iDataBuffer> buf (pIO->Save(image, mime, extraoptions));
      if (buf)
      {
	if ((list.Length()-i) > 4)
	{
	  list.Push (pIO);
	  list.Delete (i);
	}
	return csPtr<iDataBuffer> (buf);
      }
      // if we just loaded a plugin only check that.
      if (consecutive) break;
    }
    consecutive = true;
  } while (LoadNextPlugin());
  return NULL;
}
