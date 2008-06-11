/*
    Copyright (C) 2001 by Jorrit Tyberghein
    Copyright (C) 2000-2001 by Andrew Zabolotny

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
#include "plugins/engine/3d/rview.h"
#include "plugins/engine/3d/engine.h"
#include "csgeom/sphere.h"
#include "igeom/clip2d.h"
#include "iengine/camera.h"
#include "plugins/engine/3d/sector.h"
#include "csgeom/polyclip.h"
#include "iutil/dbghelp.h"
#include "ivideo/graph3d.h"


csRenderView::csRenderView () :
  scfPooledImplementationType (this),
  engine(0),
  g3d(0),
  g2d(0),
  original_camera(0)
{
  ctxt = new csRenderContext ();
  memset (ctxt, 0, sizeof (csRenderContext));
  context_id = 0;
}

csRenderView::csRenderView (iCamera *c) :
  scfPooledImplementationType (this),
  engine(0),
  g3d(0),
  g2d(0),
  original_camera(0)
{
  ctxt = new csRenderContext ();
  memset (ctxt, 0, sizeof (csRenderContext));
  ctxt->icamera = c;
  context_id = 0;
}

csRenderView::csRenderView (
  iCamera *c, iClipper2D *v, iGraphics3D *ig3d, iGraphics2D *ig2d) :
  scfPooledImplementationType (this),
    engine(0),
    g3d(ig3d),
    g2d(ig2d),
    original_camera(0)
{
  ctxt = new csRenderContext ();
  memset (ctxt, 0, sizeof (csRenderContext));
  ctxt->icamera = c;
  ctxt->iview = v;
  if (v)
  {
    UpdateFrustum ();
  }

  context_id = 0;
}

csRenderView::csRenderView (const csRenderView& other) :
  scfPooledImplementationType (this)
{
  ctxt = new csRenderContext ();
  memset (ctxt, 0, sizeof (csRenderContext));

  ctxt->icamera.AttachNew (other.ctxt->icamera->Clone ());
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

csRenderView::~csRenderView ()
{
  delete ctxt;
}

void csRenderView::SetCamera (iCamera *icam)
{
  ctxt->icamera = icam;
}

void csRenderView::SetOriginalCamera (iCamera *icam)
{
  original_camera = icam;
}

void csRenderView::SetClipper (iClipper2D *view)
{
  ctxt->iview = view;
  UpdateFrustum ();
}

void csRenderView::SetEngine (csEngine *engine)
{
  csRenderView::engine = engine;
}

iEngine* csRenderView::GetEngine ()
{
  return (iEngine*)(csEngine*)engine;
}

void csRenderView::UpdateFrustum ()
{
  size_t i;
  csBox2 bbox;
  ctxt->icamera->SetViewportSize (g3d->GetWidth(), g3d->GetHeight());
  csVector2 shift (ctxt->icamera->GetShiftX (), ctxt->icamera->GetShiftY ());
  float inv_fov = ctxt->icamera->GetInvFOV ();
  iClipper2D* clip = ctxt->iview;
  csVector2 *poly = clip->GetClipPoly ();
  bbox.StartBoundingBox ((poly[0] - shift) * inv_fov);
  for (i = 1; i < clip->GetVertexCount (); i++)
    bbox.AddBoundingVertexSmart ((poly[i] - shift) * inv_fov);

  csPlane3 *frustum = ctxt->frustum;
  csVector3 v1 (bbox.MinX (), bbox.MinY (), 1);
  csVector3 v2 (bbox.MaxX (), bbox.MinY (), 1);
  frustum[0].Set (v1 % v2, 0);
  frustum[0].norm.Normalize ();

  csVector3 v3 (bbox.MaxX (), bbox.MaxY (), 1);
  frustum[1].Set (v2 % v3, 0);
  frustum[1].norm.Normalize ();
  v2.Set (bbox.MinX (), bbox.MaxY (), 1);
  frustum[2].Set (v3 % v2, 0);
  frustum[2].norm.Normalize ();
  frustum[3].Set (v2 % v1, 0);
  frustum[3].norm.Normalize ();
}

void csRenderView::SetFrustum (float lx, float rx, float ty, float by)
{
  leftx = lx;
  rightx = rx;
  topy = ty;
  boty = by;

  csPlane3 *frustum = ctxt->frustum;
  csVector3 v1 (leftx, topy, 1);
  csVector3 v2 (rightx, topy, 1);
  frustum[0].Set (v1 % v2, 0);
  frustum[0].norm.Normalize ();

  csVector3 v3 (rightx, boty, 1);
  frustum[1].Set (v2 % v3, 0);
  frustum[1].norm.Normalize ();
  v2.Set (leftx, boty, 1);
  frustum[2].Set (v3 % v2, 0);
  frustum[2].norm.Normalize ();
  frustum[3].Set (v2 % v1, 0);
  frustum[3].norm.Normalize ();
}

void csRenderView::CreateRenderContext ()
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

void csRenderView::RestoreRenderContext ()
{
  csRenderContext* old_ctxt = ctxt;
  ctxt = ctxt->previous;

  delete old_ctxt;
}

iCamera *csRenderView::CreateNewCamera ()
{
  // A pool for cameras?
  csRef<iCamera> newcam = ctxt->icamera->Clone ();
  ctxt->icamera = newcam;
  return ctxt->icamera;
}

uint csRenderView::GetCurrentFrameNumber () const
{
  return engine->GetCurrentFrameNumber ();
}

void csRenderView::DestroyRenderContext (csRenderContext* context)
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
