/* 
    Copyright (C) 2003 by Jorrit Tyberghein, Daniel Duhprey
              (C) 2003 Marten Svanfeldt

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
#include "csutil/scf.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/flags.h"
#include "csutil/xmltiny.h"
#include "csgeom/transfrm.h"
#include "csgeom/vector4.h"
#include "csgeom/pmtools.h"
#include "csgfx/renderbuffer.h"

#include "iengine/rview.h"
#include "iengine/camera.h"
#include "iengine/light.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/sector.h"
#include "imesh/object.h"
#include "iutil/vfs.h"
#include "iutil/plugin.h"
#include "iutil/strset.h"
#include "iutil/document.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "ivideo/rendermesh.h"
#include "ivaria/reporter.h"

//#define SHADOW_CACHE_DEBUG

#include "polymesh.h"
#include "stencil.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csStencilShadowStep)
  SCF_IMPLEMENTS_INTERFACE (iRenderStep)
  SCF_IMPLEMENTS_INTERFACE (iLightRenderStep)
  SCF_IMPLEMENTS_INTERFACE (iRenderStepContainer)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csStencilShadowCacheEntry)
  SCF_IMPLEMENTS_INTERFACE (iObjectModelListener)
SCF_IMPLEMENT_IBASE_END

csStencilShadowCacheEntry::csStencilShadowCacheEntry (
	csStencilShadowStep* parent, iMeshWrapper* mesh)
{
  SCF_CONSTRUCT_IBASE (0);
  shadow_vertex_buffer = 0;
  shadow_normal_buffer = 0;
  active_index_buffer = 0;
 
  vertex_count = 0;
  triangle_count = 0;
  edge_count = 0;

  enable_caps = false;

  meshShadows = false;

  csStencilShadowCacheEntry::parent = parent;
  meshWrapper = mesh;
  model = 0;
  closedMesh = 0;
  bufferHolder.AttachNew (new csRenderBufferHolder);

  csRef<iObjectModel> model = mesh->GetMeshObject ()->GetObjectModel ();
  model->AddListener (this);
  ObjectModelChanged (model);
}

csStencilShadowCacheEntry::~csStencilShadowCacheEntry ()
{
  delete closedMesh;
  SCF_DESTRUCT_IBASE();
}

void csStencilShadowCacheEntry::SetActiveLight (iLight *light, 
						csVector3 meshlightpos, 
						int& active_index_range, 
						int& active_edge_start)
{
  //check if this light exists in cache, and if it is ok
  csLightCacheEntry *entry = lightcache.Get (light, 0);

  if (entry == 0)
  {
    entry = new csLightCacheEntry ();
    entry->light = light;
    entry->meshLightPos = meshlightpos; 
    entry->edge_start = 0;
    entry->index_range = 0;
    entry->shadow_index_buffer = 0;
    lightcache.Put (light, entry);
  }

  /* shadow_index_buffer is set to 0 if model changes shape (from listener) */
  if (entry->shadow_index_buffer == 0 || 
  /* FIXME: replace with the technique from viscull */
      (entry->meshLightPos - meshlightpos).SquaredNorm () > 0.0) 
  {
    entry->meshLightPos = meshlightpos;
    if (entry->shadow_index_buffer == 0) 
    { 
      entry->shadow_index_buffer = csRenderBuffer::CreateIndexRenderBuffer (
        triangle_count*12, CS_BUF_DYNAMIC,
        CS_BUFCOMP_UNSIGNED_INT, 0, triangle_count*12); 
	// @@@ Is the upper range correct?
    }

    unsigned int *buf = (unsigned int *)entry->shadow_index_buffer->Lock (
    	CS_BUF_LOCK_NORMAL);
    entry->edge_start = triangle_count*3;
    int indexRange = entry->index_range = entry->edge_start;

    /* setup shadow caps */
    int i;
    for (i = 0; i < entry->edge_start; i ++) buf[i] = i;

    csVector4 lightPos4 = entry->meshLightPos;
    lightPos4.w = 0;


    int* edge_indices_array = edge_indices.GetArray ();
    for (i = 0; i < edge_count; i += 2)
    {
      csVector3 lightdir = entry->meshLightPos - edge_midpoints[i];
      if (((lightdir * edge_normals[i]) * (lightdir * edge_normals[i+1])) <= 0)
      {
        memcpy (buf+indexRange, edge_indices_array+i*3, sizeof (int)*6);
	indexRange += 6;
      }
    }

    entry->index_range = indexRange;

    entry->shadow_index_buffer->Release ();
  }

  active_index_buffer = entry->shadow_index_buffer;
  active_index_range = entry->index_range;
  active_edge_start = entry->edge_start;
}

void csStencilShadowCacheEntry::HandleEdge (EdgeInfo *e,
	csHash<EdgeInfo*>& edge_stack)
{
  double mplier = PI * 1e6;
  uint32 hash;
  hash = (uint32)(mplier * e->a.x + mplier * e->a.y + mplier * e->a.z);
  hash += (uint32)(mplier * e->b.x + mplier * e->b.y + mplier * e->b.z);

  csHash<EdgeInfo*>::Iterator it = edge_stack.GetIterator (hash);
  bool found = false;
  while (it.HasNext ()) 
  {
    EdgeInfo *t = it.Next ();
    if (e->a == t->b && e->b == t->a) 
    {
      found = true;
      edge_indices[edge_count*3 + 0] = e->ind_a;
      edge_indices[edge_count*3 + 1] = t->ind_b;
      edge_indices[edge_count*3 + 2] = t->ind_a;
      // edge_normals[edge_count] = t->norm;
      edge_midpoints[edge_count] = (t->a + t->b) / 2;
      edge_count ++;

      edge_indices[edge_count*3 + 0] = t->ind_a;
      edge_indices[edge_count*3 + 1] = e->ind_b;
      edge_indices[edge_count*3 + 2] = e->ind_a;
      // edge_normals[edge_count] = e->norm;
      edge_midpoints[edge_count] = (e->a + e->b) / 2;
      edge_count ++;
		    
      edge_stack.Delete (hash, t);
      break;
    }
  }
  if (!found) 
  { 
    edge_stack.Put (hash, e); 
  }
}

void csStencilShadowCacheEntry::HandlePoly (const csVector3* vertices, 
                                            const int* polyVertices, 
                                            const int numVerts,
                                            csArray<EdgeInfo>& edge_array, 
                                            csHash<EdgeInfo*>& edge_stack,
                                            int& NextEdge, int& TriIndex)
{
  EdgeInfo *e = &edge_array[NextEdge ++];
  e->a = vertices[polyVertices[0]];
  e->b = vertices[polyVertices[1]];
  e->ind_a = TriIndex + 0;
  e->ind_b = TriIndex + 1;
  HandleEdge (e, edge_stack);

  /* if the polygon is just a triangle this happens once
      and the net result is that each edge is handled explicitly */
  for (int j = 2; j < numVerts; j ++) 
  {
    EdgeInfo *e = &edge_array[NextEdge ++];
    e->a = vertices[polyVertices[j - 1]];
    e->b = vertices[polyVertices[j]];
    e->ind_a = TriIndex + 1;
    e->ind_b = TriIndex + 2;
    HandleEdge (e, edge_stack);
    TriIndex += 3;
  }

  e = &edge_array[NextEdge ++];
  e->a = vertices[polyVertices[numVerts - 1]];
  e->b = vertices[polyVertices[0]];
  e->ind_a = TriIndex - 1; /* TriIndex + 2 from previous triangle */
  e->ind_b = TriIndex - 3; /* TriIndex + 0 from previous taiangle */
  HandleEdge (e, edge_stack);
}

void csStencilShadowCacheEntry::ObjectModelChanged (iObjectModel* model)
{
  meshShadows = false;
  if (csStencilShadowCacheEntry::model != model)
  {
#   ifdef SHADOW_CACHE_DEBUG
    csPrintf ("New model %p, old model %p\n", model,
      csStencilShadowCacheEntry::model);
#   endif
    csStencilShadowCacheEntry::model = model;	
  }

  // Try to get a MeshShadow polygonmesh
  csRef<iPolygonMesh> mesh = model->GetPolygonMeshShadows ();
  if (mesh && mesh->GetPolygonCount () > 0)
  {
    // Stencil shadows need closed meshes.
    const csFlags& meshFlags = mesh->GetFlags ();
    // @@@ Not good when the object model changes often.
    //  Store the information or so?
    if (meshFlags.Check (CS_POLYMESH_NOTCLOSED) || 
      (!meshFlags.Check (CS_POLYMESH_CLOSED) && 
      !csPolygonMeshTools::IsMeshClosed (mesh)))
    {
      // If not closed, close it.
      if (closedMesh == 0)
	closedMesh = new csStencilPolygonMesh ();
      closedMesh->CopyFrom (mesh);

      csArray<csMeshedPolygon> newPolys;
      int* vertidx;
      int vertidx_len;
      csPolygonMeshTools::CloseMesh (mesh, newPolys, vertidx, vertidx_len);
      closedMesh->AddPolys (newPolys, vertidx);

      mesh = closedMesh;
    }
    else
    {
      delete closedMesh;
      closedMesh = 0;
    }
  }
  else
  {
    // No shadow casting for this object.
    return;
  }

  csVector3 *verts = mesh->GetVertices ();

  int new_triangle_count = 0;
  int i;
  for (i = 0; i < mesh->GetPolygonCount(); i ++) 
  {
    /* count triangles assume fan style */
    new_triangle_count += mesh->GetPolygons()[i].num_vertices - 2;
  }

  /* significant change, need to realloc vertex arrays */
  if (mesh->GetVertexCount () != vertex_count || 
      new_triangle_count != triangle_count)
  {
    vertex_count = mesh->GetVertexCount ();
	triangle_count = new_triangle_count;

    shadow_vertex_buffer = csRenderBuffer::CreateRenderBuffer (
       new_triangle_count*3, CS_BUF_DYNAMIC,
       CS_BUFCOMP_FLOAT, 3);
    shadow_normal_buffer = csRenderBuffer::CreateRenderBuffer (
       new_triangle_count*3, CS_BUF_DYNAMIC,
       CS_BUFCOMP_FLOAT, 3);

    csHash<EdgeInfo*> edge_stack(new_triangle_count*3);
    csArray<EdgeInfo> edge_array;
    edge_array.SetLength (new_triangle_count*3, EdgeInfo());
    edge_count = 0;
    int NextEdge = 0;
    int TriIndex = 0;

    face_normals.SetLength (new_triangle_count*3);
    edge_indices.SetLength(new_triangle_count*9);
    edge_normals.SetLength(new_triangle_count*3);
    edge_midpoints.SetLength(new_triangle_count*3);

    if (mesh->GetFlags ().Check (CS_POLYMESH_TRIANGLEMESH))
    {
      const csVector3* triVerts = mesh->GetVertices ();
      const csTriangle* tris = mesh->GetTriangles();
      for (int i = 0; i < mesh->GetTriangleCount(); i ++)
      {
        const csTriangle* tri = &tris[i];

        HandlePoly (triVerts, (int*)tri, 3, 
          edge_array, edge_stack, NextEdge, TriIndex);
      }
    }
    else
    {
      const csVector3* meshVerts = mesh->GetVertices ();
      const csMeshedPolygon* polys = mesh->GetPolygons();
      for (int i = 0; i < mesh->GetPolygonCount(); i ++)
      {
        const csMeshedPolygon *poly = &polys[i];

        HandlePoly (meshVerts, poly->vertices, poly->num_vertices,
          edge_array, edge_stack, NextEdge, TriIndex);
      }
    }
  }

  /* always change vertex based info */
  csVector3 *v = (csVector3*)shadow_vertex_buffer->Lock (CS_BUF_LOCK_NORMAL);
  csVector3 *n = (csVector3*)shadow_normal_buffer->Lock (CS_BUF_LOCK_NORMAL);

  int ind = 0;
  for (i = 0; i < mesh->GetPolygonCount(); i ++) 
  {
    csMeshedPolygon *poly = &mesh->GetPolygons()[i];
    csVector3 ab = verts[poly->vertices[1]] -
                   verts[poly->vertices[0]];
    csVector3 bc = verts[poly->vertices[2]] -
                   verts[poly->vertices[1]];
    csVector3 normal = ab % bc;

    for (int j = 2; j < poly->num_vertices; j ++)
    {
      v[ind] = verts[poly->vertices[0]];
      face_normals[ind++] = normal;
      v[ind] = verts[poly->vertices[j-1]];
      face_normals[ind++] = normal;
      v[ind] = verts[poly->vertices[j]];
      face_normals[ind++] = normal;
    }
  }
  memcpy (n, &face_normals[0], sizeof (csVector3) * new_triangle_count * 3);

  for (i = 0; i < triangle_count * 3; i ++) 
  {
    edge_normals[i] = face_normals[edge_indices[i * 3]];
  }

  shadow_normal_buffer->Release ();
  shadow_vertex_buffer->Release ();

  meshShadows = ((triangle_count != 0) && (vertex_count != 0));
}
/*
iRenderBuffer *csStencilShadowCacheEntry::GetRenderBuffer (csStringID name)
{
  if (name == parent->shadow_vertex_name) 
    return shadow_vertex_buffer;
  if (name == parent->shadow_normal_name) 
    return shadow_normal_buffer;
  if (name == parent->shadow_index_name) 
    return active_index_buffer;
  return 0;
}
*/
void csStencilShadowCacheEntry::UpdateBuffers ()
{
  bufferHolder->SetRenderBuffer (CS_BUFFER_POSITION, shadow_vertex_buffer);
  bufferHolder->SetRenderBuffer (CS_BUFFER_NORMAL, shadow_normal_buffer);
  bufferHolder->SetRenderBuffer (CS_BUFFER_INDEX, active_index_buffer);
}

//---------------------------------------------------------------------------

csStencilShadowStep::csStencilShadowStep (csStencilShadowType* type) :  
  shadowDrawVisCallback ()
{
  SCF_CONSTRUCT_IBASE (0);
  csStencilShadowStep::type = type;
  shadowDrawVisCallback.parent = this;
  enableShadows = false;
}

csStencilShadowStep::~csStencilShadowStep ()
{
  SCF_DESTRUCT_IBASE();
}

void csStencilShadowStep::Report (int severity, const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  csReportV (object_reg, severity, 
    "crystalspace.renderloop.step.shadow.stencil", msg,
    args);
  va_end (args);
}

bool csStencilShadowStep::Initialize (iObjectRegistry* objreg)
{
  object_reg = objreg;
  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  shmgr = CS_QUERY_REGISTRY (object_reg, iShaderManager);

  const csGraphics3DCaps* caps = g3d->GetCaps();
  enableShadows = caps->StencilShadows;
  if (!enableShadows)
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY, 
      "Renderer does not support stencil shadows");
  }
  return true;
}

void csStencilShadowStep::DrawShadow (iRenderView* rview, iLight* light, 
				      iMeshWrapper *mesh, iShader* shader, 
				      size_t shaderTicket, size_t pass)
{
  csRef<csStencilShadowCacheEntry> shadowCacheEntry = 
    shadowcache.Get (mesh, 0);
  if (shadowCacheEntry == 0) 
  {
    /* need the extra reference for the hashmap */
    shadowCacheEntry = new csStencilShadowCacheEntry (this, mesh);
    shadowcache.Put (mesh, shadowCacheEntry);
  }

  if (!shadowCacheEntry->MeshCastsShadow ()) return;

  //float s, e;
  iCamera* camera = rview->GetCamera ();
  // First create the transformation from object to camera space directly:
  //   W = Mow * O - Vow;
  //   C = Mwc * (W - Vwc)
  // ->
  //   C = Mwc * (Mow * O - Vow - Vwc)
  //   C = Mwc * Mow * O - Mwc * (Vow + Vwc)
  csReversibleTransform tr_o2c = camera->GetTransform ();
  if (!mesh->GetMovable()->IsFullTransformIdentity ())
    tr_o2c /= mesh->GetMovable()->GetFullTransform ();

  iGraphics3D* g3d = rview->GetGraphics3D ();

  csVector3 meshlightpos = light->GetCenter ()
  	* mesh->GetMovable()->GetFullTransform ();
  int index_range, edge_start;

  shadowCacheEntry->SetActiveLight (light, meshlightpos, index_range,
  	edge_start);

  csRenderMesh rmesh;
  rmesh.object2camera = tr_o2c;
  //rmesh.transform = &tr_o2c;
  rmesh.z_buf_mode = CS_ZBUF_TEST;
  //rmesh.mixmode = shader->GetMixmodeOverride (); //CS_FX_COPY;
  rmesh.material = 0;
  rmesh.buffers = shadowCacheEntry->bufferHolder;
  rmesh.meshtype = CS_MESHTYPE_TRIANGLES;

  csRenderMeshModes modes (rmesh);
  // probably shouldn't need to check this in general
  // but just in case, no need to draw if no edges are drawn
  if (edge_start < index_range) 
  {
    static csShaderVarStack stacks; // @@@ use STATIC macros
    stacks.Empty ();

    shadowCacheEntry->UpdateBuffers ();
    shmgr->PushVariables (stacks);
    shader->SetupPass (shaderTicket, &rmesh, modes, stacks);
    if (shadowCacheEntry->ShadowCaps())
    {
      rmesh.indexstart = 0;
      rmesh.indexend = index_range;
      /*
        @@@ Try to get rid of drawing the mesh twice
       */
      g3d->SetShadowState (CS_SHADOW_VOLUME_FAIL1);
      g3d->DrawMesh (&rmesh, modes, stacks);
      g3d->SetShadowState (CS_SHADOW_VOLUME_FAIL2);
      g3d->DrawMesh (&rmesh, modes, stacks);
    }
    else 
    {
      rmesh.indexstart = edge_start;
      rmesh.indexend = index_range;
      g3d->SetShadowState (CS_SHADOW_VOLUME_PASS1);
      g3d->DrawMesh (&rmesh, rmesh, stacks);
      g3d->SetShadowState (CS_SHADOW_VOLUME_PASS2);
      g3d->DrawMesh (&rmesh, rmesh, stacks);
    }
    shader->TeardownPass (shaderTicket);
    shmgr->PopVariables (stacks);
  }
}

void csStencilShadowStep::Perform (iRenderView* rview, iSector* sector,
  csShaderVarStack &stacks)
{
  /// TODO: Report error (no light)
  return;
}

void csStencilShadowStep::Perform (iRenderView* rview, iSector* sector,
	iLight* light, csShaderVarStack &stacks)
{
  iShader* shadow;
  if (!enableShadows || ((shadow = type->GetShadow ()) == 0))
  {
    for (size_t i = 0; i < steps.Length (); i++)
    {
      steps[i]->Perform (rview, sector, light, stacks);
    }
    return;
  }

  //int i;
  //test if light is in front of or behind camera
  bool lightBehindCamera = false;
  csReversibleTransform ct = rview->GetCamera ()->GetTransform ();
  const csVector3 camPlaneZ = ct.GetT2O().Col3 ();
  const csVector3 camPos = ct.GetOrigin ();
  const csVector3 lightPos = light->GetCenter ();
  csVector3 v = lightPos - camPos;
  csRef<iVisibilityCuller> culler = sector->GetVisibilityCuller ();
  
  if (camPlaneZ * v <= 0)
    lightBehindCamera = true;

  // mark those objects where we are in the shadow-volume
  // construct five planes, top, bottom, right, left and camera
  float top, bottom, left, right;
  rview->GetFrustum (left, right, bottom, top);
  
  //construct the vectors for middlepoint of each side of the camera
  csVector3 midbottom = ct.This2Other (csVector3 (0,bottom,0));
  csVector3 midtop = ct.This2Other (csVector3 (0,top,0));
  csVector3 midleft = ct.This2Other (csVector3 (left,0,0));
  csVector3 midright = ct.This2Other (csVector3 (right,0,0));

  //get camera x-vector
  csVector3 cameraXVec = ct.This2Other (csVector3 (1,0,0));
  csVector3 cameraYVec = ct.This2Other (csVector3 (0,1,0));

  csPlane3 planes[5];
  planes[0].Set (midbottom, lightPos, midbottom + cameraXVec);
  planes[1].Set (midtop, lightPos, midtop - cameraXVec);
  planes[2].Set (midleft, lightPos, midleft + cameraYVec);
  planes[3].Set (midright, lightPos, midright - cameraYVec);
  
  if (lightBehindCamera)
  {
    planes[4].Set (camPos, cameraYVec, cameraXVec);
    //planes[5] = csPlane3 (lightPos, cameraYVec, cameraXVec);
  }
  else
  {
    planes[4].Set (camPos, cameraXVec,cameraYVec );
    //planes[5] = csPlane3 (lightPos, cameraXVec,cameraYVec );
  }

  csRef<iVisibilityObjectIterator> objInShadow = culler->VisTest (planes, 5);
  while (objInShadow->HasNext() )
  {
    iMeshWrapper* obj = objInShadow->Next ()->GetMeshWrapper ();
    
    csRef<csStencilShadowCacheEntry> shadowCacheEntry = 
      shadowcache.Get (obj, 0);

    if (shadowCacheEntry == 0) 
    {
      csRef<iObjectModel> model = 
	obj->GetMeshObject ()->GetObjectModel ();
      if (!model) { continue; } // Can't do shadows on this
      /* need the extra reference for the hashmap */
      shadowCacheEntry = new csStencilShadowCacheEntry (this, obj);
      shadowcache.Put (obj, shadowCacheEntry);
    }

    shadowCacheEntry->EnableShadowCaps ();
  }

  //cull against the boundingsphere of the light
  csSphere lightSphere (lightPos, light->GetCutoffDistance ());

  g3d->SetZMode (CS_ZBUF_TEST);

  g3d->SetShadowState (CS_SHADOW_VOLUME_BEGIN);

  shadowMeshes.Truncate (0);
  culler->VisTest (lightSphere, &shadowDrawVisCallback);

  int numShadowMeshes;
  if ((numShadowMeshes = shadowMeshes.Length ()) > 0)
  {
    csVector3 rad, center;
    float maxRadius;
    csRenderMeshModes modes;
    modes.z_buf_mode = CS_ZBUF_TEST;
    size_t shaderTicket = shadow->GetTicket (modes, stacks);
    for (size_t p = 0; p < shadow->GetNumberOfPasses (shaderTicket); p ++) 
    {
      shadow->ActivatePass (shaderTicket, p);
      for (int m = 0; m < numShadowMeshes; m++)
      {
	iMeshWrapper*& sp = shadowMeshes[m];

	sp->GetRadius (rad, center);
        
	const csReversibleTransform& tf = sp->GetMovable ()->GetTransform ();
	csVector3 pos = tf.This2Other (center); //transform it
	csVector3 radWorld = tf.This2Other (rad);
	maxRadius = MAX(radWorld.x, MAX(radWorld.y, radWorld.z));

	if (!lightBehindCamera)
	{
	  // light is in front of camera
	  //test if mesh is behind camera
	  v = pos - camPos;
	}
	else
	{
	  v = pos - lightPos;
	}
	if (!(camPlaneZ*v < -maxRadius))
	{
	  DrawShadow (rview, light, sp, shadow, shaderTicket, p); 
	}
      }

      shadow->DeactivatePass (shaderTicket);
    }
  }

  //disable the reverses
  objInShadow->Reset ();
  while (objInShadow->HasNext() )
  {
    iMeshWrapper* sp = objInShadow->Next()->GetMeshWrapper ();
    csRef<csStencilShadowCacheEntry> shadowCacheEntry = 
      shadowcache.Get (sp, 0);
    if (shadowCacheEntry != 0)
      shadowCacheEntry->DisableShadowCaps ();
  }  

  g3d->SetShadowState (CS_SHADOW_VOLUME_USE);

  for (size_t i = 0; i < steps.Length (); i++)
  {
    steps[i]->Perform (rview, sector, light, stacks);
  }

  g3d->SetShadowState (CS_SHADOW_VOLUME_FINISH);
}

int csStencilShadowStep::AddStep (iRenderStep* step)
{
  csRef<iLightRenderStep> lrs = 
    SCF_QUERY_INTERFACE (step, iLightRenderStep);
  if (!lrs) return -1;
  return steps.Push (lrs);
}

int csStencilShadowStep::GetStepCount ()
{
  return steps.Length();
}

SCF_IMPLEMENT_IBASE(csStencilShadowStep::ShadowDrawVisCallback)
  SCF_IMPLEMENTS_INTERFACE(iVisibilityCullerListener)
SCF_IMPLEMENT_IBASE_END

csStencilShadowStep::ShadowDrawVisCallback::ShadowDrawVisCallback ()
{
  SCF_CONSTRUCT_IBASE(0);
}

csStencilShadowStep::ShadowDrawVisCallback::~ShadowDrawVisCallback ()
{
  SCF_DESTRUCT_IBASE();
}

void csStencilShadowStep::ShadowDrawVisCallback::ObjectVisible (
  iVisibilityObject *visobject, iMeshWrapper *mesh, uint32 /*frustum_mask*/)
{
  parent->shadowMeshes.Push (mesh);
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csStencilShadowFactory);
  SCF_IMPLEMENTS_INTERFACE (iRenderStepFactory);
SCF_IMPLEMENT_EMBEDDED_IBASE_END;

csStencilShadowFactory::csStencilShadowFactory (iObjectRegistry* object_reg,
						csStencilShadowType* type)
{
  SCF_CONSTRUCT_IBASE (0);
  csStencilShadowFactory::object_reg = object_reg;
  csStencilShadowFactory::type = type;
}

csStencilShadowFactory::~csStencilShadowFactory ()
{
  SCF_DESTRUCT_IBASE();
}

csPtr<iRenderStep> csStencilShadowFactory::Create ()
{
  csStencilShadowStep* step = new csStencilShadowStep (type);
  step->Initialize (object_reg);
  return csPtr<iRenderStep> (step);
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY(csStencilShadowType);

csStencilShadowType::csStencilShadowType (iBase *p) : csBaseRenderStepType (p)
{
  shadowLoaded = false;
}

csStencilShadowType::~csStencilShadowType ()
{
}

void csStencilShadowType::Report (int severity, const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  csReportV (object_reg, severity, 
    "crystalspace.renderloop.step.shadow.stencil.type", msg,
    args);
  va_end (args);
}

csPtr<iRenderStepFactory> csStencilShadowType::NewFactory ()
{
  return csPtr<iRenderStepFactory> (new csStencilShadowFactory (
    object_reg, this));
}

iShader* csStencilShadowType::GetShadow ()
{
  if (!shadowLoaded)
  {
    shadowLoaded = true;

    csRef<iPluginManager> plugin_mgr (
      CS_QUERY_REGISTRY (object_reg, iPluginManager));

    // Load the shadow vertex program 
    csRef<iShaderManager> shmgr = CS_QUERY_REGISTRY (object_reg,
    	iShaderManager);
    if (!shmgr) 
    {
      shmgr = CS_LOAD_PLUGIN (plugin_mgr,
        "crystalspace.graphics3d.shadermanager",
        iShaderManager);
    }
    if (!shmgr) 
    {
      Report (CS_REPORTER_SEVERITY_ERROR, "Unable to retrieve shader manager!");
      return 0;
    }

    csRef<iShaderCompiler> shcom (shmgr->GetCompiler ("XMLShader"));
    
    csRef<iVFS> vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
    csRef<iDataBuffer> buf = vfs->ReadFile ("/shader/shadow.xml");
    //csRef<iDataBuffer> buf = vfs->ReadFile ("/shader/shadowdebug.xml");
    csRef<iDocumentSystem> docsys (
      CS_QUERY_REGISTRY(object_reg, iDocumentSystem));
    if (docsys == 0)
    {
      docsys.AttachNew (new csTinyDocumentSystem ());
    }
    csRef<iDocument> shaderDoc = docsys->CreateDocument ();
    shaderDoc->Parse (buf, true);

    shadow = shcom->CompileShader (shaderDoc->GetRoot ()->GetNode ("shader"));
    
    if (!shadow)
    {
      Report (CS_REPORTER_SEVERITY_ERROR, "Unable to load shadow shader");
      return 0;
    }
    
  }
  return shadow;
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY(csStencilShadowLoader);

csStencilShadowLoader::csStencilShadowLoader (iBase *p) 
	: csBaseRenderStepLoader (p)
{
  InitTokenTable (tokens);
}

csStencilShadowLoader::~csStencilShadowLoader ()
{
}

bool csStencilShadowLoader::Initialize (iObjectRegistry* object_reg)
{
  if (csBaseRenderStepLoader::Initialize (object_reg))
  {
    return rsp.Initialize (object_reg);
  }
  else
  {
    return false;
  }
}

csPtr<iBase> csStencilShadowLoader::Parse (iDocumentNode* node,
					   iLoaderContext* ldr_context,
					   iBase* context)
{
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  csRef<iRenderStepType> type (CS_LOAD_PLUGIN (plugin_mgr,
  	"crystalspace.renderloop.step.shadow.stencil.type", 
	iRenderStepType));

  csRef<iRenderStepFactory> factory = type->NewFactory();
  csRef<iRenderStep> step = factory->Create ();

  csRef<iRenderStepContainer> steps =
    SCF_QUERY_INTERFACE (step, iRenderStepContainer);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    csStringID id = tokens.Request (child->GetValue ());
    switch (id)
    {
      case XMLTOKEN_STEPS:
	{
	  if (!rsp.ParseRenderSteps (steps, child))
	    return 0;
	}
	break;
      default:
        if (synldr) synldr->ReportBadToken (child);
	return 0;
    }
  }

  return csPtr<iBase> (step);
}

