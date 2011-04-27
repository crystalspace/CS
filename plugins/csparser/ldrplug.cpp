/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein
    Copyright (C) 1998-2000 by Ivan Avramovic <ivan@avramovic.com>

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

#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"

#include "ldrplug.h"

using namespace CS::Threading;

CS_PLUGIN_NAMESPACE_BEGIN(csparser)
{
  csLoadedPluginVector::csLoadedPluginVector ()
  {
    plugin_mgr = 0;
  }

  csLoadedPluginVector::~csLoadedPluginVector ()
  {
    DeleteAll ();
  }

  void csLoadedPluginVector::DeleteAll ()
  {
    ScopedWriteLock lock(mutex);
    size_t i;
    for (i = 0 ; i < vector.GetSize () ; i++)
    {
      csLoaderPluginRec* rec = vector[i];
      if (rec->Component && plugin_mgr)
      {
        csRef<iComponent> comp = scfQueryInterface<iComponent> (rec->Component);
        if (comp)
          plugin_mgr->UnloadPluginInstance (comp);
      }
      delete rec;
    }
    vector.DeleteAll ();
  }

  csLoaderPluginRec* csLoadedPluginVector::FindPluginRec (
    const char* name)
  {
    ScopedReadLock lock(mutex);
    size_t i;
    for (i=0 ; i<vector.GetSize () ; i++)
    {
      csLoaderPluginRec* pl = vector.Get (i);
      if ((!pl->ShortName.IsEmpty ()) && !strcmp (name, pl->ShortName))
        return pl;
      if (!strcmp (name, pl->ClassID))
        return pl;
    }
    return 0;
  }

  bool csLoadedPluginVector::GetPluginFromRec (
    csLoaderPluginRec *rec, iLoaderPlugin*& plug,
    iBinaryLoaderPlugin*& binplug)
  {
    if (!rec->Component)
    {
      ScopedWriteLock lock(mutex);
      if (!rec->Component)
      {
        rec->Component = csQueryRegistryTag (object_reg, rec->ClassID);
        if (!rec->Component)
        {
          csRef<iComponent> comp = csLoadPluginCheck<iComponent> (plugin_mgr,
            rec->ClassID);
          rec->Component = comp;
        }
        if (rec->Component)
        {
          rec->Plugin = scfQueryInterface<iLoaderPlugin> (rec->Component);
          rec->BinPlugin = 
            scfQueryInterface<iBinaryLoaderPlugin> (rec->Component);
        }
      }
    }
    plug = rec->Plugin;
    binplug = rec->BinPlugin;
    return rec->Component != 0;
  }

  bool csLoadedPluginVector::FindPlugin (
    const char* Name, iLoaderPlugin*& plug,
    iBinaryLoaderPlugin*& binplug, iDocumentNode*& defaults)
  {
    // look if there is already a loading record for this plugin
    csLoaderPluginRec* pl = FindPluginRec (Name);
    if (pl)
    {
      defaults = pl->defaults;
      return GetPluginFromRec (pl, plug, binplug);
    }

    // create a new loading record
    {
      ScopedWriteLock lock(mutex);
      vector.Push (new csLoaderPluginRec (0, Name, 0, 0, 0));
    }

    defaults = 0;
    return GetPluginFromRec (vector.Get(vector.GetSize ()-1),
      plug, binplug);
  }

  const char* csLoadedPluginVector::FindPluginClassID (const char* Name)
  {
    // look if there is already a loading record for this plugin
    csLoaderPluginRec* pl = FindPluginRec (Name);
    if (pl)
      return pl->ClassID;

    return 0;
  }


  void csLoadedPluginVector::NewPlugin
    (const char *ShortName, iDocumentNode* child)
  {
    csRef<iDocumentNode> id = child->GetNode ("id");
    csRef<iDocumentNode> defaults = child->GetNode ("defaults");

    csLoaderPluginRec* pl = FindPluginRec (ShortName);
    if (pl)
    {
      // There is already an entry with this name. We check if it has
      // the same class id. If not we replace it anyway.
      csRef<iDocumentNode> defaults = child->GetNode ("defaults");
      pl->SetDefaults (defaults);
      if (id)
      {
        const char* ClassID = id->GetContentsValue ();
        if (pl->ClassID != ClassID)
        {
          ScopedWriteLock lock(mutex);
          vector.Delete (pl);
          pl = new csLoaderPluginRec (ShortName, ClassID, 0, 0, 0);
          vector.Push (pl);
        }
        pl->SetDefaults (defaults);
      }
      else
      {
        const char* ClassID = child->GetContentsValue ();
        if (pl->ClassID != ClassID)
        {
          ScopedWriteLock lock(mutex);
          vector.Delete (pl);
          vector.Push (new csLoaderPluginRec (ShortName, ClassID, 0, 0, 0));
        }
      }
    }
    else
    {
      if (id)
      {
        const char* ClassID = id->GetContentsValue ();
        csLoaderPluginRec* pr = new csLoaderPluginRec (ShortName, ClassID, 0, 0, 0);
        csRef<iDocumentNode> defaults = child->GetNode ("defaults");
        pr->SetDefaults (defaults);
        ScopedWriteLock lock(mutex);
        vector.Push (pr);
      }
      else
      {
        const char* ClassID = child->GetContentsValue ();
        ScopedWriteLock lock(mutex);
        vector.Push (new csLoaderPluginRec (ShortName, ClassID, 0, 0, 0));
      }
    }
  }
}
CS_PLUGIN_NAMESPACE_END(csparser)
