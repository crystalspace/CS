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
#include "qsqrt.h"
#include "csutil/xmltiny.h"
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
#include "iutil/document.h"
#include "imap/reader.h"
#include "imap/ldrctxt.h"
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

/*
 * Context class for the standard loader.
 */
class StdLoaderContext : public iLoaderContext
{
private:
  iEngine* Engine;
  iRegion* region;
  csParser* parser;
public:
  StdLoaderContext (iEngine* Engine, bool ResolveOnlyRegion, csParser* parser);
  virtual ~StdLoaderContext ();

  SCF_DECLARE_IBASE;

  virtual iSector* FindSector (const char* name);
  virtual iMaterialWrapper* FindMaterial (const char* name);
  virtual iMeshFactoryWrapper* FindMeshFactory (const char* name);
  virtual iMeshWrapper* FindMeshObject (const char* name);
  virtual csParser* GetParser ();
};

SCF_IMPLEMENT_IBASE(StdLoaderContext);
  SCF_IMPLEMENTS_INTERFACE(iLoaderContext);
SCF_IMPLEMENT_IBASE_END;

StdLoaderContext::StdLoaderContext (iEngine* Engine,
	bool ResolveOnlyRegion, csParser* parser)
{
  SCF_CONSTRUCT_IBASE (NULL);
  StdLoaderContext::Engine = Engine;
  StdLoaderContext::parser = parser;
  if (ResolveOnlyRegion)
    region = Engine->GetCurrentRegion ();
  else
    region = NULL;
}

StdLoaderContext::~StdLoaderContext ()
{
}

iSector* StdLoaderContext::FindSector (const char* name)
{
  return Engine->FindSector (name, region);
}

iMaterialWrapper* StdLoaderContext::FindMaterial (const char* name)
{
  iMaterialWrapper* mat = Engine->FindMaterial (name, region);
  if (mat)
    return mat;

  iTextureWrapper* tex = Engine->FindTexture (name, region);
  if (tex)
  {
    // Add a default material with the same name as the texture
    iMaterial* material = Engine->CreateBaseMaterial (tex);
    iMaterialWrapper *mat = Engine->GetMaterialList ()
      	->NewMaterial (material);
    // First we have to extract the optional region name from the name:
    char* n = strchr (name, '/');
    if (!n) n = (char*)name;
    else n++;
    mat->QueryObject()->SetName (n);
    material->DecRef ();
    return mat;
  }

  return NULL;
}

iMeshFactoryWrapper* StdLoaderContext::FindMeshFactory (const char* name)
{
  return Engine->FindMeshFactory (name, region);
}

iMeshWrapper* StdLoaderContext::FindMeshObject (const char* name)
{
  return Engine->FindMeshObject (name, region);
}
csParser* StdLoaderContext::GetParser ()
{
  return parser;
}

//---------------------------------------------------------------------------

void csLoader::csLoaderStats::Init ()
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
  Init ();
}

// Define all tokens used through this file
CS_TOKEN_DEF_START
  CS_TOKEN_DEF (AMBIENT)
  CS_TOKEN_DEF (ADDON)
  CS_TOKEN_DEF (ATTENUATION)
  CS_TOKEN_DEF (CAMERA)
  CS_TOKEN_DEF (CENTER)
  CS_TOKEN_DEF (CLEARZBUF)
  CS_TOKEN_DEF (CLEARSCREEN)
  CS_TOKEN_DEF (COLLECTION)
  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (CONVEX)
  CS_TOKEN_DEF (CULLER)
  CS_TOKEN_DEF (CULLERP)
  CS_TOKEN_DEF (DETAIL)
  CS_TOKEN_DEF (DYNAMIC)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (FARPLANE)
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
  CS_TOKEN_DEF (LIGHTMAPCELLSIZE)
  CS_TOKEN_DEF (LMCACHE)
  CS_TOKEN_DEF (LOD)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MATERIALS)
  CS_TOKEN_DEF (MATRIX)
  CS_TOKEN_DEF (MAXLIGHTMAPSIZE)
  CS_TOKEN_DEF (MESHFACT)
  CS_TOKEN_DEF (MESHLIB)
  CS_TOKEN_DEF (MESHOBJ)
  CS_TOKEN_DEF (MESHREF)
  CS_TOKEN_DEF (MOVE)
  CS_TOKEN_DEF (NODE)
  CS_TOKEN_DEF (NOLIGHTING)
  CS_TOKEN_DEF (NOSHADOWS)
  CS_TOKEN_DEF (PARAMS)
  CS_TOKEN_DEF (PARAMSFILE)
  CS_TOKEN_DEF (PLUGIN)
  CS_TOKEN_DEF (PLUGINS)
  CS_TOKEN_DEF (POSITION)
  CS_TOKEN_DEF (PRIORITY)
  CS_TOKEN_DEF (RADIUS)
  CS_TOKEN_DEF (REGION)
  CS_TOKEN_DEF (RENDERPRIORITIES)
  CS_TOKEN_DEF (SECTOR)
  CS_TOKEN_DEF (SETTINGS)
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
    csPrintf ("crystalspace.maploader: ");
    csPrintfV (description, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

//---------------------------------------------------------------------------

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
    CS_TOKEN_TABLE (SETTINGS)
    CS_TOKEN_TABLE (PLUGINS)
  CS_TOKEN_TABLE_END

  parser.ResetParserLine ();
  char *name, *data;

  if (parser.GetObject (&buf, tokens, &name, &data))
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

    while ((cmd = parser.GetObject (&data, commands, &name, &params)) > 0)
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
        case CS_TOKEN_SETTINGS:
	  if (!LoadSettings (params))
	    return false;
	  break;
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
            iMeshFactoryWrapper* t = Engine->CreateMeshFactory (name);
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
          if (!ParseSector (name, params))
	    return false;
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

  return true;
}

bool csLoader::LoadMapFile (const char* file, bool iClearEngine,
  bool iOnlyRegion)
{
  Stats->Init ();
  if (iClearEngine) Engine->DeleteAll ();
  ResolveOnlyRegion = iOnlyRegion;
  SCF_DEC_REF (ldr_context); ldr_context = NULL;

  csRef<iDataBuffer> buf;
  buf.Take (VFS->ReadFile (file));

  if (!buf || !buf->GetSize ())
  {
    ReportError (
	      "crystalspace.maploader.parse.map",
    	      "Could not open map file '%s' on VFS!", file);
    return false;
  }

  Engine->ResetWorldSpecificSettings();

  // XML: temporary code to detect if we have an XML file. If that's
  // the case then we will use the XML parsers.
  // @@@
  char* b = **buf;
  while (*b == ' ' || *b == '\n' || *b == '\t') b++;
  if (*b == '<')
  {
    // XML.
    // First try to find out if there is an iDocumentSystem registered in the
    // object registry. If that's the case we will use that. Otherwise
    // we'll use tinyxml.
    csRef<iDocumentSystem> xml;
    xml.Take (CS_QUERY_REGISTRY (object_reg, iDocumentSystem));
    if (!xml)
    {
      xml.Take (new csTinyDocumentSystem ());
    }
    csRef<iDocument> doc = xml->CreateDocument ();
    const char* error = doc->Parse (buf);
    if (error == NULL)
    {
      if (!LoadMap (doc->GetRoot ()))
	return false;
    }
    else
    {
      ReportError (
	      "crystalspace.maploader.parse.map",
    	      "XML error '%s' for file '%s'!", error, file);
      return false;
    }
  }
  else
  {
    // Old format.
    if (!LoadMap (**buf))
    {
      return false;
    }
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

  while ((cmd = parser.GetObject (&buf, commands, &name, &params)) > 0)
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
  if (!Engine)
  {
    ReportError (
	  "crystalspace.maploader.parse.noengine",
	  "No engine present while in LoadLibrary!");
    return false;
  }
 
  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (LIBRARY)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (ADDON)
    CS_TOKEN_TABLE (TEXTURES)
    CS_TOKEN_TABLE (MATERIALS)
    CS_TOKEN_TABLE (MESHFACT)
    CS_TOKEN_TABLE (PLUGINS)
    CS_TOKEN_TABLE (MESHOBJ)
    CS_TOKEN_TABLE (MESHREF)
    CS_TOKEN_TABLE (SOUNDS)
  CS_TOKEN_TABLE_END

  char *name, *data;
  if (parser.GetObject (&buf, tokens, &name, &data))
  {
    // Empty LIBRARY () directive?
    if (!data)
      return false;

    long cmd;
    char* params;

    while ((cmd = parser.GetObject (&data, commands, &name, &params)) > 0)
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
        case CS_TOKEN_MESHREF:
          {
            iMeshWrapper* mesh = LoadMeshObjectFromFactory (params);
            if (!mesh)
	    {
      	      ReportError (
	      	  "crystalspace.maploader.load.meshobject",
		  "Could not load mesh object '%s' in library!",
		  name);
	      return false;
	    }
	    mesh->QueryObject ()->SetName (name);
	    Engine->GetMeshes ()->Add (mesh);
	    //mesh->DecRef ();
          }
          break;
        case CS_TOKEN_MESHOBJ:
          {
	    iMeshWrapper* mesh = Engine->CreateMeshWrapper (name);
            if (!LoadMeshObject (mesh, params))
	    {
      	      ReportError (
	      	  "crystalspace.maploader.load.meshobject",
		  "Could not load mesh object '%s' in library!",
		  name);
	      mesh->DecRef ();
	      return false;
	    }
	    //mesh->DecRef ();
          }
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
        case CS_TOKEN_PLUGINS:
	  if (!LoadPlugins (params))
	    return false;
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
  SCF_DEC_REF (ldr_context); ldr_context = NULL;
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

  while ((cmd = parser.GetObject (&buf, commands, &name, &params)) > 0)
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
        cmd = parser.GetCommand (&params, options, &maybename);
        if (cmd == CS_TOKEN_FILE)
          filename = maybename;
        else if (cmd == CS_PARSERR_TOKENNOTFOUND)
        {
          ReportError (
	    "crystalspace.maploader.parse.badtoken",
            "Unknown token '%s' found while parsing SOUND directive!",
	    parser.GetLastOffender());
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

  while ((cmd = parser.GetObject (&buf, commands, &name, &params)) > 0)
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

  ResolveOnlyRegion = false;
  SCF_DEC_REF (ldr_context); ldr_context = NULL;

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

  if (parser.GetObject (&buf, tokens, &name, &data))
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

    iMeshFactoryWrapper* t = Engine->CreateMeshFactory (name);
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
    CS_TOKEN_TABLE (PARAMSFILE)
    CS_TOKEN_TABLE (PLUGIN)
    CS_TOKEN_TABLE (MESHFACT)
    CS_TOKEN_TABLE (MOVE)
    CS_TOKEN_TABLE (HARDMOVE)
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

  while ((cmd = parser.GetObject (&buf, commands, &name, &params)) > 0)
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
	  iBase* mof = plug->Parse (params, GetLoaderContext (),
	  	stemp->GetMeshObjectFactory ());
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
      case CS_TOKEN_PARAMSFILE:
	if (!plug)
	{
          ReportError (
	      "crystalspace.maploader.load.plugin",
              "Could not load plugin!");
	  return false;
	}
	else
        {
          csScanStr (params, "%s", str);
          iDataBuffer *buf = VFS->ReadFile (str);
	  if (!buf)
	  {
            ReportError (
	      "crystalspace.maploader.parse.loadingfile",
	      "Error opening file '%s'!", str);
	    return false;
	  }

	  // We give here the iMeshObjectFactory as the context. If this
	  // is a new factory this will be NULL. Otherwise it is possible
	  // to append information to the already loaded factory.
	  iBase* mof = plug->Parse ((char*)(buf->GetUint8 ()),
	  	GetLoaderContext (), stemp->GetMeshObjectFactory ());
	  buf->DecRef ();
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
          mat = GetLoaderContext ()->FindMaterial (str);
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
          if (!Model)
	  {
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
	  stemp2->DecRef ();
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
          while ((cmd = parser.GetObject (&params, tok_matvec, &name, &params2)) > 0)
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
		if (!SyntaxService->ParseMatrix (&parser, params2, m))
		  return false;
		transf->SetO2T (m);
                break;
              }
              case CS_TOKEN_V:
              {
                csVector3 v;
                if (SyntaxService->ParseVector (&parser, params2, v))
		  transf->SetO2TTranslation (v);
                break;
              }
            }
          }
        }
        break;
      case CS_TOKEN_HARDMOVE:
        {
	  if (!stemp->GetMeshObjectFactory ())
	  {
            ReportError (
	      "crystalspace.maploader.parse.meshfactory",
              "Please use PARAMS before specifying HARDMOVE!");
	    return false;
	  }
	  if (!stemp->GetMeshObjectFactory ()->SupportsHardTransform ())
	  {
            ReportError (
	      "crystalspace.maploader.parse.meshfactory",
              "This factory doesn't support HARDMOVE!");
	    return false;
	  }
          char* params2;
	  csReversibleTransform tr;
          while ((cmd = parser.GetObject (&params, tok_matvec, &name, &params2)) > 0)
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
                if (!SyntaxService->ParseMatrix (&parser, params2, m))
		  return false;
                tr.SetT2O (m);
                break;
              }
              case CS_TOKEN_V:
              {
		csVector3 v;
                if (SyntaxService->ParseVector (&parser, params2, v))
		  tr.SetOrigin (v);
                break;
              }
            }
          }
	  stemp->HardTransform (tr);
        }
        break;
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

  while ((cmd = parser.GetObject (&buf, commands, &name, &params)) > 0)
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
	else if (!mesh->GetMeshObject ()->SupportsHardTransform ())
	{
          ReportError (
	    "crystalspace.maploader.parse.meshobject",
            "This mesh object doesn't support HARDMOVE!");
	  return NULL;
	}
	else
        {
          char* params2;
	  csReversibleTransform tr;
          while ((cmd = parser.GetObject (&params, tok_matvec, &name, &params2)) > 0)
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
                if (!SyntaxService->ParseMatrix (&parser, params2, m))
		  return NULL;
                tr.SetT2O (m);
                break;
              }
              case CS_TOKEN_V:
              {
		csVector3 v;
                if (SyntaxService->ParseVector (&parser, params2, v))
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
          while ((cmd = parser.GetObject (&params, tok_matvec, &name, &params2)) > 0)
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
                if (!SyntaxService->ParseMatrix (&parser, params2, m))
		  return NULL;
                mesh->GetMovable ()->SetTransform (m);
                break;
              }
              case CS_TOKEN_V:
              {
                csVector3 v;
                if (SyntaxService->ParseVector (&parser, params2, v))
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
    CS_TOKEN_TABLE (PARAMSFILE)
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

  while ((cmd = parser.GetObject (&buf, commands, &name, &params)) > 0)
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
	  if (!mesh->GetMeshObject ()->SupportsHardTransform ())
	  {
            ReportError (
	      "crystalspace.maploader.parse.meshobject",
              "This mesh object doesn't support HARDMOVE!");
	    return false;
	  }
          char* params2;
	  csReversibleTransform tr;
          while ((cmd = parser.GetObject (&params, tok_matvec, &name, &params2)) > 0)
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
                if (!SyntaxService->ParseMatrix (&parser, params2, m))
		  return false;
                tr.SetT2O (m);
                break;
              }
              case CS_TOKEN_V:
              {
		csVector3 v;
                if (SyntaxService->ParseVector (&parser, params2, v))
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
          while ((cmd = parser.GetObject (&params, tok_matvec, &name, &params2)) > 0)
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
                if (!SyntaxService->ParseMatrix (&parser, params2, m))
		  return false;
                mesh->GetMovable ()->SetTransform (m);
                break;
              }
              case CS_TOKEN_V:
              {
                csVector3 v;
                if (SyntaxService->ParseVector (&parser, params2, v))
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
	  iBase* mo = plug->Parse (params, GetLoaderContext (), NULL);
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
              "Error parsing PARAMS() in plugin '%s'!", str);
	    return false;
          }
	}
        break;
      case CS_TOKEN_PARAMSFILE:
	if (!plug)
	{
          ReportError (
	      "crystalspace.maploader.load.plugin",
              "Could not load plugin!");
	  return false;
	}
	else
        {
          csScanStr (params, "%s", str);
          iDataBuffer *buf = VFS->ReadFile (str);
	  if (!buf)
	  {
            ReportError (
	      "crystalspace.maploader.parse.loadingfile",
	      "Error opening file '%s'!", str);
	    return false;
	  }
	  iBase* mo = plug->Parse ((char*)(buf->GetUint8 ()),
	  	GetLoaderContext (), NULL);
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
              "Error parsing PARAMSFILE() in plugin '%s'!", str);
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

  while ((cmd = parser.GetObject (&buf, commands, &name, &params)) > 0)
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
	  plug->Parse (params, GetLoaderContext (), context);
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

bool csLoader::LoadSettings (char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (AMBIENT)
    CS_TOKEN_TABLE (CLEARZBUF)
    CS_TOKEN_TABLE (CLEARSCREEN)
    CS_TOKEN_TABLE (LIGHTMAPCELLSIZE)
    CS_TOKEN_TABLE (MAXLIGHTMAPSIZE)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  while ((cmd = parser.GetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (
	  "crystalspace.maploader.parse.badformat",
	  "Expected parameters instead of '%s' while parsing settings!",
	  buf);
      return false;
    }
    switch (cmd)
    {
      case CS_TOKEN_CLEARZBUF:
      {
        bool yesno;
	csScanStr (params, "%b", &yesno);
	Engine->SetClearZBuf (yesno);
        break;
      }
      case CS_TOKEN_CLEARSCREEN:
      {
        bool yesno;
	csScanStr (params, "%b", &yesno);
	Engine->SetClearScreen (yesno);
        break;
      }
      case CS_TOKEN_LIGHTMAPCELLSIZE:
      {
	int cellsize;
	csScanStr (params, "%d", &cellsize);
	if (cellsize >= 0)
	{
	  if (!csIsPowerOf2(cellsize) )
	  {
	    int newcellsize = csFindNearestPowerOf2(cellsize);
	    ReportNotify ("lightmap cell size %d (line %d) "
	      "is not a power of two, using %d", 
	      cellsize, parser.GetParserLine(), newcellsize);
	    cellsize = newcellsize;
	  }
	  Engine->SetLightmapCellSize (cellsize);
	}
	else
	{
	  ReportNotify ("bogus lightmap cell size %d, line %d", 
	    cellsize, parser.GetParserLine());
	}
	break;
      }
      case CS_TOKEN_MAXLIGHTMAPSIZE:
      {
	int max[2], num;
	csScanStr (params, "%D", max, &num);
	if (num == 1) 
	  max[1] = max[0];
	if ( (max[0] > 0) && (max[1] > 0) )
	{
	  Engine->SetMaxLightmapSize (max[0], max[1]);
	}
	else
	{
	  ReportNotify ("bogus maximum lightmap size %dx%d, line %d", 
	    max[0], max[1], parser.GetParserLine());
	}
	break;
      }
      case CS_TOKEN_AMBIENT:
      {
	csColor c;
	ParseColor (params, c);
	Engine->SetAmbientLight ( c );
	break;
      }
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    TokenError ("the settings");
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

  while ((cmd = parser.GetObject (&buf, commands, &name, &params)) > 0)
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

  if (parser.GetObject (&buf, tokens, &name, &data))
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
      if (!LoadMeshObject (mesh, data))
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
    "crystalspace.mesh.crossbuilder, crystalspace.modelconverter. ")
SCF_EXPORT_CLASS_TABLE_END

CS_IMPLEMENT_PLUGIN

csLoader::csLoader (iBase *p)
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
  Stats = new csLoaderStats ();
  ldr_context = NULL;
}

csLoader::~csLoader()
{
  loaded_plugins.DeleteAll ();
  SCF_DEC_REF(ldr_context);
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
  SCF_DEC_REF(SyntaxService);
  delete Stats;
}

iLoaderContext* csLoader::GetLoaderContext ()
{
  if (!ldr_context)
  {
    ldr_context = new StdLoaderContext (Engine, ResolveOnlyRegion,
      &parser);
  }
  return ldr_context;
}

#define GET_PLUGIN(var, intf, msgname)				\
  var = CS_QUERY_REGISTRY(object_reg, intf);			\
  if (!var) ReportNotify ("Could not get " msgname);

#define GET_CRITICAL_PLUGIN(var, intf, msgname)			\
  var = CS_QUERY_REGISTRY(object_reg, intf);			\
  if (!var) { ReportError ("crystalspace.maploader",		\
    "Failed to initialize the loader: "				\
    "Could not get " msgname); return false; }

bool csLoader::Initialize (iObjectRegistry *object_Reg)
{
  csLoader::object_reg = object_Reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);

  Reporter = CS_QUERY_REGISTRY (object_reg, iReporter);

  loaded_plugins.plugin_mgr = plugin_mgr;

  // get the virtual file system plugin
  GET_CRITICAL_PLUGIN (VFS, iVFS, "VFS");
  // get syntax services
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

  xmltokens.Register ("ambient", XMLTOKEN_AMBIENT);
  xmltokens.Register ("addon", XMLTOKEN_ADDON);
  xmltokens.Register ("attenuation", XMLTOKEN_ATTENUATION);
  xmltokens.Register ("camera", XMLTOKEN_CAMERA);
  xmltokens.Register ("center", XMLTOKEN_CENTER);
  xmltokens.Register ("clearzbuf", XMLTOKEN_CLEARZBUF);
  xmltokens.Register ("clearscreen", XMLTOKEN_CLEARSCREEN);
  xmltokens.Register ("collection", XMLTOKEN_COLLECTION);
  xmltokens.Register ("color", XMLTOKEN_COLOR);
  xmltokens.Register ("convex", XMLTOKEN_CONVEX);
  xmltokens.Register ("culler", XMLTOKEN_CULLER);
  xmltokens.Register ("cullerp", XMLTOKEN_CULLERP);
  xmltokens.Register ("detail", XMLTOKEN_DETAIL);
  xmltokens.Register ("dynamic", XMLTOKEN_DYNAMIC);
  xmltokens.Register ("dither", XMLTOKEN_DITHER);
  xmltokens.Register ("diffuse", XMLTOKEN_DIFFUSE);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("farplane", XMLTOKEN_FARPLANE);
  xmltokens.Register ("file", XMLTOKEN_FILE);
  xmltokens.Register ("fog", XMLTOKEN_FOG);
  xmltokens.Register ("forward", XMLTOKEN_FORWARD);
  xmltokens.Register ("for2d", XMLTOKEN_FOR2D);
  xmltokens.Register ("for3d", XMLTOKEN_FOR3D);
  xmltokens.Register ("halo", XMLTOKEN_HALO);
  xmltokens.Register ("hardmove", XMLTOKEN_HARDMOVE);
  xmltokens.Register ("heightgen", XMLTOKEN_HEIGHTGEN);
  xmltokens.Register ("invisible", XMLTOKEN_INVISIBLE);
  xmltokens.Register ("key", XMLTOKEN_KEY);
  xmltokens.Register ("layer", XMLTOKEN_LAYER);
  xmltokens.Register ("level", XMLTOKEN_LEVEL);
  xmltokens.Register ("library", XMLTOKEN_LIBRARY);
  xmltokens.Register ("light", XMLTOKEN_LIGHT);
  xmltokens.Register ("lightmapcellsize", XMLTOKEN_LIGHTMAPCELLSIZE);
  xmltokens.Register ("lmcache", XMLTOKEN_LMCACHE);
  xmltokens.Register ("lod", XMLTOKEN_LOD);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("materials", XMLTOKEN_MATERIALS);
  xmltokens.Register ("matrix", XMLTOKEN_MATRIX);
  xmltokens.Register ("maxlightmapsize", XMLTOKEN_MAXLIGHTMAPSIZE);
  xmltokens.Register ("meshfact", XMLTOKEN_MESHFACT);
  xmltokens.Register ("meshlib", XMLTOKEN_MESHLIB);
  xmltokens.Register ("meshobj", XMLTOKEN_MESHOBJ);
  xmltokens.Register ("meshref", XMLTOKEN_MESHREF);
  xmltokens.Register ("move", XMLTOKEN_MOVE);
  xmltokens.Register ("mipmap", XMLTOKEN_MIPMAP);
  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("node", XMLTOKEN_NODE);
  xmltokens.Register ("nolighting", XMLTOKEN_NOLIGHTING);
  xmltokens.Register ("noshadows", XMLTOKEN_NOSHADOWS);
  xmltokens.Register ("params", XMLTOKEN_PARAMS);
  xmltokens.Register ("paramsfile", XMLTOKEN_PARAMSFILE);
  xmltokens.Register ("plugin", XMLTOKEN_PLUGIN);
  xmltokens.Register ("plugins", XMLTOKEN_PLUGINS);
  xmltokens.Register ("position", XMLTOKEN_POSITION);
  xmltokens.Register ("priority", XMLTOKEN_PRIORITY);
  xmltokens.Register ("proctex", XMLTOKEN_PROCTEX);
  xmltokens.Register ("procedural", XMLTOKEN_PROCEDURAL);
  xmltokens.Register ("persistent", XMLTOKEN_PERSISTENT);
  xmltokens.Register ("radius", XMLTOKEN_RADIUS);
  xmltokens.Register ("region", XMLTOKEN_REGION);
  xmltokens.Register ("renderpriorities", XMLTOKEN_RENDERPRIORITIES);
  xmltokens.Register ("reflection", XMLTOKEN_REFLECTION);
  xmltokens.Register ("scale", XMLTOKEN_SCALE);
  xmltokens.Register ("sector", XMLTOKEN_SECTOR);
  xmltokens.Register ("settings", XMLTOKEN_SETTINGS);
  xmltokens.Register ("shift", XMLTOKEN_SHIFT);
  xmltokens.Register ("sound", XMLTOKEN_SOUND);
  xmltokens.Register ("sounds", XMLTOKEN_SOUNDS);
  xmltokens.Register ("start", XMLTOKEN_START);
  xmltokens.Register ("texture", XMLTOKEN_TEXTURE);
  xmltokens.Register ("textures", XMLTOKEN_TEXTURES);
  xmltokens.Register ("transparent", XMLTOKEN_TRANSPARENT);
  xmltokens.Register ("type", XMLTOKEN_TYPE);
  xmltokens.Register ("matset", XMLTOKEN_MATSET);
  xmltokens.Register ("up", XMLTOKEN_UP);
  xmltokens.Register ("v", XMLTOKEN_V);
  xmltokens.Register ("world", XMLTOKEN_WORLD);
  xmltokens.Register ("zfill", XMLTOKEN_ZFILL);
  xmltokens.Register ("znone", XMLTOKEN_ZNONE);
  xmltokens.Register ("zuse", XMLTOKEN_ZUSE);
  xmltokens.Register ("ztest", XMLTOKEN_ZTEST);
  xmltokens.Register ("blend", XMLTOKEN_BLEND);
  xmltokens.Register ("constant", XMLTOKEN_CONSTANT);
  xmltokens.Register ("generate", XMLTOKEN_GENERATE);
  xmltokens.Register ("height", XMLTOKEN_HEIGHT);
  xmltokens.Register ("heightmap", XMLTOKEN_HEIGHTMAP);
  xmltokens.Register ("multiply", XMLTOKEN_MULTIPLY);
  xmltokens.Register ("partsize", XMLTOKEN_PARTSIZE);
  xmltokens.Register ("single", XMLTOKEN_SINGLE);
  xmltokens.Register ("size", XMLTOKEN_SIZE);
  xmltokens.Register ("slope", XMLTOKEN_SLOPE);
  xmltokens.Register ("solid", XMLTOKEN_SOLID);
  xmltokens.Register ("value", XMLTOKEN_VALUE);

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
    "Token '%s' not recognized while parsing %s!",
    parser.GetLastOffender (), Object);
}

void csLoader::TokenError (const char *Object, const char* Token)
{
  ReportError (
    "crystalspace.maploader.parse.badtoken",
    "Token '%s' not recognized while parsing %s!",
    Token, Object);
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
  while ((cmd = parser.GetObject (&buf, commands, &xname, &params)) > 0)
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
	  iSector* s = GetLoaderContext ()->FindSector (str);
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
    CS_TOKEN_TABLE (FARPLANE)
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
    while ((cmd = parser.GetCommand (&buf, commands, &params)) > 0)
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
        case CS_TOKEN_FARPLANE:
        {
	  csPlane3 p;
          csScanStr (params, "%f,%f,%f,%f", &p.A (), &p.B (), &p.C (), &p.D ());
	  campos->SetFarPlane (&p);
	  break;
        }
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
    while ((cmd = parser.GetCommand (&buf, commands, &params)) > 0)
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
	    iLoaderContext* lc = GetLoaderContext ();
	    halo.flare.mat_center = lc->FindMaterial (mat_names[0]);
	    if (!halo.flare.mat_center)
	    {
	      ReportError (
		"crystalspace.maploader.parse.light",
    	        "Can't find material for flare!");
	      return NULL;
	    }
	    halo.flare.mat_spark1 = lc->FindMaterial (mat_names[1]);
	    if (!halo.flare.mat_spark1)
	    {
	      ReportError (
		"crystalspace.maploader.parse.light",
    	        "Can't find material for flare!");
	      return NULL;
	    }
	    halo.flare.mat_spark2 = lc->FindMaterial (mat_names[2]);
	    if (!halo.flare.mat_spark2)
	    {
	      ReportError (
		"crystalspace.maploader.parse.light",
    	        "Can't find material for flare!");
	      return NULL;
	    }
	    halo.flare.mat_spark3 = lc->FindMaterial (mat_names[3]);
	    if (!halo.flare.mat_spark3)
	    {
	      ReportError (
		"crystalspace.maploader.parse.light",
    	        "Can't find material for flare!");
	      return NULL;
	    }
	    halo.flare.mat_spark4 = lc->FindMaterial (mat_names[4]);
	    if (!halo.flare.mat_spark4)
	    {
	      ReportError (
		"crystalspace.maploader.parse.light",
    	        "Can't find material for flare!");
	      return NULL;
	    }
	    halo.flare.mat_spark5 = lc->FindMaterial (mat_names[5]);
	    if (!halo.flare.mat_spark5)
	    {
	      ReportError (
		"crystalspace.maploader.parse.light",
    	        "Can't find material for flare!");
	      return NULL;
	    }
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
      case CS_ATTN_INVERSE   : dist = 16.0f * qsqrt (dist); break;
      case CS_ATTN_REALISTIC : dist = 256.0f * dist; break;
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
		parser.GetParserLine());
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

  while ((cmd = parser.GetObject (&buf, commands, &xname, &params)) > 0)
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
    CS_TOKEN_TABLE (CULLERP)
    CS_TOKEN_TABLE (FOG)
    CS_TOKEN_TABLE (LIGHT)
    CS_TOKEN_TABLE (MESHOBJ)
    CS_TOKEN_TABLE (MESHREF)
    CS_TOKEN_TABLE (MESHLIB)
    CS_TOKEN_TABLE (NODE)
    CS_TOKEN_TABLE (KEY)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  bool do_culler = false;
  char bspname[100]; *bspname = 0;
  char culplugname[100]; *culplugname = 0;

  iSector* sector = GetLoaderContext ()->FindSector (secname);
  if (sector == NULL)
  {
    sector = Engine->CreateSector (secname);
    Stats->sectors_loaded++;
  }

  while ((cmd = parser.GetObject (&buf, commands, &name, &params)) > 0)
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
	{
          do_culler = true;
	  *culplugname = 0;
	}
        break;
      case CS_TOKEN_CULLERP:
	if (!csScanStr (params, "%s", culplugname))
	{
	  ReportError (
		"crystalspace.maploader.parse.sector",
	  	"CULLERP expects the name of a visibility culling plugin!");
	  return NULL;
	}
	else
	{
          do_culler = true;
	  *bspname = 0;
	}
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
      case CS_TOKEN_MESHLIB:
        {
	  iMeshWrapper* mesh = Engine->GetMeshes ()->FindByName (name);
	  if (!mesh)
	  {
      	    ReportError (
	      	"crystalspace.maploader.load.meshobject",
		"Could not find mesh object '%s' in sector '%s' for MESHLIB!",
		name, secname ? secname : "<noname>");
	    return NULL;
	  }
	  if (mesh->GetMovable ()->GetSectors ()->GetCount () > 0)
	  {
      	    ReportError (
	      	"crystalspace.maploader.load.meshobject",
		"Mesh '%s' is already in another sector in sector '%s'!",
		name, secname ? secname : "<noname>");
	    return NULL;
	  }
          if (!LoadMeshObject (mesh, params))
	  {
      	    ReportError (
	      	"crystalspace.maploader.load.meshobject",
		"Could not load mesh object '%s' in sector '%s'!",
		name, secname ? secname : "<noname>");
	    return NULL;
	  }
	  mesh->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
          mesh->GetMovable ()->SetSector (sector);
	  mesh->GetMovable ()->UpdateMove ();
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
    if (do_culler)
    {
      bool rc;
      if (*bspname)
        rc = sector->SetVisibilityCuller (bspname);
      else
        rc = sector->SetVisibilityCullerPlugin (culplugname);
      if (!rc)
      {
	ReportError (
	      	"crystalspace.maploader.load.sector",
		"Could not load visibility culler for sector '%s'!",
		secname ? secname : "<noname>");
	return NULL;
      }
    }
  return sector;
}

//========================================================================
// New XML versions of all functions accepting char*. Soon these
// will be the only ones remaining.
//========================================================================

bool csLoader::LoadMap (iDocumentNode* node)
{
  if (!Engine) return false;

  csRef<iDocumentNode> worldnode = node->GetNode ("world");
  if (worldnode)
  {
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
	  if (!LoadAddOn (child, (iEngine*)Engine))
	    return false;
      	  break;
        case XMLTOKEN_MESHFACT:
          {
	    const char* name = child->GetAttributeValue ("name");
            csRef<iMeshFactoryWrapper> t;
	    t.Take (Engine->CreateMeshFactory (name));
	    if (!t || !LoadMeshObjectFactory (t, child))
	    {
	      ReportError (
			   "crystalspace.maploader.load.meshfactory",
			   "Could not load mesh object factory '%s'!",
			   name);
	      return false;
	    }
          }
	  break;
        case XMLTOKEN_REGION:
	  {
	    const char* regname = child->GetContentsValue ();
	    if (regname)
	      Engine->SelectRegion (regname);
	    else
	      Engine->SelectRegion ((iRegion*)NULL);
	  }
	  break;
        case XMLTOKEN_SECTOR:
          if (!ParseSector (child))
	    return false;
          break;
        case XMLTOKEN_COLLECTION:
          if (!ParseCollection (child))
	    return false;
          break;
	case XMLTOKEN_MATSET:
          if (!ParseMaterialList (child))
            return false;
          break;
	case XMLTOKEN_PLUGINS:
	  if (!LoadPlugins (child))
	    return false;
	  break;
        case XMLTOKEN_TEXTURES:
          if (!ParseTextureList (child))
            return false;
          break;
        case XMLTOKEN_MATERIALS:
          if (!ParseMaterialList (child))
            return false;
          break;
        case XMLTOKEN_SOUNDS:
          if (!LoadSounds (child))
            return false;
          break;
        case XMLTOKEN_LIBRARY:
          if (!LoadLibraryFile (child->GetContentsValue ()))
	    return false;
          break;
        case XMLTOKEN_START:
        {
	  const char* name = child->GetAttributeValue ("name");
	  iCameraPosition* campos = Engine->GetCameraPositions ()->
	  	NewCameraPosition (name ? name : "Start");
	  if (!ParseStart (child, campos))
	    return false;
          break;
        }
        case XMLTOKEN_KEY:
	  {
            iKeyValuePair* kvp = ParseKey (child, Engine->QueryObject());
	    if (kvp)
	      kvp->DecRef ();
	    else
	      return false;
	  }
          break;
	default:
	  TokenError ("a map", value);
	  return false;
      }
    }
  }
  else
  {
    ReportError (
      "crystalspace.maploader.parse.expectedworld",
      "Expected 'world' token!");
    return false;
  }

  return true;
}

bool csLoader::LoadLibrary (iDocumentNode* node)
{
  if (!Engine)
  {
    ReportError (
	  "crystalspace.maploader.parse.noengine",
	  "No engine present while in LoadLibrary!");
    return false;
  }
 
  csRef<iDocumentNode> libnode = node->GetNode ("library");
  if (libnode)
  {
    csRef<iDocumentNodeIterator> it = libnode->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
        case XMLTOKEN_ADDON:
	  if (!LoadAddOn (child, (iEngine*)Engine))
	    return false;
      	  break;
        case XMLTOKEN_TEXTURES:
          // Append textures to engine.
          if (!ParseTextureList (child))
            return false;
          break;
        case XMLTOKEN_MATERIALS:
          if (!ParseMaterialList (child))
            return false;
          break;
        case XMLTOKEN_SOUNDS:
          if (!LoadSounds (child))
            return false;
          break;
        case XMLTOKEN_MESHREF:
          {
            iMeshWrapper* mesh = LoadMeshObjectFromFactory (child);
            if (!mesh)
	    {
      	      ReportError (
	      	  "crystalspace.maploader.load.meshobject",
		  "Could not load mesh object '%s' in library!",
		  child->GetAttributeValue ("name"));
	      return false;
	    }
	    mesh->QueryObject ()->SetName (child->GetAttributeValue ("name"));
	    Engine->GetMeshes ()->Add (mesh);
	    //mesh->DecRef ();
          }
          break;
        case XMLTOKEN_MESHOBJ:
          {
	    iMeshWrapper* mesh = Engine->CreateMeshWrapper (
			    child->GetAttributeValue ("name"));
            if (!LoadMeshObject (mesh, child))
	    {
      	      ReportError (
	      	  "crystalspace.maploader.load.meshobject",
		  "Could not load mesh object '%s' in library!",
		  child->GetAttributeValue ("name"));
	      mesh->DecRef ();
	      return false;
	    }
	    //mesh->DecRef ();
          }
          break;
        case XMLTOKEN_MESHFACT:
          {
            iMeshFactoryWrapper* t = Engine->CreateMeshFactory (
			    child->GetAttributeValue ("name"));
	    if (t)
	    {
	      if (!LoadMeshObjectFactory (t, child))
	      {
		t->DecRef ();
		ReportError (
		     "crystalspace.maploader.load.library.meshfactory",
		     "Could not load mesh object factory '%s' in library!",
		     child->GetAttributeValue ("name"));
		return false;
	      }
	      t->DecRef ();
	    }
	  }
	  break;
        case XMLTOKEN_PLUGINS:
	  if (!LoadPlugins (child))
	    return false;
          break;
	default:
          TokenError ("a library file", value);
          return false;
      }
    }
  }
  else
  {
    ReportError (
      "crystalspace.maploader.parse.expectedlib",
      "Expected 'library' token!");
    return false;
  }

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
			child->GetContentsValue ());
        break;
      default:
	TokenError ("plugin descriptors", value);
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
          iSoundWrapper *snd =
	    CS_GET_NAMED_CHILD_OBJECT (Engine->QueryObject (), iSoundWrapper, name);
          if (!snd)
            LoadSound (name, filename);
        }
        break;
      default:
        TokenError ("the list of sounds", value);
        return false;
    }
  }

  return true;
}

bool csLoader::LoadLodControl (iLODControl* lodctrl, iDocumentNode* node)
{
  float level = 1;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_LEVEL:
	level = child->GetContentsValueAsFloat ();
        break;
      default:
        TokenError ("a LOD control", value);
        return false;
    }
  }

  lodctrl->SetLOD (level);

  return true;
}

bool csLoader::LoadMeshObjectFactory (iMeshFactoryWrapper* stemp,
	iDocumentNode* node, csReversibleTransform* transf)
{
  iLoaderPlugin* plug = NULL;
  iMaterialWrapper *mat = NULL;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
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
            ReportError (
	      "crystalspace.maploader.parse.meshfactory",
              "Please use 'params' before specifying LOD!");
	    return false;
	  }
	  csRef<iLODControl> lodctrl;
	  lodctrl.Take (SCF_QUERY_INTERFACE (
	    	stemp->GetMeshObjectFactory (),
		iLODControl));
	  if (!lodctrl)
	  {
            ReportError (
	      "crystalspace.maploader.parse.meshfactory",
              "This mesh factory doesn't implement LOD control!");
	    return false;
	  }
	  if (!LoadLodControl (lodctrl, child))
	    return false;
	}
        break;
      case XMLTOKEN_ADDON:
	if (!LoadAddOn (child, stemp))
	  return false;
      	break;
      case XMLTOKEN_PARAMS:
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
	  iBase* mof = plug->Parse (child, GetLoaderContext (),
	  	stemp->GetMeshObjectFactory ());
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
      case XMLTOKEN_PARAMSFILE:
	if (!plug)
	{
          ReportError (
	      "crystalspace.maploader.load.plugin",
              "Could not load plugin!");
	  return false;
	}
	else
        {
          iDataBuffer *buf = VFS->ReadFile (child->GetContentsValue ());
	  if (!buf)
	  {
            ReportError (
	      "crystalspace.maploader.parse.loadingfile",
	      "Error opening file '%s'!", child->GetContentsValue ());
	    return false;
	  }

	  // We give here the iMeshObjectFactory as the context. If this
	  // is a new factory this will be NULL. Otherwise it is possible
	  // to append information to the already loaded factory.
	  // @@@ SWITCH TO XML HERE!
	  iBase* mof = plug->Parse ((char*)(buf->GetUint8 ()),
	  	GetLoaderContext (), stemp->GetMeshObjectFactory ());
	  buf->DecRef ();
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

      case XMLTOKEN_MATERIAL:
        {
	  if (!stemp->GetMeshObjectFactory ())
	  {
            ReportError (
	      "crystalspace.maploader.parse.meshfactory",
              "Please use 'params' before specifying 'material'!");
	    return false;
	  }
	  const char* matname = child->GetContentsValue ();
          mat = GetLoaderContext ()->FindMaterial (matname);
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
              "Material '%s' not found!", matname);
	    return false;
          }
        }
        break;

      case XMLTOKEN_FILE:
        {
          if (!ModelConverter || !CrossBuilder) return false;

	  const char* filename = child->GetContentsValue ();
          iDataBuffer *buf = VFS->ReadFile (filename);
	  if (!buf)
	  {
            ReportError (
	      "crystalspace.maploader.parse.loadingmodel",
	      "Error opening file model '%s'!", filename);
	    return false;
	  }

	  iModelData *Model = ModelConverter->Load (buf->GetUint8 (),
	  	buf->GetSize ());
	  buf->DecRef ();
          if (!Model)
	  {
            ReportError (
 	      "crystalspace.maploader.parse.loadingmodel",
	      "Error loading file model '%s'!", filename);
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
	  stemp2->DecRef ();
        }
        break;

      case XMLTOKEN_PLUGIN:
	plug = loaded_plugins.FindPlugin (child->GetContentsValue ());
        break;

      case XMLTOKEN_MESHFACT:
        {
          iMeshFactoryWrapper* t = Engine->CreateMeshFactory (child->GetAttributeValue ("name"));
	  csReversibleTransform child_transf;
          if (!LoadMeshObjectFactory (t, child, &child_transf))
	  {
	    ReportError (
	    	"crystalspace.maploader.load.meshfactory",
		"Could not load mesh object factory '%s'!",
		child->GetAttributeValue ("name"));
	    if (t) t->DecRef ();
	    return false;
	  }
	  stemp->GetChildren ()->Add (t);
	  t->SetTransform (child_transf);
	  t->DecRef ();
        }
	break;

      case XMLTOKEN_MOVE:
        {
	  if (!transf)
	  {
	    ReportError (
	    	"crystalspace.maploader.load.meshfactory",
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
            ReportError (
	      "crystalspace.maploader.parse.meshfactory",
              "Please use 'params' before specifying 'hardmove'!");
	    return false;
	  }
	  if (!stemp->GetMeshObjectFactory ()->SupportsHardTransform ())
	  {
            ReportError (
	      "crystalspace.maploader.parse.meshfactory",
              "This factory doesn't support 'hardmove'!");
	    return false;
	  }
	  csReversibleTransform tr;
	  csRef<iDocumentNode> matrix_node = child->GetNode ("matrix");
	  if (matrix_node)
	  {
	    csMatrix3 m;
	    if (!SyntaxService->ParseMatrix (matrix_node, m))
	      return false;
            tr.SetO2T (m);
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
      default:
        TokenError ("a mesh factory", value);
        return false;
    }
  }

  return true;
}

iMeshWrapper* csLoader::LoadMeshObjectFromFactory (iDocumentNode* node)
{
  if (!Engine) return NULL;

  const char* priority;

  Stats->meshes_loaded++;
  iMeshWrapper* mesh = NULL;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_LOD:
        {
          if (!mesh)
	  {
	    ReportError (
	  	  "crystalspace.maploader.load.meshobject",
	  	  "First specify the parent factory with 'factory'!");
	    return NULL;
	  }
	  if (!mesh->GetMeshObject ())
	  {
            ReportError (
	      "crystalspace.maploader.parse.meshobject",
              "Mesh object is missing!");
	    return NULL;
	  }
	  csRef<iLODControl> lodctrl;
	  lodctrl.Take (SCF_QUERY_INTERFACE (
	    	mesh->GetMeshObject (),
		iLODControl));
	  if (!lodctrl)
	  {
            ReportError (
	      "crystalspace.maploader.parse.meshobject",
              "This mesh doesn't implement LOD control!");
	    return NULL;
	  }
	  if (!LoadLodControl (lodctrl, child))
	    return NULL;
	}
        break;
      case XMLTOKEN_PRIORITY:
	priority = child->GetContentsValue ();
	break;
      case XMLTOKEN_ADDON:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with 'factory'!");
	  return NULL;
	}
	if (!LoadAddOn (child, mesh))
	  return NULL;
      	break;
      case XMLTOKEN_NOLIGHTING:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with 'factory'!");
	  return NULL;
	}
        mesh->GetFlags().Set (CS_ENTITY_NOLIGHTING);
        break;
      case XMLTOKEN_NOSHADOWS:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with 'factory'!");
	  return NULL;
	}
        mesh->GetFlags().Set (CS_ENTITY_NOSHADOWS);
        break;
      case XMLTOKEN_INVISIBLE:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with 'factory'!");
	  return NULL;
	}
        mesh->GetFlags().Set (CS_ENTITY_INVISIBLE);
        break;
      case XMLTOKEN_DETAIL:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with 'factory'!");
	  return NULL;
	}
        mesh->GetFlags().Set (CS_ENTITY_DETAIL);
        break;
      case XMLTOKEN_ZFILL:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with 'factory'!");
	  return NULL;
	}
        if (!priority) priority = "wall";
        mesh->SetZBufMode (CS_ZBUF_FILL);
        break;
      case XMLTOKEN_ZUSE:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with 'factory'!");
	  return NULL;
	}
        if (!priority) priority = "object";
        mesh->SetZBufMode (CS_ZBUF_USE);
        break;
      case XMLTOKEN_ZNONE:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with 'factory'!");
	  return NULL;
	}
        if (!priority) priority = "sky";
        mesh->SetZBufMode (CS_ZBUF_NONE);
        break;
      case XMLTOKEN_ZTEST:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with 'factory'!");
	  return NULL;
	}
        if (!priority) priority = "alpha";
        mesh->SetZBufMode (CS_ZBUF_TEST);
        break;
      case XMLTOKEN_CAMERA:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with 'factory'!");
	  return NULL;
	}
        if (!priority) priority = "sky";
        mesh->GetFlags().Set (CS_ENTITY_CAMERA);
        break;
      case XMLTOKEN_CONVEX:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with 'factory'!");
	  return NULL;
	}
        mesh->GetFlags().Set (CS_ENTITY_CONVEX);
        break;
      case XMLTOKEN_KEY:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with 'factory'!");
	  return NULL;
	}
	{
          iKeyValuePair* kvp = ParseKey (child, mesh->QueryObject());
	  if (kvp)
	    kvp->DecRef ();
	  else
	    return NULL;
	}
        break;
      case XMLTOKEN_HARDMOVE:
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with 'factory'!");
	  return NULL;
	}
	else if (!mesh->GetMeshObject ()->SupportsHardTransform ())
	{
          ReportError (
	    "crystalspace.maploader.parse.meshobject",
            "This mesh object doesn't support 'hardmove'!");
	  return NULL;
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
            tr.SetO2T (m);
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
        if (!mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"First specify the parent factory with 'factory'!");
	  return NULL;
	}
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

      case XMLTOKEN_FACTORY:
        if (mesh)
	{
	  ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"There is already a factory for this mesh!");
	  return NULL;
	}
	else
	{
          iMeshFactoryWrapper* t = Engine->GetMeshFactories ()
	  	->FindByName (child->GetContentsValue ());
          if (!t)
	  {
	    ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"Can't find factory '%s'!", child->GetContentsValue ());
	    return NULL;
	  }
	  mesh = t->CreateMeshWrapper ();
	}
        break;
      default:
        TokenError ("a mesh object", value);
	return NULL;
    }
  }

  if (!mesh)
  {
    ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	"There is no 'factory' for this mesh!");
    return NULL;
  }
  if (!priority) priority = "object";
  mesh->SetRenderPriority (Engine->GetRenderPriority (priority));

  return mesh;
}

bool csLoader::LoadMeshObject (iMeshWrapper* mesh, iDocumentNode* node)
{
  if (!Engine) return false;

  const char* priority = NULL;

  Stats->meshes_loaded++;
  iLoaderPlugin* plug = NULL;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_LOD:
        {
	  if (!mesh->GetMeshObject ())
	  {
            ReportError (
	      "crystalspace.maploader.parse.meshobject",
	      "Only use 'lod' after 'params'!");
	    return false;
	  }
	  csRef<iLODControl> lodctrl;
	  lodctrl.Take (SCF_QUERY_INTERFACE (mesh->GetMeshObject (),
		iLODControl));
	  if (!lodctrl)
	  {
            ReportError (
	      "crystalspace.maploader.parse.meshobject",
              "This mesh doesn't implement LOD control!");
	    return false;
	  }
	  if (!LoadLodControl (lodctrl, child))
	  {
	    return false;
	  }
	}
        break;
      case XMLTOKEN_PRIORITY:
	priority = child->GetContentsValue ();
	break;
      case XMLTOKEN_ADDON:
	if (!LoadAddOn (child, mesh))
	  return false;
      	break;
      case XMLTOKEN_NOLIGHTING:
        mesh->GetFlags().Set (CS_ENTITY_NOLIGHTING);
        break;
      case XMLTOKEN_NOSHADOWS:
        mesh->GetFlags().Set (CS_ENTITY_NOSHADOWS);
        break;
      case XMLTOKEN_INVISIBLE:
        mesh->GetFlags().Set (CS_ENTITY_INVISIBLE);
        break;
      case XMLTOKEN_DETAIL:
        mesh->GetFlags().Set (CS_ENTITY_DETAIL);
        break;
      case XMLTOKEN_ZFILL:
        if (!priority) priority = "wall";
        mesh->SetZBufMode (CS_ZBUF_FILL);
        break;
      case XMLTOKEN_ZUSE:
        if (!priority) priority = "object";
        mesh->SetZBufMode (CS_ZBUF_USE);
        break;
      case XMLTOKEN_ZNONE:
        if (!priority) priority = "sky";
        mesh->SetZBufMode (CS_ZBUF_NONE);
        break;
      case XMLTOKEN_ZTEST:
        if (!priority) priority = "alpha";
        mesh->SetZBufMode (CS_ZBUF_TEST);
        break;
      case XMLTOKEN_CAMERA:
        if (!priority) priority = "sky";
        mesh->GetFlags().Set (CS_ENTITY_CAMERA);
        break;
      case XMLTOKEN_CONVEX:
        mesh->GetFlags().Set (CS_ENTITY_CONVEX);
        break;
      case XMLTOKEN_KEY:
        {
          iKeyValuePair* kvp = ParseKey (child, mesh->QueryObject());
          if (kvp)
	    kvp->DecRef ();
	  else
	    return false;
	}
        break;
      case XMLTOKEN_MESHREF:
        {
          iMeshWrapper* sp = LoadMeshObjectFromFactory (child);
          if (!sp)
	  {
	    ReportError (
	      	"crystalspace.maploader.load.meshobject",
		"Could not load mesh object '%s'!",
		child->GetAttributeValue ("name"));
	    return false;
	  }
	  sp->QueryObject ()->SetName (child->GetAttributeValue ("name"));
	  sp->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
          mesh->GetChildren ()->Add (sp);
	  sp->DecRef ();
        }
        break;
      case XMLTOKEN_MESHOBJ:
        {
	  iMeshWrapper* sp = Engine->CreateMeshWrapper (
			  child->GetAttributeValue ("name"));
          if (!LoadMeshObject (sp, child))
	  {
	    ReportError (
	      	"crystalspace.maploader.load.meshobject",
		"Could not load mesh object '%s'!",
		child->GetAttributeValue ("name"));
	    return false;
	  }
	  sp->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
          mesh->GetChildren ()->Add (sp);
	  sp->DecRef ();
        }
        break;
      case XMLTOKEN_HARDMOVE:
        {
	  if (!mesh->GetMeshObject ()->SupportsHardTransform ())
	  {
            ReportError (
	      "crystalspace.maploader.parse.meshobject",
              "This mesh object doesn't support 'hardmove'!");
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
	  mesh->HardTransform (tr);
        }
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

      case XMLTOKEN_PARAMS:
	if (!plug)
	{
          ReportError (
	      "crystalspace.maploader.load.plugin",
              "Could not load plugin!");
	  return false;
	}
	else
	{
	  iBase* mo = plug->Parse (child, GetLoaderContext (), NULL);
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
              "Error parsing 'params' in mesh '%s'!",
	      mesh->QueryObject ()->GetName ());
	    return false;
          }
	}
        break;
      case XMLTOKEN_PARAMSFILE:
	if (!plug)
	{
          ReportError (
	      "crystalspace.maploader.load.plugin",
              "Could not load plugin for mesh '%s'!",
	      mesh->QueryObject ()->GetName ());
	  return false;
	}
	else
        {
	  const char* fname = child->GetContentsValue ();
	  if (!fname)
	  {
            ReportError (
	      "crystalspace.maploader.parse.loadingfile",
	      "Specify a VFS filename with 'paramsfile'!");
	    return false;
	  }
          iDataBuffer *buf = VFS->ReadFile (fname);
	  if (!buf)
	  {
            ReportError (
	      "crystalspace.maploader.parse.loadingfile",
	      "Error opening file '%s'!", fname);
	    return false;
	  }
	  // @@@ SWITCH TO XML HERE!
	  iBase* mo = plug->Parse ((char*)(buf->GetUint8 ()),
	  	GetLoaderContext (), NULL);
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
              "Error parsing 'paramsfile' '%s'!", fname);
	    return false;
          }

        }
        break;

      case XMLTOKEN_PLUGIN:
	{
	  const char* plugname = child->GetContentsValue ();
	  if (!plugname)
	  {
            ReportError (
	      "crystalspace.maploader.parse.plugin",
	      "Specify a plugin name with 'plugin'!");
	    return false;
	  }
	  plug = loaded_plugins.FindPlugin (plugname);
	}
        break;

      case XMLTOKEN_LMCACHE:
        {
	  if (!mesh->GetMeshObject ())
	  {
            ReportError (
	      "crystalspace.maploader.parse.meshobject",
	      "Only use 'lmcache' after 'params'!");
	    return false;
	  }
	  iLightingInfo* li = SCF_QUERY_INTERFACE (mesh->GetMeshObject (),
	  	iLightingInfo);
	  if (li)
	  {
	    li->SetCacheName (child->GetContentsValue ());
	    li->DecRef ();
	  }
	}
        break;
      default:
	TokenError ("a mesh object", value);
	return false;
    }
  }

  if (!priority) priority = "object";
  mesh->SetRenderPriority (Engine->GetRenderPriority (priority));

  return true;
}

bool csLoader::LoadAddOn (iDocumentNode* node, iBase* context)
{
  iLoaderPlugin* plug = NULL;

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
          ReportError (
	      "crystalspace.maploader.load.plugin",
              "Could not load plugin!");
	  return false;
	}
	else
	{
	  plug->Parse (child, GetLoaderContext (), context);
	}
        break;

      case XMLTOKEN_PLUGIN:
	plug = loaded_plugins.FindPlugin (child->GetContentsValue ());
        break;
      default:
	TokenError ("an add-on", value);
	return false;
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
	  if (cellsize >= 0)
	  {
	    if (!csIsPowerOf2 (cellsize) )
	    {
	      int newcellsize = csFindNearestPowerOf2(cellsize);
	      ReportNotify ("lightmap cell size %d "
	        "is not a power of two, using %d", 
	        cellsize, newcellsize);
	      cellsize = newcellsize;
	    }
	    Engine->SetLightmapCellSize (cellsize);
	  }
	  else
	  {
	    ReportNotify ("bogus lightmap cell size %d", cellsize);
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
	    ReportNotify ("bogus maximum lightmap size %dx%d", max[0], max[1]);
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
      default:
        TokenError ("the settings", value);
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
          ReportError (
	    "crystalspace.maploader.parse.priorities",
	    "Render priority '%s' is missing a 'level'!",
	    name);
	  return false;
	}
	long pri = levelnode->GetContentsValueAsInt ();
	int rendsort = CS_RENDPRI_NONE;
	csRef<iDocumentNode> sortnode = child->GetNode ("sort");
	if (sortnode)
	{
	  const char* sorting = sortnode->GetContentsValue ();
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
	}
	Engine->RegisterRenderPriority (name, pri, rendsort);
        break;
      }
      default:
        TokenError ("the render priorities", value);
	return false;
    }
  }

  return true;
}

iCollection* csLoader::ParseCollection (iDocumentNode* node)
{
  iCollection* collection = Engine->GetCollections ()->NewCollection (
		  node->GetAttributeValue ("name"));

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
	ReportError (
		"crystalspace.maploader.parse.collection",
         	"'addon' not yet supported in collection!");
	return NULL;
      case XMLTOKEN_MESHOBJ:
#if 0
	ReportError (
		"crystalspace.maploader.parse.collection",
         	"'meshobj' not yet supported in collection!");
#endif
        break;
      case XMLTOKEN_LIGHT:
        {
	  const char* lightname = child->GetContentsValue ();
	  iLight* l = NULL;
	  iSectorList* sl = Engine->GetSectors ();
	  int i;
	  for (i = 0 ; i < sl->GetCount () ; i++)
	  {
	    iSector* sect = sl->Get (i);
	    if ((!ResolveOnlyRegion) || (!Engine->GetCurrentRegion ()) ||
	      Engine->GetCurrentRegion ()->IsInRegion (sect->QueryObject ()))
	    {
	      l = sect->GetLights ()->FindByName (lightname);
	      if (l) break;
	    }
	  }
          if (!l)
	  {
	    ReportError (
		"crystalspace.maploader.parse.collection",
            	"Light '%s' not found!", lightname);
	    return NULL;
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
	  iSector* s = GetLoaderContext ()->FindSector (sectname);
          if (!s)
	  {
	    ReportError (
		"crystalspace.maploader.parse.collection",
            	"Sector '%s' not found!", sectname);
	    return NULL;
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
	  //@@@$$$ TODO: Collection in regions.
	  iCollection* th;
	  if (ResolveOnlyRegion && Engine->GetCurrentRegion ())
	    th = Engine->GetCurrentRegion ()->FindCollection (colname);
	  else
            th = Engine->GetCollections ()->FindByName (colname);
          if (!th)
	  {
	    ReportError (
		"crystalspace.maploader.parse.collection",
            	"Collection '%s' not found!", colname);
	    return NULL;
	  }
	  else
	  {
            collection->AddObject (th->QueryObject());
	  }
        }
        break;
      default:
        TokenError ("a collection", value);
	collection->DecRef ();
	return NULL;
    }
  }

  return collection;
}

bool csLoader::ParseStart (iDocumentNode* node, iCameraPosition* campos)
{
  const char* start_sector = "room";
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
	start_sector = child->GetContentsValue ();
	break;
      case XMLTOKEN_POSITION:
	if (!SyntaxService->ParseVector (child, pos))
	  return false;
	break;
      case XMLTOKEN_UP:
	if (!SyntaxService->ParseVector (child, up))
	  return false;
	break;
      case CS_TOKEN_FORWARD:
	if (!SyntaxService->ParseVector (child, forward))
	  return false;
	break;
      case CS_TOKEN_FARPLANE:
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
	TokenError ("a camera position", value);
	return false;
    }
  }

  campos->Set (start_sector, pos, forward, up);
  return true;
}

iStatLight* csLoader::ParseStatlight (iDocumentNode* node)
{
  const char* lightname = node->GetAttributeValue ("name");

  Stats->lights_loaded++;
  csVector3 pos;
  csColor color;
  float dist = 0;
  bool dyn;
  int attenuation = CS_ATTN_LINEAR;
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
  dist = 1;
  color.red = color.green = color.blue = 1;
  dyn = false;

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
	dist = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_CENTER:
	if (!SyntaxService->ParseVector (child, pos))
	  return NULL;
        break;
      case XMLTOKEN_COLOR:
	if (!SyntaxService->ParseColor (child, color))
	  return NULL;
        break;
      case XMLTOKEN_DYNAMIC:
	if (!SyntaxService->ParseBool (child, dyn, true))
	  return NULL;
        break;
      case XMLTOKEN_KEY:
	{
          iKeyValuePair* kvp = ParseKey (child, &Keys);
          if (kvp)
	    kvp->DecRef ();
	  else
	    return NULL;
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
            halo.cross.Intensity = 2.0;
	    halo.cross.Cross = 0.45;
	    csRef<iDocumentNode> intnode = child->GetNode ("intensity");
	    if (intnode) { halo.cross.Intensity = intnode->GetContentsValueAsFloat (); }
	    csRef<iDocumentNode> crossnode = child->GetNode ("cross");
	    if (crossnode) { halo.cross.Cross = crossnode->GetContentsValueAsFloat (); }
	  }
	  else if (!strcasecmp (type, "nova"))
	  {
            halo.type = 2;
            halo.nova.Seed = 0;
	    halo.nova.NumSpokes = 100;
	    halo.nova.Roundness = 0.5;
	    csRef<iDocumentNode> seednode = child->GetNode ("seed");
	    if (seednode) { halo.nova.Seed = seednode->GetContentsValueAsInt (); }
	    csRef<iDocumentNode> spokesnode = child->GetNode ("numspokes");
	    if (spokesnode) { halo.nova.NumSpokes = spokesnode->GetContentsValueAsInt (); }
	    csRef<iDocumentNode> roundnode = child->GetNode ("roundness");
	    if (roundnode) { halo.nova.Roundness = roundnode->GetContentsValueAsFloat (); }
	  }
	  else if (!strcasecmp (type, "flare"))
	  {
            halo.type = 3;
	    iLoaderContext* lc = GetLoaderContext ();
	    csRef<iDocumentNode> matnode;
	    
	    matnode = child->GetNode ("centermaterial");
	    halo.flare.mat_center = matnode ? lc->FindMaterial (
	    	matnode->GetContentsValue ()) : NULL;
	    if (!halo.flare.mat_center)
	    {
	      ReportError (
		  "crystalspace.maploader.parse.light",
    	          "Can't find material for flare!");
	      return NULL;
	    }
	    matnode = child->GetNode ("spark1material");
	    halo.flare.mat_spark1 = matnode ? lc->FindMaterial (
	    	matnode->GetContentsValue ()) : NULL;
	    if (!halo.flare.mat_spark1)
	    {
	      ReportError (
		  "crystalspace.maploader.parse.light",
    	          "Can't find material for flare!");
	      return NULL;
	    }
	    matnode = child->GetNode ("spark2material");
	    halo.flare.mat_spark2 = matnode ? lc->FindMaterial (
	    	matnode->GetContentsValue ()) : NULL;
	    if (!halo.flare.mat_spark2)
	    {
	      ReportError (
		  "crystalspace.maploader.parse.light",
    	          "Can't find material for flare!");
	      return NULL;
	    }
	    matnode = child->GetNode ("spark3material");
	    halo.flare.mat_spark3 = matnode ? lc->FindMaterial (
	    	matnode->GetContentsValue ()) : NULL;
	    if (!halo.flare.mat_spark3)
	    {
	      ReportError (
		  "crystalspace.maploader.parse.light",
    	          "Can't find material for flare!");
	      return NULL;
	    }
	    matnode = child->GetNode ("spark4material");
	    halo.flare.mat_spark4 = matnode ? lc->FindMaterial (
	    	matnode->GetContentsValue ()) : NULL;
	    if (!halo.flare.mat_spark4)
	    {
	      ReportError (
		  "crystalspace.maploader.parse.light",
    	          "Can't find material for flare!");
	      return NULL;
	    }
	    matnode = child->GetNode ("spark5material");
	    halo.flare.mat_spark5 = matnode ? lc->FindMaterial (
	    	matnode->GetContentsValue ()) : NULL;
	    if (!halo.flare.mat_spark5)
	    {
	      ReportError (
		  "crystalspace.maploader.parse.light",
    	          "Can't find material for flare!");
	      return NULL;
	    }
	  }
	  else
	  {
	    ReportError (
		"crystalspace.maploader.parse.light",
    	        "Unknown halo type '%s'. Use 'cross', 'nova' or 'flare'!",
		type);
	    return NULL;
	  }
        }
        break;
      case XMLTOKEN_ATTENUATION:
	{
	  const char* att = child->GetContentsValue ();
          if (!strcasecmp (att, "none")     ) attenuation = CS_ATTN_NONE;
          if (!strcasecmp (att, "linear")   ) attenuation = CS_ATTN_LINEAR;
          if (!strcasecmp (att, "inverse")  ) attenuation = CS_ATTN_INVERSE;
          if (!strcasecmp (att, "realistic")) attenuation = CS_ATTN_REALISTIC;
	  else
	  {
	    TokenError ("attenuation", att);
	    return NULL;
	  }
	}
	break;
      default:
	TokenError ("a light", value);
	return NULL;
    }
  }

  // implicit radius
  if (dist == 0)
  {
    if (color.red > color.green && color.red > color.blue) dist = color.red;
    else if (color.green > color.blue) dist = color.green;
    else dist = color.blue;
    switch (attenuation)
    {
      case CS_ATTN_NONE      : dist = 100000000; break;
      case CS_ATTN_LINEAR    : break;
      case CS_ATTN_INVERSE   : dist = 16.0f * qsqrt (dist); break;
      case CS_ATTN_REALISTIC : dist = 256.0f * dist; break;
    }
  }

  iStatLight* l = Engine->CreateLight (lightname, pos,
  	dist, color, dyn);
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

iKeyValuePair* csLoader::ParseKey (iDocumentNode* node, iObject* pParent)
{
  const char* name = node->GetAttributeValue ("name");
  if (!name)
  {
    ReportError (
		"crystalspace.maploader.parse.key",
    	        "Missing 'name' attribute for 'key'!");
    return NULL;
  }
  const char* value = node->GetAttributeValue ("value");
  if (!value)
  {
    ReportError (
		"crystalspace.maploader.parse.key",
    	        "Missing 'value' attribute for 'key'!");
    return NULL;
  }
  csKeyValuePair* cskvp = new csKeyValuePair (name, value);
  iKeyValuePair* kvp = SCF_QUERY_INTERFACE (cskvp, iKeyValuePair);
  if (pParent)
    pParent->ObjAdd (kvp->QueryObject ());
  kvp->DecRef ();
  return kvp;
}

iMapNode* csLoader::ParseNode (iDocumentNode* node, iSector* sec)
{
  iMapNode* pNode = &(new csMapNode (node->GetAttributeValue ("name")))->scfiMapNode;
  pNode->SetSector (sec);

  csVector3 pos;

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
	ReportError (
		"crystalspace.maploader.parse.node",
        	"'addon' not yet supported in node!");
	return NULL;
      case XMLTOKEN_KEY:
        {
          iKeyValuePair* kvp = ParseKey (child, pNode->QueryObject ());
          if (kvp)
	    kvp->DecRef ();
	  else
	    return NULL;
	}
        break;
      case XMLTOKEN_POSITION:
	if (!SyntaxService->ParseVector (child, pos))
	  return NULL;
        break;
      default:
        TokenError ("a node", value);
	return NULL;
    }
  }

  pNode->SetPosition (pos);

  return pNode;
}

iSector* csLoader::ParseSector (iDocumentNode* node)
{
  const char* secname = node->GetAttributeValue ("name");

  bool do_culler = false;
  const char* bspname = NULL;
  const char* culplugname = NULL;

  iSector* sector = GetLoaderContext ()->FindSector (secname);
  if (sector == NULL)
  {
    sector = Engine->CreateSector (secname);
    Stats->sectors_loaded++;
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
      case XMLTOKEN_ADDON:
	if (!LoadAddOn (child, sector))
	{
	  return NULL;
	}
      	break;
      case XMLTOKEN_CULLER:
	bspname = child->GetContentsValue ();
	if (!bspname)
	{
	  ReportError (
		"crystalspace.maploader.parse.sector",
	  	"CULLER expects the name of a mesh object!");
	  return NULL;
	}
	else
	{
          do_culler = true;
	  culplugname = NULL;
	}
        break;
      case XMLTOKEN_CULLERP:
	culplugname = child->GetContentsValue ();
	if (!culplugname)
	{
	  ReportError (
		"crystalspace.maploader.parse.sector",
	  	"CULLERP expects the name of a visibility culling plugin!");
	  return NULL;
	}
	else
	{
          do_culler = true;
	  bspname = NULL;
	}
        break;
      case XMLTOKEN_MESHREF:
        {
	  const char* meshname = child->GetAttributeValue ("name");
	  if (!meshname)
	  {
      	    ReportError (
	      	"crystalspace.maploader.load.meshobject",
		"'meshref' requires a name in sector '%s'!",
		secname ? secname : "<noname>");
	    return NULL; // @@@ Leak
	  }
          iMeshWrapper* mesh = LoadMeshObjectFromFactory (child);
          if (!mesh)
	  {
      	    ReportError (
	      	"crystalspace.maploader.load.meshobject",
		"Could not load mesh object '%s' in sector '%s'!",
		meshname, secname ? secname : "<noname>");
	    return NULL; // @@@ Leak
	  }
	  mesh->QueryObject ()->SetName (meshname);
	  mesh->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
          mesh->GetMovable ()->SetSector (sector);
	  mesh->GetMovable ()->UpdateMove ();
	  Engine->GetMeshes ()->Add (mesh);
	  mesh->DecRef ();
        }
        break;
      case XMLTOKEN_MESHOBJ:
        {
	  const char* meshname = child->GetAttributeValue ("name");
	  if (!meshname)
	  {
      	    ReportError (
	      	"crystalspace.maploader.load.meshobject",
		"'meshobj' requires a name in sector '%s'!",
		secname ? secname : "<noname>");
	    return NULL; // @@@ Leak
	  }
	  iMeshWrapper* mesh = Engine->CreateMeshWrapper (meshname);
          if (!LoadMeshObject (mesh, child))
	  {
      	    ReportError (
	      	"crystalspace.maploader.load.meshobject",
		"Could not load mesh object '%s' in sector '%s'!",
		meshname, secname ? secname : "<noname>");
	    mesh->DecRef ();
	    return NULL; // @@@ Leak
	  }
	  mesh->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
          mesh->GetMovable ()->SetSector (sector);
	  mesh->GetMovable ()->UpdateMove ();
	  mesh->DecRef ();
        }
        break;
      case XMLTOKEN_MESHLIB:
        {
	  const char* meshname = child->GetAttributeValue ("name");
	  if (!meshname)
	  {
      	    ReportError (
	      	"crystalspace.maploader.load.meshobject",
		"'meshlib' requires a name in sector '%s'!",
		secname ? secname : "<noname>");
	    return NULL; // @@@ Leak
	  }
	  iMeshWrapper* mesh = Engine->GetMeshes ()->FindByName (meshname);
	  if (!mesh)
	  {
      	    ReportError (
	      	"crystalspace.maploader.load.meshobject",
		"Could not find mesh object '%s' in sector '%s' for MESHLIB!",
		meshname, secname ? secname : "<noname>");
	    return NULL;
	  }
	  if (mesh->GetMovable ()->GetSectors ()->GetCount () > 0)
	  {
      	    ReportError (
	      	"crystalspace.maploader.load.meshobject",
		"Mesh '%s' is already in another sector in sector '%s'!",
		meshname, secname ? secname : "<noname>");
	    return NULL;
	  }
          if (!LoadMeshObject (mesh, child))
	  {
      	    ReportError (
	      	"crystalspace.maploader.load.meshobject",
		"Could not load mesh object '%s' in sector '%s'!",
		meshname, secname ? secname : "<noname>");
	    return NULL;
	  }
	  mesh->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
          mesh->GetMovable ()->SetSector (sector);
	  mesh->GetMovable ()->UpdateMove ();
        }
        break;
      case XMLTOKEN_LIGHT:
        {
	  iStatLight* sl = ParseStatlight (child);
	  if (!sl)
	  {
	    return NULL; // @@@ Leak
	  }
          sector->GetLights ()->Add (sl->QueryLight ());
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
	    return NULL; // @@@ Leak
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
          iKeyValuePair* kvp = ParseKey (child, sector->QueryObject());
	  if (kvp)
	    kvp->DecRef ();
	  else
	    return NULL;
        }
        break;
      default:
	TokenError ("a sector", value);
	return NULL;
    }
  }
  if (!(flags & CS_LOADER_NOBSP))
    if (do_culler)
    {
      bool rc;
      if (bspname)
        rc = sector->SetVisibilityCuller (bspname);
      else
        rc = sector->SetVisibilityCullerPlugin (culplugname);
      if (!rc)
      {
	ReportError (
	      	"crystalspace.maploader.load.sector",
		"Could not load visibility culler for sector '%s'!",
		secname ? secname : "<noname>");
	return NULL;
      }
    }
  return sector;
}


//========================================================================
//========================================================================

