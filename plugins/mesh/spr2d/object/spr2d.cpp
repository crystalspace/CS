/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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
#include "csgeom/math.h"
#include "csgfx/renderbuffer.h"
#include "csgeom/math3d.h"
#include "csgeom/math2d.h"
#include "csgeom/box.h"
#include "csgeom/transfrm.h"
#include "cstool/rbuflock.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "iengine/camera.h"
#include "igeom/clip2d.h"
#include "iengine/engine.h"
#include "iengine/light.h"
#include "iutil/objreg.h"
#include "spr2d.h"
#include "csqsqrt.h"

CS_LEAKGUARD_IMPLEMENT (csSprite2DMeshObject);
CS_LEAKGUARD_IMPLEMENT (csSprite2DMeshObjectFactory);

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csSprite2DMeshObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iSprite2DState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iParticle)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSprite2DMeshObject::ObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSprite2DMeshObject::Sprite2DState)
  SCF_IMPLEMENTS_INTERFACE (iSprite2DState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSprite2DMeshObject::Particle)
  SCF_IMPLEMENTS_INTERFACE (iParticle)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSprite2DMeshObject::eiRenderBufferAccessor)
  SCF_IMPLEMENTS_INTERFACE (iRenderBufferAccessor)
SCF_IMPLEMENT_IBASE_END

csSprite2DMeshObject::csSprite2DMeshObject (csSprite2DMeshObjectFactory* factory)
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSprite2DState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiParticle);
  csSprite2DMeshObject::factory = factory;
  logparent = 0;
  ifactory = SCF_QUERY_INTERFACE (factory, iMeshObjectFactory);
  material = factory->GetMaterialWrapper ();
  lighting = factory->HasLighting ();
  MixMode = factory->GetMixMode ();
  initialized = false;
  vis_cb = 0;
  current_lod = 1;
  current_features = 0;
  uvani = 0;

  vertices_dirty = true;
  texels_dirty = true;
  colors_dirty = true;
  indicesSize = (size_t)-1;
}

csSprite2DMeshObject::~csSprite2DMeshObject ()
{
  if (vis_cb) vis_cb->DecRef ();
  delete uvani;
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiSprite2DState);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiParticle);
  SCF_DESTRUCT_IBASE ();
}

void csSprite2DMeshObject::SetupObject ()
{
  if (!initialized)
  {
    initialized = true;
    float max_sq_dist = 0;
    size_t i;
    bbox_2d.StartBoundingBox(vertices[0].pos);
    for (i = 0 ; i < vertices.Length () ; i++)
    {
      csSprite2DVertex& v = vertices[i];
      bbox_2d.AddBoundingVertexSmart(v.pos);
      if (!lighting)
      {
        // If there is no lighting then we need to copy the color_init
        // array to color.
        v.color = vertices[i].color_init;
        v.color.Clamp (2, 2, 2);
      }
      float sqdist = v.pos.x*v.pos.x + v.pos.y*v.pos.y;
      if (sqdist > max_sq_dist) max_sq_dist = sqdist;
    }
    float max_dist = csQsqrt (max_sq_dist);
    radius.Set (max_dist, max_dist, max_dist);

    bufferHolder.AttachNew (new csRenderBufferHolder);
    csRef<iRenderBufferAccessor> newAccessor;
    newAccessor.AttachNew (new eiRenderBufferAccessor (this));
    bufferHolder->SetAccessor (newAccessor, CS_BUFFER_ALL_MASK);
  }
}

static csVector3 cam;

void csSprite2DMeshObject::UpdateLighting (const csArray<iLight*>& lights,
    const csVector3& pos)
{
  if (!lighting) return;
  csColor color (0, 0, 0);

  // @@@ GET AMBIENT
  //csSector* sect = movable.GetSector (0);
  //if (sect)
  //{
    //int r, g, b;
    //sect->GetAmbientColor (r, g, b);
    //color.Set (r / 128.0, g / 128.0, b / 128.0);
  //}

  int i;
  int num_lights = lights.Length ();
  for (i = 0; i < num_lights; i++)
  {
    csColor light_color = lights[i]->GetColor () * (256. / CS_NORMAL_LIGHT_LEVEL);
    float sq_light_radius = csSquare (lights [i]->GetCutoffDistance ());
    // Compute light position.
    csVector3 wor_light_pos = lights [i]->GetCenter ();
    float wor_sq_dist =
      csSquaredDist::PointPoint (wor_light_pos, pos);
    if (wor_sq_dist >= sq_light_radius) continue;
    float wor_dist = csQsqrt (wor_sq_dist);
    float cosinus = 1.;
    cosinus /= wor_dist;
    light_color *= cosinus * lights [i]->GetBrightnessAtDistance (wor_dist);
    color += light_color;
  }
  for (size_t j = 0 ; j < vertices.Length () ; j++)
  {
    vertices[j].color = vertices[j].color_init + color;
    vertices[j].color.Clamp (2, 2, 2);
  }
  colors_dirty = true;

}

void csSprite2DMeshObject::UpdateLighting (const csArray<iLight*>& lights,
    iMovable* movable, csVector3 offset)
{
  if (!lighting) return;
  csVector3 pos = movable->GetFullPosition ();
  UpdateLighting (lights, pos + offset);
}

csRenderMesh** csSprite2DMeshObject::GetRenderMeshes (int &n, 
						      iRenderView* rview, 
						      iMovable* movable, 
						      uint32 frustum_mask,
						      csVector3 offset)
{
  SetupObject ();

  iCamera* camera = rview->GetCamera ();

  // Camera transformation for the single 'position' vector.
  cam = rview->GetCamera ()->GetTransform ().Other2This (
  	movable->GetFullPosition () + offset);
  if (cam.z < SMALL_Z) 
  {
    n = 0;
    return 0;
  }

  if (factory->light_mgr)
  {
    const csArray<iLight*>& relevant_lights = factory->light_mgr
    	->GetRelevantLights (logparent, -1, false);
    UpdateLighting (relevant_lights, movable, offset);
  }

  csReversibleTransform temp = camera->GetTransform ();
  if (!movable->IsFullTransformIdentity ())
    temp /= movable->GetFullTransform ();

  int clip_portal, clip_plane, clip_z_plane;
  rview->CalculateClipSettings (frustum_mask, clip_portal, clip_plane, 
    clip_z_plane);
  csVector3 camera_origin = temp.GetT2OTranslation ();

  csReversibleTransform tr_o2c;
  tr_o2c.SetO2TTranslation (-temp.Other2This (offset));

  bool meshCreated;
  csRenderMesh*& rm = rmHolder.GetUnusedMesh (meshCreated,
    rview->GetCurrentFrameNumber ());
  if (meshCreated)
  {
    rm->meshtype = CS_MESHTYPE_TRIANGLEFAN;
    rm->material = material;
    rm->buffers = bufferHolder;
    rm->geometryInstance = this;
  }
  
  rm->mixmode = MixMode;
  rm->clip_portal = clip_portal;
  rm->clip_plane = clip_plane;
  rm->clip_z_plane = clip_z_plane;
  rm->do_mirror = false/* camera->IsMirrored () */; 
    /*
      Force to false as the front-face culling will let the sprite 
      disappear. 
     */
  rm->indexstart = 0;
  rm->indexend = vertices.Length();
  rm->object2camera = tr_o2c;
  rm->camera_origin = camera_origin;

  n = 1; 
  return &rm; 
}

void csSprite2DMeshObject::PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer)
{
  if (!holder) return;

  if (buffer == CS_BUFFER_INDEX)
  {
    size_t indexSize = vertices.Length();
    if (!index_buffer.IsValid() || 
      (indicesSize != indexSize))
    {
      index_buffer = csRenderBuffer::CreateIndexRenderBuffer (
	indexSize, CS_BUF_DYNAMIC, 
	CS_BUFCOMP_UNSIGNED_INT, 0, vertices.Length() - 1);
      
      holder->SetRenderBuffer (CS_BUFFER_INDEX, index_buffer);

      csRenderBufferLock<uint> indexLock (index_buffer);
      uint* ptr = indexLock;

      for (size_t i = 0; i < vertices.Length(); i++)
      {
	*ptr++ = (uint)i;
      }
      indicesSize = indexSize;
    }
  }
  else if (buffer == CS_BUFFER_TEXCOORD0)
  {
    if (texels_dirty)
    {
      int texels_count;
      const csVector2 *uvani_uv = 0;
      if (!uvani)
	texels_count = vertices.Length ();
      else
	uvani_uv = uvani->GetVertices (texels_count);
	  
      size_t texelSize = texels_count;
      if (!texel_buffer.IsValid() || (texel_buffer->GetSize()
      	!= texelSize * sizeof(float) * 2))
      {
	texel_buffer = csRenderBuffer::CreateRenderBuffer (
	  texelSize, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2);
	holder->SetRenderBuffer (CS_BUFFER_TEXCOORD0, texel_buffer);
      }

      csRenderBufferLock<csVector2> texelLock (texel_buffer);

      for (size_t i = 0; i < (size_t)texels_count; i++)
      {
	csVector2& v = texelLock[i];
	if (!uvani)
	{
	  v.x = vertices[i].u;
	  v.y = vertices[i].v;
	}
	else
	{
	  v.x = uvani_uv[i].x;
	  v.y = uvani_uv[i].y;
	}
      }
      texels_dirty = false;
    }
  }
  else if (buffer == CS_BUFFER_COLOR)
  {
    if (colors_dirty)
    {
      size_t color_size = vertices.Length();
      if (!color_buffer.IsValid() || (color_buffer->GetSize() 
	!= color_size * sizeof(float) * 2))
      {
	color_buffer = csRenderBuffer::CreateRenderBuffer (
	  color_size, CS_BUF_STATIC, 
	  CS_BUFCOMP_FLOAT, 3);
	holder->SetRenderBuffer (CS_BUFFER_COLOR, color_buffer);
      }

      csRenderBufferLock<csColor> colorLock (color_buffer);

      for (size_t i = 0; i < vertices.Length(); i++)
      {
	colorLock[i] = vertices[i].color;
      }
      colors_dirty = false;
    }
  }
  else if (buffer == CS_BUFFER_POSITION)
  {
    if (vertices_dirty)
    {
      size_t vertices_size = vertices.Length();
      if (!vertex_buffer.IsValid() || (vertex_buffer->GetSize() 
	!= vertices_size * sizeof(float) * 3))
      {
	vertex_buffer = csRenderBuffer::CreateRenderBuffer (
	  vertices_size, CS_BUF_STATIC, 
	  CS_BUFCOMP_FLOAT, 3);
	holder->SetRenderBuffer (CS_BUFFER_POSITION, vertex_buffer);
      }

      csRenderBufferLock<csVector3> vertexLock (vertex_buffer);

      for (size_t i = 0; i < vertices.Length(); i++)
      {
	vertexLock[i].Set (vertices[i].pos.x, vertices[i].pos.y, 0.0f);
      }
      vertices_dirty = false;
    }
  }
}

void csSprite2DMeshObject::GetObjectBoundingBox (csBox3& bbox)
{
  SetupObject ();
  bbox.Set (-radius, radius);
}

void csSprite2DMeshObject::SetObjectBoundingBox (const csBox3&)
{
  // @@@ TODO
}

void csSprite2DMeshObject::HardTransform (const csReversibleTransform& t)
{
  (void)t;
  //@@@ TODO
}

void csSprite2DMeshObject::CreateRegularVertices (int n, bool setuv)
{
  double angle_inc = TWO_PI / n;
  double angle = 0.0;
  vertices.SetLength (n);
  size_t i;
  for (i = 0; i < vertices.Length (); i++, angle += angle_inc)
  {
    vertices [i].pos.y = cos (angle);
    vertices [i].pos.x = sin (angle);
    if (setuv)
    {
      // reuse sin/cos values and scale to [0..1]
      vertices [i].u = vertices [i].pos.x / 2.0f + 0.5f;
      vertices [i].v = vertices [i].pos.y / 2.0f + 0.5f;
    }
    vertices [i].color.Set (1, 1, 1);
    vertices [i].color_init.Set (1, 1, 1);
  }

  vertices_dirty = true;
  texels_dirty = true;
  colors_dirty = true;

  scfiObjectModel.ShapeChanged ();
}

void csSprite2DMeshObject::NextFrame (csTicks current_time, const csVector3& /*pos*/)
{
  if (uvani && !uvani->halted)
  {
    int old_frame_index = uvani->frameindex;
    uvani->Advance (current_time);
    texels_dirty |= (old_frame_index != uvani->frameindex);
  }
}

void csSprite2DMeshObject::Particle::UpdateLighting (const csArray<iLight*>& lights,
    const csReversibleTransform& transform)
{
  csVector3 new_pos = transform.This2Other (part_pos);
  scfParent->UpdateLighting (lights, new_pos);
}

   
csRenderMesh** csSprite2DMeshObject::Particle::GetRenderMeshes (int& n, 
	iRenderView* rview, iMovable* movable, uint32 frustum_mask)
{
  return scfParent->GetRenderMeshes (n, rview, movable, frustum_mask, part_pos);
}

void csSprite2DMeshObject::Particle::SetColor (const csColor& col)
{
  csColoredVertices& vertices = scfParent->GetVertices ();
  size_t i;
  for (i = 0 ; i < vertices.Length () ; i++)
    vertices[i].color_init = col;
  if (!scfParent->lighting)
    for (i = 0 ; i < vertices.Length () ; i++)
      vertices[i].color = col;

  scfParent->colors_dirty = true;

}

void csSprite2DMeshObject::Particle::AddColor (const csColor& col)
{
  csColoredVertices& vertices = scfParent->GetVertices ();
  size_t i;
  for (i = 0 ; i < vertices.Length () ; i++)
    vertices[i].color_init += col;
  if (!scfParent->lighting)
    for (i = 0 ; i < vertices.Length () ; i++)
      vertices[i].color = vertices[i].color_init;

  scfParent->colors_dirty = true;

}

void csSprite2DMeshObject::Particle::ScaleBy (float factor)
{
  csColoredVertices& vertices = scfParent->GetVertices ();
  size_t i;
  for (i = 0; i < vertices.Length (); i++)
    vertices[i].pos *= factor;

  scfParent->vertices_dirty = true;

  scfParent->scfiObjectModel.ShapeChanged ();
}

void csSprite2DMeshObject::Particle::Rotate (float angle)
{
  csColoredVertices& vertices = scfParent->GetVertices ();
  size_t i;
  for (i = 0; i < vertices.Length (); i++)
    vertices[i].pos.Rotate (angle);

  scfParent->vertices_dirty = true;

  scfParent->scfiObjectModel.ShapeChanged ();
}

void csSprite2DMeshObject::Sprite2DState::SetUVAnimation (const char *name,
	int style, bool loop)
{
  if (name)
  {
    iSprite2DUVAnimation *ani = scfParent->factory->GetUVAnimation (name);
    if (ani && ani->GetFrameCount ())
    {
      scfParent->uvani = new uvAnimationControl ();
      scfParent->uvani->ani = ani;
      scfParent->uvani->last_time = 0;
      scfParent->uvani->frameindex = 0;
      scfParent->uvani->framecount = ani->GetFrameCount ();
      scfParent->uvani->frame = ani->GetFrame (0);
      scfParent->uvani->style = style;
      scfParent->uvani->counter = 0;
      scfParent->uvani->loop = loop;
      scfParent->uvani->halted = false;
    }
  }
  else
  {
    // stop animation and show the normal texture
    delete scfParent->uvani;
    scfParent->uvani = 0;
  }
}

void csSprite2DMeshObject::Sprite2DState::StopUVAnimation (int idx)
{
  if (scfParent->uvani)
  {
    if (idx != -1)
    {
      scfParent->uvani->frameindex = MIN(MAX(idx, 0),
      	scfParent->uvani->framecount-1);
      scfParent->uvani->frame = scfParent->uvani->ani->GetFrame (
      	scfParent->uvani->frameindex);
    }
    scfParent->uvani->halted = true;
  }
}

void csSprite2DMeshObject::Sprite2DState::PlayUVAnimation (int idx, int style, bool loop)
{
  if (scfParent->uvani)
  {
    if (idx != -1)
    {
      scfParent->uvani->frameindex = MIN(MAX(idx, 0), scfParent->uvani->framecount-1);
      scfParent->uvani->frame = scfParent->uvani->ani->GetFrame (scfParent->uvani->frameindex);
    }
    scfParent->uvani->halted = false;
    scfParent->uvani->counter = 0;
    scfParent->uvani->last_time = 0;
    scfParent->uvani->loop = loop;
    scfParent->uvani->style = style;
  }
}

int csSprite2DMeshObject::Sprite2DState::GetUVAnimationCount () const
{
  return scfParent->factory->GetUVAnimationCount ();
}

iSprite2DUVAnimation *csSprite2DMeshObject::Sprite2DState::CreateUVAnimation ()
{
  return scfParent->factory->CreateUVAnimation ();
}

void csSprite2DMeshObject::Sprite2DState::RemoveUVAnimation (
	iSprite2DUVAnimation *anim)
{
  scfParent->factory->RemoveUVAnimation (anim);
}

iSprite2DUVAnimation *csSprite2DMeshObject::Sprite2DState::GetUVAnimation (
	const char *name) const
{
  return scfParent->factory->GetUVAnimation (name);
}

iSprite2DUVAnimation *csSprite2DMeshObject::Sprite2DState::GetUVAnimation (
	int idx) const
{
  return scfParent->factory->GetUVAnimation (idx);
}

iSprite2DUVAnimation *csSprite2DMeshObject::Sprite2DState::GetUVAnimation (
	int idx, int &style, bool &loop) const
{
  style = scfParent->uvani->style;
  loop = scfParent->uvani->loop;
  return scfParent->factory->GetUVAnimation (idx);
}


void csSprite2DMeshObject::uvAnimationControl::Advance (csTicks current_time)
{
  int oldframeindex = frameindex;
  // the goal is to find the next frame to show
  if (style < 0)
  { // every (-1*style)-th frame show a new pic
    counter--;
    if (counter < style)
    {
      counter = 0;
      frameindex++;
      if (frameindex == framecount)
	if (loop)
	  frameindex = 0;
	else
	{
	  frameindex = framecount-1;
	  halted = true;
	}
    }
  }
  else
    if (style > 0)
    { // skip to next frame every <style> millisecond
      if (last_time == 0)
	last_time = current_time;
      counter += (current_time - last_time);
      last_time = current_time;
      while (counter > style)
      {
	counter -= style;
	frameindex++;
	if (frameindex == framecount)
	  if (loop)
	    frameindex = 0;
	  else
	    {
	      frameindex = framecount-1;
	      halted = true;
	    }
      }
    }
    else
    { // style == 0 -> use time indices attached to the frames
      if (last_time == 0)
	last_time = current_time;
      while (frame->GetDuration () + last_time < current_time)
      {
	frameindex++;
	if (frameindex == framecount)
	  if (loop)
	  {
	    frameindex = 0;
	  }
	  else
	  {
	    frameindex = framecount-1;
	    halted = true;
	    break;
	  }
	last_time += frame->GetDuration ();
	frame = ani->GetFrame (frameindex);
      }
    }

  if (oldframeindex != frameindex)
    frame = ani->GetFrame (frameindex);

}

const csVector2 *csSprite2DMeshObject::uvAnimationControl::GetVertices (
	int &num)
{
  num = frame->GetUVCount ();
  return frame->GetUVCoo ();
}

// The hit beam methods in sprite2d make a couple of small presumptions.
// 1) The sprite is always facing the start of the beam.
// 2) Since it is always facing the the beam, only one side
// of its bounding box can be hit (if at all).

void csSprite2DMeshObject::CheckBeam (const csVector3& start,
      const csVector3& pl, float sqr, csMatrix3& o2t)
{
  // This method is an optimized version of LookAt() based on
  // the presumption that the up vector is always (0,1,0).
  // This is used to create a transform to move the intersection
  // to the sprites vector space, then it is tested against the 2d
  // coords, which are conveniently located at z=0.
  // The transformation matrix is stored and used again if the
  // start vector for the beam is in the same position. MHV.

  csVector3 pl2 = pl * csQisqrt (sqr);
  csVector3 v1( pl2.z, 0, -pl2.x);
  sqr = v1*v1;
  v1 *= csQisqrt(sqr);
  csVector3 v2(pl2.y * v1.z, pl2.z * v1.x - pl2.x * v1.z, -pl2.y * v1.x);
  o2t.Set (v1.x, v2.x, pl2.x,
           v1.y, v2.y, pl2.y,
           v1.z, v2.z, pl2.z);
}


bool csSprite2DMeshObject::HitBeamOutline(const csVector3& start,
      const csVector3& end, csVector3& isect, float* pr)
{
  csVector2 cen = bbox_2d.GetCenter();
  csVector3 pl = start - csVector3(cen.x, cen.y, 0);
  float sqr = pl * pl;
  if (sqr < SMALL_EPSILON) return false; // Too close, Cannot intersect
  float dist;
  csIntersect3::SegmentPlane(start, end, pl, 0, isect, dist);
  if (pr) *pr = dist;
  csMatrix3 o2t;
  CheckBeam (start, pl, sqr, o2t);
  csVector3 r = o2t * isect;
  int trail, len = vertices.Length();
  trail = len - 1;
  csVector2 isec(r.x, r.y);
  int i;
  for (i = 0; i < len; trail = i++)
    if (csMath2::WhichSide2D(isec, vertices[trail].pos, vertices[i].pos) > 0)
      return false;
  return true;
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csSprite2DMeshObjectFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iSprite2DFactoryState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSprite2DMeshObjectFactory::Sprite2DFactoryState)
  SCF_IMPLEMENTS_INTERFACE (iSprite2DFactoryState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSprite2DMeshObjectFactory::csSprite2DMeshObjectFactory (iMeshObjectType* pParent,
	iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSprite2DFactoryState);
  material = 0;
  MixMode = 0;
  lighting = true;
  logparent = 0;
  spr2d_type = pParent;
  csSprite2DMeshObjectFactory::object_reg = object_reg;
  light_mgr = CS_QUERY_REGISTRY (object_reg, iLightManager);
  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
}

csSprite2DMeshObjectFactory::~csSprite2DMeshObjectFactory ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiSprite2DFactoryState);
  SCF_DESTRUCT_IBASE ();
}

csPtr<iMeshObject> csSprite2DMeshObjectFactory::NewInstance ()
{
  csSprite2DMeshObject* cm = new csSprite2DMeshObject (this);
  csRef<iMeshObject> im (SCF_QUERY_INTERFACE (cm, iMeshObject));
  cm->DecRef ();
  return csPtr<iMeshObject> (im);
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csSprite2DMeshObjectType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSprite2DMeshObjectType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csSprite2DMeshObjectType)


csSprite2DMeshObjectType::csSprite2DMeshObjectType (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSprite2DMeshObjectType::~csSprite2DMeshObjectType ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

csPtr<iMeshObjectFactory> csSprite2DMeshObjectType::NewFactory ()
{
  csSprite2DMeshObjectFactory* cm = new csSprite2DMeshObjectFactory (this,
  	object_reg);
  csRef<iMeshObjectFactory> ifact =
  	SCF_QUERY_INTERFACE (cm, iMeshObjectFactory);
  cm->DecRef ();
  return csPtr<iMeshObjectFactory> (ifact);
}

bool csSprite2DMeshObjectType::Initialize (iObjectRegistry* object_reg)
{
  csSprite2DMeshObjectType::object_reg = object_reg;
  return true;
}
