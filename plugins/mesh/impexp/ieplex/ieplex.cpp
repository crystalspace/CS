/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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
#include "iutil/databuff.h"
#include "iutil/objreg.h"
#include "iutil/stringarray.h"
#include "csutil/csstring.h"
#include "csutil/array.h"
#include "csutil/refarr.h"
#include "iutil/plugin.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imesh/mdlconv.h"
#include "imesh/mdldata.h"

#define MDLPLEX_CLASSNAME	"crystalspace.modelconverter.multiplexer"

typedef csArray<csModelConverterFormat const *> csModelConverterFormatVector;

class csModelConverterMultiplexer : public iModelConverter
{
private:
  csRef<iStringArray> classlist;
  csRefArray<iModelConverter> Converters;
  csRef<iPluginManager> plugin_mgr;

  bool LoadNextPlugin ();
public:
  SCF_DECLARE_IBASE;
  csModelConverterFormatVector Formats;

  /// constructor
  csModelConverterMultiplexer (iBase *p);

  /// destructor
  virtual ~csModelConverterMultiplexer ();

  bool Initialize (iObjectRegistry *object_reg);
  virtual size_t GetFormatCount ();
  virtual const csModelConverterFormat *GetFormat (size_t idx);
  virtual csPtr<iModelData> Load (uint8* Buffer, size_t Size);
  virtual csPtr<iDataBuffer> Save (iModelData*, const char *Format);

  struct Component : public iComponent {
    SCF_DECLARE_EMBEDDED_IBASE (csModelConverterMultiplexer);
    virtual bool Initialize (iObjectRegistry *object_reg)
    { return scfParent->Initialize (object_reg); }
  } scfiComponent;
};

SCF_IMPLEMENT_IBASE (csModelConverterMultiplexer)
  SCF_IMPLEMENTS_INTERFACE (iModelConverter)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csModelConverterMultiplexer::Component)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csModelConverterMultiplexer);


CS_IMPLEMENT_PLUGIN

csModelConverterMultiplexer::csModelConverterMultiplexer (iBase *p)
{
  SCF_CONSTRUCT_IBASE (p);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
}

csModelConverterMultiplexer::~csModelConverterMultiplexer ()
{
  plugin_mgr = 0;
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csModelConverterMultiplexer::Initialize (iObjectRegistry *object_reg)
{
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  classlist = iSCF::SCF->QueryClassList ("crystalspace.modelconverter.");
  return true;
}

bool csModelConverterMultiplexer::LoadNextPlugin ()
{
  if (!classlist.IsValid())
    return false;
  
  char const* classname = 0;
  do
  {
    if (classname)
      classlist->DeleteIndex (0);
    if (classlist->Length() == 0)
    {
      classlist = 0;
      plugin_mgr = 0;
      return false;
    }
    classname = classlist->Get(0);
  } while (!strcasecmp (classname, MDLPLEX_CLASSNAME));
  
  csRef<iModelConverter> plugin =
    CS_LOAD_PLUGIN (plugin_mgr, classname, iModelConverter);
  if (plugin.IsValid())
  {
    // remember the plugin
    Converters.Push (plugin);
    // and load its description, since we gonna return it on request
    for (size_t i=0; i<plugin->GetFormatCount (); i++)
      Formats.Push (plugin->GetFormat (i));
  }
  classlist->DeleteIndex (0);
  return true;
}

size_t csModelConverterMultiplexer::GetFormatCount ()
{
  while (LoadNextPlugin ());
  return Formats.Length ();
}

const csModelConverterFormat *csModelConverterMultiplexer::GetFormat (size_t idx)
{
  while (LoadNextPlugin ());
  return Formats.Get (idx);
}

csPtr<iModelData> csModelConverterMultiplexer::Load (uint8* Buffer, size_t Size)
{
  bool consecutive = false; // set to true if we searched the list completely.
  do
  {
    size_t i = Converters.Length();
    while (i-- > 0)
      // i is decremented after comparison but before we use it below;
      //  hence it goes from Converters.Length()-1 to 0
    {
      csRef<iModelConverter> conv = Converters.Get(i);
      csRef<iModelData> mdl (conv->Load (Buffer, Size));
      if (mdl)
      {
	/*
	  move used plugin to the bottom of the list.
	  the idea is that some formats are used more
	  commonly than other formats and that those
	  plugins are asked first. 
	 */
	if ((Converters.Length()-i) > 4)
	  // keep a 'top 4'; no need to shuffle the list
	  // when a plugin is already one of the first asked
	{
	  Converters.Push (conv);
	  Converters.DeleteIndex (i);
	}
	return csPtr<iModelData> (mdl);
      }
      // if we just loaded a plugin only check that.
      if (consecutive) break;
    }
    consecutive = true;
  } while (LoadNextPlugin());
  return 0;
}

csPtr<iDataBuffer> csModelConverterMultiplexer::Save (iModelData *mdl,
	const char *Format)
{
  bool consecutive = false; // set to true if we searched the list completely.
  do
  {
    size_t i = Converters.Length();
    while (i-- > 0)
    {
      csRef<iModelConverter> conv = Converters.Get(i);
      csRef<iDataBuffer> dbuf (conv->Save (mdl, Format));
      if (dbuf)
      {
	if ((Converters.Length()-i) > 4)
	{
	  Converters.Push (conv);
	  Converters.DeleteIndex (i);
	}
	return csPtr<iDataBuffer> (dbuf);
      }
      // if we just loaded a plugin only check that.
      if (consecutive) break;
    }
    consecutive = true;
  } while (LoadNextPlugin());
  return 0;
}
