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

#include "cssysdef.h"
#include "csutil/scf.h"
#include "csutil/sysfunc.h"

#include "iengine/sector.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"


#include <iostream>
#include <sstream>
#include <string>

#include "memcache.h"



CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

MemoryCache::MemoryCache () : scfImplementationType (this)
{
}

MemoryCache::~MemoryCache ()
{
}

void MemoryCache::Add (const char* type, const char* name, iLoadingResource* resource)
{
  resources[name] = resource;
}

csRef<iLoadingResource> MemoryCache::Get (const char* type, const char* name)
{
  Resources::const_iterator found = resources.find(name);
  if (found != resources.end())
  {
    return found->second;
  }
  return 0;
}

void MemoryCache::Release (const char* type, const char* name)
{
}

}
CS_PLUGIN_NAMESPACE_END(CSE)
