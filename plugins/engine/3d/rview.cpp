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
#include "ivideo/vbufmgr.h"


SCF_IMPLEMENT_IBASE(csRenderView)
  SCF_IMPLEMENTS_INTERFACE(iRenderView)
SCF_IMPLEMENT_IBASE_END

csRenderView::csRenderView () :
  engine(0),
  g3d(0),
  g2d(0),
  original_camera(0)
{
  SCF_CONSTRUCT_IBASE (0);
  ctxt = new csRenderContext ();
  memset (ctxt, 0, sizeof (csRenderContext));
  context_id = 0;
}

csRenderView::csRenderView (iCamera *c) :
  engine(0),
  g3d(0),
  g2d(0),
  original_camera(0)
{
  SCF_CONSTRUCT_IBASE (0);
  ctxt = new csRenderContext ();
  memset (ctxt, 0, sizeof (csRenderContext));
  c->IncRef ();
  ctxt->icamera = c;
  context_id = 0;
}

csRenderView::csRenderView (
  iCamera *c,
  iClipper2D *v,
  iGraphics3D *ig3d,
  iGraphics2D *ig2d) :
    engine(0),
    g3d(ig3d),
    g2d(ig2d),
    original_camera(0)
{
  SCF_CONSTRUCT_IBASE (0);
  ctxt = new csRenderContext ();
  memset (ctxt, 0, sizeof (csRenderContext));
  c->IncRef ();
  ctxt->icamera = c;
  ctxt->iview = v;
  if (v)
  {
    v->IncRef ();
    UpdateFrustum ();
  }

  context_id = 0;
}

csRenderView::~csRenderView ()
{
  if (ctxt)
  {
    if (ctxt->icamera) ctxt->icamera->DecRef ();
    if (ctxt->iview) ctxt->iview->DecRef ();
    delete ctxt;
  }
  SCF_DESTRUCT_IBASE ();
}

void csRenderView::SetCamera (iCamera *icam)
{
  icam->IncRef ();
  if (ctxt->icamera) ctxt->icamera->DecRef ();
  ctxt->icamera = icam;
}

void csRenderView::SetOriginalCamera (iCamera *icam)
{
  original_camera = icam;
}

void csRenderView::SetClipper (iClipper2D *view)
{
  view->IncRef ();
  if (ctxt->iview) ctxt->iview->DecRef ();
  ctxt->iview = view;
  UpdateFrustum ();
}

void csRenderView::SetEngine (csEngine *engine)
{
  csRenderView::engine = engine;
}

// Remove this for old fog

//#define USE_EXP_FOG

// After such number of values fog density coefficient can be considered 0.
#ifdef USE_EXP_FOG
# define FOG_EXP_TABLE_SIZE  1600
static float *fog_exp_table = 0;

static void InitializeFogTable ()
{
  fog_exp_table = new float[FOG_EXP_TABLE_SIZE];

  int i;
  for (i = 0; i < FOG_EXP_TABLE_SIZE; i++)
    fog_exp_table[i] = 1 - exp (-float (i) / 256.);
}
#endif
#define SMALL_D 0.01f
void csRenderView::CalculateFogPolygon (G3DPolygonDP &poly)
{
  if (!ctxt->fog_info || poly.num < 3)
  {
    poly.use_fog = false;
    return ;
  }

  poly.use_fog = true;

#ifdef USE_EXP_FOG
  if (!fog_exp_table) InitializeFogTable ();
#endif

  // Get the plane normal of the polygon. Using this we can calculate

  // '1/z' at every screen space point.
  float inv_aspect = ctxt->icamera->GetInvFOV ();
  float Ac, Bc, Cc, Dc, inv_Dc;
  Ac = poly.normal.A ();
  Bc = poly.normal.B ();
  Cc = poly.normal.C ();
  Dc = poly.normal.D ();

  float M, N, O;
  if (ABS (Dc) < SMALL_D) Dc = -SMALL_D;
  if (ABS (Dc) < SMALL_D)
  {
    // The Dc component of the plane normal is too small. This means that
    // the plane of the polygon is almost perpendicular to the eye of the
    // viewer. In this case, nothing much can be seen of the plane anyway
    // so we just take one value for the entire polygon.
    M = 0;
    N = 0;

    // For O choose the transformed z value of one vertex.
    // That way Z buffering should at least work.
    O = 1 / poly.z_value;
  }
  else
  {
    inv_Dc = 1 / Dc;
    M = -Ac * inv_Dc * inv_aspect;
    N = -Bc * inv_Dc * inv_aspect;
    O = -Cc * inv_Dc;
  }

  float shift_x = ctxt->icamera->GetShiftX ();
  float shift_y = ctxt->icamera->GetShiftY ();
  int i;
  for (i = 0; i < poly.num; i++)
  {
    // Calculate the original 3D coordinate again (camera space).
    csVector3 v;
    v.z = 1.0f /
      (
        M * (poly.vertices[i].x - shift_x) +
        N * (poly.vertices[i].y - shift_y) +
        O
      );
    v.x = (poly.vertices[i].x - shift_x) * v.z * inv_aspect;
    v.y = (poly.vertices[i].y - shift_y) * v.z * inv_aspect;

    // Initialize fog vertex.
    poly.fog_info[i].r = 0;
    poly.fog_info[i].g = 0;
    poly.fog_info[i].b = 0;
    poly.fog_info[i].intensity = 0;
    poly.fog_info[i].intensity2 = 0;

    // Consider a ray between (0,0,0) and v and calculate the thickness of every
    // fogged sector in between.
    csFogInfo *fi = ctxt->fog_info;
    while (fi)
    {
      float dist1, dist2;
      if (fi->has_incoming_plane)
      {
        const csPlane3 &pl = fi->incoming_plane;
        float denom = pl.norm.x * v.x + pl.norm.y * v.y + pl.norm.z * v.z;

        //dist1 = v.Norm () * (-pl.DD / denom);
        dist1 = v.z * (-pl.DD / denom);
      }
      else
        dist1 = 0;

      const csPlane3 &pl = fi->outgoing_plane;
      float denom = pl.norm.x * v.x + pl.norm.y * v.y + pl.norm.z * v.z;

      //dist2 = v.Norm () * (-pl.DD / denom);
      dist2 = v.z * (-pl.DD / denom);

#ifdef USE_EXP_FOG
      // Implement semi-exponential fog (linearily approximated)
      uint table_index = QRound (
          (100 * ABS (dist2 - dist1)) * fi->fog->density);
      float I2;
      if (table_index < FOG_EXP_TABLE_SIZE)
        I2 = fog_exp_table[table_index];
      else
        I2 = fog_exp_table[FOG_EXP_TABLE_SIZE - 1];
#else
      float I2 = ABS (dist2 - dist1) * fi->fog->density;
#endif
      if (I2 > CS_FOG_MAXVALUE)
        I2 = CS_FOGTABLE_CLAMPVALUE;
      else
        I2 = I2 * CS_FOGTABLE_DISTANCESCALE;

      if (poly.fog_info[i].intensity)
      {
        // We already have a previous fog level. In this case we do some
        // mathematical tricks to combine both fog levels. Substitute one
        // fog expresion in the other. The basic fog expression is:
        //	C = I*F + (1-I)*P
        //	with I = intensity
        //	     F = fog color
        //	     P = polygon color
        //	     C = result
        float I1 = poly.fog_info[i].intensity;
        float I = I1 + I2 - I1 * I2;
        if (I > CS_FOGTABLE_CLAMPVALUE) I = CS_FOGTABLE_CLAMPVALUE;
        poly.fog_info[i].intensity = I;

        float fact = 1.0f / I;
        poly.fog_info[i].r =
          (
            I2 * fi->fog->red +
            I1 * poly.fog_info[i].r +
            I1 * I2 * poly.fog_info[i].r
          ) * fact;
        poly.fog_info[i].g =
          (
            I2 * fi->fog->green +
            I1 * poly.fog_info[i].g +
            I1 * I2 * poly.fog_info[i].g
          ) * fact;
        poly.fog_info[i].b =
          (
            I2 * fi->fog->blue +
            I1 * poly.fog_info[i].b +
            I1 * I2 * poly.fog_info[i].b
          ) * fact;
      }
      else
      {
        // The first fog level.
        poly.fog_info[i].intensity = I2;
        poly.fog_info[i].r = fi->fog->red;
        poly.fog_info[i].g = fi->fog->green;
        poly.fog_info[i].b = fi->fog->blue;
      }

      fi = fi->next;
    }
  }
}

// @@@ We should be able to avoid having the need for two functions
// which are almost exactly the same.
void csRenderView::CalculateFogPolygon (G3DPolygonDPFX &poly)
{
  if (!ctxt->fog_info || poly.num < 3)
  {
    poly.use_fog = false;
    return ;
  }

  poly.use_fog = true;

#ifdef USE_EXP_FOG
  if (!fog_exp_table) InitializeFogTable ();
#endif

  float shift_x = ctxt->icamera->GetShiftX ();
  float shift_y = ctxt->icamera->GetShiftY ();
  float inv_aspect = ctxt->icamera->GetInvFOV ();

  int i;
  for (i = 0; i < poly.num; i++)
  {
    // Calculate the original 3D coordinate again (camera space).
    csVector3 v;
    v.z = 1.0f / poly.z[i];
    v.x = (poly.vertices[i].x - shift_x) * v.z * inv_aspect;
    v.y = (poly.vertices[i].y - shift_y) * v.z * inv_aspect;

    // Initialize fog vertex.
    poly.fog_info[i].r = 0;
    poly.fog_info[i].g = 0;
    poly.fog_info[i].b = 0;
    poly.fog_info[i].intensity = 0;
    poly.fog_info[i].intensity2 = 0;

    // Consider a ray between (0,0,0) and v and calculate the thickness of every

    // fogged sector in between.
    csFogInfo *fi = ctxt->fog_info;
    while (fi)
    {
      float dist1, dist2;
      if (fi->has_incoming_plane)
      {
        const csPlane3 &pl = fi->incoming_plane;
        float denom = pl.norm.x * v.x + pl.norm.y * v.y + pl.norm.z * v.z;

        //dist1 = v.Norm () * (-pl.DD / denom);
        dist1 = v.z * (-pl.DD / denom);
      }
      else
        dist1 = 0;

      //@@@ assume all FX polygons have no outgoing plane
      if (!ctxt->added_fog_info)
      {
        const csPlane3 &pl = fi->outgoing_plane;
        float denom = pl.norm.x * v.x + pl.norm.y * v.y + pl.norm.z * v.z;

        //dist2 = v.Norm () * (-pl.DD / denom);
        dist2 = v.z * (-pl.DD / denom);
      }
      else
        dist2 = v.Norm ();

#ifdef USE_EXP_FOG
      // Implement semi-exponential fog (linearily approximated)
      uint table_index = QRound (
          (100 * ABS (dist2 - dist1)) * fi->fog->density);
      float I2;
      if (table_index < FOG_EXP_TABLE_SIZE)
        I2 = fog_exp_table[table_index];
      else
        I2 = fog_exp_table[FOG_EXP_TABLE_SIZE - 1];
#else
      float I2 = ABS (dist2 - dist1) * fi->fog->density;
#endif
      if (I2 > CS_FOG_MAXVALUE)
        I2 = CS_FOGTABLE_CLAMPVALUE;
      else
        I2 = I2 * CS_FOGTABLE_DISTANCESCALE;

      if (poly.fog_info[i].intensity)
      {
        // We already have a previous fog level. In this case we do some
        // mathematical tricks to combine both fog levels. Substitute one
        // fog expresion in the other. The basic fog expression is:
        //	C = I*F + (1-I)*P
        //	with I = intensity
        //	     F = fog color
        //	     P = polygon color
        //	     C = result
        float I1 = poly.fog_info[i].intensity;
        float I = I1 + I2 - I1 * I2;
        if (I > CS_FOGTABLE_CLAMPVALUE) I = CS_FOGTABLE_CLAMPVALUE;
        poly.fog_info[i].intensity = I;

        float fact = 1.0f / I;
        poly.fog_info[i].r =
          (
            I2 * fi->fog->red +
            I1 * poly.fog_info[i].r +
            I1 * I2 * poly.fog_info[i].r
          ) * fact;
        poly.fog_info[i].g =
          (
            I2 * fi->fog->green +
            I1 * poly.fog_info[i].g +
            I1 * I2 * poly.fog_info[i].g
          ) * fact;
        poly.fog_info[i].b =
          (
            I2 * fi->fog->blue +
            I1 * poly.fog_info[i].b +
            I1 * I2 * poly.fog_info[i].b
          ) * fact;
      }
      else
      {
        // The first fog level.
        poly.fog_info[i].intensity = I2;
        poly.fog_info[i].r = fi->fog->red;
        poly.fog_info[i].g = fi->fog->green;
        poly.fog_info[i].b = fi->fog->blue;
      }

      fi = fi->next;
    }
  }
}

void csRenderView::CalculateFogMesh (
  const csTransform &tr_o2c,
  G3DPolygonMesh &mesh)
{
  if (!ctxt->fog_info)
  {
    mesh.do_fog = false;
    return ;
  }

  mesh.do_fog = true;

#ifdef USE_EXP_FOG
  if (!fog_exp_table) InitializeFogTable ();
#endif
  //CS_ASSERT (mesh.buffers[0]->IsLocked ());

  int i;
  int num_vertices = mesh.polybuf->GetVertexCount ();
  csVector3 *verts = mesh.polybuf->GetVertices ();
  if(!mesh.vertex_fog) mesh.vertex_fog = new G3DFogInfo[num_vertices];
  for (i = 0; i < num_vertices; i++)
  {
    // This is stupid!!! The purpose of DrawTriangleMesh is to be
    // able to avoid doing camera transformation in the engine at all.
    // And here we do it again... So this remains here until someone
    // fixes this calculation to work with object/world space coordinates
    // (depending on how the mesh is defined). For now this works and
    // that's most important :-)
    csVector3 v;
    if (mesh.vertex_mode == G3DPolygonMesh::VM_VIEWSPACE)
      v = verts[i];
    else
    {
      v = tr_o2c * verts[i];
    }

    // Initialize fog vertex.
    mesh.vertex_fog[i].r = 0;
    mesh.vertex_fog[i].g = 0;
    mesh.vertex_fog[i].b = 0;
    mesh.vertex_fog[i].intensity = 0;
    mesh.vertex_fog[i].intensity2 = 0;

    // Consider a ray between (0,0,0) and v and calculate the thickness of every

    // fogged sector in between.
    csFogInfo *finfo = ctxt->fog_info;
    while (finfo)
    {
      float dist1, dist2;
      if (finfo->has_incoming_plane)
      {
        const csPlane3 &pl = finfo->incoming_plane;
        float denom = pl.norm.x * v.x + pl.norm.y * v.y + pl.norm.z * v.z;

        //dist1 = v.Norm () * (-pl.DD / denom);
        dist1 = v.z * (-pl.DD / denom);
      }
      else
        dist1 = 0;

      //@@@ assume all FX polygons have no outgoing plane
      if (!ctxt->added_fog_info)
      {
        const csPlane3 &pl = finfo->outgoing_plane;
        float denom = pl.norm.x * v.x + pl.norm.y * v.y + pl.norm.z * v.z;

        //dist2 = v.Norm () * (-pl.DD / denom);
        dist2 = v.z * (-pl.DD / denom);
      }
      else
        dist2 = v.Norm ();

#ifdef USE_EXP_FOG
      // Implement semi-exponential fog (linearily approximated)
      uint table_index = QRound (
          (100 * ABS (dist2 - dist1)) * finfo->fog->density);
      float I2;
      if (table_index < FOG_EXP_TABLE_SIZE)
        I2 = fog_exp_table[table_index];
      else
        I2 = fog_exp_table[FOG_EXP_TABLE_SIZE - 1];
#else
      float I2 = ABS (dist2 - dist1) * finfo->fog->density;
#endif
      if (I2 > CS_FOG_MAXVALUE)
        I2 = CS_FOGTABLE_CLAMPVALUE;
      else
        I2 = I2 * CS_FOGTABLE_DISTANCESCALE;

      if (mesh.vertex_fog[i].intensity)
      {
        // We already have a previous fog level. In this case we do some
        // mathematical tricks to combine both fog levels. Substitute one
        // fog expresion in the other. The basic fog expression is:
        //	C = I*F + (1-I)*P
        //	with I = intensity
        //	     F = fog color
        //	     P = polygon color
        //	     C = result
        float I1 = mesh.vertex_fog[i].intensity;
        float I = I1 + I2 - I1 * I2;
        if (I > CS_FOGTABLE_CLAMPVALUE) I = CS_FOGTABLE_CLAMPVALUE;
        mesh.vertex_fog[i].intensity = I;

        float fact = 1.0f / I;
        mesh.vertex_fog[i].r =
          (
            I2 * finfo->fog->red +
            I1 * mesh.vertex_fog[i].r +
            I1 * I2 * mesh.vertex_fog[i].r
          ) * fact;
        mesh.vertex_fog[i].g =
          (
            I2 * finfo->fog->green +
            I1 * mesh.vertex_fog[i].g +
            I1 * I2 * mesh.vertex_fog[i].g
          ) * fact;
        mesh.vertex_fog[i].b =
          (
            I2 * finfo->fog->blue +
            I1 * mesh.vertex_fog[i].b +
            I1 * I2 * mesh.vertex_fog[i].b
          ) * fact;
      }
      else
      {
        // The first fog level.
        mesh.vertex_fog[i].intensity = I2;
        mesh.vertex_fog[i].r = finfo->fog->red;
        mesh.vertex_fog[i].g = finfo->fog->green;
        mesh.vertex_fog[i].b = finfo->fog->blue;
      }

      finfo = finfo->next;
    }
  }
}


void csRenderView::CalculateFogMesh (
  const csTransform &tr_o2c,
  G3DTriangleMesh &mesh)
{
  if (!ctxt->fog_info)
  {
    mesh.do_fog = false;
    return ;
  }

  mesh.do_fog = true;

#ifdef USE_EXP_FOG
  if (!fog_exp_table) InitializeFogTable ();
#endif
  CS_ASSERT (mesh.buffers[0]->IsLocked ());

  int i;
  int num_vertices = mesh.buffers[0]->GetVertexCount ();
  csVector3 *verts = mesh.buffers[0]->GetVertices ();
  for (i = 0; i < num_vertices; i++)
  {
    // This is stupid!!! The purpose of DrawTriangleMesh is to be
    // able to avoid doing camera transformation in the engine at all.
    // And here we do it again... So this remains here until someone
    // fixes this calculation to work with object/world space coordinates
    // (depending on how the mesh is defined). For now this works and
    // that's most important :-)
    csVector3 v;
    if (mesh.vertex_mode == G3DTriangleMesh::VM_VIEWSPACE)
      v = verts[i];
    else
    {
      v = tr_o2c * verts[i];
    }

    // Initialize fog vertex.
    mesh.vertex_fog[i].r = 0;
    mesh.vertex_fog[i].g = 0;
    mesh.vertex_fog[i].b = 0;
    mesh.vertex_fog[i].intensity = 0;
    mesh.vertex_fog[i].intensity2 = 0;

    // Consider a ray between (0,0,0) and v and calculate the thickness of every

    // fogged sector in between.
    csFogInfo *finfo = ctxt->fog_info;
    while (finfo)
    {
      float dist1, dist2;
      if (finfo->has_incoming_plane)
      {
        const csPlane3 &pl = finfo->incoming_plane;
        float denom = pl.norm.x * v.x + pl.norm.y * v.y + pl.norm.z * v.z;

        //dist1 = v.Norm () * (-pl.DD / denom);
        dist1 = v.z * (-pl.DD / denom);
      }
      else
        dist1 = 0;

      //@@@ assume all FX polygons have no outgoing plane
      if (!ctxt->added_fog_info)
      {
        const csPlane3 &pl = finfo->outgoing_plane;
        float denom = pl.norm.x * v.x + pl.norm.y * v.y + pl.norm.z * v.z;

        //dist2 = v.Norm () * (-pl.DD / denom);
        dist2 = v.z * (-pl.DD / denom);
      }
      else
        dist2 = v.Norm ();

#ifdef USE_EXP_FOG
      // Implement semi-exponential fog (linearily approximated)
      uint table_index = QRound (
          (100 * ABS (dist2 - dist1)) * finfo->fog->density);
      float I2;
      if (table_index < FOG_EXP_TABLE_SIZE)
        I2 = fog_exp_table[table_index];
      else
        I2 = fog_exp_table[FOG_EXP_TABLE_SIZE - 1];
#else
      float I2 = ABS (dist2 - dist1) * finfo->fog->density;
#endif
      if (I2 > CS_FOG_MAXVALUE)
        I2 = CS_FOGTABLE_CLAMPVALUE;
      else
        I2 = I2 * CS_FOGTABLE_DISTANCESCALE;

      if (mesh.vertex_fog[i].intensity)
      {
        // We already have a previous fog level. In this case we do some
        // mathematical tricks to combine both fog levels. Substitute one
        // fog expresion in the other. The basic fog expression is:
        //	C = I*F + (1-I)*P
        //	with I = intensity
        //	     F = fog color
        //	     P = polygon color
        //	     C = result
        float I1 = mesh.vertex_fog[i].intensity;
        float I = I1 + I2 - I1 * I2;
        if (I > CS_FOGTABLE_CLAMPVALUE) I = CS_FOGTABLE_CLAMPVALUE;
        mesh.vertex_fog[i].intensity = I;

        float fact = 1.0f / I;
        mesh.vertex_fog[i].r =
          (
            I2 * finfo->fog->red +
            I1 * mesh.vertex_fog[i].r +
            I1 * I2 * mesh.vertex_fog[i].r
          ) * fact;
        mesh.vertex_fog[i].g =
          (
            I2 * finfo->fog->green +
            I1 * mesh.vertex_fog[i].g +
            I1 * I2 * mesh.vertex_fog[i].g
          ) * fact;
        mesh.vertex_fog[i].b =
          (
            I2 * finfo->fog->blue +
            I1 * mesh.vertex_fog[i].b +
            I1 * I2 * mesh.vertex_fog[i].b
          ) * fact;
      }
      else
      {
        // The first fog level.
        mesh.vertex_fog[i].intensity = I2;
        mesh.vertex_fog[i].r = finfo->fog->red;
        mesh.vertex_fog[i].g = finfo->fog->green;
        mesh.vertex_fog[i].b = finfo->fog->blue;
      }

      finfo = finfo->next;
    }
  }
}

void csRenderView::UpdateFrustum ()
{
  int i;
  csBox2 bbox;
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

void csRenderView::TestSphereFrustumWorld (
  csRenderContext *ctxt,
  const csVector3 &center,
  float radius,
  bool &inside,
  bool &outside)
{
  float dist;
  csPlane3 *frust = ctxt->clip_planes;
  outside = true;
  inside = true;

  dist = frust[0].Classify (center);
  if (dist < radius) inside = false;
  if ((-dist) <= radius)
  {
    dist = frust[1].Classify (center);
    if (dist < radius) inside = false;
    if ((-dist) <= radius)
    {
      dist = frust[2].Classify (center);
      if (dist < radius) inside = false;
      if ((-dist) <= radius)
      {
        dist = frust[3].Classify (center);
        if (dist < radius) inside = false;
        if ((-dist) <= radius) outside = false;
      }
    }
  }
}

bool csRenderView::TestBSphere (
  const csReversibleTransform &w2c,
  const csSphere &sphere)
{
  //------
  // First transform bounding sphere from object space to camera space
  // by using the given transform (if needed).
  //------
  csSphere tr_sphere = w2c.Other2This (sphere);
  const csVector3 &tr_center = tr_sphere.GetCenter ();
  float radius = tr_sphere.GetRadius ();

  //------
  // Test if object is behind the camera plane.
  //------
  if (tr_center.z + radius <= 0) return false;

  //------
  // Test against far plane if needed.
  //------
  csPlane3 *far_plane = ctxt->icamera->GetFarPlane ();
  if (far_plane)
  {
    // Ok, so this is not really far plane clipping - we just dont draw this
    // object if the bounding sphere is further away than the D
    // part of the farplane.
    if (tr_center.z - radius > far_plane->D ()) return false;
  }

  //------
  // Check if we're fully inside the bounding sphere.
  //------
  bool fully_inside = csSquaredDist::PointPoint (
        csVector3 (0),
        tr_center) <= radius *
    radius;

  //------
  // Test if there is a chance we must clip to current portal.
  //------
  bool outside, inside;
  if (!fully_inside)
  {
    TestSphereFrustumWorld (
      ctxt,
      sphere.GetCenter (),
      radius,
      inside,
      outside);
    if (outside) return false;
  }

  //------
  // Test if there is a chance we must clip to current plane.
  //------
  if (ctxt->do_clip_plane)
  {
    //bool mirror = GetCamera ()->IsMirrored ();
    float dist = ctxt->clip_plane.Classify (tr_center);
    dist = -dist;
    if ((-dist) > radius) return false;
  }

  return true;
}

void csRenderView::CalculateClipSettings (
  uint32 frustum_mask,
  int &clip_portal,
  int &clip_plane,
  int &clip_z_plane)
{
  if (frustum_mask & 0xf)
    clip_portal = CS_CLIP_NEEDED;
  else
    clip_portal = CS_CLIP_NOT;

  if (frustum_mask & 0x10)
    clip_z_plane = CS_CLIP_NEEDED;
  else
    clip_z_plane = CS_CLIP_NOT;

  if (frustum_mask & 0x20)
    clip_plane = CS_CLIP_NEEDED;
  else
    clip_plane = CS_CLIP_NOT;
}

bool csRenderView::ClipBSphere (
  const csSphere &cam_sphere,
  const csSphere &world_sphere,
  int &clip_portal,
  int &clip_plane,
  int &clip_z_plane)
{
  float radius = cam_sphere.GetRadius ();
  float cam_z = cam_sphere.GetCenter ().z;

  //------
  // Test if object is behind the camera plane.
  //------
  if (cam_z + radius <= 0) return false;

  //------
  // Test against far plane if needed.
  //------
  csPlane3 *far_plane = ctxt->icamera->GetFarPlane ();
  if (far_plane)
  {
    // Ok, so this is not really far plane clipping - we just dont draw this
    // object if the bounding sphere is further away than the D
    // part of the farplane.
    if (cam_z - radius > far_plane->D ()) return false;
  }

  //------
  // Check if we're fully inside the bounding sphere.
  //------
  bool fully_inside = csSquaredDist::PointPoint (
        csVector3 (0), cam_sphere.GetCenter ()) <= radius * radius;

  //------
  // Test if there is a chance we must clip to current portal.
  //------
  bool outside, inside;
  if (fully_inside)
  {
    clip_portal = CS_CLIP_NEEDED;
  }
  else
  {
    TestSphereFrustumWorld (
      ctxt,
      world_sphere.GetCenter (),
      radius,
      inside,
      outside);
    if (outside)
      return false;
    if (!inside)
      clip_portal = CS_CLIP_NEEDED;
    else
      clip_portal = CS_CLIP_NOT;
  }

  //------
  // Test if there is a chance we must clip to the z-plane.
  //------
  if (cam_z - radius > 0)
    clip_z_plane = CS_CLIP_NOT;
  else
    clip_z_plane = CS_CLIP_NEEDED;

  //------
  // Test if there is a chance we must clip to current plane.
  //------
  clip_plane = CS_CLIP_NOT;
  if (ctxt->do_clip_plane)
  {
    float dist = ctxt->clip_plane.Classify (cam_sphere.GetCenter ());
    dist = -dist;
    if ((-dist) > radius)
      return false;
    else if (dist <= radius)
      clip_plane = CS_CLIP_NEEDED;
  }

  return true;
}

void csRenderView::SetupClipPlanes ()
{
  csPlane3* frust = ctxt->frustum;
  const csReversibleTransform& tr = ctxt->icamera->GetTransform ();
  csVector3 o2tmult = tr.GetO2T () * tr.GetO2TTranslation ();
  csPlane3* clip_planes = ctxt->clip_planes;
  clip_planes[0].Set (tr.GetT2O() * frust[0].norm, -frust[0].norm*o2tmult);
  clip_planes[1].Set (tr.GetT2O() * frust[1].norm, -frust[1].norm*o2tmult);
  clip_planes[2].Set (tr.GetT2O() * frust[2].norm, -frust[2].norm*o2tmult);
  clip_planes[3].Set (tr.GetT2O() * frust[3].norm, -frust[3].norm*o2tmult);
  csPlane3 pz0 (0, 0, 1, 0);	// Inverted!!!.
  clip_planes[4] = tr.This2Other (pz0);
  if (ctxt->do_clip_plane)
  {
    // We have a real near clipping plane. In that case
    // we add both the Z=0 plane and the near clipping plane.
    csPlane3 pznear = ctxt->clip_plane;
    pznear.Invert ();
    clip_planes[5] = tr.This2Other (pznear);
    ctxt->clip_planes_mask = 0x3f;
  }
  else
  {
    ctxt->clip_planes_mask = 0x1f;
  }
  csPlane3 *far_plane = ctxt->icamera->GetFarPlane ();
  if (far_plane)
  {
    csPlane3 fp = *far_plane;
    clip_planes[6] = tr.This2Other (fp);
    ctxt->clip_planes_mask |= 0x40;
  }
}

void csRenderView::SetupClipPlanes (const csReversibleTransform& tr_o2c,
  	csPlane3* planes, uint32& frustum_mask)
{
  csPlane3* frust = ctxt->frustum;
  csVector3 o2tmult = tr_o2c.GetO2T () * tr_o2c.GetO2TTranslation ();
  planes[0].Set (tr_o2c.GetT2O() * frust[0].norm, -frust[0].norm*o2tmult);
  planes[1].Set (tr_o2c.GetT2O() * frust[1].norm, -frust[1].norm*o2tmult);
  planes[2].Set (tr_o2c.GetT2O() * frust[2].norm, -frust[2].norm*o2tmult);
  planes[3].Set (tr_o2c.GetT2O() * frust[3].norm, -frust[3].norm*o2tmult);
  csPlane3 pz0 (0, 0, 1, 0);	// Inverted!!!.
  planes[4] = tr_o2c.This2Other (pz0);
  if (ctxt->do_clip_plane)
  {
    // We have a real near clipping plane. In that case
    // we add both the Z=0 plane and the near clipping plane.
    csPlane3 pznear = ctxt->clip_plane;
    pznear.Invert ();
    planes[5] = tr_o2c.This2Other (pznear);
    frustum_mask = 0x3f;
  }
  else
  {
    frustum_mask = 0x1f;
  }
  csPlane3 *far_plane = ctxt->icamera->GetFarPlane ();
  if (far_plane)
  {
    csPlane3 fp = *far_plane;
    planes[6] = tr_o2c.This2Other (fp);
    frustum_mask |= 0x40;
  }
}

bool csRenderView::ClipBBox (
  csPlane3* planes,
  uint32& frustum_mask,
  const csBox3 &obox,
  int &clip_portal,
  int &clip_plane,
  int &clip_z_plane)
{
  uint32 outClipMask;
  if (!csIntersect3::BoxFrustum (obox, planes, frustum_mask, outClipMask))
    return false;	// Not visible.
  frustum_mask = outClipMask;

  if (outClipMask & 0xf)
    clip_portal = CS_CLIP_NEEDED;
  else
    clip_portal = CS_CLIP_NOT;

  if (outClipMask & 0x10)
    clip_z_plane = CS_CLIP_NEEDED;
  else
    clip_z_plane = CS_CLIP_NOT;

  if (outClipMask & 0x20)
    clip_plane = CS_CLIP_NEEDED;
  else
    clip_plane = CS_CLIP_NOT;

  return true;
}

void csRenderView::CreateRenderContext ()
{
  csRenderContext *old_ctxt = ctxt;

  // @@@ Use a pool for render contexts?
  // A pool would work very well here since the number of render contexts
  // is limited by recursion depth.
  ctxt = new csRenderContext ();
  *ctxt = *old_ctxt;
  if (ctxt->icamera) ctxt->icamera->IncRef ();
  if (ctxt->iview) ctxt->iview->IncRef ();
  // The camera transform id is copied from the old
  // context. Only when we do space warping on the camera
  // do we have to change it (CreateNewCamera() function).
  context_id++;
  ctxt->context_id = context_id;
}

void csRenderView::RestoreRenderContext (csRenderContext *original)
{
  csRenderContext *old_ctxt = ctxt;
  ctxt = original;

  if (old_ctxt->icamera) old_ctxt->icamera->DecRef ();
  if (old_ctxt->iview) old_ctxt->iview->DecRef ();
  delete old_ctxt;
}

iCamera *csRenderView::CreateNewCamera ()
{
  // A pool for cameras?
  iCamera *newcam = ctxt->icamera->Clone ();
  ctxt->icamera->DecRef ();
  ctxt->icamera = newcam;
  return ctxt->icamera;
}

uint csRenderView::GetCurrentFrameNumber () const
{
  return engine->GetCurrentFrameNumber ();
}
