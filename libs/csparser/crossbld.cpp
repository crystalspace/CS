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
#include "csengine/engine.h"
#include "csengine/texture.h"
#include "csengine/thing.h"
#include "csengine/polygon.h"
#include "csgfx/csimage.h"
#include "cssys/system.h"
#include "csutil/scfstrv.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "ivideo/txtmgr.h"
#include "imesh/sprite3d.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "igraphic/loader.h"

// helper function for image loading

iImage *LoadImage (UByte* iBuffer, ULong iSize, int iFormat)
{
  iImageLoader *ImageLoader =
    QUERY_PLUGIN_ID (System, CS_FUNCID_IMGLOADER, iImageLoader);
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
csTextureWrapper *ivconload_Quake2Textures(csEngine *engine,Archive &pakarchive,char const*prefixstring)
{
  // go through and load all .pcx and .bmp files in the archive
  void *currententry = pakarchive.first_file();

  csTextureWrapper *defaulttexture = NULL;

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
void *csCrossBuild_ThingTemplateFactory::CrossBuild(converter &buildsource)
{
  csThing *newtemplate = new csThing ();
  CrossBuild ((void*)newtemplate, buildsource);
  return newtemplate;
}

/// full thing template builder (second variant)
void csCrossBuild_ThingTemplateFactory::CrossBuild(void* object, converter &buildsource)
{
  csThing *newtemplate = (csThing*)object;
  buildsource.set_animation_frame(0);

  // Add the vertices
  Add_Vertices(*newtemplate, buildsource);

  // Build the triangle mesh
  Build_TriangleMesh(*newtemplate, buildsource);
}

/// frame build method
void csCrossBuild_ThingTemplateFactory::Add_Vertices (csThing &framesource, converter& buildsource)
{
  for (int coordindex=0; coordindex<buildsource.num_cor3; coordindex++)
  {
    // standard 3D coords seem to swap y and z axis compared to CS
    framesource.AddVertex(
    		buildsource.cor3[0][coordindex]/20.0,
    		buildsource.cor3[2][coordindex]/20.0,
    		buildsource.cor3[1][coordindex]/20.0);
  }
}


/// triangle mesh builder
void csCrossBuild_ThingTemplateFactory::Build_TriangleMesh(csThing& meshsource,converter& buildsource)
{
  for (int triangleindex=0; triangleindex<buildsource.num_face; triangleindex++)
  {
    char buf[10];
    sprintf (buf, "t%d", triangleindex);
    csPolygon3D* ptemp = meshsource.NewPolygon (NULL); //@@@### MATERIAL WRAPPER
    int a = buildsource.face[0][triangleindex];
    int b = buildsource.face[1][triangleindex];
    int c = buildsource.face[2][triangleindex];
    ptemp->AddVertex (a);
    ptemp->AddVertex (b);
    ptemp->AddVertex (c);
    ptemp->SetTextureType (POLYTXT_GOURAUD);
    csPolyTexGouraud* gs = ptemp->GetGouraudInfo ();
    gs->SetUV (0, buildsource.cor3_uv[0][a], buildsource.cor3_uv[1][a]);
    gs->SetUV (1, buildsource.cor3_uv[0][b], buildsource.cor3_uv[1][b]);
    gs->SetUV (2, buildsource.cor3_uv[0][c], buildsource.cor3_uv[1][c]);
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
  csEngine *importdestination) const
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

  csTextureWrapper *defaultskin = Import_Quake2Textures (skinpath,
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

csTextureWrapper *csCrossBuild_Quake2Importer::Import_Quake2Textures (
  char const*skinpath, char const*modelname, csEngine *importdestination) const
{
  // go through and load all .pcx and .bmp files in the archive
  iStrVector *skinlist = localVFS.FindFiles(skinpath);
  int const skinfilecount = skinlist->Length();

  csTextureWrapper *defaulttexture = NULL;

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
      defaulttexture = importdestination->GetTextures()->NewTexture (newskin);
      defaulttexture->Register (System->G3D->GetTextureManager ());

      printf("added texture %s...\n",skinfilename);

      const char *prefixstring = modelname;
      if (prefixstring)
      {
        char *prefixedname = new char[strlen(skinfilename)+strlen(prefixstring)+1];
        strcpy(prefixedname,prefixstring);
	strcat(prefixedname,skinfilename);
	defaulttexture->SetName (prefixedname);
	delete[] prefixedname;
      }
      defaulttexture->SetName (skinfilename);

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
