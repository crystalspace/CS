/*
    Copyright (C) 2011 by Jelle Hellemans

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

#ifndef __CS_CSUTIL_MEMCACHE_H__
#define __CS_CSUTIL_MEMCACHE_H__

#include "csutil/redblacktree.h"
#include "csutil/scf_implementation.h"
#include "imap/resource.h"

namespace CS
{
  namespace Resource
  {
    class MemoryCache : public scfImplementation1<MemoryCache, iResourceCache>
    {
    public:
      MemoryCache () : scfImplementationType (this) {};
      virtual ~MemoryCache () {};

      // iResourceCache
      virtual void Add (CS::Resource::TypeID type, const char* name, iLoadingResource* resource)
      {
        Resources& resources = resourcesHash.GetOrCreate (type);
        resources.Put (name, resource);
      }

      virtual csRef<iLoadingResource> Get (CS::Resource::TypeID type, const char* name)
      {
        Resources& resources = resourcesHash.GetOrCreate (type);
        return resources.Get (name, csRef<iLoadingResource> ());
      }

      virtual void Release (CS::Resource::TypeID type, const char* name)
      {
        Resources& resources = resourcesHash.GetOrCreate (type);
        resources.Delete (name);
      }

    private:
      typedef csRedBlackTreeMap<csString, csRef<iLoadingResource> > Resources;
      csHash<Resources, CS::Resource::TypeID> resourcesHash;
    };
  }
}

#endif // __CS_CSUTIL_MEMCACHE_H__
