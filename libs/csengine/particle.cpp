/*
    Copyright (C) 2000 by Jorrit Tyberghein
    (C) W.C.A. Wijngaards, 2000

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
#include "csengine/meshobj.h"
#include "csengine/light.h"
#include "csengine/engine.h"
#include "csengine/particle.h"
#include "csengine/rview.h"
#include "csengine/sector.h"
#include "csgeom/matrix3.h"
#include "csgeom/fastsqrt.h"
#include "imspr2d.h"
#include <math.h>
#include <stdlib.h>

IMPLEMENT_CSOBJTYPE (csParticleSystem, csSprite)
IMPLEMENT_CSOBJTYPE (csNewtonianParticleSystem, csParticleSystem)


csParticleSystem::csParticleSystem (csObject* theParent)
  : csSprite (theParent), ptree_bbox (NULL)
{
  CONSTRUCT_IBASE (NULL);
  ptree_bbox.SetOwner (this);
  ptree_obj = &ptree_bbox;
  particles.SetLength(0);
  self_destruct = false;
  time_to_live = 0;
  to_delete = false;
  // add me to the engine
  csEngine::current_engine->sprites.Push (this);
  // defaults
  change_size = false;
  change_color = false;
  change_alpha = false;
  change_rotation = false;
  // bbox is empty.
  prev_time = 0;
  
  iMeshObjectType* type = QUERY_PLUGIN_CLASS (csEngine::System, "crystalspace.meshobj.spr2d",
      "MeshObj", iMeshObjectType);
  if (!type) type = LOAD_PLUGIN (csEngine::System, "crystalspace.meshobj.spr2d", "MeshObj", iMeshObjectType);
  spr_factory = type->NewFactory ();
}


csParticleSystem::~csParticleSystem()
{
  // delete all my particles
  for (int i=0 ; i < particles.Length() ; i++)
    if (particles[i])
      GetParticle (i)->DecRef ();
  spr_factory->DecRef ();
}

void csParticleSystem::UpdateInPolygonTrees ()
{
  ptree_bbox.RemoveFromTree ();

  // If we are not in a sector which has a polygon tree
  // then we don't really update. We should consider if this is
  // a good idea. Do we only want this object updated when we
  // want to use it in a polygon tree? It is certainly more
  // efficient to do it this way when the sprite is currently
  // moving in normal convex sectors.
  int i;
  csPolygonTree* tree = NULL;
  csVector& sects = movable.GetSectors ();
  for (i = 0 ; i < sects.Length () ; i++)
  {
    tree = ((csSector*)sects[i])->GetStaticTree ();
    if (tree) break;
  }
  if (!tree) return;

  // The bounding box for a particle system is world space.
  csBox3 b = GetBoundingBox ();
  ptree_bbox.Update (b, this);

  // Here we need to insert in trees where this sprite lives.
  for (i = 0 ; i < sects.Length () ; i++)
  {
    tree = ((csSector*)sects[i])->GetStaticTree ();
    if (tree)
    {
      // Temporarily increase reference to prevent free.
      ptree_bbox.GetBaseStub ()->IncRef ();
      tree->AddObject (&ptree_bbox);
      ptree_bbox.GetBaseStub ()->DecRef ();
    }
  }
}


void csParticleSystem :: AppendRectSprite(float width, float height, 
  csMaterialWrapper *mat, bool lighted)
{
  iParticle *pTicle;
  iMeshObject* sprmesh = spr_factory->NewInstance ();
  iParticle* part = QUERY_INTERFACE (sprmesh, iParticle);
  iSprite2DState* state = QUERY_INTERFACE (sprmesh, iSprite2DState);
  csColoredVertices& vs = state->GetVertices();

  vs.SetLimit(4);
  vs.SetLength(4);
  vs[0].pos.Set(-width,-height); vs[0].u=0.; vs[0].v=1.;
  vs[1].pos.Set(-width,+height); vs[1].u=0.; vs[1].v=0.;
  vs[2].pos.Set(+width,+height); vs[2].u=1.; vs[2].v=0.;
  vs[3].pos.Set(+width,-height); vs[3].u=1.; vs[3].v=1.;
  state->SetLighting( lighted );
  part->SetColor( csColor(1.0, 1.0, 1.0) );
  state->SetMaterialWrapper (QUERY_INTERFACE (mat, iMaterialWrapper));
  AppendParticle(pTicle = part);
  pTicle->DecRef ();
  part->DecRef(); 
  
}


void csParticleSystem :: AppendRegularSprite(int n, float radius, 
  csMaterialWrapper* mat, bool lighted)
{
  iParticle *pTicle;
  iMeshObject* sprmesh = spr_factory->NewInstance ();
  iParticle* part = QUERY_INTERFACE (sprmesh, iParticle);
  iSprite2DState* state = QUERY_INTERFACE (sprmesh, iSprite2DState);
  state->CreateRegularVertices(n, true);
  part->ScaleBy(radius);
  state->SetMaterialWrapper (QUERY_INTERFACE (mat, iMaterialWrapper));
  state->SetLighting( lighted );
  part->SetColor( csColor(1.0, 1.0, 1.0) );

  AppendParticle(pTicle = part);
  pTicle->DecRef ();
  part->DecRef(); 
}


void csParticleSystem :: SetMixmode(UInt mode)
{
  for(int i = 0; i<particles.Length(); i++)
    GetParticle(i)->SetMixmode(mode);
}


void csParticleSystem :: SetColor(const csColor& col)
{
  for(int i = 0; i<particles.Length(); i++)
    GetParticle(i)->SetColor(col);
}


void csParticleSystem :: AddColor(const csColor& col)
{
  for(int i = 0; i<particles.Length(); i++)
    GetParticle(i)->AddColor(col);
}


void csParticleSystem :: MoveToSector(csSector *sector)
{
  //for(int i = 0; i<particles.Length(); i++)
    //GetParticle(i)->MoveToSector(sector);
  csSprite::MoveToSector (sector);
}


void csParticleSystem :: SetPosition(const csVector3& pos)
{
  for(int i = 0; i<particles.Length(); i++)
    GetParticle(i)->SetPosition(pos);
}


void csParticleSystem :: MovePosition(const csVector3& move)
{
  for(int i = 0; i<particles.Length(); i++)
    GetParticle(i)->MovePosition(move);
}


void csParticleSystem :: ScaleBy(float factor)
{
  for(int i = 0; i<particles.Length(); i++)
    GetParticle(i)->ScaleBy(factor);
}


void csParticleSystem :: Rotate(float angle)
{
  for(int i = 0; i<particles.Length(); i++)
    GetParticle(i)->Rotate(angle);
}


void csParticleSystem :: Update(cs_time elapsed_time)
{
  if(self_destruct)
  {
    if(elapsed_time >= time_to_live)
    {
      to_delete = true;
      time_to_live = 0;
      /// and a calling virtual function can process without crashing
      return;
    }
    time_to_live -= elapsed_time;
  }
  float elapsed_seconds = ((float)elapsed_time) / 1000.0;
  if(change_color)
    AddColor(colorpersecond * elapsed_seconds);
  if(change_size)
    ScaleBy(pow(scalepersecond, elapsed_seconds));
  if(change_alpha)
  {
    alpha_now += alphapersecond * elapsed_seconds;
    if(alpha_now < 0.0f) alpha_now = 0.0f;
    else if(alpha_now > 1.0f) alpha_now = 1.0f;
    SetMixmode(CS_FX_SETALPHA(alpha_now));
  }
  if(change_rotation)
    Rotate(anglepersecond * elapsed_seconds);
}

void csParticleSystem::Draw (csRenderView& rview)
{
  UpdateDeferedLighting (movable.GetPosition ());
  iRenderView* irview = QUERY_INTERFACE (&rview, iRenderView);
  for (int i = 0; i<particles.Length(); i++)
    GetParticle(i)->Draw (irview);
}

void csParticleSystem::UpdateLighting (csLight** lights, int num_lights)
{
  defered_num_lights = 0;
  // @@@ NOT EFFICIENT!!!
  iLight** ilights = new iLight*[num_lights];
  int i;
  for (i = 0 ; i < num_lights ; i++)
    ilights[i] = QUERY_INTERFACE (lights[i], iLight);
  for (i = 0; i<particles.Length(); i++)
    GetParticle(i)->UpdateLighting (ilights, num_lights);
  delete[] ilights;
}

void csParticleSystem::DeferUpdateLighting (int flags, int num_lights)
{
  csSprite::DeferUpdateLighting (flags, num_lights);
}

const csVector3& csParticleSystem::GetPosition () const
{
  return movable.GetPosition ();
}

//---------------------------------------------------------------------

csVector3& csParticleSystem::GetRandomDirection ()
{
  static csVector3 dir;
  dir.x = 2.0 * rand() / (1.0+RAND_MAX) - 1.0;
  dir.y = 2.0 * rand() / (1.0+RAND_MAX) - 1.0;
  dir.z = 2.0 * rand() / (1.0+RAND_MAX) - 1.0;
  return dir;
}

csVector3& csParticleSystem::GetRandomDirection (const csVector3& magnitude,
	const csVector3& offset)
{
  static csVector3 dir;
  dir.x = (rand() / (1.0+RAND_MAX)) * magnitude.x;
  dir.y = (rand() / (1.0+RAND_MAX)) * magnitude.y;
  dir.z = (rand() / (1.0+RAND_MAX)) * magnitude.z;
  dir = dir + offset;
  return dir;
}

//-- csNewtonianParticleSystem ------------------------------------------

csNewtonianParticleSystem :: csNewtonianParticleSystem(csObject* theParent, int max)
  : csParticleSystem(theParent)
{
  // create csVector3's
  part_speed = new csVector3 [max];
  part_accel = new csVector3 [max];
}


csNewtonianParticleSystem :: ~csNewtonianParticleSystem()
{
  delete[] part_speed;
  delete[] part_accel;
}


void csNewtonianParticleSystem :: Update(cs_time elapsed_time)
{
  csVector3 move;
  csParticleSystem::Update(elapsed_time);
  // time passed; together with CS 1 unit = 1 meter makes units right.
  float delta_t = elapsed_time / 1000.0f; // in seconds
  for(int i=0; i<particles.Length(); i++)
  {
    // notice that the ordering of the lines (1) and (2) makes the
    // resulting newpos = a*dt^2 + v*dt + oldposition (i.e. paraboloid).
    part_speed[i] += part_accel[i] * delta_t; // (1)
    move = part_speed[i] * delta_t; // (2)
    GetParticle(i)->MovePosition (move); 
  }
}

