/*
  Copyright (C) 2009 by Frank Richter
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.
  
  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __PROGCACHE_H__
#define __PROGCACHE_H__

#include "iutil/hiercache.h"
#include "csutil/csstring.h"
#include "csutil/ref.h"
#include "csutil/set.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{
  struct ProfileLimits;
  class ProgramCache;

  struct ProgramObjectID
  {
    csString archive;
    csString item;
    
    ProgramObjectID () {}
    ProgramObjectID (const char* a, const char* i) : archive (a), item (i) {}
  };

  class ProgramObject
  {
    friend class ProgramCache;
    
    ProgramObjectID id;
    csString objectCode;
    uint flags;
    csSet<csString> unusedParams;
  public:
    enum { flagPositionInvariant = 1 };
  
    ProgramObject ();
    ProgramObject (const char* objectCode, uint flags,
      const csSet<csString>& unusedParams);
    
    bool IsValid () const { return !objectCode.IsEmpty(); }
    
    const char* GetObjectCode() const { return objectCode; }
    const ProgramObjectID& GetID() const { return id; }
    uint GetFlags() const { return flags; }
    const csSet<csString>& GetUnusedParams() const { return unusedParams; }
    
    void SetID (const ProgramObjectID& id) { this->id = id; }
  };

  class ProgramCache
  {
    csRef<iHierarchicalCache> cache;
  public:
    void SetCache (iHierarchicalCache* cache) { this->cache = cache; }
  
    bool SearchObject (const char* source, const ProfileLimits& limits,
      ProgramObject& program);
    bool LoadObject (const ProgramObjectID& id, ProgramObject& program);
    bool WriteObject (const char* source, const ProfileLimits& limits,
      const ProgramObject& program, ProgramObjectID& id,
      csString& failReason);
  };
}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)

#endif // __PROGCACHE_H__
