/*
    Copyright (C) 2000 by Andrew Zabolotny

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
#include "csengine/rview.h"
#include "csengine/engine.h"
#include "csgeom/polyclip.h"
#include "igraph3d.h"
#include "iclip2.h"
#include "icamera.h"

csFrustumView::csFrustumView () : light_frustum (NULL), callback (NULL),
  callback_data (NULL)
{
  memset (this, 0, sizeof (csFrustumView));
}

csFrustumView::csFrustumView (const csFrustumView &iCopy)
{
  // hehe. kind of trick.
  memcpy (this, &iCopy, sizeof (csFrustumView));
  // Leave cleanup actions alone to original copy
  cleanup = NULL;
}

csFrustumView::~csFrustumView ()
{
  while (cleanup)
  {
    csFrustrumViewCleanup *next = cleanup->next;
    cleanup->action (this, cleanup);
    cleanup = next;
  }
  delete light_frustum;
}

bool csFrustumView::DeregisterCleanup (csFrustrumViewCleanup *action)
{
  csFrustrumViewCleanup **pcur = &cleanup;
  csFrustrumViewCleanup *cur = cleanup;
  while (cur)
  {
    if (cur == action)
    {
      *pcur = cur->next;
      return true;
    }
    pcur = &cur->next;
    cur = cur->next;
  }
  return false;
}

//---------------------------------------------------------------------------

IMPLEMENT_IBASE_EXT (csRenderView)
  IMPLEMENTS_EMBEDDED_INTERFACE (iRenderView)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csRenderView::RenderView)
  IMPLEMENTS_INTERFACE (iRenderView)
IMPLEMENT_EMBEDDED_IBASE_END

csRenderView::csRenderView () :
    csCamera (), engine (NULL), view (NULL), g3d (NULL), g2d (NULL),
    portal_polygon (NULL), previous_sector (NULL), this_sector (NULL),
    do_clip_plane (false), do_clip_frustum (false),
    callback (NULL), callback_data (NULL), fog_info (NULL),
    added_fog_info (false)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiRenderView);
}

csRenderView::csRenderView (const csCamera& c) :
    csCamera (c), engine (NULL), view (NULL), g3d (NULL), g2d (NULL),
    portal_polygon (NULL), previous_sector (NULL), this_sector (NULL),
    do_clip_plane (false), do_clip_frustum (false),
    callback (NULL), callback_data (NULL), fog_info (NULL),
    added_fog_info (false)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiRenderView);
}

csRenderView::csRenderView (const csCamera& c, csClipper* v, iGraphics3D* ig3d,
    iGraphics2D* ig2d) :
    csCamera (c), engine (NULL), view (v), g3d (ig3d), g2d (ig2d),
    portal_polygon (NULL), previous_sector (NULL), this_sector (NULL),
    do_clip_plane (false), do_clip_frustum (false),
    callback (NULL), callback_data (NULL), fog_info (NULL),
    added_fog_info (false)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiRenderView);
}

// Remove this for old fog
//#define USE_EXP_FOG

// After such number of values fog density coefficient can be considered 0.
#ifdef USE_EXP_FOG

#define FOG_EXP_TABLE_SIZE 1600
static float *fog_exp_table = NULL;

static void InitializeFogTable ()
{
  fog_exp_table = new float [FOG_EXP_TABLE_SIZE];
  for (int i = 0; i < FOG_EXP_TABLE_SIZE; i++)
    fog_exp_table [i] = 1 - exp (-float (i) / 256.);
}
#endif

#define SMALL_D 0.01
void csRenderView::CalculateFogPolygon (G3DPolygonDP& poly)
{
  if (!fog_info || poly.num < 3) { poly.use_fog = false; return; }
  poly.use_fog = true;

#ifdef USE_EXP_FOG
  if (!fog_exp_table)
    InitializeFogTable ();
#endif

  // Get the plane normal of the polygon. Using this we can calculate
  // '1/z' at every screen space point.
  float inv_aspect = poly.inv_aspect;
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
    O = 1/poly.z_value;
  }
  else
  {
    inv_Dc = 1/Dc;
    M = -Ac*inv_Dc*inv_aspect;
    N = -Bc*inv_Dc*inv_aspect;
    O = -Cc*inv_Dc;
  }

  int i;
  for (i = 0 ; i < poly.num ; i++)
  {
    // Calculate the original 3D coordinate again (camera space).
    csVector3 v;
    v.z = 1. / (M * (poly.vertices[i].sx - GetShiftX ()) + N * (poly.vertices[i].sy - GetShiftY ()) + O);
    v.x = (poly.vertices[i].sx - GetShiftX ()) * v.z * inv_aspect;
    v.y = (poly.vertices[i].sy - GetShiftY ()) * v.z * inv_aspect;

    // Initialize fog vertex.
    poly.fog_info[i].r = 0;
    poly.fog_info[i].g = 0;
    poly.fog_info[i].b = 0;
    poly.fog_info[i].intensity = 0;

    // Consider a ray between (0,0,0) and v and calculate the thickness of every
    // fogged sector in between.
    csFogInfo* fi = fog_info;
    while (fi)
    {
      float dist1, dist2;
      if (fi->has_incoming_plane)
      {
	const csPlane3& pl = fi->incoming_plane;
	float denom = pl.norm.x*v.x + pl.norm.y*v.y + pl.norm.z*v.z;
	//dist1 = v.Norm () * (-pl.DD / denom);
	dist1 = v.z * (-pl.DD / denom);
      }
      else
        dist1 = 0;
      const csPlane3& pl = fi->outgoing_plane;
      float denom = pl.norm.x*v.x + pl.norm.y*v.y + pl.norm.z*v.z;
      //dist2 = v.Norm () * (-pl.DD / denom);
      dist2 = v.z * (-pl.DD / denom);

#ifdef USE_EXP_FOG
      // Implement semi-exponential fog (linearily approximated)
      UInt table_index = QRound ((100 * ABS (dist2 - dist1)) * fi->fog->density);
      float I2;
      if (table_index < FOG_EXP_TABLE_SIZE)
        I2 = fog_exp_table [table_index];
      else
        I2 = fog_exp_table[FOG_EXP_TABLE_SIZE-1];
#else
      float I2 = ABS (dist2 - dist1) * fi->fog->density;
#endif

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
	poly.fog_info[i].intensity = I1 + I2 - I1 * I2;
	float fact = 1. / (I1 + I2 - I1*I2);
	poly.fog_info[i].r = (I2*fi->fog->red + I1*poly.fog_info[i].r + I1*I2*poly.fog_info[i].r) * fact;
	poly.fog_info[i].g = (I2*fi->fog->green + I1*poly.fog_info[i].g + I1*I2*poly.fog_info[i].g) * fact;
	poly.fog_info[i].b = (I2*fi->fog->blue + I1*poly.fog_info[i].b + I1*I2*poly.fog_info[i].b) * fact;
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
void csRenderView::CalculateFogPolygon (G3DPolygonDPFX& poly)
{
  if (!fog_info || poly.num < 3) { poly.use_fog = false; return; }
  poly.use_fog = true;

#ifdef USE_EXP_FOG
  if (!fog_exp_table)
    InitializeFogTable ();
#endif

  float inv_aspect = poly.inv_aspect;

  int i;
  for (i = 0 ; i < poly.num ; i++)
  {
    // Calculate the original 3D coordinate again (camera space).
    csVector3 v;
    v.z = 1. / poly.vertices[i].z;
    v.x = (poly.vertices[i].sx - GetShiftX ()) * v.z * inv_aspect;
    v.y = (poly.vertices[i].sy - GetShiftY ()) * v.z * inv_aspect;

    // Initialize fog vertex.
    poly.fog_info[i].r = 0;
    poly.fog_info[i].g = 0;
    poly.fog_info[i].b = 0;
    poly.fog_info[i].intensity = 0;

    // Consider a ray between (0,0,0) and v and calculate the thickness of every
    // fogged sector in between.
    csFogInfo* fi = fog_info;
    while (fi)
    {
      float dist1, dist2;
      if (fi->has_incoming_plane)
      {
	const csPlane3& pl = fi->incoming_plane;
	float denom = pl.norm.x*v.x + pl.norm.y*v.y + pl.norm.z*v.z;
	//dist1 = v.Norm () * (-pl.DD / denom);
        dist1 = v.z * (-pl.DD / denom);
      }
      else
        dist1 = 0;
      //@@@ assume all FX polygons have no outgoing plane
      if (!added_fog_info)
      {
        const csPlane3& pl = fi->outgoing_plane;
        float denom = pl.norm.x*v.x + pl.norm.y*v.y + pl.norm.z*v.z;
        //dist2 = v.Norm () * (-pl.DD / denom);
        dist2 = v.z * (-pl.DD / denom);
      }
      else
        dist2 = v.Norm();

#ifdef USE_EXP_FOG
      // Implement semi-exponential fog (linearily approximated)
      UInt table_index = QRound ((100 * ABS (dist2 - dist1)) * fi->fog->density);
      float I2;
      if (table_index < FOG_EXP_TABLE_SIZE)
        I2 = fog_exp_table [table_index];
      else
        I2 = fog_exp_table[FOG_EXP_TABLE_SIZE-1];
#else
      float I2 = ABS (dist2 - dist1) * fi->fog->density;
#endif

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
	poly.fog_info[i].intensity = I1 + I2 - I1*I2;
	float fact = 1. / (I1 + I2 - I1*I2);
	poly.fog_info[i].r = (I2*fi->fog->red + I1*poly.fog_info[i].r + I1*I2*poly.fog_info[i].r) * fact;
	poly.fog_info[i].g = (I2*fi->fog->green + I1*poly.fog_info[i].g + I1*I2*poly.fog_info[i].g) * fact;
	poly.fog_info[i].b = (I2*fi->fog->blue + I1*poly.fog_info[i].b + I1*I2*poly.fog_info[i].b) * fact;
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

void csRenderView::CalculateFogMesh (const csTransform& tr_o2c, G3DTriangleMesh& mesh)
{
  if (!fog_info) { mesh.do_fog = false; return; }
  mesh.do_fog = true;

#ifdef USE_EXP_FOG
  if (!fog_exp_table)
    InitializeFogTable ();
#endif

  int i;
  csVector3* verts = mesh.vertices[0];
  for (i = 0 ; i < mesh.num_vertices ; i++)
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

    // Consider a ray between (0,0,0) and v and calculate the thickness of every
    // fogged sector in between.
    csFogInfo* fog_info = fog_info;
    while (fog_info)
    {
      float dist1, dist2;
      if (fog_info->has_incoming_plane)
      {
	const csPlane3& pl = fog_info->incoming_plane;
	float denom = pl.norm.x*v.x + pl.norm.y*v.y + pl.norm.z*v.z;
	//dist1 = v.Norm () * (-pl.DD / denom);
        dist1 = v.z * (-pl.DD / denom);
      }
      else
        dist1 = 0;
      //@@@ assume all FX polygons have no outgoing plane
      if (!added_fog_info)
      {
        const csPlane3& pl = fog_info->outgoing_plane;
        float denom = pl.norm.x*v.x + pl.norm.y*v.y + pl.norm.z*v.z;
        //dist2 = v.Norm () * (-pl.DD / denom);
        dist2 = v.z * (-pl.DD / denom);
      }
      else
        dist2 = v.Norm();

#ifdef USE_EXP_FOG
      // Implement semi-exponential fog (linearily approximated)
      UInt table_index = QRound ((100 * ABS (dist2 - dist1)) * fog_info->fog->density);
      float I2;
      if (table_index < FOG_EXP_TABLE_SIZE)
        I2 = fog_exp_table [table_index];
      else
        I2 = fog_exp_table[FOG_EXP_TABLE_SIZE-1];
#else
      float I2 = ABS (dist2 - dist1) * fog_info->fog->density;
#endif

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
	mesh.vertex_fog[i].intensity = I1 + I2 - I1*I2;
	float fact = 1. / (I1 + I2 - I1*I2);
	mesh.vertex_fog[i].r = (I2*fog_info->fog->red + I1*mesh.vertex_fog[i].r + I1*I2*mesh.vertex_fog[i].r) * fact;
	mesh.vertex_fog[i].g = (I2*fog_info->fog->green + I1*mesh.vertex_fog[i].g + I1*I2*mesh.vertex_fog[i].g) * fact;
	mesh.vertex_fog[i].b = (I2*fog_info->fog->blue + I1*mesh.vertex_fog[i].b + I1*I2*mesh.vertex_fog[i].b) * fact;
      }
      else
      {
        // The first fog level.
        mesh.vertex_fog[i].intensity = I2;
        mesh.vertex_fog[i].r = fog_info->fog->red;
        mesh.vertex_fog[i].g = fog_info->fog->green;
        mesh.vertex_fog[i].b = fog_info->fog->blue;
      }
      fog_info = fog_info->next;
    }
  }
}

iEngine* csRenderView::RenderView::GetEngine ()
{
  return QUERY_INTERFACE (scfParent->engine, iEngine);
}

iClipper2D* csRenderView::RenderView::GetClipper ()
{
  return QUERY_INTERFACE (scfParent->view, iClipper2D);
}

iCamera* csRenderView::RenderView::GetCamera ()
{
  return QUERY_INTERFACE (scfParent, iCamera);
}

