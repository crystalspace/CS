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
// can be used in a Crystal Space world.

#include "sysdef.h"
#include "csparser/crossbld.h"
#include "csengine/world.h"
#include "csgfxldr/csimage.h"
#include "cssys/system.h"
#include "csutil/scfstrv.h"
#include "itexture.h"
#include "itxtmgr.h"

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
csBase *csCrossBuild_SpriteTemplateFactory::CrossBuild (converter &buildsource)
{
  CHK (csSpriteTemplate *newtemplate = new csSpriteTemplate());
  CrossBuild ((csBase*)newtemplate, buildsource);
  newtemplate->GenerateLOD ();
  newtemplate->ComputeBoundingBox ();
  return newtemplate;
}

/// full sprite template builder (second variant)
void csCrossBuild_SpriteTemplateFactory::CrossBuild (csBase* object,
  converter &buildsource)
{
  csSpriteTemplate *newtemplate = (csSpriteTemplate*)object;
  buildsource.set_animation_frame(0);

  newtemplate->SetNumVertices(buildsource.num_cor3);

  // build the triangle mesh
  Build_TriangleMesh(*newtemplate, buildsource);

  // build all the frames
  for (int frameindex=0; 
           frameindex<=buildsource.set_animation_frame(frameindex); 
	   frameindex++)
  {
    Build_Frame(*newtemplate, buildsource);
  }

  // make a 'default' action showing the first frame, if any
  if (newtemplate->GetFrame (0) != NULL)
  {
    csSpriteAction *defaultaction = newtemplate->AddAction();
    defaultaction->SetName("default");
    defaultaction->AddFrame( newtemplate->GetFrame(0), 1000 );
  }

}

/// frame build method
void csCrossBuild_SpriteTemplateFactory::Build_Frame
  (csSpriteTemplate &framesource, converter& buildsource)
{
  csFrame *newframe = framesource.AddFrame();

  newframe->SetName(buildsource.object_name);

  for (int coordindex=0; coordindex<buildsource.num_cor3; coordindex++)
  {
    // standard 3D coords seem to swap y and z axis compared to CS
    newframe->SetVertex(coordindex,
      buildsource.cor3[0][coordindex]/20.0,
      buildsource.cor3[2][coordindex]/20.0,
      buildsource.cor3[1][coordindex]/20.0);
    newframe->SetTexel(coordindex,
      buildsource.cor3_uv[0][coordindex],
      buildsource.cor3_uv[1][coordindex]);
  }
}


/// triangle mesh builder
void csCrossBuild_SpriteTemplateFactory::Build_TriangleMesh(csSpriteTemplate& meshsource,converter& buildsource)
{
  // get the mesh from the sprite template
  csTriangleMesh *targetmesh = meshsource.GetBaseMesh();

  for (int triangleindex=0; triangleindex<buildsource.num_face; triangleindex++)
  {
    int a = buildsource.face[0][triangleindex];
    int b = buildsource.face[1][triangleindex];
    int c = buildsource.face[2][triangleindex];
    targetmesh->AddTriangle(a,b,c);
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
  CHK (delete [] modelrawdata);

  return newtemplate;
}


// load textures from a quake-pakked file.  All textures will have
// their names prefixed by the prefixstring if it is non-NULL
// returns the 'default' texture for the sprite in this file
csTextureHandle *ivconload_Quake2Textures(csWorld *world,Archive &pakarchive,char *prefixstring)
{
  // go through and load all .pcx and .bmp files in the archive
  void *currententry = pakarchive.first_file();

  csTextureHandle *defaulttexture = NULL;

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
      CHK (delete [] skinrawdata);
      fclose(tempfile);

      // convert to a gif which we CAN read
      char buffer[1024];
      sprintf(buffer,"convert %s %s.gif",tempfilename,tempfilename);
      system(buffer);

      // read in the converted skin
      sprintf(buffer,"%s.gif",tempfilename);
      tempfile = fopen(buffer,"rb");
      ImageFile *newskin = ImageLoader::load(tempfile);
      fclose(tempfile);
      unlink(buffer);
      unlink(tempfilename);

      if (!defaulttexture)
        defaulttexture = world->GetTextures()->NewTexture(newskin);
/*
      iTextureManager *txtmgr = System->G3D->GetTextureManager ();
      iTextureHandle *th = txtmgr->RegisterTexture(defaulttexture->GetImageFile(),
        defaulttexture->for_3d, defaulttexture->for_2d);
      defaulttexture->SetTextureHandle(th);
      */

      printf("added texture %s...\n",filename);

      if (prefixstring)
      {
        CHK (char *prefixedname = new char[strlen(filename)+strlen(prefixstring)+1]);
        strcpy(prefixedname,prefixstring);
	strcat(prefixedname,filename);
	defaulttexture->SetName (prefixedname);
	CHK (delete[] prefixedname);
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
csBase *csCrossBuild_ThingTemplateFactory::CrossBuild(converter &buildsource)
{
  CHK (csThingTemplate *newtemplate = new csThingTemplate());
  CrossBuild ((csBase*)newtemplate, buildsource);
  return newtemplate;
}

/// full thing template builder (second variant)
void csCrossBuild_ThingTemplateFactory::CrossBuild(csBase* object, converter &buildsource)
{
  csThingTemplate *newtemplate = (csThingTemplate*)object;
  buildsource.set_animation_frame(0);

  // Add the vertices
  Add_Vertices(*newtemplate, buildsource);

  // Build the triangle mesh
  Build_TriangleMesh(*newtemplate, buildsource);
}

/// frame build method
void csCrossBuild_ThingTemplateFactory::Add_Vertices (csThingTemplate &framesource, converter& buildsource)
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
void csCrossBuild_ThingTemplateFactory::Build_TriangleMesh(csThingTemplate& meshsource,converter& buildsource)
{
  for (int triangleindex=0; triangleindex<buildsource.num_face; triangleindex++)
  {
    char buf[10];
    sprintf (buf, "t%d", triangleindex);
    CHK (csPolygonTemplate* ptemp = new csPolygonTemplate (&meshsource, buf));
    int a = buildsource.face[0][triangleindex];
    int b = buildsource.face[1][triangleindex];
    int c = buildsource.face[2][triangleindex];
    ptemp->AddVertex (a);
    ptemp->AddVertex (b);
    ptemp->AddVertex (c);
    ptemp->SetUV (0, buildsource.cor3_uv[0][a], buildsource.cor3_uv[1][a]);
    ptemp->SetUV (1, buildsource.cor3_uv[0][b], buildsource.cor3_uv[1][b]);
    ptemp->SetUV (2, buildsource.cor3_uv[0][c], buildsource.cor3_uv[1][c]);
    ptemp->SetGouraud ();
    meshsource.AddPolygon (ptemp);
  }
}

csCrossBuild_Quake2Importer::csCrossBuild_Quake2Importer()
      : localVFS (*(System->VFS) )
{
}


csCrossBuild_Quake2Importer::csCrossBuild_Quake2Importer(csVFS &specialVFS)
      : localVFS (specialVFS)
{
}

csCrossBuild_Quake2Importer::~csCrossBuild_Quake2Importer()
{
}

csSpriteTemplate *csCrossBuild_Quake2Importer::Import_Quake2File(
			char *md2filebase,
			char *skinpath,
			char *modelname,
			csWorld *importdestination) const
{
#if 0
  csFile *modelfile = localVFS.Open(md2filebase,VFS_FILE_READ);

  csSpriteTemplate *newtemplate = 
  	Import_Quake2SpriteTemplate(*modelfile);

  delete modelfile;

  // name this texture as appropriate
  newtemplate->SetName (modelname);

  /* make this template available for use in the given CS world */
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

  csTextureHandle *defaultskin = Import_Quake2Textures(skinpath,modelname,importdestination);
  
  newtemplate->SetTexture(importdestination->GetTextures (),
    CONST_CAST(char*)(defaultskin->GetName ()));

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


csSpriteTemplate *csCrossBuild_Quake2Importer::Import_Quake2SpriteTemplate(
			csFile &modelfile) const
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
  CHK (delete [] modelrawdata);

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

csTextureHandle * csCrossBuild_Quake2Importer::Import_Quake2Textures(
			char *skinpath,
			char *modelname,
			csWorld *importdestination) const
{
  // go through and load all .pcx and .bmp files in the archive
  iStrVector *skinlist = localVFS.FindFiles(skinpath);
  int const skinfilecount = skinlist->Length();

  csTextureHandle *defaulttexture = NULL;

  for (int skinfileindex = 0; skinfileindex < skinfilecount; skinfileindex++)
  {
    char *skinfilename = skinlist->Get(skinfileindex);

    // if the file name ends in .pcx, load it
    char *fileextension = skinfilename + strlen(skinfilename) - 3;
    if (   (strcasecmp("gif",fileextension) == 0) 
	|| (strcasecmp("bmp",fileextension) == 0)
       )
    {
      size_t imagefilesize;
      char *imagedata = localVFS.ReadFile(skinfilename,imagefilesize);

      csImageFile *newskin = ImageLoader::load((unsigned char *)imagedata,imagefilesize);

      CHK (delete [] imagedata);

      //if (!defaulttexture)
      defaulttexture = importdestination->GetTextures()->NewTexture(newskin);

      iTextureManager *txtmgr = System->G3D->GetTextureManager ();
      iTextureHandle *th = txtmgr->RegisterTexture (defaulttexture->GetImageFile (),
        defaulttexture->for_3d, defaulttexture->for_2d);
      defaulttexture->SetTextureHandle (th);

      printf("added texture %s...\n",skinfilename);

      char *prefixstring = modelname;
      if (prefixstring)
      {
        CHK (char *prefixedname = new char[strlen(skinfilename)+strlen(prefixstring)+1]);
        strcpy(prefixedname,prefixstring);
	strcat(prefixedname,skinfilename);
	defaulttexture->SetName (prefixedname);
	CHK (delete[] prefixedname);
      }
      defaulttexture->SetName (skinfilename);

    }
  } /* end for(int skinfileindex...) */

  skinlist->DecRef ();

  return defaulttexture;
}

/// given a prefix representing an action name, make a csSpriteAction
/// by concatinating all the frames that start with that prefix
csSpriteAction *  csCrossBuild_Quake2Importer::Make_NamedAction(
			csSpriteTemplate &frameholder,
			char *prefixstring,
			int delay) const
{
  csSpriteAction *newaction = frameholder.AddAction();

  newaction->SetName(prefixstring);

  // check all the frame names, do any match?
  for (int frameindex=0; frameindex < frameholder.GetNumFrames(); frameindex++)
  {
    csFrame *curframe = frameholder.GetFrame(frameindex);
    char *framename = curframe->GetName();
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
			csSpriteTemplate &frameholder) const
{
  static char *actionnames[] = {
  	"stand", "run", "attack", "pain1", "pain2",
	"pain3", "jump", "flip",  "salute", "taunt",
	"wave", "point", "crstand", "crwalk", "crattack",
	"crpain", "crdeath", "death1", "death2", "death3", NULL };

  for (char **actionname=actionnames; *actionname != NULL; actionname++)
  {
    Make_NamedAction(frameholder,*actionname,100);
    //printf("add action %s\n",*actionname);
  }
}


