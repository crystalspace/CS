/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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
#include "csengine/portal.h"
#include "csengine/polygon.h"
#include "csengine/pol2d.h"
#include "csengine/sector.h"
#include "csengine/engine.h"
#include "csengine/tranman.h"
#include "csengine/stats.h"
#include "csengine/cbuffer.h"
#include "csengine/quadtr3d.h"
#include "csengine/covtree.h"
#include "itexture.h"
#include "irview.h"

IMPLEMENT_IBASE (csPortal)
  IMPLEMENTS_INTERFACE (iPortal)
IMPLEMENT_IBASE_END

csPortal::csPortal ()
{
  CONSTRUCT_IBASE (NULL);
  filter_texture = NULL;
  filter_r = 1;
  filter_g = 1;
  filter_b = 1;
  sector = NULL;
}

void csPortal::ObjectToWorld (const csReversibleTransform& t)
{
  if (flags.Check (CS_PORTAL_STATICDEST))
    warp_wor = warp_obj * t;
  else
    warp_wor = t.GetInverse () * warp_obj * t;
}

void csPortal::HardTransform (const csReversibleTransform& t)
{
  ObjectToWorld (t);
  warp_obj = warp_wor;
}

void csPortal::SetWarp (const csTransform& t)
{
  flags.Set (CS_PORTAL_WARP);
  warp_obj = t;
  csMatrix3 m = warp_obj.GetO2T ();
  flags.SetBool (CS_PORTAL_MIRROR, ( ( ( m.Col1() % m.Col2() ) * m.Col3() ) < 0 ));

  warp_wor = warp_obj;
}

void csPortal::SetWarp (const csMatrix3& m_w, const csVector3& v_w_before, const csVector3& v_w_after)
{
  flags.Set (CS_PORTAL_WARP);

  warp_obj = csTransform (m_w.GetInverse (), v_w_after - m_w * v_w_before);

  // If the three colunms of the transformation matrix are taken
  // as vectors, V1, V2, and V3, then V1 x V2 = ( + or - ) V3.
  // The result is positive for non-mirroring transforms, and
  // negative for mirroring transforms.  Thus, (V1 x V2) * V3 
  // will equal +1 or -1, depending on whether the transform is
  // mirroring.
  csMatrix3 m = warp_obj.GetO2T ();
  flags.SetBool (CS_PORTAL_MIRROR, ( ( ( m.Col1() % m.Col2() ) * m.Col3() ) < 0 ));

  warp_wor = warp_obj;
}

void csPortal::WarpSpace (csReversibleTransform& t, bool& mirror)
{
  // warp_wor is a world -> warp space transformation.
  // t is a world -> camera space transformation.
  // Set t to equal a warp -> camera space transformation by
  // reversing warp and then applying the old t.
  t /= warp_wor;
  if (flags.Check (CS_PORTAL_MIRROR)) mirror = !mirror;
}

/// Calculate inverse perspective corrected point for this camera.
static void InvPerspective (const csVector2& p, float z, csVector3& v,
	float inv_aspect, float shift_x, float shift_y)
{
  v.z = z;
  v.x = (p.x - shift_x) * z * inv_aspect;
  v.y = (p.y - shift_y) * z * inv_aspect;
}

bool csPortal::Draw (csPolygon2D* new_clipper, csPolygon3D* portal_polygon,
    	iRenderView* rview)
{
  if (!sector) CompleteSector ();
  iCamera* icam = rview->GetCamera ();
  const csReversibleTransform& camtrans = icam->GetTransform ();
  float inv_aspect = icam->GetInvFOV ();
  float shift_x = icam->GetShiftX ();
  float shift_y = icam->GetShiftY ();

  // Initialize the 2D/3D culler. We only traverse through portals
  // after the culler has been used in the previous sector so this is
  // safe to do here.
  if (csEngine::current_engine->GetEngineMode () == CS_ENGINE_FRONT2BACK)
  {
    csCBuffer* c_buffer = csEngine::current_engine->GetCBuffer ();
    csCoverageMaskTree* covtree = csEngine::current_engine->GetCovtree ();
    csQuadTree3D* quad3d = csEngine::current_engine->GetQuad3D ();
    if (c_buffer)
    {
      c_buffer->Initialize ();
      c_buffer->InsertPolygon (new_clipper->GetVertices (), new_clipper->GetNumVertices (), true);
    }
    else if (quad3d)
    {
      csVector3 corners[4];
      InvPerspective (csVector2 (0, 0), 1, corners[0],
      	inv_aspect, shift_x, shift_y);
      corners[0] = camtrans.This2Other (corners[0]);
      InvPerspective (csVector2 (csEngine::current_engine->frame_width-1, 0), 1, corners[1],
      	inv_aspect, shift_x, shift_y);
      corners[1] = camtrans.This2Other (corners[1]);
      InvPerspective (csVector2 (csEngine::current_engine->frame_width-1,
      	csEngine::current_engine->frame_height-1), 1, corners[2], inv_aspect, shift_x, shift_y);
      corners[2] = camtrans.This2Other (corners[2]);
      InvPerspective (csVector2 (0, csEngine::current_engine->frame_height-1), 1, corners[3],
      	inv_aspect, shift_x, shift_y);
      corners[3] = camtrans.This2Other (corners[3]);
      quad3d->SetMainFrustum (camtrans.GetOrigin (), corners);
      quad3d->MakeEmpty ();
    }
    else if (covtree)
    {
      covtree->MakeEmpty ();
      covtree->UpdatePolygonInverted (new_clipper->GetVertices (), new_clipper->GetNumVertices ());
    }
  }

  if (sector->draw_busy >= 5)
    return false;

  Stats::portals_drawn++;
  if (!new_clipper->GetNumVertices ())
    return false;

  csPolygonClipper new_view (new_clipper, icam->IsMirrored (), true);

  //@@@ THIS SHOULD HAPPEN DIFFERENTLY.
  csRenderView new_rview = *(rview->GetPrivateObject ());
  new_rview.SetView (&new_view);
  new_rview.ResetFogInfo ();
  new_rview.SetPortalPolygon (portal_polygon);
  new_rview.SetPreviousSector (rview->GetPrivateObject ()->GetThisSector ());
  new_rview.SetClipPlane (portal_polygon->GetPlane ()->GetCameraPlane());
  new_rview.GetClipPlane ().Invert ();
  if (flags.Check (CS_PORTAL_CLIPDEST)) new_rview.UseClipPlane (true);

  csTranCookie old_cookie = 0;
  if (flags.Check (CS_PORTAL_WARP))
  {
    bool mirror = new_rview.IsMirrored ();
    WarpSpace (new_rview, mirror);
    new_rview.SetMirrored (mirror);
    old_cookie = csEngine::current_engine->tr_manager.NewCameraFrame ();
  }

  sector->Draw (new_rview);

  if (flags.Check (CS_PORTAL_WARP))
    csEngine::current_engine->tr_manager.RestoreCameraFrame (old_cookie);

  return true;
}

csPolygon3D* csPortal::HitBeam (const csVector3& start, const csVector3& end,
	csVector3& isect)
{
  if (!sector) CompleteSector ();

  if (sector->draw_busy >= 5)
    return NULL;
  if (flags.Check (CS_PORTAL_WARP))
  {
    csVector3 new_start = warp_wor.Other2This (start);
    csVector3 new_end = warp_wor.Other2This (end);
    csVector3 new_isect;
    csPolygon3D* p = sector->HitBeam (new_start, new_end, new_isect);
    if (p)
      isect = warp_wor.This2Other (new_isect);
    return p;
  }
  else return sector->HitBeam (start, end, isect);
}

csObject* csPortal::HitBeam (const csVector3& start, const csVector3& end,
	csPolygon3D** polygonPtr)
{
  if (!sector) CompleteSector ();

  if (sector->draw_busy >= 5)
    return NULL;
  if (flags.Check (CS_PORTAL_WARP))
  {
    csVector3 new_start = warp_wor.Other2This (start);
    csVector3 new_end = warp_wor.Other2This (end);
    csObject* o = sector->HitBeam (new_start, new_end, polygonPtr);
    return o;
  }
  else return sector->HitBeam (start, end, polygonPtr);
}

void csPortal::CheckFrustum (csFrustumView& lview, int alpha)
{
  if (!sector) CompleteSector ();
  if (sector->draw_busy > csSector::cfg_reflections) return;

  csFrustumView new_lview = lview;
  if (lview.light_frustum)
    new_lview.light_frustum = new csFrustum (*lview.light_frustum);

  // If copied_frustums is true we copied the frustums and we need to delete them
  // later.
  bool copied_frustums = false;

  csTranCookie old_cookie = 0;
  if (flags.Check (CS_PORTAL_WARP))
  {
    old_cookie = csEngine::current_engine->tr_manager.NewCameraFrame ();
    new_lview.light_frustum->Transform (&warp_wor);

    if (flags.Check (CS_PORTAL_MIRROR)) new_lview.mirror = !lview.mirror;
    new_lview.light_frustum->SetMirrored (new_lview.mirror);

    // Transform all shadow frustums. First make a copy.
    // Note that we only copy the relevant shadow frustums.
    // We know that csPolygon3D::CalculateLighting() called
    // csPolygon3D::MarkRelevantShadowFrustums() some time before
    // calling this function so the 'relevant' flags are still valid.
    new_lview.shadows.Clear ();	// Don't delete elements.
    csShadowFrustum* sf, * copy_sf;
    sf = lview.shadows.GetFirst ();
    while (sf)
    {
      if (sf->relevant)
      {
        copy_sf = new csShadowFrustum (*sf);
        new_lview.shadows.AddLast (copy_sf);
      }
      sf = sf->next;
    }
    copied_frustums = true;
    new_lview.shadows.Transform (&warp_wor);

    if (alpha)
    {
      float fr, fg, fb;
      if (filter_texture)
      {
        UByte mr, mg, mb;
        filter_texture->GetMeanColor (mr, mg, mb);
        fr = mr / 255.; fg = mg / 255.; fb = mb / 255.;
      }
      else
      {
        fr = filter_r;
        fg = filter_g;
        fb = filter_b;
      }
      new_lview.r = lview.r * fr;
      new_lview.g = lview.g * fg;
      new_lview.b = lview.b * fb;
    }

    // Don't go further if the light intensity is almost zero.
    if (new_lview.r < SMALL_EPSILON && new_lview.g < SMALL_EPSILON && new_lview.b < SMALL_EPSILON)
      return;
  }
  else if (lview.shadows.GetFirst ())
  {
    // There is no space warping. In this case we still want to
    // remove all non-relevant shadow frustums if there are any.
    // We know that csPolygon3D::CalculateLighting() called
    // csPolygon3D::MarkRelevantShadowFrustums() some time before
    // calling this function so the 'relevant' flags are still valid.
    new_lview.shadows.Clear ();	// Don't delete elements.
    csShadowFrustum* sf, * copy_sf;
    sf = lview.shadows.GetFirst ();
    while (sf)
    {
      if (sf->relevant)
      {
        copy_sf = new csShadowFrustum (*sf);
        new_lview.shadows.AddLast (copy_sf);
      }
      sf = sf->next;
    }
    copied_frustums = true;
  }

  sector->RealCheckFrustum (new_lview);

  if (flags.Check (CS_PORTAL_WARP))
    csEngine::current_engine->tr_manager.RestoreCameraFrame (old_cookie);

  if (copied_frustums)
  {
    // Delete all copied frustums.
    new_lview.shadows.DeleteFrustums ();
    new_lview.shadows.Clear ();
  }
}

iSector *csPortal::GetPortal ()
{
  return &GetSector ()->scfiSector;
}

void csPortal::SetPortal (iSector *iDest)
{
  SetSector (iDest->GetPrivateObject ());
}

void csPortal::SetMirror (iPolygon3D *iPoly)
{
  csPolygon3D *poly = iPoly->GetPrivateObject ();
  SetWarp (csTransform::GetReflect (*(poly->GetPolyPlane ())));
}
