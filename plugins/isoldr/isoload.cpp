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
#include "cssys/sysfunc.h"
#include "qint.h"

#include "csutil/cfgfile.h"
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "csutil/plugldr.h"

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
#include "imesh/skeleton.h"
#include "imesh/object.h"
#include "iengine/engine.h"
#include "iengine/motion.h"
#include "iengine/skelbone.h"
#include "iengine/material.h"
#include "iengine/movable.h"
#include "iengine/halo.h"
#include "iengine/campos.h"
#include "iengine/light.h"
#include "iengine/statlght.h"
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

#include "qint.h"

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

SCF_EXPORT_CLASS_TABLE (isoload)
  SCF_EXPORT_CLASS_DEP (csIsoLoader, "crystalspace.iso.loader",
    "iso engine level and library loader", "crystalspace.kernel., "
    "crystalspace.mesh.loader., "
    "crystalspace.engine.3d, crystalspace.graphics3d., "
	"crystalspace.engine.iso, crystalspace.syntax.loader.service.text ")
SCF_EXPORT_CLASS_TABLE_END

// Define all tokens used through this file
CS_TOKEN_DEF_START
  CS_TOKEN_DEF (WORLD)
  CS_TOKEN_DEF (GRID)
  CS_TOKEN_DEF (GRIDS)
  CS_TOKEN_DEF (SPRITE)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MATERIALS)
  CS_TOKEN_DEF (SIZE)
  CS_TOKEN_DEF (SPACE)
  CS_TOKEN_DEF (MULT)
  CS_TOKEN_DEF (LIGHT)
  CS_TOKEN_DEF (ATTENUATION)
  CS_TOKEN_DEF (POSITION)
  CS_TOKEN_DEF (RADIUS)
  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (TILE2D)
  CS_TOKEN_DEF (START)
  CS_TOKEN_DEF (END)
  CS_TOKEN_DEF (KEY)
  CS_TOKEN_DEF (STYLE)
  CS_TOKEN_DEF (PLUGINS)
  CS_TOKEN_DEF (PLUGIN)
  CS_TOKEN_DEF (MESHFACT)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (FILE)
  CS_TOKEN_DEF (PARAMS)
  CS_TOKEN_DEF (MOVE)
  CS_TOKEN_DEF (MESHOBJ)
  CS_TOKEN_DEF (ZFILL)
  CS_TOKEN_DEF (ZNONE)
  CS_TOKEN_DEF (ZUSE)
  CS_TOKEN_DEF (ZTEST)
  CS_TOKEN_DEF (PRIORITY)
  CS_TOKEN_DEF (MATRIX)
  CS_TOKEN_DEF (V)
CS_TOKEN_DEF_END

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
  virtual iMeshFactoryWrapper* FindMeshFactory (const char* name);
  virtual iMeshWrapper* FindMeshObject (const char* name);
};

SCF_IMPLEMENT_IBASE(StdIsoLoaderContext);
  SCF_IMPLEMENTS_INTERFACE(iLoaderContext);
SCF_IMPLEMENT_IBASE_END;

StdIsoLoaderContext::StdIsoLoaderContext (iIsoEngine* Engine)
{
  SCF_CONSTRUCT_IBASE (NULL);
  StdIsoLoaderContext::Engine = Engine;
}

StdIsoLoaderContext::~StdIsoLoaderContext ()
{
}

iSector* StdIsoLoaderContext::FindSector (const char* /*name*/)
{
  return NULL;
}

iMaterialWrapper* StdIsoLoaderContext::FindMaterial (const char* name)
{
  return Engine->GetMaterialList ()->FindByName (name);
}

iMeshFactoryWrapper* StdIsoLoaderContext::FindMeshFactory (const char* name)
{
  return Engine->GetMeshFactories ()->FindByName(name);
}

iMeshWrapper* StdIsoLoaderContext::FindMeshObject (const char* /*name*/)
{
  return NULL;
}

//---------------------------------------------------------------------------



//-- Begin - Helpers ----------------------------------------------
// CnP from csparser.cpp
iLoaderContext* csIsoLoader::GetLoaderContext ()
{
  if (!ldr_context)
  {
    ldr_context = new StdIsoLoaderContext (Engine);
  }
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
  return NULL;
}

iMeshFactoryWrapper *csIsoLoader::FindMeshFactory (const char *iName)
{
  iMeshFactoryWrapper* mfw;

  mfw = Engine->GetMeshFactories ()->FindByName(iName);
  if (mfw)
    return mfw;

  ReportError("crystalspace.iso.loader.findmeshfactory",
    "Could not find mesh factory named '%s'!",iName);
  return NULL;
}

void csIsoLoader::TokenError (const char *Object)
{
  ReportError ("crystalspace.iso.loader.tokenerror.badtoken",
    "Token '%s' not found while parsing a %s!",
    csGetLastOffender (), Object);
}

bool csIsoLoader::CheckParams(char* params, const char* tag, char *data)
{
   if (!params)
   {
     ReportError (tag, "Expected parameters but got '%s' !", data);
     return false;
   }
   return true;
}

bool csIsoLoader::CheckToken(int cmd, const char* tag, char* data)
{

  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
//     TokenError (tag);
     ReportNotify("%s\nfunction %s\ncmd %d\ndata %s or %s\n",
       "Bad token found while parsing - Diagnostics follow",
       tag, cmd, data, csGetLastOffender ());
     return false;
  }

  return true;
}

//-- End - Helpers -----------------------------------------------

bool csIsoLoader::LoadMapFile (const char* file)
{
  iDataBuffer *buf = VFS->ReadFile (file);

  if (!buf || !buf->GetSize ())
  {
    if (buf) buf->DecRef ();
      ReportError ("crystalspace.iso.loader.loadmapfile.nomap",
    	"Could not open map file '%s' on VFS!", file);
    return false;
  }

  //  iConfigFile *cfg = new csConfigFile ("map.cfg", VFS);

  if (!LoadMap (**buf))
  {
    buf->DecRef ();
    return false;
  }

  buf->DecRef ();
  return true;
}

bool csIsoLoader::LoadMap (char* buf)
{
  if (!Engine) return false;

  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (WORLD)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (GRIDS)
    CS_TOKEN_TABLE (MATERIALS)
    CS_TOKEN_TABLE (PLUGINS)
    CS_TOKEN_TABLE (MESHFACT)
    CS_TOKEN_TABLE (START)
  CS_TOKEN_TABLE_END

  csResetParserLine();
  char *name = NULL; 
  char *data = NULL;
  char *tag = "crystalspace.iso.loader.loadmap";
  long cmd;
  bool set_view = false;

  // Parse the WORLD token
  if ((cmd = csGetObject (&buf, tokens, &name, &data)) > 0)
  {
    if (!CheckParams(data, tag, buf))
      return false;
  }

  // Make sure token is valid
  if (!CheckToken(cmd, tag, buf))
    return false;   

  char* params;

  while ((cmd = csGetObject (&data, commands, &name, &params)) > 0)
  {

    if (!CheckParams (params, tag, data))
      return false;

    switch (cmd)
    {
      case CS_TOKEN_GRIDS:
        if (!ParseGridList (params, name))
          return false;
        break;

      case CS_TOKEN_MATERIALS:
        if (!ParseMaterialList (params, name))
          return false;
        break;

      case CS_TOKEN_PLUGINS:
        if (!ParsePluginList (params, name))
          return false;
        break;

      case CS_TOKEN_MESHFACT:
        if(!ParseMeshFactory (params, name))
          return false;
        break;

      case CS_TOKEN_START:
        if(!ParseStart (params, name))
          return false;
        else
          set_view = true;
        break;

    }
  }

  if (!CheckToken(cmd, tag, data))
    return false;   

  // before wrapping up see if we need to set the view 
  // only set the view if a START tag was given
  if (set_view == true)
  {
    if (!world->FindGrid(start_v))
    {
      ReportError(tag,"START POSITION outside world space - bye!");
      return false;
    }
    view = Engine->CreateView(world);
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

bool csIsoLoader::ParseStart (char* buf, const char* /*prefix*/)
{
  CS_TOKEN_TABLE_START(commands)
    CS_TOKEN_TABLE (POSITION)
  CS_TOKEN_TABLE_END

  long cmd;
  char* params;
  char* name;
  char* tag = "crystalspace.iso.loader.parsestart";

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {

    if (!CheckParams (params, tag, buf))
      return false;

    switch (cmd)
    {
      case CS_TOKEN_POSITION:
        {
          if(!Syntax->ParseVector (params, start_v))
            return false;
        }
        break;
    }
  }

  if (!CheckToken(cmd, tag, buf))
    return false;   

  return true;
}

bool csIsoLoader::ParsePluginList(char* buf, const char* /*prefix*/)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (PLUGIN)
  CS_TOKEN_TABLE_END

  char* tag = "crystalspace.iso.loader.parsepluginlist";
  char* params;
  char* name;
  char str[255];
  long cmd;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {

    if (!CheckParams(params, tag, buf))
      return false;

    switch (cmd)
    {
      case CS_TOKEN_PLUGIN:
        {
          csScanStr(params, "%s", str);
          loaded_plugins.NewPlugin (name, str);
        }
        break;
    }
  }

  if (!CheckToken(cmd, tag, buf))
    return false;   

  return true;
}

bool csIsoLoader::ParseGridList (char* buf, const char* /*prefix*/)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (GRID)
  CS_TOKEN_TABLE_END

  char* tag = "crystalspace.iso.loader.parsegridlist";
  char* params;
  char* name;
  long cmd;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {

    if (!CheckParams(params, tag, buf))
      return false;

    switch (cmd)
    {
      case CS_TOKEN_GRID:
		    if (!ParseGrid (params, name))
          return false;
        break;
    }
  }

  if (!CheckToken(cmd, tag, buf))
    return false;   

  return true;
}

bool csIsoLoader::ParseGrid (char* buf, const char* /*prefix*/)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (SIZE)
    CS_TOKEN_TABLE (SPACE)
    CS_TOKEN_TABLE (MULT)
    CS_TOKEN_TABLE (LIGHT)
	  CS_TOKEN_TABLE (TILE2D)
    CS_TOKEN_TABLE (MESHOBJ)
  CS_TOKEN_TABLE_END

  char* tag = "crystalspace.iso.loader.parsegrid";
  char* params;
  char* name;
  long cmd;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {

    if (!CheckParams(params, tag, buf))
      return false;

    switch (cmd)
    {
      case CS_TOKEN_SIZE:
        {
			    int x=0,z=0;

          // switch x & z cause creategrid expects it like that
		      csScanStr(params,"%d,%d",&x,&z);
          current_grid = world->CreateGrid(z,x);
        }
        break;

      case CS_TOKEN_SPACE:
        {
			    int minx=0,minz=0;
			    float miny=0,maxy=0;
          csScanStr(params,"%d,%d,%f,%f",&minx,&minz,&miny,&maxy);
          current_grid->SetSpace(minx,minz,miny,maxy);
        }
        break;

      case CS_TOKEN_MULT:
        {
          int multx=0,multz=0;

          // May have to switch these as well - check out later !!
		      csScanStr(params,"%d,%d",&multx, &multz);
          current_grid->SetGroundMult(multx, multz);
        }
        break;

      case CS_TOKEN_LIGHT:
        if (!ParseLight (params, name))
          return false;
        break;

      case CS_TOKEN_TILE2D:
        if (!ParseTile2D (params, name))
          return false;
        break;

      case CS_TOKEN_MESHOBJ:
        if (!ParseMeshObject (params, name))
          return false;
        break;

    }
  }

  if (!CheckToken(cmd, tag, buf))
    return false;   

  return true;
}

bool csIsoLoader::ParseLight (char* buf, const char* /*prefix*/)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (ATTENUATION)
    CS_TOKEN_TABLE (POSITION)
    CS_TOKEN_TABLE (RADIUS)
    CS_TOKEN_TABLE (COLOR)
  CS_TOKEN_TABLE_END

  char* tag = "crystalspace.iso.loader.parselight";
  char* params;
  char* name;
  long cmd;

  iIsoLight *light = Engine->CreateLight();

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {

    if (!CheckParams(params, tag, buf))
      return false;

    switch (cmd)
    {
      case CS_TOKEN_ATTENUATION:
        {
			    int att=0;
	        csScanStr(params,"%d",&att);
          light->SetAttenuation(att);
        }
        break;

      case CS_TOKEN_RADIUS:
        {
			    float rad=0;
			    csScanStr(params,"%f",&rad);
			    light->SetRadius(rad);
        }
        break;

      case CS_TOKEN_COLOR:
        {
			    float r=0,g=0,b=0;
		      csScanStr(params,"%f,%f,%f",&r,&g,&b);
          light->SetColor(csColor(r,g,b));
        }
        break;

      case CS_TOKEN_POSITION:
        {
			    float xpos=0,ypos=0,zpos=0;
          csScanStr(params,"%f,%f,%f",&xpos,&ypos,&zpos);
			    light->SetPosition(csVector3(xpos,ypos,zpos));
        }
        break;
    }
  }

  if (!CheckToken(cmd, tag, buf))
    return false;   

  // Tie the grid & the lights together 
  if (light && current_grid)
  {
    light->SetGrid(current_grid);
    current_grid->RegisterLight(light);
  }
  else
	  ReportNotify("Warning: Cannot bind light to grid - this might be bad");

  light->DecRef();

  return true;
}


bool csIsoLoader::ParseTile2D (char* buf, const char* /*prefix*/)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (START)
    CS_TOKEN_TABLE (END)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (STYLE)
  CS_TOKEN_TABLE_END

  char* tag = "crystalspace.iso.loader.parsetile2d";
  char* params;
  char* name;
  long cmd;
  csVector3 start,end;
  int offx,offz;

  iMaterialWrapper *mat_wrap=0;

//  ReportNotify("Parsing tile 2D");

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!CheckParams (params, tag, buf))
      return false;

    switch (cmd)
    {
      case CS_TOKEN_START:
	      Syntax->ParseVector (params, start);
        break;

      case CS_TOKEN_END:
        Syntax->ParseVector (params, end);
        break;

      case CS_TOKEN_MATERIAL:
        {
          char material_name[255];
          material_name[0]=0;

		      csScanStr(params, "%s", material_name);

			    // Check to see if the material is already loaded
          mat_wrap = FindMaterial(material_name);

			    // If no wrapper then - bad !
		      if (!mat_wrap)
          {
            ReportError(tag,"Cant find a material called %s", material_name);
            return false;
          }
        }
        break;

      case CS_TOKEN_STYLE:
        {
			    ReportNotify("TILE STYLE not implemented",params);
        }
        break;
    }
  }

  if (!CheckToken(cmd, tag, buf))
    return false;   

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
//        ReportNotify("Tiling at %d %d %d",x,y,z);

        sprite = Engine->CreateFloorSprite(csVector3(x,y,z), 1.0, 1.0);
        sprite->SetMaterialWrapper(mat_wrap);
        world->AddSprite(sprite);
        sprite->DecRef ();

        // Set heightmap bsed on ground value precision ??
        for(mj=0; mj<current_grid->GetGroundMultY(); mj++)
          for(mi=0; mi<current_grid->GetGroundMultX(); mi++)
            current_grid->SetGroundValue(z-offz, x-offx, mi, mj, y);
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

        sprite = Engine->CreateXWallSprite(csVector3(x,y,z), 1.0, 1.0);
        sprite->SetMaterialWrapper(mat_wrap);
        world->AddSprite(sprite);
        sprite->DecRef ();
      }

	    // Only need to map for z iterations at max y coz x aint changin'
      for(mj=0; mj<current_grid->GetGroundMultY(); mj++)
        for(mi=0; mi<current_grid->GetGroundMultX(); mi++)
	        current_grid->SetGroundValue(z-offz, x-offx, mi, mj, y);
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
        
        sprite = Engine->CreateZWallSprite(csVector3(x,y,z), 1.0, 1.0);
        sprite->SetMaterialWrapper(mat_wrap);
        world->AddSprite(sprite);
        sprite->DecRef ();
      }

	    // Only need to map for x iterations at max y coz z aint changin'
      for(mj=0; mj<current_grid->GetGroundMultY(); mj++)
        for(mi=0; mi<current_grid->GetGroundMultX(); mi++)
	        current_grid->SetGroundValue(z-offz, x-offx, mi, mj, y);
    }
  }

//  ReportNotify("Finished tiling");

  return true;
}

bool csIsoLoader::ParseMaterialList (char* buf, const char* /*prefix*/)
{
  if (!Engine) return false;

  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
  CS_TOKEN_TABLE_END

  char* tag = "crystalspace.iso.loader.parsemateriallist";
  char* name;
  char* params;
  long cmd;

  char vfsfilename[255];
 
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {

    if (!CheckParams(params, tag, buf))
      return false;

    vfsfilename[0] = 0;

    switch (cmd)
    {
      case CS_TOKEN_MATERIAL:
        {

		      csScanStr(params, "%s", vfsfilename);

		      if (!Engine->CreateMaterialWrapper(vfsfilename, name))
			      ReportNotify("WARNING: '%s' Not Loaded from '%s'",
              name, vfsfilename);
        }
        break;
    }
  }

  if (!CheckToken(cmd, tag, buf))
    return false;   

  return true;
}

bool csIsoLoader::ParseMeshFactory(char *buf, const char *prefix)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (FILE)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (PARAMS)
    CS_TOKEN_TABLE (PLUGIN)
    CS_TOKEN_TABLE (MESHFACT)
    CS_TOKEN_TABLE (MOVE)
  CS_TOKEN_TABLE_END

  char* tag = "crystalspace.iso.loader.parsemeshfactory";
  char* params;
  char* name;
  long cmd;

  char classId[255];
  iLoaderPlugin* plug = NULL;

  iMeshFactoryWrapper* mfw = Engine->CreateMeshFactory(prefix);

  SCF_DEC_REF (ldr_context); ldr_context = NULL;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {

    if (!CheckParams(params, tag, buf))
      return false;

    switch (cmd)
    {

      case CS_TOKEN_PLUGIN:
        {
          classId[0] = 0;
	        csScanStr (params, "%s", classId);
	        plug = loaded_plugins.FindPlugin (classId);
        }
        break;

      case CS_TOKEN_PARAMS:	
        {
          if (!mfw)
          {
            ReportError(tag,"Define PLUGIN before PARAMS in %s.",
              prefix);
            return false;
          }

          if (!plug)
          {
            ReportError (tag, "Could not load plugin!");
	          return false;
          }

      	  // iMeshFactoryWrapper context.
          iBase* mof = plug->Parse (params, GetLoaderContext(), mfw);
	        if (!mof)
          {
            ReportError (tag, "Plugin loaded but cant parse PARAMS!");
	          return false;
          }
     	    iMeshObjectFactory* mof2 = SCF_QUERY_INTERFACE (mof,
              iMeshObjectFactory);
	        if (!mof2)
          {
            ReportError (tag,
              "Returned object does not implement iMeshObjectFactory!");
	          return false;
          }
	        mfw->SetMeshObjectFactory (mof2);
	        mof2->SetLogicalParent (mfw);
	        mof2->DecRef ();
	        mof->DecRef ();
        }
        break;

      case CS_TOKEN_MESHFACT:
      case CS_TOKEN_MATERIAL:
      case CS_TOKEN_FILE:
      case CS_TOKEN_MOVE:
	      {
          ReportNotify (tag,"Token '%s' not implemnted ... Yet!", name);
        }
        break; 
    }
  }

  if (!CheckToken(cmd, tag, buf))
    return false;   

  return true;
}

bool csIsoLoader::ParseMeshObject (char* buf, const char* prefix)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MESHOBJ)
    CS_TOKEN_TABLE (MOVE)
    CS_TOKEN_TABLE (PLUGIN)
    CS_TOKEN_TABLE (PARAMS)
    CS_TOKEN_TABLE (ZFILL)
    CS_TOKEN_TABLE (ZNONE)
    CS_TOKEN_TABLE (ZUSE)
    CS_TOKEN_TABLE (ZTEST)
    CS_TOKEN_TABLE (KEY)
    CS_TOKEN_TABLE (PRIORITY)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_matvec)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  char* tag = "crystalspace.iso.loader.parsemeshobject"; 
  char* params;
  char* name;
  char str[255]; str[0] = '\0';
  long cmd;

  iMeshObject* meshobj = NULL;
  iLoaderPlugin* plug = NULL;
  csMatrix3 m;
  csVector3 v;

  SCF_DEC_REF (ldr_context); ldr_context = NULL;

  iIsoMeshSprite* meshspr = Engine->CreateMeshSprite();

  // It all gets a bit werid in here, .. The isoEngine doesnt
  // use iMeshWrappers it uses isoSprites to wrap mesh objects
  // so dont bother creating iMeshWrappers in this function
  // create iIsoMeshSprites instead. 
  //
  // Side effects are KEY doesnt work

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {

    if (!CheckParams(params, tag, buf))
      return false;

    switch (cmd)
    {

      // isoengine doesnt support priority - but its here to 
      // allow cut & paste of 3d engine meshs with a minimum 
      // of hassle
      case CS_TOKEN_PRIORITY:
        break;

      case CS_TOKEN_ZFILL:
        meshspr->SetZBufMode (CS_ZBUF_FILL);
        break;

      case CS_TOKEN_ZUSE:
        meshspr->SetZBufMode (CS_ZBUF_USE);
        break;

      case CS_TOKEN_ZNONE:
        meshspr->SetZBufMode (CS_ZBUF_NONE);
        break;

      case CS_TOKEN_ZTEST:
        meshspr->SetZBufMode (CS_ZBUF_TEST);
        break;

      case CS_TOKEN_KEY:
        {
           ReportNotify("WARNING: KEY token not active in %s ... Yet",prefix);
        }
        break;

      case CS_TOKEN_MOVE:
        {
          char* params2;

          while ((cmd = csGetObject (&params, tok_matvec, &name, &params2)) > 0)
          {

            if (!CheckParams(params2, tag, params))
              return false;

            meshspr->SetTransform (csMatrix3 ());     // Identity
            meshspr->SetPosition (csVector3 (0));

            switch (cmd)
            {
              case CS_TOKEN_MATRIX:
                {
                  if (!Syntax->ParseMatrix (params2, m))
		                return false;
                  meshspr->SetTransform (m);
                }
                break;
              
              case CS_TOKEN_V:
                {
                  if (!Syntax->ParseVector (params2, v))
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
                break;
            }
          }
        }
        break;

      case CS_TOKEN_PARAMS:
        {
          if (!plug)
          {
            ReportError (tag, "plugin not loaded in %s.",prefix);
	          return false;
          }

	        iBase* mo = plug->Parse (params, GetLoaderContext(), NULL);
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
          meshspr->SetMeshObject(meshobj);
          mo->DecRef();
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

  if (!CheckToken(cmd, tag, buf))
    return false;   

  if (meshspr && meshobj)
  {
    // iIsoGrid *gr = world->FindPos(meshspr->GetPosition());
    world->AddSprite(meshspr);
    meshspr->DecRef();
    meshobj->DecRef();
  }

  return true;
}

bool csIsoLoader::LoadPlugins (char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (PLUGIN)
  CS_TOKEN_TABLE_END

  char* tag = "crystalspace.iso.loader.loadplugins";
  char* params;
  char* name;
  char str[255];
  long cmd;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {

    if (!CheckParams(params, tag, buf))
      return false;

    switch (cmd)
    {
      case CS_TOKEN_PLUGIN:
        {
	        csScanStr (params, "%s", str);
	        loaded_plugins.NewPlugin (name, str);
        }
        break;
    }
  }

  if (!CheckToken(cmd, tag, buf))
    return false;   

  return true;
}

csIsoLoader::csIsoLoader(iBase *p)
{
  SCF_CONSTRUCT_IBASE(p);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);

  world = NULL;
  view = NULL;
  current_grid = NULL;
  plugin_mgr = NULL;
  object_reg = NULL;
  Engine = NULL;
  G3D = NULL;
  Reporter = NULL;
  VFS = NULL;
  ldr_context = NULL;
}

csIsoLoader::~csIsoLoader()
{
  SCF_DEC_REF(ldr_context);
  SCF_DEC_REF(plugin_mgr);
  SCF_DEC_REF(world);
  SCF_DEC_REF(view);
  SCF_DEC_REF(Engine);
  SCF_DEC_REF(VFS);
  SCF_DEC_REF(G3D);
  SCF_DEC_REF(Syntax);
  SCF_DEC_REF(Reporter);
}

bool csIsoLoader::Initialize(iObjectRegistry *object_Reg)
{

  char* tag = "crystalspace.iso.loader.initialize";

  csIsoLoader::object_reg = object_Reg;

  // The plugin manager - needed mostly for the mesh object plugins
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
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

  world = Engine->CreateWorld();

  return true;
}
