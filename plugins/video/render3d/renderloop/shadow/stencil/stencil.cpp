/* 
    Copyright (C) 2003 by Jorrit Tyberghein, Daniel Duhprey

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
#include "csutil/garray.h"
#include "csgeom/transfrm.h"
#include "csgeom/vector4.h"

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
#include "ivideo/rendermesh.h"

//#define SHADOW_CULL 1

#include "stencil.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csStencilShadowStep)
  SCF_IMPLEMENTS_INTERFACE (iRenderStep)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iLightRenderStep)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csStencilShadowStep::LightRenderStep)
  SCF_IMPLEMENTS_INTERFACE (iLightRenderStep)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csStencilShadowStep::Component)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csStencilShadowCacheEntry)
  SCF_IMPLEMENTS_INTERFACE (iObjectModelListener)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iRenderBufferSource);
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent);
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csStencilShadowCacheEntry::RenderBufferSource)
  SCF_IMPLEMENTS_INTERFACE (iRenderBufferSource)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csStencilShadowCacheEntry::Component)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csStencilShadowCacheEntry::csStencilShadowCacheEntry (iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiRenderBufferSource);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  shadow_vertex_buffer = 0;
  shadow_normal_buffer = 0;
  active_index_buffer = 0;
 
  vertex_count = 0;
  triangle_count = 0;
  edge_count = 0;
  edge_indices = 0;
  edge_midpoints = 0;
  edge_normals = 0;

  enable_caps = false;
}

csStencilShadowCacheEntry::~csStencilShadowCacheEntry ()
{
}

bool csStencilShadowCacheEntry::Initialize (iObjectRegistry *objreg) 
{
  model = 0;
  g3d = CS_QUERY_REGISTRY (objreg, iGraphics3D);
  if (!g3d) { return false; }
  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (objreg,
	"crystalspace.renderer.stringset", iStringSet);
  shadow_vertex_name = strings->Request ("shadow vertices");
  shadow_normal_name = strings->Request ("shadow normals");
  shadow_index_name = strings->Request ("indices");
  return true;
}

void csStencilShadowCacheEntry::SetActiveLight (iLight *light, 
  csVector3 meshlightpos, int& active_index_range, int& active_edge_start)
{
  //check if this light exists in cache, and if it is ok
  csLightCacheEntry *entry = 
    (csLightCacheEntry*)lightcache.Get ((csHashKey)light);

  if (entry == 0) {
    entry = new csLightCacheEntry ();
    entry->light = light;
    entry->meshLightPos = meshlightpos; 
    entry->edge_start = 0;
    entry->index_range = 0;
    entry->shadow_index_buffer = 0;
    lightcache.Put ((csHashKey)light, entry);
  }

  /* shadow_index_buffer is set to 0 if model changes shape (from listener) */
  if (entry->shadow_index_buffer == 0 || 
  /* FIXME: replace with the technique from viscull */
      (entry->meshLightPos - meshlightpos).SquaredNorm () > 0.02) 
  {
    entry->meshLightPos = meshlightpos;
    if (entry->shadow_index_buffer == 0) { 
      entry->shadow_index_buffer = g3d->CreateRenderBuffer (
        sizeof (unsigned int)*triangle_count*12, CS_BUF_STATIC/*CS_BUF_INDEX*/,
        CS_BUFCOMP_UNSIGNED_INT, 1, true);
    }

    unsigned int *buf = (unsigned int *)entry->shadow_index_buffer->Lock (CS_BUF_LOCK_NORMAL);
    entry->edge_start = triangle_count*3;
    entry->index_range = entry->edge_start;

    /* setup shadow caps */
    int i;
    for (i = 0; i < entry->edge_start; i ++) buf[i] = i;

    csVector4 lightPos4 = entry->meshLightPos;
    lightPos4.w = 0;

/*
#if _MSC_VER >= 1300 
    int source_index = entry->edge_start;

    float* dotTable = new float [edge_start];
    csVector4 lightdir;

    for (i = 0; i < edge_count; i += 2)
    {
      lightdir = lightPos4 - edge_midpoints[i];
      dotTable[i] = (lightdir * edge_normals[i]);
      dotTable[i+1] = (lightdir * edge_normals[i+1]);
    }

    for (i = 0; i < edge_start; i+= 2)
    {
      source_index = edge_start + 3*i;

      if (dotTable[i] * dotTable[i+1] < 0)
      {
        __asm
        {
          push esi
          push edi
          push ecx

          mov esi, [ibuf]
          mov edi, [buf]
          
          mov ecx, source_index
          shl ecx, 2
          add esi, ecx

          mov ecx, index_range
          shl ecx, 2
          add edi, ecx

          //do copy with mmx-routines
          movq mm0, [esi]
          movq mm1, [esi+8]
          movq mm2, [esi+16]

          movq [edi], mm0        
          movq [edi+8], mm1
          movq [edi+16], mm2
          
          emms
          pop esi
          pop edi
          pop ecx
        }
        index_range += 6;
      }
    }

    delete [] dotTable;
    
#else
*/

    for (i = 0; i < edge_count; i += 2)
    {
      csVector3 lightdir = entry->meshLightPos - edge_midpoints[i];
      if (((lightdir * edge_normals[i]) * (lightdir * edge_normals[i+1])) < 0) 
      {
        buf[entry->index_range ++] = edge_indices[i*3 + 0];
        buf[entry->index_range ++] = edge_indices[i*3 + 1];
        buf[entry->index_range ++] = edge_indices[i*3 + 2];
        buf[entry->index_range ++] = edge_indices[i*3 + 3];
        buf[entry->index_range ++] = edge_indices[i*3 + 4];
        buf[entry->index_range ++] = edge_indices[i*3 + 5];
      }
    }
// #endif

    entry->shadow_index_buffer->Release ();
  }

  active_index_buffer = entry->shadow_index_buffer;
  active_index_range = entry->index_range;
  active_edge_start = entry->edge_start;
}

void csStencilShadowCacheEntry::HandleEdge (EdgeInfo *e, csHashMap& edge_stack)
{
  double mplier = PI * 1e6;
  csHashKey hash;
  hash = (int)(mplier * e->a.x + mplier * e->a.y + mplier * e->a.z);
  hash += (int)(mplier * e->b.x + mplier * e->b.y + mplier * e->b.z);

  csHashIterator it (&edge_stack, hash);
  bool found = false;
  while (it.HasNext()) {
    EdgeInfo *t = (EdgeInfo *)it.Next();
    if (e->a == t->b && e->b == t->a) {
      found = true;
      edge_indices[edge_count*3 + 0] = e->ind_a;
      edge_indices[edge_count*3 + 1] = t->ind_b;
      edge_indices[edge_count*3 + 2] = t->ind_a;
      edge_normals[edge_count] = t->norm;
      edge_midpoints[edge_count] = (t->a + t->b) / 2;
      edge_count ++;

      edge_indices[edge_count*3 + 0] = t->ind_a;
      edge_indices[edge_count*3 + 1] = e->ind_b;
      edge_indices[edge_count*3 + 2] = e->ind_a;
      edge_normals[edge_count] = e->norm;
      edge_midpoints[edge_count] = (e->a + e->b) / 2;
      edge_count ++;
		    
      edge_stack.Delete (hash, t);
      break;
    }
  }
  if (!found) { 
    edge_stack.Put (hash, e); 
  }
}

void csStencilShadowCacheEntry::ObjectModelChanged (iObjectModel* model)
{
  if (csStencilShadowCacheEntry::model != model) {
    printf ("New model %8.8x, old model %8.8x\n", model, csStencilShadowCacheEntry::model);
    csStencilShadowCacheEntry::model = model;	
  }

  //first try to use a MeshShadow polygonmesh
  //but if we don't get any, attempt to use collidemesh
  csRef<iPolygonMesh> mesh = model->GetPolygonMeshShadows ();
  if (!mesh)
    mesh = model->GetPolygonMeshColldet (); //@@@ CHECK IF WE WANT TO DO THIS
    if (!mesh)
      return; 

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

    shadow_vertex_buffer = g3d->CreateRenderBuffer (
       sizeof (csVector3)*new_triangle_count*3, CS_BUF_STATIC,
       CS_BUFCOMP_FLOAT, 3, false);
    shadow_normal_buffer = g3d->CreateRenderBuffer (
       sizeof (csVector3)*new_triangle_count*3, CS_BUF_STATIC,
       CS_BUFCOMP_FLOAT, 3, false);

    csHashMap edge_stack(new_triangle_count*3);
    csArray<EdgeInfo> edge_array;
    edge_array.SetLength (new_triangle_count*3, EdgeInfo());
    edge_count = 0;
    int NextEdge = 0;
    int TriIndex = 0;

    delete [] edge_indices;
    edge_indices = new int[new_triangle_count*9];
    delete [] edge_normals;
    edge_normals = new csVector3[new_triangle_count*3];
    delete [] edge_midpoints;
    edge_midpoints = new csVector3[new_triangle_count*3];

    for (int i = 0; i < mesh->GetPolygonCount(); i ++)
    {
      csMeshedPolygon *poly = &mesh->GetPolygons()[i];
      csVector3 ab = mesh->GetVertices()[poly->vertices[1]] -
                     mesh->GetVertices()[poly->vertices[0]];
      csVector3 bc = mesh->GetVertices()[poly->vertices[2]] -
                     mesh->GetVertices()[poly->vertices[1]];
      csVector3 normal = ab % bc;
      normal.Normalize();

      EdgeInfo *e = &edge_array[NextEdge ++];
      e->a = mesh->GetVertices()[poly->vertices[0]];
      e->b = mesh->GetVertices()[poly->vertices[1]];
      e->ind_a = TriIndex + 0;
      e->ind_b = TriIndex + 1;
      e->norm = normal;
      HandleEdge (e, edge_stack);

      /* if the polygon is just a triangle this happens once
         and the net result is that each edge is handled explicitly */
      for (int j = 2; j < poly->num_vertices; j ++) {
        EdgeInfo *e = &edge_array[NextEdge ++];
        e->a = mesh->GetVertices()[poly->vertices[j-1]];
        e->b = mesh->GetVertices()[poly->vertices[j]];
        e->ind_a = TriIndex + 1;
        e->ind_b = TriIndex + 2;
        e->norm = normal;
    	HandleEdge (e, edge_stack);
        TriIndex += 3;
      }

      e = &edge_array[NextEdge ++];
      e->a = mesh->GetVertices()[poly->vertices[poly->num_vertices-1]];
      e->b = mesh->GetVertices()[poly->vertices[0]];
      e->ind_a = TriIndex - 1; /* TriIndex + 2 from previous triangle */
      e->ind_b = TriIndex - 3; /* TriIndex + 0 from previous taiangle */
      e->norm = normal;
      HandleEdge (e, edge_stack);
    }
  }

  /* always change vertex based info */
  csVector3 *v = (csVector3*)shadow_vertex_buffer->Lock (CS_BUF_LOCK_NORMAL);
  csVector3 *n = (csVector3*)shadow_normal_buffer->Lock (CS_BUF_LOCK_NORMAL);

  int ind = 0;
  for (int i = 0; i < mesh->GetPolygonCount(); i ++) 
  {
    csMeshedPolygon *poly = &mesh->GetPolygons()[i];
    csVector3 ab = verts[poly->vertices[1]] -
                   verts[poly->vertices[0]];
    csVector3 bc = verts[poly->vertices[2]] -
                   verts[poly->vertices[1]];
    csVector3 normal = ab % bc;
    normal.Normalize();

    for (int j = 2; j < poly->num_vertices; j ++) {
      v[ind] = verts[poly->vertices[0]];
      n[ind++] = normal;
      v[ind] = verts[poly->vertices[j-1]];
      n[ind++] = normal;
      v[ind] = verts[poly->vertices[j]];
      n[ind++] = normal;
    }
  }
  shadow_normal_buffer->Release ();
  shadow_vertex_buffer->Release ();
}

iRenderBuffer *csStencilShadowCacheEntry::GetRenderBuffer (csStringID name)
{
  if (name == shadow_vertex_name) {
    return shadow_vertex_buffer;
  }
  if (name == shadow_normal_name) {
    return shadow_normal_buffer;
  }
  if (name == shadow_index_name) {
    return active_index_buffer;
  }
  return 0;
}

csStencilShadowStep::csStencilShadowStep (iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiLightRenderStep);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
}

csStencilShadowStep::~csStencilShadowStep ()
{
}

bool csStencilShadowStep::Initialize (iObjectRegistry* objreg)
{
  object_reg = objreg;
  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  // Load the shadow vertex program 
  csRef<iShaderManager> shmgr = CS_QUERY_REGISTRY (object_reg, iShaderManager);
  if (!shmgr) {
    shmgr = CS_LOAD_PLUGIN (plugin_mgr,
      "crystalspace.graphics3d.shadermanager",
      iShaderManager);

    if (!shmgr) {
      printf ("Unable to load ShaderManager!\n");
      return false;
    }
  }
  shadow = shmgr->CreateShader ();
  if (!shadow) {
    printf ("Unable to create new shader\n");
	return false;
  }

  csRef<iVFS> vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
  csRef<iDataBuffer> buf = vfs->ReadFile ("/shader/shadow.xml");
  // csRef<iDataBuffer> buf = vfs->ReadFile ("/shader/shadowdebug.xml");
  shadow->Load (buf);
  shadow->Prepare ();

  return true;
}

/*
void csStencil::Start (iVisibilityCuller* cul)
{
  g3d->SetShadowState (CS_SHADOW_VOLUME_BEGIN);
  culler = cul;
}
*/

void csStencilShadowStep::DrawShadow (iRenderView* rview, iLight* light, iMeshWrapper *mesh, iShaderPass *pass)
{
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
  
  csRef<csStencilShadowCacheEntry> shadowCacheEntry = 
    (csStencilShadowCacheEntry*)shadowcache.Get((csHashKey)mesh);
  if (shadowCacheEntry == 0) {
    /* need the extra reference for the hashmap */
    shadowCacheEntry = new csStencilShadowCacheEntry (this);
    shadowCacheEntry->Initialize (object_reg);
    csRef<iObjectModel> model = mesh->GetMeshObject ()->GetObjectModel ();
    model->AddListener (shadowCacheEntry);
    shadowCacheEntry->ObjectModelChanged (model);
    shadowcache.Put ((csHashKey)mesh, shadowCacheEntry);
  }

  csVector3 meshlightpos = light->GetCenter () * mesh->GetMovable()->GetFullTransform ();
  int index_range, edge_start;

  shadowCacheEntry->SetActiveLight (light, meshlightpos, index_range, edge_start);

  csRenderMesh rmesh;
  rmesh.transform = &tr_o2c;
  rmesh.z_buf_mode = CS_ZBUF_TEST;
  rmesh.mixmode = CS_FX_COPY;
  rmesh.material = 0;
  csRef<iRenderBufferSource> buffer_source = 
    SCF_QUERY_INTERFACE (shadowCacheEntry, iRenderBufferSource);
  rmesh.buffersource = buffer_source;
  rmesh.meshtype = CS_MESHTYPE_TRIANGLES;

  // probably shouldn't need to check this in general
  // but just in case, no need to draw if no edges are drawn
  if (edge_start < index_range) {
    pass->SetupState (&rmesh);
    if (shadowCacheEntry->ShadowCaps())
    {
      rmesh.indexstart = 0;
      rmesh.indexend = index_range;
      /*
        WTF? The same mesh drawn twice, one immediately after another? -
	That has to be optimized, kids! [res]
       */
      g3d->SetShadowState (CS_SHADOW_VOLUME_FAIL1);
      g3d->DrawMesh (&rmesh);
      g3d->SetShadowState (CS_SHADOW_VOLUME_FAIL2);
      g3d->DrawMesh (&rmesh);
    }
    else 
    {
      rmesh.indexstart = edge_start;
      rmesh.indexend = index_range;
      g3d->SetShadowState (CS_SHADOW_VOLUME_PASS1);
      g3d->DrawMesh (&rmesh);
      g3d->SetShadowState (CS_SHADOW_VOLUME_PASS2);
      g3d->DrawMesh (&rmesh);
    }
    pass->ResetState ();
  }
}

void csStencilShadowStep::Perform (iRenderView* rview, iSector* sector)
{
  /// TODO: Report error (no light)
  return;
}

void csStencilShadowStep::Perform (iRenderView* rview, iSector* sector, iLight* light)
{
  int i;
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

  csPlane3* planes = new csPlane3[5];
  planes[0] = csPlane3(midbottom, lightPos, midbottom + cameraXVec);
  planes[1] = csPlane3(midtop, lightPos, midtop - cameraXVec);
  planes[2] = csPlane3(midleft, lightPos, midleft + cameraYVec);
  planes[3] = csPlane3(midright, lightPos, midright - cameraYVec);
  
  if (lightBehindCamera)
  {
    planes[4] = csPlane3(camPos,cameraYVec, cameraXVec );
    //planes[5] = csPlane3(lightPos,cameraYVec, cameraXVec );
  }
  else
  {
    planes[4] = csPlane3(camPos,cameraXVec, cameraYVec );
    //planes[5] = csPlane3(lightPos,cameraXVec, cameraYVec );
  }

#ifdef SHADOW_CULL
  csRef<iVisibilityObjectIterator> objInShadow = culler->VisTest (planes, 5);
  while (!objInShadow->HasNext() )
  {
    iMeshWrapper* obj = objInShadow->Next ()->GetMeshWrapper ();
    
    csRef<csStencilShadowCacheEntry> shadowCacheEntry = 
	(csStencilShadowCacheEntry*)shadowcache.Get((csHashKey)obj);

    if (shadowCacheEntry == 0) {
      csRef<iObjectModel> model = 
	SCF_QUERY_INTERFACE (obj->GetFactory()->GetMeshObjectFactory(), 
		iObjectModel);
      if (!model) { continue; } // Can't do shadows on this
      /* need the extra reference for the hashmap */
      shadowCacheEntry = new csStencilShadowCacheEntry (this);
      shadowCacheEntry->Initialize (object_reg);
      model->AddListener (shadowCacheEntry);
      shadowCacheEntry->ObjectModelChanged (model);
      shadowcache.Put ((csHashKey)obj, shadowCacheEntry);
    }

    shadowCacheEntry->EnableShadowCaps ();
  }

  //cull against the boundingsphere of the light
  csSphere lightSphere (lightPos, light->GetInfluenceRadius ());

  csRef<iVisibilityObjectIterator> objInLight = culler->VisTest (lightSphere);
#endif

  g3d->SetZMode (CS_ZBUF_TEST);
  
  csVector3 rad, center;
  float maxRadius = 0;
  csRef<iShaderTechnique> tech = shadow->GetBestTechnique ();
  for (int p = 0; p < tech->GetPassCount (); p ++) 
  {
    csRef<iShaderPass> pass = tech->GetPass (p);
    pass->Activate (0);
#ifdef SHADOW_CULL
    while (!objInLight->HasNext ())
    {
      iMeshWrapper *sp = objInLight->Next()->GetMeshWrapper ();
#else
    csRef<iSectorRenderMeshList> objs = sector->GetRenderMeshes();
    for (i = 0; i < objs->GetCount(); i ++) 
    {
      iMeshWrapper* sp;
      iVisibilityObject* visobj;
      csRenderMesh* mesh;
      bool visible;
      objs->Get (i, sp, visobj, mesh, &visible);
#endif
      if (sp) 
      {
        sp->GetRadius (rad, center);
        
        csVector3 pos = sp->GetMovable() ->GetTransform ().This2Other (center); //transform it
        csVector3 radWorld = sp->GetMovable ()->GetTransform ().This2Other (rad);
        maxRadius = MAX(radWorld.x, MAX(radWorld.y, radWorld.z));
  
        if (!lightBehindCamera) {
          // light is in front of camera
          //test if mesh is behind camera
          v = pos - camPos;
        } else {
          v = pos - lightPos;
        }
        if (!(camPlaneZ*v < -maxRadius))
        {
          DrawShadow (rview, light, sp, pass); 
        }
      }
    }
    pass->Deactivate ();
  }

  //disable the reverses
#ifdef SHADOW_CULL
  objInShadow->Reset ();
  while (!objInShadow->HasNext() )
  {
    iMeshWrapper* sp = objInShadow->Next()->GetMeshWrapper ();
    csRef<csStencilShadowCacheEntry> shadowCacheEntry = 
      (csStencilShadowCacheEntry*)shadowcache.Get((csHashKey)sp);
    if (shadowCacheEntry != 0)
      shadowCacheEntry->DisableShadowCaps ();
  }  
#endif

  g3d->SetShadowState (CS_SHADOW_VOLUME_USE);

}

/*
void csStencil::Finish ()
{
  g3d->SetShadowState (CS_SHADOW_VOLUME_FINISH);
}
*/

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csStencilShadowFactory);
  SCF_IMPLEMENTS_INTERFACE (iRenderStepFactory);
SCF_IMPLEMENT_EMBEDDED_IBASE_END;

csStencilShadowFactory::csStencilShadowFactory (
	iBase *p, iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE (p);
  csStencilShadowFactory::object_reg = object_reg;
}

csStencilShadowFactory::~csStencilShadowFactory ()
{
}

csPtr<iRenderStep> csStencilShadowFactory::Create ()
{
  csStencilShadowStep* step = new csStencilShadowStep (this);
  step->Initialize (object_reg);
  return csPtr<iRenderStep> (step);
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY(csStencilShadowType);

csStencilShadowType::csStencilShadowType (iBase *p) : csBaseRenderStepType (p)
{
}

csStencilShadowType::~csStencilShadowType ()
{
}

csPtr<iRenderStepFactory> csStencilShadowType::NewFactory ()
{
  // FIXME fix the problem with multiple inheritance so that 0 becomes this
  return csPtr<iRenderStepFactory> (new csStencilShadowFactory (0, object_reg));
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY(csStencilShadowLoader);

csStencilShadowLoader::csStencilShadowLoader (iBase *p) 
	: csBaseRenderStepLoader (p)
{
   // init_token_table (tokens);
}

csStencilShadowLoader::~csStencilShadowLoader ()
{
}

csPtr<iBase> csStencilShadowLoader::Parse (iDocumentNode* node,
                                     iLoaderContext* ldr_context,
                                     iBase* context)
{
  csRef<csStencilShadowStep> step = new csStencilShadowStep (context);
  step->Initialize (object_reg);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    csStringID id = tokens.Request (child->GetValue ());
    switch (id)
    {
      default:
        if (synldr) synldr->ReportBadToken (child);
	return 0;
    }
  }

  return csPtr<iBase> (step);
}
