/*
        Cloth Mesh Object
            Charles Quarra
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
#include "csutil/scf.h"
#include "csutil/cscolor.h"
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

#include "plugin.h"
#include "cloth.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csStuffObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iVertexBufferManagerClient)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iGeneralMeshState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csStuffObject::eiVertexBufferManagerClient)
  SCF_IMPLEMENTS_INTERFACE (iVertexBufferManagerClient)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csStuffObject::ObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csStuffObject::eiGeneralMeshState) 
  SCF_IMPLEMENTS_INTERFACE (iGeneralMeshState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

void csStuffObject::SetupVertexBuffer ()
{  int count=0;
 if (!vbuf)
 {
	 	printf(" SETUPVERTEX BUFFER %u \n",count++);
   iObjectRegistry* object_reg = ((StuffFactory*)factory)->object_reg;
   	printf(" SETUPVERTEX BUFFER %u \n",count++);
   csRef<iGraphics3D> g3d (CS_QUERY_REGISTRY (object_reg, iGraphics3D));
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

bool csStuffObject::Initialize(iObjectRegistry *iO_R) {
printf("initializtion \n");
if (iO_R==NULL) { 	printf(" NO OBJECT_REG! \n"); return false; };
Obj_Reg=iO_R;
shift.Set(0,0,0);
max_radius.Set(5.0,5.0,0.0);
vis_cb=NULL;
vertices=NULL;
texels=NULL;
colors=NULL;
mesh.triangles=NULL;
mesh.vertex_fog=NULL;
setup=false;
return true;
};

void csStuffObject::SetupMesh()
    {
 
	 if (setup) return;
      printf( " setting up the mesh... \n");	
       setup=true;
       csTriangle *triangles;
	int num_triangles;
	//hardcoded for now
	Xsize=40;
	Ysize=40;
        num_vertices=( Xsize + 1 )*( Ysize + 1 );
	num_triangles=4 * Xsize * Ysize;
	int count = 0;
	printf(" %u \n",count++);
	if (vertices) delete[](vertices);
	if (texels) delete[](texels);
	if (colors) delete[](colors);
	//vertices=new csVector3[num_vertices];
	texels=new csVector2[num_vertices];
	colors=new csColor[num_vertices];
	Integrator=new ClothIntegrator( Xsize , Ysize , shift , object_bbox ,csVector3(0,0,0.1) );
	vertices=Integrator->GetVerticeBuffer();
	
	float xindex=0.0;
	float yindex=0.0;
	time=0.0;
	int i;
	int j;
	int k;
 object_bbox.StartBoundingBox ( shift + csVector3(0,0,0.1));
    
     for (i=0;i<num_vertices;i++) {
		 vertices[i].z=0.0;  //(xindex-3)*(yindex-3);
		 vertices[i].x=xindex;
		 vertices[i].y=yindex;
		 object_bbox.AddBoundingVertexSmart ( vertices[i] + shift +csVector3(0,0,-0.1) );
		 texels[i].Set(xindex/2.1f , yindex/2.1f );
		 colors[i].Set (1.0f, 1.0f, 1.0f);
		 xindex+=2.0/Xsize;
		 if ( ((i+1)%(Xsize+1))==0 ) { xindex=0.0; yindex+=2.0/Ysize;  };
		 	 };	
	 	printf(" %u \n",count++);

	SetupVertexBuffer();
		printf(" %u \n",count++);

  triangles = new csTriangle[num_triangles];
   k=0;
   for (i=0;i<Xsize;i++) {
	for (j=0;j<Ysize;j++) {
				
	   triangles[k].a=i + (Xsize + 1)*j;   // ahh lot of funs with triangles
	   triangles[k].b=i + 1 + (Xsize + 1)*j;
	   triangles[k].c=i + (Xsize + 1)*(j+1);  
	   k++;

	   triangles[k].a=i + (Xsize + 1)*(j+1);
	   triangles[k].b=i + 1 + (Xsize + 1)*j;
	   triangles[k].c=i + 1 + (Xsize + 1)*(j+1);
	   k++;

	   //-----------now anti-clock-wise for drawing the other side
	   triangles[k].a=i + (Xsize + 1)*j;   // ahh lot of funs with triangles
	   triangles[k].c=i + 1 + (Xsize + 1)*j;
	   triangles[k].b=i + (Xsize + 1)*(j+1);  
	   k++;

	   triangles[k].a=i + (Xsize + 1)*(j+1);
	   triangles[k].c=i + 1 + (Xsize + 1)*j;
	   triangles[k].b=i + 1 + (Xsize + 1)*(j+1);
	   k++;

          };		   
   };

  mesh.num_triangles = num_triangles;
  if (mesh.triangles)  { delete[](mesh.triangles);  };
  mesh.triangles=triangles;  
  mesh.vertex_fog = new G3DFogInfo[num_vertices];
  //memcpy (mesh.triangles, triangles, sizeof(csTriangle)*num_triangles);
    mesh.morph_factor = 0.0f;
    mesh.num_vertices_pool = 1;
    mesh.do_morph_texels = false;
    mesh.do_morph_colors = false;
    mesh.vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;

    /*
    Integrator->ComputeInitial();
    Integrator->ComputeInitial();
    Integrator->ComputeInitial();
    Integrator->ComputeInitial();
    */
       printf( " finished setting up the mesh... \n");	

};


void csStuffObject::UpdateMesh() 
{
	Integrator->Compute();
	/*
time+=0.05;
	
	float xindex=0.0;
	float yindex=0.0;
	int i;
	int j;
	int k;
 object_bbox.StartBoundingBox ( shift +csVector3(0,0,1.0));
    
     for (i=0;i<num_vertices;i++) {
		 vertices[i].z=cos(1./(cos(pow( xindex-3 , 2 )+pow( yindex-3 ,2)-time*time)+2.1) );
		 vertices[i].x=xindex;
		 vertices[i].y=yindex;
		 object_bbox.AddBoundingVertexSmart ( vertices[i] + shift +csVector3(0,0,-1.0) );
	//	 texels[i].Set(xindex/5.1f , yindex/5.1f );
	//	 colors[i].Set (1.0f, 1.0f, 1.0f);
		 xindex+=5.0/Xsize;
		 if ( ((i+1)%(Xsize+1))==0 ) { xindex=0.0; yindex+=5.0/Ysize;  };
		 	 };	
*/

};


//-------------------MeshObject implementation
//

csStuffObject::csStuffObject(iMeshObjectFactory *fact) {
printf("creator \n");
factory = fact;
vbuf=NULL;
vbufmgr=NULL;
SCF_CONSTRUCT_IBASE(fact);
SCF_CONSTRUCT_EMBEDDED_IBASE(scfiVertexBufferManagerClient);
SCF_CONSTRUCT_EMBEDDED_IBASE(scfiObjectModel);
SCF_CONSTRUCT_EMBEDDED_IBASE(scfiGeneralMeshState);
};

csStuffObject::~csStuffObject() 
{
	// this is YUCK
//	if (mesh.vertex_fog) { delete[](mesh.vertex_fog); };
//	if (mesh.triangl

};


bool csStuffObject::DrawTest(iRenderView *rview, iMovable *movable) {
//printf(" draw test \n");
// SetupMesh();

 iCamera* camera = rview->GetCamera ();
 csReversibleTransform tr_o2c = camera->GetTransform () / movable->GetFullTransform ();

  csVector3 radius;
  csSphere sphere;
  GetRadius (radius, sphere.GetCenter ());
  float max_radius = radius.x;
  if (max_radius < radius.y) max_radius = radius.y;
  if (max_radius < radius.z) max_radius = radius.z;
  sphere.SetRadius (max_radius);
  
   int clip_portal, clip_plane, clip_z_plane;
  if (rview->ClipBSphere (tr_o2c, sphere, clip_portal, clip_plane,
  	clip_z_plane) == false)
     {   return false; };



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


bool csStuffObject::Draw(iRenderView *rview, iMovable *movable, csZBufMode zbufmode){
//printf("       DRAW\n");
 if (!material)
  {
    printf ("INTERNAL ERROR: ball used without material!\n");
    return false;
  }
  iMaterialHandle* mat = material->GetMaterialHandle ();
  if (!mat)
  {
    printf ("INTERNAL ERROR: ball used without valid material handle!\n");
    return false;
  }

  if (vis_cb) if (!vis_cb->BeforeDrawing (this, rview)) return false;

  iGraphics3D* g3d = rview->GetGraphics3D ();

  // Prepare for rendering.
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, zbufmode);

  //SetupVertexBuffer ();
  material->Visit ();
  mesh.mat_handle = mat;
  mesh.use_vertex_color =true;
  mesh.mixmode = MixMode | CS_FX_GOURAUD;
  CS_ASSERT (!vbuf->IsLocked ());
  vbufmgr->LockBuffer (vbuf,
  	vertices, texels, colors, num_vertices, 0);
  rview->CalculateFogMesh (g3d->GetObjectToCamera (), mesh);
  g3d->DrawTriangleMesh (mesh);
  vbufmgr->UnlockBuffer (vbuf);

return true;
};

void csStuffObject::GetObjectBoundingBox (csBox3& bbox, int /*type*/)
{
  SetupMesh ();
  bbox = object_bbox;
  	printf("  exiting GOBB  \n");
}


 iMeshObjectFactory* csStuffObject::GetFactory () const
 { return factory; };
 void csStuffObject::UpdateLighting (iLight **,int, iMovable *) {
  SetupMesh();
  	printf(" exiting UpdateLighting  \n");
 };
 void csStuffObject::SetVisibleCallback(iMeshObjectDrawCallback *cb) { vis_cb=cb; };
 iMeshObjectDrawCallback* csStuffObject::GetVisibleCallback () const { return vis_cb; };
 void csStuffObject::NextFrame (unsigned int)
 {
   Integrator->Compute();	 
   //UpdateMesh();
 };
 bool csStuffObject::WantToDie () const { return false; };
 void csStuffObject::HardTransform (const csReversibleTransform &) {};
 bool csStuffObject::SupportsHardTransform () const { return false; };
 bool csStuffObject::HitBeamOutline (const csVector3 &, const csVector3 &, csVector3 &, float *)
    { return false; };
 bool csStuffObject::HitBeamObject (const csVector3 &, const csVector3 &, csVector3 &, float *)
     { return false; };
 void csStuffObject::SetLogicalParent (iBase *) {};
 iBase *csStuffObject::GetLogicalParent () const { return NULL; };
 // iObjectModel *csStuffObject::GetObjectModel () { return NULL; };

//-----------Vertex Buffer Manager Client embedded implementation
 void csStuffObject::eiVertexBufferManagerClient::ManagerClosing() { };



//--//--//--//--//-- StuffFactory

SCF_IMPLEMENT_IBASE(StuffFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE(StuffFactory::Component)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

StuffFactory::StuffFactory(iBase* parent) {
SCF_CONSTRUCT_IBASE( parent )
SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent)
object_reg=NULL;
};

StuffFactory::~StuffFactory() {};

bool StuffFactory::Initialize(iObjectRegistry* iO_R) { object_reg=iO_R; return true; };

csPtr<iMeshObject> StuffFactory::NewInstance() {

csStuffObject *obj = new csStuffObject((iMeshObjectFactory*) this);
bool initOk = obj->Initialize(object_reg);
if (!initOk) { delete(obj);  return NULL;  };
csRef<iMeshObject> itface (SCF_QUERY_INTERFACE ( obj, iMeshObject ));
return csPtr<iMeshObject> (itface);	// DecRef is ok here.
};

 void StuffFactory::HardTransform (const csReversibleTransform &) { };
 bool StuffFactory::SupportsHardTransform() const { return false; };
 void StuffFactory::SetLogicalParent(iBase *) {};
 iBase *StuffFactory::GetLogicalParent ()const { return NULL; };


//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (StuffMeshObjectType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (StuffMeshObjectType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (StuffMeshObjectType)

SCF_EXPORT_CLASS_TABLE (cloth)
SCF_EXPORT_CLASS ( StuffMeshObjectType , "crystalspace.mesh.object.cloth" ,
               "CrystalSpace Cloth mesh object type" )
SCF_EXPORT_CLASS_TABLE_END

StuffMeshObjectType::StuffMeshObjectType (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

StuffMeshObjectType::~StuffMeshObjectType ()
{
}

csPtr<iMeshObjectFactory> StuffMeshObjectType::NewFactory ()
{
  StuffFactory* cm = new StuffFactory (this);
  cm->Initialize(object_reg);
  csRef<iMeshObjectFactory> ifact (
  	SCF_QUERY_INTERFACE (cm, iMeshObjectFactory));
  return csPtr<iMeshObjectFactory> (ifact);	// DecRef is ok here.
}

