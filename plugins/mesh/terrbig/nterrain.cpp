#define CS_SYSDEF_PROVIDE_HARDWARE_MMIO 1

#include "cssysdef.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/polyclip.h"
#include "csgeom/poly2d.h"
#include "csgeom/vector3.h"
#include "csutil/garray.h"
#include "iengine/camera.h"
#include "iengine/engine.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/vbufmgr.h"
#include "iengine/material.h"
#include "iengine/rview.h"
#include "ivideo/txtmgr.h"
#include "igraphic/image.h"
#include "iutil/objreg.h"
#include "csgfx/rgbpixel.h"
#include "imesh/object.h"
#include "csutil/mmapio.h"
#include "nterrain.h"

///////////////////////////// SCF stuff ///////////////////////////////////////////////////////////////////////

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csBigTerrainObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
SCF_IMPLEMENT_IBASE_END

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

csBigTerrainObject::csBigTerrainObject(iObjectRegistry* _obj_reg, iMeshObjectFactory *_pFactory):pFactory(_pFactory), object_reg(_obj_reg), terrain(NULL), nTextures(1)
{
  SCF_CONSTRUCT_IBASE (NULL)

  info = new nTerrainInfo();

  InitMesh(info);
}

csBigTerrainObject::~csBigTerrainObject()
{
  if (terrain) delete terrain;
  if (info)    
  {
    delete [] info->mesh;
    delete [] info->triq;
    delete info;
  }
}

void 
csBigTerrainObject::SetupVertexBuffer (iVertexBuffer *&vbuf1)
{
 if (!vbuf1)
 {
   if (!vbufmgr)
   {
     iObjectRegistry* object_reg = ((csBigTerrainObjectFactory*)pFactory)->object_reg;
     iGraphics3D* g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);

     // @@@ priority should be a parameter.
     vbufmgr = g3d->GetVertexBufferManager ();
     g3d->DecRef ();

     //vbufmgr->AddClient (&scfiVertexBufferManagerClient);
   }
   vbuf = vbufmgr->CreateBuffer (1);
 }
}

void csBigTerrainObject::InitMesh (nTerrainInfo *info)
{
  int i;
 
  info->mesh = new G3DTriangleMesh[nTextures];
  info->triq = new nTerrainInfo::triangle_queue[nTextures];

  for(i=0; i<nTextures; ++i)
  {
    info->mesh[i].triangles = info->triq[i].triangles.GetArray();
    info->mesh[i].morph_factor = 0;
    info->mesh[i].num_vertices_pool = 1;
    info->mesh[i].use_vertex_color = false;
    info->mesh[i].do_morph_texels = false;
    info->mesh[i].do_morph_colors = false;
    info->mesh[i].do_fog = false;
    info->mesh[i].vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
    info->mesh[i].mixmode = CS_FX_GOURAUD;
  }
}


bool 
csBigTerrainObject::DrawTest (iRenderView* rview, iMovable* movable)
{
  if (terrain)
  {
    iCamera* cam = rview->GetCamera ();

    terrain->SetObjectToCamera(cam->GetTransform());
    terrain->SetCameraOrigin(cam->GetTransform().GetOrigin());
    terrain->AssembleTerrain(rview, info);

    return true;
  }

  return false;
}

void 
csBigTerrainObject::UpdateLighting (iLight** lights, int num_lights, iMovable* movable)
{
  return;
}

bool 
csBigTerrainObject::Draw (iRenderView* rview, iMovable* movable, csZBufMode zbufMode)
{
  int i;
  static bufcount=0;

  iGraphics3D* pG3D = rview->GetGraphics3D ();
  iCamera* pCamera = rview->GetCamera ();

  csReversibleTransform& camtrans = pCamera->GetTransform ();
  const csVector3& origin = camtrans.GetOrigin ();

  pG3D->SetObjectToCamera (&camtrans);
  pG3D->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, zbufMode );

  SetupVertexBuffer(vbuf);

  bufcount++;

  vbufmgr->LockBuffer(vbuf, 
	 	      info->vertices.GetArray(), 
		      info->texels.GetArray(),
		      info->colors.GetArray(),
		      info->vertices.Length(),
		      bufcount);

  for(i=0; i<nTextures; ++i)
  {
    info->mesh[i].buffers[0]=vbuf;
    pG3D->DrawTriangleMesh(info->mesh[i]);    
  }

  vbufmgr->UnlockBuffer(vbuf);

  return true;
}

void 
csBigTerrainObject::GetObjectBoundingBox (csBox3& bbox, int type)
{
	
}

void 
csBigTerrainObject::GetRadius (csVector3& rad, csVector3& cent)
{

}

bool 
csBigTerrainObject::HitBeamOutline (const csVector3& start, const csVector3& end, csVector3& isect, float* pr)
{
	return false;
}

bool 
csBigTerrainObject::HitBeamObject (const csVector3& start, const csVector3& end, csVector3& isect, float* pr)
{
	return false;

}


///////////////////////////////////////////////////////////////////////////////////////////

SCF_IMPLEMENT_IBASE (csBigTerrainObjectFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
SCF_IMPLEMENT_IBASE_END

csBigTerrainObjectFactory::csBigTerrainObjectFactory (iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE (NULL);
  csBigTerrainObjectFactory::object_reg = object_reg;
  logparent = NULL;
}

csBigTerrainObjectFactory::~csBigTerrainObjectFactory ()
{
}

iMeshObject* csBigTerrainObjectFactory::NewInstance ()
{
  csBigTerrainObject* pTerrObj = new csBigTerrainObject (object_reg, this);
  return (iMeshObject*)pTerrObj;
}


