/*
    Copyright (C) 2002 by Richard Uren <richard@starport.net>

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
#include "qint.h"

#include "csutil/cfgfile.h"
#include "csutil/scanstr.h"
#include "csutil/plugldr.h"
#include "csutil/xmltiny.h"

#include "cstool/gentrtex.h"
#include "cstool/keyval.h"
#include "cstool/sndwrap.h"
#include "cstool/mdltool.h"
#include "csgfx/csimage.h"

#include "iutil/databuff.h"
#include "imap/reader.h"
#include "imap/services.h"
#include "imap/ldrctxt.h"

#include "imesh/lighting.h"
#include "imesh/sprite3d.h"
#include "imesh/object.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/movable.h"
#include "iengine/halo.h"
#include "iengine/campos.h"
#include "iengine/light.h"
#include "iengine/mesh.h"
#include "ivaria/keyval.h"
#include "ivideo/material.h"
#include "igraphic/imageio.h"
#include "isound/loader.h"
#include "isound/renderer.h"
#include "iutil/vfs.h"
#include "iutil/plugin.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"

#include "ivaria/reporter.h"
#include "ivaria/iso.h"
#include "ivaria/isoldr.h"

#include "isoload.h"

// ---------- Plugin Stuff -------------

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE(csIsoLoader);
  SCF_IMPLEMENTS_INTERFACE(iIsoLoader);
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent);
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (csIsoLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY(csIsoLoader);


enum
{
  XMLTOKEN_WORLD = 1,
  XMLTOKEN_GRID,
  XMLTOKEN_GRIDS,
  XMLTOKEN_SPRITE,
  XMLTOKEN_HEIGHTMAP,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_MATERIALS,
  XMLTOKEN_SIZE,
  XMLTOKEN_SPACE,
  XMLTOKEN_MULT,
  XMLTOKEN_LIGHT,
  XMLTOKEN_ATTENUATION,
  XMLTOKEN_POSITION,
  XMLTOKEN_RADIUS,
  XMLTOKEN_COLOR,
  XMLTOKEN_TILE2D,
  XMLTOKEN_START,
  XMLTOKEN_END,
  XMLTOKEN_KEY,
  XMLTOKEN_STYLE,
  XMLTOKEN_PLUGINS,
  XMLTOKEN_PLUGIN,
  XMLTOKEN_MESHFACT,
  XMLTOKEN_FACTORY,
  XMLTOKEN_FILE,
  XMLTOKEN_PARAMS,
  XMLTOKEN_MOVE,
  XMLTOKEN_MESHOBJ,
  XMLTOKEN_ZFILL,
  XMLTOKEN_ZNONE,
  XMLTOKEN_ZUSE,
  XMLTOKEN_ZTEST,
  XMLTOKEN_PRIORITY,
  XMLTOKEN_MATRIX,
  XMLTOKEN_V
};

/*
 * Context class for the standard loader.
 * Ahhh the things we do for compatibility :-)
 */
class StdIsoLoaderContext : public iLoaderContext
{
private:
  iIsoEngine* Engine;

public:
  StdIsoLoaderContext (iIsoEngine* Engine);
  virtual ~StdIsoLoaderContext ();

  SCF_DECLARE_IBASE;

  virtual iSector* FindSector (const char* name);
  virtual iMaterialWrapper* FindMaterial (const char* name);
  virtual iMaterialWrapper* FindNamedMaterial (const char* name,const char* filename);
  virtual iMeshFactoryWrapper* FindMeshFactory (const char* name);
  virtual iMeshWrapper* FindMeshObject (const char* name);
  virtual iTextureWrapper* FindTexture (const char* name);
  virtual iTextureWrapper* FindNamedTexture (const char* name,const char* filename);
  virtual iLight *FindLight (const char* name);
  virtual bool CheckDupes () const { return false; }
  virtual iRegion* GetRegion () const { return 0; }
  virtual bool CurrentRegionOnly () const { return false; }
};

SCF_IMPLEMENT_IBASE(StdIsoLoaderContext);
  SCF_IMPLEMENTS_INTERFACE(iLoaderContext);
SCF_IMPLEMENT_IBASE_END;

StdIsoLoaderContext::StdIsoLoaderContext (iIsoEngine* Engine)
{
  SCF_CONSTRUCT_IBASE (0);
  StdIsoLoaderContext::Engine = Engine;
}

StdIsoLoaderContext::~StdIsoLoaderContext ()
{
  SCF_DESTRUCT_IBASE();
}

iSector* StdIsoLoaderContext::FindSector (const char* /*name*/)
{
  return 0;
}

iMaterialWrapper* StdIsoLoaderContext::FindMaterial (const char* name)
{
  return Engine->GetMaterialList ()->FindByName (name);
}

iMaterialWrapper* StdIsoLoaderContext::FindNamedMaterial (const char* name,
                                                          const char* filename)
{
  return Engine->GetMaterialList ()->FindByName (name);
}

iMeshFactoryWrapper* StdIsoLoaderContext::FindMeshFactory (const char* name)
{
  return Engine->GetMeshFactories ()->FindByName(name);
}

iMeshWrapper* StdIsoLoaderContext::FindMeshObject (const char* /*name*/)
{
  return 0;
}

iTextureWrapper* StdIsoLoaderContext::FindTexture (const char* /*name*/)
{
  return 0;
}

iTextureWrapper* StdIsoLoaderContext::FindNamedTexture (const char* /*name*/,
                                                        const char* /*filename*/ )
{
  return 0;
}

iLight* StdIsoLoaderContext::FindLight(const char *name)
{
  // TODO: Implement this.
  return 0;
}

//---------------------------------------------------------------------------



//-- Begin - Helpers ----------------------------------------------
// CnP from csparser.cpp
iLoaderContext* csIsoLoader::GetLoaderContext ()
{
  if (!ldr_context)
    ldr_context = csPtr<iLoaderContext> (
    	new StdIsoLoaderContext (Engine));
  return ldr_context;
}

void csIsoLoader::ReportError (const char* id, const char* description, ...)
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

void csIsoLoader::ReportNotify (const char* description, ...)
{
  va_list arg;
  va_start (arg, description);

  if (Reporter)
  {
    Reporter->ReportV (CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.iso.loader",
    	description, arg);
  }
  else
  {
    csPrintfV (description, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

iMaterialWrapper *csIsoLoader::FindMaterial (const char *iName)
{
  iMaterialWrapper* mat;

  mat = Engine->GetMaterialList ()->FindByName (iName);
  if (mat)
    return mat;

  ReportError ("crystalspace.iso.loader.findmaterial",
    "Could not find material named '%s'!", iName);
  return 0;
}

iMeshFactoryWrapper *csIsoLoader::FindMeshFactory (const char *iName)
{
  iMeshFactoryWrapper* mfw;

  mfw = Engine->GetMeshFactories ()->FindByName(iName);
  if (mfw)
    return mfw;

  ReportError("crystalspace.iso.loader.findmeshfactory",
    "Could not find mesh factory named '%s'!",iName);
  return 0;
}

//-- End - Helpers -----------------------------------------------

bool csIsoLoader::TestXml (const char* file, iDataBuffer* buf,
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
    if (error != 0)
    {
      ReportError (
	      "crystalspace.maploader.parse.xml",
    	      "XML error '%s' for file '%s'!", error, file);
      doc = 0;
      return false;
    }
  }
  return true;
}

bool csIsoLoader::LoadMapFile (const char* file)
{
  csRef<iDataBuffer> buf (VFS->ReadFile (file));

  if (!buf || !buf->GetSize ())
  {
      ReportError ("crystalspace.iso.loader.loadmapfile.nomap",
    	"Could not open map file '%s' on VFS!", file);
    return false;
  }

  //  iConfigFile *cfg = new csConfigFile ("map.cfg", VFS);

  csRef<iDocument> doc;
  bool er = TestXml (file, buf, doc);
  if (!er) return false;
  if (doc)
  {
    if (!LoadMap (doc->GetRoot ())) return false;
  }
  else
  {
    return false;
  }

  return true;
}

bool csIsoLoader::LoadMap (iDocumentNode* node)
{
  if (!Engine) return false;

  char *tag = "crystalspace.iso.loader.loadmap";
  bool set_view = false;

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
        case XMLTOKEN_GRIDS:
          if (!ParseGridList (child, child->GetAttributeValue ("name")))
            return false;
          break;

        case XMLTOKEN_MATERIALS:
          if (!ParseMaterialList (child, child->GetAttributeValue ("name")))
            return false;
          break;

        case XMLTOKEN_PLUGINS:
          if (!ParsePluginList (child, child->GetAttributeValue ("name")))
            return false;
          break;

        case XMLTOKEN_MESHFACT:
          if(!ParseMeshFactory (child, child->GetAttributeValue ("name")))
            return false;
          break;

        case XMLTOKEN_START:
          if(!ParseStart (child, child->GetAttributeValue ("name")))
            return false;
          else
            set_view = true;
          break;

	default:
	  ReportError (tag, "Bad token <%s>!", value);
	  return false;
      }
    }
  }

  // before wrapping up see if we need to set the view 
  // only set the view if a START tag was given
  if (set_view == true)
  {
    if (!world->FindGrid(start_v))
    {
      ReportError (tag,"START POSITION outside world space - bye!");
      return false;
    }
    view = csPtr<iIsoView> (Engine->CreateView(world));
    view->SetScroll (start_v, 
      csVector2(G3D->GetWidth()/2, G3D->GetHeight()/2));
 
    // Register the view so it can be retreived later
    if (!object_reg->Register(view,"iIsoView"))
    {
      ReportError(tag,"Cannot register view ! - Bye !");
      return false;
    }
  }

  // Register world.
  if(!object_reg->Register(world,"iIsoWorld"))
  {
    ReportError(tag,"Cannot register World ! - Bye !");
    return false;
  }

  return true;
}

bool csIsoLoader::ParseStart (iDocumentNode* node, const char* /*prefix*/)
{
  char* tag = "crystalspace.iso.loader.parsestart";

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_POSITION:
        {
          if (!Syntax->ParseVector (child, start_v))
            return false;
        }
        break;
      default:
	ReportError (tag, "Bad token <%s>!", value);
	return false;
    }
  }

  return true;
}

bool csIsoLoader::ParsePluginList (iDocumentNode* node, const char* /*prefix*/)
{
  char* tag = "crystalspace.iso.loader.parsepluginlist";

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
        {
	  const char* plug = child->GetContentsValue ();
          loaded_plugins.NewPlugin (child->GetAttributeValue ("name"), plug);
        }
        break;
      default:
	ReportError (tag, "Bad token <%s>!", value);
	return false;
    }
  }

  return true;
}

bool csIsoLoader::ParseGridList (iDocumentNode* node, const char* /*prefix*/)
{
  char* tag = "crystalspace.iso.loader.parsegridlist";

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_GRID:
	if (!ParseGrid (child, child->GetAttributeValue ("name")))
          return false;
        break;
      default:
	ReportError (tag, "Bad token <%s>!", value);
	return false;
    }
  }

  return true;
}

bool csIsoLoader::ParseGrid (iDocumentNode* node, const char* /*prefix*/)
{
  char* tag = "crystalspace.iso.loader.parsegrid";

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_SIZE:
        {
	  int x, z;
	  x = child->GetAttributeValueAsInt ("x");
	  z = child->GetAttributeValueAsInt ("z");
          // switch x & z cause creategrid expects it like that
          current_grid = world->CreateGrid (z, x);
        }
        break;

      case XMLTOKEN_SPACE:
        {
	  int minx, minz;
	  float miny, maxy;
	  minx = child->GetAttributeValueAsInt ("minx");
	  minz = child->GetAttributeValueAsInt ("minz");
	  miny = child->GetAttributeValueAsFloat ("miny");
	  maxy = child->GetAttributeValueAsFloat ("maxy");
          current_grid->SetSpace(minx,minz,miny,maxy);
        }
        break;

      case XMLTOKEN_MULT:
        {
          int multx, multz;
	  multx = child->GetAttributeValueAsInt ("x");
	  multz = child->GetAttributeValueAsInt ("z");

          // May have to switch these as well - check out later !!
          current_grid->SetGroundMult(multx, multz);
        }
        break;

      case XMLTOKEN_LIGHT:
        if (!ParseLight (child, child->GetAttributeValue ("name")))
          return false;
        break;

      case XMLTOKEN_TILE2D:
        if (!ParseTile2D (child, child->GetAttributeValue ("name")))
          return false;
        break;

      case XMLTOKEN_MESHOBJ:
        if (!ParseMeshObject (child, child->GetAttributeValue ("name")))
          return false;
        break;

      default:
	ReportError (tag, "Bad token <%s>!", value);
	return false;
    }
  }

  return true;
}

bool csIsoLoader::ParseLight (iDocumentNode* node, const char* /*prefix*/)
{
  char* tag = "crystalspace.iso.loader.parselight";
  csRef<iIsoLight> light (csPtr<iIsoLight> (Engine->CreateLight()));

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_ATTENUATION:
        light->SetAttenuation (child->GetContentsValueAsInt ());
        break;

      case XMLTOKEN_RADIUS:
	light->SetRadius (child->GetContentsValueAsFloat ());
        break;

      case XMLTOKEN_COLOR:
	{
	  csColor col;
	  if (!Syntax->ParseColor (child, col))
	    return false;
          light->SetColor (col);
        }
        break;

      case XMLTOKEN_POSITION:
        {
	  csVector3 p;
	  if (!Syntax->ParseVector (child, p))
	    return false;
	  light->SetPosition (p);
        }
        break;

      default:
	ReportError (tag, "Bad token <%s>!", value);
	return false;
    }
  }

  // Tie the grid & the lights together 
  if (light && current_grid)
  {
    light->SetGrid(current_grid);
    current_grid->RegisterLight(light);
  }
  else
    ReportNotify("Warning: Cannot bind light to grid - this might be bad");

  return true;
}


bool csIsoLoader::ParseTile2D (iDocumentNode* node, const char* /*prefix*/)
{
  char* tag = "crystalspace.iso.loader.parsetile2d";
  csVector3 start, end;
  int offx, offz;
  int height_map = false;
  iMaterialWrapper *mat_wrap = 0;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_START:
	if (!Syntax->ParseVector (child, start))
	  return false;
        break;

      case XMLTOKEN_END:
	if (!Syntax->ParseVector (child, end))
	  return false;
        break;

      case XMLTOKEN_MATERIAL:
        {
	  const char* material_name = child->GetContentsValue ();
	  // Check to see if the material is already loaded
          mat_wrap = FindMaterial(material_name);

	  // If no wrapper then - bad !
	  if (!mat_wrap)
          {
            ReportError (tag,"Cant find a material called %s", material_name);
            return false;
          }
        }
        break;

      case XMLTOKEN_HEIGHTMAP:
        height_map = true;
        break;

      case XMLTOKEN_STYLE:
        {
	  ReportNotify("TILE STYLE not implemented", "");
        }
        break;

      default:
	ReportError (tag, "Bad token <%s>!", value);
	return false;
    }
  }

  // some sanity checking - to cut down on the runtime exceptions
  if (!mat_wrap)
  {
    ReportError(tag,"Missing a material. Cant tile without it ..bye!");
    return false;
  }

  // Ensure start is in the world & in this grid 
  iIsoGrid *tmpgrid = world->FindGrid(start);
  if (!tmpgrid)
  {
    ReportError(tag,"Start vector outside world space, cant tile - bye!");
    return false;
  }

  if (tmpgrid != current_grid)
  {
    ReportError(tag,"Start vector not inside current grid, cant tile - bye!");
    return false;
  }

  // Ensure end is in the world and belongs to this grid
  tmpgrid = world->FindGrid(end);
  if (!tmpgrid)
  {
    ReportError(tag,"End vector outside world space, cant tile - bye!");
    return false;
  }

  if (tmpgrid != current_grid)
  {
    ReportError(tag,"End vector not inside current grid, cant tile - bye!");
    return false;
  }


  // We're here so the definition parsed ok - now do some tiling.
  // Note: Can only tile in 2 dimensions at once so either
  // startx == endx  (tile X wall) or starty == endy (tile floor)
  // or startz == endz (tile Z wall)

  int x,y,z;
  int mi,mj;
  iIsoSprite *sprite = 0;

  // grid offsets - note z,x order
  current_grid->GetGridOffset(offz,offx);

  if (start.y == end.y)
  {
    y = QInt(start.y);
    for(z=QInt(start.z); z<end.z; z++)
    {
      for(x=QInt(start.x); x<end.x; x++)
      {
// ReportNotify("Tiling at %d %d %d %f %f %f",x,y,z,start.x,start.y,start.z);

        sprite = Engine->CreateFloorSprite(csVector3(x,y,z), 1.0, 1.0);
        sprite->SetMaterialWrapper(mat_wrap);
        world->AddSprite(sprite);
        sprite->DecRef ();

        if (height_map == true)
        {
          // Set heightmap based on ground value precision ??
          for(mj=0; mj<current_grid->GetGroundMultY(); mj++)
            for(mi=0; mi<current_grid->GetGroundMultX(); mi++)
              current_grid->SetGroundValue(z-offz, x-offx, mi, mj, y);
        }
      }
    }
  } 
  else if (start.x == end.x)
  {  
    x = QInt(start.x);
    for(z=QInt(start.z); z<end.z; z++)
    {
      for(y=QInt(start.y); y<end.y; y++)
      {
//        ReportNotify("Tiling at %d %d %d",x,y,z);

        sprite = Engine->CreateZWallSprite(csVector3(x,y,z), 1.0, 1.0);
        sprite->SetMaterialWrapper(mat_wrap);
        world->AddSprite(sprite);
        sprite->DecRef ();
      }

      if (height_map == true)
      {
  	    // Only need to map for z iterations at max y coz x aint changin'
        for(mj=0; mj<current_grid->GetGroundMultY(); mj++)
          for(mi=0; mi<current_grid->GetGroundMultX(); mi++)
	          current_grid->SetGroundValue(z-offz, x-offx, mi, mj, y);
      }
    }
  }
  else if (start.z == end.z)
  {
    z = QInt(start.z);
    for(x=QInt(start.x); x<end.x; x++)
    {
      for(y=QInt(start.y); y<end.y; y++)
      {		
//        ReportNotify("Tiling at %d %d %d",x,y,z);
        
        sprite = Engine->CreateXWallSprite(csVector3(x,y,z), 1.0, 1.0);
        sprite->SetMaterialWrapper(mat_wrap);
        world->AddSprite(sprite);
        sprite->DecRef ();
      }

      if (height_map == true)
      {
        // Only need to map for x iterations at max y coz z aint changin'
        for(mj=0; mj<current_grid->GetGroundMultY(); mj++)
          for(mi=0; mi<current_grid->GetGroundMultX(); mi++)
	          current_grid->SetGroundValue(z-offz, x-offx, mi, mj, y);
      }
    }
  }

//  ReportNotify("Finished tiling");

  return true;
}

bool csIsoLoader::ParseMaterialList (iDocumentNode* node,
  const char* /*prefix*/)
{
  char* tag = "crystalspace.iso.loader.parsemateriallist";

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
      case XMLTOKEN_MATERIAL:
        {
	  const char* vfsfilename = child->GetContentsValue ();
	  const char* name = child->GetAttributeValue ("name");
	  if (!Engine->CreateMaterialWrapper (vfsfilename, name))
	    ReportNotify ("WARNING: '%s' Not Loaded from '%s'",
	    	name, vfsfilename);
        }
        break;

      default:
	ReportError (tag, "Bad token <%s>!", value);
	return false;
    }
  }

  return true;
}

bool csIsoLoader::ParseMeshFactory(iDocumentNode* node, const char *prefix)
{
  char* tag = "crystalspace.iso.loader.parsemeshfactory";
  iLoaderPlugin* plug = 0;

  iMeshFactoryWrapper* mfw = Engine->CreateMeshFactory (prefix);

  ldr_context = 0;

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
        {
	  const char* classId = child->GetContentsValue ();
	  plug = loaded_plugins.FindPlugin (classId);
          if (!plug)
	  {
            ReportError (tag, "Could not load plugin!");
	    return false;
	  }
        }
        break;

      case XMLTOKEN_PARAMS:	
        {
      	  // iMeshFactoryWrapper context.
          csRef<iBase> mof (plug->Parse (child, GetLoaderContext(), mfw));
          if (!mof)
          {
            ReportError (tag, "Plugin loaded but cant parse <params>!");
	          return false;
          }
     	  csRef<iMeshObjectFactory> mof2 (SCF_QUERY_INTERFACE (mof,
              iMeshObjectFactory));
	  if (!mof2)
          {
            ReportError (tag,
              "Returned object does not implement iMeshObjectFactory!");
	          return false;
          }
	  mfw->SetMeshObjectFactory (mof2);
	  mof2->SetLogicalParent (mfw);
        }
        break;

      case XMLTOKEN_MESHFACT:
      case XMLTOKEN_MATERIAL:
      case XMLTOKEN_FILE:
      case XMLTOKEN_MOVE:
	{
          ReportNotify (tag,"Token '%s' not implemented ... Yet!", value);
        }
        break; 

      default:
	ReportError (tag, "Bad token <%s>!", value);
	return false;
    }
  }

  return true;
}

bool csIsoLoader::ParseMeshObject (iDocumentNode* node, const char* prefix)
{
  char* tag = "crystalspace.iso.loader.parsemeshobject"; 

  csRef<iMeshObject> meshobj;
  iLoaderPlugin* plug = 0;
  csMatrix3 m;
  csVector3 v;

  ldr_context = 0;

  iIsoMeshSprite* meshspr = Engine->CreateMeshSprite();

  // It all gets a bit weird in here, .. The isoEngine doesnt
  // use iMeshWrappers it uses isoSprites to wrap mesh objects
  // so dont bother creating iMeshWrappers in this function
  // create iIsoMeshSprites instead. 
  //
  // Side effects are KEY doesnt work

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      // isoengine doesnt support priority - but its here to 
      // allow cut & paste of 3d engine meshs with a minimum 
      // of hassle
      case XMLTOKEN_PRIORITY:
        break;

      case XMLTOKEN_ZFILL:
        meshspr->SetZBufMode (CS_ZBUF_FILL);
        break;

      case XMLTOKEN_ZUSE:
        meshspr->SetZBufMode (CS_ZBUF_USE);
        break;

      case XMLTOKEN_ZNONE:
        meshspr->SetZBufMode (CS_ZBUF_NONE);
        break;

      case XMLTOKEN_ZTEST:
        meshspr->SetZBufMode (CS_ZBUF_TEST);
        break;

      case XMLTOKEN_KEY:
        {
          ReportNotify ("WARNING: KEY token not active in %s ... Yet", prefix);
        }
        break;

      case XMLTOKEN_MOVE:
        {
          meshspr->SetTransform (csMatrix3 ());     // Identity
          meshspr->SetPosition (csVector3 (0));

	  csRef<iDocumentNode> matnode = child->GetNode ("matrix");
	  if (matnode)
	  {
            if (!Syntax->ParseMatrix (matnode, m))
	      return false;
            meshspr->SetTransform (m);
	  }
	  csRef<iDocumentNode> vecnode = child->GetNode ("v");
	  if (vecnode)
	  {
	    if (!Syntax->ParseVector (vecnode, v))
	      return false;

	    // Error check the vector, make sure it exists
	    // The sprite can be added to a grid outside
	    // the curently parsed grid - a feature ? 
	    if (!world->FindGrid(v))
	    {
	      ReportError(tag, 
		"MeshObject position outside the world in definition %s!",
		prefix);
	      return false;
	    }
	    meshspr->SetPosition (v);
	  }
	}              
	break;

      case XMLTOKEN_PARAMS:
        {
          csRef<iBase> mo (plug->Parse (child, GetLoaderContext(), 0));
	  if (!mo)
          {
            ReportError (tag,"Error in PARAMS() for %s.",prefix);
	          return false;
          }

          // Find the newly created mesh object
          meshobj = SCF_QUERY_INTERFACE (mo, iMeshObject);
          if (!meshobj)
          {
            ReportError (tag, 
              "Returned object does not implement iMeshObject!");
	    return false;
          }
          meshspr->SetMeshObject (meshobj);
        }
        break;

      case XMLTOKEN_PLUGIN:
        {
	  const char* plugname = child->GetContentsValue ();
          plug = loaded_plugins.FindPlugin (plugname);
          if (!plug)
          {
            ReportError (tag, "plugin not loaded in %s.", prefix);
	    return false;
          }
        }
        break;

      default:
	ReportError (tag, "Bad token <%s>!", value);
	return false;
    }
  }

  if (meshspr && meshobj)
  {
    // iIsoGrid *gr = world->FindPos(meshspr->GetPosition());
    world->AddSprite (meshspr);
    meshspr->DecRef ();
  }

  return true;
}

bool csIsoLoader::LoadPlugins (iDocumentNode* node)
{
  char* tag = "crystalspace.iso.loader.loadplugins";

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
	ReportError (tag, "Bad token <%s>!", value);
	return false;
    }
  }

  return true;
}

// --- End XML loaders

csIsoLoader::csIsoLoader(iBase *p)
{
  SCF_CONSTRUCT_IBASE(p);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);

  current_grid = 0;
  object_reg = 0;
}

csIsoLoader::~csIsoLoader()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

bool csIsoLoader::Initialize(iObjectRegistry *object_Reg)
{

  char* tag = "crystalspace.iso.loader.initialize";

  csIsoLoader::object_reg = object_Reg;

  // The plugin manager - needed mostly for the mesh object plugins
  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));
  if (!plugin_mgr)
  {
    ReportError (tag, "Failed to initialize iPluginManager");
    return false;
  }

  loaded_plugins.plugin_mgr = plugin_mgr;

  Reporter = CS_QUERY_REGISTRY (object_reg, iReporter);

  // Cant load a file if we cant find a file system
  VFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (!VFS)
  {
    ReportError (tag, "Failed to initialize VFS plugin !");
    return false;
  }

  // Load optional plugins ... probably used later
  Engine = CS_QUERY_REGISTRY(object_reg, iIsoEngine);
  if (!Engine)
  {
    ReportError (tag, "Failed to initialize iso Engine !");
    return false;
  }

  // Use the most excellent syntax services to parse
  // matrices, vectors and other generic stuff
  Syntax = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  if (!Syntax)
  {
    Syntax = CS_LOAD_PLUGIN (plugin_mgr,
    	"crystalspace.syntax.loader.service.text", iSyntaxService);
    if (!Syntax)
    {
      ReportError (tag, "Could not load the syntax services!");
      return false;
    }
    if (!object_reg->Register (Syntax, "iSyntaxService"))
    {
      ReportError (tag, "Could not register the syntax services!");
      return false;
    }
  }

  G3D = CS_QUERY_REGISTRY(object_reg, iGraphics3D);

  world = csPtr<iIsoWorld> (Engine->CreateWorld());

  xmltokens.Register ("world", XMLTOKEN_WORLD);
  xmltokens.Register ("grid", XMLTOKEN_GRID);
  xmltokens.Register ("grids", XMLTOKEN_GRIDS);
  xmltokens.Register ("sprite", XMLTOKEN_SPRITE);
  xmltokens.Register ("heightmap", XMLTOKEN_HEIGHTMAP);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("materials", XMLTOKEN_MATERIALS);
  xmltokens.Register ("size", XMLTOKEN_SIZE);
  xmltokens.Register ("space", XMLTOKEN_SPACE);
  xmltokens.Register ("mult", XMLTOKEN_MULT);
  xmltokens.Register ("light", XMLTOKEN_LIGHT);
  xmltokens.Register ("attenuation", XMLTOKEN_ATTENUATION);
  xmltokens.Register ("position", XMLTOKEN_POSITION);
  xmltokens.Register ("radius", XMLTOKEN_RADIUS);
  xmltokens.Register ("color", XMLTOKEN_COLOR);
  xmltokens.Register ("tile2d", XMLTOKEN_TILE2D);
  xmltokens.Register ("start", XMLTOKEN_START);
  xmltokens.Register ("end", XMLTOKEN_END);
  xmltokens.Register ("key", XMLTOKEN_KEY);
  xmltokens.Register ("style", XMLTOKEN_STYLE);
  xmltokens.Register ("plugins", XMLTOKEN_PLUGINS);
  xmltokens.Register ("plugin", XMLTOKEN_PLUGIN);
  xmltokens.Register ("meshfact", XMLTOKEN_MESHFACT);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("file", XMLTOKEN_FILE);
  xmltokens.Register ("params", XMLTOKEN_PARAMS);
  xmltokens.Register ("move", XMLTOKEN_MOVE);
  xmltokens.Register ("meshobj", XMLTOKEN_MESHOBJ);
  xmltokens.Register ("zfill", XMLTOKEN_ZFILL);
  xmltokens.Register ("znone", XMLTOKEN_ZNONE);
  xmltokens.Register ("zuse", XMLTOKEN_ZUSE);
  xmltokens.Register ("ztest", XMLTOKEN_ZTEST);
  xmltokens.Register ("priority", XMLTOKEN_PRIORITY);
  xmltokens.Register ("matrix", XMLTOKEN_MATRIX);
  xmltokens.Register ("v", XMLTOKEN_V);
  return true;
}
