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

#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "isomesh.h"
#include "csutil/scf.h"
#include "ivideo/graph3d.h"
#include "csgeom/sphere.h"
#include "csgeom/math2d.h"
#include "csgeom/polyclip.h"
#include "ivideo/material.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"
#include "iengine/material.h"
#include "imesh/object.h"
#include "qint.h"

#include "iengine/movable.h"
#include "iengine/rview.h"
#include "iengine/camera.h"

struct iPortal;

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
  virtual void SetParent (iMovable* /*parent*/) { return; }
  virtual void SetSector (iSector* ) { updatenumber++; }
  virtual void ClearSectors () { updatenumber++; }
  virtual void AddSector (iSector* ) { updatenumber++; }
  virtual iSectorList *GetSectors () {return (iSectorList*)0;}
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
  virtual void AddListener (iMovableListener* /*listener*/)
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

  virtual bool IsTransformIdentity () const { return false; }
  virtual bool IsFullTransformIdentity () const { return false; }
  virtual void TransformIdentity ()
  {
    isomesh->SetTransform (csMatrix3 ());
    isomesh->SetPosition (csVector3 (0));
  }
};

SCF_IMPLEMENT_IBASE (csIsoFakeMovable)
  SCF_IMPLEMENTS_INTERFACE (iMovable)
SCF_IMPLEMENT_IBASE_END


/// fake 3d render view ...
class csIsoFakeRenderView : public iRenderView
{
  iCamera *fakecam;
  iIsoRenderView *isorview;
public:
  SCF_DECLARE_IBASE;
  csIsoFakeRenderView()
  {
  }
  virtual ~csIsoFakeRenderView()
  {
  }

  /// set data to render an isometric mesh
  void SetIsoData(iIsoRenderView *r, iCamera *cam)
  {
    isorview = r;
    fakecam = cam;
  }

  //---------------- iRenderView -----------------------
  virtual csRenderContext* GetRenderContext () {return 0;}
  virtual iCamera* CreateNewCamera() {return fakecam;} //@@@ copy?
  virtual iEngine* GetEngine () {return 0;}
  virtual iGraphics2D* GetGraphics2D ()
  {return isorview->GetG3D()->GetDriver2D();}
  virtual iGraphics3D* GetGraphics3D () {return isorview->GetG3D();}
  virtual void GetFrustum (float&, float&, float&, float&) {}
  virtual iClipper2D* GetClipper () {return isorview->GetClipper();}
  virtual iCamera* GetCamera () {return fakecam;}
  virtual void CalculateFogPolygon (G3DPolygonDP& poly) { poly.use_fog = false; }
  virtual void CalculateFogPolygon (G3DPolygonDPFX& poly) { poly.use_fog = false; }
  virtual void CalculateFogMesh (const csTransform& , G3DTriangleMesh& mesh)
  {
    mesh.do_fog = false;
  }
  virtual void CalculateFogMesh (const csTransform& , G3DPolygonMesh& mesh)
  {
    mesh.do_fog = false;
  }

  virtual bool TestBSphere (const csReversibleTransform& o2c,
	const csSphere& sphere)
  {
    csSphere tr_sphere = o2c.Other2This (sphere);
    const csVector3& tr_center = tr_sphere.GetCenter ();
    float radius = tr_sphere.GetRadius ();

    float sx = fakecam->GetShiftX ();
    float sy = fakecam->GetShiftY ();
    float inv_fov = fakecam->GetInvFOV ();
    const csRect& rect = isorview->GetView()->GetRect();
    float xmin = (rect.xmin - sx) * inv_fov;
    float ymin = (rect.ymin - sy) * inv_fov;
    float xmax = (rect.xmax - sx) * inv_fov;
    float ymax = (rect.ymax - sy) * inv_fov;
    /// test if chance that we must clip to a portal -> or the Toplevel clipper
    /// better: only if it crosses that.
    bool outside = true, inside = true;
    csVector3 v1 (xmin, ymin, 1);
    csVector3 v2 (xmax, ymin, 1);
    float dist = csVector3::Unit (v1 % v2) * tr_center;
    if ((-dist) <= radius)
    {
      if (dist < radius) inside = false;
      csVector3 v3 (xmax, ymax, 1);
      dist = csVector3::Unit (v2 % v3) * tr_center;
      if ((-dist) <= radius)
      {
        if (dist < radius) inside = false;
        v2.Set (xmin, ymax, 1);
        dist = csVector3::Unit (v3 % v2) * tr_center;
        if ((-dist) <= radius)
        {
          if (dist < radius) inside = false;
          dist = csVector3::Unit (v2 % v1) * tr_center;
          if ((-dist) <= radius)
	  {
	    outside = false;
            if (dist < radius) inside = false;
	  }
        }
      }
    }
    if (outside) return false;
    return true;
  }
  virtual bool ClipBBox (const csBox2& sbox, const csBox3& /*cbox*/,
          int& clip_portal, int& clip_plane, int& clip_z_plane)
  {
    clip_plane = CS_CLIP_NOT;
    /// test if chance that we must clip to a portal -> or the Toplevel clipper
    /// better: only if it crosses that.
    const csRect& rect = isorview->GetView()->GetRect();
    if( (rect.xmin >= QInt(sbox.MinX())) || (rect.xmax <= QInt(sbox.MaxX())) ||
        (rect.ymin >= QInt(sbox.MinY())) || (rect.ymax <= QInt(sbox.MaxY())) )
      clip_portal = CS_CLIP_NEEDED;
    else
      clip_portal = CS_CLIP_NOT;
    /// test if z becomes negative, should never happen
    clip_z_plane = CS_CLIP_NOT;
    //printf("ClipBBox %g,%g %g,%g gives portal=%d plane=%d z=%d return true\n",
      //sbox.MinX(), sbox.MinY(), sbox.MaxX(), sbox.MaxY(), clip_portal,
      //clip_plane, clip_z_plane);
    return true;
  }
  virtual void CalculateClipSettings (uint32,
    int &clip_portal, int &clip_plane, int &clip_z_plane)
  {
    clip_plane = CS_CLIP_NOT;
    clip_portal = CS_CLIP_NOT;
    clip_z_plane = CS_CLIP_NOT;
  }
  virtual bool ClipBBox (const csBox3& /*cbox*/,
          int& clip_portal, int& clip_plane, int& clip_z_plane)
  {
    clip_plane = CS_CLIP_NOT;
    clip_portal = CS_CLIP_NOT;
    clip_z_plane = CS_CLIP_NOT;
    return true;
  }
  virtual bool ClipBBox (csPlane3*, uint32&, const csBox3& /*obox*/,
          int& clip_portal, int& clip_plane, int& clip_z_plane)
  {
    clip_plane = CS_CLIP_NOT;
    clip_portal = CS_CLIP_NOT;
    clip_z_plane = CS_CLIP_NOT;
    return true;
  }
  virtual void SetupClipPlanes (const csReversibleTransform&,
  	csPlane3*, uint32& frustum_mask)
  {
    frustum_mask = 0;
  }

  virtual iSector* GetThisSector () {return 0;}
  virtual iSector* GetPreviousSector () {return 0;}
  virtual iPortal* GetLastPortal () {return 0;}
  virtual iCamera* GetOriginalCamera () const { return 0; }
  virtual uint GetCurrentFrameNumber () const 
  { return 0; /* @@@ Iso engine doesn't have frame number */}
};

SCF_IMPLEMENT_IBASE (csIsoFakeRenderView)
  SCF_IMPLEMENTS_INTERFACE (iRenderView)
SCF_IMPLEMENT_IBASE_END


//------------ IsoMeshSprite ------------------------------------

SCF_IMPLEMENT_IBASE (csIsoMeshSprite)
  SCF_IMPLEMENTS_INTERFACE (iIsoMeshSprite)
  SCF_IMPLEMENTS_INTERFACE (iIsoSprite)
SCF_IMPLEMENT_IBASE_END

CS_IMPLEMENT_STATIC_CLASSVAR (csIsoMeshSprite, pos, GetVertexPosition, csVector3, (0))

csIsoMeshSprite::csIsoMeshSprite (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  position.Set(0,0,0);
  transform.Identity();
  grid = 0;
  mesh = 0;
  zbufmode = CS_ZBUF_USE;
}

csIsoMeshSprite::~csIsoMeshSprite ()
{
  if (mesh)
    mesh->DecRef();
  SCF_DESTRUCT_IBASE();
}


void csIsoMeshSprite::SetMeshObject(iMeshObject *mesh)
{
  if(mesh) mesh->IncRef();
  if(csIsoMeshSprite::mesh) csIsoMeshSprite::mesh->DecRef();
  csIsoMeshSprite::mesh = mesh;
  if (mesh) mesh->SetLogicalParent (this);
}

const csArray<iLight*>& csIsoMeshSprite::GetRelevantLights (int /*maxLights*/,
	bool /*desireSorting*/)
{
  grid->GetFakeLights (position, relevant_lights);
  return relevant_lights;
}

void csIsoMeshSprite::Draw(iIsoRenderView *rview)
{
  //printf("isomeshsprite::Draw(%g, %g, %g)\n", position.x, position.y,
    //position.z);

  /// update animation
  mesh->NextFrame(csGetTicks (), csVector3 (0, 0, 0));

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

  if (mesh->DrawTest (fakerview, movable, 0xff))	// @@@ Fake!
  {
    if (mesh->Draw (fakerview, movable, zbufmode))
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
  return 0;
}

void csIsoMeshSprite::SetMixMode(uint /*mode*/)
{
  // nothing
}

uint csIsoMeshSprite::GetMixMode() const
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
    if (gridcall) gridcall->GridChange (this);
  }
}

void csIsoMeshSprite::SetAllColors(const csColor& /*color*/)
{
  // nothing to happen
}

const csVector3& csIsoMeshSprite::GetVertexPosition(int /*i*/)
{
  return *GetVertexPosition ();;
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

// ---------------------------------------------------------------------------
// csIsoMeshFactoryWrapper
// ---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_EXT (csIsoMeshFactoryWrapper)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iMeshFactoryWrapper)
  SCF_IMPLEMENTS_INTERFACE (csIsoMeshFactoryWrapper)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csIsoMeshFactoryWrapper::MeshFactoryWrapper)
  SCF_IMPLEMENTS_INTERFACE (iMeshFactoryWrapper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csIsoMeshFactoryWrapper::csIsoMeshFactoryWrapper (iMeshObjectFactory* meshFact)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMeshFactoryWrapper);
  csIsoMeshFactoryWrapper::meshFact = meshFact;
  meshFact->IncRef ();
}

csIsoMeshFactoryWrapper::csIsoMeshFactoryWrapper ()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMeshFactoryWrapper);
  csIsoMeshFactoryWrapper::meshFact = 0;
}

csIsoMeshFactoryWrapper::~csIsoMeshFactoryWrapper ()
{
  if (meshFact)
    meshFact->DecRef ();
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiMeshFactoryWrapper);
}

void csIsoMeshFactoryWrapper::SetMeshObjectFactory (
	iMeshObjectFactory* meshFact)
{
  if (meshFact) meshFact->IncRef ();
  if (csIsoMeshFactoryWrapper::meshFact)
    csIsoMeshFactoryWrapper::meshFact->DecRef ();
  csIsoMeshFactoryWrapper::meshFact = meshFact;
}

void csIsoMeshFactoryWrapper::HardTransform (const csReversibleTransform& t)
{
  meshFact->HardTransform (t);
}

//--------------------------------------------------------------------------
// csIsoMeshFactoryList
//--------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csIsoMeshFactoryList)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iMeshFactoryList)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csIsoMeshFactoryList::MeshFactoryList)
  SCF_IMPLEMENTS_INTERFACE (iMeshFactoryList)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csIsoMeshFactoryList::csIsoMeshFactoryList ()
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMeshFactoryList);
}

csIsoMeshFactoryList::~csIsoMeshFactoryList ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiMeshFactoryList);
  SCF_DESTRUCT_IBASE();
}

int csIsoMeshFactoryList::MeshFactoryList::GetCount () const
  { return scfParent->Length (); }
iMeshFactoryWrapper *csIsoMeshFactoryList::MeshFactoryList::Get (int n) const
  { return scfParent->Get (n); }
int csIsoMeshFactoryList::MeshFactoryList::Add (iMeshFactoryWrapper *obj)
  { scfParent->Push (obj); return true; }
bool csIsoMeshFactoryList::MeshFactoryList::Remove (iMeshFactoryWrapper *obj)
  { scfParent->Delete (obj); return true; }
bool csIsoMeshFactoryList::MeshFactoryList::Remove (int n)
  { scfParent->Delete (scfParent->Get (n)); return true; }
void csIsoMeshFactoryList::MeshFactoryList::RemoveAll ()
  { scfParent->DeleteAll (); }
int csIsoMeshFactoryList::MeshFactoryList::Find (iMeshFactoryWrapper *obj) const
  { return scfParent->Find (obj); }
iMeshFactoryWrapper *csIsoMeshFactoryList::MeshFactoryList::FindByName (const char *Name) const
  { return scfParent->FindByName (Name); }
