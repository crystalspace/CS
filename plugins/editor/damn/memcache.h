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

#ifndef __DAMN_MEMCACHE_H__
#define __DAMN_MEMCACHE_H__

#include "imap/resource.h"
#include "include/resource.h"

#include <csutil/scf_implementation.h>
#include <iengine/engine.h>
#include <iengine/mesh.h>

#include <string>
#include <map>

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{
class MemoryCache : public scfImplementation1<MemoryCache,iResourceCache>
{
public:
  MemoryCache ();
  virtual ~MemoryCache ();

  // iResourceCache
  virtual void Add (const char* name, iLoading* resource);

  virtual csRef<iLoading> Get (const char* name);

  virtual void Release (const char* name);


private:
  iObjectRegistry* object_reg;
  
  typedef std::map<std::string, csRef<iLoading> > Resources;
  Resources resources;
};
}
CS_PLUGIN_NAMESPACE_END(CSE)

#endif
