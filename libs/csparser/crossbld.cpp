/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

// Crystal Space builder. Given a converter instance, extracts various
// Crystal Space structures from the converter, including frames,
// actions, sprite templates, and things. The resulting structures
// can be used in a Crystal Space engine.

#include "cssysdef.h"
#include "csparser/crossbld.h"
#include "csgfx/csimage.h"
#include "cssys/system.h"
#include "csutil/scfstrv.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"
#include "iobject/object.h"
#include "imesh/sprite3d.h"
#include "imesh/object.h"
#include "iengine/engine.h"
#include "iengine/mesh.h"
#include "iengine/thing.h"
#include "iengine/polygon.h"
#include "iengine/ptextype.h"
#include "iengine/texture.h"
#include "igraphic/imageio.h"

// helper function for image loading

iImage *LoadImage (UByte* iBuffer, ULong iSize, int iFormat)
{
  iImageIO *ImageLoader =
    QUERY_PLUGIN_ID (System, CS_FUNCID_IMGLOADER, iImageIO);
  if (!ImageLoader) return NULL;
  iImage *img = ImageLoader->Load(iBuffer, iSize, iFormat);
  ImageLoader->DecRef();
  return img;
}

//
// definitions of method for the base csCrossBuild_Factory
// These are really just placeholders, they shouldn't do much
//

/// constructor
csCrossBuild_Factory::csCrossBuild_Factory()
{
}

/// destructor
csCrossBuild_Factory::~csCrossBuild_Factory()
{
}

//
// method definitions for csCrossBuild_SpriteTemplateFactory
//

/// constructor
csCrossBuild_SpriteTemplateFactory::csCrossBuild_SpriteTemplateFactory()
  : csCrossBuild_Factory()
{
}

/// destructor
csCrossBuild_SpriteTemplateFactory::~csCrossBuild_SpriteTemplateFactory()
{
}

/// full sprite template builder
void* csCrossBuild_SpriteTemplateFactory::CrossBuild (converter &buildsource)
{
  iMeshObjectType* type = QUERY_PLUGIN_CLASS (System, "crystalspace.mesh.object.sprite.3d", "MeshObj", iMeshObjectType);
  if (!type)
  {
    type = LOAD_PLUGIN (System, "crystalspace.mesh.object.sprite.3d", "MeshObj", iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.sprite.3d\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  CrossBuild ((void*)fact, buildsource);
  return fact;
}

/// full sprite template builder (second variant)
void csCrossBuild_SpriteTemplateFactory::CrossBuild (void* object,
  converter &buildsource)
{
  iMeshObjectFactory* fact = (iMeshObjectFactory*)object;
  iSprite3DFactoryState* fstate = QUERY_INTERFACE (fact, iSprite3DFactoryState);
  buildsource.set_animation_frame(0);

  // build the triangle mesh
  Build_TriangleMesh(fstate, buildsource);

  // build all the frames
  for (int frameindex=0; 
    frameindex<=buildsource.set_animation_frame(frameindex); 
    frameindex++)
  {
    Build_Frame(fstate, buildsource);
  }

  // make a 'default' action showing the first frame, if any
  if (fstate->GetFrame (0) != NULL)
  {
    iSpriteAction *defaultaction = fstate->AddAction();
    defaultaction->SetName("default");
    defaultaction->AddFrame( fstate->GetFrame(0), 1000 );
  }

  fstate->DecRef ();
}

/// frame build method
void csCrossBuild_SpriteTemplateFactory::Build_Frame
  (iSprite3DFactoryState* framesource, converter& buildsource)
{
  iSpriteFrame *newframe = framesource->AddFrame();
  newframe->SetName(buildsource.object_name);
  int anm_idx = newframe->GetAnmIndex();
  int tex_idx = newframe->GetTexIndex();

  if (framesource->GetNumFrames() == 1)
    framesource->AddVertices(buildsource.num_cor3);

  for (int coordindex=0; coordindex<buildsource.num_cor3; coordindex++)
  {
    // standard 3D coords seem to swap y and z axis compared to CS
    framesource->GetVertex(anm_idx, coordindex) = csVector3 (
      buildsource.cor3[0][coordindex]/20.0,
      buildsource.cor3[2][coordindex]/20.0,
      buildsource.cor3[1][coordindex]/20.0);
    framesource->GetTexel(tex_idx, coordindex) = csVector2 (
      buildsource.cor3_uv[0][coordindex],
      buildsource.cor3_uv[1][coordindex]);
  }
}

/// triangle mesh builder
void csCrossBuild_SpriteTemplateFactory::Build_TriangleMesh(iSprite3DFactoryState* meshsource,converter& buildsource)
{
  for (int triangleindex=0; triangleindex<buildsource.num_face; triangleindex++)
  {
    // triangulate since CS sprites handle polys of 3rd order only
    for (int ivert=2; ivert < buildsource.face_order[triangleindex]; ivert++)
    {
      int a = buildsource.face[0][triangleindex];
      int b = buildsource.face[ivert-1][triangleindex];
      int c = buildsource.face[ivert][triangleindex];
      meshsource->AddTriangle(a,b,c);
    }
  }
}


#if 0
// build sprite template from a quake-pakked file
csSpriteTemplate *ivconbuild_Quake2csSpriteTemplate(Archive &pakarchive)
{

  // read in the model data and, since the converter needs a file to
  // read from spew into a temporary file
  size_t modeldatasize;
  char *modelrawdata = pakarchive.read(modelfilename,&modeldatasize);
  char *tempfilenamebase = tmpnam(NULL);
  char tempfilename[2000];
  sprintf(tempfilename,"%s.md2",tempfilenamebase);
  FILE *tempfile = fopen(tempfilename,"wb");
  fwrite(modelrawdata,modeldatasize,1,tempfile);
  delete [] modelrawdata;

  return newtemplate;
}


// load textures from a quake-pakked file.  All textures will have
// their names prefixed by the prefixstring if it is non-NULL
// returns the 'default' texture for the sprite in this file
iTextureWrapper *ivconload_Quake2Textures(iEngine *engine,
	Archive &pakarchive, char const*prefixstring)
{
  // go through and load all .pcx and .bmp files in the archive
  void *currententry = pakarchive.first_file();

  iTextureWrapper *defaulttexture = NULL;

  while (currententry != NULL)
  {
    char *filename = pakarchive.get_file_name(currententry);

    // if the file name ends in .pcx, load it
    char *fileextension = filename + strlen(filename) - 3;
    if (   (strcmp("pcx",fileextension) == 0) 
        || (strcmp("PCX",fileextension) == 0) 
	|| (strcmp("bmp",fileextension) == 0)
	|| (strcmp("BMP",fileextension) == 0)
       )
    {
      // read from spew into a temporary file
      size_t skindatasize;
      char *skinrawdata = pakarchive.read(filename,&skindatasize);
      char *tempfilename = tmpnam(NULL);
      FILE *tempfile = fopen(tempfilename,"wb");
      fwrite(skinrawdata,skindatasize,1,tempfile);
      delete [] skinrawdata;
      fclose(tempfile);

      // convert to a gif which we CAN read
      char buffer[1024];
      sprintf(buffer,"convert %s %s.gif",tempfilename,tempfilename);
      system(buffer);

      // read in the converted skin
      sprintf(buffer,"%s.gif",tempfilename);
      tempfile = fopen(buffer,"rb");
      iImage *newskin = csImageLoader::Load(tempfile);
      fclose(tempfile);
      unlink(buffer);
      unlink(tempfilename);

      if (!defaulttexture)
        defaulttexture = engine->GetTextures()->NewTexture(newskin);
/*
      iTextureManager *txtmgr = System->G3D->GetTextureManager ();
      iTextureHandle *th = txtmgr->RegisterTexture(defaulttexture->GetImageFile(),
        defaulttexture->flags, false);
      defaulttexture->SetTextureHandle(th);
      */

      printf("added texture %s...\n",filename);

      if (prefixstring)
      {
        char *prefixedname = new char[strlen(filename)+strlen(prefixstring)+1];
        strcpy(prefixedname,prefixstring);
	strcat(prefixedname,filename);
	defaulttexture->SetName (prefixedname);
	delete[] prefixedname;
      }
      defaulttexture->SetName (filename);
    }

    currententry = pakarchive.next_file(currententry);
  }

  return defaulttexture;
}


#endif

//
// method definitions for csCrossBuild_ThingTemplateFactory
//

/// constructor
csCrossBuild_ThingTemplateFactory::csCrossBuild_ThingTemplateFactory()
  : csCrossBuild_Factory()
{
}

/// destructor
csCrossBuild_ThingTemplateFactory::~csCrossBuild_ThingTemplateFactory()
{
}

/// full thing template builder
void *csCrossBuild_ThingTemplateFactory::CrossBuild (converter &buildsource)
{
  iEngine* engine = QUERY_PLUGIN (System, iEngine);
  iMeshObjectType* thing_type = engine->GetThingType ();
  engine->DecRef ();
  iMeshObjectFactory* thing_fact = thing_type->NewFactory ();
  iMeshObject* thing_obj = QUERY_INTERFACE (thing_fact, iMeshObject);
  thing_fact->DecRef ();

  iThingState* thing_state = QUERY_INTERFACE (thing_obj, iThingState);

  CrossBuild ((void*)thing_state, buildsource);
  thing_state->DecRef ();
  return thing_obj;
}

/// full thing template builder (second variant)
void csCrossBuild_ThingTemplateFactory::CrossBuild (void* object,
	converter &buildsource)
{
  iThingState* thing_state = (iThingState*)object;
  buildsource.set_animation_frame (0);

  // Add the vertices
  Add_Vertices (thing_state, buildsource);

  // Build the triangle mesh
  Build_TriangleMesh (thing_state, buildsource);
}

/// frame build method
void csCrossBuild_ThingTemplateFactory::Add_Vertices (
	iThingState* thing_state, converter& buildsource)
{
  for (int coordindex=0; coordindex<buildsource.num_cor3; coordindex++)
  {
    // standard 3D coords seem to swap y and z axis compared to CS
    thing_state->CreateVertex (
    	csVector3 (
    		buildsource.cor3[0][coordindex]/20.0,
    		buildsource.cor3[2][coordindex]/20.0,
    		buildsource.cor3[1][coordindex]/20.0));
  }
}


/// triangle mesh builder
void csCrossBuild_ThingTemplateFactory::Build_TriangleMesh (
	iThingState* thing_state, converter& buildsource)
{
  for (int triangleindex=0; triangleindex<buildsource.num_face; triangleindex++)
  {
    char buf[10];
    sprintf (buf, "t%d", triangleindex);
    iPolygon3D* ptemp = thing_state->CreatePolygon ();
    //@@@ MATERIAL!!! ptemp->SetMaterial (NULL);
    int a = buildsource.face[0][triangleindex];
    int b = buildsource.face[1][triangleindex];
    int c = buildsource.face[2][triangleindex];
    ptemp->CreateVertex (a);
    ptemp->CreateVertex (b);
    ptemp->CreateVertex (c);
    ptemp->SetTextureType (POLYTXT_GOURAUD);
    iPolyTexType* pt = ptemp->GetPolyTexType ();
    iPolyTexFlat* gs = QUERY_INTERFACE (pt, iPolyTexFlat);
    gs->SetUV (0, buildsource.cor3_uv[0][a], buildsource.cor3_uv[1][a]);
    gs->SetUV (1, buildsource.cor3_uv[0][b], buildsource.cor3_uv[1][b]);
    gs->SetUV (2, buildsource.cor3_uv[0][c], buildsource.cor3_uv[1][c]);
    gs->DecRef ();
  }
}

csCrossBuild_Quake2Importer::csCrossBuild_Quake2Importer()
      : localVFS (*(System->VFS) )
{
}


csCrossBuild_Quake2Importer::csCrossBuild_Quake2Importer(iVFS *specialVFS)
      : localVFS (*specialVFS)
{
}

csCrossBuild_Quake2Importer::~csCrossBuild_Quake2Importer()
{
}

iMeshObjectFactory *csCrossBuild_Quake2Importer::Import_Quake2File (
  char const*md2filebase, char const*skinpath, char const*modelname,
  iEngine *importdestination) const
{
#if 0
  iFile *modelfile = localVFS.Open(md2filebase,VFS_FILE_READ);

  csSpriteTemplate *newtemplate = 
  	Import_Quake2SpriteTemplate(modelfile);

  delete modelfile;

  // name this texture as appropriate
  newtemplate->SetName (modelname);

  /* make this template available for use in the given CS engine */
  importdestination->sprite_templates.Push(newtemplate);

  // arrange to load in any textures accompanying the model as skins
  char fakeskinpath[300];
  if (skinpath == NULL)
  {
    skinpath = fakeskinpath;
    strcpy(fakeskinpath,md2filebase);
    char *lastslash = fakeskinpath;
    char *walkpath = fakeskinpath;
    while (*walkpath)
    {
      if (*walkpath == '/') lastslash = walkpath;
      walkpath++;
    }
    lastslash = '\0';
  }

  iTextureWrapper *defaultskin = Import_Quake2Textures (skinpath,
    modelname, importdestination);
  
  newtemplate->SetTexture (importdestination->GetTextures (),
    CONST_CAST(char*, defaultskin->GetName()));

  newtemplate->GenerateLOD ();
  newtemplate->ComputeBoundingBox ();
#else
  (void) md2filebase;
  (void) skinpath;
  (void) modelname;
  (void) importdestination;
#endif

  return NULL;
}


iMeshObjectFactory *csCrossBuild_Quake2Importer::Import_Quake2MeshObjectFactory(
			iFile *modelfile) const
{
#if 0
  size_t modeldatasize = modelfile.GetSize();
  char *modelrawdata = new char [modeldatasize];
  if (modelfile.Read(modelrawdata,modeldatasize) == 0)
  {
    return NULL;
  }

  char *tempfilenamebase = tmpnam(NULL);
  char tempfilename[2000];
  sprintf(tempfilename,"%s.md2",tempfilenamebase);
  FILE *tempfile = fopen(tempfilename,"wb");
  fwrite(modelrawdata,modeldatasize,1,tempfile);
  delete [] modelrawdata;

  converter modeldata;
  modeldata.ivcon(tempfilename);
  fclose(tempfile);

  // now we have the data read in, make a sprite template
  csCrossBuild_SpriteTemplateFactory localfactory;
  csSpriteTemplate * newtemplate = (csSpriteTemplate *)localfactory.CrossBuild(modeldata);

  return newtemplate;
#else
  (void) modelfile;
#endif
  return NULL;
}

iTextureWrapper *csCrossBuild_Quake2Importer::Import_Quake2Textures (
  char const*skinpath, char const*modelname, iEngine *importdestination) const
{
  // go through and load all .pcx and .bmp files in the archive
  iStrVector *skinlist = localVFS.FindFiles(skinpath);
  int const skinfilecount = skinlist->Length();

  iTextureWrapper *defaulttexture = NULL;

  for (int skinfileindex = 0; skinfileindex < skinfilecount; skinfileindex++)
  {
    char *skinfilename = skinlist->Get(skinfileindex);

    // if the file name ends in .pcx, load it
    char *fileextension = skinfilename + strlen(skinfilename) - 3;
    if (   (strcasecmp("gif",fileextension) == 0) 
	|| (strcasecmp("bmp",fileextension) == 0)
       )
    {
      iDataBuffer *imagedata = localVFS.ReadFile(skinfilename);

      iImage *newskin = LoadImage(imagedata->GetUint8 (),
        imagedata->GetSize (), importdestination->GetTextureFormat ());

      imagedata->DecRef ();

      //if (!defaulttexture)
      defaulttexture = importdestination->GetTextureList ()->NewTexture (newskin);
      defaulttexture->Register (System->G3D->GetTextureManager ());

      printf("added texture %s...\n",skinfilename);

      const char *prefixstring = modelname;
      if (prefixstring)
      {
        char *prefixedname = new char[strlen(skinfilename)+strlen(prefixstring)+1];
        strcpy(prefixedname,prefixstring);
	strcat(prefixedname,skinfilename);
	defaulttexture->QueryObject ()->SetName (prefixedname);
	delete[] prefixedname;
      }
      defaulttexture->QueryObject ()->SetName (skinfilename);

    }
  } /* end for(int skinfileindex...) */

  skinlist->DecRef ();

  return defaulttexture;
}

/// given a prefix representing an action name, make a csSpriteAction
/// by concatinating all the frames that start with that prefix
iSpriteAction *  csCrossBuild_Quake2Importer::Make_NamedAction(
			iSprite3DFactoryState* frameholder,
			char const*prefixstring,
			int delay) const
{
  iSpriteAction *newaction = frameholder->AddAction();

  newaction->SetName(prefixstring);

  // check all the frame names, do any match?
  for (int frameindex=0; frameindex < frameholder->GetNumFrames(); frameindex++)
  {
    iSpriteFrame *curframe = frameholder->GetFrame(frameindex);
    const char *framename = curframe->GetName();
    if ( strncmp( prefixstring,framename,strlen(prefixstring) ) == 0 )
    {
      newaction->AddFrame(curframe,delay);
    }
  }

  return newaction;
}

/// build all the standard quake 2 actions, assuming the sprite
/// has frames with names that start with the proper action names
void              csCrossBuild_Quake2Importer::Build_Quake2Actions(
			iSprite3DFactoryState* frameholder) const
{
  static char const*actionnames[] = {
  	"stand", "run", "attack", "pain1", "pain2",
	"pain3", "jump", "flip",  "salute", "taunt",
	"wave", "point", "crstand", "crwalk", "crattack",
	"crpain", "crdeath", "death1", "death2", "death3", NULL };

  for (char const* const*actionname=actionnames; *actionname != NULL; actionname++)
  {
    Make_NamedAction(frameholder,*actionname,100);
    //printf("add action %s\n",*actionname);
  }
}
