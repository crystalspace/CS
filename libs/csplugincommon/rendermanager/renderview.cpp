/*
    Copyright (C)2007 by Marten Svanfeldt
                 2001 by Jorrit Tyberghein
                 2000-2001 by Andrew Zabolotny

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

#include "csgeom/polyclip.h"
#include "csgeom/sphere.h"
#include "csplugincommon/rendermanager/renderview.h"
#include "iengine/camera.h"
#include "igeom/clip2d.h"
#include "iutil/dbghelp.h"
#include "ivideo/graph3d.h"

using namespace CS::RenderManager;

RenderView::RenderView () :
  scfPooledImplementationType (this),
  engine(0),
  g3d(0),
  g2d(0),
  original_camera(0),
  viewWidth (0), viewHeight (0)
{
  ctxt = new csRenderContext ();
  memset (ctxt, 0, sizeof (csRenderContext));
  context_id = 0;
}

RenderView::RenderView (iCamera *c) :
  scfPooledImplementationType (this),
  ctxt (nullptr),
  engine(0),
  g3d(0),
  g2d(0),
  viewWidth (0), viewHeight (0)
{
  InitialiseFromCamera (c);
}

RenderView::RenderView (iCamera *c, iClipper2D *v, iGraphics3D *ig3d) :
  scfPooledImplementationType (this),
  ctxt (nullptr),
  engine(0),
  g3d(ig3d),
  g2d(g3d->GetDriver2D ())
{
  InitialiseFromCamera (c);

  ctxt->iview = v;

  if (g3d)
  {
    viewWidth = g3d->GetWidth(); viewHeight = g3d->GetHeight();
  }
  else
  {
    viewWidth = 0; viewHeight = 0;
  }

  if (v)
  {
    UpdateFrustum ();
  }
}

RenderView::RenderView (iView* v)
: scfPooledImplementationType (this), ctxt (nullptr),
  meshFilter (v->GetMeshFilter ())
{
  InitialiseFromView (v);
}

RenderView::RenderView (const RenderView& other) :
  scfPooledImplementationType (this), viewWidth (other.viewWidth),
  viewHeight (other.viewHeight)
{
  ctxt = new csRenderContext ();
  memset (ctxt, 0, sizeof (csRenderContext));

  ctxt->icamera.AttachNew (other.ctxt->icamera->Clone ());
  original_camera = ctxt->icamera;
  ctxt->iview = other.ctxt->iview;	// @@@ Is this right?
  memcpy (ctxt->frustum, other.ctxt->frustum, sizeof (other.ctxt->frustum));
  memcpy (ctxt->clip_planes, other.ctxt->clip_planes,
    sizeof (other.ctxt->clip_planes));
  ctxt->clip_planes_mask = other.ctxt->clip_planes_mask;
  ctxt->last_portal = other.ctxt->last_portal;
  ctxt->previous_sector = other.ctxt->previous_sector;
  ctxt->this_sector = other.ctxt->this_sector;
  ctxt->clip_plane = other.ctxt->clip_plane;
  ctxt->do_clip_plane = other.ctxt->do_clip_plane;
  ctxt->do_clip_frustum = other.ctxt->do_clip_frustum;
  // @@@ fog_info and added_fog_info?

  context_id = 0;
  engine = other.engine;
  g3d = other.g3d;
  g2d = other.g2d;
  original_camera = 0;	// @@@ Right?
  leftx = other.leftx;
  rightx = other.rightx;
  topy = other.topy;
  boty = other.boty;
}

RenderView::RenderView (const RenderView& other, bool keepCamera) :
  scfPooledImplementationType (this), viewWidth (other.viewWidth),
  viewHeight (other.viewHeight)
{
  ctxt = new csRenderContext ();
  memset (ctxt, 0, sizeof (csRenderContext));

  if (keepCamera)
  {
    ctxt->icamera = other.ctxt->icamera;
    original_camera = other.original_camera;
  }
  else
  {
    ctxt->icamera.AttachNew (other.ctxt->icamera->Clone ());
    original_camera = ctxt->icamera;
  }
  ctxt->iview = other.ctxt->iview;	// @@@ Is this right?
  memcpy (ctxt->frustum, other.ctxt->frustum, sizeof (other.ctxt->frustum));
  memcpy (ctxt->clip_planes, other.ctxt->clip_planes,
    sizeof (other.ctxt->clip_planes));
  ctxt->clip_planes_mask = other.ctxt->clip_planes_mask;
  ctxt->last_portal = other.ctxt->last_portal;
  ctxt->previous_sector = other.ctxt->previous_sector;
  ctxt->this_sector = other.ctxt->this_sector;
  ctxt->clip_plane = other.ctxt->clip_plane;
  ctxt->do_clip_plane = other.ctxt->do_clip_plane;
  ctxt->do_clip_frustum = other.ctxt->do_clip_frustum;
  // @@@ fog_info and added_fog_info?

  context_id = 0;
  engine = other.engine;
  g3d = other.g3d;
  g2d = other.g2d;
  original_camera = 0;	// @@@ Right?
  leftx = other.leftx;
  rightx = other.rightx;
  topy = other.topy;
  boty = other.boty;
}

RenderView::~RenderView ()
{
  delete ctxt;
}

void RenderView::InitialiseFromView (iView* v)
{
  engine = v->GetEngine ();
  g3d = v->GetContext ();
  g2d = g3d->GetDriver2D ();
  original_camera = v->GetCamera ();

  viewWidth = v->GetWidth();
  viewHeight = v->GetHeight();

  iClipper2D* clipper = v->GetClipper ();

  if (ctxt) delete ctxt;
  ctxt = new csRenderContext ();
  memset (ctxt, 0, sizeof (csRenderContext));
  ctxt->icamera = v->GetCamera ();
  ctxt->this_sector = v->GetCamera ()->GetSector ();
  ctxt->iview = clipper;
  if (clipper)
  {
    UpdateFrustum ();
  }

  context_id = 0;
}

void RenderView::InitialiseFromCamera (iCamera* camera)
{
  original_camera = camera;
  if (ctxt) delete ctxt;
  ctxt = new csRenderContext ();
  memset (ctxt, 0, sizeof (csRenderContext));
  ctxt->icamera = camera;
  context_id = 0;
}

void RenderView::SetCamera (iCamera *icam)
{
  ctxt->icamera = icam;
}

void RenderView::SetOriginalCamera (iCamera *icam)
{
  original_camera = icam;
}

void RenderView::SetClipper (iClipper2D *view)
{
  ctxt->iview = view;
  UpdateFrustum ();
}

void RenderView::SetEngine (iEngine *engine)
{
  RenderView::engine = engine;
}

iEngine* RenderView::GetEngine ()
{
  return engine;
}

void RenderView::UpdateFrustum ()
{
  size_t i;
  csBox2 bbox;
  iClipper2D* clip = ctxt->iview;
  csVector2 *poly = clip->GetClipPoly ();
  bbox.StartBoundingBox (poly[0]);
  for (i = 1; i < clip->GetVertexCount (); i++)
    bbox.AddBoundingVertexSmart (poly[i]);

  SetFrustumFromBox (bbox);
}

void RenderView::SetFrustumFromBox (const csBox2& box)
{
  float iw = 2.0f/viewWidth;
  float ih = 2.0f/viewHeight;
  float lx_n = csClamp (box.MinX() * iw - 1.0f, 1.0f, -1.0f);
  float rx_n = csClamp (box.MaxX() * iw - 1.0f, 1.0f, -1.0f);
  float ty_n = csClamp (box.MinY() * ih - 1.0f, 1.0f, -1.0f);
  float by_n = csClamp (box.MaxY() * ih - 1.0f, 1.0f, -1.0f);
 
  CS::Math::Matrix4 invMatrix_inv_t =
    ctxt->icamera->GetProjectionMatrix().GetTranspose();

  int n = 0;
  csPlane3 *frustum = ctxt->frustum;
  csPlane3 p;
  // Back plane
  p.Set (0, 0, -1, 1);
  frustum[n] = invMatrix_inv_t * p;
  frustum[n].Normalize();

  n++;
  // Far plane
  /*p.Set (0, 0, -1, 1);
  clipPlanes[n] = invMatrix_inv_t * p;
  clipPlanes[n].Normalize();
  n++;*/
  // Left plane
  p.Set (1, 0, 0, -lx_n);
  frustum[n] = invMatrix_inv_t * p;
  frustum[n].Normalize();
  n++;
  // Right plane
  p.Set (-1, 0, 0, rx_n);
  frustum[n] = invMatrix_inv_t * p;
  frustum[n].Normalize();
  n++;
  // Bottom plane
  p.Set (0, -1, 0, by_n);
  frustum[n] = invMatrix_inv_t * p;
  frustum[n].Normalize();
  n++;
  // Top plane
  p.Set (0, 1, 0, -ty_n);
  frustum[n] = invMatrix_inv_t * p;
  frustum[n].Normalize();
  n++;
}

void RenderView::SetFrustum (float lx, float rx, float ty, float by)
{
  leftx = lx;
  rightx = rx;
  topy = ty;
  boty = by;
 
  //SetFrustumFromBox (csBox2 (lx, ty, rx, by));
}

void RenderView::CreateRenderContext ()
{
  csRenderContext* old_ctxt = ctxt;

  // @@@ Use a pool for render contexts?
  // A pool would work very well here since the number of render contexts
  // is limited by recursion depth.
  ctxt = new csRenderContext ();
  *ctxt = *old_ctxt;
  ctxt->previous = old_ctxt;
  // The camera transform id is copied from the old
  // context. Only when we do space warping on the camera
  // do we have to change it (CreateNewCamera() function).
  context_id++;
  ctxt->context_id = context_id;
}

void RenderView::RestoreRenderContext ()
{
  csRenderContext* old_ctxt = ctxt;
  ctxt = ctxt->previous;

  delete old_ctxt;
}

iCamera *RenderView::CreateNewCamera ()
{
  // A pool for cameras?
  csRef<iCamera> newcam;
  newcam.AttachNew (ctxt->icamera->Clone ());
  ctxt->icamera = newcam;
  return ctxt->icamera;
}

uint RenderView::GetCurrentFrameNumber () const
{
  return engine->GetCurrentFrameNumber ();
}


void RenderView::DestroyRenderContext (csRenderContext* context)
{
  if (context == ctxt)
  {
    ctxt = context->previous;
  }
  else
  {
    // Its somewhere in the middle, scan starting from ctxt
    csRenderContext* localctxt = ctxt;
    while (localctxt)
    {
      if (localctxt->previous == context)
      {
        localctxt->previous = context->previous;
        break;
      }
      localctxt = localctxt->previous;
    }
  }

  delete context;
}

void RenderView::SetMeshFilter (const CS::Utility::MeshFilter& filter)
{
  // NB: If that assignment becomes a problem COW-wrap meshFilter.
  meshFilter = filter;
}

RenderView* RenderViewCache::GetRenderView (iView* view)
{
  csRef<RenderView> rview;
  for (size_t i = 0; i < iView2RenderViews.GetSize (); ++i)
  {
    if (!iView2RenderViews[i]->view.IsValid ())
    {
      iView2RenderViews.DeleteIndex (i);
      continue;
    }

    if (iView2RenderViews[i]->view == view)
    {
      rview = iView2RenderViews[i]->rview;
    }
  }

  if (!rview.IsValid ())
  {
#include "csutil/custom_new_disable.h"
    rview.AttachNew (new (renderViewPool) RenderView(view));
#include "csutil/custom_new_enable.h"

    csRef<View2RenderView> psTemp;
    psTemp.AttachNew (new View2RenderView (view, rview));
    iView2RenderViews.Push (psTemp);
  }
  else
  {
    // Re-construct the RenderView in the existing memory.
    rview->InitialiseFromView (view);
  }

  return rview;
}

RenderView* RenderViewCache::GetRenderView (RenderView* view, iPortal* portal, iCamera* camera)
{
  csRef<RenderView> rview;
  for (size_t i = 0; i < rViewPortal2RenderViews.GetSize (); ++i)
  {
    if (!rViewPortal2RenderViews[i]->view.IsValid () ||
        !rViewPortal2RenderViews[i]->portal.IsValid ())
    {
      rViewPortal2RenderViews.DeleteIndex (i);
      continue;
    }

    if (rViewPortal2RenderViews[i]->view == view &&
        rViewPortal2RenderViews[i]->portal == portal)
    {
      rview = rViewPortal2RenderViews[i]->rview;
      break;
    }
  }

  if (!rview.IsValid ())
  {
#include "csutil/custom_new_disable.h"
    rview.AttachNew (new (renderViewPool) RenderView(camera, 0, view->GetGraphics3D ()));
#include "csutil/custom_new_enable.h"

    csRef<RViewPortal2RenderView> psTemp;
    psTemp.AttachNew (new RViewPortal2RenderView (view, portal, rview));
    rViewPortal2RenderViews.Push (psTemp);
  }
  else
  {
    // Re-construct the RenderView in the existing memory.
    rview->InitialiseFromCamera (camera);
  }

  return rview;
}

csPtr<RenderView> RenderViewCache::CreateRenderView ()
{
  csRef<RenderView> rview;
#include "csutil/custom_new_disable.h"
  rview.AttachNew (new (renderViewPool) RenderView ());
#include "csutil/custom_new_enable.h"
  return csPtr<RenderView> (rview);
}

csPtr<RenderView> RenderViewCache::CreateRenderView (RenderView* view)
{
  csRef<RenderView> rview;
#include "csutil/custom_new_disable.h"
  rview.AttachNew (new (renderViewPool) RenderView (*view));
#include "csutil/custom_new_enable.h"
  return csPtr<RenderView> (rview);
}

csPtr<RenderView> RenderViewCache::CreateRenderView (RenderView* view, bool keepCamera)
{
  csRef<RenderView> rview;
#include "csutil/custom_new_disable.h"
  rview.AttachNew (new (renderViewPool) RenderView (*view, keepCamera));
#include "csutil/custom_new_enable.h"
  return csPtr<RenderView> (rview);
}
