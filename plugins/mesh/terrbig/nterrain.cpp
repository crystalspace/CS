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

csBigTerrainObject::csBigTerrainObject():pFactory(NULL), terrain(NULL)
{
}

csBigTerrainObject::~csBigTerrainObject()
{
  if (terrain) delete terrain;
}

void 
csBigTerrainObject::SetupVertexBuffer (iVertexBuffer *&vbuf1)
{
 if (!vbuf1)
 {
   if (!vbufmgr)
   {
     //iObjectRegistry* object_reg = ((csTerrFuncObjectFactory*)pFactory)->object_reg;
     //iGraphics3D* g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);

     // @@@ priority should be a parameter.
     //vbufmgr = g3d->GetVertexBufferManager ();
     //g3d->DecRef ();

     //vbufmgr->AddClient (&scfiVertexBufferManagerClient);
   }
   //vbuf1 = vbufmgr->CreateBuffer (1);
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
    terrain->AssembleTerrain(rview);

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
  iGraphics3D* pG3D = rview->GetGraphics3D ();
  iCamera* pCamera = rview->GetCamera ();

  csReversibleTransform& camtrans = pCamera->GetTransform ();
  const csVector3& origin = camtrans.GetOrigin ();

  pG3D->SetObjectToCamera (&camtrans);
  pG3D->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, zbufMode );

  SetupVertexBuffer(vbuf);

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





