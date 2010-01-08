/*
    Copyright (C) 2008 by Frank Richter

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

#include "iutil/stringarray.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"

#include "csutil/csstring.h"
#include "csutil/rootedhiercache.h"
#include "csutil/scfstringarray.h"
#include "csutil/set.h"

#include "plexhiercache.h"

CS_PLUGIN_NAMESPACE_BEGIN(ShaderManager)
{
  PlexHierarchicalCache::PlexHierarchicalCache ()
   : scfImplementationType (this)
  {
  }

  void PlexHierarchicalCache::AddSubShaderCache (iHierarchicalCache* cache,
                                                 int priority)
  {
    size_t insertBefore = 0;
    for (; insertBefore < caches.GetSize(); insertBefore++)
    {
      if (caches[insertBefore].priority < priority) break;
    }
    SubCache subCache;
    subCache.cache = cache;
    subCache.priority = priority;
    caches.Insert (insertBefore, subCache);
  }

  void PlexHierarchicalCache::RemoveSubShaderCache (iHierarchicalCache* cache)
  {
    size_t index = caches.FindKey (
      csArrayCmp<SubCache, iHierarchicalCache*> (cache, CacheSort));
    if (index != csArrayItemNotFound)
      caches.DeleteIndex (index);
  }
  
  void PlexHierarchicalCache::RemoveAllSubShaderCaches ()
  {
    caches.DeleteAll ();
  }

  bool PlexHierarchicalCache::CacheData (const void* data, size_t size,
				  	 const char* path)
  {
    for (size_t i = 0; i < caches.GetSize(); i++)
    {
      if (caches[i].cache->CacheData (data, size, path))
        return true;
    }
    
    return false;
  }
  
  csPtr<iDataBuffer> PlexHierarchicalCache::ReadCache (const char* path)
  {
    csRef<iDataBuffer> ret;
    for (size_t i = 0; i < caches.GetSize(); i++)
    {
      ret = caches[i].cache->ReadCache (path);
      if (ret.IsValid()) break;
    }
    
    return csPtr<iDataBuffer> (ret);
  }
  
  bool PlexHierarchicalCache::ClearCache (const char* path)
  {
    bool ret = true;
    for (size_t i = 0; i < caches.GetSize(); i++)
    {
      ret &= caches[i].cache->ClearCache (path);
    }
    
    return ret;
  }
  
  void PlexHierarchicalCache::Flush ()
  {
    for (size_t i = 0; i < caches.GetSize(); i++)
    {
      caches[i].cache->Flush ();
    }
  }
  
  csPtr<iHierarchicalCache> PlexHierarchicalCache::GetRootedCache (const char* base)
  {
    return csPtr<iHierarchicalCache> (
      new CS::Utility::RootedHierarchicalCache (this, base));
  }
  
  csPtr<iStringArray> PlexHierarchicalCache::GetSubItems (const char* path)
  {
    csSet<csString> items;
    for (size_t i = 0; i < caches.GetSize(); i++)
    {
      csRef<iStringArray> cacheItems = caches[i].cache->GetSubItems (path);
      if (!cacheItems.IsValid()) continue;
      for (size_t j = 0; j < cacheItems->GetSize(); j++)
        items.Add (cacheItems->Get (j));
    }
    
    scfStringArray* newArray = new scfStringArray;
    csSet<csString>::GlobalIterator iter (items.GetIterator());
    while (iter.HasNext ())
    {
      const csString& path = iter.Next();
      newArray->Push (path);
    }
    return csPtr<iStringArray> (newArray);
  }
  
  iHierarchicalCache* PlexHierarchicalCache::GetTopCache()
  {
    return this;
  }
}
CS_PLUGIN_NAMESPACE_END(ShaderManager)

