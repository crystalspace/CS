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

#include "csutil/util.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/stringarray.h"
#include "ivaria/reporter.h"
#include "igraphic/image.h"

#include "imgplex.h"

#define IMGPLEX_CLASSNAME "crystalspace.graphic.image.io.multiplexer"

CS_PLUGIN_NAMESPACE_BEGIN(ImgPlex)
{

SCF_IMPLEMENT_FACTORY(csImageIOMultiplexer)

csImageIOMultiplexer::csImageIOMultiplexer (iBase *pParent) :
  scfImplementationType (this, pParent)
{
}

csImageIOMultiplexer::~csImageIOMultiplexer ()
{
  if (classlist) classlist->Empty ();
}

bool csImageIOMultiplexer::Initialize (iObjectRegistry *object_reg)
{
  if (object_reg)
  {
    plugin_mgr = csQueryRegistry<iPluginManager> (object_reg);

    // query registry for image plugins...
    classlist = csPtr<iStringArray> (
      iSCF::SCF->QueryClassList ("crystalspace.graphic.image.io."));
    // but don't load them yet

    return true;
  }
  return false;
}

void csImageIOMultiplexer::StoreDesc (
	const csImageIOFileFormatDescriptions& format)
{
  // add the formats coming in to our ever growing list
  size_t i;
  for (i = 0; i < format.GetSize (); i++)
    formats.Push (format[i]);
}

bool csImageIOMultiplexer::LoadNextPlugin ()
{
  if (!classlist) return false;
  
  csRef<iImageIO> plugin;
  while (classlist && !plugin)
  {
    char const* classname = 0;
    do
    {
      if (classname) classlist->DeleteIndex (0);
      if (classlist->GetSize () == 0)
      {
	classlist = 0;
	plugin_mgr = 0;
	return false;
      }
      classname = classlist->Get(0);
    } while (!strcasecmp (classname, IMGPLEX_CLASSNAME));
    
    plugin = csLoadPlugin<iImageIO> (plugin_mgr, classname);
    if (plugin)
    {
      // remember the plugin
      list.Push (plugin);
      // and load its description, since we gonna return it on request
      StoreDesc (plugin->GetDescription ());
    }
    classlist->DeleteIndex (0);
  }
  return true;
}

const csImageIOFileFormatDescriptions& csImageIOMultiplexer::GetDescription ()
{
  // need all plugins.
  while (LoadNextPlugin()); 
  return formats;
}

csPtr<iImage> csImageIOMultiplexer::Load (iDataBuffer* buf, int iFormat)
{
  CS::Threading::MutexScopedLock slock(lock);
  bool consecutive = false; // set to true if we searched the list completely.
  do
  {
    size_t i = list.GetSize ();
    while (i-- > 0)
      // i is decremented after comparison but before we use it below;
      //  hence it goes from list.GetSize ()-1 to 0
    {
      csRef<iImageIO> pIO = (iImageIO*)list.Get(i);
      csRef<iImage> img (pIO->Load(buf, iFormat));
      if (img)
      {
	/*
	  move used plugin to the bottom of the list.
	  the idea is that some formats are used more
	  commonly than other formats and that those
	  plugins are asked first. 
	 */
	if ((list.GetSize ()-i) > 4)
	  // keep a 'top 4'; no need to shuffle the list
	  // when a plugin is already one of the first asked
	{
	  list.Push (pIO);
	  list.DeleteIndex (i);
	}
	return csPtr<iImage> (img);
      }
      // if we just loaded a plugin only check that.
      if (consecutive) break;
    }
    consecutive = true;
  } while (LoadNextPlugin());
  return 0;
}

csPtr<iDataBuffer> csImageIOMultiplexer::Save (
  iImage *image, iImageIO::FileFormatDescription *format,
  const char* extraoptions)
{
  // same algortihm as in Load()
  bool consecutive = false; 
  do
  {
    size_t i = list.GetSize ();
    while (i-- > 0)
    {
      csRef<iImageIO> pIO = (iImageIO*)list.Get(i);
      csRef<iDataBuffer> buf (pIO->Save(image, format, extraoptions));
      if (buf)
      {
	if ((list.GetSize ()-i) > 4)
	{
	  list.Push (pIO);
	  list.DeleteIndex (i);
	}
	return csPtr<iDataBuffer> (buf);
      }
      // if we just loaded a plugin only check that.
      if (consecutive) break;
    }
    consecutive = true;
  } while (LoadNextPlugin());
  return 0;
}

csPtr<iDataBuffer> csImageIOMultiplexer::Save (iImage *image, const char *mime,
  const char* extraoptions)
{
  // same algortihm as in Load()
  bool consecutive = false; 
  do
  {
    size_t i = list.GetSize ();
    while (i-- > 0)
    {
      csRef<iImageIO> pIO = (iImageIO*)list.Get(i);
      csRef<iDataBuffer> buf (pIO->Save(image, mime, extraoptions));
      if (buf)
      {
	if ((list.GetSize ()-i) > 4)
	{
	  list.Push (pIO);
	  list.DeleteIndex (i);
	}
	return csPtr<iDataBuffer> (buf);
      }
      // if we just loaded a plugin only check that.
      if (consecutive) break;
    }
    consecutive = true;
  } while (LoadNextPlugin());
  return 0;
}

}
CS_PLUGIN_NAMESPACE_END(ImgPlex)
