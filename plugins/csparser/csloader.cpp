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
#include "ivideo/texture.h"
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
  csLoader* loader;
public:
  StdLoaderContext (iEngine* Engine, bool ResolveOnlyRegion,
    csLoader* loader);
  virtual ~StdLoaderContext ();

  SCF_DECLARE_IBASE;

  virtual iSector* FindSector (const char* name);
  virtual iMaterialWrapper* FindMaterial (const char* name);
  virtual iMeshFactoryWrapper* FindMeshFactory (const char* name);
  virtual iMeshWrapper* FindMeshObject (const char* name);
  virtual iTextureWrapper* FindTexture (const char* name);
};

SCF_IMPLEMENT_IBASE(StdLoaderContext);
  SCF_IMPLEMENTS_INTERFACE(iLoaderContext);
SCF_IMPLEMENT_IBASE_END;

StdLoaderContext::StdLoaderContext (iEngine* Engine,
	bool ResolveOnlyRegion, csLoader* loader)
{
  SCF_CONSTRUCT_IBASE (NULL);
  StdLoaderContext::Engine = Engine;
  if (ResolveOnlyRegion)
    region = Engine->GetCurrentRegion ();
  else
    region = NULL;
  StdLoaderContext::loader = loader;
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
  // @@@ in case the material is not found a replacement is taken.
  // however, somehow the location of the errorneous material name
  // should be reported. 
  iMaterialWrapper* mat = Engine->FindMaterial (name, region);
  if (mat)
    return mat;

  loader->ReportNotify ("Could not find material '%s'. "
    "Creating new material using texture with that name", name);
  iTextureWrapper* tex = FindTexture (name);
  if (tex)
  {
    // Add a default material with the same name as the texture
    csRef<iMaterial> material (Engine->CreateBaseMaterial (tex));
    iMaterialWrapper *mat = Engine->GetMaterialList ()
      	->NewMaterial (material);
    // First we have to extract the optional region name from the name:
    char* n = strchr (name, '/');
    if (!n) n = (char*)name;
    else n++;
    mat->QueryObject()->SetName (n);

    // @@@ should this be done here?
    iTextureManager *tm;
    if ((loader->G3D) && (tm = loader->G3D->GetTextureManager()))
    {
      tex->Register (tm);
      tex->GetTextureHandle()->Prepare();
      mat->Register (tm);
      mat->GetMaterialHandle()->Prepare();
    }
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

iTextureWrapper* StdLoaderContext::FindTexture (const char* name)
{
  // @@@ in case the texture is not found a replacement is taken.
  // however, somehow the location of the errorneous texture name
  // should be reported. 
  iTextureWrapper* result;
  if (region)
    result = region->FindTexture (name);
  else
    result = Engine->GetTextureList ()->FindByName (name);

  if (!result)
  {
    loader->ReportNotify ("Could not find texture '%s'. Attempting to load.", 
      name);
    csRef<iTextureWrapper> rc = loader->LoadTexture (name, name);
    result = rc;
  }
  return result;
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
  ReportNotifyV ("crystalspace.maploader", description, arg);
  va_end (arg);
}

void csLoader::ReportNotifyV (const char* id, const char* description,
	va_list arg)
{
  if (Reporter)
  {
    Reporter->ReportV (CS_REPORTER_SEVERITY_NOTIFY, id,
    	description, arg);
  }
  else
  {
    csPrintf ("%s: ", id);
    csPrintfV (description, arg);
    csPrintf ("\n");
  }
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

  if (Reporter)
  {
    Reporter->ReportV (CS_REPORTER_SEVERITY_WARNING, id, description, arg);
  }
  else
  {
    char buf[1024];
    vsprintf (buf, description, arg);
    csPrintf ("Warning ID: %s\n", id);
    csPrintf ("Description: %s\n", buf);
  }
  va_end (arg);
}

void csLoader::ReportWarning (const char* id, iDocumentNode* node, const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  char buf[1024];
  vsprintf (buf, description, arg);
  va_end (arg);
  SyntaxService->Report (id, CS_REPORTER_SEVERITY_WARNING, node, buf);
}
//---------------------------------------------------------------------------

// XML: temporary code to detect if we have an XML file. If that's
// the case then we will use the XML parsers. Returns false on failure
// to parse XML.
bool csLoader::TestXml (const char* file, iDataBuffer* buf,
	csRef<iDocument>& doc)
{
  const char* b = **buf;
  while (*b == ' ' || *b == '\n' || *b == '\t') b++;
  if (*b == '<')
  {
    // XML.
    // First try to find out if there is an iDocumentSystem registered in the
    // object registry. If that's the case we will use that. Otherwise
    // we'll use tinyxml.
    csRef<iDocumentSystem> xml (
    	CS_QUERY_REGISTRY (object_reg, iDocumentSystem));
    if (!xml) xml = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
    doc = xml->CreateDocument ();
    const char* error = doc->Parse (buf);
    if (error != NULL)
    {
      ReportError (
	      "crystalspace.maploader.parse.xml",
    	      "XML error '%s' for file '%s'!", error, file);
      doc = NULL;
      return false;
    }
  }
  return true;
}

csPtr<iBase> csLoader::TestXmlPlugParse (iLoaderPlugin* plug, iDataBuffer* buf,
  	iBase* context, const char* fname)
{
  csRef<iDocument> doc;
  bool er = TestXml (fname, buf, doc);
  if (!er) return NULL;
  if (doc)
  {
    // First find the <params> node in the loaded file.
    csRef<iDocumentNode> paramsnode = doc->GetRoot ()->GetNode ("params");
    if (!paramsnode)
    {
      SyntaxService->ReportError (
	      "crystalspace.maploader.load.plugin",
              doc->GetRoot (), "Could not find <params> in '%s'!", fname);
      return NULL;
    }
    return plug->Parse (paramsnode, GetLoaderContext (), context);
  }
  else
  {
    ReportError ("crystalspace.maploader", 
	    "Please convert your models to XML using cs2xml (file '%s')!",
	    fname);
    return NULL;
  }
}

//---------------------------------------------------------------------------

bool csLoader::LoadMapFile (const char* file, bool iClearEngine,
  bool iOnlyRegion)
{
  Stats->Init ();
  if (iClearEngine) Engine->DeleteAll ();
  ResolveOnlyRegion = iOnlyRegion;
  ldr_context = NULL;

  csRef<iDataBuffer> buf (VFS->ReadFile (file));

  if (!buf || !buf->GetSize ())
  {
    ReportError (
	      "crystalspace.maploader.parse.map",
    	      "Could not open map file '%s' on VFS!", file);
    return false;
  }

  Engine->ResetWorldSpecificSettings();

  csRef<iDocument> doc;
  bool er = TestXml (file, buf, doc);
  if (!er) return false;
  if (doc)
  {
    if (!LoadMap (doc->GetRoot ())) return false;
  }
  else
  {
    ReportError ("crystalspace.maploader", 
	    "Please convert your map to XML using cs2xml (file '%s')!", file);
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

  return true;
}

//---------------------------------------------------------------------------

bool csLoader::LoadLibraryFile (const char* fname)
{
  csRef<iDataBuffer> buf (VFS->ReadFile (fname));

  if (!buf || !buf->GetSize ())
  {
    ReportError (
	      "crystalspace.maploader.parse.library",
    	      "Could not open library file '%s' on VFS!", fname);
    return false;
  }

  ResolveOnlyRegion = false;
  ldr_context = NULL;

  csRef<iDocument> doc;
  bool er = TestXml (fname, buf, doc);
  if (!er) return false;
  if (doc)
  {
    return LoadLibrary (doc->GetRoot ());
  }
  else
  {
    ReportError ("crystalspace.maploader",
      "Please convert your library to XML using cs2xml (file '%s')!", fname);
    return false;
  }
}

//---------------------------------------------------------------------------

csPtr<iMeshFactoryWrapper> csLoader::LoadMeshObjectFactory (const char* fname)
{
  if (!Engine) return NULL;

  ResolveOnlyRegion = false;
  ldr_context = NULL;

  csRef<iDataBuffer> databuff (VFS->ReadFile (fname));

  if (!databuff || !databuff->GetSize ())
  {
    ReportError (
	      "crystalspace.maploader.parse.meshfactory",
    	      "Could not open mesh object file '%s' on VFS!", fname);
    return NULL;
  }

  csRef<iDocument> doc;
  bool er = TestXml (fname, databuff, doc);
  if (!er) return NULL;
  if (doc)
  {
    csRef<iDocumentNode> meshfactnode = doc->GetRoot ()->GetNode ("meshfact");
    if (!meshfactnode)
    {
      ReportError (
	      "crystalspace.maploader.parse.map",
    	      "File '%s' does not seem to contain a 'meshfact'!", fname);
      return NULL;
    }
    csRef<iMeshFactoryWrapper> t (Engine->CreateMeshFactory (
      	meshfactnode->GetAttributeValue ("name")));
    if (LoadMeshObjectFactory (t, meshfactnode))
    {
      return csPtr<iMeshFactoryWrapper> (t);
    }
    else
    {
      // Error is already reported.
      iMeshFactoryWrapper* factwrap = Engine->GetMeshFactories ()
      	  ->FindByName (meshfactnode->GetAttributeValue ("name"));
      Engine->GetMeshFactories ()->Remove (factwrap);
      return NULL;
    }
  }
  else
  {
    ReportError ("crystalspace.maploader",
      "Please convert your mesh factory to XML using cs2xml (file '%s')!",
      fname);
    return NULL;
  }
  return NULL;
}

//---------------------------------------------------------------------------

csPtr<iMeshWrapper> csLoader::LoadMeshObject (const char* fname)
{
  if (!Engine) return NULL;

  csRef<iDataBuffer> databuff (VFS->ReadFile (fname));
  csRef<iMeshWrapper> mesh;

  if (!databuff || !databuff->GetSize ())
  {
    ReportError (
	      "crystalspace.maploader.parse.meshobject",
    	      "Could not open mesh object file '%s' on VFS!", fname);
    return NULL;
  }

  csRef<iDocument> doc;
  bool er = TestXml (fname, databuff, doc);
  if (!er) return NULL;
  if (doc)
  {
    csRef<iDocumentNode> meshobjnode = doc->GetRoot ()->GetNode ("meshobj");
    if (!meshobjnode)
    {
      ReportError (
	      "crystalspace.maploader.parse.map",
    	      "File '%s' does not seem to contain a 'meshobj'!", fname);
      return NULL;
    }
    mesh = Engine->CreateMeshWrapper (
    	meshobjnode->GetAttributeValue ("name"));
    if (!LoadMeshObject (mesh, meshobjnode))
    {
      // Error is already reported.
      mesh = NULL;
    }
  }
  else
  {
    ReportError ("crystalspace.maploader",
	    "Please convert your mesh object to XML using cs2xml (file '%s')!",
	    fname);
    return NULL;
  }
  return csPtr<iMeshWrapper> (mesh);
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

  object_reg = NULL;

  flags = 0;
  ResolveOnlyRegion = false;
  Stats = new csLoaderStats ();
}

csLoader::~csLoader()
{
  loaded_plugins.DeleteAll ();
  delete Stats;
}

iLoaderContext* csLoader::GetLoaderContext ()
{
  if (!ldr_context)
  {
    ldr_context = csPtr<iLoaderContext> (
    	new StdLoaderContext (Engine, ResolveOnlyRegion, this));
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
  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));

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

  xmltokens.Register ("alphamodifier1", XMLTOKEN_ALPHAMODIFIER1);
  xmltokens.Register ("alphamodifier2", XMLTOKEN_ALPHAMODIFIER2);
  xmltokens.Register ("alphamodifier3", XMLTOKEN_ALPHAMODIFIER3);
  xmltokens.Register ("alphaoperation", XMLTOKEN_ALPHAOPERATION);
  xmltokens.Register ("alphasource1", XMLTOKEN_ALPHASOURCE1);
  xmltokens.Register ("alphasource2", XMLTOKEN_ALPHASOURCE2);
  xmltokens.Register ("alphasource3", XMLTOKEN_ALPHASOURCE3);
  xmltokens.Register ("blending", XMLTOKEN_BLENDING);
  xmltokens.Register ("colormodifier1", XMLTOKEN_COLORMODIFIER1);
  xmltokens.Register ("colormodifier2", XMLTOKEN_COLORMODIFIER2);
  xmltokens.Register ("colormodifier3", XMLTOKEN_COLORMODIFIER3);
  xmltokens.Register ("coloroperation", XMLTOKEN_COLOROPERATION);
  xmltokens.Register ("colorsource1", XMLTOKEN_COLORSOURCE1);
  xmltokens.Register ("colorsource2", XMLTOKEN_COLORSOURCE2);
  xmltokens.Register ("colorsource3", XMLTOKEN_COLORSOURCE3);
  xmltokens.Register ("destinationblend", XMLTOKEN_DESTINATIONBLEND);
  xmltokens.Register ("effect", XMLTOKEN_EFFECT);
  xmltokens.Register ("effects", XMLTOKEN_EFFECTS);
  xmltokens.Register ("pass", XMLTOKEN_PASS);
  xmltokens.Register ("quality", XMLTOKEN_QUALITY);
  xmltokens.Register ("shading", XMLTOKEN_SHADING);
  xmltokens.Register ("sourceblend", XMLTOKEN_SOURCEBLEND);
  xmltokens.Register ("technique", XMLTOKEN_TECHNIQUE);
  xmltokens.Register ("texturesource", XMLTOKEN_TEXTURESOURCE);
  xmltokens.Register ("texturecoordinatesource", XMLTOKEN_TEXTURECOORDSOURCE);
  xmltokens.Register ("variable", XMLTOKEN_VARIABLE);
  xmltokens.Register ("vertexcolor", XMLTOKEN_VERTEXCOLOR);
  xmltokens.Register ("vertexprogram", XMLTOKEN_VERTEXPROGRAM);
  xmltokens.Register ("vertexprogramconstant", XMLTOKEN_VERTEXPROGRAMCONST);

  xmltokens.Register ("runsequence", XMLTOKEN_RUNSEQUENCE);
  xmltokens.Register ("sequence", XMLTOKEN_SEQUENCE);
  xmltokens.Register ("sequences", XMLTOKEN_SEQUENCES);
  xmltokens.Register ("trigger", XMLTOKEN_TRIGGER);
  xmltokens.Register ("triggers", XMLTOKEN_TRIGGERS);
  xmltokens.Register ("testtrigger", XMLTOKEN_TESTTRIGGER);
  xmltokens.Register ("checktrigger", XMLTOKEN_CHECKTRIGGER);
  xmltokens.Register ("setfog", XMLTOKEN_SETFOG);
  xmltokens.Register ("fadefog", XMLTOKEN_FADEFOG);
  xmltokens.Register ("setlight", XMLTOKEN_SETLIGHT);
  xmltokens.Register ("fadelight", XMLTOKEN_FADELIGHT);
  xmltokens.Register ("rotate", XMLTOKEN_ROTATE);
  xmltokens.Register ("rotx", XMLTOKEN_ROTX);
  xmltokens.Register ("roty", XMLTOKEN_ROTY);
  xmltokens.Register ("rotz", XMLTOKEN_ROTZ);
  xmltokens.Register ("enabletrigger", XMLTOKEN_ENABLETRIGGER);
  xmltokens.Register ("disabletrigger", XMLTOKEN_DISABLETRIGGER);
  xmltokens.Register ("delay", XMLTOKEN_DELAY);
  xmltokens.Register ("fire", XMLTOKEN_FIRE);
  xmltokens.Register ("sectorvis", XMLTOKEN_SECTORVIS);
  return true;
}

void csLoader::SetMode (int iFlags)
{
  flags = iFlags;
}

//--- Parsing of Engine Objects ---------------------------------------------

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
            csRef<iMeshFactoryWrapper> t (Engine->CreateMeshFactory (name));
	    if (!t || !LoadMeshObjectFactory (t, child))
	    {
	      // Error is already reported.
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
	case XMLTOKEN_SEQUENCES:
	  if (!LoadSequences (child))
	    return false;
	  break;
	case XMLTOKEN_TRIGGERS:
	  if (!LoadTriggers (child))
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
        case XMLTOKEN_EFFECTS:
          LoadEffectFile(child->GetContentsValue());
          break;
	default:
	  SyntaxService->ReportBadToken (child);
	  return false;
      }
    }
  }
  else
  {
    SyntaxService->ReportError (
      "crystalspace.maploader.parse.expectedworld",
      node, "Expected 'world' token!");
    return false;
  }

  return true;
}

bool csLoader::LoadLibrary (iDocumentNode* node)
{
  if (!Engine)
  {
    SyntaxService->ReportError (
	  "crystalspace.maploader.parse.noengine",
	  node, "No engine present while in LoadLibrary!");
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
	case XMLTOKEN_SEQUENCES:
	  if (!LoadSequences (child))
	    return false;
	  break;
	case XMLTOKEN_TRIGGERS:
	  if (!LoadTriggers (child))
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
	      // Error is already reported.
	      return false;
	    }
	    mesh->QueryObject ()->SetName (child->GetAttributeValue ("name"));
	    Engine->GetMeshes ()->Add (mesh);
	    //mesh->DecRef ();
          }
          break;
        case XMLTOKEN_MESHOBJ:
          {
	    csRef<iMeshWrapper> mesh (Engine->CreateMeshWrapper (
			    child->GetAttributeValue ("name")));
            if (!LoadMeshObject (mesh, child))
	    {
	      // Error is already reported.
	      return false;
	    }
          }
          break;
        case XMLTOKEN_MESHFACT:
          {
            csRef<iMeshFactoryWrapper> t (Engine->CreateMeshFactory (
			    child->GetAttributeValue ("name")));
	    if (t)
	    {
	      if (!LoadMeshObjectFactory (t, child))
	      {
	        // Error is already reported.
		return false;
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
  }
  else
  {
    SyntaxService->ReportError (
      "crystalspace.maploader.parse.expectedlib",
      node, "Expected 'library' token!");
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
	SyntaxService->ReportBadToken (child);
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
  iBinaryLoaderPlugin* binplug = NULL;
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
      case XMLTOKEN_ADDON:
	if (!LoadAddOn (child, stemp))
	  return false;
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
	  // is a new factory this will be NULL. Otherwise it is possible
	  // to append information to the already loaded factory.
	  csRef<iBase> mof (plug->Parse (child, GetLoaderContext (),
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
          csRef<iDataBuffer> buf (VFS->ReadFile (child->GetContentsValue ()));
	  if (!buf)
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.loadingfile",
	      child, "Error opening file '%s'!", child->GetContentsValue ());
	    return false;
	  }

	  // We give here the iMeshObjectFactory as the context. If this
	  // is a new factory this will be NULL. Otherwise it is possible
	  // to append information to the already loaded factory.
	  csRef<iBase> mof;
	  if (plug)
	    mof = TestXmlPlugParse (plug, buf, stemp->GetMeshObjectFactory (),
	    	child->GetContentsValue ());
	  else
	    mof = binplug->Parse ((void*)(buf->GetUint8 ()),
	  	GetLoaderContext (), stemp->GetMeshObjectFactory ());
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
          mat = GetLoaderContext ()->FindMaterial (matname);
          if (mat)
	  {
	    csRef<iSprite3DFactoryState> state (SCF_QUERY_INTERFACE (
	    	stemp->GetMeshObjectFactory (),
		iSprite3DFactoryState));
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
	if (!loaded_plugins.FindPlugin (child->GetContentsValue (),
		plug, binplug))
	{
	  SyntaxService->ReportError (
 	      "crystalspace.maploader.parse.addon",
	      child, "Error loading plugin '%s'!", child->GetContentsValue ());
	  return false;
	}
        break;

      case XMLTOKEN_MESHFACT:
        {
          csRef<iMeshFactoryWrapper> t (Engine->CreateMeshFactory (
	  	child->GetAttributeValue ("name")));
	  csReversibleTransform child_transf;
          if (!LoadMeshObjectFactory (t, child, &child_transf))
	  {
	    // Error is already reported above.
	    return false;
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
	SyntaxService->ReportBadToken (child);
        return false;
    }
  }

  return true;
}

iMeshWrapper* csLoader::LoadMeshObjectFromFactory (iDocumentNode* node)
{
  if (!Engine) return NULL;

  const char* priority = '\0';

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
	    SyntaxService->ReportError (
	  	  "crystalspace.maploader.load.meshobject",
	  	  child, "First specify the parent factory with 'factory'!");
	    return NULL;
	  }
	  if (!mesh->GetMeshObject ())
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.meshobject",
              child, "Mesh object is missing!");
	    return NULL;
	  }
	  csRef<iLODControl> lodctrl (SCF_QUERY_INTERFACE (
	    	mesh->GetMeshObject (),
		iLODControl));
	  if (!lodctrl)
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.meshobject",
              child, "This mesh doesn't implement LOD control!");
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
	  SyntaxService->ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	child, "First specify the parent factory with 'factory'!");
	  return NULL;
	}
	if (!LoadAddOn (child, mesh))
	  return NULL;
      	break;
      case XMLTOKEN_NOLIGHTING:
        if (!mesh)
	{
	  SyntaxService->ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	child, "First specify the parent factory with 'factory'!");
	  return NULL;
	}
        mesh->GetFlags().Set (CS_ENTITY_NOLIGHTING);
        break;
      case XMLTOKEN_NOSHADOWS:
        if (!mesh)
	{
	  SyntaxService->ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	child, "First specify the parent factory with 'factory'!");
	  return NULL;
	}
        mesh->GetFlags().Set (CS_ENTITY_NOSHADOWS);
        break;
      case XMLTOKEN_INVISIBLE:
        if (!mesh)
	{
	  SyntaxService->ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	child, "First specify the parent factory with 'factory'!");
	  return NULL;
	}
        mesh->GetFlags().Set (CS_ENTITY_INVISIBLE);
        break;
      case XMLTOKEN_DETAIL:
        if (!mesh)
	{
	  SyntaxService->ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	child, "First specify the parent factory with 'factory'!");
	  return NULL;
	}
        mesh->GetFlags().Set (CS_ENTITY_DETAIL);
        break;
      case XMLTOKEN_ZFILL:
        if (!mesh)
	{
	  SyntaxService->ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	child, "First specify the parent factory with 'factory'!");
	  return NULL;
	}
        if (!priority) priority = "wall";
        mesh->SetZBufMode (CS_ZBUF_FILL);
        break;
      case XMLTOKEN_ZUSE:
        if (!mesh)
	{
	  SyntaxService->ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	child, "First specify the parent factory with 'factory'!");
	  return NULL;
	}
        if (!priority) priority = "object";
        mesh->SetZBufMode (CS_ZBUF_USE);
        break;
      case XMLTOKEN_ZNONE:
        if (!mesh)
	{
	  SyntaxService->ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	child, "First specify the parent factory with 'factory'!");
	  return NULL;
	}
        if (!priority) priority = "sky";
        mesh->SetZBufMode (CS_ZBUF_NONE);
        break;
      case XMLTOKEN_ZTEST:
        if (!mesh)
	{
	  SyntaxService->ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	child, "First specify the parent factory with 'factory'!");
	  return NULL;
	}
        if (!priority) priority = "alpha";
        mesh->SetZBufMode (CS_ZBUF_TEST);
        break;
      case XMLTOKEN_CAMERA:
        if (!mesh)
	{
	  SyntaxService->ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	child, "First specify the parent factory with 'factory'!");
	  return NULL;
	}
        if (!priority) priority = "sky";
        mesh->GetFlags().Set (CS_ENTITY_CAMERA);
        break;
      case XMLTOKEN_CONVEX:
        if (!mesh)
	{
	  SyntaxService->ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	child, "First specify the parent factory with 'factory'!");
	  return NULL;
	}
        mesh->GetFlags().Set (CS_ENTITY_CONVEX);
        break;
      case XMLTOKEN_KEY:
        if (!mesh)
	{
	  SyntaxService->ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	child, "First specify the parent factory with 'factory'!");
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
	  SyntaxService->ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	child, "First specify the parent factory with 'factory'!");
	  return NULL;
	}
	else if (!mesh->GetMeshObject ()->SupportsHardTransform ())
	{
          SyntaxService->ReportError (
	    "crystalspace.maploader.parse.meshobject",
            child, "This mesh object doesn't support 'hardmove'!");
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
	  SyntaxService->ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	child, "First specify the parent factory with 'factory'!");
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
	  SyntaxService->ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	child, "There is already a factory for this mesh!");
	  return NULL;
	}
	else
	{
          iMeshFactoryWrapper* t = Engine->GetMeshFactories ()
	  	->FindByName (child->GetContentsValue ());
          if (!t)
	  {
	    SyntaxService->ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	child, "Can't find factory '%s'!", child->GetContentsValue ());
	    return NULL;
	  }
	  mesh = t->CreateMeshWrapper ();
	}
        break;
      default:
	SyntaxService->ReportBadToken (child);
	return NULL;
    }
  }

  if (!mesh)
  {
    SyntaxService->ReportError (
	  	"crystalspace.maploader.load.meshobject",
	  	node, "There is no 'factory' for this mesh!");
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
  iBinaryLoaderPlugin* binplug = NULL;

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
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.meshobject",
	      child, "Only use 'lod' after 'params'!");
	    return false;
	  }
	  csRef<iLODControl> lodctrl (
	  	SCF_QUERY_INTERFACE (mesh->GetMeshObject (), iLODControl));
	  if (!lodctrl)
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.meshobject",
              child, "This mesh doesn't implement LOD control!");
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
	    // Error is already reported.
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
	  csRef<iMeshWrapper> sp (Engine->CreateMeshWrapper (
			  child->GetAttributeValue ("name")));
          if (!LoadMeshObject (sp, child))
	  {
	    // Error is already reported.
	    return false;
	  }
	  sp->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
          mesh->GetChildren ()->Add (sp);
        }
        break;
      case XMLTOKEN_HARDMOVE:
        {
	  if (!mesh->GetMeshObject ()->SupportsHardTransform ())
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.meshobject",
              child, "This mesh object doesn't support 'hardmove'!");
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
          SyntaxService->ReportError (
	      "crystalspace.maploader.load.plugin",
              child, "Could not load plugin!");
	  return false;
	}
	else
	{
	  csRef<iBase> mo (plug->Parse (child, GetLoaderContext (), NULL));
          if (mo)
          {
	    csRef<iMeshObject> mo2 (SCF_QUERY_INTERFACE (mo, iMeshObject));
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
	      csRef<iMeshFactoryWrapper> mfw (SCF_QUERY_INTERFACE (lp,
	      	iMeshFactoryWrapper));
	      if (mfw)
	        mesh->SetFactory (mfw);
	    }
          }
          else
          {
	    // Error is reported by plug->Parse().
	    return false;
          }
	}
        break;
      case XMLTOKEN_PARAMSFILE:
	if (!plug && !binplug)
	{
          SyntaxService->ReportError (
	      "crystalspace.maploader.load.plugin",
              child, "Could not load plugin for mesh '%s'!",
	      mesh->QueryObject ()->GetName ());
	  return false;
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
          csRef<iDataBuffer> buf (VFS->ReadFile (fname));
	  if (!buf)
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.loadingfile",
	      child, "Error opening file '%s'!", fname);
	    return false;
	  }
	  csRef<iBase> mo;
	  if (plug)
	    mo = TestXmlPlugParse (plug, buf, NULL, fname);
	  else
	    mo = binplug->Parse ((void*)(buf->GetUint8 ()),
	  	GetLoaderContext (), NULL);
          if (mo)
          {
	    csRef<iMeshObject> mo2 (SCF_QUERY_INTERFACE (mo, iMeshObject));
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
	      csRef<iMeshFactoryWrapper> mfw (SCF_QUERY_INTERFACE (lp,
	      	iMeshFactoryWrapper));
	      if (mfw)
	        mesh->SetFactory (mfw);
	    }
          }
          else
          {
	    // Error is reported by plug->Parse ().
	    return false;
          }

        }
        break;

      case XMLTOKEN_PLUGIN:
	{
	  const char* plugname = child->GetContentsValue ();
	  if (!plugname)
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.plugin",
	      child, "Specify a plugin name with 'plugin'!");
	    return false;
	  }
	  if (!loaded_plugins.FindPlugin (plugname, plug, binplug))
	  {
	    SyntaxService->ReportError (
 	        "crystalspace.maploader.parse.addon",
	        child, "Error loading plugin '%s'!", plugname);
	    return false;
	  }
	}
        break;

      case XMLTOKEN_LMCACHE:
        {
	  if (!mesh->GetMeshObject ())
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.meshobject",
	      child, "Only use 'lmcache' after 'params'!");
	    return false;
	  }
	  csRef<iLightingInfo> li (SCF_QUERY_INTERFACE (mesh->GetMeshObject (),
	  	iLightingInfo));
	  if (li)
	    li->SetCacheName (child->GetContentsValue ());
	}
        break;
      default:
	SyntaxService->ReportBadToken (child);
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
  iBinaryLoaderPlugin* binplug = NULL;

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
          SyntaxService->ReportError (
	      "crystalspace.maploader.load.plugin",
              child, "Could not load plugin!");
	  return false;
	}
	else
	{
	  csRef<iBase> rc (plug->Parse (child, GetLoaderContext (), context));
	  if (!rc) return false;
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
	  const char* fname = child->GetContentsValue ();
	  if (!fname)
	  {
            SyntaxService->ReportError (
	      "crystalspace.maploader.parse.loadingfile",
	      child, "Specify a VFS filename with 'paramsfile'!");
	    return false;
	  }
          csRef<iDataBuffer> buf (VFS->ReadFile (fname));
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
	    csRef<iBase> ret (TestXmlPlugParse (plug, buf, NULL, fname));
	    rc = (ret != NULL);
	  }
	  else
	  {
	    csRef<iBase> ret (binplug->Parse ((void*)(buf->GetUint8 ()),
	  	GetLoaderContext (), NULL));
	    rc = (ret != NULL);
	  }
	  if (!rc)
	    return false;
	}
        break;

      case XMLTOKEN_PLUGIN:
	if (!loaded_plugins.FindPlugin (child->GetContentsValue (),
		plug, binplug))
	{
	  SyntaxService->ReportError (
 	      "crystalspace.maploader.parse.addon",
	      child, "Error loading plugin '%s'!", child->GetContentsValue ());
	  return false;
	}
        break;
      default:
	SyntaxService->ReportBadToken (child);
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
	SyntaxService->ReportError (
		"crystalspace.maploader.parse.collection",
         	child, "'addon' not yet supported in collection!");
	return NULL;
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
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.collection",
            	child, "Light '%s' not found!", lightname);
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
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.collection",
            	child, "Sector '%s' not found!", sectname);
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
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.collection",
            	child, "Collection '%s' not found!", colname);
	    return NULL;
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
	      SyntaxService->ReportError (
		  "crystalspace.maploader.parse.light",
    	          child, "Can't find material for flare!");
	      return NULL;
	    }
	    matnode = child->GetNode ("spark1material");
	    halo.flare.mat_spark1 = matnode ? lc->FindMaterial (
	    	matnode->GetContentsValue ()) : NULL;
	    if (!halo.flare.mat_spark1)
	    {
	      SyntaxService->ReportError (
		  "crystalspace.maploader.parse.light",
    	          child, "Can't find material for flare!");
	      return NULL;
	    }
	    matnode = child->GetNode ("spark2material");
	    halo.flare.mat_spark2 = matnode ? lc->FindMaterial (
	    	matnode->GetContentsValue ()) : NULL;
	    if (!halo.flare.mat_spark2)
	    {
	      SyntaxService->ReportError (
		  "crystalspace.maploader.parse.light",
    	          child, "Can't find material for flare!");
	      return NULL;
	    }
	    matnode = child->GetNode ("spark3material");
	    halo.flare.mat_spark3 = matnode ? lc->FindMaterial (
	    	matnode->GetContentsValue ()) : NULL;
	    if (!halo.flare.mat_spark3)
	    {
	      SyntaxService->ReportError (
		  "crystalspace.maploader.parse.light",
    	          child, "Can't find material for flare!");
	      return NULL;
	    }
	    matnode = child->GetNode ("spark4material");
	    halo.flare.mat_spark4 = matnode ? lc->FindMaterial (
	    	matnode->GetContentsValue ()) : NULL;
	    if (!halo.flare.mat_spark4)
	    {
	      SyntaxService->ReportError (
		  "crystalspace.maploader.parse.light",
    	          child, "Can't find material for flare!");
	      return NULL;
	    }
	    matnode = child->GetNode ("spark5material");
	    halo.flare.mat_spark5 = matnode ? lc->FindMaterial (
	    	matnode->GetContentsValue ()) : NULL;
	    if (!halo.flare.mat_spark5)
	    {
	      SyntaxService->ReportError (
		  "crystalspace.maploader.parse.light",
    	          child, "Can't find material for flare!");
	      return NULL;
	    }
	  }
	  else
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.light",
    	        child,
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
          else if (!strcasecmp (att, "linear")   ) attenuation = CS_ATTN_LINEAR;
          else if (!strcasecmp (att, "inverse")  ) attenuation = CS_ATTN_INVERSE;
          else if (!strcasecmp (att, "realistic")) attenuation = CS_ATTN_REALISTIC;
	  else
	  {
	    SyntaxService->ReportBadToken (child);
	    return NULL;
	  }
	}
	break;
      default:
	SyntaxService->ReportBadToken (child);
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

  csRef<iStatLight> l (Engine->CreateLight (lightname, pos,
  	dist, color, dyn));
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

  l->IncRef ();	// To make sure smart pointer doesn't release.
  return l;
}

iKeyValuePair* csLoader::ParseKey (iDocumentNode* node, iObject* pParent)
{
  const char* name = node->GetAttributeValue ("name");
  if (!name)
  {
    SyntaxService->ReportError (
		"crystalspace.maploader.parse.key",
    	        node, "Missing 'name' attribute for 'key'!");
    return NULL;
  }
  const char* value = node->GetAttributeValue ("value");
  if (!value)
  {
    SyntaxService->ReportError (
		"crystalspace.maploader.parse.key",
    	        node, "Missing 'value' attribute for 'key'!");
    return NULL;
  }
  csKeyValuePair* cskvp = new csKeyValuePair (name, value);
  csRef<iKeyValuePair> kvp (SCF_QUERY_INTERFACE (cskvp, iKeyValuePair));
  if (pParent)
    pParent->ObjAdd (kvp->QueryObject ());
  return kvp;
}

iMapNode* csLoader::ParseNode (iDocumentNode* node, iSector* sec)
{
  iMapNode* pNode = &(new csMapNode (
  	node->GetAttributeValue ("name")))->scfiMapNode;
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
	SyntaxService->ReportError (
		"crystalspace.maploader.parse.node",
        	child, "'addon' not yet supported in node!");
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
	SyntaxService->ReportBadToken (child);
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
	  SyntaxService->ReportError (
		"crystalspace.maploader.parse.sector",
	  	child, "CULLER expects the name of a mesh object!");
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
	  SyntaxService->ReportError (
		"crystalspace.maploader.parse.sector",
	  	child,
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
      	    SyntaxService->ReportError (
	      	"crystalspace.maploader.load.meshobject",
		child, "'meshref' requires a name in sector '%s'!",
		secname ? secname : "<noname>");
	    return NULL; // @@@ Leak
	  }
          iMeshWrapper* mesh = LoadMeshObjectFromFactory (child);
          if (!mesh)
	  {
	    // Error is already reported.
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
      	    SyntaxService->ReportError (
	      	"crystalspace.maploader.load.meshobject",
		child, "'meshobj' requires a name in sector '%s'!",
		secname ? secname : "<noname>");
	    return NULL; // @@@ Leak
	  }
	  csRef<iMeshWrapper> mesh (Engine->CreateMeshWrapper (meshname));
          if (!LoadMeshObject (mesh, child))
	  {
	    // Error is already reported.
	    return NULL; // @@@ Leak
	  }
	  mesh->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
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
		child, "'meshlib' requires a name in sector '%s'!",
		secname ? secname : "<noname>");
	    return NULL; // @@@ Leak
	  }
	  iMeshWrapper* mesh = Engine->GetMeshes ()->FindByName (meshname);
	  if (!mesh)
	  {
      	    SyntaxService->ReportError (
	      	"crystalspace.maploader.load.meshobject",
		child, "Could not find mesh object '%s' in sector '%s' for MESHLIB!",
		meshname, secname ? secname : "<noname>");
	    return NULL;
	  }
	  if (mesh->GetMovable ()->GetSectors ()->GetCount () > 0)
	  {
      	    SyntaxService->ReportError (
	      	"crystalspace.maploader.load.meshobject",
		child, "Mesh '%s' is already in another sector in sector '%s'!",
		meshname, secname ? secname : "<noname>");
	    return NULL;
	  }
          if (!LoadMeshObject (mesh, child))
	  {
	    // Error is already reported.
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
	SyntaxService->ReportBadToken (child);
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
	SyntaxService->ReportError (
	      	"crystalspace.maploader.load.sector",
		node, "Could not load visibility culler for sector '%s'!",
		secname ? secname : "<noname>");
	return NULL;
      }
    }
  return sector;
}

iEngineSequenceManager* csLoader::GetEngineSequenceManager ()
{
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
      return NULL;
    }
  }
  return eseqmgr;
}

//========================================================================
//========================================================================

