/*
    Copyright (C) 2001 by W.C.A. Wijngaards
  
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

#define CS_SYSDEF_PROVIDE_ALLOCA
#include "cssysdef.h"
#include "isomesh.h"
#include "csutil/scf.h"
#include "ivideo/graph3d.h"
#include "csgeom/math2d.h"
#include "csgeom/polyclip.h"
#include "ivideo/material.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"
#include "iengine/material.h"
#include "imesh/object.h"
#include "isys/system.h"
#include "qint.h"

#include "iengine/movable.h"
#include "iengine/rview.h"
#include "iengine/camera.h"

//------------- Fake 3d Interfaces -----------------------------
/// fake a movable for a MeshSprite
class csIsoFakeMovable : public iMovable
{
  iIsoMeshSprite *isomesh;
  /// a transform, for passing refs, information is kept in the mesh though.
  csReversibleTransform obj;
  long updatenumber;
public:
  SCF_DECLARE_IBASE;
  csIsoFakeMovable(iIsoMeshSprite *t) {isomesh = t; updatenumber = 0;}
  virtual ~csIsoFakeMovable() {}

  //----- iMovable -------------------------------
  virtual iMovable* GetParent () const {return 0;}
  virtual void SetSector (iSector* ) { updatenumber++; }
  virtual void ClearSectors () { updatenumber++; }
  virtual void AddSector (iSector* ) { updatenumber++; }
  virtual const iSectorList *GetSectors () const {return (iSectorList*)0;}
  virtual iSector* GetSector (int ) const {return 0;}
  virtual int GetSectorCount () const { return 0; }
  virtual bool InSector () const {return true;}
  virtual void SetPosition (iSector*, const csVector3& v)
  { isomesh->SetPosition(v); updatenumber++; }
  virtual void SetPosition (const csVector3& v)
  {
    isomesh->SetPosition(v);
    updatenumber++;
  }
  virtual const csVector3& GetPosition () const {return isomesh->GetPosition();}
  virtual const csVector3 GetFullPosition () const {return isomesh->GetPosition();}
  virtual void SetTransform (const csReversibleTransform& t) 
  {
    isomesh->SetTransform(t.GetT2O());
    isomesh->SetPosition(t.GetOrigin());
    updatenumber++;
  }
  virtual csReversibleTransform& GetTransform () 
  { 
    obj.SetT2O(isomesh->GetTransform()); 
    obj.SetOrigin(isomesh->GetPosition());
    return obj;
  }
  virtual csReversibleTransform GetFullTransform () const
  { 
    csReversibleTransform obj;
    obj.SetT2O(isomesh->GetTransform()); 
    obj.SetOrigin(isomesh->GetPosition());
    return obj;
  }
  virtual void MovePosition (const csVector3& v)
  {
    isomesh->MovePosition(v);
    updatenumber++;
  }
  virtual void SetTransform (const csMatrix3& matrix)
  {
    isomesh->SetTransform( matrix );
    updatenumber++;
  }
  virtual void Transform (const csMatrix3& matrix)
  {
    isomesh->SetTransform( matrix * isomesh->GetTransform() );
    updatenumber++;
  }
  virtual void AddListener (iMovableListener* /*listener*/, void* /*userdata*/)
  {
     /// does not work
  }
  virtual void RemoveListener (iMovableListener* )
  {
     /// does not work
  }
  virtual void UpdateMove () { updatenumber++; }
  virtual long GetUpdateNumber () const
  {
     return updatenumber;
  }

};

SCF_IMPLEMENT_IBASE (csIsoFakeMovable)
  SCF_IMPLEMENTS_INTERFACE (iMovable)
SCF_IMPLEMENT_IBASE_END


/// fake 3d render view ...
class csIsoFakeRenderView : public iRenderView {
  iCamera *fakecam;
  iIsoRenderView *isorview;
  /**
   * A callback function. If this is set then no drawing is done.
   * Instead the callback function is called.
   */
  iDrawFuncCallback* callback;
  
public:
  SCF_DECLARE_IBASE;
  csIsoFakeRenderView() 
  {
    callback = NULL;
  }
  virtual ~csIsoFakeRenderView() 
  {
    SCF_DEC_REF (callback);
  }

  /// set data to render an isometric mesh
  void SetIsoData(iIsoRenderView *r, iCamera *cam)
  {
    isorview = r;
    fakecam = cam;
  }

  //---------------- iRenderView -----------------------
  virtual csRenderContext* GetRenderContext () {return 0;}
  virtual void CreateRenderContext() {}
  virtual void RestoreRenderContext (csRenderContext* ) {}
  virtual iCamera* CreateNewCamera() {return fakecam;} //@@@ copy?
  virtual iEngine* GetEngine () {return 0;}
  virtual iGraphics2D* GetGraphics2D () 
  {return isorview->GetG3D()->GetDriver2D();}
  virtual iGraphics3D* GetGraphics3D () {return isorview->GetG3D();}
  virtual void SetFrustum (float, float, float, float) {}
  virtual void GetFrustum (float&, float&, float&, float&) {}
  virtual iClipper2D* GetClipper () {return isorview->GetClipper();}
  virtual void SetClipper (iClipper2D*) {}
  virtual bool IsClipperRequired () {return false;}
  virtual bool GetClipPlane (csPlane3& ) {return false;}
  virtual csPlane3& GetClipPlane () {return *(csPlane3*)0;}
  virtual void SetClipPlane (const csPlane3& ) {}
  virtual void UseClipPlane (bool ) {}
  virtual void UseClipFrustum (bool ) {}
  virtual csFogInfo* GetFirstFogInfo () {return 0;}
  virtual void SetFirstFogInfo (csFogInfo* ) {}
  virtual bool AddedFogInfo () {return false;}
  virtual void ResetFogInfo () {}
  virtual iCamera* GetCamera () {return fakecam;}
  virtual void CalculateFogPolygon (G3DPolygonDP& ) {}
  virtual void CalculateFogPolygon (G3DPolygonDPFX& ) {}
  virtual void CalculateFogMesh (const csTransform& , G3DTriangleMesh& ) {}
  virtual bool ClipBBox (const csBox2& sbox, const csBox3& /*cbox*/,
          int& clip_portal, int& clip_plane, int& clip_z_plane) 
  {
    clip_plane = CS_CLIP_NOT;
    /// test if chance that we must clip to a portal -> or the Toplevel clipper
    /// better: only if it crosses that.
    const csRect& rect = isorview->GetView()->GetRect();
    if( (rect.xmin >= QInt(sbox.MinX())) || (rect.xmax <= QInt(sbox.MaxX())) ||
        (rect.ymin >= QInt(sbox.MinY())) || (rect.ymax <= QInt(sbox.MaxY())) )
      clip_portal = CS_CLIP_TOPLEVEL;
    else clip_portal = CS_CLIP_NOT;
    /// test if z becomes negative, should never happen
    clip_z_plane = CS_CLIP_NOT;
    return true;
  }

  virtual iSector* GetThisSector () {return 0;}
  virtual void SetThisSector (iSector* ) {}
  virtual iSector* GetPreviousSector () {return 0;}
  virtual void SetPreviousSector (iSector* ) {}
  virtual iPolygon3D* GetPortalPolygon () {return 0;}
  virtual void SetPortalPolygon (iPolygon3D* ) {}
  virtual int GetRenderRecursionLevel () {return 0;}
  virtual void SetRenderRecursionLevel (int ) {}
  virtual void AttachRenderContextData (void* key, iBase* data)
  {
    (void)key;
    (void)data;
  }
  virtual iBase* FindRenderContextData (void* key)
  {
    (void)key;
    return 0;
  }
  virtual void DeleteRenderContextData (void* key)
  {
    (void)key;
  }
  virtual void SetCallback (iDrawFuncCallback* cb)
  {
    SCF_SET_REF (callback, cb);
  }
  virtual iDrawFuncCallback* GetCallback ()
  {
    return callback;
  }
  virtual void CallCallback (int type, void* data)
  {
    callback->DrawFunc (this, type, data);
  }
};

SCF_IMPLEMENT_IBASE (csIsoFakeRenderView)
  SCF_IMPLEMENTS_INTERFACE (iRenderView)
SCF_IMPLEMENT_IBASE_END


//------------ IsoMeshSprite ------------------------------------

SCF_IMPLEMENT_IBASE (csIsoMeshSprite)
  SCF_IMPLEMENTS_INTERFACE (iIsoMeshSprite)
  SCF_IMPLEMENTS_INTERFACE (iIsoSprite)
SCF_IMPLEMENT_IBASE_END

csIsoMeshSprite::csIsoMeshSprite (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  position.Set(0,0,0);
  transform.Identity();
  grid = NULL;
  gridcall = NULL;
  mesh = NULL;
  zbufmode = CS_ZBUF_USE;
}

csIsoMeshSprite::~csIsoMeshSprite ()
{
  if (mesh) mesh->DecRef();
  if (gridcall) gridcall->DecRef ();
}


void csIsoMeshSprite::SetMeshObject(iMeshObject *mesh)
{
  if(mesh) mesh->IncRef();
  if(csIsoMeshSprite::mesh) csIsoMeshSprite::mesh->DecRef();
  csIsoMeshSprite::mesh = mesh;
}

void csIsoMeshSprite::Draw(iIsoRenderView *rview)
{
  //printf("isomeshsprite::Draw(%g, %g, %g)\n", position.x, position.y,
    //position.z);

  /// update animation
  mesh->NextFrame(rview->GetView()->GetEngine()->GetSystem()->GetTime());

  //iGraphics3D* g3d = rview->GetG3D ();
  iIsoView* view = rview->GetView ();

  // Prepare for rendering.
  //g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, zbufmode);

  /// create a fake environment for the mesh so that it will render nicely.
  /// the meshes use a perspective transform, and we do not want that.
  /// but an approximation of the isometric transform can be given.

  // It works like this:
  // camera.x,y,z are a linear combination of x,y,z.
  // -> this gives the fake camera transformation matrix.
  // the fake camera z = iso.z-iso.x.
  // The camera translation is then used to correct stuff.
  // the z translation is set to -zlowerbound.
  // Now the fov is set to iso.z - iso.x - zlowerbound.
  // (for the iso coords of the center position).
  // (this is why it is only an approximation).
  //
  // when the mesh object draws, the following will happen:
  // the point is transformed using the given camera.
  // iz = fov/z.   (note that fov is almost equal to z, so iz is almost 1)
  // sx = x*iz + shiftx;
  // sy = y*iz + shifty;
  //   this will leave x,y almost unchanged, and shifted (as per iso camera).
  // the depth buffer is filled with 1./z. , but this is (due to fake camera)
  // equal to 1./(iso.z - iso.x - zlowerbound), which is exactly what the
  // isometric engine uses as depth buffer values.

  /// create fake moveable
  csIsoFakeMovable* movable = new csIsoFakeMovable(this);

  /// create fake camera
  iCamera *fakecam = view->GetFakeCamera(position, rview);

  /// create fake renderview
  csIsoFakeRenderView *fakerview = new csIsoFakeRenderView();
  fakerview->SetIsoData(rview, fakecam);

  if(mesh->DrawTest(fakerview, movable))
  {
    //printf("mesh draw()\n");
    /// UpdateLighting ....
    iLight **lights = NULL;
    int numlights = 0;
    grid->GetFakeLights(position, lights, numlights);
    mesh->UpdateLighting(lights, numlights, movable);
    if(mesh->Draw(fakerview, movable, zbufmode))
    {
      //printf("mesh prob vis\n");
      /// was probably visible
    }
  }

  delete movable;
  delete fakerview;
  return;// true;
}


//------ further iIsoSprite compliance implementation ---------

int csIsoMeshSprite::GetVertexCount() const
{
  return 0;
}

void csIsoMeshSprite::AddVertex(const csVector3& /*coord*/, float /*u*/, 
  float /*v*/)
{
  /// no effect
}


void csIsoMeshSprite::SetPosition(const csVector3& newpos) 
{
  /// manage movement of the sprite, oldpos, newpos
  csVector3 oldpos = position;
  position = newpos; // already set, so it can be overridden
  if(grid) grid->MoveSprite(this, oldpos, newpos);
}

void csIsoMeshSprite::MovePosition(const csVector3& delta) 
{
  SetPosition(position + delta);
}


void csIsoMeshSprite::SetMaterialWrapper(iMaterialWrapper * /*material*/)
{
  // nothing
}

iMaterialWrapper* csIsoMeshSprite::GetMaterialWrapper() const
{
  return NULL;
}

void csIsoMeshSprite::SetMixMode(UInt /*mode*/)
{
  // nothing
}

UInt csIsoMeshSprite::GetMixMode() const
{
  /// CS_FX_COPY -> rendered in main pass,
  /// CS_FX_ADD -> in alpha pass
  if((zbufmode == CS_ZBUF_USE)
     ||(zbufmode == CS_ZBUF_FILL))
     return CS_FX_COPY;
  return CS_FX_ADD;
}

void csIsoMeshSprite::SetGrid(iIsoGrid *grid)
{
  if(csIsoMeshSprite::grid != grid)
  {
    csIsoMeshSprite::grid = grid;
    if(gridcall) gridcall->GridChange (this);
  }
}

void csIsoMeshSprite::SetAllColors(const csColor& /*color*/)
{
  // nothing to happen
}

const csVector3& csIsoMeshSprite::GetVertexPosition(int /*i*/)
{
  static csVector3 pos(0,0,0);
  return pos;
}

void csIsoMeshSprite::AddToVertexColor(int /*i*/, const csColor& /*color*/)
{
  /// nothing
}

void csIsoMeshSprite::ResetAllColors()
{
  /// nothing
}

void csIsoMeshSprite::SetAllStaticColors(const csColor& /*color*/)
{
  /// nothing
}

void csIsoMeshSprite::AddToVertexStaticColor(int /*i*/, 
  const csColor& /*color*/)
{
  /// nothing
}


