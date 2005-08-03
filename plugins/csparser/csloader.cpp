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
#include "csutil/sysfunc.h"
#include "csqint.h"
#include "csqsqrt.h"
#include "ivideo/graph3d.h"
#include "csutil/xmltiny.h"
#include "csutil/cfgfile.h"
#include "csutil/scanstr.h"
#include "csutil/scfstr.h"
#include "cstool/gentrtex.h"
#include "cstool/keyval.h"
#include "cstool/sndwrap.h"
#include "cstool/mapnode.h"
#include "cstool/mdltool.h"
#include "cstool/vfsdirchange.h"
#include "csloader.h"

#include "iutil/databuff.h"
#include "iutil/document.h"
#include "imap/reader.h"
#include "imap/ldrctxt.h"
#include "imesh/lighting.h"
#include "imesh/sprite3d.h"
#include "imesh/object.h"
#include "iengine/engine.h"
#include "iengine/region.h"
#include "iengine/texture.h"
#include "iengine/material.h"
#include "iengine/collectn.h"
#include "iengine/sector.h"
#include "iengine/movable.h"
#include "iengine/halo.h"
#include "iengine/campos.h"
#include "iengine/light.h"
#include "iengine/mesh.h"
#include "iengine/lod.h"
#include "iengine/imposter.h"
#include "iengine/sharevar.h"
#include "iengine/viscull.h"
#include "iengine/portal.h"
#include "iengine/portalcontainer.h"
#include "ivideo/material.h"
#include "ivideo/texture.h"
#include "igraphic/imageio.h"
#include "isound/loader.h"
#include "isound/renderer.h"
#include "iutil/vfs.h"
#include "iutil/plugin.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "iutil/verbositymanager.h"
#include "imesh/thing.h"
#include "imesh/nullmesh.h"
#include "imesh/mdlconv.h"
#include "imesh/mdldata.h"
#include "imesh/crossbld.h"
#include "ivaria/reporter.h"
#include "csgeom/poly3d.h"
#include "csgeom/polymesh.h"
#include "igeom/polymesh.h"
#include "igeom/objmodel.h"

#include "loadtex.h"

#include "ivideo/shader/shader.h"
#include "iengine/renderloop.h"

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csLoaderStatus)
  SCF_IMPLEMENTS_INTERFACE (iLoaderStatus)
SCF_IMPLEMENT_IBASE_END

csLoaderStatus::csLoaderStatus ()
{
  SCF_CONSTRUCT_IBASE (0);
  //mutex = csMutex::Create (true);
}

csLoaderStatus::~csLoaderStatus ()
{
  SCF_DESTRUCT_IBASE ();
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(StdLoaderContext);
  SCF_IMPLEMENTS_INTERFACE(iLoaderContext);
SCF_IMPLEMENT_IBASE_END

StdLoaderContext::StdLoaderContext (iEngine* Engine,
	iRegion* region, bool curRegOnly, csLoader* loader,
	bool checkDupes)
{
  SCF_CONSTRUCT_IBASE (0);
  StdLoaderContext::Engine = Engine;
  StdLoaderContext::region = region;
  StdLoaderContext::curRegOnly = curRegOnly;
  StdLoaderContext::loader = loader;
  StdLoaderContext::checkDupes = checkDupes;
}

StdLoaderContext::~StdLoaderContext ()
{
  SCF_DESTRUCT_IBASE ();
}

iSector* StdLoaderContext::FindSector (const char* name)
{
  return Engine->FindSector (name, curRegOnly ? region : 0);
}

iMaterialWrapper* StdLoaderContext::FindMaterial (const char* filename)
{
  // @@@ in case the material is not found a replacement is taken.
  // however, somehow the location of the errorneous material name
  // should be reported. 
  iMaterialWrapper* mat = Engine->FindMaterial (filename, curRegOnly ? region : 0);
  if (mat)
    return mat;

  if (csLoader::do_verbose)
    loader->ReportNotify ("Could not find material '%s'. "
      "Creating new material using texture with that name", filename);
  iTextureWrapper* tex = FindTexture (filename);
  if (tex)
  {
    // Add a default material with the same name as the texture
    csRef<iMaterial> material = Engine->CreateBaseMaterial (tex);
    // First we have to extract the optional region name from the name:
    char const* n = strchr (filename, '/');
    if (!n) n = filename;
    else n++;
    iMaterialWrapper *mat = Engine->GetMaterialList ()
      	->NewMaterial (material, n);
    if (region) region->QueryObject ()->ObjAdd (mat->QueryObject ());

    // @@@ should this be done here?
    iTextureManager *tm;
    if ((loader->G3D) && (tm = loader->G3D->GetTextureManager()))
    {
      tex->Register (tm);
    }
    return mat;
  }

  return 0;
}

iMaterialWrapper* StdLoaderContext::FindNamedMaterial (const char* name, 
                                                       const char *filename)
{
  // @@@ in case the material is not found a replacement is taken.
  // however, somehow the location of the errorneous material name
  // should be reported. 
  iMaterialWrapper* mat = Engine->FindMaterial (name, curRegOnly ? region : 0);
  if (mat)
    return mat;

  if (csLoader::do_verbose)
    loader->ReportNotify ("Could not find material '%s'. "
      "Creating new material using texture with that name", name);
  iTextureWrapper* tex = FindNamedTexture (name,filename);
  if (tex)
  {
    // Add a default material with the same name as the texture
    csRef<iMaterial> material = Engine->CreateBaseMaterial (tex);
    // First we have to extract the optional region name from the name:
    char const* n = strchr (name, '/');
    if (!n) n = name;
    else n++;
    iMaterialWrapper *mat = Engine->GetMaterialList ()
      	->NewMaterial (material, n);
    if (region) region->QueryObject ()->ObjAdd (mat->QueryObject ());

    // @@@ should this be done here?
    iTextureManager *tm;
    if ((loader->G3D) && (tm = loader->G3D->GetTextureManager()))
    {
      tex->Register (tm);
    }
    return mat;
  }

  return 0;
}


iMeshFactoryWrapper* StdLoaderContext::FindMeshFactory (const char* name)
{
  return Engine->FindMeshFactory (name, curRegOnly ? region : 0);
}

iMeshWrapper* StdLoaderContext::FindMeshObject (const char* name)
{
  return Engine->FindMeshObject (name, curRegOnly ? region : 0);
}

iLight* StdLoaderContext::FindLight (const char *name)
{
  csRef<iLightIterator> li = Engine->GetLightIterator (
  	curRegOnly ? region : 0);
  iLight *light;

  while (li->HasNext ())
  {
    light = li->Next ();
    if (!strcmp (light->QueryObject ()->GetName (), name))
      return light;
  }
  return 0;
}

iShader* StdLoaderContext::FindShader (const char *name)
{
  csRef<iShaderManager> shaderMgr = CS_QUERY_REGISTRY (
  	loader->object_reg, iShaderManager);
  if (!shaderMgr) return 0;
  if (!curRegOnly || !region) return shaderMgr->GetShader (name);

  csRefArray<iShader> shaders = shaderMgr->GetShaders ();
  size_t i;
  for (i = 0 ; i < shaders.Length () ; i++)
  {
    iShader* s = shaders[i];
    if (region->IsInRegion (s->QueryObject ())
    	&& !strcmp (name, s->QueryObject ()->GetName ()))
      return s;
  }

  return 0;
}

iTextureWrapper* StdLoaderContext::FindTexture (const char* name)
{
  // @@@ in case the texture is not found a replacement is taken.
  // however, somehow the location of the errorneous texture name
  // should be reported. 
  iTextureWrapper* result;
  if (region && curRegOnly)
    result = region->FindTexture (name);
  else
    result = Engine->GetTextureList ()->FindByName (name);

  if (!result)
  {
    if (csLoader::do_verbose)
      loader->ReportNotify ("Could not find texture '%s'. Attempting to load.", 
        name);
    csRef<iTextureWrapper> rc = loader->LoadTexture (name, name);
    if (region) region->QueryObject ()->ObjAdd (rc->QueryObject ());
    result = rc;
  }
  return result;
}

iTextureWrapper* StdLoaderContext::FindNamedTexture (const char* name,
                                                     const char *filename)
{
  // @@@ in case the texture is not found a replacement is taken.
  // however, somehow the location of the errorneous texture name
  // should be reported. 
  iTextureWrapper* result;
  if (region && curRegOnly)
    result = region->FindTexture (name);
  else
    result = Engine->GetTextureList ()->FindByName (name);

  if (!result)
  {
    if (csLoader::do_verbose)
      loader->ReportNotify ("Could not find texture '%s'. Attempting to load.", 
        name);
    csRef<iTextureWrapper> rc = loader->LoadTexture (name, filename, 2, 0,
      false, false);
    if (region)
      region->QueryObject ()->ObjAdd (rc->QueryObject ());
    result = rc;
  }
  return result;
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(ThreadedLoaderContext);
  SCF_IMPLEMENTS_INTERFACE(iLoaderContext);
SCF_IMPLEMENT_IBASE_END

ThreadedLoaderContext::ThreadedLoaderContext (iEngine* Engine,
	iRegion* region, bool curRegOnly, csLoader* loader,
	bool checkDupes)
{
  SCF_CONSTRUCT_IBASE (0);
  ThreadedLoaderContext::region = region;
  ThreadedLoaderContext::curRegOnly = curRegOnly;
  ThreadedLoaderContext::loader = loader;
  ThreadedLoaderContext::checkDupes = checkDupes;
}

ThreadedLoaderContext::~ThreadedLoaderContext ()
{
  SCF_DESTRUCT_IBASE ();
}

iSector* ThreadedLoaderContext::FindSector (const char* name)
{
  return Engine->FindSector (name, curRegOnly ? region : 0);
}

iMaterialWrapper* ThreadedLoaderContext::FindMaterial (const char* name)
{
  // @@@ in case the material is not found a replacement is taken.
  // however, somehow the location of the errorneous material name
  // should be reported. 
  iMaterialWrapper* mat = Engine->FindMaterial (name, curRegOnly ? region : 0);
  if (mat)
    return mat;

  if (csLoader::do_verbose)
    loader->ReportNotify ("Could not find material '%s'. "
      "Creating new material using texture with that name", name);
  iTextureWrapper* tex = FindTexture (name);
  if (tex)
  {
    // Add a default material with the same name as the texture
    csRef<iMaterial> material = Engine->CreateBaseMaterial (tex);
    // First we have to extract the optional region name from the name:
    char const* n = strchr (name, '/');
    if (!n) n = name;
    else n++;
    iMaterialWrapper *mat = Engine->GetMaterialList ()
      	->NewMaterial (material, n);
    if (region) region->QueryObject ()->ObjAdd (mat->QueryObject ());

    // @@@ should this be done here?
    iTextureManager *tm;
    if ((loader->G3D) && (tm = loader->G3D->GetTextureManager()))
    {
      tex->Register (tm);
    }
    return mat;
  }

  return 0;
}

iMaterialWrapper* ThreadedLoaderContext::FindNamedMaterial (const char* name,
                                                            const char* filename)
{
  // @@@ in case the material is not found a replacement is taken.
  // however, somehow the location of the errorneous material name
  // should be reported. 
  iMaterialWrapper* mat = Engine->FindMaterial (name, curRegOnly ? region : 0);
  if (mat)
    return mat;

  if (csLoader::do_verbose)
    loader->ReportNotify ("Could not find material '%s'. "
      "Creating new material using texture with that name", name);
  iTextureWrapper* tex = FindNamedTexture (name,filename);
  if (tex)
  {
    // Add a default material with the same name as the texture
    csRef<iMaterial> material = Engine->CreateBaseMaterial (tex);
    // First we have to extract the optional region name from the name:
    char const* n = strchr (name, '/');
    if (!n) n = name;
    else n++;
    iMaterialWrapper *mat = Engine->GetMaterialList ()
      	->NewMaterial (material, n);
    if (region) region->QueryObject ()->ObjAdd (mat->QueryObject ());

    // @@@ should this be done here?
    iTextureManager *tm;
    if ((loader->G3D) && (tm = loader->G3D->GetTextureManager()))
    {
      tex->Register (tm);
    }
    return mat;
  }

  return 0;
}

iMeshFactoryWrapper* ThreadedLoaderContext::FindMeshFactory (const char* name)
{
  return Engine->FindMeshFactory (name, curRegOnly ? region : 0);
}

iMeshWrapper* ThreadedLoaderContext::FindMeshObject (const char* name)
{
  return Engine->FindMeshObject (name, curRegOnly ? region : 0);
}

iLight* ThreadedLoaderContext::FindLight (const char *name)
{
  csRef<iLightIterator> li = Engine->GetLightIterator (
  	curRegOnly ? region : 0);
  iLight *light;

  while (li->HasNext ())
  {
    light = li->Next ();
    if (!strcmp (light->QueryObject ()->GetName (),name))
      return light;
  }
  return 0;
}

iShader* ThreadedLoaderContext::FindShader (const char *name)
{
  csRef<iShaderManager> shaderMgr = CS_QUERY_REGISTRY (
  	loader->object_reg, iShaderManager);
  if (!shaderMgr) return 0;
  if (!curRegOnly || !region) return shaderMgr->GetShader (name);

  csRefArray<iShader> shaders = shaderMgr->GetShaders ();
  size_t i;
  for (i = 0 ; i < shaders.Length () ; i++)
  {
    iShader* s = shaders[i];
    if (region->IsInRegion (s->QueryObject ())
    	&& !strcmp (name, s->QueryObject ()->GetName ()))
      return s;
  }
  return 0;
}

iTextureWrapper* ThreadedLoaderContext::FindTexture (const char* name)
{
  // @@@ in case the texture is not found a replacement is taken.
  // however, somehow the location of the errorneous texture name
  // should be reported. 
  iTextureWrapper* result;
  if (region && curRegOnly)
    result = region->FindTexture (name);
  else
    result = Engine->GetTextureList ()->FindByName (name);

  if (!result)
  {
    if (csLoader::do_verbose)
      loader->ReportNotify ("Could not find texture '%s'. Attempting to load.", 
        name);
    csRef<iTextureWrapper> rc = loader->LoadTexture (name, name);
    if (region) region->QueryObject ()->ObjAdd (rc->QueryObject ());
    result = rc;
  }
  return result;
}

iTextureWrapper* ThreadedLoaderContext::FindNamedTexture (const char* name,
                                                          const char* filename)
{
  // @@@ in case the texture is not found a replacement is taken.
  // however, somehow the location of the errorneous texture name
  // should be reported. 
  iTextureWrapper* result;
  if (region && curRegOnly)
    result = region->FindTexture (name);
  else
    result = Engine->GetTextureList ()->FindByName (name);

  if (!result)
  {
    if (csLoader::do_verbose)
      loader->ReportNotify ("Could not find texture '%s'. Attempting to load.", 
        name);
    csRef<iTextureWrapper> rc = loader->LoadTexture (name, filename);
    if (region) region->QueryObject ()->ObjAdd (rc->QueryObject ());
    result = rc;
  }
  return result;
}

//---------------------------------------------------------------------------

bool csLoader::do_verbose = false;

void csLoader::ReportError (const char* id, const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  csReportV (object_reg, CS_REPORTER_SEVERITY_ERROR, id, description, arg);
  va_end (arg);
}

void csLoader::ReportNotify (const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  ReportNotifyV ("crystalspace.maploader", description, arg);
  va_end (arg);
}

void csLoader::ReportNotifyV (const char* id, const char* description,
	va_list arg)
{
  csReportV (object_reg, CS_REPORTER_SEVERITY_NOTIFY, id, description, arg);
}

void csLoader::ReportNotify2 (const char* id, const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  ReportNotifyV (id, description, arg);
  va_end (arg);
}

void csLoader::ReportWarning (const char* id, const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  csReportV (object_reg, CS_REPORTER_SEVERITY_WARNING, id, description, arg);
  va_end (arg);
}

void csLoader::ReportWarning (const char* id, iDocumentNode* node,
	const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  csString buf;
  buf.FormatV (description, arg);
  va_end (arg);
  SyntaxService->Report (id, CS_REPORTER_SEVERITY_WARNING, node, "%s", 
    buf.GetData());
}
//---------------------------------------------------------------------------

bool csLoader::LoadStructuredDoc (const char* file, iFile* buf,
	csRef<iDocument>& doc)
{
  csRef<iDocumentSystem> docsys (
      CS_QUERY_REGISTRY (object_reg, iDocumentSystem));
  if (!docsys) docsys = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
  doc = docsys->CreateDocument ();
  const char* error = doc->Parse (buf, true);
  if (error != 0)
  {
    ReportError (
	    "crystalspace.maploader.parse.plugin",
	    "Document system error for file '%s': %s!", file, error);
    doc = 0;
    return false;
  }
  return true;
}

csPtr<iBase> csLoader::LoadStructuredMap (iLoaderContext* ldr_context,
	iLoaderPlugin* plug, iFile* buf,
  	iBase* context, const char* fname)
{
  csRef<iDocument> doc;
  csString filename (fname);
  csVfsDirectoryChanger dirChanger (VFS);
  size_t slashPos = filename.FindLast ('/');
  if (slashPos != (size_t)-1)
  {
    dirChanger.ChangeTo (filename);
    filename.DeleteAt (0, slashPos + 1);
  }
  bool er = LoadStructuredDoc (filename, buf, doc);
  csRef<iBase> ret;
  if (er)
  {
    if (doc)
    {
      // First find the <params> node in the loaded file.
      csRef<iDocumentNode> paramsnode = doc->GetRoot ()->GetNode ("params");
      if (!paramsnode)
      {
        SyntaxService->ReportError (
	        "crystalspace.maploader.load.plugin",
                doc->GetRoot (), "Could not find <params> in '%s'!", fname);
      }
      else
      {
        ret = plug->Parse (paramsnode, ldr_context, context);
      }
    }
    else
    {
      ReportError ("crystalspace.maploader.load.plugin",
	      "File does not appear to be a structured map file (%s)!", fname);
    }
  }
  return csPtr<iBase> (ret);
}

//---------------------------------------------------------------------------

csPtr<iLoaderStatus> csLoader::ThreadedLoadMapFile (const char* filename,
	iRegion* /*region*/, bool /*curRegOnly*/, bool /*checkDupes*/)
{
  return 0;
}

bool csLoader::Load (const char* fname, iBase*& result, iRegion* region,
  	bool curRegOnly, bool checkDupes)
{
  result = 0;

  csRef<iFile> buf = VFS->Open (fname, VFS_FILE_READ);

  if (!buf)
  {
    ReportError (
	      "crystalspace.maploader.parse",
    	      "Could not open map file '%s' on VFS!", fname);
    return false;
  }

  csRef<iDocument> doc;
  bool er = LoadStructuredDoc (fname, buf, doc);
  if (!er) return false;

  if (doc)
  {
    csRef<iDocumentNode> node = doc->GetRoot ();
    return Load (node, result, region, curRegOnly, checkDupes);
  }
  else
  {
    ReportError ("crystalspace.maploader.parse",
      "File does not appear to be correct XML file (%s)!", fname);
  }
  return false;
}

bool csLoader::Load (iDocumentNode* node, iBase*& result, iRegion* region,
  	bool curRegOnly, bool checkDupes)
{
  result = 0;

  csRef<iLoaderContext> ldr_context = csPtr<iLoaderContext> (
	new StdLoaderContext (Engine, region, curRegOnly, this, checkDupes));

  csRef<iDocumentNode> meshfactnode = node->GetNode ("meshfact");
  if (meshfactnode)
  {
    const char* meshfactname = meshfactnode->GetAttributeValue ("name");
    if (ldr_context->CheckDupes () && meshfactname)
    {
      iMeshFactoryWrapper* t = Engine->FindMeshFactory (meshfactname);
      if (t) { result = t; return true; }
    }

    csRef<iMeshFactoryWrapper> t = Engine->CreateMeshFactory (
        meshfactname);
    if (LoadMeshObjectFactory (ldr_context, t, 0, meshfactnode))
    {
      AddToRegion (ldr_context, t->QueryObject ());
      result = t;
      return true;
    }
    else
    {
      // Error is already reported.
      Engine->GetMeshFactories ()->Remove (t);
      result = 0;
      return false;
    }
  }

  csRef<iDocumentNode> meshobjnode = node->GetNode ("meshobj");
  if (meshobjnode)
  {
    const char* meshobjname = meshobjnode->GetAttributeValue ("name");
    if (ldr_context->CheckDupes () && meshobjname)
    {
      iMeshWrapper* t = Engine->FindMeshObject (meshobjname);
      if (t) { result = t; return true; }
    }
    csRef<iMeshWrapper> t = Engine->CreateMeshWrapper (meshobjname);
    if (LoadMeshObject (ldr_context, t, 0, meshobjnode))
    {
      AddToRegion (ldr_context, t->QueryObject ());
      result = t;
      return true;
    }
    else
    {
      // Error is already reported.
      Engine->GetMeshes ()->Remove (t);
      result = 0;
      return false;
    }
  }

  csRef<iDocumentNode> worldnode = node->GetNode ("world");
  if (worldnode)
  {
    result = Engine;
    return LoadMap (ldr_context, worldnode);
  }

  csRef<iDocumentNode> libnode = node->GetNode ("library");
  if (libnode)
  {
    result = 0;
    return LoadLibrary (ldr_context, libnode);
  }

  ReportError ("crystalspace.maploader.parse",
    "File doesn't seem to be a world, library, meshfact, or meshobj file!");

  return false;
}

bool csLoader::LoadMapFile (const char* file, bool clearEngine,
  iRegion* region, bool curRegOnly, bool checkdupes)
{
  csRef<iFile> buf = VFS->Open (file, VFS_FILE_READ);

  if (!buf)
  {
    ReportError (
	      "crystalspace.maploader.parse.map",
    	      "Could not open map file '%s' on VFS!", file);
    return false;
  }

  csRef<iDocument> doc;
  bool er = LoadStructuredDoc (file, buf, doc);
  if (!er) return false;

  if (doc)
  {
    csRef<iDocumentNode> world_node = doc->GetRoot ()->GetNode ("world");
    if (!world_node)
    {
      SyntaxService->ReportError (
        "crystalspace.maploader.parse.expectedworld",
        world_node, "Expected 'world' token!");
      return false;
    }
    return LoadMap (world_node, clearEngine, region, curRegOnly, checkdupes);
  }
  else
  {
    ReportError ("crystalspace.maploader.parse.plugin", 
      "File does not appear to be a structured map file (%s)!", file);
    return false;
  }

  return true;
}

bool csLoader::LoadMap (iDocumentNode* world_node, bool clearEngine,
  iRegion* region, bool curRegOnly, bool checkdupes)
{
  if (clearEngine)
  {
    Engine->DeleteAll ();
    Engine->ResetWorldSpecificSettings();
  }
  csRef<iLoaderContext> ldr_context = csPtr<iLoaderContext> (
	new StdLoaderContext (Engine, region, curRegOnly, this, checkdupes));

  return LoadMap (ldr_context, world_node);
}

//---------------------------------------------------------------------------

bool csLoader::LoadLibraryFile (const char* fname, iRegion* region,
	bool curRegOnly, bool checkDupes)
{
  csRef<iFile> buf = VFS->Open (fname, VFS_FILE_READ);

  if (!buf)
  {
    ReportError (
	      "crystalspace.maploader.parse.library",
    	      "Could not open library file '%s' on VFS!", fname);
    return false;
  }

  csRef<iLoaderContext> ldr_context = csPtr<iLoaderContext> (
	new StdLoaderContext (Engine, region, curRegOnly, this, checkDupes));

  csRef<iDocument> doc;
  bool er = LoadStructuredDoc (fname, buf, doc);
  if (!er) return false;
  if (doc)
  {
    csRef<iDocumentNode> lib_node = doc->GetRoot ()->GetNode ("library");
    if (!lib_node)
    {
      SyntaxService->ReportError (
        "crystalspace.maploader.parse.expectedlib",
        lib_node, "Expected 'library' token!");
      return false;
    }
    return LoadLibrary (ldr_context, lib_node);
  }
  else
  {
    ReportError ("crystalspace.maploader.parse.plugin",
      "File does not appear to be a structure map library (%s)!", fname);
  }
  return false;
}

bool csLoader::LoadLibrary (iDocumentNode* lib_node, iRegion* region,
	bool curRegOnly, bool checkDupes)
{
  csRef<iLoaderContext> ldr_context = csPtr<iLoaderContext> (
	new StdLoaderContext (Engine, region, curRegOnly, this, checkDupes));

  return LoadLibrary (ldr_context, lib_node);
}

//---------------------------------------------------------------------------

void csLoader::AddToRegion (iLoaderContext* ldr_context, iObject* obj)
{
  if (ldr_context->GetRegion ())
    ldr_context->GetRegion ()->QueryObject ()->ObjAdd (obj);
}

//---------------------------------------------------------------------------

csPtr<iMeshFactoryWrapper> csLoader::LoadMeshObjectFactory (const char* fname)
{
  if (!Engine) return 0;

  csRef<iLoaderContext> ldr_context = csPtr<iLoaderContext> (
	new StdLoaderContext (Engine, 0, true, this, false));

  csRef<iFile> databuff (VFS->Open (fname, VFS_FILE_READ));

  if (!databuff || !databuff->GetSize ())
  {
    ReportError (
	      "crystalspace.maploader.parse.meshfactory",
    	      "Could not open mesh object file '%s' on VFS!", fname);
    return 0;
  }

  csRef<iDocument> doc;
  bool er = LoadStructuredDoc (fname, databuff, doc);
  if (!er) return 0;
  if (doc)
  {
    csRef<iDocumentNode> meshfactnode = doc->GetRoot ()->GetNode ("meshfact");
    if (!meshfactnode)
    {
      ReportError (
	      "crystalspace.maploader.parse.map",
    	      "File '%s' does not seem to contain a 'meshfact'!", fname);
      return 0;
    }
    csRef<iMeshFactoryWrapper> t = Engine->CreateMeshFactory (
      	meshfactnode->GetAttributeValue ("name"));
    if (LoadMeshObjectFactory (ldr_context, t, 0, meshfactnode))
    {
      AddToRegion (ldr_context, t->QueryObject ());
      return csPtr<iMeshFactoryWrapper> (t);
    }
    else
    {
      // Error is already reported.
      Engine->GetMeshFactories ()->Remove (t);
      return 0;
    }
  }
  else
  {
    ReportError ("crystalspace.maploader.parse.plugin",
      "File does not appear to be a structured mesh factory (%s)!", fname);
    return 0;
  }
  return 0;
}

//---------------------------------------------------------------------------

csPtr<iMeshWrapper> csLoader::LoadMeshObject (const char* fname)
{
  if (!Engine) return 0;

  csRef<iFile> databuff (VFS->Open (fname, VFS_FILE_READ));
  csRef<iMeshWrapper> mesh;
  csRef<iLoaderContext> ldr_context = csPtr<iLoaderContext> (
	new StdLoaderContext (Engine, 0, true, this, false));

  if (!databuff || !databuff->GetSize ())
  {
    ReportError (
	      "crystalspace.maploader.parse.meshobject",
    	      "Could not open mesh object file '%s' on VFS!", fname);
    return 0;
  }

  csRef<iDocument> doc;
  bool er = LoadStructuredDoc (fname, databuff, doc);
  if (!er) return 0;
  if (doc)
  {
    csRef<iDocumentNode> meshobjnode = doc->GetRoot ()->GetNode ("meshobj");
    if (!meshobjnode)
    {
      ReportError (
	      "crystalspace.maploader.parse.map",
    	      "File '%s' does not seem to contain a 'meshobj'!", fname);
      return 0;
    }
    mesh = Engine->CreateMeshWrapper (
    	meshobjnode->GetAttributeValue ("name"));
    if (LoadMeshObject (ldr_context, mesh, 0, meshobjnode))
    {
      AddToRegion (ldr_context, mesh->QueryObject ());
    }
    else
    {
      // Error is already reported.
      Engine->GetMeshes ()->Remove (mesh);
      mesh = 0;
    }
  }
  else
  {
    ReportError ("crystalspace.maploader.parse.plugin",
      "File does not appear to be a structured mesh object (%s)!", fname);
    return 0;
  }
  return csPtr<iMeshWrapper> (mesh);
}

/************ iLoader implementation **************/

//--- Plugin stuff -----------------------------------------------------------

SCF_IMPLEMENT_IBASE(csLoader);
  SCF_IMPLEMENTS_INTERFACE(iLoader);
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent);
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY(csLoader)

CS_IMPLEMENT_PLUGIN

csLoader::csLoader (iBase *p)
{
  SCF_CONSTRUCT_IBASE(p);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  object_reg = 0;
  do_verbose = false;
}

csLoader::~csLoader()
{
  loaded_plugins.DeleteAll ();
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

#define GET_PLUGIN(var, intf, msgname)				\
  var = CS_QUERY_REGISTRY(object_reg, intf);			\
  if (!var && do_verbose) ReportNotify ("Could not get " msgname);

#define GET_CRITICAL_PLUGIN(var, intf, msgname)			\
  var = CS_QUERY_REGISTRY(object_reg, intf);			\
  if (!var) { ReportError ("crystalspace.maploader",		\
    "Failed to initialize loader: "				\
    "Could not get " msgname); return false; }

bool csLoader::Initialize (iObjectRegistry *object_Reg)
{
  csLoader::object_reg = object_Reg;
  loaded_plugins.SetObjectRegistry (object_reg);
  csRef<iVerbosityManager> verbosemgr (
    CS_QUERY_REGISTRY (object_reg, iVerbosityManager));
  if (verbosemgr) 
    do_verbose = verbosemgr->Enabled ("loader");
  else
    do_verbose = false;

  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));

  Reporter = CS_QUERY_REGISTRY (object_reg, iReporter);

  loaded_plugins.plugin_mgr = plugin_mgr;

  // Get the virtual file system plugin.
  GET_CRITICAL_PLUGIN (VFS, iVFS, "VFS");
  // Get syntax services.
  SyntaxService = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  if (!SyntaxService)
  {
    SyntaxService = CS_LOAD_PLUGIN (plugin_mgr,
    	"crystalspace.syntax.loader.service.text", iSyntaxService);
    if (!SyntaxService)
    {
      ReportError ("crystalspace.maploader",
	"Could not load the syntax services!");
      return false;
    }
    if (!object_reg->Register (SyntaxService, "iSyntaxService"))
    {
      ReportError ("crystalspace.maploader",
	"Could not register the syntax services!");
      return false;
    }
  }

  // Get all optional plugins.
  GET_PLUGIN (ImageLoader, iImageIO, "image loader");
  GET_PLUGIN (SoundLoader, iSoundLoader, "sound loader");
  GET_PLUGIN (Engine, iEngine, "engine");
  GET_PLUGIN (G3D, iGraphics3D, "video driver");
  GET_PLUGIN (SoundRender, iSoundRender, "sound driver");
  GET_PLUGIN (ModelConverter, iModelConverter, "model converter");
  GET_PLUGIN (CrossBuilder, iCrossBuilder, "model crossbuilder");

  InitTokenTable (xmltokens);

  stringSet = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.shared.stringset", iStringSet);

  return true;
}

//--- Parsing of Engine Objects ---------------------------------------------

bool csLoader::LoadMap (iLoaderContext* ldr_context, iDocumentNode* worldnode)
{
  if (!Engine)
  {
    SyntaxService->ReportError (
		"crystalspace.maploader.parse",
		worldnode, "The engine plugin is missing!");
    return false;
  }

  // Will be set to true if we find a <shader> section.
  bool shader_given = false;

  csRef<iDocumentNode> sequences;
  csRef<iDocumentNode> triggers;

  csRef<iDocumentNodeIterator> it = worldnode->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_SETTINGS:
	if (!LoadSettings (child))
	  return false;
	break;
      case XMLTOKEN_RENDERPRIORITIES:
	Engine->ClearRenderPriorities ();
	if (!LoadRenderPriorities (child))
	  return false;
	break;
      case XMLTOKEN_ADDON:
	if (!LoadAddOn (ldr_context, child, (iEngine*)Engine, false))
	  return false;
      	break;
      case XMLTOKEN_META:
	if (!LoadAddOn (ldr_context, child, (iEngine*)Engine, true))
	  return false;
      	break;
      case XMLTOKEN_MESHFACT:
        {
	  const char* name = child->GetAttributeValue ("name");
	  if (ldr_context->CheckDupes () && name)
	  {
	    iMeshFactoryWrapper* t = Engine->FindMeshFactory (name);
	    if (t) break;
	  }
          csRef<iMeshFactoryWrapper> t = Engine->CreateMeshFactory (name);
	  if (!t || !LoadMeshObjectFactory (ldr_context, t, 0, child))
	  {
	    // Error is already reported.
	    return false;
	  }
	  else
	  {
	    AddToRegion (ldr_context, t->QueryObject ());
	  }
        }
	break;
      case XMLTOKEN_REGION:
	SyntaxService->ReportError (
		"crystalspace.maploader.parse.region",
		worldnode, "<region> is no longer supported!");
	break;
      case XMLTOKEN_SECTOR:
        if (!ParseSector (ldr_context, child))
	  return false;
        break;
      case XMLTOKEN_COLLECTION:
        if (!ParseCollection (ldr_context, child))
	  return false;
        break;
      case XMLTOKEN_SEQUENCES:
	// Defer sequence parsing to later.
	sequences = child;
	break;
      case XMLTOKEN_TRIGGERS:
	// Defer trigger parsing to later.
	triggers = child;
	break;
      case XMLTOKEN_PLUGINS:
	if (!LoadPlugins (child))
	  return false;
	break;
      case XMLTOKEN_TEXTURES:
        if (!ParseTextureList (ldr_context, child))
          return false;
        break;
      case XMLTOKEN_MATERIALS:
        if (!ParseMaterialList (ldr_context, child))
          return false;
        break;
      case  XMLTOKEN_VARIABLES:
	if (!ParseVariableList (ldr_context, child))
	  return false;
	break;
      case XMLTOKEN_SOUNDS:
        if (!LoadSounds (child))
          return false;
        break;
      case XMLTOKEN_LIBRARY:
      {
	if (!LoadLibraryFromNode (ldr_context, child))
	  return false;
	break;
      }
      case XMLTOKEN_START:
      {
	const char* name = child->GetAttributeValue ("name");
	iCameraPosition* campos = Engine->GetCameraPositions ()->
	  	NewCameraPosition (name ? name : "Start");
	AddToRegion (ldr_context, campos->QueryObject ());
	if (!ParseStart (child, campos))
	  return false;
        break;
      }
      case XMLTOKEN_KEY:
      {
        iKeyValuePair* kvp = 0;
        SyntaxService->ParseKey (child, kvp);
        if (kvp)
        {
          Engine->QueryObject()->ObjAdd (kvp->QueryObject ());
	  kvp->DecRef ();
        }
	else
	  return false;
        break;
      }
      case XMLTOKEN_SHADERS:
	shader_given = true;
        ParseShaderList (ldr_context, child);
        break;
      default:
	SyntaxService->ReportBadToken (child);
	return false;
    }
  }

  // Sequences and triggers are parsed at the end because
  // all sectors and other objects need to be present.
  if (sequences)
    if (!LoadSequences (ldr_context, sequences))
      return false;
  if (triggers)
    if (!LoadTriggers (ldr_context, triggers))
      return false;

  return true;
}

bool csLoader::LoadLibraryFromNode (iLoaderContext* ldr_context,
	iDocumentNode* child)
{
  csRef<iVFS> vfs = CS_QUERY_REGISTRY(object_reg, iVFS);
  const char* name = child->GetAttributeValue ("checkdupes");
  bool dupes = ldr_context->CheckDupes ();
  if (name)
  {
    if (!strcasecmp (name, "true") || !strcasecmp (name, "yes") ||
	    	!strcasecmp (name, "1") || !strcasecmp (name, "on"))
      dupes = true;
    else
      dupes = false;
  }
  const char* file = child->GetAttributeValue ("file");
  if (file)
  {
    const char* path = child->GetAttributeValue ("path");
    if (path)
    {
      vfs->PushDir ();
      vfs->ChDir (path);
    }
    bool rc = LoadLibraryFile (file,
	  	  ldr_context->GetRegion (), ldr_context->CurrentRegionOnly (),
		  dupes);
    if (path)
    {
      vfs->PopDir ();
    }
    if (!rc)
      return false;
  }
  else
  {
    if (!LoadLibraryFile (child->GetContentsValue (),
	  	ldr_context->GetRegion (), ldr_context->CurrentRegionOnly (),
		ldr_context->CheckDupes ()))
    return false;
  }
  return true;
}

bool csLoader::LoadLibrary (iLoaderContext* ldr_context, iDocumentNode* libnode)
{
  if (!Engine)
  {
    SyntaxService->ReportError (
	  "crystalspace.maploader.parse.noengine",
	  libnode, "No engine present while in LoadLibrary!");
    return false;
  }
 
  csRef<iDocumentNode> sequences;
  csRef<iDocumentNode> triggers;

  csRef<iDocumentNodeIterator> it = libnode->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_LIBRARY:
      {
	if (!LoadLibraryFromNode (ldr_context, child))
	  return false;
	break;
      }
      case XMLTOKEN_ADDON:
	if (!LoadAddOn (ldr_context, child, (iEngine*)Engine, false))
	  return false;
      	break;
      case XMLTOKEN_META:
	if (!LoadAddOn (ldr_context, child, (iEngine*)Engine, true))
	  return false;
      	break;
      case XMLTOKEN_SEQUENCES:
	// Defer sequence parsing to later.
	sequences = child;
	break;
      case XMLTOKEN_TRIGGERS:
	// Defer trigger parsing to later.
	triggers = child;
	break;
      case XMLTOKEN_TEXTURES:
        // Append textures to engine.
        if (!ParseTextureList (ldr_context, child))
          return false;
        break;
      case XMLTOKEN_MATERIALS:
        if (!ParseMaterialList (ldr_context, child))
          return false;
        break;
      case XMLTOKEN_SHADERS:
        if (!ParseShaderList (ldr_context, child))
	  return false;
        break;
      case  XMLTOKEN_VARIABLES:
	if (!ParseVariableList (ldr_context, child))
	  return false;
	break;
      case XMLTOKEN_SOUNDS:
        if (!LoadSounds (child))
          return false;
        break;
      case XMLTOKEN_MESHREF:
        {
          iMeshWrapper* mesh = LoadMeshObjectFromFactory (ldr_context, child);
          if (!mesh)
	  {
	    // Error is already reported.
	    return false;
	  }
	  mesh->QueryObject ()->SetName (child->GetAttributeValue ("name"));
	  Engine->AddMeshAndChildren (mesh);
	  //mesh->DecRef ();
        }
        break;
      case XMLTOKEN_MESHOBJ:
        {
	  csRef<iMeshWrapper> mesh = Engine->CreateMeshWrapper (
			    child->GetAttributeValue ("name"));
          if (!LoadMeshObject (ldr_context, mesh, 0, child))
	  {
	    // Error is already reported.
	    return false;
	  }
	  else
	  {
	    AddToRegion (ldr_context, mesh->QueryObject ());
	  }
        }
        break;
      case XMLTOKEN_MESHFACT:
        {
          csRef<iMeshFactoryWrapper> t = Engine->CreateMeshFactory (
	    child->GetAttributeValue ("name"));
	  if (t)
	  {
	    if (!LoadMeshObjectFactory (ldr_context, t, 0, child))
	    {
	      // Error is already reported.
	      return false;
	    }
	    else
	    {
	      AddToRegion (ldr_context, t->QueryObject ());
	    }
	  }
	}
	break;
      case XMLTOKEN_PLUGINS:
	if (!LoadPlugins (child))
	  return false;
        break;
      default:
	SyntaxService->ReportBadToken (child);
        return false;
    }
  }

  // Sequences and triggers are parsed at the end because
  // all sectors and other objects need to be present.
  if (sequences)
    if (!LoadSequences (ldr_context, sequences))
      return false;
  if (triggers)
    if (!LoadTriggers (ldr_context, triggers))
      return false;

  return true;
}

bool csLoader::LoadPlugins (iDocumentNode* node)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_PLUGIN:
	loaded_plugins.NewPlugin (child->GetAttributeValue ("name"),
			child);
        break;
      default:
	SyntaxService->ReportBadToken (child);
	return false;
    }
  }

  return true;
}

bool csLoader::LoadSounds (iDocumentNode* node)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_SOUND:
        {
          const char* name = child->GetAttributeValue ("name");
          const char* filename = name;
          csRef<iDocumentNode> filenode = child->GetNode ("file");
          if (filenode)
          {
            filename = filenode->GetContentsValue ();
          }
          csRef<iSoundWrapper> snd = CS_GET_NAMED_CHILD_OBJECT (
                Engine->QueryObject (), iSoundWrapper, name);
          if (!snd)
            snd = LoadSound (name, filename);
          if (snd)
          {
            csRef<iDocumentNodeIterator> it2 (child->GetNodes ());
            while (it2->HasNext ())
            {
              csRef<iDocumentNode> child2 = it2->Next ();
              if (child2->GetType () != CS_NODE_ELEMENT) continue;
              switch (xmltokens.Request (child2->GetValue ()))
              {
                case XMLTOKEN_KEY:
                  {
                    iKeyValuePair *kvp = 0;
                    SyntaxService->ParseKey (child2, kvp);
                    if (kvp)
                    {
                      snd->QueryObject ()->ObjAdd (kvp->QueryObject ());
                      kvp->DecRef ();
                    } else
                      return false;
                  }
                  break;
              }
            }
          }
        }
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return false;
    }
  }

  return true;
}

bool csLoader::LoadLodControl (iLODControl* lodctrl, iDocumentNode* node)
{
  lodctrl->SetLOD (0, 1);
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_DISTANCE:
	{
	  csRef<iDocumentAttribute> at;
	  at = child->GetAttribute ("varm");
	  if (at)
	  {
	    // We use variables.
	    iSharedVariable *varm = Engine->GetVariableList()->FindByName (
	    	child->GetAttributeValue ("varm"));
	    iSharedVariable *vara = Engine->GetVariableList()->FindByName (
	    	child->GetAttributeValue ("vara"));
	    lodctrl->SetLOD (varm, vara);
	    break;
	  }

	  at = child->GetAttribute ("m");
	  if (at)
	  {
	    float lodm = child->GetAttributeValueAsFloat ("m");
	    float loda = child->GetAttributeValueAsFloat ("a");
	    lodctrl->SetLOD (lodm, loda);
	  }
	  else
	  {
	    float d0 = child->GetAttributeValueAsFloat ("d0");
	    float d1 = child->GetAttributeValueAsFloat ("d1");
	    float lodm = 1.0 / (d1-d0);
	    float loda = -lodm * d0;
	    lodctrl->SetLOD (lodm, loda);
	  }
	}
        break;
      default:
	SyntaxService->ReportBadToken (child);
        return false;
    }
  }

  return true;
}

//--------------------------------------------------------------------

// Private class implementing iPolygonMesh for a general triangle mesh.
class PolygonMeshMesh : public iPolygonMesh
{
private:
  csVector3* vertices;
  csMeshedPolygon* polygons;
  csTriangle* vertex_indices;
  int num_verts;
  int num_tris;
  csFlags flags;

public:
  PolygonMeshMesh (int num_verts, int num_tris)
  {
    SCF_CONSTRUCT_IBASE (0);
    PolygonMeshMesh::num_verts = num_verts;
    PolygonMeshMesh::num_tris = num_tris;
    vertices = new csVector3[num_verts];
    polygons = new csMeshedPolygon[num_tris];
    vertex_indices = new csTriangle[num_tris];

    int i;
    for (i = 0 ; i < num_tris ; i++)
    {
      polygons[i].num_vertices = 3;
      polygons[i].vertices = (int*)&vertex_indices[i];
    }
    flags.Set (CS_POLYMESH_TRIANGLEMESH);
  }
  virtual ~PolygonMeshMesh ()
  {
    delete[] vertices;
    delete[] polygons;
    delete[] vertex_indices;
    SCF_DESTRUCT_IBASE();
  }

  SCF_DECLARE_IBASE;

  virtual int GetVertexCount () { return num_verts; }
  virtual csVector3* GetVertices () { return vertices; }
  virtual int GetPolygonCount () { return num_tris; }
  virtual csMeshedPolygon* GetPolygons () { return polygons; }
  virtual int GetTriangleCount () { return num_tris; }
  virtual csTriangle* GetTriangles () { return vertex_indices; }
  virtual void Lock () { }
  virtual void Unlock () { }
  virtual csFlags& GetFlags () { return flags; }
  virtual uint32 GetChangeNumber () const { return 0; }
};

SCF_IMPLEMENT_IBASE (PolygonMeshMesh)
  SCF_IMPLEMENTS_INTERFACE (iPolygonMesh)
SCF_IMPLEMENT_IBASE_END

bool csLoader::ParsePolyMeshChildBox (iDocumentNode* child,
	csRef<iPolygonMesh>& polymesh)
{
  csBox3 b;
  if (!SyntaxService->ParseBox (child, b))
    return false;
  polymesh = csPtr<iPolygonMesh> (new csPolygonMeshBox (b));
  return true;
}

bool csLoader::ParsePolyMeshChildMesh (iDocumentNode* child,
	csRef<iPolygonMesh>& polymesh)
{
  int num_vt = 0;
  int num_tri = 0;
  csRef<iDocumentNodeIterator> child_it = child->GetNodes ();
  while (child_it->HasNext ())
  {
    csRef<iDocumentNode> child_child = child_it->Next ();
    if (child_child->GetType () != CS_NODE_ELEMENT) continue;
    const char* child_value = child_child->GetValue ();
    csStringID child_id = xmltokens.Request (child_value);
    switch (child_id)
    {
      case XMLTOKEN_V: num_vt++; break;
      case XMLTOKEN_T: num_tri++; break;
      default:
	SyntaxService->ReportBadToken (child_child);
	return false;
    }
  }

  polymesh = csPtr<iPolygonMesh> (new PolygonMeshMesh (num_vt, num_tri));
  csVector3* vt = polymesh->GetVertices ();
  csMeshedPolygon* po = polymesh->GetPolygons ();
  num_vt = 0;
  num_tri = 0;

  child_it = child->GetNodes ();
  while (child_it->HasNext ())
  {
    csRef<iDocumentNode> child_child = child_it->Next ();
    if (child_child->GetType () != CS_NODE_ELEMENT) continue;
    const char* child_value = child_child->GetValue ();
    csStringID child_id = xmltokens.Request (child_value);
    switch (child_id)
    {
      case XMLTOKEN_V:
	if (!SyntaxService->ParseVector (child_child, vt[num_vt]))
	  return false;
	num_vt++;
	break;
      case XMLTOKEN_T:
	po[num_tri].vertices[0] = child_child->GetAttributeValueAsInt ("v1");
	po[num_tri].vertices[1] = child_child->GetAttributeValueAsInt ("v2");
	po[num_tri].vertices[2] = child_child->GetAttributeValueAsInt ("v3");
	num_tri++;
	break;
      default:
	SyntaxService->ReportBadToken (child_child);
	return false;
    }
  }
  return true;
}

bool csLoader::ParsePolyMesh (iDocumentNode* node, iObjectModel* objmodel)
{
  csRef<iPolygonMesh> polymesh;
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  bool colldet = false;
  bool viscull = false;
  bool shadows = false;
  bool convex = false;
  bool notconvex = false;
  bool closed = false;
  bool notclosed = false;
  bool use_default_mesh = false;
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_DEFAULT:
        if (polymesh)
	{
	  SyntaxService->ReportError (
	    "crystalspace.maploader.parse.polymesh", child,
	    "Use either <default>, <box>, or <mesh>!");
	  return false;
	}
	use_default_mesh = true;
        break;
      case XMLTOKEN_BOX:
        if (polymesh || use_default_mesh)
	{
	  SyntaxService->ReportError (
	    "crystalspace.maploader.parse.polymesh", child,
	    "Use either <default>, <box>, or <mesh>!");
	  return false;
	}
        if (!ParsePolyMeshChildBox (child, polymesh))
	  return false;
        break;
      case XMLTOKEN_MESH:
        if (polymesh || use_default_mesh)
	{
	  SyntaxService->ReportError (
	    "crystalspace.maploader.parse.polymesh", child,
	    "Use either <default>, <box>, or <mesh>!");
	  return false;
	}
        if (!ParsePolyMeshChildMesh (child, polymesh))
	  return false;
        break;
      case XMLTOKEN_CLOSED:
        closed = true;
	notclosed = false;
        break;
      case XMLTOKEN_NOTCLOSED:
        closed = false;
        notclosed = true;
        break;
      case XMLTOKEN_CONVEX:
        convex = true;
	notconvex = false;
        break;
      case XMLTOKEN_NOTCONVEX:
        convex = false;
        notconvex = true;
        break;
      case XMLTOKEN_COLLDET:
        colldet = true;
	break;
      case XMLTOKEN_VISCULL:
        viscull = true;
	break;
      case XMLTOKEN_SHADOWS:
        shadows = true;
	break;
      default:
	SyntaxService->ReportBadToken (child);
        return false;
    }
  }
  if (!colldet && !viscull && !shadows)
  {
    SyntaxService->ReportError (
	"crystalspace.maploader.parse.polymesh",
	node, "Please specify either <shadows/>, <viscull/> or <colldet/>!");
    return false;
  }
#undef CS_CLO
#undef CS_CON
#define CS_CLO (CS_POLYMESH_CLOSED|CS_POLYMESH_NOTCLOSED)
#define CS_CON (CS_POLYMESH_CONVEX|CS_POLYMESH_NOTCONVEX)
  if (use_default_mesh)
  {
    if (colldet && objmodel->GetPolygonMeshColldet ())
    {
      csFlags& flags = objmodel->GetPolygonMeshColldet ()->GetFlags ();
      if (closed) flags.Set (CS_CLO, CS_POLYMESH_CLOSED);
      if (notclosed) flags.Set (CS_CLO, CS_POLYMESH_NOTCLOSED);
      if (convex) flags.Set (CS_CON, CS_POLYMESH_CONVEX);
      if (notconvex) flags.Set (CS_CON, CS_POLYMESH_NOTCONVEX);
    }
    if (viscull && objmodel->GetPolygonMeshViscull ())
    {
      csFlags& flags = objmodel->GetPolygonMeshViscull ()->GetFlags ();
      if (closed) flags.Set (CS_CLO, CS_POLYMESH_CLOSED);
      if (notclosed) flags.Set (CS_CLO, CS_POLYMESH_NOTCLOSED);
      if (convex) flags.Set (CS_CON, CS_POLYMESH_CONVEX);
      if (notconvex) flags.Set (CS_CON, CS_POLYMESH_NOTCONVEX);
    }
    if (shadows && objmodel->GetPolygonMeshShadows ())
    {
      csFlags& flags = objmodel->GetPolygonMeshShadows ()->GetFlags ();
      if (closed) flags.Set (CS_CLO, CS_POLYMESH_CLOSED);
      if (notclosed) flags.Set (CS_CLO, CS_POLYMESH_NOTCLOSED);
      if (convex) flags.Set (CS_CON, CS_POLYMESH_CONVEX);
      if (notconvex) flags.Set (CS_CON, CS_POLYMESH_NOTCONVEX);
    }
  }
  else
  {
    if (polymesh)
    {
      csFlags& flags = polymesh->GetFlags ();
      if (closed) flags.Set (CS_CLO, CS_POLYMESH_CLOSED);
      if (notclosed) flags.Set (CS_CLO, CS_POLYMESH_NOTCLOSED);
      if (convex) flags.Set (CS_CON, CS_POLYMESH_CONVEX);
      if (notconvex) flags.Set (CS_CON, CS_POLYMESH_NOTCONVEX);
    }

    if (colldet)
      objmodel->SetPolygonMeshColldet (polymesh);
    if (viscull)
      objmodel->SetPolygonMeshViscull (polymesh);
    if (shadows)
      objmodel->SetPolygonMeshShadows (polymesh);
  }
#undef CS_CLO
#undef CS_CON

  return true;
}

bool csLoader::LoadMeshObjectFactory (iLoaderContext* ldr_context,
	iMeshFactoryWrapper* stemp, iMeshFactoryWrapper* parent,
	iDocumentNode* node, csReversibleTransform* transf)
{
  iLoaderPlugin* plug = 0;
  iBinaryLoaderPlugin* binplug = 0;
  iMaterialWrapper *mat = 0;
  bool staticshape = false;
  csRef<iDocumentNodeIterator> prev_it;
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (true)
  {
    if (!it->HasNext ())
    {
      // Iterator has finished. Check if we still have to continue
      // with the normal iterator first (non-defaults).
      if (!prev_it) break;
      it = prev_it;
      prev_it = 0;
      continue;
    }

    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_LOD:
        {
	  if (!stemp->GetMeshObjectFactory ())
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.meshfactory",
              child, "Please use 'params' before specifying LOD!");
	    return false;
	  }
	  csRef<iLODControl> lodctrl (SCF_QUERY_INTERFACE (
	    	stemp->GetMeshObjectFactory (),
		iLODControl));
	  if (!lodctrl)
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.meshfactory",
              child, "This mesh factory doesn't implement LOD control!");
	    return false;
	  }
	  if (!LoadLodControl (lodctrl, child))
	    return false;
	}
        break;
      case XMLTOKEN_KEY:
        {
          iKeyValuePair* kvp = 0;
          SyntaxService->ParseKey (child, kvp);
	  if (kvp)
          {
            stemp->QueryObject()->ObjAdd (kvp->QueryObject ());
	    kvp->DecRef ();
          } else
	    return false;
        }
        break;
      case XMLTOKEN_ADDON:
	if (!LoadAddOn (ldr_context, child, stemp, false))
	  return false;
      	break;
      case XMLTOKEN_META:
	if (!LoadAddOn (ldr_context, child, stemp, true))
	  return false;
      	break;
      case XMLTOKEN_LODLEVEL:
        {
          if (!parent)
	  {
	    SyntaxService->ReportError (
	  	    "crystalspace.maploader.load.meshfactory", child,
		    "Factory must be part of a hierarchy for <lodlevel>!");
	    return false;
	  }
	  parent->AddFactoryToStaticLOD (child->GetContentsValueAsInt (),
	  	stemp);
        }
        break;
      case XMLTOKEN_STATICLOD:
        {
	  iLODControl* lodctrl = stemp->CreateStaticLOD ();
	  if (!LoadLodControl (lodctrl, child))
	    return false;
        }
        break;
      case XMLTOKEN_STATICSHAPE:
        if (!SyntaxService->ParseBool (child, staticshape, true))
	  return false;
        break;
      case XMLTOKEN_NULLMESH:
        {
	  if (plug)
	  {
            SyntaxService->ReportError (
	        "crystalspace.maploader.load.plugin",
                child, "Don't specify the plugin if you use <nullmesh>!");
	    return false;
	  }
	  if (stemp->GetMeshObjectFactory ())
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.meshfactory", child,
	      "Please don't use <params> in combination with <nullmesh>!");
	    return false;
	  }
	  csRef<iPluginManager> plugin_mgr =
	  	CS_QUERY_REGISTRY (object_reg, iPluginManager);
	  csRef<iMeshObjectType> type = CS_QUERY_PLUGIN_CLASS (
		plugin_mgr, "crystalspace.mesh.object.null",
		iMeshObjectType);
	  if (!type)
	    type = CS_LOAD_PLUGIN (plugin_mgr,
	    	"crystalspace.mesh.object.null", iMeshObjectType);
	  if (!type)
	  {
            SyntaxService->ReportError (
	        "crystalspace.maploader.load.plugin",
                child, "Could not find the nullmesh plugin!");
	    return false;
	  }
	  csRef<iMeshObjectFactory> fact = type->NewFactory ();
	  stemp->SetMeshObjectFactory (fact);
	  fact->SetLogicalParent (stemp);
	  csBox3 b;
	  if (!SyntaxService->ParseBox (child, b))
	    return false;
	  csRef<iNullFactoryState> nullmesh = SCF_QUERY_INTERFACE (
		fact, iNullFactoryState);
	  nullmesh->SetBoundingBox (b);
	}
        break;
      case XMLTOKEN_PARAMS:
	if (!plug)
	{
          SyntaxService->ReportError (
	      "crystalspace.maploader.load.plugin",
              child, "Could not load plugin!");
	  return false;
	}
	else
	{
	  // We give here the iMeshObjectFactory as the context. If this
	  // is a new factory this will be 0. Otherwise it is possible
	  // to append information to the already loaded factory.
	  csRef<iBase> mof (plug->Parse (child, ldr_context,
	  	stemp->GetMeshObjectFactory ()));
	  if (!mof)
	  {
	    // Error is reported by plug->Parse().
	    return false;
	  }
	  else
	  {
	    csRef<iMeshObjectFactory> mof2 (SCF_QUERY_INTERFACE (mof,
	    	iMeshObjectFactory));
	    if (!mof2)
	    {
              SyntaxService->ReportError (
	        "crystalspace.maploader.parse.meshfactory",
		child,
		"Returned object does not implement iMeshObjectFactory!");
	      return false;
	    }
	    stemp->SetMeshObjectFactory (mof2);
	    mof2->SetLogicalParent (stemp);
	  }
	}
        break;
      case XMLTOKEN_PARAMSFILE:
	if (!plug && !binplug)
	{
          SyntaxService->ReportError (
	      "crystalspace.maploader.load.plugin",
              child, "Could not load plugin!");
	  return false;
	}
	else
        {
          csRef<iFile> buf (VFS->Open (child->GetContentsValue (),
	  	VFS_FILE_READ));
	  if (!buf)
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.loadingfile",
	      child, "Error opening file '%s'!", child->GetContentsValue ());
	    return false;
	  }
	  // We give here the iMeshObjectFactory as the context. If this
	  // is a new factory this will be 0. Otherwise it is possible
	  // to append information to the already loaded factory.
	  csRef<iBase> mof;
	  if (plug)
	    mof = LoadStructuredMap (ldr_context,
	    	plug, buf, stemp->GetMeshObjectFactory (),
	    	child->GetContentsValue ());
	  else
	  {
	    csRef<iDataBuffer> dbuf = VFS->ReadFile (
	    	child->GetContentsValue ());
	    mof = binplug->Parse ((void*)(dbuf->GetUint8 ()),
	  	ldr_context, stemp->GetMeshObjectFactory ());
	  }
	  if (!mof)
	  {
	    // Error is reported by plug->Parse().
	    return false;
	  }
	  else
	  {
	    csRef<iMeshObjectFactory> mof2 (SCF_QUERY_INTERFACE (mof,
	    	iMeshObjectFactory));
	    if (!mof2)
	    {
              SyntaxService->ReportError (
	        "crystalspace.maploader.parse.meshfactory",
		child,
		"Returned object does not implement iMeshObjectFactory!");
	      return false;
	    }
	    stemp->SetMeshObjectFactory (mof2);
	    mof2->SetLogicalParent (stemp);
	  }
        }
        break;

      case XMLTOKEN_POLYMESH:
        {
	  if (!stemp->GetMeshObjectFactory ())
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.meshfactory",
              child, "Please use 'params' before specifying 'polymesh'!");
	    return false;
	  }
	  iObjectModel* objmodel = stemp->GetMeshObjectFactory ()
	  	->GetObjectModel ();
	  if (!objmodel)
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.meshfactory", child,
	      "This factory doesn't support setting of other 'polymesh'!");
	    return false;
	  }
	  if (!ParsePolyMesh (child, objmodel))
	  {
	    // Error already reported.
	    return false;
	  }
	}
	break;

      case XMLTOKEN_CLOSED:
	if (!stemp->GetMeshObjectFactory ())
	{
          SyntaxService->ReportError (
	    "crystalspace.maploader.parse.meshfactory",
            child, "Please use 'params' before specifying 'closed'!");
	  return false;
	}
	else
        {
	  iObjectModel* objmodel = stemp->GetMeshObjectFactory ()
	  	->GetObjectModel ();
          if (objmodel->GetPolygonMeshShadows ())
            objmodel->GetPolygonMeshShadows ()->GetFlags ().Set (
	      CS_POLYMESH_CLOSED | CS_POLYMESH_NOTCLOSED, CS_POLYMESH_CLOSED);
          if (objmodel->GetPolygonMeshViscull ())
            objmodel->GetPolygonMeshViscull ()->GetFlags ().Set (
	      CS_POLYMESH_CLOSED | CS_POLYMESH_NOTCLOSED, CS_POLYMESH_CLOSED);
          if (objmodel->GetPolygonMeshShadows ())
            objmodel->GetPolygonMeshShadows ()->GetFlags ().Set (
	      CS_POLYMESH_CLOSED | CS_POLYMESH_NOTCLOSED, CS_POLYMESH_CLOSED);
        }
        break;
      case XMLTOKEN_CONVEX:
	if (!stemp->GetMeshObjectFactory ())
	{
          SyntaxService->ReportError (
	    "crystalspace.maploader.parse.meshfactory",
            child, "Please use 'params' before specifying 'convex'!");
	  return false;
	}
	else
        {
	  iObjectModel* objmodel = stemp->GetMeshObjectFactory ()
	  	->GetObjectModel ();
          if (objmodel->GetPolygonMeshShadows ())
            objmodel->GetPolygonMeshShadows ()->GetFlags ().Set (
	      CS_POLYMESH_CONVEX | CS_POLYMESH_NOTCONVEX, CS_POLYMESH_CONVEX);
          if (objmodel->GetPolygonMeshViscull ())
            objmodel->GetPolygonMeshViscull ()->GetFlags ().Set (
	      CS_POLYMESH_CONVEX | CS_POLYMESH_NOTCONVEX, CS_POLYMESH_CONVEX);
          if (objmodel->GetPolygonMeshShadows ())
            objmodel->GetPolygonMeshShadows ()->GetFlags ().Set (
	      CS_POLYMESH_CONVEX | CS_POLYMESH_NOTCONVEX, CS_POLYMESH_CONVEX);
        }
        break;
      case XMLTOKEN_MATERIAL:
        {
	  if (!stemp->GetMeshObjectFactory ())
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.meshfactory",
              child, "Please use 'params' before specifying 'material'!");
	    return false;
	  }
	  const char* matname = child->GetContentsValue ();
          mat = ldr_context->FindMaterial (matname);
          if (mat)
	  {
	    csRef<iSprite3DFactoryState> state (SCF_QUERY_INTERFACE (
	    	stemp->GetMeshObjectFactory (),
		iSprite3DFactoryState));
	    //@@@@@@@@@@@@@@@@@@@@ Use SetMaterial() on iMeshObjectFactory!
	    if (!state)
	    {
              SyntaxService->ReportError (
	        "crystalspace.maploader.parse.meshfactory",
                child, "Only use MATERIAL keyword with 3D sprite factories!");
	      return false;
	    }
            state->SetMaterialWrapper (mat);
	  }
          else
          {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.unknownmaterial",
              child, "Material '%s' not found!", matname);
	    return false;
          }
        }
        break;

      case XMLTOKEN_FILE:
        {
          if (!ModelConverter || !CrossBuilder) return false;

	  const char* filename = child->GetContentsValue ();
          csRef<iDataBuffer> buf (VFS->ReadFile (filename));
	  if (!buf)
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.loadingmodel",
	      child, "Error opening file model '%s'!", filename);
	    return false;
	  }

	  csRef<iModelData> Model (ModelConverter->Load (buf->GetUint8 (),
	  	buf->GetSize ()));
          if (!Model)
	  {
            SyntaxService->ReportError (
 	      "crystalspace.maploader.parse.loadingmodel",
	      child, "Error loading file model '%s'!", filename);
	    return false;
	  }

	  csModelDataTools::SplitObjectsByMaterial (Model);
	  csModelDataTools::MergeObjects (Model, false);
	  iMeshFactoryWrapper *stemp2 =
	    CrossBuilder->BuildSpriteFactoryHierarchy (Model, Engine, mat);

	  stemp->SetMeshObjectFactory (stemp2->GetMeshObjectFactory ());
	  int i;
	  iMeshFactoryList* mfl2 = stemp2->GetChildren ();
	  iMeshFactoryList* mfl = stemp->GetChildren ();
	  for (i=0; i<mfl2->GetCount (); i++)
	    mfl->Add (mfl2->Get (i));
	  stemp2->DecRef ();
        }
        break;

      case XMLTOKEN_PLUGIN:
        {
	  if (prev_it || plug || binplug)
	  {
	    SyntaxService->ReportError (
 	        "crystalspace.maploader.parse.meshfact",
	        child, "Please specify only one plugin!");
	    return false;
	  }

	  iDocumentNode* defaults = 0;
	  if (!loaded_plugins.FindPlugin (child->GetContentsValue (),
		plug, binplug, defaults))
	  {
	    SyntaxService->ReportError (
 	        "crystalspace.maploader.parse.meshfact",
	        child, "Error loading plugin '%s'!", child->GetContentsValue ());
	    return false;
	  }
	  if (defaults)
	  {
	    // Set aside current iterator and start a new one.
	    prev_it = it;
	    it = defaults->GetNodes ();
	  }
	}
        break;

      case XMLTOKEN_MESHFACT:
        {
          csRef<iMeshFactoryWrapper> t = Engine->CreateMeshFactory (
	  	child->GetAttributeValue ("name"));
	  csReversibleTransform child_transf;
          if (!LoadMeshObjectFactory (ldr_context, t, stemp, child,
	  	&child_transf))
	  {
	    // Error is already reported above.
	    return false;
	  }
	  else
	  {
	    AddToRegion (ldr_context, t->QueryObject ());
	  }
	  stemp->GetChildren ()->Add (t);
	  t->SetTransform (child_transf);
        }
	break;

      case XMLTOKEN_MOVE:
        {
	  if (!transf)
	  {
	    SyntaxService->ReportError (
	    	"crystalspace.maploader.load.meshfactory",
		child,
		"'move' is only useful for hierarchical transformations!");
	    return false;
	  }
	  csRef<iDocumentNode> matrix_node = child->GetNode ("matrix");
	  if (matrix_node)
	  {
	    csMatrix3 m;
	    if (!SyntaxService->ParseMatrix (matrix_node, m))
	      return false;
            transf->SetO2T (m);
	  }
	  csRef<iDocumentNode> vector_node = child->GetNode ("v");
	  if (vector_node)
	  {
	    csVector3 v;
	    if (!SyntaxService->ParseVector (vector_node, v))
	      return false;
            transf->SetO2TTranslation (v);
	  }
        }
        break;
      case XMLTOKEN_HARDMOVE:
        {
	  if (!stemp->GetMeshObjectFactory ())
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.meshfactory",
              child, "Please use 'params' before specifying 'hardmove'!");
	    return false;
	  }
	  if (!stemp->GetMeshObjectFactory ()->SupportsHardTransform ())
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.meshfactory",
              child, "This factory doesn't support 'hardmove'!");
	    return false;
	  }
	  csReversibleTransform tr;
	  csRef<iDocumentNode> matrix_node = child->GetNode ("matrix");
	  if (matrix_node)
	  {
	    csMatrix3 m;
	    if (!SyntaxService->ParseMatrix (matrix_node, m))
	      return false;
            tr.SetT2O (m);
	  }
	  csRef<iDocumentNode> vector_node = child->GetNode ("v");
	  if (vector_node)
	  {
	    csVector3 v;
	    if (!SyntaxService->ParseVector (vector_node, v))
	      return false;
            tr.SetOrigin (v);
	  }
	  stemp->HardTransform (tr);
        }
        break;

      case XMLTOKEN_ZUSE:
        stemp->SetZBufMode (CS_ZBUF_USE);
        break;
      case XMLTOKEN_ZFILL:
        stemp->SetZBufMode (CS_ZBUF_FILL);
        break;
      case XMLTOKEN_ZNONE:
        stemp->SetZBufMode (CS_ZBUF_NONE);
        break;
      case XMLTOKEN_ZTEST:
        stemp->SetZBufMode (CS_ZBUF_TEST);
        break;
      case XMLTOKEN_PRIORITY:
        stemp->SetRenderPriority (
		Engine->GetRenderPriority (child->GetContentsValue ()));
        break;
      case XMLTOKEN_SHADERVAR:
	{
	  if (!stemp->GetMeshObjectFactory ())
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.meshfactory",
              child, "Please use 'params' before specifying 'shadervar'!");
	    return false;
	  }
	  csRef<iShaderVariableContext> svc = stemp->GetSVContext();
	  CS_ASSERT (svc.IsValid());
	  //create a new variable
	  const char* varname = child->GetAttributeValue ("name");
	  csRef<csShaderVariable> var;
	  var.AttachNew (new csShaderVariable (stringSet->Request (varname)));
	  if (!SyntaxService->ParseShaderVar (child, *var))
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.load.meshobject", child,
	      "Error loading shader variable '%s' in mesh factory '%s'.", 
	      varname, stemp->QueryObject()->GetName());
	    break;
	  }
	  svc->AddVariable (var);
	}
	break;
      default:
	SyntaxService->ReportBadToken (child);
        return false;
    }
  }

  stemp->GetMeshObjectFactory ()->GetFlags ().SetBool (CS_FACTORY_STATICSHAPE,
  	staticshape);

  return true;
}

// Return true if the matrix does not scale.
static bool TestOrthoMatrix (csMatrix3& m)
{
  // Test if the matrix does not scale. Scaling meshes is illegal
  // in CS (must be done through hardmove).
  csVector3 v = m * csVector3 (1, 1, 1);
  float norm = v.Norm ();
  float desired_norm = 1.7320508f;
  return ABS (norm-desired_norm) < 0.01f;
}

bool csLoader::HandleMeshParameter (iLoaderContext* ldr_context,
	iMeshWrapper* mesh, iMeshWrapper* parent, iDocumentNode* child,
	csStringID id, bool& handled, char*& priority,
	bool do_portal_container, bool& staticpos, bool& staticshape,
	bool& zmodeChanged, bool& prioChanged,
	bool recursive)
{
#undef TEST_MISSING_MESH
#define TEST_MISSING_MESH \
  if (!mesh) \
  { \
    SyntaxService->ReportError ( \
	"crystalspace.maploader.load.meshobject", \
	child, do_portal_container ? "Specify at least one portal first!" : \
	"First specify the parent factory with 'factory'!"); \
    return false; \
  }

  handled = true;
  switch (id)
  {
    case XMLTOKEN_STATICPOS:
      if (!SyntaxService->ParseBool (child, staticpos, true))
	return false;
      break;
    case XMLTOKEN_STATICSHAPE:
      if (!SyntaxService->ParseBool (child, staticshape, true))
	return false;
      break;
    case XMLTOKEN_STATICLOD:
      {
	TEST_MISSING_MESH
	iLODControl* lodctrl = mesh->CreateStaticLOD ();
	if (!LoadLodControl (lodctrl, child))
	  return false;
      }
      break;
    case XMLTOKEN_LODLEVEL:
      {
	TEST_MISSING_MESH
        if (!parent)
	{
	  SyntaxService->ReportError (
	  	  "crystalspace.maploader.load.meshobject", child,
		  "Mesh must be part of a hierarchical mesh for <lodlevel>!");
	  return false;
	}
        if (!parent->GetStaticLOD ())
	{
	  SyntaxService->ReportError (
	  	  "crystalspace.maploader.load.meshobject", child,
		  "Parent mesh must use <staticlod>!");
	  return false;
	}
	parent->AddMeshToStaticLOD (child->GetContentsValueAsInt (), mesh);
      }
      break;
    case XMLTOKEN_LOD:
      {
	TEST_MISSING_MESH
	if (!mesh->GetMeshObject ())
	{
          SyntaxService->ReportError (
	      "crystalspace.maploader.parse.meshobject",
              child, "Mesh object is missing!");
	  return false;
	}
	csRef<iLODControl> lodctrl (SCF_QUERY_INTERFACE (
	    	mesh->GetMeshObject (),
		iLODControl));
	if (!lodctrl)
	{
          SyntaxService->ReportError (
	      "crystalspace.maploader.parse.meshobject",
              child, "This mesh doesn't implement LOD control!");
	  return false;
	}
	if (!LoadLodControl (lodctrl, child))
	  return false;
      }
      break;
    case XMLTOKEN_PRIORITY:
      delete[] priority;
      priority = csStrNew (child->GetContentsValue ());
      if (recursive)
        mesh->SetRenderPriorityRecursive (Engine->GetRenderPriority (priority));
      else
        mesh->SetRenderPriority (Engine->GetRenderPriority (priority));
      prioChanged = true;
      break;
    case XMLTOKEN_ADDON:
      TEST_MISSING_MESH
      if (!LoadAddOn (ldr_context, child, mesh, false))
	return false;
      break;
    case XMLTOKEN_META:
      TEST_MISSING_MESH
      if (!LoadAddOn (ldr_context, child, mesh, true))
	return false;
      break;
    case XMLTOKEN_NOLIGHTING:
      TEST_MISSING_MESH
      if (recursive)
        mesh->SetFlagsRecursive (CS_ENTITY_NOLIGHTING, CS_ENTITY_NOLIGHTING);
      else
        mesh->GetFlags ().Set (CS_ENTITY_NOLIGHTING, CS_ENTITY_NOLIGHTING);
      break;
    case XMLTOKEN_NOSHADOWS:
      TEST_MISSING_MESH
      if (recursive)
        mesh->SetFlagsRecursive (CS_ENTITY_NOSHADOWS, CS_ENTITY_NOSHADOWS);
      else
        mesh->GetFlags ().Set (CS_ENTITY_NOSHADOWS, CS_ENTITY_NOSHADOWS);
      break;
    case XMLTOKEN_NOCLIP:
      TEST_MISSING_MESH
      if (recursive)
        mesh->SetFlagsRecursive (CS_ENTITY_NOCLIP, CS_ENTITY_NOCLIP);
      else
        mesh->GetFlags ().Set (CS_ENTITY_NOCLIP, CS_ENTITY_NOCLIP);
      break;
    case XMLTOKEN_NOHITBEAM:
      TEST_MISSING_MESH
      if (recursive)
        mesh->SetFlagsRecursive (CS_ENTITY_NOHITBEAM, CS_ENTITY_NOHITBEAM);
      else
        mesh->GetFlags ().Set (CS_ENTITY_NOHITBEAM, CS_ENTITY_NOHITBEAM);
      break;
    case XMLTOKEN_INVISIBLEMESH:
      TEST_MISSING_MESH
      if (recursive)
        mesh->SetFlagsRecursive (CS_ENTITY_INVISIBLEMESH,
		CS_ENTITY_INVISIBLEMESH);
      else
        mesh->GetFlags ().Set (CS_ENTITY_INVISIBLEMESH,
		CS_ENTITY_INVISIBLEMESH);
      break;
    case XMLTOKEN_INVISIBLE:
      TEST_MISSING_MESH
      if (recursive)
        mesh->SetFlagsRecursive (CS_ENTITY_INVISIBLE, CS_ENTITY_INVISIBLE);
      else
        mesh->GetFlags ().Set (CS_ENTITY_INVISIBLE, CS_ENTITY_INVISIBLE);
      break;
    case XMLTOKEN_DETAIL:
      TEST_MISSING_MESH
      if (recursive)
        mesh->SetFlagsRecursive (CS_ENTITY_DETAIL, CS_ENTITY_DETAIL);
      else
        mesh->GetFlags ().Set (CS_ENTITY_DETAIL, CS_ENTITY_DETAIL);
      break;
    case XMLTOKEN_IMPOSTER:
      TEST_MISSING_MESH
      if (!ParseImposterSettings (mesh, child))
        return false;
      break;
    case XMLTOKEN_ZFILL:
      TEST_MISSING_MESH
      if (!priority) priority = csStrNew ("wall");
      if (recursive)
      {
        mesh->SetRenderPriorityRecursive (Engine->GetRenderPriority (priority));
        mesh->SetZBufModeRecursive (CS_ZBUF_FILL);
      }
      else
      {
        mesh->SetRenderPriority (Engine->GetRenderPriority (priority));
        mesh->SetZBufMode (CS_ZBUF_FILL);
      }
      zmodeChanged = true;
      break;
    case XMLTOKEN_ZUSE:
      TEST_MISSING_MESH
      if (!priority) priority = csStrNew ("object");
      if (recursive)
      {
        mesh->SetRenderPriorityRecursive (Engine->GetRenderPriority (priority));
        mesh->SetZBufModeRecursive (CS_ZBUF_USE);
      }
      else
      {
        mesh->SetRenderPriority (Engine->GetRenderPriority (priority));
        mesh->SetZBufMode (CS_ZBUF_USE);
      }
      zmodeChanged = true;
      break;
    case XMLTOKEN_ZNONE:
      TEST_MISSING_MESH
      if (!priority) priority = csStrNew ("sky");
      if (recursive)
      {
        mesh->SetRenderPriorityRecursive (Engine->GetRenderPriority (priority));
        mesh->SetZBufModeRecursive (CS_ZBUF_NONE);
      }
      else
      {
        mesh->SetRenderPriority (Engine->GetRenderPriority (priority));
        mesh->SetZBufMode (CS_ZBUF_NONE);
      }
      zmodeChanged = true;
      break;
    case XMLTOKEN_ZTEST:
      TEST_MISSING_MESH
      if (!priority) priority = csStrNew ("alpha");
      if (recursive)
      {
        mesh->SetRenderPriorityRecursive (Engine->GetRenderPriority (priority));
        mesh->SetZBufModeRecursive (CS_ZBUF_TEST);
      }
      else
      {
        mesh->SetRenderPriority (Engine->GetRenderPriority (priority));
        mesh->SetZBufMode (CS_ZBUF_TEST);
      }
      zmodeChanged = true;
      break;
    case XMLTOKEN_CAMERA:
      TEST_MISSING_MESH
      if (!priority) priority = csStrNew ("sky");
      if (recursive)
      {
        mesh->SetRenderPriorityRecursive (Engine->GetRenderPriority (priority));
        mesh->SetFlagsRecursive (CS_ENTITY_CAMERA, CS_ENTITY_CAMERA);
      }
      else
      {
        mesh->SetRenderPriority (Engine->GetRenderPriority (priority));
        mesh->GetFlags ().Set (CS_ENTITY_CAMERA, CS_ENTITY_CAMERA);
      }
      break;
    case XMLTOKEN_BADOCCLUDER:
      TEST_MISSING_MESH
      else
      {
	//Apply the flag CS_CULLER_HINT_BADOCCLUDER to all the meshes in 
        //the meshes' hierarchy, starting from the 'mesh' mesh object.
        csRefArray<iMeshWrapper> meshesArray;
        CollectAllChildren (mesh, meshesArray);
        size_t i, count = meshesArray.Length ();
        for (i = 0; i < count; i++)
        {
          csRef<iVisibilityObject> visobj = SCF_QUERY_INTERFACE 
            (meshesArray[i], iVisibilityObject);
          if (visobj)
            visobj->GetCullerFlags ().Set (CS_CULLER_HINT_BADOCCLUDER);
        }
      }
      break;
    case XMLTOKEN_GOODOCCLUDER:
      TEST_MISSING_MESH
      else
      {
	//Apply the flag CS_CULLER_HINT_GOODOCCLUDER to all the meshes in 
        //the meshes' hierarchy, starting from the 'mesh' mesh object.
        csRefArray<iMeshWrapper> meshesArray;
        CollectAllChildren (mesh, meshesArray);
        size_t i, count = meshesArray.Length ();
        for (i = 0; i < count; i++)
        {
          csRef<iVisibilityObject> visobj = SCF_QUERY_INTERFACE 
            (meshesArray[i], iVisibilityObject);
          if (visobj)
            visobj->GetCullerFlags ().Set (CS_CULLER_HINT_GOODOCCLUDER);
        }
      }
      break;
    case XMLTOKEN_CLOSED:
      TEST_MISSING_MESH
      else
      {
        if (recursive)//Test if recursion on children has been specified.
        {
          csRefArray<iMeshWrapper> meshesArray;
          CollectAllChildren (mesh, meshesArray);
          size_t i, count = meshesArray.Length ();
          for (i = 0; i < count; i++)
          {
            ClosedFlags (meshesArray[i]);
          }
        }//if
        else
          ClosedFlags (mesh);
      }
      break;
    case XMLTOKEN_CONVEX:
      TEST_MISSING_MESH
      else
      {
        if (recursive)//Test if recursion on children has been specified.
        {
          csRefArray<iMeshWrapper> meshesArray;
          CollectAllChildren (mesh, meshesArray);
          size_t i, count = meshesArray.Length ();
          for (i = 0; i < count; i++)
          {
            ConvexFlags (meshesArray[i]);
          }
        }//if
        else
          ConvexFlags (mesh);
      }
      break;
    case XMLTOKEN_KEY:
      TEST_MISSING_MESH
      else
      {
        iKeyValuePair* kvp = 0;
        SyntaxService->ParseKey (child, kvp);
	if (kvp)
        {
          mesh->QueryObject()->ObjAdd (kvp->QueryObject ());
	  kvp->DecRef ();
        } else
	  return false;
      }
      break;
    case XMLTOKEN_HARDMOVE:
      TEST_MISSING_MESH
      if (!mesh->GetMeshObject())
      {
	SyntaxService->ReportError (
		"crystalspace.maploader.load.meshobject",
		child, "Please specify the params of the meshobject first!");
	return false;
      }
      else if (!mesh->GetMeshObject ()->SupportsHardTransform ())
      {
        SyntaxService->ReportError (
	    "crystalspace.maploader.parse.meshobject",
            child, "This mesh object doesn't support 'hardmove'!");
	return false;
      }
      else
      {
	csReversibleTransform tr;
	csRef<iDocumentNode> matrix_node = child->GetNode ("matrix");
	if (matrix_node)
	{
	  csMatrix3 m;
	  if (!SyntaxService->ParseMatrix (matrix_node, m))
	    return false;
          tr.SetT2O (m);
	}
	csRef<iDocumentNode> vector_node = child->GetNode ("v");
	if (vector_node)
	{
	  csVector3 v;
	  if (!SyntaxService->ParseVector (vector_node, v))
	    return false;
          tr.SetOrigin (v);
	}
	mesh->HardTransform (tr);
      }
      break;
    case XMLTOKEN_MOVE:
      TEST_MISSING_MESH
      else
      {
        mesh->GetMovable ()->SetTransform (csMatrix3 ());     // Identity
        mesh->GetMovable ()->SetPosition (csVector3 (0));
	csRef<iDocumentNode> matrix_node = child->GetNode ("matrix");
	if (matrix_node)
	{
	  csMatrix3 m;
	  if (!SyntaxService->ParseMatrix (matrix_node, m))
	    return false;
	  if (!TestOrthoMatrix (m))
	  {
	    ReportWarning (
	        "crystalspace.maploader.load.mesh",
                child, "Scaling of mesh objects is not allowed in CS!");
	  }
          mesh->GetMovable ()->SetTransform (m);
	}
	csRef<iDocumentNode> vector_node = child->GetNode ("v");
	if (vector_node)
	{
	  csVector3 v;
	  if (!SyntaxService->ParseVector (vector_node, v))
	    return false;
          mesh->GetMovable ()->SetPosition (v);
	}
	mesh->GetMovable ()->UpdateMove ();
      }
      break;
    case XMLTOKEN_BOX:
      TEST_MISSING_MESH
      else
      {
	csBox3 b;
	if (!SyntaxService->ParseBox (child, b))
	  return false;
        mesh->GetMeshObject ()->GetObjectModel ()->SetObjectBoundingBox (b);
      }
      break;
    case XMLTOKEN_SHADERVAR:
      TEST_MISSING_MESH
      else
      {
	csRef<iShaderVariableContext> svc = SCF_QUERY_INTERFACE (mesh,
		iShaderVariableContext);
	CS_ASSERT (svc.IsValid());
        //create a new variable
        const char* varname = child->GetAttributeValue ("name");
	csRef<csShaderVariable> var;
	var.AttachNew (new csShaderVariable (stringSet->Request (varname)));
	if (!SyntaxService->ParseShaderVar (child, *var))
        {
	  SyntaxService->ReportError (
	    "crystalspace.maploader.load.meshobject", child,
	    "Error loading shader variable '%s' in mesh '%s'.", 
	    varname, mesh->QueryObject()->GetName());
          break;
        }
	svc->AddVariable (var);
      }
      break;
    default:
      handled = false;
      return true;
  }
  return true;
#undef TEST_MISSING_MESH
}

iMeshWrapper* csLoader::LoadMeshObjectFromFactory (iLoaderContext* ldr_context,
	iDocumentNode* node)
{
  if (!Engine) return 0;

  char* priority = '\0';

  iMeshWrapper* mesh = 0;
  bool staticpos = false;
  bool staticshape = false;
  bool zbufSet = false;
  bool prioSet = false;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    bool handled;
    if (!HandleMeshParameter (ldr_context, mesh, 0, child, id,
    	handled, priority, false, staticpos, staticshape, zbufSet,
	prioSet, true))
      goto error;
    if (!handled) switch (id)
    {
      case XMLTOKEN_FACTORY:
        if (mesh)
	{
	  SyntaxService->ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	child, "There is already a factory for this mesh!");
	  goto error;
	}
	else
	{
	  // @@@ Handle regions correctly here???
          iMeshFactoryWrapper* t = Engine->GetMeshFactories ()
	  	->FindByName (child->GetContentsValue ());
          if (!t)
	  {
	    SyntaxService->ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	child, "Can't find factory '%s'!", child->GetContentsValue ());
	    goto error;
	  }
	  mesh = t->CreateMeshWrapper ();
	  if (mesh)
	  {
	    AddToRegion (ldr_context, mesh->QueryObject ());
	  }
	}
        break;
      default:
	SyntaxService->ReportBadToken (child);
	goto error;
    }
  }

  if (!mesh)
  {
    SyntaxService->ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	node, "There is no 'factory' for this mesh!");
    goto error;
  }
  if (!priority) priority = csStrNew ("object");
  mesh->SetRenderPriorityRecursive (Engine->GetRenderPriority (priority));
  
  //I had to put these ugly curly brackets. It's due to the uglier label
  //below! 'children' and 'set' need an initialization indeed. Luca
  {
    csRefArray<iMeshWrapper> meshesArray;
    CollectAllChildren (mesh, meshesArray);
    size_t i, count = meshesArray.Length ();
    for (i = 0; i < count; i++)
    {
      iMeshWrapper* mesh = meshesArray[i];
      mesh->GetMeshObject ()->GetFlags ().SetBool (
        CS_MESH_STATICPOS, staticpos);
      mesh->GetMeshObject ()->GetFlags ().SetBool (
        CS_MESH_STATICSHAPE, staticshape);
    }
  }

  delete[] priority;
  return mesh;

error:
  delete[] priority;
  return 0;
}

bool csLoader::LoadPolyMeshInSector (iLoaderContext* ldr_context,
	iMeshWrapper* mesh, iDocumentNode* node)
{
  iObjectModel* objmodel = mesh->GetMeshObject ()->GetObjectModel ();
  csRef<iPolygonMesh> polymesh;
  bool colldet = false;
  bool viscull = false;
  bool shadows = false;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_ADDON:
        if (!LoadAddOn (ldr_context, child, mesh, false))
	  return false;
        break;
      case XMLTOKEN_META:
        if (!LoadAddOn (ldr_context, child, mesh, true))
	  return false;
        break;
      case XMLTOKEN_MOVE:
      {
        mesh->GetMovable ()->SetTransform (csMatrix3 ());     // Identity
        mesh->GetMovable ()->SetPosition (csVector3 (0));
	csRef<iDocumentNode> matrix_node = child->GetNode ("matrix");
	if (matrix_node)
	{
	  csMatrix3 m;
	  if (!SyntaxService->ParseMatrix (matrix_node, m))
	    return false;
	  if (!TestOrthoMatrix (m))
	  {
	    ReportWarning (
	        "crystalspace.maploader.load.mesh",
                child, "Scaling of mesh objects is not allowed in CS!");
	  }
          mesh->GetMovable ()->SetTransform (m);
	}
	csRef<iDocumentNode> vector_node = child->GetNode ("v");
	if (vector_node)
	{
	  csVector3 v;
	  if (!SyntaxService->ParseVector (vector_node, v))
	    return false;
          mesh->GetMovable ()->SetPosition (v);
	}
	mesh->GetMovable ()->UpdateMove ();
        break;
      }
      case XMLTOKEN_BOX:
        if (!ParsePolyMeshChildBox (child, polymesh))
	  return false;
        break;
      case XMLTOKEN_MESH:
        if (!ParsePolyMeshChildMesh (child, polymesh))
	  return false;
        break;
      case XMLTOKEN_COLLDET:
        colldet = true;
	break;
      case XMLTOKEN_VISCULL:
        viscull = true;
	break;
      case XMLTOKEN_SHADOWS:
        shadows = true;
	break;
      default:
	SyntaxService->ReportBadToken (child);
        return false;
    }
  }

  if (!colldet && !viscull && !shadows)
  {
    SyntaxService->ReportError (
	"crystalspace.maploader.parse.polymesh",
	node, "Please specify either <shadows/>, <viscull/> or <colldet/>!");
    return false;
  }
  if (!polymesh)
  {
    SyntaxService->ReportError (
	"crystalspace.maploader.parse.sector.polymesh",
	node, "Please specify either <mesh/> or <box/>!");
    return false;
  }

  if (colldet)
    objmodel->SetPolygonMeshColldet (polymesh);
  if (viscull)
    objmodel->SetPolygonMeshViscull (polymesh);
  if (shadows)
    objmodel->SetPolygonMeshShadows (polymesh);

  csRef<iNullMeshState> nullmesh = SCF_QUERY_INTERFACE (
    	mesh->GetMeshObject (), iNullMeshState);
  CS_ASSERT (nullmesh != 0);
  int i;
  csBox3 bbox;
  csVector3* vt = polymesh->GetVertices ();
  bbox.StartBoundingBox (vt[0]);
  for (i = 1 ; i < polymesh->GetVertexCount () ; i++)
  {
    bbox.AddBoundingVertexSmart (vt[i]);
  }
  nullmesh->SetBoundingBox (bbox);

  return true;
}

bool csLoader::HandleMeshObjectPluginResult (iBase* mo, iDocumentNode* child,
	iMeshWrapper* mesh, bool keepZbuf, bool keepPrio)
{
  csRef<iMeshObject> mo2 = SCF_QUERY_INTERFACE (mo, iMeshObject);
  if (!mo2)
  {
    SyntaxService->ReportError (
      "crystalspace.maploader.parse.mesh",
      child, "Returned object does not implement iMeshObject!");
    return false;
  }
  mesh->SetMeshObject (mo2);
  mo2->SetLogicalParent (mesh);
  if (mo2->GetFactory () && mo2->GetFactory ()->GetLogicalParent ())
  {
    iBase* lp = mo2->GetFactory ()->GetLogicalParent ();
    csRef<iMeshFactoryWrapper> mfw = SCF_QUERY_INTERFACE (lp,
	      	iMeshFactoryWrapper);
    if (mfw)
    {
      mesh->SetFactory (mfw);
      if (!keepZbuf) mesh->SetZBufMode (mfw->GetZBufMode ());
      if (!keepPrio) mesh->SetRenderPriority (mfw->GetRenderPriority ());
    }
  }
  return true;
}

bool csLoader::LoadMeshObject (iLoaderContext* ldr_context,
	iMeshWrapper* mesh, iMeshWrapper* parent, iDocumentNode* node)
{
  if (!Engine) return false;

  char* priority = 0;

  iLoaderPlugin* plug = 0;
  iBinaryLoaderPlugin* binplug = 0;
  bool staticpos = false;
  bool staticshape = false;
  bool zbufSet = false;
  bool prioSet = false;

  csRef<iDocumentNodeIterator> prev_it;
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (true)
  {
    if (!it->HasNext ())
    {
      // Iterator has finished. Check if we still have to continue
      // with the normal iterator first (non-defaults).
      if (!prev_it) break;
      it = prev_it;
      prev_it = 0;
      continue;
    }

    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    bool handled;
    if (!HandleMeshParameter (ldr_context, mesh, parent, child, id,
      handled, priority, false, staticpos, staticshape, zbufSet, prioSet))
      goto error;
    if (!handled) switch (id)
    {
      case XMLTOKEN_PORTAL:
        {
	  iMeshWrapper* container_mesh = 0;
          if (!ParsePortal (ldr_context, child, 0, 0, container_mesh, mesh))
	    return 0;
	}
        break;
      case XMLTOKEN_PORTALS:
        if (!ParsePortals (ldr_context, child, 0, mesh))
	  return 0;
        break;
      case XMLTOKEN_MESHREF:
        {
          iMeshWrapper* sp = LoadMeshObjectFromFactory (ldr_context, child);
          if (!sp)
	  {
	    // Error is already reported.
	    goto error;
	  }
	  sp->QueryObject ()->SetName (child->GetAttributeValue ("name"));
          mesh->GetChildren ()->Add (sp);
	  sp->DecRef ();
        }
        break;
      case XMLTOKEN_MESHOBJ:
        {
	  csRef<iMeshWrapper> sp = Engine->CreateMeshWrapper (
			  child->GetAttributeValue ("name"));
          if (!LoadMeshObject (ldr_context, sp, mesh, child))
	  {
	    // Error is already reported.
	    goto error;
	  }
	  else
	  {
	    AddToRegion (ldr_context, sp->QueryObject ());
	  }
          mesh->GetChildren ()->Add (sp);
        }
        break;
      case XMLTOKEN_LIGHT:
        {
          iLight * light = ParseStatlight (ldr_context, child);
          if (light)
          {
            light->GetMovable ()->SetParent (mesh->GetMovable ());
          }
          else
          {
            goto error;
          }
        }
      case XMLTOKEN_NULLMESH:
        {
	  if (plug)
	  {
            SyntaxService->ReportError (
	        "crystalspace.maploader.load.plugin",
                child, "Don't specify the plugin if you use <nullmesh>!");
	    goto error;
	  }
	  if (mesh->GetMeshObject ())
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.mesh", child,
	      "Please don't use <params> in combination with <nullmesh>!");
	    goto error;
	  }
	  csRef<iPluginManager> plugin_mgr =
	  	CS_QUERY_REGISTRY (object_reg, iPluginManager);
	  csRef<iMeshObjectType> type = CS_QUERY_PLUGIN_CLASS (
		plugin_mgr, "crystalspace.mesh.object.null",
		iMeshObjectType);
	  if (!type)
	    type = CS_LOAD_PLUGIN (plugin_mgr,
	    	"crystalspace.mesh.object.null", iMeshObjectType);
	  if (!type)
	  {
            SyntaxService->ReportError (
	        "crystalspace.maploader.load.plugin",
                child, "Could not find the nullmesh plugin!");
	    goto error;
	  }
	  csRef<iMeshObjectFactory> fact = type->NewFactory ();
	  csRef<iMeshObject> mo = fact->NewInstance ();
	  mesh->SetMeshObject (mo);
	  mo->SetLogicalParent (mesh);
	  csBox3 b;
	  if (!SyntaxService->ParseBox (child, b))
	    goto error;
	  csRef<iNullMeshState> nullmesh = SCF_QUERY_INTERFACE (
		mo, iNullMeshState);
	  if (nullmesh)
	    nullmesh->SetBoundingBox (b);
	}
        break;

      case XMLTOKEN_PARAMS:
	if (!plug)
	{
          SyntaxService->ReportError (
	      "crystalspace.maploader.load.plugin",
              child, "Could not load plugin!");
	  goto error;
	}
	else
	{
	  csRef<iBase> mo = plug->Parse (child, ldr_context, mesh);
          if (!mo || !HandleMeshObjectPluginResult (mo, child, mesh, zbufSet, 
	    prioSet))
	    goto error;	// Error already reported.
	}
        break;
      case XMLTOKEN_FILE:
        {
	  const char* fname = child->GetContentsValue ();
	  if (!fname)
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.loadingfile",
	      child, "Specify a VFS filename with 'file'!");
	    goto error;
	  }
          csRef<iFile> buf = VFS->Open (fname, VFS_FILE_READ);
	  if (!buf)
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.loadingfile",
	      child, "Error opening file '%s'!", fname);
	    goto error;
	  }
	  csRef<iDocument> doc;
	  bool er = LoadStructuredDoc (fname, buf, doc);
	  if (!er)
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.loadingfile",
	      child, "'%s' is not an XML file!", fname);
	    goto error;
	  }
	  csRef<iDocumentNode> paramsnode = doc->GetRoot ()->GetNode ("params");
	  if (paramsnode)
	  {
	    if (!plug && !binplug)
	    {
	      SyntaxService->ReportError (
		"crystalspace.maploader.load.plugin",
		child, "Could not load plugin for mesh '%s'!",
		mesh->QueryObject ()->GetName ());
	      goto error;
	    }
	    csRef<iBase> mo;
	    if (plug)
	      mo = plug->Parse (paramsnode, ldr_context, mesh);
	    else
	    {
	      csRef<iDataBuffer> dbuf = VFS->ReadFile (fname);
	      mo = binplug->Parse ((void*)(dbuf->GetUint8 ()),
	  	  ldr_context, mesh);
	    }
            if (!mo || !HandleMeshObjectPluginResult (mo, child, mesh,
	      zbufSet, prioSet))
	      goto error;	// Error already reported.
	    break;
	  }
	  csRef<iDocumentNode> meshobjnode = doc->GetRoot ()->GetNode (
	  	"meshobj");
	  if (meshobjnode)
	  {
	    if (!LoadMeshObject (ldr_context, mesh, parent, meshobjnode))
	      goto error;
	    break;
	  }
	  csRef<iDocumentNode> meshfactnode = doc->GetRoot ()->GetNode (
	  	"meshfact");
	  if (meshfactnode)
	  {
	    const char* meshfactname = meshfactnode->GetAttributeValue ("name");
	    // @@@ Handle regions correctly here???
            csRef<iMeshFactoryWrapper> t = Engine->GetMeshFactories ()
	  	->FindByName (meshfactname);
	    if (!t)
	    {
              t = Engine->CreateMeshFactory (meshfactname);
	      if (!t || !LoadMeshObjectFactory (ldr_context, t, 0,
	      	meshfactnode))
	      {
	        // Error is already reported.
	        goto error;
	      }
	      else
	      {
	        AddToRegion (ldr_context, t->QueryObject ());
	      }
	    }
	    break;
	  }
	  SyntaxService->ReportError (
		"crystalspace.maploader.load.plugin", child,
		"File '%s' doesn't contain <params>, <meshobj>, nor <meshfact>!",
		fname);
	  goto error;
	}
	break;
      case XMLTOKEN_PARAMSFILE:
	if (!plug && !binplug)
	{
          SyntaxService->ReportError (
	      "crystalspace.maploader.load.plugin",
              child, "Could not load plugin for mesh '%s'!",
	      mesh->QueryObject ()->GetName ());
	  goto error;
	}
	else
        {
	  const char* fname = child->GetContentsValue ();
	  if (!fname)
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.loadingfile",
	      child, "Specify a VFS filename with 'paramsfile'!");
	    goto error;
	  }
          csRef<iFile> buf (VFS->Open (fname, VFS_FILE_READ));
	  if (!buf)
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.loadingfile",
	      child, "Error opening file '%s'!", fname);
	    goto error;
	  }
	  csRef<iBase> mo;
	  if (plug)
	    mo = LoadStructuredMap (ldr_context, plug, buf, mesh, fname);
	  else
	  {
	    csRef<iDataBuffer> dbuf = VFS->ReadFile (fname);
	    mo = binplug->Parse ((void*)(dbuf->GetUint8 ()),
	  	ldr_context, mesh);
	  }
          if (!mo || !HandleMeshObjectPluginResult (mo, child, mesh,
	      zbufSet, prioSet))
	    goto error;	// Error already reported.
	}
        break;

      case XMLTOKEN_POLYMESH:
        {
	  if (!mesh->GetMeshObject ())
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.mesh",
              child, "Please use 'params' before specifying 'polymesh'!");
	    goto error;
	  }
	  iObjectModel* objmodel = mesh->GetMeshObject ()->GetObjectModel ();
	  if (!objmodel)
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.mesh", child,
	      "This mesh doesn't support setting of other 'polymesh'!");
	    goto error;
	  }
	  if (!ParsePolyMesh (child, objmodel))
	  {
	    // Error already reported.
	    goto error;
	  }
	}
	break;

      case XMLTOKEN_PLUGIN:
	{
	  if (prev_it || plug || binplug)
	  {
	    SyntaxService->ReportError (
 	        "crystalspace.maploader.parse.plugin",
	        child, "Please specify only one plugin!");
	    return false;
	  }

	  const char* plugname = child->GetContentsValue ();
	  if (!plugname)
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.plugin",
	      child, "Specify a plugin name with 'plugin'!");
	    goto error;
	  }
	  iDocumentNode* defaults = 0;
	  if (!loaded_plugins.FindPlugin (plugname, plug, binplug, defaults))
	  {
	    SyntaxService->ReportError (
 	        "crystalspace.maploader.parse.meshobj",
	        child, "Error loading plugin '%s'!", plugname);
	    goto error;
	  }
	  if (defaults)
	  {
	    // Set aside current iterator and start a new one.
	    prev_it = it;
	    it = defaults->GetNodes ();
	  }
	}
        break;
      default:
	SyntaxService->ReportBadToken (child);
	goto error;
    }
  }

  if (priority)
    mesh->SetRenderPriority (Engine->GetRenderPriority (priority));
  mesh->GetMeshObject ()->GetFlags ().SetBool (CS_MESH_STATICPOS, staticpos);
  mesh->GetMeshObject ()->GetFlags ().SetBool (CS_MESH_STATICSHAPE, staticshape);

  delete[] priority;
  return true;

error:
  delete[] priority;
  return false;
}

bool csLoader::ParseImposterSettings (iMeshWrapper* mesh, iDocumentNode *node)
{
  csRef<iImposter> imposter = SCF_QUERY_INTERFACE (mesh, iImposter);
  if (!imposter)
  {
    SyntaxService->ReportError (
	    "crystalspace.maploader.parse.meshobject",
	    node, "This mesh doesn't implement impostering!");
    return false;
  }
  const char *s = node->GetAttributeValue ("active");
  if (s && !strcmp (s, "no"))
    imposter->SetImposterActive (false);
  else
    imposter->SetImposterActive (true);

  s = node->GetAttributeValue ("range");
  iSharedVariable *var = Engine->GetVariableList()->FindByName (s);
  if (!var)
  {
    SyntaxService->ReportError (
	    "crystalspace.maploader.parse.meshobject",
	    node, "Imposter range variable (%s) doesn't exist!", s);
    return false;
  }
  imposter->SetMinDistance (var);

  s = node->GetAttributeValue ("tolerance");
  iSharedVariable* var2 = Engine->GetVariableList ()->FindByName (s);
  if (!var2)
  {
    SyntaxService->ReportError (
	    "crystalspace.maploader.parse.meshobject", node,
	    "Imposter rotation tolerance variable (%s) doesn't exist!",
	    s);
    return false;
  }
  imposter->SetRotationTolerance (var2);
  char const* const name = mesh->QueryObject()->GetName ();
  ReportWarning ("crystalspace.maploader.parse.meshobject", node, 
    "Set mesh %s to imposter active=%s, range=%f, tolerance=%f", 
    name ? name : "<noname>", imposter->GetImposterActive () ? "yes" : "no", 
    var->Get (), var2->Get ());
  return true;
}

bool csLoader::LoadAddOn (iLoaderContext* ldr_context,
	iDocumentNode* node, iBase* context, bool is_meta)
{
  iLoaderPlugin* plug = 0;
  iBinaryLoaderPlugin* binplug = 0;

  const char* plugin_name = node->GetAttributeValue ("plugin");
  if (plugin_name != 0)
  {
    // Short-hand notation: <addon plugin="bla"> ... </addon>
    iDocumentNode* defaults = 0;
    if (!loaded_plugins.FindPlugin (plugin_name, plug, binplug, defaults))
    {
      if (!is_meta)
        ReportWarning (
 	        "crystalspace.maploader.parse.addon",
	        node, "Couldn't find or load addon plugin '%s'!",
		plugin_name);
      return true;
    }
    if (!plug)
    {
      if (!is_meta)
        ReportWarning (
	        "crystalspace.maploader.load.plugin",
                node, "Could not find or load addon plugin!");
      return true;
    }
    if (defaults != 0)
    {
      ReportWarning (
	        "crystalspace.maploader.load.plugin",
                node, "'defaults' section is ignored for addons!");
    }
    csRef<iBase> rc = plug->Parse (node, ldr_context, context);
    if (!rc) return false;
    return true;
  }
  else
  {
    // Long notation: <addon> <plugin>bla</plugin> <params>...</params> </addon>
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
        case XMLTOKEN_PARAMS:
	  if (!plug)
	  {
	    if (!is_meta)
              ReportWarning (
	        "crystalspace.maploader.load.plugin",
                child, "Could not find or load plugin!");
	    return true;
	  }
	  else
	  {
	    csRef<iBase> rc = plug->Parse (child, ldr_context, context);
	    if (!rc) return false;
	  }
          break;

        case XMLTOKEN_PARAMSFILE:
	  if (!plug && !binplug)
	  {
	    if (!is_meta)
              ReportWarning (
	        "crystalspace.maploader.load.plugin",
                child, "Could not find or load plugin!");
	    return true;
	  }
	  else
	  {
	    const char* fname = child->GetContentsValue ();
	    if (!fname)
	    {
              SyntaxService->ReportError (
	        "crystalspace.maploader.parse.loadingfile",
	        child, "Specify a VFS filename with 'paramsfile'!");
	      return false;
	    }
            csRef<iFile> buf (VFS->Open (fname, VFS_FILE_READ));
	    if (!buf)
	    {
              SyntaxService->ReportError (
	        "crystalspace.maploader.parse.loadingfile",
	        child, "Error opening file '%s'!", fname);
	      return false;
	    }
	    bool rc;
	    if (plug)
	    {
	      csRef<iBase> ret (LoadStructuredMap (ldr_context,
	    	  plug, buf, 0, fname));
	      rc = (ret != 0);
	    }
	    else
	    {
	      csRef<iDataBuffer> dbuf = VFS->ReadFile (fname);
	      csRef<iBase> ret = binplug->Parse ((void*)(dbuf->GetUint8 ()),
	  	  ldr_context, 0);
	      rc = (ret != 0);
	    }
	    if (!rc)
	      return false;
	  }
          break;

        case XMLTOKEN_PLUGIN:
	  {
	    iDocumentNode* defaults = 0;
	    if (!loaded_plugins.FindPlugin (child->GetContentsValue (),
		  plug, binplug, defaults))
	    {
	      if (!is_meta)
	        ReportWarning (
 	          "crystalspace.maploader.parse.addon",
	          child, "Could not find or load plugin '%s'!",
		  child->GetContentsValue ());
	      return true;
	    }
	    if (defaults != 0)
	    {
	      ReportWarning (
	        "crystalspace.maploader.parse.addon",
                child, "'defaults' section is ignored for addons!");
	    }
	  }
          break;
        default:
	  SyntaxService->ReportBadToken (child);
	  return false;
      }
    }
  }
  return true;
}

bool csLoader::LoadSettings (iDocumentNode* node)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_FASTMESH:
	ReportWarning (
	    "crystalspace.maploader.parse.xml",
	    "<fastmesh> is no longer needed.");
	break;
      case XMLTOKEN_CLEARZBUF:
        {
	  bool yesno;
	  if (!SyntaxService->ParseBool (child, yesno, true))
	    return false;
	  Engine->SetClearZBuf (yesno);
        }
        break;
      case XMLTOKEN_CLEARSCREEN:
        {
	  bool yesno;
	  if (!SyntaxService->ParseBool (child, yesno, true))
	    return false;
	  Engine->SetClearScreen (yesno);
        }
        break;
      case XMLTOKEN_LIGHTMAPCELLSIZE:
        {
	  int cellsize = child->GetContentsValueAsInt ();
	  if (cellsize > 0)
	  {
	    if (!csIsPowerOf2 (cellsize) )
	    {
	      int newcellsize = csFindNearestPowerOf2(cellsize);
	      if (do_verbose)
	        ReportNotify ("lightmap cell size %d "
	          "is not a power of two, using %d", 
	          cellsize, newcellsize);
	      cellsize = newcellsize;
	    }
	    csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
		iPluginManager));
	    csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
		"crystalspace.mesh.object.thing", iMeshObjectType));
	    if (!type)
	    {
	      type = CS_LOAD_PLUGIN (plugin_mgr,
	      	"crystalspace.mesh.object.thing", iMeshObjectType);
	    }
	    csRef<iThingEnvironment> te = SCF_QUERY_INTERFACE (type,
		iThingEnvironment);
	    te->SetLightmapCellSize (cellsize);
	  }
	  else
	  {
	    SyntaxService->ReportError (
 	          "crystalspace.maploader.parse.settings",
	          child, "Bogus lightmap cell size %d", cellsize);
	    return false;
	  }
        }
	break;
      case XMLTOKEN_MAXLIGHTMAPSIZE:
        {
	  int max[2];
	  max[0] = child->GetAttributeValueAsInt ("horizontal");
	  max[1] = child->GetAttributeValueAsInt ("vertical");
	  if ( (max[0] > 0) && (max[1] > 0) )
	  {
	    Engine->SetMaxLightmapSize (max[0], max[1]);
	  }
	  else
	  {
	    SyntaxService->ReportError (
 	          "crystalspace.maploader.parse.settings",
	          child, "Bogus maximum lightmap size %dx%d", max[0], max[1]);
	    return false;
	  }
        }
	break;
      case XMLTOKEN_AMBIENT:
        {
	  csColor c;
	  if (!SyntaxService->ParseColor (child, c))
	    return false;
	  Engine->SetAmbientLight (c);
        }
	break;
      case XMLTOKEN_RENDERLOOP:
	{
	  const char* loopName = child->GetContentsValue ();
	  if (loopName)
	  {
	    iRenderLoop* loop = 
	      Engine->GetRenderLoopManager()->Retrieve (loopName);
	    if (loop)
	    {
	      Engine->SetCurrentDefaultRenderloop (loop);
	    }
	    else
	    {
	      SyntaxService->Report (
		"crystalspace.maploader.parse.settings",
		CS_REPORTER_SEVERITY_WARNING,
		child,
		"Render loop '%s' not found",
		loopName);
	    }
	  }
	  else
	  {
	    SyntaxService->Report (
	      "crystalspace.maploader.parse.settings",
	      CS_REPORTER_SEVERITY_WARNING,
	      child,
	      "Expected render loop name: %s",
	      loopName);
	  }
        }
	break;
      default:
	SyntaxService->ReportBadToken (child);
        return false;
    }
  }

  return true;
}

bool csLoader::LoadRenderPriorities (iDocumentNode* node)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_PRIORITY:
      {
	const char* name = child->GetAttributeValue ("name");
	csRef<iDocumentNode> levelnode = child->GetNode ("level");
	if (!levelnode)
	{
          SyntaxService->ReportError (
	    "crystalspace.maploader.parse.priorities",
	    child, "Render priority '%s' is missing a 'level'!",
	    name);
	  return false;
	}
	long pri = levelnode->GetContentsValueAsInt ();

	csRef<iDocumentNode> cameranode = child->GetNode ("camera");
	if (cameranode)
	{
	  ReportWarning ("crystalspace.maploader",
	      "The <camera/> flag for render priorities is no longer needed");
	}

	csRenderPrioritySorting rendsort = CS_RENDPRI_SORT_NONE;
	csRef<iDocumentNode> sortnode = child->GetNode ("sort");
	if (sortnode)
	{
	  const char* sorting = sortnode->GetContentsValue ();
	  if (!strcmp (sorting, "BACK2FRONT"))
	  {
	    rendsort = CS_RENDPRI_SORT_BACK2FRONT;
	  }
	  else if (!strcmp (sorting, "FRONT2BACK"))
	  {
	    rendsort = CS_RENDPRI_SORT_FRONT2BACK;
	  }
	  else if (!strcmp (sorting, "NONE"))
	  {
	    rendsort = CS_RENDPRI_SORT_NONE;
	  }
	  else
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.priorities",
	      child, "Unknown sorting attribute '%s' for the render priority!",
	      sorting);
	    return false;
	  }
	}
	Engine->RegisterRenderPriority (name, pri, rendsort);
        break;
      }
      default:
	SyntaxService->ReportBadToken (child);
	return false;
    }
  }

  return true;
}

iCollection* csLoader::ParseCollection (iLoaderContext* ldr_context,
	iDocumentNode* node)
{
  iCollection* collection = Engine->GetCollections ()->NewCollection (
		  node->GetAttributeValue ("name"));
  AddToRegion (ldr_context, collection->QueryObject ());

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_ADDON:
	SyntaxService->ReportError (
		"crystalspace.maploader.parse.collection",
         	child, "'addon' not yet supported in collection!");
	return 0;
      case XMLTOKEN_META:
	SyntaxService->ReportError (
		"crystalspace.maploader.parse.collection",
         	child, "'addon' not yet supported in collection!");
	return 0;
      case XMLTOKEN_KEY:
	{
          iKeyValuePair* kvp = 0;
          SyntaxService->ParseKey (child, kvp);
          if (kvp)
          {
            collection->QueryObject ()->ObjAdd (kvp->QueryObject ());
	    kvp->DecRef ();
          } else
	    return 0;
	}
        break;
      case XMLTOKEN_MESHOBJ:
#if 0
	SyntaxService->ReportError (
		"crystalspace.maploader.parse.collection",
         	child, "'meshobj' not yet supported in collection!");
#endif
        break;
      case XMLTOKEN_LIGHT:
        {
	  const char* lightname = child->GetContentsValue ();
	  iLight* l = 0;
	  iSectorList* sl = Engine->GetSectors ();
	  int i;
	  for (i = 0 ; i < sl->GetCount () ; i++)
	  {
	    iSector* sect = sl->Get (i);
	    if ((!ldr_context->GetRegion ()) ||
	    	(!ldr_context->CurrentRegionOnly ()) ||
	        ldr_context->GetRegion ()->IsInRegion (sect->QueryObject ()))
	    {
	      l = sect->GetLights ()->FindByName (lightname);
	      if (l) break;
	    }
	  }
          if (!l)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.collection",
            	child, "Light '%s' not found!", lightname);
	    return 0;
	  }
	  else
	  {
	    collection->AddObject (l->QueryObject ());
	  }
        }
        break;
      case XMLTOKEN_SECTOR:
        {
	  const char* sectname = child->GetContentsValue ();
	  iSector* s = ldr_context->FindSector (sectname);
          if (!s)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.collection",
            	child, "Sector '%s' not found!", sectname);
	    return 0;
	  }
	  else
	  {
            collection->AddObject (s->QueryObject ());
	  }
        }
        break;
      case XMLTOKEN_COLLECTION:
        {
	  const char* colname = child->GetContentsValue ();
	  iCollection* th;
	  if (ldr_context->GetRegion () && ldr_context->CurrentRegionOnly ())
	    th = ldr_context->GetRegion ()->FindCollection (colname);
	  else
            th = Engine->GetCollections ()->FindByName (colname);
          if (!th)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.collection",
            	child, "Collection '%s' not found!", colname);
	    return 0;
	  }
	  else
	  {
            collection->AddObject (th->QueryObject());
	  }
        }
        break;
      default:
	SyntaxService->ReportBadToken (child);
	collection->DecRef ();
	return 0;
    }
  }

  return collection;
}

bool csLoader::ParseStart (iDocumentNode* node, iCameraPosition* campos)
{
  char* start_sector = csStrNew ("room");
  csVector3 pos (0, 0, 0);
  csVector3 up (0, 1, 0);
  csVector3 forward (0, 0, 1);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_SECTOR:
	delete[] start_sector;
	start_sector = csStrNew (child->GetContentsValue ());
	break;
      case XMLTOKEN_POSITION:
	if (!SyntaxService->ParseVector (child, pos))
	  return false;
	break;
      case XMLTOKEN_UP:
	if (!SyntaxService->ParseVector (child, up))
	  return false;
	break;
      case XMLTOKEN_FORWARD:
	if (!SyntaxService->ParseVector (child, forward))
	  return false;
	break;
      case XMLTOKEN_FARPLANE:
        {
	  csPlane3 p;
	  p.A () = child->GetAttributeValueAsFloat ("a");
	  p.B () = child->GetAttributeValueAsFloat ("b");
	  p.C () = child->GetAttributeValueAsFloat ("c");
	  p.D () = child->GetAttributeValueAsFloat ("d");
	  campos->SetFarPlane (&p);
        }
	break;
      default:
	SyntaxService->ReportBadToken (child);
	return false;
    }
  }

  campos->Set (start_sector, pos, forward, up);
  delete[] start_sector;
  return true;
}

iLight* csLoader::ParseStatlight (iLoaderContext* ldr_context,
	iDocumentNode* node)
{
  const char* lightname = node->GetAttributeValue ("name");

  csVector3 pos;

  csVector3 attenvec (0, 0, 0);
  float spotfalloffInner = 0, spotfalloffOuter = 1;
  csVector3 direction (1, 0, 0);
  csLightType type = CS_LIGHT_POINTLIGHT;

  float distbright = 1;

  float influenceRadius = 0;
  bool influenceOverride = false;

  csLightAttenuationMode attenuation = CS_ATTN_LINEAR;
  float dist = 0;

  csColor color;
  csLightDynamicType dyn;
  struct csHaloDef
  {
    int type;
    union
    {
      struct
      {
        float Intensity;
        float Cross;
      } cross;
      struct
      {
        int Seed;
        int NumSpokes;
        float Roundness;
      } nova;
      struct
      {
        iMaterialWrapper* mat_center;
        iMaterialWrapper* mat_spark1;
        iMaterialWrapper* mat_spark2;
        iMaterialWrapper* mat_spark3;
        iMaterialWrapper* mat_spark4;
        iMaterialWrapper* mat_spark5;
      } flare;
    };
  } halo;

  // This csObject will contain all key-value pairs as children
  csObject Keys;

  memset (&halo, 0, sizeof (halo));

  // New format.
  pos.x = pos.y = pos.z = 0;
  color.red = color.green = color.blue = 1;
  dyn = CS_LIGHT_DYNAMICTYPE_STATIC;

  dist = 1;


  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_RADIUS:
	{
	  dist = child->GetContentsValueAsFloat ();
	  csRef<iDocumentAttribute> attr;
	  if (attr = child->GetAttribute ("brightness"))
	  {
	    distbright = attr->GetValueAsFloat();
	  }
	}
        break;
      case XMLTOKEN_CENTER:
	if (!SyntaxService->ParseVector (child, pos))
	  return 0;
        break;
      case XMLTOKEN_COLOR:
	if (!SyntaxService->ParseColor (child, color))
	  return 0;
        break;
      case XMLTOKEN_DYNAMIC:
        {
	  bool d;
	  if (!SyntaxService->ParseBool (child, d, true))
	    return 0;
	  if (d)
	    dyn = CS_LIGHT_DYNAMICTYPE_PSEUDO;
	  else
	    dyn = CS_LIGHT_DYNAMICTYPE_STATIC;
	}
        break;
      case XMLTOKEN_KEY:
	{
          iKeyValuePair* kvp = 0;
          SyntaxService->ParseKey (child, kvp);
          if (kvp)
          {
            Keys.ObjAdd (kvp->QueryObject ());
	    kvp->DecRef ();
          } else
	    return 0;
	}
        break;
      case XMLTOKEN_HALO:
	{
	  const char* type;
	  csRef<iDocumentNode> typenode = child->GetNode ("type");
	  if (!typenode)
	  {
	    // Default halo, type 'cross' assumed.
	    type = "cross";
	  }
	  else
	  {
	    type = typenode->GetContentsValue ();
	  }

	  if (!strcasecmp (type, "cross"))
	  {
	    halo.type = 1;
            halo.cross.Intensity = 2.0f;
	    halo.cross.Cross = 0.45f;
	    csRef<iDocumentNode> intnode = child->GetNode ("intensity");
	    if (intnode)
	    {
	      halo.cross.Intensity = intnode->GetContentsValueAsFloat ();
	    }
	    csRef<iDocumentNode> crossnode = child->GetNode ("cross");
	    if (crossnode)
	    {
	      halo.cross.Cross = crossnode->GetContentsValueAsFloat ();
	    }
	  }
	  else if (!strcasecmp (type, "nova"))
	  {
            halo.type = 2;
            halo.nova.Seed = 0;
	    halo.nova.NumSpokes = 100;
	    halo.nova.Roundness = 0.5;
	    csRef<iDocumentNode> seednode = child->GetNode ("seed");
	    if (seednode)
	    {
	      halo.nova.Seed = seednode->GetContentsValueAsInt ();
	    }
	    csRef<iDocumentNode> spokesnode = child->GetNode ("numspokes");
	    if (spokesnode)
	    {
	      halo.nova.NumSpokes = spokesnode->GetContentsValueAsInt ();
	    }
	    csRef<iDocumentNode> roundnode = child->GetNode ("roundness");
	    if (roundnode)
	    {
	      halo.nova.Roundness = roundnode->GetContentsValueAsFloat ();
	    }
	  }
	  else if (!strcasecmp (type, "flare"))
	  {
            halo.type = 3;
	    iLoaderContext* lc = ldr_context;
	    csRef<iDocumentNode> matnode;
	    
	    matnode = child->GetNode ("centermaterial");
	    halo.flare.mat_center = matnode ? lc->FindMaterial (
	    	matnode->GetContentsValue ()) : 0;
	    if (!halo.flare.mat_center)
	    {
	      SyntaxService->ReportError (
		  "crystalspace.maploader.parse.light",
    	          child, "Can't find material for flare!");
	      return 0;
	    }
	    matnode = child->GetNode ("spark1material");
	    halo.flare.mat_spark1 = matnode ? lc->FindMaterial (
	    	matnode->GetContentsValue ()) : 0;
	    if (!halo.flare.mat_spark1)
	    {
	      SyntaxService->ReportError (
		  "crystalspace.maploader.parse.light",
    	          child, "Can't find material for flare!");
	      return 0;
	    }
	    matnode = child->GetNode ("spark2material");
	    halo.flare.mat_spark2 = matnode ? lc->FindMaterial (
	    	matnode->GetContentsValue ()) : 0;
	    if (!halo.flare.mat_spark2)
	    {
	      SyntaxService->ReportError (
		  "crystalspace.maploader.parse.light",
    	          child, "Can't find material for flare!");
	      return 0;
	    }
	    matnode = child->GetNode ("spark3material");
	    halo.flare.mat_spark3 = matnode ? lc->FindMaterial (
	    	matnode->GetContentsValue ()) : 0;
	    if (!halo.flare.mat_spark3)
	    {
	      SyntaxService->ReportError (
		  "crystalspace.maploader.parse.light",
    	          child, "Can't find material for flare!");
	      return 0;
	    }
	    matnode = child->GetNode ("spark4material");
	    halo.flare.mat_spark4 = matnode ? lc->FindMaterial (
	    	matnode->GetContentsValue ()) : 0;
	    if (!halo.flare.mat_spark4)
	    {
	      SyntaxService->ReportError (
		  "crystalspace.maploader.parse.light",
    	          child, "Can't find material for flare!");
	      return 0;
	    }
	    matnode = child->GetNode ("spark5material");
	    halo.flare.mat_spark5 = matnode ? lc->FindMaterial (
	    	matnode->GetContentsValue ()) : 0;
	    if (!halo.flare.mat_spark5)
	    {
	      SyntaxService->ReportError (
		  "crystalspace.maploader.parse.light",
    	          child, "Can't find material for flare!");
	      return 0;
	    }
	  }
	  else
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.light",
    	        child,
		"Unknown halo type '%s'. Use 'cross', 'nova' or 'flare'!",
		type);
	    return 0;
	  }
        }
        break;
      case XMLTOKEN_ATTENUATION:
	{
	  const char* att = child->GetContentsValue();
	  if (att)
	  {
	    if (!strcasecmp (att, "none"))
	      attenuation = CS_ATTN_NONE;
	    else if (!strcasecmp (att, "linear"))
	      attenuation = CS_ATTN_LINEAR;
	    else if (!strcasecmp (att, "inverse"))
	      attenuation = CS_ATTN_INVERSE;
	    else if (!strcasecmp (att, "realistic"))
	      attenuation = CS_ATTN_REALISTIC;
	    else if (!strcasecmp (att, "clq"))
	      attenuation = CS_ATTN_CLQ;
	    else
	    {
	      SyntaxService->ReportBadToken (child);
	      return 0;
	    }
	  }
	  else
	  {
	    attenuation = CS_ATTN_CLQ;
	  }

	  attenvec.x = child->GetAttributeValueAsFloat ("c");
	  attenvec.y = child->GetAttributeValueAsFloat ("l");
	  attenvec.z = child->GetAttributeValueAsFloat ("q");
	}
	break;
      case XMLTOKEN_INFLUENCERADIUS:
	{
	  influenceRadius = child->GetContentsValueAsFloat();
	  influenceOverride = true;
	}
	break;
      case XMLTOKEN_ATTENUATIONVECTOR:
        {
	  //@@@ should be scrapped in favor of specification via
	  // "attenuation".
          if (!SyntaxService->ParseVector (child, attenvec))
	    return 0;
	  attenuation = CS_ATTN_CLQ;
        }
        break;
      case XMLTOKEN_TYPE:
        {
          const char* t = child->GetContentsValue ();
          if (t)
          {
            if (!strcasecmp (t, "point"))
              type = CS_LIGHT_POINTLIGHT;
            else if (!strcasecmp (t, "directional"))
              type = CS_LIGHT_DIRECTIONAL;
            else if (!strcasecmp (t, "spotlight"))
              type = CS_LIGHT_SPOTLIGHT;
            else
            {
              SyntaxService->ReportBadToken (child);
              return 0;
            }
          }
        }
        break;
      case XMLTOKEN_DIRECTION:
        {
          if (!SyntaxService->ParseVector (child, direction))
            return 0;
        }
        break;
      case XMLTOKEN_SPOTLIGHTFALLOFF:
        {
          spotfalloffInner = child->GetAttributeValueAsFloat ("inner");
          spotfalloffInner *= (PI/180);
          spotfalloffInner = cosf(spotfalloffInner);
          spotfalloffOuter = child->GetAttributeValueAsFloat ("outer");
          spotfalloffOuter *= (PI/180);
          spotfalloffOuter = cosf(spotfalloffOuter);
        }
        break;
      default:
	SyntaxService->ReportBadToken (child);
	return 0;
    }
  }

  // implicit radius
  if (dist == 0)
  {
    if (color.red > color.green && color.red > color.blue) dist = color.red;
    else if (color.green > color.blue) dist = color.green;
    else dist = color.blue;
  }

  csRef<iLight> l = Engine->CreateLight (lightname, pos,
  	dist, color, dyn);
  AddToRegion (ldr_context, l->QueryObject ());
  l->SetType (type);
  l->SetDirection (direction);
  l->SetSpotLightFalloff (spotfalloffInner, spotfalloffOuter);

  switch (halo.type)
  {
    case 1:
      l->CreateCrossHalo (halo.cross.Intensity,
      	halo.cross.Cross);
      break;
    case 2:
      l->CreateNovaHalo (halo.nova.Seed, halo.nova.NumSpokes,
      	halo.nova.Roundness);
      break;
    case 3:
      {
	iMaterialWrapper* ifmc = halo.flare.mat_center;
	iMaterialWrapper* ifm1 = halo.flare.mat_spark1;
	iMaterialWrapper* ifm2 = halo.flare.mat_spark2;
	iMaterialWrapper* ifm3 = halo.flare.mat_spark3;
	iMaterialWrapper* ifm4 = halo.flare.mat_spark4;
	iMaterialWrapper* ifm5 = halo.flare.mat_spark5;
        iFlareHalo* flare = l->CreateFlareHalo ();
	flare->AddComponent (0.0f, 1.2f, 1.2f, CS_FX_ADD, ifmc);
	flare->AddComponent (0.3f, 0.1f, 0.1f, CS_FX_ADD, ifm3);
	flare->AddComponent (0.6f, 0.4f, 0.4f, CS_FX_ADD, ifm4);
	flare->AddComponent (0.8f, 0.05f, 0.05f, CS_FX_ADD, ifm5);
	flare->AddComponent (1.0f, 0.7f, 0.7f, CS_FX_ADD, ifm1);
	flare->AddComponent (1.3f, 0.1f, 0.1f, CS_FX_ADD, ifm3);
	flare->AddComponent (1.5f, 0.3f, 0.3f, CS_FX_ADD, ifm4);
	flare->AddComponent (1.8f, 0.1f, 0.1f, CS_FX_ADD, ifm5);
	flare->AddComponent (2.0f, 0.5f, 0.5f, CS_FX_ADD, ifm2);
	flare->AddComponent (2.1f, 0.15f, 0.15f, CS_FX_ADD, ifm3);
	flare->AddComponent (2.5f, 0.2f, 0.2f, CS_FX_ADD, ifm3);
	flare->AddComponent (2.8f, 0.4f, 0.4f, CS_FX_ADD, ifm4);
	flare->AddComponent (3.0f, 3.0f, 3.0f, CS_FX_ADD, ifm1);
	flare->AddComponent (3.1f, 0.05f, 0.05f, CS_FX_ADD, ifm5);
	flare->AddComponent (3.3f, 0.15f, 0.15f, CS_FX_ADD, ifm2);
      }
      break;
  }
  l->SetAttenuationMode (attenuation);
  if (attenuation == CS_ATTN_CLQ)
  {
    if (attenvec.IsZero())
    {
      //@@TODO:
    }
    else
    {
      l->SetAttenuationConstants (attenvec);
    }
  }

  if (influenceOverride) l->SetCutoffDistance (influenceRadius);
  else l->SetCutoffDistance (dist);

  // Move the key-value pairs from 'Keys' to the light object
  l->QueryObject ()->ObjAddChildren (&Keys);
  Keys.ObjRemoveAll ();

  l->IncRef ();	// To make sure smart pointer doesn't release.
  return l;
}

iMapNode* csLoader::ParseNode (iDocumentNode* node, iSector* sec)
{
  iMapNode* pNode = (iMapNode*)(new csMapNode (
  	node->GetAttributeValue ("name")));
  pNode->SetSector (sec);

  csVector3 pos, v;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_ADDON:
	SyntaxService->ReportError (
		"crystalspace.maploader.parse.node",
        	child, "'addon' not yet supported in node!");
	return 0;
      case XMLTOKEN_META:
	SyntaxService->ReportError (
		"crystalspace.maploader.parse.node",
        	child, "'meta' not yet supported in node!");
	return 0;
      case XMLTOKEN_KEY:
        {
          iKeyValuePair* kvp = 0;
          SyntaxService->ParseKey (child, kvp);
          if (kvp)
          {
            pNode->QueryObject ()->ObjAdd (kvp->QueryObject ());
	    kvp->DecRef ();
          } else
	    return 0;
	}
        break;
      case XMLTOKEN_POSITION:
	if (!SyntaxService->ParseVector (child, pos))
	  return 0;
        break;
      case XMLTOKEN_XVECTOR:
	if (!SyntaxService->ParseVector (child, v))
	  return 0;
	pNode->SetXVector (v);
        break;
      case XMLTOKEN_YVECTOR:
	if (!SyntaxService->ParseVector (child, v))
	  return 0;
	pNode->SetYVector (v);
        break;
      case XMLTOKEN_ZVECTOR:
	if (!SyntaxService->ParseVector (child, v))
	  return 0;
	pNode->SetZVector (v);
        break;
      default:
	SyntaxService->ReportBadToken (child);
	return 0;
    }
  }

  pNode->SetPosition (pos);

  return pNode;
}

class csMissingSectorCallback : public iPortalCallback
{
public:
  csRef<iLoaderContext> ldr_context;
  char* sectorname;
  bool autoresolve;

  SCF_DECLARE_IBASE;
  csMissingSectorCallback (iLoaderContext* ldr_context, const char* sector,
  	bool autoresolve)
  {
    SCF_CONSTRUCT_IBASE (0);
    csMissingSectorCallback::ldr_context = ldr_context;
    sectorname = csStrNew (sector);
    csMissingSectorCallback::autoresolve = autoresolve;
  }
  virtual ~csMissingSectorCallback ()
  {
    delete[] sectorname;
    SCF_DESTRUCT_IBASE();
  }
  
  virtual bool Traverse (iPortal* portal, iBase* /*context*/)
  {
    iSector* sector = ldr_context->FindSector (sectorname);
    if (!sector) return false;
    portal->SetSector (sector);
    // For efficiency reasons we deallocate the name here.
    if (!autoresolve)
    {
      delete[] sectorname;
      sectorname = 0;
      portal->RemoveMissingSectorCallback (this);
    }
    return true;
  }
};

SCF_IMPLEMENT_IBASE (csMissingSectorCallback)
  SCF_IMPLEMENTS_INTERFACE (iPortalCallback)
SCF_IMPLEMENT_IBASE_END

bool csLoader::ParsePortal (iLoaderContext* ldr_context,
	iDocumentNode* node, iSector* sourceSector, const char* container_name,
	iMeshWrapper*& container_mesh, iMeshWrapper* parent)
{
  const char* name = node->GetAttributeValue ("name");
  iSector* destSector = 0;
  csPoly3D poly;

  csMatrix3 m_w; m_w.Identity ();
  csVector3 v_w_before (0, 0, 0);
  csVector3 v_w_after (0, 0, 0);
  uint32 flags = 0;
  bool do_warp = false;
  bool do_mirror = false;
  int msv = -1;
  scfString destSectorName;
  bool autoresolve = false;

  // Array of keys we need to parse later.
  csRefArray<iDocumentNode> key_nodes;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    bool handled;
    if (!SyntaxService->HandlePortalParameter (child, ldr_context,
        flags, do_mirror, do_warp, msv, m_w, v_w_before, v_w_after,
	&destSectorName, handled, autoresolve))
    {
      return false;
    }
    if (!handled)
    {
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
	case XMLTOKEN_KEY:
	  key_nodes.Push (child);
	  break;
        case XMLTOKEN_V:
          {
            csVector3 vec;
            if (!SyntaxService->ParseVector (child, vec))
	      return false;
	    poly.AddVertex (vec);
	  }
          break;
        default:
	  SyntaxService->ReportBadToken (child);
	  return false;
      }
    }
  }

  iPortal* portal;
  // If autoresolve is true we clear the sector since we want the callback
  // to be used.
  if (autoresolve)
    destSector = 0;
  else
    destSector = ldr_context->FindSector (destSectorName.GetData ());
  csRef<iMeshWrapper> mesh;
  if (container_mesh)
  {
    mesh = container_mesh;
    csRef<iPortalContainer> pc = SCF_QUERY_INTERFACE (mesh->GetMeshObject (),
    	iPortalContainer);
    CS_ASSERT (pc != 0);
    portal = pc->CreatePortal (poly.GetVertices (), (int)poly.GetVertexCount ());
    portal->SetSector (destSector);
  }
  else if (parent)
  {
    CS_ASSERT (sourceSector == 0);
    mesh = Engine->CreatePortal (
  	  container_name ? container_name : name,
  	  parent, destSector,
  	  poly.GetVertices (), (int)poly.GetVertexCount (), portal);
    AddToRegion (ldr_context, mesh->QueryObject ());
  }
  else
  {
    CS_ASSERT (sourceSector != 0);
    mesh = Engine->CreatePortal (
  	  container_name ? container_name : name,
  	  sourceSector, csVector3 (0), destSector,
  	  poly.GetVertices (), (int)poly.GetVertexCount (), portal);
    AddToRegion (ldr_context, mesh->QueryObject ());
  }
  container_mesh = mesh;
  if (name)
    portal->SetName (name);
  if (!destSector)
  {
    // Create a callback to find the sector at runtime when the
    // portal is first used.
    csMissingSectorCallback* missing_cb = new csMissingSectorCallback (
	    	ldr_context, destSectorName.GetData (), autoresolve);
    portal->SetMissingSectorCallback (missing_cb);
    missing_cb->DecRef ();
  }

  portal->GetFlags ().Set (flags);
  if (do_mirror)
  {
    csPlane3 p = poly.ComputePlane ();
    portal->SetWarp (csTransform::GetReflect (p));
  }
  else if (do_warp)
  {
    portal->SetWarp (m_w, v_w_before, v_w_after);
  }
  if (msv != -1)
  {
    portal->SetMaximumSectorVisit (msv);
  }

  size_t i;
  for (i = 0 ; i < key_nodes.Length () ; i++)
  {
    iKeyValuePair* kvp = 0;
    SyntaxService->ParseKey (key_nodes[i], kvp);
    if (kvp)
    {
      container_mesh->QueryObject()->ObjAdd (kvp->QueryObject ());
      kvp->DecRef ();
    } else
      return false;
  }

  return true;
}

bool csLoader::ParsePortals (iLoaderContext* ldr_context,
	iDocumentNode* node, iSector* sourceSector,
	iMeshWrapper* parent)
{
  const char* container_name = node->GetAttributeValue ("name");
  iMeshWrapper* container_mesh = 0;
  char* priority = 0;
  bool staticpos = false;
  bool staticshape = false;
  bool zbufSet = false;
  bool prioSet = false;
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    bool handled;
    if (!HandleMeshParameter (ldr_context, container_mesh, parent, child, id,
    	handled, priority, true, staticpos, staticshape, zbufSet, prioSet))
      goto error;
    if (!handled) switch (id)
    {
      case XMLTOKEN_PORTAL:
        if (!ParsePortal (ldr_context, child, sourceSector,
		container_name, container_mesh, parent))
	  goto error;
        break;
      default:
	SyntaxService->ReportBadToken (child);
	goto error;
    }
  }

  if (priority)
    container_mesh->SetRenderPriority (Engine->GetRenderPriority (priority));
  container_mesh->GetMeshObject ()->GetFlags ().SetBool (CS_MESH_STATICPOS,
  	staticpos);
  container_mesh->GetMeshObject ()->GetFlags ().SetBool (CS_MESH_STATICSHAPE,
  	staticshape);

  delete[] priority;
  return true;

error:
  delete[] priority;
  return false;
}

iSector* csLoader::ParseSector (iLoaderContext* ldr_context,
	iDocumentNode* node)
{
  const char* secname = node->GetAttributeValue ("name");

  bool do_culler = false;
  char* culplugname = 0;

  iSector* sector = ldr_context->FindSector (secname);
  if (sector == 0)
  {
    sector = Engine->CreateSector (secname);
    AddToRegion (ldr_context, sector->QueryObject ());
  }

  csRef<iDocumentNode> culler_params;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_RENDERLOOP:
	{
	  const char* loopName = child->GetContentsValue ();
	  if (loopName)
	  {
	    iRenderLoop* loop = 
	      Engine->GetRenderLoopManager()->Retrieve (loopName);
	    if (loop)
	    {
	      sector->SetRenderLoop (loop);
	    }
	    else
	    {
	      SyntaxService->Report (
		"crystalspace.maploader.parse.sector",
		CS_REPORTER_SEVERITY_WARNING,
		child,
		"Render loop '%s' in sector '%s' not found",
		loopName, secname);
	    }
	  }
	  else
	  {
	    SyntaxService->Report (
	      "crystalspace.maploader.parse.sector",
	      CS_REPORTER_SEVERITY_WARNING,
	      child,
	      "Expected render loop name '%s' in sector '%s'",
	      loopName, secname);
	  }
	}
	break;
      case XMLTOKEN_ADDON:
	if (!LoadAddOn (ldr_context, child, sector, false))
	  goto error;
      	break;
      case XMLTOKEN_META:
	if (!LoadAddOn (ldr_context, child, sector, true))
	  goto error;
      	break;
      case XMLTOKEN_PORTAL:
	{
	  iMeshWrapper* container_mesh = 0;
          if (!ParsePortal (ldr_context, child, sector, 0, container_mesh, 0))
	    goto error;
	}
        break;
      case XMLTOKEN_PORTALS:
        if (!ParsePortals (ldr_context, child, sector, 0))
	  goto error;
        break;
      case XMLTOKEN_CULLER:
	SyntaxService->ReportError (
	  "crystalspace.maploader.parse.sector",
	  child, "<culler> no longer supported! Convert your level to Dynavis using 'levtool'!");
	csPrintf ("<culler> no longer supported! Convert your level to Dynavis using 'levtool'!");
	goto error;
      case XMLTOKEN_CULLERP:
        {
	  const char* pluginname = child->GetAttributeValue ("plugin");
	  if (pluginname)
	  {
	    // New way to write cullerp.
	    culplugname = csStrNew (pluginname);
	    culler_params = child;	// Remember for later.
	  }
	  else
	  {
	    // Old way.
	    culplugname = csStrNew (child->GetContentsValue ());
	    culler_params = 0;
	  }
	  if (!culplugname)
	  {
	    SyntaxService->ReportError (
		  "crystalspace.maploader.parse.sector",
	  	  child,
		  "CULLERP expects the name of a visibility culling plugin!");
	    goto error;
	  }
	  else
	  {
            do_culler = true;
	  }
	}
        break;
      case XMLTOKEN_MESHREF:
        {
	  const char* meshname = child->GetAttributeValue ("name");
	  if (!meshname)
	  {
      	    SyntaxService->ReportError (
	      	"crystalspace.maploader.load.meshobject",
		child, "'meshref' requires a name in sector '%s'!",
		secname ? secname : "<noname>");
	    goto error;
	  }
          iMeshWrapper* mesh = LoadMeshObjectFromFactory (ldr_context, child);
          if (!mesh)
	  {
	    // Error is already reported.
	    goto error;
	  }
	  mesh->QueryObject ()->SetName (meshname);
          mesh->GetMovable ()->SetSector (sector);
	  mesh->GetMovable ()->UpdateMove ();
	  Engine->AddMeshAndChildren (mesh);
	  mesh->DecRef ();
        }
        break;
      case XMLTOKEN_POLYMESH:
        {
	  const char* meshname = child->GetAttributeValue ("name");
	  if (!meshname)
	  {
      	    SyntaxService->ReportError (
	      	"crystalspace.maploader.load.polymesh",
		child, "'polymesh' requires a name in sector '%s'!",
		secname ? secname : "<noname>");
	    goto error;
	  }
	  csRef<iMeshWrapper> mesh = Engine->CreateMeshWrapper (
	  	"crystalspace.mesh.object.null", meshname);
          if (!LoadPolyMeshInSector (ldr_context, mesh, child))
	  {
	    // Error is already reported.
	    goto error;
	  }
	  else
	  {
	    AddToRegion (ldr_context, mesh->QueryObject ());
	  }
          mesh->GetMovable ()->SetSector (sector);
	  mesh->GetMovable ()->UpdateMove ();
        }
        break;
      case XMLTOKEN_MESHOBJ:
        {
	  const char* meshname = child->GetAttributeValue ("name");
	  if (!meshname)
	  {
      	    SyntaxService->ReportError (
	      	"crystalspace.maploader.load.meshobject",
		child, "'meshobj' requires a name in sector '%s'!",
		secname ? secname : "<noname>");
	    goto error;
	  }
	  csRef<iMeshWrapper> mesh = Engine->CreateMeshWrapper (meshname);
          if (!LoadMeshObject (ldr_context, mesh, 0, child))
	  {
	    // Error is already reported.
	    goto error;
	  }
	  else
	  {
	    AddToRegion (ldr_context, mesh->QueryObject ());
	  }
          mesh->GetMovable ()->SetSector (sector);
	  mesh->GetMovable ()->UpdateMove ();
        }
        break;
      case XMLTOKEN_MESHLIB:
        {
	  const char* meshname = child->GetAttributeValue ("name");
	  if (!meshname)
	  {
      	    SyntaxService->ReportError (
	      	"crystalspace.maploader.load.meshobject",
		child, "'meshlib' requires a name (sector '%s')!",
		secname ? secname : "<noname>");
	    goto error;
	  }
	  iMeshWrapper* mesh = Engine->GetMeshes ()->FindByName (meshname);
	  if (!mesh)
	  {
      	    SyntaxService->ReportError (
	      	"crystalspace.maploader.load.meshobject",
		child,
		"Could not find mesh object '%s' (sector '%s') for MESHLIB!",
		meshname, secname ? secname : "<noname>");
	    goto error;
	  }
          if (!LoadMeshObject (ldr_context, mesh, 0, child))
	  {
	    // Error is already reported.
	    goto error;
	  }
          mesh->GetMovable ()->GetSectors ()->Add (sector);
	  mesh->GetMovable ()->UpdateMove ();
        }
        break;
      case XMLTOKEN_LIGHT:
        {
	  iLight* sl = ParseStatlight (ldr_context, child);
	  if (!sl) goto error;
          sector->GetLights ()->Add (sl);
	  sl->DecRef ();
	}
        break;
      case XMLTOKEN_NODE:
        {
          iMapNode *n = ParseNode (child, sector);
	  if (n)
	  {
	    n->DecRef ();
	  }
	  else
	  {
	    goto error;
	  }
	}
        break;
      case XMLTOKEN_FOG:
        {
          csFog *f = sector->GetFog ();
          f->enabled = true;
	  f->red = child->GetAttributeValueAsFloat ("red");
	  f->green = child->GetAttributeValueAsFloat ("green");
	  f->blue = child->GetAttributeValueAsFloat ("blue");
	  f->density = child->GetAttributeValueAsFloat ("density");
        }
        break;
      case XMLTOKEN_KEY:
        {
          iKeyValuePair* kvp = 0;
          SyntaxService->ParseKey (child, kvp);
	  if (kvp)
          {
            sector->QueryObject()->ObjAdd (kvp->QueryObject ());
	    kvp->DecRef ();
          } else
	    goto error;
        }
        break;
      default:
	SyntaxService->ReportBadToken (child);
	goto error;
    }
  }
  if (do_culler)
  {
    bool rc = sector->SetVisibilityCullerPlugin (culplugname, culler_params);
    if (!rc)
    {
      SyntaxService->ReportError (
	      	"crystalspace.maploader.load.sector",
		node, "Could not load visibility culler for sector '%s'!",
		secname ? secname : "<noname>");
      goto error;
    }
  }

  delete[] culplugname;
  return sector;

error:
  delete[] culplugname;
  return 0;
}

iEngineSequenceManager* csLoader::GetEngineSequenceManager ()
{
  if (!eseqmgr)
  {
    eseqmgr = CS_QUERY_REGISTRY (object_reg, iEngineSequenceManager);
    if (!eseqmgr)
    {
      csRef<iPluginManager> plugin_mgr (
  	  CS_QUERY_REGISTRY (object_reg, iPluginManager));
      eseqmgr = CS_LOAD_PLUGIN (plugin_mgr,
    	  "crystalspace.utilities.sequence.engine", iEngineSequenceManager);
      if (!eseqmgr)
      {
        ReportError ("crystalspace.maploader",
	  "Could not load the engine sequence manager!");
        return 0;
      }
      if (!object_reg->Register (eseqmgr, "iEngineSequenceManager"))
      {
        ReportError ("crystalspace.maploader",
	  "Could not register the engine sequence manager!");
        return 0;
      }
    }
  }
  return eseqmgr;
}

bool csLoader::ParseVariableList (iLoaderContext* ldr_context,
	iDocumentNode* node)
{
  if (!Engine) return false;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_VARIABLE:
	if (!ParseSharedVariable (ldr_context, child))
	  return false;
        break;
      default:
        SyntaxService->ReportBadToken (child);
	return false;
    }
  }

  return true;
}

bool csLoader::ParseSharedVariable (iLoaderContext* ldr_context,
	iDocumentNode* node)
{
  csRef<iSharedVariable> v = Engine->GetVariableList()->New ();
  AddToRegion (ldr_context, v->QueryObject ());

  v->SetName (node->GetAttributeValue ("name"));

  if (v->GetName ())
  {
    csRef<iDocumentNode> colornode = node->GetNode ("color");
    csRef<iDocumentNode> vectornode = node->GetNode ("v");
    if (colornode)
    {
      csColor c;
      if (!SyntaxService->ParseColor (colornode, c))
	return false;
      v->SetColor (c);
    }
    else if (vectornode)
    {
      csVector3 vec;
      if (!SyntaxService->ParseVector (vectornode, vec))
	return false;
      v->SetVector (vec);
    }
    else
    {
      v->Set (node->GetAttributeValueAsFloat ("value"));
    }
    Engine->GetVariableList ()->Add (v);
  }
  else
  {
    SyntaxService->ReportError ("crystalspace.maploader",
    	node, "Variable tag does not have 'name' attribute.");
    return false;
  }
  return true;
}

//========================================================================
//========================================================================

bool csLoader::ParseShaderList (iLoaderContext* ldr_context,
	iDocumentNode* node)
{
  csRef<iShaderManager> shaderMgr (
    CS_QUERY_REGISTRY (csLoader::object_reg, iShaderManager));

  if(!shaderMgr)
  {
    ReportNotify ("iShaderManager not found, ignoring shaders!");
    return true;
  }

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_SHADER:
      {
	ParseShader (ldr_context, child, shaderMgr);
      }
      break;
    }
  }
  return true;
}

bool csLoader::ParseShader (iLoaderContext* ldr_context,
			    iDocumentNode* node,
			    iShaderManager* shaderMgr)
{
  /*csRef<iShader> shader (shaderMgr->CreateShader ());
  //test if we have a childnode named file, if so load from file, else
  //use inline loading
  csRef<iDocumentNode> fileChild = child->GetNode ("file");
  if (fileChild)
  {
    csRef<iDataBuffer> db = VFS->ReadFile (fileChild->GetContentsValue ());
    shader->Load (db);
  }
  else
  {
    shader->Load (child);
  }*/

  csRef<iDocumentNode> shaderNode;
  csRef<iDocumentNode> fileChild = node->GetNode ("file");

  csRef<iVFS> vfs;
  vfs = CS_QUERY_REGISTRY(object_reg, iVFS);
  csVfsDirectoryChanger dirChanger (vfs);

  if (fileChild)
  {
    csString filename (fileChild->GetContentsValue ());
    csRef<iFile> shaderFile = vfs->Open (filename, VFS_FILE_READ);

    if(!shaderFile)
    {
      ReportWarning ("crystalspace.maploader",
	"Unable to open shader file '%s'!", filename.GetData());
      return false;
    }

    csRef<iDocumentSystem> docsys =
      CS_QUERY_REGISTRY(object_reg, iDocumentSystem);
    if (docsys == 0)
      docsys.AttachNew (new csTinyDocumentSystem ());
    csRef<iDocument> shaderDoc = docsys->CreateDocument ();
    const char* err = shaderDoc->Parse (shaderFile, false);
    if (err != 0)
    {
      ReportWarning ("crystalspace.maploader",
	"Could not parse shader file '%s': %s",
	filename.GetData(), err);
      return false;
    }
    shaderNode = shaderDoc->GetRoot ()->GetNode ("shader");
    dirChanger.ChangeTo (filename);
  }
  else
  {
    shaderNode = node->GetNode ("shader");
  }

  const char* name = shaderNode->GetAttributeValue ("name");
  if (ldr_context->CheckDupes () && name)
  {
    iShader* m = shaderMgr->GetShader (name);
    if (m) return true;
  }

  const char* type = shaderNode->GetAttributeValue ("compiler");
  if (type == 0)
    type = shaderNode->GetAttributeValue ("type");
  if (type == 0)
  {
    SyntaxService->ReportError ("crystalspace.maploader", shaderNode,
      "'compiler' attribute is missing!");

    return false;
  }
  csRef<iShaderCompiler> shcom = shaderMgr->GetCompiler (type);
  csRef<iShader> shader = shcom->CompileShader (shaderNode);
  if (shader)
  {
    shader->SetFileName(fileChild->GetContentsValue ());
    AddToRegion (ldr_context, shader->QueryObject ());
    shaderMgr->RegisterShader (shader);
  }
  else 
    return false;
  return true;
}

void csLoader::CollectAllChildren (iMeshWrapper* meshWrapper,
	csRefArray<iMeshWrapper>& meshesArray)
{  
  size_t lastMeshVisited = 0;
  meshesArray.Push (meshWrapper);
    
  while (lastMeshVisited < meshesArray.Length ())
  {
    // Get the children of the current mesh (ie 'mesh').
    csRef<iMeshList> mL = meshesArray[lastMeshVisited++]->GetChildren ();
    int i;
    for (i = 0; i < mL->GetCount (); i++)
      meshesArray.Push (mL->Get (i));
  }

  return;
}

void csLoader::ClosedFlags (iMeshWrapper* mesh)
{
  iObjectModel* objmodel = mesh->GetMeshObject ()->GetObjectModel ();
  if (objmodel->GetPolygonMeshShadows ())
    objmodel->GetPolygonMeshShadows ()->GetFlags ().Set (
    CS_POLYMESH_CLOSED | CS_POLYMESH_NOTCLOSED, CS_POLYMESH_CLOSED);
  if (objmodel->GetPolygonMeshViscull ())
    objmodel->GetPolygonMeshViscull ()->GetFlags ().Set (
    CS_POLYMESH_CLOSED | CS_POLYMESH_NOTCLOSED, CS_POLYMESH_CLOSED);
  if (objmodel->GetPolygonMeshShadows ())
    objmodel->GetPolygonMeshShadows ()->GetFlags ().Set (
    CS_POLYMESH_CLOSED | CS_POLYMESH_NOTCLOSED, CS_POLYMESH_CLOSED);
}

void csLoader::ConvexFlags (iMeshWrapper* mesh)
{
  iObjectModel* objmodel = mesh->GetMeshObject ()->GetObjectModel ();
  if (objmodel->GetPolygonMeshShadows ())
    objmodel->GetPolygonMeshShadows ()->GetFlags ().Set (
    CS_POLYMESH_CONVEX | CS_POLYMESH_NOTCONVEX, CS_POLYMESH_CONVEX);
  if (objmodel->GetPolygonMeshViscull ())
    objmodel->GetPolygonMeshViscull ()->GetFlags ().Set (
    CS_POLYMESH_CONVEX | CS_POLYMESH_NOTCONVEX, CS_POLYMESH_CONVEX);
  if (objmodel->GetPolygonMeshShadows ())
    objmodel->GetPolygonMeshShadows ()->GetFlags ().Set (
    CS_POLYMESH_CONVEX | CS_POLYMESH_NOTCONVEX, CS_POLYMESH_CONVEX);
}
