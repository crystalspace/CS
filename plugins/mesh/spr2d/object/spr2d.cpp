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
#include "csqsqrt.h"

#include "csgeom/box.h"
#include "csgeom/math.h"
#include "csgeom/math3d.h"
#include "csgeom/math2d.h"
#include "csgeom/transfrm.h"
#include "csgfx/renderbuffer.h"
#include "cstool/rbuflock.h"
#include "csutil/scfarray.h"

#include "iengine/movable.h"
#include "iengine/rview.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/material.h"
#include "ivideo/rendermesh.h"
#include "iengine/material.h"
#include "iengine/camera.h"
#include "igeom/clip2d.h"
#include "iengine/engine.h"
#include "iengine/light.h"
#include "iutil/objreg.h"

#include "spr2d.h"

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(Spr2D)
{

CS_LEAKGUARD_IMPLEMENT (csSprite2DMeshObject);
CS_LEAKGUARD_IMPLEMENT (csSprite2DMeshObjectFactory);

csSprite2DMeshObject::csSprite2DMeshObject (csSprite2DMeshObjectFactory* factory) :
  scfImplementationType (this), uvani (0), vertices_dirty (true), 
  texels_dirty (true), colors_dirty (true), indicesSize ((size_t)-1), 
  logparent (0), factory (factory), initialized (false), current_lod (1), 
  current_features (0)
{
  ifactory = scfQueryInterface<iMeshObjectFactory> (factory);
  material = factory->GetMaterialWrapper ();
  lighting = factory->HasLighting ();
  MixMode = factory->GetMixMode ();
  scfVertices.AttachNew (new scfArrayWrap<iColoredVertices, 
    csColoredVertices> (vertices));
}

csSprite2DMeshObject::~csSprite2DMeshObject ()
{
  delete uvani;
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
    radius = csQsqrt (max_sq_dist);

    bufferHolder.AttachNew (new csRenderBufferHolder);
    csRef<iRenderBufferAccessor> newAccessor;
    newAccessor.AttachNew (new RenderBufferAccessor (this));
    bufferHolder->SetAccessor (newAccessor, (uint32)CS_BUFFER_ALL_MASK);
    svcontext.AttachNew (new csShaderVariableContext);
  }
}

static csVector3 cam;

void csSprite2DMeshObject::UpdateLighting (
    const csArray<iLightSectorInfluence*>& lights,
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
  int num_lights = (int)lights.Length ();
  for (i = 0; i < num_lights; i++)
  {
    iLight* li = lights[i]->GetLight ();
    csColor light_color = li->GetColor ()
    	* (256. / CS_NORMAL_LIGHT_LEVEL);
    float sq_light_radius = csSquare (li->GetCutoffDistance ());
    // Compute light position.
    csVector3 wor_light_pos = li->GetMovable ()->GetFullPosition ();
    float wor_sq_dist =
      csSquaredDist::PointPoint (wor_light_pos, pos);
    if (wor_sq_dist >= sq_light_radius) continue;
    float wor_dist = csQsqrt (wor_sq_dist);
    float cosinus = 1.;
    cosinus /= wor_dist;
    light_color *= cosinus * li->GetBrightnessAtDistance (wor_dist);
    color += light_color;
  }
  for (size_t j = 0 ; j < vertices.Length () ; j++)
  {
    vertices[j].color = vertices[j].color_init + color;
    vertices[j].color.Clamp (2, 2, 2);
  }
  colors_dirty = true;

}

void csSprite2DMeshObject::UpdateLighting (
    const csArray<iLightSectorInfluence*>& lights,
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
    const csArray<iLightSectorInfluence*>& relevant_lights = factory->light_mgr
    	->GetRelevantLights (logparent, -1, false);
    UpdateLighting (relevant_lights, movable, offset);
  }

  csReversibleTransform temp = camera->GetTransform ();
  if (!movable->IsFullTransformIdentity ())
    temp /= movable->GetFullTransform ();

  int clip_portal, clip_plane, clip_z_plane;
  rview->CalculateClipSettings (frustum_mask, clip_portal, clip_plane, 
    clip_z_plane);

  csReversibleTransform tr_o2c;
  tr_o2c.SetO2TTranslation (-temp.Other2This (offset));

  bool meshCreated;
  csRenderMesh*& rm = rmHolder.GetUnusedMesh (meshCreated,
    rview->GetCurrentFrameNumber ());
  if (meshCreated)
  {
    rm->meshtype = CS_MESHTYPE_TRIANGLEFAN;
    rm->buffers = bufferHolder;
    rm->variablecontext = svcontext;
    rm->geometryInstance = this;
  }
  
  rm->material = material;//Moved this statement out of the above 'if'
    //to make the change of the material possible at any time
    //(thru a call to either iSprite2DState::SetMaterialWrapper () or
    //csSprite2DMeshObject::SetMaterialWrapper (). Luca
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
  rm->worldspace_origin = movable->GetFullPosition ();

  rm->object2world = tr_o2c.GetInverse () * camera->GetTransform ();
  rm->indexend = (uint)vertices.Length();


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
	texels_count = (int)vertices.Length ();
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

  ShapeChanged ();
}

void csSprite2DMeshObject::NextFrame (csTicks current_time,
	const csVector3& /*pos*/, uint /*currentFrame*/)
{
  if (uvani && !uvani->halted)
  {
    int old_frame_index = uvani->frameindex;
    uvani->Advance (current_time);
    texels_dirty |= (old_frame_index != uvani->frameindex);
  }
}

void csSprite2DMeshObject::UpdateLighting (
	const csArray<iLightSectorInfluence*>& lights,
	const csReversibleTransform& transform)
{
  csVector3 new_pos = transform.This2Other (part_pos);
  UpdateLighting (lights, new_pos);
}

   
void csSprite2DMeshObject::AddColor (const csColor& col)
{
  iColoredVertices* vertices = GetVertices ();
  size_t i;
  for (i = 0 ; i < vertices->GetSize(); i++)
    vertices->Get (i).color_init += col;
  if (!lighting)
    for (i = 0 ; i < vertices->GetSize(); i++)
      vertices->Get (i).color = vertices->Get (i).color_init;

  colors_dirty = true;
}

void csSprite2DMeshObject::ScaleBy (float factor)
{
  iColoredVertices* vertices = GetVertices ();
  size_t i;
  for (i = 0 ; i < vertices->GetSize(); i++)
    vertices->Get (i).pos *= factor;

  vertices_dirty = true;
  ShapeChanged ();
}

void csSprite2DMeshObject::Rotate (float angle)
{
  iColoredVertices* vertices = GetVertices ();
  size_t i;
  for (i = 0 ; i < vertices->GetSize(); i++)
    vertices->Get (i).pos.Rotate (angle);

  vertices_dirty = true;
  ShapeChanged ();
}

void csSprite2DMeshObject::SetUVAnimation (const char *name,
	int style, bool loop)
{
  if (name)
  {
    iSprite2DUVAnimation *ani = factory->GetUVAnimation (name);
    if (ani && ani->GetFrameCount ())
    {
      uvani = new uvAnimationControl ();
      uvani->ani = ani;
      uvani->last_time = 0;
      uvani->frameindex = 0;
      uvani->framecount = ani->GetFrameCount ();
      uvani->frame = ani->GetFrame (0);
      uvani->style = style;
      uvani->counter = 0;
      uvani->loop = loop;
      uvani->halted = false;
    }
  }
  else
  {
    // stop animation and show the normal texture
    delete uvani;
    uvani = 0;
  }
}

void csSprite2DMeshObject::StopUVAnimation (int idx)
{
  if (uvani)
  {
    if (idx != -1)
    {
      uvani->frameindex = MIN(MAX(idx, 0), uvani->framecount-1);
      uvani->frame = uvani->ani->GetFrame (uvani->frameindex);
    }
    uvani->halted = true;
  }
}

void csSprite2DMeshObject::PlayUVAnimation (int idx, int style, bool loop)
{
  if (uvani)
  {
    if (idx != -1)
    {
      uvani->frameindex = MIN(MAX(idx, 0), uvani->framecount-1);
      uvani->frame = uvani->ani->GetFrame (uvani->frameindex);
    }
    uvani->halted = false;
    uvani->counter = 0;
    uvani->last_time = 0;
    uvani->loop = loop;
    uvani->style = style;
  }
}

int csSprite2DMeshObject::GetUVAnimationCount () const
{
  return factory->GetUVAnimationCount ();
}

iSprite2DUVAnimation *csSprite2DMeshObject::CreateUVAnimation ()
{
  return factory->CreateUVAnimation ();
}

void csSprite2DMeshObject::RemoveUVAnimation (
	iSprite2DUVAnimation *anim)
{
  factory->RemoveUVAnimation (anim);
}

iSprite2DUVAnimation *csSprite2DMeshObject::GetUVAnimation (
	const char *name) const
{
  return factory->GetUVAnimation (name);
}

iSprite2DUVAnimation *csSprite2DMeshObject::GetUVAnimation (
	int idx) const
{
  return factory->GetUVAnimation (idx);
}

iSprite2DUVAnimation *csSprite2DMeshObject::GetUVAnimation (
	int idx, int &style, bool &loop) const
{
  style = uvani->style;
  loop = uvani->loop;
  return factory->GetUVAnimation (idx);
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
// 2) Since it is always facing the beam, only one side
// of its bounding box can be hit (if at all).

void csSprite2DMeshObject::CheckBeam (const csVector3& /*start*/,
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
  int trail, len = (int)vertices.Length();
  trail = len - 1;
  csVector2 isec(r.x, r.y);
  int i;
  for (i = 0; i < len; trail = i++)
    if (csMath2::WhichSide2D(isec, vertices[trail].pos, vertices[i].pos) > 0)
      return false;
  return true;
}

//----------------------------------------------------------------------

csSprite2DMeshObjectFactory::csSprite2DMeshObjectFactory (iMeshObjectType* pParent,
  iObjectRegistry* object_reg) : scfImplementationType (this, pParent),
  material (0), logparent (0), spr2d_type (pParent), MixMode (0), 
  lighting (true), object_reg (object_reg)
{
  light_mgr = csQueryRegistry<iLightManager> (object_reg);
  g3d = csQueryRegistry<iGraphics3D> (object_reg);
  csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (
    object_reg, "crystalspace.shared.stringset");
}

csSprite2DMeshObjectFactory::~csSprite2DMeshObjectFactory ()
{
}

csPtr<iMeshObject> csSprite2DMeshObjectFactory::NewInstance ()
{
  csRef<csSprite2DMeshObject> cm;
  cm.AttachNew (new csSprite2DMeshObject (this));
  csRef<iMeshObject> im (scfQueryInterface<iMeshObject> (cm));
  return csPtr<iMeshObject> (im);
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY (csSprite2DMeshObjectType)

csSprite2DMeshObjectType::csSprite2DMeshObjectType (iBase* pParent) :
  scfImplementationType (this, pParent)
{
}

csSprite2DMeshObjectType::~csSprite2DMeshObjectType ()
{
}

csPtr<iMeshObjectFactory> csSprite2DMeshObjectType::NewFactory ()
{
  csRef<csSprite2DMeshObjectFactory> cm;
  cm.AttachNew (new csSprite2DMeshObjectFactory (this,
  	object_reg));
  csRef<iMeshObjectFactory> ifact =
  	scfQueryInterface<iMeshObjectFactory> (cm);
  return csPtr<iMeshObjectFactory> (ifact);
}

bool csSprite2DMeshObjectType::Initialize (iObjectRegistry* object_reg)
{
  csSprite2DMeshObjectType::object_reg = object_reg;
  return true;
}

}
CS_PLUGIN_NAMESPACE_END(Spr2D)
