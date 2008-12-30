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
#include "csutil/scfstringarray.h"

#include "csutil/vfshiercache.h"

namespace CS
{
  namespace Utility
  {
    VfsHierarchicalCache::VfsHierarchicalCache (iObjectRegistry* object_reg,
      const char* vfsdir)
     : scfImplementationType (this), vfsdir (vfsdir), readonly (false)
    {
      vfs = csQueryRegistry<iVFS> (object_reg);
      CS_ASSERT(vfs);
      
      // Ensure no '/' at the end of vfsdir
      if (this->vfsdir.GetAt (this->vfsdir.Length()-1) == '/')
        this->vfsdir.Truncate (this->vfsdir.Length()-1);
    }
  
    VfsHierarchicalCache::VfsHierarchicalCache (VfsHierarchicalCache* parentCache,
      const char* vfsdir)
     : scfImplementationType (this), parent (parentCache), vfsdir (vfsdir),
       readonly (false)
    {
      vfs = parentCache->vfs;
      
      // Ensure no '/' at the end of vfsdir
      if (this->vfsdir.GetAt (this->vfsdir.Length()-1) == '/')
        this->vfsdir.Truncate (this->vfsdir.Length()-1);
    }
    
    VfsHierarchicalCache::~VfsHierarchicalCache ()
    {
    }
  
    void VfsHierarchicalCache::EnsureDirectories (const char* path)
    {
      CS_ASSERT(path);
      CS_ASSERT(*path);
      CS_ASSERT(path[strlen(path)-1] != '/');
      
      csStringFast<512> pathWithSlash (path);
      pathWithSlash.Append ("/");
      if (vfs->Exists (pathWithSlash))
      {
        // File exists as directory as we want it
      }
      else if (vfs->Exists (path))
      {
        // File exists as file - delete
        vfs->DeleteFile (path);
      }
      else
      {
        // File does not exist - VFS will create it
      }
    }
    
    void VfsHierarchicalCache::EnsureFile (const char* path)
    {
      CS_ASSERT(path);
      CS_ASSERT(*path);
      CS_ASSERT(path[strlen(path)-1] != '/');
      
      csStringFast<512> pathWithSlash (path);
      pathWithSlash.Append ("/");
      if (vfs->Exists (pathWithSlash))
      {
        // File exists as directory
        RecursiveDelete (pathWithSlash);
      }
      else if (vfs->Exists (path))
      {
        // File exists as file, okay
      }
      else
      {
        // File does not exist
        csStringFast<512> dirPart (path);
        dirPart.Truncate (dirPart.FindLast ('/'));
        EnsureDirectories (dirPart);
      }
    }
      
    bool VfsHierarchicalCache::RecursiveDelete (const char* fn)
    {
      csRef<iStringArray> files;
      files = vfs->FindFiles (fn);
      for (size_t i = 0; i < files->GetSize(); i++)
      {
	const char* entry = files->Get (i);
	if (entry[strlen(entry)-1] == '/')
	{
	  RecursiveDelete (entry);
	}
	else
	  vfs->DeleteFile (entry);
      }
      return vfs->DeleteFile (fn);
    }
    
    bool VfsHierarchicalCache::CacheData (const void* data, size_t size,
                                          const char* path)
    {
      if (readonly) return false;
      if (!path || !*path || (*path != '/')) return false;
    
      csStringFast<512> fullPath (vfsdir);
      fullPath.Append (path);
      
      EnsureFile (fullPath);
      
      return vfs->WriteFile (fullPath, (char*)data, size);
    }
    
    csPtr<iDataBuffer> VfsHierarchicalCache::ReadCache (const char* path)
    {
      if (!path || !*path || (*path != '/')) return 0;
    
      csStringFast<512> fullPath (vfsdir);
      fullPath.Append (path);
      
      return vfs->ReadFile (fullPath, false);
    }
    
    bool VfsHierarchicalCache::ClearCache (const char* path)
    {
      if (readonly) return false;
      if (!path || !*path || (*path != '/')) return false;
    
      csStringFast<512> fullPath (vfsdir);
      fullPath.Append (path);
      
      return RecursiveDelete (fullPath);
    }
    
    void VfsHierarchicalCache::Flush ()
    {
      if (readonly) return;
      vfs->Sync();
    }
    
    csPtr<iHierarchicalCache> VfsHierarchicalCache::GetRootedCache (const char* base)
    {
      if (!base || !*base || (*base != '/')) return 0;
    
      csStringFast<512> fullPath (vfsdir);
      fullPath.Append (base);
      
      VfsHierarchicalCache* newCache = new VfsHierarchicalCache (this, fullPath);
      newCache->SetReadOnly (readonly);
      return csPtr<iHierarchicalCache> (newCache);
    }
    
    csPtr<iStringArray> VfsHierarchicalCache::GetSubItems (const char* path)
    {
      csStringFast<512> fullPath (vfsdir);
      fullPath.Append (path);
      
      if (fullPath.GetAt (fullPath.Length()-1) != '/')
        fullPath.Append ("/");
      
      csRef<iStringArray> vfsArray (vfs->FindFiles (fullPath));
      scfStringArray* newArray = new scfStringArray;
      for (size_t i = 0; i < vfsArray->GetSize(); i++)
      {
        newArray->Push (vfsArray->Get (i) + fullPath.Length());
      }
      return csPtr<iStringArray> (newArray);
    }
    
    iHierarchicalCache* VfsHierarchicalCache::GetTopCache()
    {
      return parent.IsValid() ? parent->GetTopCache() : this;
    }
  } // namespace Utility
} // namespace CS

