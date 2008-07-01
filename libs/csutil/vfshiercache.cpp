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

#include "iutil/objreg.h"
#include "iutil/vfs.h"

#include "csutil/vfshiercache.h"

namespace CS
{
  namespace Utility
  {
    VfsHierarchicalCache::VfsHierarchicalCache (iObjectRegistry* object_reg,
      const char* vfsdir)
     : scfImplementationType (this), object_reg (object_reg), vfsdir (vfsdir)
    {
      vfs = csQueryRegistry<iVFS> (object_reg);
      CS_ASSERT(vfs);
      
      // Ensure no '/' at the end of vfsdir
      if (this->vfsdir.GetAt (this->vfsdir.Length()-1) == '/')
        this->vfsdir.Truncate (this->vfsdir.Length()-1);
    }
  
    VfsHierarchicalCache::~VfsHierarchicalCache ()
    {
    }
  
    bool VfsHierarchicalCache::CacheData (const void* data, size_t size,
                                          const char* path)
    {
      if (!path || !*path || (*path != '/')) return false;
    
      csString fullPath (vfsdir);
      fullPath.Append (path);
      
      /* @@@ TODO: Check if items down to destination are dirs and destination
       * itself a file. If not, remove as needed. */
      
      return vfs->WriteFile (fullPath, (char*)data, size);
    }
    
    csPtr<iDataBuffer> VfsHierarchicalCache::ReadCache (const char* path)
    {
      if (!path || !*path || (*path != '/')) return 0;
    
      csString fullPath (vfsdir);
      fullPath.Append (path);
      
      return vfs->ReadFile (fullPath, false);
    }
    
    bool VfsHierarchicalCache::ClearCache (const char* path)
    {
      if (!path || !*path || (*path != '/')) return false;
    
      csString fullPath (vfsdir);
      fullPath.Append (path);
      
      return vfs->DeleteFile (fullPath);
    }
    
    void VfsHierarchicalCache::Flush ()
    {
      vfs->Sync();
    }
    
    csPtr<iHierarchicalCache> VfsHierarchicalCache::GetRootedCache (const char* base)
    {
      if (!base || !*base || (*base != '/')) return 0;
    
      csString fullPath (vfsdir);
      
      return csPtr<iHierarchicalCache> (new VfsHierarchicalCache (object_reg, fullPath));
    }
      
  } // namespace Utility
} // namespace CS

