/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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
#include "csengine/stats.h"
#include "csengine/cbuffer.h"
#include "csengine/lview.h"
#include "csengine/light.h"
#include "csutil/debug.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "iengine/rview.h"
#include "iengine/fview.h"

SCF_IMPLEMENT_IBASE_EXT(csPortal)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iPortal)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csPortal::Portal)
  SCF_IMPLEMENTS_INTERFACE(iPortal)
  SCF_IMPLEMENTS_INTERFACE(iReference)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csPortal::csPortal ()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPortal);
  DG_TYPE (this, "csPortal");
  filter_texture = NULL;
  filter_r = 1;
  filter_g = 1;
  filter_b = 1;
  sector = NULL;
  sector_cb = NULL;
  portal_cb = NULL;
}

csPortal::~csPortal ()
{
  // Before destruction the destination of the portal needs to be

  // set to NULL.
  CS_ASSERT (sector == NULL);
  if (filter_texture) filter_texture->DecRef ();
  if (sector_cb) sector_cb->DecRef ();
  if (portal_cb) portal_cb->DecRef ();
}

iReferencedObject *csPortal::GetReferencedObject () const
{
  if (!sector) return NULL;

  iReferencedObject *refobj = SCF_QUERY_INTERFACE_FAST (
      sector,
      iReferencedObject);
  refobj->DecRef ();
  return refobj;
}

void csPortal::SetReferencedObject (iReferencedObject *b)
{
  if (b == NULL)
  {
    SetSector (NULL);
  }
  else
  {
    iSector *s = SCF_QUERY_INTERFACE_FAST (b, iSector);
    CS_ASSERT (s != NULL);
    SetSector (s);
    s->DecRef ();
  }
}

iSector *csPortal::GetSector () const
{
  return sector;
}

void csPortal::SetSector (iSector *s)
{
  if (sector != s)
  {
    if (sector)
    {
      // First unlink from the previous sector.
      iReferencedObject *refobj = SCF_QUERY_INTERFACE_FAST (
          sector,
          iReferencedObject);
      CS_ASSERT (refobj != NULL);
      refobj->RemoveReference (&scfiPortal);
      refobj->DecRef ();
    }

    sector = s;
    if (sector)
    {
      // Link to the new sector.
      iReferencedObject *refobj = SCF_QUERY_INTERFACE_FAST (
          sector,
          iReferencedObject);
      CS_ASSERT (refobj != NULL);
      refobj->AddReference (&scfiPortal);
      refobj->DecRef ();
    }
  }
}

csFlags &csPortal::GetFlags ()
{
  return flags;
}

bool csPortal::CompleteSector (iBase *context)
{
  if (sector)
  {
    if (portal_cb)
      return portal_cb->Traverse (&(this->scfiPortal), context);
    else
      return true;
  }
  else if (sector_cb)
  {
    return sector_cb->Traverse (&(this->scfiPortal), context);
  }

  return false;
}

void csPortal::ObjectToWorld (const csReversibleTransform &t)
{
  if (flags.Check (CS_PORTAL_STATICDEST))
    warp_wor = warp_obj * t;
  else
    warp_wor = t.GetInverse () * warp_obj * t;
}

void csPortal::HardTransform (const csReversibleTransform &t)
{
  ObjectToWorld (t);
  warp_obj = warp_wor;
}

void csPortal::SetWarp (const csTransform &t)
{
  flags.Set (CS_PORTAL_WARP);
  warp_obj = t;

  csMatrix3 m = warp_obj.GetO2T ();
  flags.SetBool (
      CS_PORTAL_MIRROR,
      (((m.Col1 () % m.Col2 ()) * m.Col3 ()) < 0));

  warp_wor = warp_obj;
}

void csPortal::SetWarp (
  const csMatrix3 &m_w,
  const csVector3 &v_w_before,
  const csVector3 &v_w_after)
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
  flags.SetBool (
      CS_PORTAL_MIRROR,
      (((m.Col1 () % m.Col2 ()) * m.Col3 ()) < 0));

  warp_wor = warp_obj;
}

const csReversibleTransform &csPortal::GetWarp () const
{
  return warp_obj;
}

void csPortal::SetFilter (iTextureHandle *ft)
{
  if (ft) ft->IncRef ();
  if (filter_texture) filter_texture->DecRef ();
  filter_texture = ft;
}

iTextureHandle *csPortal::GetTextureFilter () const
{
  return filter_texture;
}

void csPortal::SetFilter (float r, float g, float b)
{
  filter_r = r;
  filter_g = g;
  filter_b = b;
  if (filter_texture)
  {
    filter_texture->DecRef ();
    filter_texture = NULL;
  }
}

void csPortal::GetColorFilter (float &r, float &g, float &b) const
{
  r = filter_r;
  g = filter_g;
  b = filter_b;
}

csVector3 csPortal::Warp (const csVector3 &pos) const
{
  return warp_wor.Other2This (pos);
}

void csPortal::WarpSpace (csReversibleTransform &t, bool &mirror) const
{
  // warp_wor is a world -> warp space transformation.

  // t is a world -> camera space transformation.

  // Set t to equal a warp -> camera space transformation by

  // reversing warp and then applying the old t.
  t /= warp_wor;
  if (flags.Check (CS_PORTAL_MIRROR)) mirror = !mirror;
}

bool csPortal::Draw (
  csPolygon2D *new_clipper,
  csPolygon3D *portal_polygon,
  iRenderView *rview)
{
  if (!CompleteSector (rview)) return false;
  if (sector->GetPrivateObject ()->draw_busy >= 5) return false;

  Stats::portals_drawn++;
  if (!new_clipper->GetVertexCount ()) return false;

  iCamera *icam = rview->GetCamera ();
  csPolygonClipper new_view (new_clipper, icam->IsMirrored (), true);

  csRenderContext *old_ctxt = rview->GetRenderContext ();
  rview->CreateRenderContext ();
  rview->SetRenderRecursionLevel (rview->GetRenderRecursionLevel () + 1);
  rview->SetClipper (&new_view);
  rview->ResetFogInfo ();
  rview->SetPortalPolygon (&portal_polygon->scfiPolygon3D);
  rview->SetPreviousSector (rview->GetThisSector ());
  rview->SetClipPlane (portal_polygon->GetPlane ()->GetCameraPlane ());
  rview->GetClipPlane ().Invert ();
  if (flags.Check (CS_PORTAL_CLIPDEST))
  {
    rview->UseClipPlane (true);
    rview->UseClipFrustum (true);
  }

  // When going through a portal we first remember the old clipper

  // and clip plane (if any). Then we set a new one. Later we restore.
  iGraphics3D *G3D = rview->GetGraphics3D ();
  iClipper2D *old_clipper = G3D->GetClipper ();
  if (old_clipper) old_clipper->IncRef ();

  int old_cliptype = G3D->GetClipType ();
  G3D->SetClipper (
      rview->GetClipper (),
      rview->IsClipperRequired () ? CS_CLIPPER_REQUIRED : CS_CLIPPER_OPTIONAL);

  csPlane3 old_near_plane = G3D->GetNearPlane ();
  bool old_do_near_plane = G3D->HasNearPlane ();
  csPlane3 cp;
  if (rview->GetClipPlane (cp))
    G3D->SetNearPlane (cp);
  else
    G3D->ResetNearPlane ();

  if (flags.Check (CS_PORTAL_WARP))
  {
    iCamera *inewcam = rview->CreateNewCamera ();

    bool mirror = inewcam->IsMirrored ();
    WarpSpace (inewcam->GetTransform (), mirror);
    inewcam->SetMirrored (mirror);

    sector->GetPrivateObject ()->Draw (rview);
  }
  else
    sector->GetPrivateObject ()->Draw (rview);

  rview->RestoreRenderContext (old_ctxt);

  // Now restore our G3D clipper and plane.
  G3D->SetClipper (old_clipper, old_cliptype);
  if (old_clipper) old_clipper->DecRef ();
  if (old_do_near_plane)
    G3D->SetNearPlane (old_near_plane);
  else
    G3D->ResetNearPlane ();

  return true;
}

csPolygon3D *csPortal::HitBeam (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect)
{
  if (!CompleteSector (NULL)) return NULL;
  if (sector->GetPrivateObject ()->draw_busy >= 5) return NULL;
  if (flags.Check (CS_PORTAL_WARP))
  {
    csVector3 new_start = warp_wor.Other2This (start);
    csVector3 new_end = warp_wor.Other2This (end);
    csVector3 new_isect;
    iPolygon3D *p = sector->HitBeam (new_start, new_end, new_isect);
    if (p) isect = warp_wor.This2Other (new_isect);
    return p ? p->GetPrivateObject () : NULL;
  }
  else
  {
    iPolygon3D *p = sector->HitBeam (start, end, isect);
    return p ? p->GetPrivateObject () : NULL;
  }
}

csMeshWrapper *csPortal::HitBeam (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect,
  csPolygon3D **polygonPtr)
{
  if (!CompleteSector (NULL)) return NULL;
  if (sector->GetPrivateObject ()->draw_busy >= 5) return NULL;
  if (flags.Check (CS_PORTAL_WARP))
  {
    csVector3 new_start = warp_wor.Other2This (start);
    csVector3 new_end = warp_wor.Other2This (end);
    csMeshWrapper *o = sector->GetPrivateObject ()->HitBeam (
        new_start,
        new_end,
        isect,
        polygonPtr);
    return o;
  }
  else
    return sector->GetPrivateObject ()->HitBeam (
        start,
        end,
        isect,
        polygonPtr);
}

void csPortal::CheckFrustum (iFrustumView *lview, int alpha)
{
  if (!CompleteSector (lview)) return ;
  if (sector->GetPrivateObject ()->draw_busy > csSector::cfg_reflections)
    return ;

  csFrustumContext *old_ctxt = lview->GetFrustumContext ();
  lview->CreateFrustumContext ();

  csFrustumContext *new_ctxt = lview->GetFrustumContext ();
  if (old_ctxt->GetLightFrustum ())
    new_ctxt->SetLightFrustum (new csFrustum (*old_ctxt->GetLightFrustum ()));
  lview->StartNewShadowBlock ();

  // If copied_frustums is true we copied the frustums and we need to

  // delete them later.
  bool copied_frustums = false;

  // If true then we have to restore color.
  iLightingProcessInfo *linfo = NULL;
  bool restore_color = false;
  csColor old_color;

  if (flags.Check (CS_PORTAL_WARP))
  {
    new_ctxt->GetLightFrustum ()->Transform (&warp_wor);

    if (flags.Check (CS_PORTAL_MIRROR))
      new_ctxt->SetMirrored (!old_ctxt->IsMirrored ());
    new_ctxt->GetLightFrustum ()->SetMirrored (new_ctxt->IsMirrored ());

    // Transform all shadow frustums. First make a copy.

    // Note that we only copy the relevant shadow frustums.

    // We know that csPolygon3D::CalculateLighting() called

    // csPolygon3D::MarkRelevantShadowFrustums() some time before

    // calling this function so the 'relevant' flags are still valid.
    iShadowBlock *slist = old_ctxt->GetShadows ()->GetFirstShadowBlock ();
    while (slist)
    {
      iShadowBlock *copy_slist = new_ctxt->GetShadows ()->NewShadowBlock (
          slist->GetSector (),
          slist->GetRecLevel ());
      copy_slist->AddRelevantShadows (slist, &warp_wor);
      slist = old_ctxt->GetShadows ()->GetNextShadowBlock (slist);
    }

    copied_frustums = true;

    iFrustumViewUserdata *ud = lview->GetUserdata ();
    if (ud) linfo = SCF_QUERY_INTERFACE (ud, iLightingProcessInfo);
    if (linfo)
    {
      linfo->DecRef ();
      if (alpha)
      {
        float fr, fg, fb;
        if (filter_texture)
        {
          uint8 mr, mg, mb;
          filter_texture->GetMeanColor (mr, mg, mb);
          fr = mr / 255.0f;
          fg = mg / 255.0f;
          fb = mb / 255.0f;
        }
        else
        {
          fr = filter_r;
          fg = filter_g;
          fb = filter_b;
        }

        restore_color = true;
        old_color = linfo->GetColor ();
        linfo->SetColor (
            csColor (
              linfo->GetColor ().red * fr,
              linfo->GetColor ().green * fg,
              linfo->GetColor ().blue * fb));
      }

      // Don't go further if the light intensity is almost zero.
      if (
        linfo->GetColor ().red < SMALL_EPSILON &&
        linfo->GetColor ().green < SMALL_EPSILON &&
        linfo->GetColor ().blue < SMALL_EPSILON)
        goto stop;
    }
  }
  else
  {
    // There is no space warping. In this case we still want to

    // remove all non-relevant shadow frustums if there are any.

    // We know that csPolygon3D::CalculateLighting() called

    // csPolygon3D::MarkRelevantShadowFrustums() some time before

    // calling this function so the 'relevant' flags are still valid.
    iShadowBlock *slist = old_ctxt->GetShadows ()->GetFirstShadowBlock ();
    while (slist)
    {
      copied_frustums = true; // Only set to true here
      iShadowBlock *copy_slist = new_ctxt->GetShadows ()->NewShadowBlock (
          slist->GetSector (),
          slist->GetRecLevel ());
      copy_slist->AddRelevantShadows (slist);

      slist = old_ctxt->GetShadows ()->GetNextShadowBlock (slist);
    }
  }

  sector->GetPrivateObject ()->RealCheckFrustum (lview);

  if (copied_frustums)
  {
    // Delete all copied frustums.
    new_ctxt->GetShadows ()->DeleteAllShadows ();
  }

stop:
  lview->RestoreFrustumContext (old_ctxt);
  if (restore_color)
  {
    linfo->SetColor (old_color);
  }
}

void csPortal::SetMirror (iPolygon3D *iPoly)
{
  csPolygon3D *poly = iPoly->GetPrivateObject ();
  SetWarp (csTransform::GetReflect (*(poly->GetPolyPlane ())));
}

void csPortal::SetPortalCallback (iPortalCallback *cb)
{
  if (cb) cb->IncRef ();
  if (portal_cb) portal_cb->DecRef ();
  portal_cb = cb;
}

iPortalCallback *csPortal::GetPortalCallback () const
{
  return portal_cb;
}

void csPortal::SetMissingSectorCallback (iPortalCallback *cb)
{
  if (cb) cb->IncRef ();
  if (sector_cb) sector_cb->DecRef ();
  sector_cb = cb;
}

iPortalCallback *csPortal::GetMissingSectorCallback () const
{
  return sector_cb;
}
