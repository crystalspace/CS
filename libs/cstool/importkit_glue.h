/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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

#ifndef __CS_LIBS_CSTOOL_IMPORTKIT_GLUE_H__
#define __CS_LIBS_CSTOOL_IMPORTKIT_GLUE_H__

#include "iutil/stringarray.h"
#include "iutil/vfs.h"

#include "csgeom/tri.h"
#include "csgeom/vector2.h"
#include "csgeom/vector3.h"
#include "csutil/blockallocator.h"
#include "csutil/csstring.h"
#include "csutil/dirtyaccessarray.h"

struct iMaterialWrapper;
struct iMeshFactoryWrapper;
struct iObject;
struct iTextureWrapper;

namespace CrystalSpace
{
  namespace ImportKitImpl 
  { 
    struct GluedModel
    {
      csDirtyAccessArray<csVector3> allVertices;
      csDirtyAccessArray<csVector2> allTCs;
      csDirtyAccessArray<csVector3> allNormals;
      csDirtyAccessArray<csTriangle> tris;
    };

    /// Bulk of the import kit implementation.
    class Glue
    {
      iObjectRegistry* objectReg;
      csRef<iVFS> vfs;
      csBlockAllocator<ImportKitImpl::GluedModel> glueModelPool;
      csHash<size_t, csPtrKey<iMaterialWrapper> > material2texture;
      csHash<size_t, csPtrKey<iTextureWrapper> > texture2id;
      size_t texId;

      /// Helper to get a unique name to be used for temporary file names
      static const char* GetTempName();

      /// Helper to mount the file system root into a unique mount point.
      class VfsRootMounter
      {
	csString rootPath;
	csRef<iStringArray> mounted;
	iVFS* vfs;
      public:
	VfsRootMounter (iVFS* vfs) : vfs(vfs)
	{
	  rootPath.Format ("/Root_%s", GetTempName());
	  mounted = vfs->MountRoot (rootPath);
	}
	~VfsRootMounter ()
	{
	  for (size_t i = 0; i < mounted->Length(); i++)
	    vfs->Unmount (mounted->Get (i), 0);
	}
	const csString& GetRootPath() { return rootPath; }
      };
      friend class VfsRootMounter;

      /**
       * Change the VFS current working directory to the current native 
       * working directory.
       */
      bool ChangeToCurrentNative (const char* nativeRoot);
      /// Check whether an engine object is a material and handle if so
      bool ProbeMaterial (ImportKit::Container& container, iObject* obj);
      /// Check whether an engine object is a texture and handle if so
      bool ProbeTexture (ImportKit::Container& container, iObject* obj);
      /// Check whether an engine object is a mesh factory and handle if so
      bool ProbeMeshFactory (ImportKit::Container& container, iObject* obj);
      /// Check whether an engine object is a genmesh factory and handle if so
      bool ProbeGMFactory (ImportKit::Container& container, 
	iMeshFactoryWrapper* fact, const char* name);
      /// Check whether an engine object is a spr3d factory and handle if so
      bool ProbeSpr3dFactory (ImportKit::Container& container, 
	iMeshFactoryWrapper* fact, const char* name);
      /// Check whether an engine object is a Thing factory and handle if so
      bool ProbeThingFactory (ImportKit::Container& container, 
	iMeshFactoryWrapper* fact, const char* name);
    public:
      Glue (iObjectRegistry* objectReg);
      ~Glue();

      bool PopulateContainer (const char* filename, 
	const char* path, ImportKit::Container& container);
    };

  } // namespace ImportKitImpl 
} // namespace CrystalSpace

#endif // __CS_LIBS_CSTOOL_IMPORTKIT_GLUE_H__
