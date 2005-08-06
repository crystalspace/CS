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

#include "cssysdef.h"

#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/region.h"
#include "iengine/texture.h"
#include "imap/loader.h"
#include "imesh/genmesh.h"
#include "imesh/object.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivideo/material.h"
#include "ivideo/texture.h"

#include "csutil/xmltiny.h"
#include "cstool/vfsdirchange.h"

#include "cstool/importkit.h"
#include "importkit_glue.h"

#include <ctype.h>

namespace CrystalSpace
{
  namespace ImportKitImpl 
  { 
    
    Glue::Glue (iObjectRegistry* objectReg) : objectReg(objectReg), texId(0)
    {
      vfs = CS_QUERY_REGISTRY (objectReg, iVFS);
    }

    Glue::~Glue()
    {
    }

    CS_IMPLEMENT_STATIC_VAR(GetTempScratch, csString, ())

    const char* Glue::GetTempName()
    {
      static int n = 0;
      GetTempScratch()->Format (CS_TEMP_FILE);
      GetTempScratch()->AppendFmt ("_%d", n++);
      return GetTempScratch()->GetData();
    }

    bool Glue::PopulateContainer (const char* filename, 
				  const char* path, 
				  ImportKit::Container& container)
    {
      csVfsDirectoryChanger dirChange (vfs);
      dirChange.PushDir();
      VfsRootMounter vfsRoot (vfs);

      if (!ChangeToCurrentNative (vfsRoot.GetRootPath()))
	return false;

      if (path && !vfs->ChDirAuto (path, 0, 0, filename))
	return false;

      csRef<iEngine> engine;
      CS_QUERY_REGISTRY_PLUGIN (engine, objectReg,
	"crystalspace.engine.3d", iEngine);
      if (!engine.IsValid()) return false;
      csRef<iLoader> loader;
      CS_QUERY_REGISTRY_PLUGIN (loader, objectReg,
	"crystalspace.level.loader", iLoader);

      csString regionName;
      regionName.Format ("ImportKitRegion_%s", GetTempName());
      csRef<iRegion> loadRegion = engine->CreateRegion (regionName);

      {
	csRef<iFile> file = vfs->Open (filename, VFS_FILE_READ);
	if (!file) return false;

	csRef<iDocumentSystem> docsys (
	    CS_QUERY_REGISTRY (objectReg, iDocumentSystem));
	if (!docsys) docsys = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
	csRef<iDocument> doc = docsys->CreateDocument ();
	const char* error = doc->Parse (file, true);
	if (error != 0)
	  return false;

	iBase* result;
	if (!loader->Load (doc->GetRoot(), result, loadRegion))
	  return false;
      }

      csRef<iObjectIterator> regionObjects = 
	loadRegion->QueryObject()->GetIterator();
      while (regionObjects->HasNext())
      {
	csRef<iObject> obj = regionObjects->Next();

	if (ProbeMeshFactory (container, obj)) continue;
	if (ProbeMaterial (container, obj)) continue;
	if (ProbeTexture (container, obj)) continue;
      }

      engine->RemoveObject (loadRegion);

      return container.models.Length() > 0;
    }

#if defined(CS_PLATFORM_WIN32) && !defined(__CYGWIN__)
  #define getcwd  _getcwd
#endif

    bool Glue::ChangeToCurrentNative (const char* nativeRoot)
    {
      char* cwd = getcwd (0, 0);
      bool ret = false;

      csString path (nativeRoot);
      const char* p = cwd;
#if defined(CS_PLATFORM_WIN32)
      // Adjust for driver letter on Win32
      path << '/';
      path << (char)tolower(*p);
      p += 2;
#endif
      while (*p != 0)
      {
	const char ch = *p;
	if (ch == CS_PATH_SEPARATOR)
	  path << '/';
	else
	  path << ch;
	p++;
      }
      ret = vfs->ChDir (path);

      free (cwd);
      return ret;
    }

    bool Glue::ProbeMaterial (ImportKit::Container& container, 
			      iObject* obj)
    {
      csRef<iMaterialWrapper> mat = 
	scfQueryInterface<iMaterialWrapper> (obj);
      if (!mat) return false;

      csRef<iMaterialEngine> matEngine =
	scfQueryInterface<iMaterialEngine> (mat->GetMaterial());
      if (!matEngine) return false;

      if (texture2id.In (matEngine->GetTextureWrapper ()))
      {
	material2texture.Put ((iMaterialWrapper*)mat, 
	  texture2id.Get (matEngine->GetTextureWrapper(), 0));
      }

      return true;
    }

    bool Glue::ProbeTexture (ImportKit::Container& container, 
			     iObject* obj)
    {
      csRef<iTextureWrapper> tex = 
	scfQueryInterface<iTextureWrapper> (obj);
      if (!tex) return false;

      texture2id.Put ((iTextureWrapper*)tex, texId++);
      
      ImportKit::Container::Material newMat;
      newMat.name = csStrNewW (obj->GetName());
      newMat.texture = csStrNew (tex->GetTextureHandle()->GetImageName());

      container.materials.Push (newMat);

      return true;
    }

    bool Glue::ProbeMeshFactory (ImportKit::Container& container, 
				 iObject* obj)
    {
      csRef<iMeshFactoryWrapper> fact = 
	scfQueryInterface<iMeshFactoryWrapper> (obj);
      if (!fact) return false;
      if (ProbeGMFactory (container, fact, obj->GetName()))
	return true;
      return false;
    }

    bool Glue::ProbeGMFactory (ImportKit::Container& container, 
			       iMeshFactoryWrapper* fact, const char* name)
    {
      csRef<iGeneralFactoryState> gmfact = 
	scfQueryInterface<iGeneralFactoryState> (fact->GetMeshObjectFactory());
      if (!gmfact) return false;

      GluedModel* model = glueModelPool.Alloc();
      int vc = gmfact->GetVertexCount ();
      model->allVertices.SetLength (vc);
      memcpy (model->allVertices.GetArray(), gmfact->GetVertices(),
	sizeof (csVector3) * vc);
      model->allTCs.SetLength (vc);
      memcpy (model->allTCs.GetArray(), gmfact->GetTexels(),
	sizeof (csVector2) * vc);
      model->allNormals.SetLength (vc);
      memcpy (model->allNormals.GetArray(), gmfact->GetNormals(),
	sizeof (csVector3) * vc);
      size_t tc = gmfact->GetTriangleCount();
      model->tris.SetLength (tc);
      memcpy (model->tris.GetArray(), gmfact->GetTriangles(),
	sizeof(csTriangle) * tc);

      ImportKit::Container::Model newModel;
      ImportKit::Container::Model::Mesh newMesh;
      newMesh.vertexCount = vc;
      newMesh.verts = (float*)model->allVertices.GetArray();
      newMesh.texcoords = (float*)model->allTCs.GetArray();
      newMesh.normals = (float*)model->allNormals.GetArray();
      newMesh.triCount = tc;
      newMesh.tris = (uint*)model->tris.GetArray();
      newMesh.material = 
	material2texture.Get (gmfact->GetMaterialWrapper(), (size_t)-1);

      newModel.meshes.Push (newMesh);
      newModel.glueModel = model;
      newModel.name = csStrNewW (name);
      container.models.Push (newModel);
      
      return true;
    }

  } // namespace ImportKitImpl 
} // namespace CrystalSpace
