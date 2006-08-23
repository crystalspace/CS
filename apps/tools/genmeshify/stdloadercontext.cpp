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

#include "crystalspace.h"

#include "genmeshify.h"
#include "stdloadercontext.h"

namespace genmeshify
{

StdLoaderContext::StdLoaderContext (App* app, iEngine* Engine, 
                                    iRegion* region, bool checkDupes) : 
  scfImplementationType (this), app (app), Engine (Engine), region (region),
  checkDupes (checkDupes)
{
}

StdLoaderContext::~StdLoaderContext ()
{
}

iSector* StdLoaderContext::FindSector (const char* name)
{
  iSector* s = Engine->FindSector (name, 0);
  return s;
}

iMaterialWrapper* StdLoaderContext::FindMaterial (const char* filename)
{
  iMaterialWrapper* mat = Engine->FindMaterial (filename, 0);
  if (mat)
    return mat;

  app->Report (CS_REPORTER_SEVERITY_NOTIFY, "Could not find material '%s'. "
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

    return mat;
  }

  return 0;
}

iMaterialWrapper* StdLoaderContext::FindNamedMaterial (const char* name, 
                                                       const char *filename)
{
  iMaterialWrapper* mat = Engine->FindMaterial (name, 0);
  if (mat)
    return mat;

  app->Report (CS_REPORTER_SEVERITY_NOTIFY, "Could not find material '%s'. "
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
    return mat;
  }

  return 0;
}


iMeshFactoryWrapper* StdLoaderContext::FindMeshFactory (const char* name)
{
  iMeshFactoryWrapper* fact = Engine->FindMeshFactory (name, 0);
  return fact;
}

iMeshWrapper* StdLoaderContext::FindMeshObject (const char* name)
{
  iMeshWrapper* mesh = Engine->FindMeshObject (name, 0);
  return mesh;
}

iLight* StdLoaderContext::FindLight (const char *name)
{
  csRef<iLightIterator> li = Engine->GetLightIterator (0);
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
  	app->objectRegistry, iShaderManager);
  if (!shaderMgr) return 0;
  iShader* shader = shaderMgr->GetShader (name);
  return shader;
}

iTextureWrapper* StdLoaderContext::FindTexture (const char* name)
{
  iTextureWrapper* result;
  result = Engine->GetTextureList ()->FindByName (name);

  if (!result)
  {
    app->Report (CS_REPORTER_SEVERITY_NOTIFY, 
      "Could not find texture '%s'. Attempting to load.", name);
    csRef<iTextureWrapper> rc = app->loader->LoadTexture (name, name,
    	CS_TEXTURE_3D, 0, true, true, true, region);
    result = rc;
  }
  return result;
}

iTextureWrapper* StdLoaderContext::FindNamedTexture (const char* name,
                                                     const char *filename)
{
  iTextureWrapper* result;
  result = Engine->GetTextureList ()->FindByName (name);

  if (!result)
  {
    app->Report (CS_REPORTER_SEVERITY_NOTIFY, 
      "Could not find texture '%s'. Attempting to load.", name);
    csRef<iTextureWrapper> rc = app->loader->LoadTexture (name, filename,
	CS_TEXTURE_3D, 0, false, false, region != 0);
    result = rc;
  }
  return result;
}

} // namespace genmeshify
