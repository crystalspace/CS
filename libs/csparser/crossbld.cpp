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

#include "csparser/crossbld.h"

// build action list by concatenating all frames with names
// that begin with a certain string (the 'prefixstring')
static csSpriteAction *ivconmake_named_csSpriteAction
  (csSpriteTemplate &frameholder, char *prefixstring, int delay)
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

// build up standard quake2 named actions by glomming together frames that
// start with that action name
static void ivconbuild_Quake2Actions (csSpriteTemplate &frameholder)
{
  char *actionnames[] = {
  	"stand", "run", "attack", "pain1", "pain2",
	"pain3", "jump", "flip",  "salute", "taunt",
	"wave", "point", "crstand", "crwalk", "crattack",
	"crpain", "crdeath", "death1", "death2", "death3", NULL };

  for (char **actionname=actionnames; *actionname != NULL; actionname++)
  {
    ivconmake_named_csSpriteAction(frameholder,*actionname,100);
    //printf("add action %s\n",*actionname);
  }
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

  // if there exist quake2 actions, add them
  // --This is a temporary hack.  Model-specific stuff should and
  // will go into model-specific cross builder classes -GJH
  ivconbuild_Quake2Actions(*newtemplate);
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


