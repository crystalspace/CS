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
#include "cssys/sysfunc.h"
#include "qint.h"
#include "csutil/cfgfile.h"
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "cstool/gentrtex.h"
#include "cstool/keyval.h"
#include "cstool/sndwrap.h"
#include "cstool/mapnode.h"
#include "cstool/mdltool.h"
#include "csgfx/csimage.h"
#include "csloader.h"

#include "iutil/databuff.h"
#include "imap/reader.h"
#include "imesh/lighting.h"
#include "imesh/sprite3d.h"
#include "imesh/skeleton.h"
#include "imesh/object.h"
#include "iengine/engine.h"
#include "iengine/motion.h"
#include "iengine/skelbone.h"
#include "iengine/region.h"
#include "iengine/texture.h"
#include "iengine/material.h"
#include "iengine/collectn.h"
#include "iengine/sector.h"
#include "iengine/movable.h"
#include "iengine/halo.h"
#include "iengine/campos.h"
#include "iengine/light.h"
#include "iengine/statlght.h"
#include "iengine/mesh.h"
#include "iengine/lod.h"
#include "ivideo/material.h"
#include "igraphic/imageio.h"
#include "isound/loader.h"
#include "isound/renderer.h"
#include "iutil/vfs.h"
#include "iutil/plugin.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/portal.h"
#include "imesh/thing/thing.h"
#include "imesh/mdlconv.h"
#include "imesh/mdldata.h"
#include "imesh/crossbld.h"
#include "ivaria/reporter.h"

//---------------------------------------------------------------------------

void csLoader::csLoaderStats::Init()
{
  polygons_loaded = 0;
  portals_loaded  = 0;
  sectors_loaded  = 0;
  things_loaded   = 0;
  lights_loaded   = 0;
  curves_loaded   = 0;
  meshes_loaded   = 0;
  sounds_loaded   = 0;
}

csLoader::csLoaderStats::csLoaderStats()
{
  Init();
}

// Define all tokens used through this file
CS_TOKEN_DEF_START
  CS_TOKEN_DEF (ADDON)
  CS_TOKEN_DEF (ATTENUATION)
  CS_TOKEN_DEF (CAMERA)
  CS_TOKEN_DEF (CENTER)
  CS_TOKEN_DEF (COLLECTION)
  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (CONVEX)
  CS_TOKEN_DEF (CULLER)
  CS_TOKEN_DEF (DETAIL)
  CS_TOKEN_DEF (DYNAMIC)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (FILE)
  CS_TOKEN_DEF (FOG)
  CS_TOKEN_DEF (FORWARD)
  CS_TOKEN_DEF (HALO)
  CS_TOKEN_DEF (HARDMOVE)
  CS_TOKEN_DEF (INVISIBLE)
  CS_TOKEN_DEF (KEY)
  CS_TOKEN_DEF (LEVEL)
  CS_TOKEN_DEF (LIBRARY)
  CS_TOKEN_DEF (LIGHT)
  CS_TOKEN_DEF (LMCACHE)
  CS_TOKEN_DEF (LOD)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MATERIALS)
  CS_TOKEN_DEF (MATRIX)
  CS_TOKEN_DEF (MESHFACT)
  CS_TOKEN_DEF (MESHOBJ)
  CS_TOKEN_DEF (MESHREF)
  CS_TOKEN_DEF (MOVE)
  CS_TOKEN_DEF (NODE)
  CS_TOKEN_DEF (NOLIGHTING)
  CS_TOKEN_DEF (NOSHADOWS)
  CS_TOKEN_DEF (PARAMS)
  CS_TOKEN_DEF (PLUGIN)
  CS_TOKEN_DEF (PLUGINS)
  CS_TOKEN_DEF (POSITION)
  CS_TOKEN_DEF (PRIORITY)
  CS_TOKEN_DEF (RADIUS)
  CS_TOKEN_DEF (REGION)
  CS_TOKEN_DEF (RENDERPRIORITIES)
  CS_TOKEN_DEF (SECTOR)
  CS_TOKEN_DEF (SOUND)
  CS_TOKEN_DEF (SOUNDS)
  CS_TOKEN_DEF (START)
  CS_TOKEN_DEF (TEXTURES)
  CS_TOKEN_DEF (MAT_SET)
  CS_TOKEN_DEF (UP)
  CS_TOKEN_DEF (V)
  CS_TOKEN_DEF (WORLD)
  CS_TOKEN_DEF (ZFILL)
  CS_TOKEN_DEF (ZNONE)
  CS_TOKEN_DEF (ZUSE)
  CS_TOKEN_DEF (ZTEST)
CS_TOKEN_DEF_END

//---------------------------------------------------------------------------

void csLoader::ReportError (const char* id, const char* description, ...)
{
  va_list arg;
  va_start (arg, description);

  if (Reporter)
  {
    Reporter->ReportV (CS_REPORTER_SEVERITY_ERROR, id, description, arg);
  }
  else
  {
    char buf[1024];
    vsprintf (buf, description, arg);
    csPrintf ("Error ID: %s\n", id);
    csPrintf ("Description: %s\n", buf);
  }
  va_end (arg);
}

void csLoader::ReportNotify (const char* description, ...)
{
  va_list arg;
  va_start (arg, description);

  if (Reporter)
  {
    Reporter->ReportV (CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.maploader",
    	description, arg);
  }
  else
  {
    csPrintfV (description, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

iMaterialWrapper *csLoader::FindMaterial (const char *iName)
{
  iMaterialWrapper* mat;
  if (ResolveOnlyRegion && Engine->GetCurrentRegion ())
    mat = Engine->GetCurrentRegion ()->FindMaterial (iName);
  else
    mat = Engine->GetMaterialList ()->FindByName (iName);
  if (mat)
    return mat;

  iTextureWrapper* tex;
  if (ResolveOnlyRegion && Engine->GetCurrentRegion ())
    tex = Engine->GetCurrentRegion ()->FindTexture (iName);
  else
    tex = Engine->GetTextureList ()->FindByName (iName);
  if (tex)
  {
    // Add a default material with the same name as the texture
    iMaterial* material = Engine->CreateBaseMaterial (tex);
    iMaterialWrapper *mat = Engine->GetMaterialList ()->NewMaterial (material);
    mat->QueryObject()->SetName (iName);
    material->DecRef ();
    return mat;
  }

  ReportError (
    "crystalspace.maploader.find.material",
    "Could not find material named '%s'!", iName);
  return NULL;
}

//---------------------------------------------------------------------------

iSector* csLoader::FindSector (const char* name)
{
  if (ResolveOnlyRegion && Engine->GetCurrentRegion ())
  {
    // We can't use iRegion::FindSector() here because that routine
    // also returns sectors that are added to the region but not linked
    // to the engine. Here we need to find sectors that are linked to the
    // engine AND are in a region. Otherwise we risk finding the dummy
    // sectors that are created during the loading process.
    iRegion* reg = Engine->GetCurrentRegion ();
    iSectorList* sl = Engine->GetSectors ();
    for (int i = 0 ; i < sl->GetCount () ; i++)
    {
      iSector* s = sl->Get (i);
      if (!strcmp (s->QueryObject ()->GetName (), name))
      {
        if (reg->IsInRegion (s->QueryObject ()))
          return s;
      }
    }
    return NULL;
  }
  else
    return Engine->GetSectors ()->FindByName (name);
}

bool csLoader::ResolvePortalSectors (iThingState *th)
{
  int i;
  for (i=0;  i < th->GetPolygonCount ();  i++)
  {
    iPolygon3D* p = th->GetPolygon (i);
    if (p && p->GetPortal ())
    {
      iPortal *portal = p->GetPortal ();
      iSector *stmp = portal->GetSector ();
      // First we check if this sector already has some meshes.
      // If so then this is not a sector we have to resolve.
      // This test is here to make this code a little more robust.
      if (stmp->GetMeshes ()->GetCount () > 0) continue;
      iSector* snew = FindSector (stmp->QueryObject ()->GetName ());
      if (!snew)
      {
	ReportError (
	  "crystalspace.maploader.load.portals",
	  "Sector '%s' not found for portal in polygon '%s/%s'!",
          stmp->QueryObject ()->GetName (),
          p->QueryObject ()->GetObjectParent ()->GetName (),
          p->QueryObject ()->GetName ());
        return false;
      }
      portal->SetSector (snew);
      // This DecRef() is safe since we know this is supposed to be a dummy
      // sector. So there will only be one reference to that sector (from
      // this portal).
      stmp->DecRef();
    }
  }
  return true;
}

bool csLoader::LoadMap (char* buf)
{
  if (!Engine) return false;

  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (WORLD)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (ADDON)
    CS_TOKEN_TABLE (SECTOR)
    CS_TOKEN_TABLE (COLLECTION)
    CS_TOKEN_TABLE (MESHFACT)
    CS_TOKEN_TABLE (MATERIALS)
    CS_TOKEN_TABLE (MAT_SET)
    CS_TOKEN_TABLE (TEXTURES)
    CS_TOKEN_TABLE (LIBRARY)
    CS_TOKEN_TABLE (START)
    CS_TOKEN_TABLE (SOUNDS)
    CS_TOKEN_TABLE (KEY)
    CS_TOKEN_TABLE (REGION)
    CS_TOKEN_TABLE (RENDERPRIORITIES)
    CS_TOKEN_TABLE (PLUGINS)
  CS_TOKEN_TABLE_END

  csResetParserLine();
  char *name, *data;

  if (csGetObject (&buf, tokens, &name, &data))
  {
    if (!data)
    {
      ReportError (
	  "crystalspace.maploader.parse.badformat",
	  "Expected parameters instead of '%s' while parsing world!", buf);
      return false;
    }
    long cmd;
    char* params;

    while ((cmd = csGetObject (&data, commands, &name, &params)) > 0)
    {
      if (!params)
      {
        ReportError (
	  "crystalspace.maploader.parse.badformat",
	  "Expected parameters instead of '%s' while parsing world!", data);
        return false;
      }
      switch (cmd)
      {
        case CS_TOKEN_RENDERPRIORITIES:
	  Engine->ClearRenderPriorities ();
	  if (!LoadRenderPriorities (params))
	    return false;
	  break;
        case CS_TOKEN_ADDON:
	  if (!LoadAddOn (params, (iEngine*)Engine))
	    return false;
      	  break;
        case CS_TOKEN_MESHFACT:
          {
            iMeshFactoryWrapper* t = Engine->CreateMeshFactory(name);
	    if (!t || !LoadMeshObjectFactory (t, params))
	    {
	      ReportError (
			   "crystalspace.maploader.load.meshfactory",
			   "Could not load mesh object factory '%s'!",
			   name);
	      if (t) t->DecRef ();
	      return false;
	    }
	    if (t) t->DecRef ();
          }
	  break;
        case CS_TOKEN_REGION:
	  {
	    char str[255];
	    csScanStr (params, "%s", str);
	    if (*str)
	      Engine->SelectRegion (str);
	    else
	      Engine->SelectRegion ((iRegion*)NULL);
	  }
	  break;
        case CS_TOKEN_SECTOR:
          if (!FindSector (name))
	  {
            if (!ParseSector (name, params))
	      return false;
	  }
          break;
        case CS_TOKEN_COLLECTION:
          if (!ParseCollection (name, params))
	    return false;
          break;
	case CS_TOKEN_MAT_SET:
          if (!ParseMaterialList (params, name))
            return false;
          break;
	case CS_TOKEN_PLUGINS:
	  if (!LoadPlugins (params))
	    return false;
	  break;
        case CS_TOKEN_TEXTURES:
          if (!ParseTextureList (params))
            return false;
          break;
        case CS_TOKEN_MATERIALS:
          if (!ParseMaterialList (params))
            return false;
          break;
        case CS_TOKEN_SOUNDS:
          if (!LoadSounds (params))
            return false;
          break;
        case CS_TOKEN_LIBRARY:
          if (!LoadLibraryFile (name))
	    return false;
          break;
        case CS_TOKEN_START:
        {
	  iCameraPosition* campos = Engine->GetCameraPositions ()->
	  	NewCameraPosition (name ? name : "Start");
	  if (!ParseStart (params, campos))
	    return false;
          break;
        }
        case CS_TOKEN_KEY:
	  {
            iKeyValuePair* kvp = ParseKey (params, Engine->QueryObject());
	    if (kvp)
	      kvp->DecRef ();
	    else
	      return false;
	  }
          break;
      }
    }
    if (cmd == CS_PARSERR_TOKENNOTFOUND)
    {
      TokenError ("a map");
      return false;
    }
  }
 
  int i,j;
  for (i=0; i<Engine->GetSectors ()->GetCount(); i++)
  {
    iSector *Sector = Engine->GetSectors ()->Get (i);
    if (ResolveOnlyRegion)
    {
      // This test avoids redoing sectors that are not in this region.
      // @@@ But it would be better if we could even limit this loop
      // to the sectors that were loaded here.
      iRegion* region = Engine->GetCurrentRegion ();
      if (region && !region->IsInRegion (Sector->QueryObject ()))
      {
        continue;
      }
    }
    iMeshList* ml = Sector->GetMeshes ();
    for (j=0 ; j < ml->GetCount () ; j++)
    {
      iMeshWrapper *Mesh = ml->Get (j);
      if (Mesh)
      {
        iThingState* Thing = SCF_QUERY_INTERFACE_SAFE (
		Mesh->GetMeshObject(), iThingState);
        if (Thing)
        {
          bool rc = ResolvePortalSectors (Thing);
	  Thing->DecRef ();
	  if (!rc) return false;
        }
      }
    }
  }

  return true;
}

bool csLoader::LoadMapFile (const char* file, bool iClearEngine,
  bool iOnlyRegion)
{
  Stats->Init ();
  if (iClearEngine) Engine->DeleteAll ();
  ResolveOnlyRegion = iOnlyRegion;

  iDataBuffer *buf = VFS->ReadFile (file);

  if (!buf || !buf->GetSize ())
  {
    if (buf) buf->DecRef ();
    ReportError (
	      "crystalspace.maploader.parse.map",
    	      "Could not open map file '%s' on VFS!", file);
    return false;
  }

  iConfigFile *cfg = new csConfigFile ("map.cfg", VFS);
  if (cfg)
  {
    Engine->SetLightmapCellSize (cfg->GetInt ("Engine.Lighting.LightmapSize",
    	Engine->GetLightmapCellSize ()));
    cfg->DecRef();
  }

  if (!LoadMap (**buf))
  {
    buf->DecRef ();
    return false;
  }

  if (Stats->polygons_loaded)
  {
    ReportNotify ("Loaded map file:");
    ReportNotify ("  %d polygons (%d portals),", Stats->polygons_loaded,
      Stats->portals_loaded);
    ReportNotify ("  %d sectors, %d things, %d meshes, ", Stats->sectors_loaded,
      Stats->things_loaded, Stats->meshes_loaded);
    ReportNotify ("  %d curves, %d lights, %d sounds.", Stats->curves_loaded,
      Stats->lights_loaded, Stats->sounds_loaded);
  } /* endif */

  buf->DecRef ();

  return true;
}

//---------------------------------------------------------------------------

bool csLoader::LoadPlugins (char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (PLUGIN)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (
	  "crystalspace.maploader.parse.badformat",
	  "Expected parameters instead of '%s' while parsing plugin!", buf);
      return false;
    }
    switch (cmd)
    {
      case CS_TOKEN_PLUGIN:
	csScanStr (params, "%s", str);
	loaded_plugins.NewPlugin (name, str);
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    TokenError ("plugin descriptors");
    return false;
  }

  return true;
}

//---------------------------------------------------------------------------

bool csLoader::LoadLibrary (char* buf)
{
  if (!Engine) return false;

  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (LIBRARY)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (ADDON)
    CS_TOKEN_TABLE (TEXTURES)
    CS_TOKEN_TABLE (MATERIALS)
    CS_TOKEN_TABLE (MESHFACT)
    CS_TOKEN_TABLE (SOUNDS)
  CS_TOKEN_TABLE_END

  char *name, *data;
  if (csGetObject (&buf, tokens, &name, &data))
  {
    // Empty LIBRARY () directive?
    if (!data)
      return false;

    long cmd;
    char* params;

    while ((cmd = csGetObject (&data, commands, &name, &params)) > 0)
    {
      if (!params)
      {
        ReportError (
	  "crystalspace.maploader.parse.badformat",
	  "Expected parameters instead of '%s' while parsing library!", data);
        return false;
      }

      switch (cmd)
      {
        case CS_TOKEN_ADDON:
	  if (!LoadAddOn (params, (iEngine*)Engine))
	    return false;
      	  break;
        case CS_TOKEN_TEXTURES:
          // Append textures to engine.
          if (!ParseTextureList (params))
            return false;
          break;
        case CS_TOKEN_MATERIALS:
          if (!ParseMaterialList (params))
            return false;
          break;
        case CS_TOKEN_SOUNDS:
          if (!LoadSounds (params))
            return false;
          break;
        case CS_TOKEN_MESHFACT:
          {
            iMeshFactoryWrapper* t = Engine->CreateMeshFactory (name);
	    if (t)
	    {
	      if (!LoadMeshObjectFactory (t, params))
	      {
		t->DecRef ();
		ReportError (
			     "crystalspace.maploader.load.library.meshfactory",
			     "Could not load mesh object factory '%s' in library!",
			     name);
		return false;
	      }
	      t->DecRef ();
	    }
	  }
	  break;
      }
    }
    if (cmd == CS_PARSERR_TOKENNOTFOUND)
    {
      TokenError ("a library file");
      return false;
    }
  }
  return true;
}

bool csLoader::LoadLibraryFile (const char* fname)
{
  iDataBuffer *buf = VFS->ReadFile (fname);

  if (!buf || !buf->GetSize ())
  {
    if (buf) buf->DecRef ();
    ReportError (
	      "crystalspace.maploader.parse.library",
    	      "Could not open library file '%s' on VFS!", fname);
    return false;
  }

  ResolveOnlyRegion = false;
  bool retcode = LoadLibrary (**buf);

  buf->DecRef ();
  
  return retcode;
}

//---------------------------------------------------------------------------

bool csLoader::LoadSounds (char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (SOUND)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (options)
    CS_TOKEN_TABLE (FILE)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (
	  "crystalspace.maploader.parse.badformat",
	  "Expected parameters instead of '%s' while parsing sounds!", buf);
      return false;
    }
    switch (cmd)
    {
      case CS_TOKEN_SOUND:
      {
        const char* filename = name;
        char* maybename;
        cmd = csGetCommand (&params, options, &maybename);
        if (cmd == CS_TOKEN_FILE)
          filename = maybename;
        else if (cmd == CS_PARSERR_TOKENNOTFOUND)
        {
          ReportError (
	    "crystalspace.maploader.parse.badtoken",
            "Unknown token '%s' found while parsing SOUND directive!",
	    csGetLastOffender());
	  return false;
        }
        iSoundWrapper *snd =
	  CS_GET_NAMED_CHILD_OBJECT (Engine->QueryObject (), iSoundWrapper, name);
        if (!snd)
          LoadSound (name, filename);
      }
      break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    TokenError ("the list of sounds");
    return false;
  }

  return true;
}

//---------------------------------------------------------------------------

bool csLoader::LoadLodControl (iLODControl* lodctrl, char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (LEVEL)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  float level = 1;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (
	  "crystalspace.maploader.parse.badformat",
	  "Expected parameters instead of '%s' while parsing a LOD control!",
	  buf);
      return false;
    }
    switch (cmd)
    {
      case CS_TOKEN_LEVEL:
        csScanStr (params, "%f", &level);
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    TokenError ("a LOD control");
    return false;
  }

  lodctrl->SetLOD (level);

  return true;
}

//---------------------------------------------------------------------------

iMeshFactoryWrapper* csLoader::LoadMeshObjectFactory (const char* fname)
{
  if (!Engine) return NULL;

  iDataBuffer *databuff = VFS->ReadFile (fname);

  if (!databuff || !databuff->GetSize ())
  {
    if (databuff) databuff->DecRef ();
    ReportError (
	      "crystalspace.maploader.parse.meshfactory",
    	      "Could not open mesh object file '%s' on VFS!", fname);
    return NULL;
  }

  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (MESHFACT)
  CS_TOKEN_TABLE_END

  char *name, *data;
  char *buf = **databuff;

  if (csGetObject (&buf, tokens, &name, &data))
  {
    if (!data)
    {
      ReportError (
	  "crystalspace.maploader.parse.badformat",
	  "Expected parameters instead of '%s' while parsing mesh factory!",
	  buf);
      databuff->DecRef ();
      return NULL;
    }

    iMeshFactoryWrapper* t = Engine->CreateMeshFactory(name);
    if (LoadMeshObjectFactory (t, data))
    {
      databuff->DecRef ();
      return t;
    }
    else
    {
      ReportError (
	      	"crystalspace.maploader.load.meshfactory",
		"Could not load mesh object factory '%s' from file '%s'!",
		name, fname);
      iMeshFactoryWrapper* factwrap = Engine->GetMeshFactories ()
      	->FindByName (name);
      Engine->GetMeshFactories ()->Remove (factwrap);
      databuff->DecRef ();
      return NULL;
    }
  }
  databuff->DecRef ();
  return NULL;
}

bool csLoader::LoadMeshObjectFactory (iMeshFactoryWrapper* stemp, char* buf,
	csReversibleTransform* transf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (ADDON)
    CS_TOKEN_TABLE (FILE)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (PARAMS)
    CS_TOKEN_TABLE (PLUGIN)
    CS_TOKEN_TABLE (MESHFACT)
    CS_TOKEN_TABLE (MOVE)
    CS_TOKEN_TABLE (LOD)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_matvec)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];
  iLoaderPlugin* plug = NULL;
  str[0] = 0;
  iMaterialWrapper *mat = NULL;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (
	  "crystalspace.maploader.parse.badformat",
	  "Expected parameters instead of '%s' while parsing mesh factory!",
	  buf);
      return false;
    }
    switch (cmd)
    {
      case CS_TOKEN_LOD:
        {
	  if (!stemp->GetMeshObjectFactory ())
	  {
            ReportError (
	      "crystalspace.maploader.parse.meshfactory",
              "Please use PARAMS before specifying LOD!");
	    return false;
	  }
	  iLODControl* lodctrl = SCF_QUERY_INTERFACE (
	    	stemp->GetMeshObjectFactory (),
		iLODControl);
	  if (!lodctrl)
	  {
            ReportError (
	      "crystalspace.maploader.parse.meshfactory",
              "This mesh factory doesn't implement LOD control!");
	    return false;
	  }
	  if (!LoadLodControl (lodctrl, params))
	  {
	    lodctrl->DecRef ();
	    return false;
	  }
	  lodctrl->DecRef ();
	}
        break;
      case CS_TOKEN_ADDON:
	if (!LoadAddOn (params, stemp))
	  return false;
      	break;
      case CS_TOKEN_PARAMS:
	if (!plug)
	{
          ReportError (
	      "crystalspace.maploader.load.plugin",
              "Could not load plugin!");
	  return false;
	}
	else
	{
	  // We give here the iMeshObjectFactory as the context. If this
	  // is a new factory this will be NULL. Otherwise it is possible
	  // to append information to the already loaded factory.
	  iBase* mof = plug->Parse (params, Engine->GetMaterialList (),
	  	Engine->GetMeshFactories (), stemp->GetMeshObjectFactory ());
	  if (!mof)
	  {
            ReportError (
	      "crystalspace.maploader.parse.plugin",
              "Could not parse plugin!");
	    return false;
	  }
	  else
	  {
	    iMeshObjectFactory* mof2 = SCF_QUERY_INTERFACE (mof,
	    	iMeshObjectFactory);
	    if (!mof2)
	    {
              ReportError (
	        "crystalspace.maploader.parse.meshfactory",
		"Returned object does not implement iMeshObjectFactory!");
	      return false;
	    }
	    stemp->SetMeshObjectFactory (mof2);
	    mof2->SetLogicalParent (stemp);
	    mof2->DecRef ();
	    mof->DecRef ();
	  }
	}
        break;

      case CS_TOKEN_MATERIAL:
        {
	  if (!stemp->GetMeshObjectFactory ())
	  {
            ReportError (
	      "crystalspace.maploader.parse.meshfactory",
              "Please use PARAMS before specifying MATERIAL!");
	    return false;
	  }
          csScanStr (params, "%s", str);
          mat = FindMaterial (str);
          if (mat)
	  {
	    iSprite3DFactoryState* state = SCF_QUERY_INTERFACE (
	    	stemp->GetMeshObjectFactory (),
		iSprite3DFactoryState);
	    if (!state)
	    {
              ReportError (
	        "crystalspace.maploader.parse.meshfactory",
                "Only use MATERIAL keyword with 3D sprite factories!");
	      return false;
	    }
            state->SetMaterialWrapper (mat);
	    state->DecRef ();
	  }
          else
          {
            ReportError (
	      "crystalspace.maploader.parse.unknownmaterial",
              "Material '%s' not found!", str);
	    return false;
          }
        }
        break;

      case CS_TOKEN_FILE:
        {
          if (!ModelConverter || !CrossBuilder) return false;

          csScanStr (params, "%s", str);
          iDataBuffer *buf = VFS->ReadFile (str);
	  if (!buf)
	  {
            ReportError (
	      "crystalspace.maploader.parse.loadingmodel",
	      "Error opening file model '%s'!", str);
	    return false;
	  }

	  iModelData *Model = ModelConverter->Load (buf->GetUint8 (),
	  	buf->GetSize ());
	  buf->DecRef ();
          if (!Model) {
            ReportError (
 	      "crystalspace.maploader.parse.loadingmodel",
	      "Error loading file model '%s'!", str);
	    return false;
	  }

	  csModelDataTools::SplitObjectsByMaterial (Model);
	  csModelDataTools::MergeObjects (Model, false);
	  iMeshFactoryWrapper *stemp2 =
	    CrossBuilder->BuildSpriteFactoryHierarchy (Model, Engine, mat);
	  Model->DecRef ();

	  stemp->SetMeshObjectFactory (stemp2->GetMeshObjectFactory ());
	  int i;
	  iMeshFactoryList* mfl2 = stemp2->GetChildren ();
	  iMeshFactoryList* mfl = stemp->GetChildren ();
	  for (i=0; i<mfl2->GetCount (); i++)
	    mfl->Add (mfl2->Get (i));
        }
        break;

      case CS_TOKEN_PLUGIN:
	{
	  csScanStr (params, "%s", str);
	  plug = loaded_plugins.FindPlugin (str);
	}
        break;

      case CS_TOKEN_MESHFACT:
        {
          iMeshFactoryWrapper* t = Engine->CreateMeshFactory (name);
	  csReversibleTransform child_transf;
          if (!LoadMeshObjectFactory (t, params, &child_transf))
	  {
	    ReportError (
	    	"crystalspace.maploader.load.meshfactory",
		"Could not load mesh object factory '%s'!",
		name);
	    if (t) t->DecRef ();
	    return false;
	  }
	  stemp->GetChildren ()->Add (t);
	  t->SetTransform (child_transf);
	  t->DecRef ();
        }
	break;

      case CS_TOKEN_MOVE:
        {
	  if (!transf)
	  {
	    ReportError (
	    	"crystalspace.maploader.load.meshfactory",
		"MOVE is only useful for hierarchical transformations!");
	    return false;
	  }
          char* params2;
          while ((cmd = csGetObject (&params, tok_matvec, &name, &params2)) > 0)
          {
            if (!params2)
            {
	      ReportError (
		"crystalspace.maploader.parse.badformat",
		"Expected parameters instead of '%s' while parsing move!",
		params);
	      return false;
            }
            switch (cmd)
            {
              case CS_TOKEN_MATRIX:
              {
                csMatrix3 m;
                if (!ParseMatrix (params2, m))
		  return false;
		transf->SetO2T (m);
                break;
              }
              case CS_TOKEN_V:
              {
                csVector3 v;
                ParseVector (params2, v);
		transf->SetO2TTranslation (v);
                break;
              }
            }
          }
        }
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    TokenError ("a mesh factory");
    return false;
  }

  return true;
}

iMeshWrapper* csLoader::LoadMeshObjectFromFactory (char* buf)
{
  if (!Engine) return NULL;

  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (ADDON)
    CS_TOKEN_TABLE (KEY)
    CS_TOKEN_TABLE (MOVE)
    CS_TOKEN_TABLE (HARDMOVE)
    CS_TOKEN_TABLE (NOLIGHTING)
    CS_TOKEN_TABLE (NOSHADOWS)
    CS_TOKEN_TABLE (INVISIBLE)
    CS_TOKEN_TABLE (DETAIL)
    CS_TOKEN_TABLE (ZFILL)
    CS_TOKEN_TABLE (ZNONE)
    CS_TOKEN_TABLE (ZUSE)
    CS_TOKEN_TABLE (ZTEST)
    CS_TOKEN_TABLE (CAMERA)
    CS_TOKEN_TABLE (CONVEX)
    CS_TOKEN_TABLE (PRIORITY)
    CS_TOKEN_TABLE (LOD)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_matvec)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];
  str[0] = 0;
  char priority[255]; priority[0] = 0;

  Stats->meshes_loaded++;
  iMeshWrapper* mesh = NULL;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (
	  "crystalspace.maploader.parse.badformat",
	  "Expected parameters instead of '%s' while parsing mesh object!",
	  buf);
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_LOD:
        {
          if (!mesh)
	  {
	    ReportError (
	  	  "crystalspace.maploader.load.meshobject",
	  	  "First specify the parent factory with FACTORY!");
	    return NULL;
	  }
	  if (!mesh->GetMeshObject ())
	  {
            ReportError (
	      "crystalspace.maploader.parse.meshobject",
              "Mesh object is missing!");
	    return NULL;
	  }
	  iLODControl* lodctrl = SCF_QUERY_INTERFACE (
	    	mesh->GetMeshObject (),
		iLODControl);
	  if (!lodctrl)
	  {
            ReportError (
	      "crystalspace.maploader.parse.meshobject",
              "This mesh doesn't implement LOD control!");
	    return NULL;
	  }
	  if (!LoadLodControl (lodctrl, params))
	  {
	    lodctrl->DecRef ();
	    return NULL;
	  }
	  lodctrl->DecRef ();
	}
        break;
      case CS_TOKEN_PRIORITY:
	csScanStr (params, "%s", priority);
	break;
      case CS_TOKEN_ADDON:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with FACTORY!");
	  return NULL;
	}
	if (!LoadAddOn (params, mesh))
	  return NULL;
      	break;
      case CS_TOKEN_NOLIGHTING:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with FACTORY!");
	  return NULL;
	}
        mesh->GetFlags().Set (CS_ENTITY_NOLIGHTING);
        break;
      case CS_TOKEN_NOSHADOWS:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with FACTORY!");
	  return NULL;
	}
        mesh->GetFlags().Set (CS_ENTITY_NOSHADOWS);
        break;
      case CS_TOKEN_INVISIBLE:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with FACTORY!");
	  return NULL;
	}
        mesh->GetFlags().Set (CS_ENTITY_INVISIBLE);
        break;
      case CS_TOKEN_DETAIL:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with FACTORY!");
	  return NULL;
	}
        mesh->GetFlags().Set (CS_ENTITY_DETAIL);
        break;
      case CS_TOKEN_ZFILL:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with FACTORY!");
	  return NULL;
	}
        if (!priority[0]) strcpy (priority, "wall");
        mesh->SetZBufMode (CS_ZBUF_FILL);
        break;
      case CS_TOKEN_ZUSE:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with FACTORY!");
	  return NULL;
	}
        if (!priority[0]) strcpy (priority, "object");
        mesh->SetZBufMode (CS_ZBUF_USE);
        break;
      case CS_TOKEN_ZNONE:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with FACTORY!");
	  return NULL;
	}
        if (!priority[0]) strcpy (priority, "sky");
        mesh->SetZBufMode (CS_ZBUF_NONE);
        break;
      case CS_TOKEN_ZTEST:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with FACTORY!");
	  return NULL;
	}
        if (!priority[0]) strcpy (priority, "alpha");
        mesh->SetZBufMode (CS_ZBUF_TEST);
        break;
      case CS_TOKEN_CAMERA:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with FACTORY!");
	  return NULL;
	}
        if (!priority[0]) strcpy (priority, "sky");
        mesh->GetFlags().Set (CS_ENTITY_CAMERA);
        break;
      case CS_TOKEN_CONVEX:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with FACTORY!");
	  return NULL;
	}
        mesh->GetFlags().Set (CS_ENTITY_CONVEX);
        break;
      case CS_TOKEN_KEY:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with FACTORY!");
	  return NULL;
	}
	{
          iKeyValuePair* kvp = ParseKey (params, mesh->QueryObject());
	  if (kvp)
	    kvp->DecRef ();
	  else
	    return NULL;
	}
        break;
      case CS_TOKEN_HARDMOVE:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with FACTORY!");
	  return NULL;
	}
	else
        {
          char* params2;
	  csReversibleTransform tr;
          while ((cmd = csGetObject (&params, tok_matvec, &name, &params2)) > 0)
          {
            if (!params2)
            {
	      ReportError (
		"crystalspace.maploader.parse.badformat",
		"Expected parameters instead of '%s' while parsing hardmove!",
		params);
	      return NULL;
            }
            switch (cmd)
            {
              case CS_TOKEN_MATRIX:
              {
		csMatrix3 m;
                if (!ParseMatrix (params2, m))
		  return NULL;
                tr.SetT2O (m);
                break;
              }
              case CS_TOKEN_V:
              {
		csVector3 v;
                ParseVector (params2, v);
		tr.SetOrigin (v);
                break;
              }
            }
          }
	  mesh->HardTransform (tr);
        }
        break;
      case CS_TOKEN_MOVE:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with FACTORY!");
	  return NULL;
	}
	else
        {
          char* params2;
          mesh->GetMovable ()->SetTransform (csMatrix3 ());     // Identity
          mesh->GetMovable ()->SetPosition (csVector3 (0));
          while ((cmd = csGetObject (&params, tok_matvec, &name, &params2)) > 0)
          {
            if (!params2)
            {
	      ReportError (
		"crystalspace.maploader.parse.badformat",
		"Expected parameters instead of '%s' while parsing move!",
		params);
	      return NULL;
            }
            switch (cmd)
            {
              case CS_TOKEN_MATRIX:
              {
                csMatrix3 m;
                if (!ParseMatrix (params2, m))
		  return NULL;
                mesh->GetMovable ()->SetTransform (m);
                break;
              }
              case CS_TOKEN_V:
              {
                csVector3 v;
                ParseVector (params2, v);
                mesh->GetMovable ()->SetPosition (v);
                break;
              }
            }
          }
	  mesh->GetMovable ()->UpdateMove ();
        }
        break;

      case CS_TOKEN_FACTORY:
        if (mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"There is already a factory for this mesh!");
	  return NULL;
	}
	else
	{
	  csScanStr (params, "%s", str);
          iMeshFactoryWrapper* t = Engine->GetMeshFactories ()
	  	->FindByName (str);
          if (!t)
	  {
	    ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"Can't find factory '%s'!", name);
	    return NULL;
	  }
	  mesh = t->CreateMeshWrapper ();
	}
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    TokenError ("a mesh object");
    return NULL;
  }

  if (!mesh)
  {
    ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"There is no FACTORY for this mesh!");
    return NULL;
  }
  if (!priority[0]) strcpy (priority, "object");
  mesh->SetRenderPriority (Engine->GetRenderPriority (priority));

  return mesh;
}

bool csLoader::LoadMeshObject (iMeshWrapper* mesh, char* buf)
{
  if (!Engine) return false;

  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (ADDON)
    CS_TOKEN_TABLE (KEY)
    CS_TOKEN_TABLE (MESHOBJ)
    CS_TOKEN_TABLE (MESHREF)
    CS_TOKEN_TABLE (MOVE)
    CS_TOKEN_TABLE (HARDMOVE)
    CS_TOKEN_TABLE (PLUGIN)
    CS_TOKEN_TABLE (PARAMS)
    CS_TOKEN_TABLE (NOLIGHTING)
    CS_TOKEN_TABLE (NOSHADOWS)
    CS_TOKEN_TABLE (INVISIBLE)
    CS_TOKEN_TABLE (DETAIL)
    CS_TOKEN_TABLE (ZFILL)
    CS_TOKEN_TABLE (ZNONE)
    CS_TOKEN_TABLE (ZUSE)
    CS_TOKEN_TABLE (ZTEST)
    CS_TOKEN_TABLE (CAMERA)
    CS_TOKEN_TABLE (CONVEX)
    CS_TOKEN_TABLE (PRIORITY)
    CS_TOKEN_TABLE (LOD)
    CS_TOKEN_TABLE (LMCACHE)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_matvec)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];
  str[0] = 0;
  char priority[255]; priority[0] = 0;

  Stats->meshes_loaded++;
  iLoaderPlugin* plug = NULL;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (
	  "crystalspace.maploader.parse.badformat",
	  "Expected parameters instead of '%s' while parsing mesh object!",
	  buf);
      return false;
    }
    switch (cmd)
    {
      case CS_TOKEN_LOD:
        {
	  if (!mesh->GetMeshObject ())
	  {
            ReportError (
	      "crystalspace.maploader.parse.meshobject",
	      "Only use LOD after PARAMS!");
	    return false;
	  }
	  iLODControl* lodctrl = SCF_QUERY_INTERFACE (
	    	mesh->GetMeshObject (),
		iLODControl);
	  if (!lodctrl)
	  {
            ReportError (
	      "crystalspace.maploader.parse.meshobject",
              "This mesh doesn't implement LOD control!");
	    return false;
	  }
	  if (!LoadLodControl (lodctrl, params))
	  {
	    lodctrl->DecRef ();
	    return false;
	  }
	  lodctrl->DecRef ();
	}
        break;
      case CS_TOKEN_PRIORITY:
	csScanStr (params, "%s", priority);
	break;
      case CS_TOKEN_ADDON:
	if (!LoadAddOn (params, mesh))
	  return false;
      	break;
      case CS_TOKEN_NOLIGHTING:
        mesh->GetFlags().Set (CS_ENTITY_NOLIGHTING);
        break;
      case CS_TOKEN_NOSHADOWS:
        mesh->GetFlags().Set (CS_ENTITY_NOSHADOWS);
        break;
      case CS_TOKEN_INVISIBLE:
        mesh->GetFlags().Set (CS_ENTITY_INVISIBLE);
        break;
      case CS_TOKEN_DETAIL:
        mesh->GetFlags().Set (CS_ENTITY_DETAIL);
        break;
      case CS_TOKEN_ZFILL:
        if (!priority[0]) strcpy (priority, "wall");
        mesh->SetZBufMode (CS_ZBUF_FILL);
        break;
      case CS_TOKEN_ZUSE:
        if (!priority[0]) strcpy (priority, "object");
        mesh->SetZBufMode (CS_ZBUF_USE);
        break;
      case CS_TOKEN_ZNONE:
        if (!priority[0]) strcpy (priority, "sky");
        mesh->SetZBufMode (CS_ZBUF_NONE);
        break;
      case CS_TOKEN_ZTEST:
        if (!priority[0]) strcpy (priority, "alpha");
        mesh->SetZBufMode (CS_ZBUF_TEST);
        break;
      case CS_TOKEN_CAMERA:
        if (!priority[0]) strcpy (priority, "sky");
        mesh->GetFlags().Set (CS_ENTITY_CAMERA);
        break;
      case CS_TOKEN_CONVEX:
        mesh->GetFlags().Set (CS_ENTITY_CONVEX);
        break;
      case CS_TOKEN_KEY:
        {
          iKeyValuePair* kvp = ParseKey (params, mesh->QueryObject());
          if (kvp)
	    kvp->DecRef ();
	  else
	    return false;
	}
        break;
      case CS_TOKEN_MESHREF:
        {
          iMeshWrapper* sp = LoadMeshObjectFromFactory (params);
          if (!sp)
	  {
	    ReportError (
	      	"crystalspace.maploader.load.meshobject",
		"Could not load mesh object '%s'!",
		name);
	    return false;
	  }
	  sp->QueryObject ()->SetName (name);
	  sp->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
          mesh->GetChildren ()->Add (sp);
	  sp->DecRef ();
        }
        break;
      case CS_TOKEN_MESHOBJ:
        {
	  iMeshWrapper* sp = Engine->CreateMeshWrapper (name);
          if (!LoadMeshObject (sp, params))
	  {
	    ReportError (
	      	"crystalspace.maploader.load.meshobject",
		"Could not load mesh object '%s'!",
		name);
	    return false;
	  }
	  sp->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
          mesh->GetChildren ()->Add (sp);
	  sp->DecRef ();
        }
        break;
      case CS_TOKEN_HARDMOVE:
        {
          char* params2;
	  csReversibleTransform tr;
          while ((cmd = csGetObject (&params, tok_matvec, &name, &params2)) > 0)
          {
            if (!params2)
            {
	      ReportError (
		"crystalspace.maploader.parse.badformat",
		"Expected parameters instead of '%s' while parsing hardmove!",
		params);
	      return false;
            }
            switch (cmd)
            {
              case CS_TOKEN_MATRIX:
              {
		csMatrix3 m;
                if (!ParseMatrix (params2, m))
		  return false;
                tr.SetT2O (m);
                break;
              }
              case CS_TOKEN_V:
              {
		csVector3 v;
                ParseVector (params2, v);
		tr.SetOrigin (v);
                break;
              }
            }
          }
	  mesh->HardTransform (tr);
        }
        break;
      case CS_TOKEN_MOVE:
        {
          char* params2;
          mesh->GetMovable ()->SetTransform (csMatrix3 ());     // Identity
          mesh->GetMovable ()->SetPosition (csVector3 (0));
          while ((cmd = csGetObject (&params, tok_matvec, &name, &params2)) > 0)
          {
            if (!params2)
            {
	      ReportError (
		"crystalspace.maploader.parse.badformat",
		"Expected parameters instead of '%s' while parsing move!",
		params);
	      return false;
            }
            switch (cmd)
            {
              case CS_TOKEN_MATRIX:
              {
                csMatrix3 m;
                if (!ParseMatrix (params2, m))
		  return false;
                mesh->GetMovable ()->SetTransform (m);
                break;
              }
              case CS_TOKEN_V:
              {
                csVector3 v;
                ParseVector (params2, v);
                mesh->GetMovable ()->SetPosition (v);
                break;
              }
            }
          }
	  mesh->GetMovable ()->UpdateMove ();
        }
        break;

      case CS_TOKEN_PARAMS:
	if (!plug)
	{
          ReportError (
	      "crystalspace.maploader.load.plugin",
              "Could not load plugin!");
	  return false;
	}
	else
	{
	  iBase* mo = plug->Parse (params, Engine->GetMaterialList (),
	  	Engine->GetMeshFactories (), NULL);
          if (mo)
          {
	    iMeshObject* mo2 = SCF_QUERY_INTERFACE (mo, iMeshObject);
	    if (!mo2)
	    {
              ReportError (
	        "crystalspace.maploader.parse.mesh",
		"Returned object does not implement iMeshObject!");
	      return false;
	    }
	    mesh->SetMeshObject (mo2);
	    mo2->SetLogicalParent (mesh);
	    if (mo2->GetFactory () && mo2->GetFactory ()->GetLogicalParent ())
	    {
	      iBase* lp = mo2->GetFactory ()->GetLogicalParent ();
	      iMeshFactoryWrapper* mfw = SCF_QUERY_INTERFACE (lp,
	      	iMeshFactoryWrapper);
	      if (mfw)
	      {
	        mesh->SetFactory (mfw);
		mfw->DecRef ();
	      }
	    }
	    mo2->DecRef ();
            mo->DecRef ();
          }
          else
          {
            ReportError (
	      "crystalspace.maploader.parse.plugin",
              "Error parsing PARAM() in plugin '%s'!", str);
	    return false;
          }
	}
        break;

      case CS_TOKEN_PLUGIN:
	{
	  csScanStr (params, "%s", str);
	  plug = loaded_plugins.FindPlugin (str);
	}
        break;

      case CS_TOKEN_LMCACHE:
        {
	  if (!mesh->GetMeshObject ())
	  {
            ReportError (
	      "crystalspace.maploader.parse.meshobject",
	      "Only use LMCACHE after PARAMS!");
	    return false;
	  }
	  iLightingInfo* li = SCF_QUERY_INTERFACE (mesh->GetMeshObject (),
	  	iLightingInfo);
	  if (li)
	  {
	    csScanStr (params, "%s", str);
	    li->SetCacheName (str);
	    li->DecRef ();
	  }
	}
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    TokenError ("a mesh object");
    return false;
  }

  if (!priority[0]) strcpy (priority, "object");
  mesh->SetRenderPriority (Engine->GetRenderPriority (priority));

  return true;
}

//---------------------------------------------------------------------------

bool csLoader::LoadAddOn (char* buf, iBase* context)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (PLUGIN)
    CS_TOKEN_TABLE (PARAMS)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];
  str[0] = 0;

  iLoaderPlugin* plug = NULL;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (
	  "crystalspace.maploader.parse.badformat",
	  "Expected parameters instead of '%s' while parsing add-on!",
	  buf);
      return false;
    }
    switch (cmd)
    {
      case CS_TOKEN_PARAMS:
	if (!plug)
	{
          ReportError (
	      "crystalspace.maploader.load.plugin",
              "Could not load plugin!");
	  return false;
	}
	else
	{
	  plug->Parse (params, Engine->GetMaterialList (),
	  	Engine->GetMeshFactories (), context);
	}
        break;

      case CS_TOKEN_PLUGIN:
	{
	  csScanStr (params, "%s", str);
	  plug = loaded_plugins.FindPlugin (str);
	}
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    TokenError ("an add-on");
    return false;
  }

  return true;
}

//---------------------------------------------------------------------------

bool csLoader::LoadRenderPriorities (char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (PRIORITY)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (
	  "crystalspace.maploader.parse.badformat",
	  "Expected parameters instead of '%s' while parsing priority!",
	  buf);
      return false;
    }
    switch (cmd)
    {
      case CS_TOKEN_PRIORITY:
      {
        long pri;
	char sorting[100];
	csScanStr (params, "%d,%s", &pri, sorting);
	int rendsort = CS_RENDPRI_NONE;
	if (!strcmp (sorting, "BACK2FRONT"))
	{
	  rendsort = CS_RENDPRI_BACK2FRONT;
	}
	else if (!strcmp (sorting, "FRONT2BACK"))
	{
	  rendsort = CS_RENDPRI_FRONT2BACK;
	}
	else if (!strcmp (sorting, "NONE"))
	{
	  rendsort = CS_RENDPRI_NONE;
	}
	else
	{
          ReportError (
	    "crystalspace.maploader.parse.priorities",
	    "Unknown sorting attribute '%s' for the render priority!",
	    sorting);
	  return false;
	}
	Engine->RegisterRenderPriority (name, pri, rendsort);
        break;
      }
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    TokenError ("the render priorities");
    return false;
  }

  return true;
}

//---------------------------------------------------------------------------

iMeshWrapper* csLoader::LoadMeshObject (const char* fname)
{
  if (!Engine) return NULL;

  iDataBuffer *databuff = VFS->ReadFile (fname);
  iMeshWrapper* mesh = NULL;

  if (!databuff || !databuff->GetSize ())
  {
    if (databuff) databuff->DecRef ();
    ReportError (
	      "crystalspace.maploader.parse.meshobject",
    	      "Could not open mesh object file '%s' on VFS!", fname);
    return NULL;
  }

  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (MESHOBJ)
  CS_TOKEN_TABLE_END

  char *name, *data;
  char *buf = **databuff;

  if (csGetObject (&buf, tokens, &name, &data))
  {
    if (!data)
    {
      ReportError (
	  "crystalspace.maploader.parse.badformat",
	  "Expected parameters instead of '%s' while parsing mesh object!",
	  buf);
    }
    else
    {
      mesh = Engine->CreateMeshWrapper (name);
      if (!LoadMeshObject (mesh, buf))
      {
	mesh->DecRef ();
	ReportError (
		     "crystalspace.maploader.load.meshobject",
		     "Could not load mesh object '%s' from file '%s'!",
		     name, fname);
      }
    }
  }
  databuff->DecRef ();
  return mesh;
}

/************ iLoader implementation **************/

//--- Plugin stuff -----------------------------------------------------------

SCF_IMPLEMENT_IBASE(csLoader);
  SCF_IMPLEMENTS_INTERFACE(iLoader);
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent);
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (csLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY(csLoader);

SCF_EXPORT_CLASS_TABLE (csparser)
  SCF_EXPORT_CLASS_DEP (csLoader, "crystalspace.level.loader",
    "Level and library file loader", "crystalspace.kernel., "
    "crystalspace.sound.loader., crystalspace.image.loader, "
    "crystalspace.mesh.loader., "
    "crystalspace.engine.3d, crystalspace.graphics3d., "
    "crystalspace.sound.render., crystalspace.motion.manager., "
    "crystalspace.mesh.crossbuilder, crystalspace.modelconverter.")
SCF_EXPORT_CLASS_TABLE_END

CS_IMPLEMENT_PLUGIN

csLoader::csLoader(iBase *p)
{
  SCF_CONSTRUCT_IBASE(p);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);

  plugin_mgr = NULL;
  object_reg = NULL;
  VFS = NULL;
  ImageLoader = NULL;
  SoundLoader = NULL;
  Engine = NULL;
  G3D = NULL;
  SoundRender = NULL;
  ModelConverter = NULL;
  CrossBuilder = NULL;

  flags = 0;
  ResolveOnlyRegion = false;
  Stats = new csLoaderStats();
}

csLoader::~csLoader()
{
  loaded_plugins.DeleteAll ();
  SCF_DEC_REF(plugin_mgr);
  SCF_DEC_REF(Reporter);
  SCF_DEC_REF(VFS);
  SCF_DEC_REF(ImageLoader);
  SCF_DEC_REF(SoundLoader);
  SCF_DEC_REF(Engine);
  SCF_DEC_REF(G3D);
  SCF_DEC_REF(SoundRender);
  SCF_DEC_REF(ModelConverter);
  SCF_DEC_REF(CrossBuilder);
  delete Stats;
}

#define GET_PLUGIN(var, intf, msgname)	\
  var = CS_QUERY_REGISTRY(object_reg, intf);

bool csLoader::Initialize(iObjectRegistry *object_Reg)
{
  csLoader::object_reg = object_Reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);

  GET_PLUGIN (Reporter, iReporter, "reporter");

  loaded_plugins.plugin_mgr = plugin_mgr;

  // get the virtual file system plugin
  VFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (!VFS)
  {
    ReportError (
	"crystalspace.maploader.novfs",
	"Failed to initialize the loader: No VFS plugin.");
    return false;
  }

  // get all optional plugins
  GET_PLUGIN (ImageLoader, iImageIO, "image-loader");
  GET_PLUGIN (SoundLoader, iSoundLoader, "sound-loader");
  GET_PLUGIN (Engine, iEngine, "engine");
  GET_PLUGIN (G3D, iGraphics3D, "video-driver");
  GET_PLUGIN (SoundRender, iSoundRender, "sound-driver");
  GET_PLUGIN (ModelConverter, iModelConverter, "model-converter");
  GET_PLUGIN (CrossBuilder, iCrossBuilder, "model-crossbuilder");

  return true;
}

void csLoader::SetMode (int iFlags)
{
  flags = iFlags;
}

void csLoader::TokenError (const char *Object)
{
  ReportError (
    "crystalspace.maploader.parse.badtoken",
    "Token '%s' not found while parsing a %s!",
    csGetLastOffender (), Object);
}

//--- Parsing of Engine Objects ---------------------------------------------

iCollection* csLoader::ParseCollection (char* name, char* buf)
{
  CS_TOKEN_TABLE_START(commands)
    CS_TOKEN_TABLE (ADDON)
    CS_TOKEN_TABLE (MESHOBJ)
    CS_TOKEN_TABLE (COLLECTION)
    CS_TOKEN_TABLE (LIGHT)
    CS_TOKEN_TABLE (SECTOR)
  CS_TOKEN_TABLE_END

  char* xname;
  long cmd;
  char* params;

  iCollection* collection = Engine->GetCollections ()->NewCollection (name);

  char str[255];
  while ((cmd = csGetObject (&buf, commands, &xname, &params)) > 0)
  {
    if (!params)
    {
      ReportError (
	  "crystalspace.maploader.parse.badformat",
	  "Expected parameters instead of '%s' while parsing collection!",
	  buf);
      return NULL;
    }
    str[0] = 0;
    switch (cmd)
    {
      case CS_TOKEN_ADDON:
	ReportError (
		"crystalspace.maploader.parse.collection",
         	"ADDON not yet supported in collection!");
	return NULL;
      	break;
      case CS_TOKEN_MESHOBJ:
        {
# if 0
//@@@@@@
          csScanStr (params, "%s", str);
	  iMeshWrapper* spr = Engine->FindMeshObject (str, ResolveOnlyRegion);
          if (!spr)
            ReportError ("crystalspace.maploader.parse.meshobject",
	    	"Mesh object '%s' not found!", str);
	  else
            collection->AddObject (spr->QueryObject());
# endif
        }
        break;
      case CS_TOKEN_LIGHT:
        {
          csScanStr (params, "%s", str);
	  iLight* l = NULL;
	  iSectorList* sl = Engine->GetSectors ();
	  int i;
	  for (i = 0 ; i < sl->GetCount () ; i++)
	  {
	    iSector* sect = sl->Get (i);
	    if ((!ResolveOnlyRegion) || (!Engine->GetCurrentRegion ()) ||
	      Engine->GetCurrentRegion ()->IsInRegion (sect->QueryObject ()))
	    {
	      l = sect->GetLights ()->FindByName (str);
	      if (l) break;
	    }
	  }
          if (!l)
	  {
	    ReportError (
		"crystalspace.maploader.parse.collection",
            	"Light '%s' not found!", str);
	    return NULL;
	  }
	  else
	    collection->AddObject (l->QueryObject ());
        }
        break;
      case CS_TOKEN_SECTOR:
        {
          csScanStr (params, "%s", str);
	  iSector* s = FindSector (str);
          if (!s)
	  {
	    ReportError (
		"crystalspace.maploader.parse.collection",
            	"Sector '%s' not found!", str);
	    return NULL;
	  }
	  else
            collection->AddObject (s->QueryObject ());
        }
        break;
      case CS_TOKEN_COLLECTION:
        {
          csScanStr (params, "%s", str);
	  //@@@$$$ TODO: Collection in regions.
	  iCollection* th;
	  if (ResolveOnlyRegion && Engine->GetCurrentRegion ())
	    th = Engine->GetCurrentRegion ()->FindCollection (str);
	  else
            th = Engine->GetCollections ()->FindByName (str);
          if (!th)
	  {
	    ReportError (
		"crystalspace.maploader.parse.collection",
            	"Collection '%s' not found!", str);
	    return NULL;
	  }
	  else
            collection->AddObject (th->QueryObject());
        }
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
    TokenError ("a collection");

  return collection;
}

bool csLoader::ParseStart (char* buf, iCameraPosition* campos)
{
  CS_TOKEN_TABLE_START(commands)
    CS_TOKEN_TABLE (SECTOR)
    CS_TOKEN_TABLE (POSITION)
    CS_TOKEN_TABLE (UP)
    CS_TOKEN_TABLE (FORWARD)
  CS_TOKEN_TABLE_END

  long cmd;
  char* params;

  char start_sector [100];
  strcpy (start_sector, "room");
  csVector3 pos (0, 0, 0);
  csVector3 up (0, 1, 0);
  csVector3 forward (0, 0, 1);
  if (strchr (buf, '('))
  {
    // New syntax.
    while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
    {
      switch (cmd)
      {
        case CS_TOKEN_SECTOR:
          csScanStr (params, "%s", start_sector);
          break;
        case CS_TOKEN_POSITION:
	  csScanStr (params, "%f,%f,%f", &pos.x, &pos.y, &pos.z);
	  break;
        case CS_TOKEN_UP:
	  csScanStr (params, "%f,%f,%f", &up.x, &up.y, &up.z);
	  break;
        case CS_TOKEN_FORWARD:
	  csScanStr (params, "%f,%f,%f", &forward.x, &forward.y, &forward.z);
	  break;
      }
    }
    if (cmd == CS_PARSERR_TOKENNOTFOUND)
    {
      TokenError ("a camera position");
      return false;
    }
  }
  else
  {
    csScanStr (buf, "%s,%f,%f,%f", &start_sector,
      &pos.x, &pos.y, &pos.z);
  }

  campos->Set (start_sector, pos, forward, up);
  return true;
}

iStatLight* csLoader::ParseStatlight (char* name, char* buf)
{
  CS_TOKEN_TABLE_START(commands)
    CS_TOKEN_TABLE (ATTENUATION)
    CS_TOKEN_TABLE (CENTER)
    CS_TOKEN_TABLE (RADIUS)
    CS_TOKEN_TABLE (DYNAMIC)
    CS_TOKEN_TABLE (COLOR)
    CS_TOKEN_TABLE (HALO)
    CS_TOKEN_TABLE (KEY)
  CS_TOKEN_TABLE_END

  long cmd;
  char* params;

  Stats->lights_loaded++;
  float x, y, z, dist = 0, r, g, b;
  int cnt;
  bool dyn;
  int attenuation = CS_ATTN_LINEAR;
  char str [100];
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

  if (strchr (buf, ':'))
  {
    // Still support old format for backwards compatibility.
    int d;
    csScanStr (buf, "%f,%f,%f:%f,%f,%f,%f,%d",
          &x, &y, &z, &dist, &r, &g, &b, &d);
    dyn = bool (d);
  }
  else
  {
    // New format.
    x = y = z = 0;
    dist = 1;
    r = g = b = 1;
    dyn = false;
    while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
    {
      switch (cmd)
      {
        case CS_TOKEN_RADIUS:
          csScanStr (params, "%f", &dist);
          break;
        case CS_TOKEN_CENTER:
          csScanStr (params, "%f,%f,%f", &x, &y, &z);
          break;
        case CS_TOKEN_COLOR:
          csScanStr (params, "%f,%f,%f", &r, &g, &b);
          break;
        case CS_TOKEN_DYNAMIC:
          dyn = true;
          break;
        case CS_TOKEN_KEY:
	  {
            iKeyValuePair* kvp = ParseKey (params, &Keys);
            if (kvp)
	      kvp->DecRef ();
	    else
	      return NULL;
	  }
          break;
        case CS_TOKEN_HALO:
	  str[0] = 0;
          cnt = csScanStr (params, "%s", str);
          if (cnt == 0 || !strcmp (str, "CROSS"))
          {
            params = strchr (params, ',');
            if (params) params++;
defaulthalo:
            halo.type = 1;
            halo.cross.Intensity = 2.0; halo.cross.Cross = 0.45;
            if (params)
              csScanStr (params, "%f,%f", &halo.cross.Intensity,
	      	&halo.cross.Cross);
          }
          else if (!strcmp (str, "NOVA"))
          {
            params = strchr (params, ',');
            if (params) params++;
            halo.type = 2;
            halo.nova.Seed = 0; halo.nova.NumSpokes = 100;
	    halo.nova.Roundness = 0.5;
            if (params)
              csScanStr (params, "%d,%d,%f", &halo.nova.Seed,
	      	&halo.nova.NumSpokes, &halo.nova.Roundness);
          }
          else if (!strcmp (str, "FLARE"))
          {
            params = strchr (params, ',');
            if (params) params++;
            halo.type = 3;
	    char mat_names[8][255];
	    int cur_idx = 0;
	    while (params && cur_idx < 6)
	    {
	      char* end = strchr (params, ',');
	      int l;
	      if (end) l = end-params;
	      else l = strlen (params);
	      strncpy (mat_names[cur_idx], params, l);
	      mat_names[cur_idx][l] = 0;
	      cur_idx++;
	      params = end+1;
	    }
	    halo.flare.mat_center = FindMaterial (mat_names[0]);
	    halo.flare.mat_spark1 = FindMaterial (mat_names[1]);
	    halo.flare.mat_spark2 = FindMaterial (mat_names[2]);
	    halo.flare.mat_spark3 = FindMaterial (mat_names[3]);
	    halo.flare.mat_spark4 = FindMaterial (mat_names[4]);
	    halo.flare.mat_spark5 = FindMaterial (mat_names[5]);
          }
          else
            goto defaulthalo;
          break;
        case CS_TOKEN_ATTENUATION:
          csScanStr (params, "%s", str);
          if (strcmp (str, "none")      == 0) attenuation = CS_ATTN_NONE;
          if (strcmp (str, "linear")    == 0) attenuation = CS_ATTN_LINEAR;
          if (strcmp (str, "inverse")   == 0) attenuation = CS_ATTN_INVERSE;
          if (strcmp (str, "realistic") == 0) attenuation = CS_ATTN_REALISTIC;
      }
    }
    if (cmd == CS_PARSERR_TOKENNOTFOUND)
    {
      TokenError ("a light");
      return NULL;
    }
  }

  // implicit radius
  if (dist == 0)
  {
    if (r > g && r > b) dist = r;
    else if (g > b) dist = g;
    else dist = b;
    switch (attenuation)
    {
      case CS_ATTN_NONE      : dist = 100000000; break;
      case CS_ATTN_LINEAR    : break;
      case CS_ATTN_INVERSE   : dist = 16 * sqrt (dist); break;
      case CS_ATTN_REALISTIC : dist = 256 * dist; break;
    }
  }

  iStatLight* l = Engine->CreateLight (name, csVector3(x, y, z),
  	dist, csColor(r, g, b), dyn);
  switch (halo.type)
  {
    case 1:
      l->QueryLight ()->CreateCrossHalo (halo.cross.Intensity,
      	halo.cross.Cross);
      break;
    case 2:
      l->QueryLight ()->CreateNovaHalo (halo.nova.Seed, halo.nova.NumSpokes,
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
        iFlareHalo* flare = l->QueryLight ()->CreateFlareHalo ();
	flare->AddComponent (0.0, 1.2, 1.2, CS_FX_ADD, ifmc);
	flare->AddComponent (0.3, 0.1, 0.1, CS_FX_ADD, ifm3);
	flare->AddComponent (0.6, 0.4, 0.4, CS_FX_ADD, ifm4);
	flare->AddComponent (0.8, .05, .05, CS_FX_ADD, ifm5);
	flare->AddComponent (1.0, 0.7, 0.7, CS_FX_ADD, ifm1);
	flare->AddComponent (1.3, 0.1, 0.1, CS_FX_ADD, ifm3);
	flare->AddComponent (1.5, 0.3, 0.3, CS_FX_ADD, ifm4);
	flare->AddComponent (1.8, 0.1, 0.1, CS_FX_ADD, ifm5);
	flare->AddComponent (2.0, 0.5, 0.5, CS_FX_ADD, ifm2);
	flare->AddComponent (2.1, .15, .15, CS_FX_ADD, ifm3);
	flare->AddComponent (2.5, 0.2, 0.2, CS_FX_ADD, ifm3);
	flare->AddComponent (2.8, 0.4, 0.4, CS_FX_ADD, ifm4);
	flare->AddComponent (3.0, 3.0, 3.0, CS_FX_ADD, ifm1);
	flare->AddComponent (3.1, 0.05, 0.05, CS_FX_ADD, ifm5);
	flare->AddComponent (3.3, .15, .15, CS_FX_ADD, ifm2);
      }
      break;
  }
  l->QueryLight ()->SetAttenuation (attenuation);

  // Move the key-value pairs from 'Keys' to the light object
  l->QueryObject ()->ObjAddChildren (&Keys);
  Keys.ObjRemoveAll ();

  return l;
}

iKeyValuePair* csLoader::ParseKey (char* buf, iObject* pParent)
{
  char Key  [256];
  char Value[10000]; //Value can potentially grow _very_ large.
  if (csScanStr(buf, "%S,%S", Key, Value) == 2)
  {
    csKeyValuePair* cskvp = new csKeyValuePair (Key, Value);
    iKeyValuePair* kvp = SCF_QUERY_INTERFACE (cskvp, iKeyValuePair);
    if (pParent)
      pParent->ObjAdd (kvp->QueryObject ());
    kvp->DecRef ();
    return kvp;
  }
  else
  {
    ReportError (
		"crystalspace.maploader.parse.key",
    	        "Illegal Syntax for KEY() command in line %d!",
		csGetParserLine());
    return NULL;
  }
}

iMapNode* csLoader::ParseNode (char* name, char* buf, iSector* sec)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (ADDON)
    CS_TOKEN_TABLE (KEY)
    CS_TOKEN_TABLE (POSITION)
  CS_TOKEN_TABLE_END

  iMapNode* pNode = &(new csMapNode (name))->scfiMapNode;
  pNode->SetSector (sec);

  long  cmd;
  char* xname;
  char* params;

  float x = 0;
  float y = 0;
  float z = 0;

  while ((cmd = csGetObject (&buf, commands, &xname, &params)) > 0)
  {
    if (!params)
    {
      ReportError (
	  "crystalspace.maploader.parse.badformat",
	  "Expected parameters instead of '%s' while parsing node!",
	  buf);
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_ADDON:
	ReportError (
		"crystalspace.maploader.parse.node",
        	"ADDON not yet supported in node!");
	return NULL;
      case CS_TOKEN_KEY:
        {
          iKeyValuePair* kvp = ParseKey (params, pNode->QueryObject ());
          if (kvp)
	    kvp->DecRef ();
	  else
	    return NULL;
	}
        break;
      case CS_TOKEN_POSITION:
        csScanStr (params, "%f,%f,%f", &x, &y, &z);
        break;
      default:
        abort ();
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
    TokenError ("a node");

  pNode->SetPosition(csVector3(x,y,z));

  return pNode;
}

iSector* csLoader::ParseSector (char* secname, char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (ADDON)
    CS_TOKEN_TABLE (CULLER)
    CS_TOKEN_TABLE (FOG)
    CS_TOKEN_TABLE (LIGHT)
    CS_TOKEN_TABLE (MESHOBJ)
    CS_TOKEN_TABLE (MESHREF)
    CS_TOKEN_TABLE (NODE)
    CS_TOKEN_TABLE (KEY)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  bool do_culler = false;
  char bspname[100];

  iSector *sector = Engine->CreateSector (secname);
  Stats->sectors_loaded++;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (
	  "crystalspace.maploader.parse.badformat",
	  "Expected parameters instead of '%s' while parsing sector!",
	  buf);
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_ADDON:
	if (!LoadAddOn (params, sector))
	{
	  return NULL;
	}
      	break;
      case CS_TOKEN_CULLER:
	if (!csScanStr (params, "%s", bspname))
	{
	  ReportError (
		"crystalspace.maploader.parse.sector",
	  	"CULLER expects the name of a mesh object!");
	  return NULL;
	}
	else
          do_culler = true;
        break;
      case CS_TOKEN_MESHREF:
        {
          iMeshWrapper* mesh = LoadMeshObjectFromFactory (params);
          if (!mesh)
	  {
      	    ReportError (
	      	"crystalspace.maploader.load.meshobject",
		"Could not load mesh object '%s' in sector '%s'!",
		name, secname ? secname : "<noname>");
	    return NULL; // @@@ Leak
	  }
	  mesh->QueryObject ()->SetName (name);
	  mesh->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
          mesh->GetMovable ()->SetSector (sector);
	  mesh->GetMovable ()->UpdateMove ();
	  Engine->GetMeshes ()->Add (mesh);
	  mesh->DecRef ();
        }
        break;
      case CS_TOKEN_MESHOBJ:
        {
	  iMeshWrapper* mesh = Engine->CreateMeshWrapper (name);
          if (!LoadMeshObject (mesh, params))
	  {
      	    ReportError (
	      	"crystalspace.maploader.load.meshobject",
		"Could not load mesh object '%s' in sector '%s'!",
		name, secname ? secname : "<noname>");
	    mesh->DecRef ();
	    return NULL; // @@@ Leak
	  }
	  mesh->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
          mesh->GetMovable ()->SetSector (sector);
	  mesh->GetMovable ()->UpdateMove ();
	  mesh->DecRef ();
        }
        break;
      case CS_TOKEN_LIGHT:
        {
	  iStatLight* sl = ParseStatlight (name, params);
	  if (!sl)
	  {
	    return NULL; // @@@ Leak
	  }
          sector->GetLights ()->Add (sl->QueryLight ());
	  sl->DecRef ();
	}
        break;
      case CS_TOKEN_NODE:
        {
          iMapNode *n = ParseNode (name, params, sector);
	  if (n)
	  {
	    n->DecRef ();
	  }
	  else
	  {
	    return NULL; // @@@ Leak
	  }
	}
        break;
      case CS_TOKEN_FOG:
        {
          csFog *f = sector->GetFog ();
          f->enabled = true;
          csScanStr (params, "%f,%f,%f,%f", 
		     &f->red, &f->green, &f->blue, &f->density);
        }
        break;
      case CS_TOKEN_KEY:
      {
        iKeyValuePair* kvp = ParseKey (params, sector->QueryObject());
	if (kvp)
	  kvp->DecRef ();
	else
	  return NULL;
        break;
      }
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
    TokenError ("a sector");

  if (!(flags & CS_LOADER_NOBSP))
    if (do_culler) sector->SetVisibilityCuller (bspname);
  return sector;
}
