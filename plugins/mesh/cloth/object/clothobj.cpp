/*
        Cloth mesh object plugin
        Copyl3ft (C) 2002 by Charles Quarra

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

#include <stdarg.h>
#include <string.h>
#include "cssysdef.h"
#include "qsqrt.h"
#include "csutil/sysfunc.h"
#include "csutil/scf.h"
#include "csutil/cscolor.h"
#include "csutil/virtclk.h"
#include "iengine/rview.h"
#include "iengine/movable.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/material.h"
#include "ivideo/vbufmgr.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/camera.h"
#include "iengine/light.h"
#include "igeom/clip2d.h"
#include "iutil/cfgmgr.h"
#include "iutil/objreg.h"
#include "imap/parser.h"

#include "clothobj.h"
#include "cloth.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csStuffObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iVertexBufferManagerClient)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iClothMeshState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csStuffObject::eiVertexBufferManagerClient)
  SCF_IMPLEMENTS_INTERFACE (iVertexBufferManagerClient)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csStuffObject::ObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csStuffObject::csClothMeshState) 
  SCF_IMPLEMENTS_INTERFACE (iClothMeshState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

void csStuffObject::SetupVertexBuffer ()
{
  int count=0;
  if (!vbuf)
  {
    printf(" SETUPVERTEX BUFFER %u \n",count++);
    csRef<iGraphics3D> g3d (CS_QUERY_REGISTRY (Obj_Reg, iGraphics3D));
    // @@@ priority should be a parameter.
    printf(" %u \n",count++);
    
    vbufmgr = g3d->GetVertexBufferManager ();
    vbuf = vbufmgr->CreateBuffer (0);
    //  vbuf1 = vbufmgr->CreateBuffer (1);
    printf(" %u \n",count++);
    
    vbufmgr->AddClient (&scfiVertexBufferManagerClient);
    mesh.buffers[0] = vbuf;
    // mesh.buffers[1] = vbuf1;	 
  }
 printf(" EXITING SETUPVERTEX BUFFER %u \n",count++);
 
}

bool csStuffObject::Initialize(iObjectRegistry *iO_R)
{
  printf("initializtion \n");
  if (iO_R==0) { 	printf(" NO OBJECT_REG! \n"); return false; };
  Obj_Reg=iO_R;
  shift.Set(0,0,0);
  max_radius.Set(5.0,5.0,0.0);
  vis_cb=0;
  vertices=0;
  material=0;
  texels=0;
  colors=0;
  mesh.triangles=0;
  mesh.vertex_fog=0;
  setup=false;
  return true;
};

void csStuffObject::SetupMesh()
{
  if (setup) return;

#ifdef CS_DEBUG
  csPrintf (" setting up the mesh... \n");
#endif
  setup      = true;
  fact_state = SCF_QUERY_INTERFACE (factory, iClothFactoryState);
  Fabric     = new Cloth (fact_state, shift, object_bbox, csVector3 (0.0f, -0.15f, 0.0f));
  
  // after initializing a Cloth, all triangulation and eventually refinement is already done,
  // and i want to copy that to factory just once (and maybe save/cache it)
  //memcpy ( factory_mesh->GetTriangles() , Fabric->triangles , sizeof(csTriangle)*num_triangles );
#ifdef CS_DEBUG
  csPrintf (" setting up the mesh2... \n");	
#endif
  Dynamics      = new Integrator ( Fabric );
  num_vertices  = Fabric -> nverts;
  vertices      = Fabric -> vertices;
  texels        = fact_state-> GetTexels(); 
  colors        = fact_state-> GetColors(); 
  material      = fact_state->GetMaterialWrapper();
  
  SetupVertexBuffer();
  mesh.num_triangles = Fabric -> ntris;  
  mesh.triangles     = Fabric -> triangles;  
  mesh.vertex_fog = new G3DFogInfo[num_vertices];
  
  //memcpy (mesh.triangles, triangles, sizeof(csTriangle)*num_triangles);
  mesh.morph_factor = 0.0f;
  mesh.num_vertices_pool = 1;
  mesh.do_morph_texels = false;
  mesh.do_morph_colors = false;
  mesh.vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
  
  //Adams-Moulton integrator needs 4 euler steps before being able to take over
  Dynamics->ComputeInitial();
  Dynamics->ComputeInitial();
  Dynamics->ComputeInitial();
  Dynamics->ComputeInitial();  
	
  printf( " finished setting up the mesh... \n");	
  
  //printf( " summing diff %u + %u = %u \n ",(int)15 , (short)28 , ((int)15) + ((short)28) );
};


//-------------------MeshObject implementation
//

csStuffObject::csStuffObject(iMeshObjectFactory *fact) 
{
  printf("creator \n");
  factory = fact;
  vbufmgr=0;
  
  SCF_CONSTRUCT_IBASE(fact);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiVertexBufferManagerClient);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiObjectModel);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiClothMeshState);
};

csStuffObject::~csStuffObject() 
{
  delete [] mesh.vertex_fog;
  delete    Fabric;
  delete    Dynamics;
  
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiVertexBufferManagerClient);
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiObjectModel);
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiClothMeshState);
  SCF_DESTRUCT_IBASE();
};

bool csStuffObject::DrawTest(iRenderView *rview, iMovable *movable,
	uint32 frustum_mask) 
{
  //printf(" draw test \n");
  // SetupMesh();
  
  iCamera* camera = rview->GetCamera ();
  csReversibleTransform tr_o2c = camera->GetTransform ();
  if (!movable->IsFullTransformIdentity ())
    tr_o2c /= movable->GetFullTransform ();
  
  int clip_portal, clip_plane, clip_z_plane;
  rview->CalculateClipSettings (frustum_mask, clip_portal, clip_plane,
  	clip_z_plane);

  iGraphics3D* g3d = rview->GetGraphics3D ();
  g3d->SetObjectToCamera (&tr_o2c);
  mesh.clip_portal = clip_portal;
  mesh.clip_plane = clip_plane;
  mesh.clip_z_plane = clip_z_plane;
  mesh.do_mirror = camera->IsMirrored ();
     
  return true;
};

void csStuffObject::GetRadius (csVector3& rad, csVector3& cent)
{
  rad = csVector3(50.0);
  cent.Set (0);
}


bool csStuffObject::Draw(iRenderView *rview, iMovable*, csZBufMode zbufmode)
{
  //printf("       DRAW\n");
  iGraphics3D* g3d = rview->GetGraphics3D ();
  if (!material)
  {
    csRef<iEngine> e = CS_QUERY_REGISTRY(Obj_Reg, iEngine);
    //csRef<iComponent> plugin ( CS_REQUEST_PLUGIN( CS_REQUEST_IMAGELOADER ) );
    csRef<iLoader> loader = CS_QUERY_REGISTRY ( Obj_Reg , iLoader  );
    iTextureManager* txtmgr = g3d->GetTextureManager ();

    printf ("INTERNAL ERROR: cloth used without material!\n");
    iTextureWrapper* txt = loader->LoadTexture ("spark",    
  	"/lib/std/spark.png", CS_TEXTURE_3D, txtmgr, true);
    if (txt == 0)
    {
      printf( " no texture for u!! \n" );
      return false;
    }
    material = e->GetMaterialList()->FindByName("spark");
    
    return false;
  }
  iMaterialHandle* mat = material->GetMaterialHandle ();
  if (!mat)
  {
    printf ("INTERNAL ERROR: cloth used without valid material handle!\n");
    return false;
  }
  
  if (vis_cb) if (!vis_cb->BeforeDrawing (this, rview)) return false;
  
  // Prepare for rendering.
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, zbufmode);

  //SetupVertexBuffer ();
  material->Visit ();
  mesh.mat_handle = mat;
  mesh.use_vertex_color =true;
  mesh.mixmode = MixMode;
  CS_ASSERT (!vbuf->IsLocked ());
  vbufmgr->LockBuffer (vbuf,vertices, texels, colors, num_vertices, 0,
  	object_bbox);
  rview->CalculateFogMesh (g3d->GetObjectToCamera (), mesh);
  g3d->DrawTriangleMesh (mesh);
  vbufmgr->UnlockBuffer (vbuf);
  return true;
};

void csStuffObject::GetObjectBoundingBox (csBox3& bbox, int /*type*/)
{  
  SetupMesh();
  bbox = object_bbox;  
}

iMeshObjectFactory* csStuffObject::GetFactory () const
{ return factory; }
 
void csStuffObject::SetVisibleCallback(iMeshObjectDrawCallback *cb) 
{ vis_cb=cb; }
 
iMeshObjectDrawCallback* csStuffObject::GetVisibleCallback () const 
{ return vis_cb; }
 
void csStuffObject::NextFrame (unsigned int ticks, const csVector3& /*pos*/)
{ Dynamics->Update(ticks); }
 
void csStuffObject::HardTransform (const csReversibleTransform &) 
{}
   
bool csStuffObject::SupportsHardTransform () const 
{ return false; }
 
bool csStuffObject::HitBeamOutline (const csVector3& start,
  const csVector3& end, csVector3& isect, float* pr)
{ 
  csSegment3 seg (start, end);
  int i, max = mesh.num_triangles;
  csTriangle *tr = mesh.triangles;
  csVector3 *vrt = vertices;
  for (i = 0 ; i < max ; i++)
  {
    if (csIntersect3::IntersectTriangle (vrt[tr[i].a], vrt[tr[i].b],vrt[tr[i].c], seg, isect))
    {
      if (pr)
		*pr = qsqrt (csSquaredDist::PointPoint (start, isect) / csSquaredDist::PointPoint (start, end));
      return true;
    }
  }
  return false;
}
	
bool csStuffObject::HitBeamObject (const csVector3& start,
  const csVector3& end, csVector3& isect, float* pr, int* polygon_idx)
{
  if (polygon_idx) *polygon_idx = -1;
  // This is now closer to an outline hitting method. It will
  // return as soon as it touches any triangle in the mesh, and
  // will be a bit faster than its more accurate cousin (below).
  
  csSegment3 seg (start, end);
  int i, max = mesh.num_triangles;
  csTriangle *tr = mesh.triangles;
  csVector3 *vrt = vertices;
  for (i = 0 ; i < max ; i++)
  {
    if (csIntersect3::IntersectTriangle (vrt[tr[i].a], vrt[tr[i].b],
	vrt[tr[i].c], seg, isect))
    {
      if (pr)
	*pr = qsqrt (csSquaredDist::PointPoint (start, isect)
		/ csSquaredDist::PointPoint (start, end));
      return true;
    }
  }
  return false;
}
  
void csStuffObject::SetLogicalParent (iBase *) 
{}
   
iBase *csStuffObject::GetLogicalParent () const 
{ return 0; }
 
// iObjectModel *csStuffObject::GetObjectModel () { return 0; };

//-----------Vertex Buffer Manager Client embedded implementation
void csStuffObject::eiVertexBufferManagerClient::ManagerClosing() 
{ }

//--//--//--//--//-- StuffFactory

SCF_IMPLEMENT_IBASE(StuffFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iClothFactoryState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE(StuffFactory::csClothFactoryState)
  SCF_IMPLEMENTS_INTERFACE (iClothFactoryState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END
 
StuffFactory::StuffFactory(iBase* parent) 
{
  SCF_CONSTRUCT_IBASE( parent )
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiClothFactoryState)	
  object_reg       = 0;
  material         = 0;		
  factory_vertices = 0; 
  factory_texels   = 0;
  factory_colors   = 0;
  factory_triangles= 0;
}

StuffFactory::~StuffFactory() 
{
  if (factory_vertices)  { delete [] factory_vertices; };
  if (factory_texels)    { delete [] factory_texels;   };
  if (factory_colors)    { delete [] factory_colors;   };
  if (factory_triangles) { delete [] factory_triangles; };	

  SCF_DESTRUCT_EMBEDDED_IBASE(scfiClothFactoryState);
  SCF_DESTRUCT_IBASE();
}

bool StuffFactory::Initialize(iObjectRegistry* iO_R)
{
  object_reg=iO_R;
  return true;
}

csPtr<iMeshObject> StuffFactory::NewInstance()
{
  csStuffObject *obj = new csStuffObject((iMeshObjectFactory*) this);
  bool initOk = obj->Initialize(object_reg);
  if (!initOk) { delete(obj);  return 0;  };
  csRef<iMeshObject> itface (SCF_QUERY_INTERFACE ( obj, iMeshObject ));
  obj->DecRef ();
  return csPtr<iMeshObject> (itface);
}

void StuffFactory::HardTransform (const csReversibleTransform &) 
{ }

bool StuffFactory::SupportsHardTransform() const 
{ return false; }

void StuffFactory::SetLogicalParent(iBase *) 
{}
  
iBase *StuffFactory::GetLogicalParent ()const 
{ return 0; }
  
//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (StuffMeshObjectType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (StuffMeshObjectType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (StuffMeshObjectType)


StuffMeshObjectType::StuffMeshObjectType (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

StuffMeshObjectType::~StuffMeshObjectType ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

csPtr<iMeshObjectFactory> StuffMeshObjectType::NewFactory ()
{
  StuffFactory* cm = new StuffFactory (this);
  cm->Initialize(object_reg);
  csRef<iMeshObjectFactory> ifact (
  	SCF_QUERY_INTERFACE (cm, iMeshObjectFactory));
  cm->DecRef ();
  return csPtr<iMeshObjectFactory> (ifact);
}

