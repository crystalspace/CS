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
#include "csengine/rview.h"
#include "csengine/engine.h"
#include "csengine/polygon.h"
#include "csengine/sector.h"
#include "csgeom/polyclip.h"
#include "ivideo/graph3d.h"
#include "igeom/clip2d.h"
#include "iengine/camera.h"

IMPLEMENT_IBASE (csRenderView)
  IMPLEMENTS_INTERFACE (iRenderView)
IMPLEMENT_IBASE_END

csRenderView::csRenderView () :
    ctxt (NULL), iengine (NULL), g3d (NULL), g2d (NULL),
    callback (NULL), callback_data (NULL)
{
  CONSTRUCT_IBASE (NULL);
  ctxt = new csRenderContext ();
  memset (ctxt, 0, sizeof (csRenderContext));
}

csRenderView::csRenderView (iCamera* c) :
    ctxt (NULL), iengine (NULL), g3d (NULL), g2d (NULL),
    callback (NULL), callback_data (NULL)
{
  CONSTRUCT_IBASE (NULL);
  ctxt = new csRenderContext ();
  memset (ctxt, 0, sizeof (csRenderContext));
  c->IncRef ();
  ctxt->icamera = c;
}

csRenderView::csRenderView (iCamera* c, csClipper* v, iGraphics3D* ig3d,
    	iGraphics2D* ig2d) :
    ctxt (NULL), iengine (NULL), g3d (ig3d), g2d (ig2d),
    callback (NULL), callback_data (NULL)
{
  CONSTRUCT_IBASE (NULL);
  ctxt = new csRenderContext ();
  memset (ctxt, 0, sizeof (csRenderContext));
  c->IncRef ();
  ctxt->icamera = c;
  ctxt->iview = QUERY_INTERFACE (v, iClipper2D);
}

csRenderView::~csRenderView ()
{
  if (ctxt)
  {
    if (ctxt->icamera) ctxt->icamera->DecRef ();
    DeleteRenderContextData (ctxt);
    delete ctxt;
  }
  if (iengine) iengine->DecRef ();
}

void csRenderView::SetCamera (iCamera* icam)
{
  icam->IncRef ();
  if (ctxt->icamera) ctxt->icamera->DecRef ();
  ctxt->icamera = icam;
}

void csRenderView::SetClipper (iClipper2D* view)
{
  view->IncRef ();
  if (ctxt->iview) ctxt->iview->DecRef ();
  ctxt->iview = view;
}

void csRenderView::SetEngine (iEngine* engine)
{
  engine->IncRef ();
  if (iengine) iengine->DecRef ();
  iengine = engine;
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
  if (!ctxt->fog_info || poly.num < 3) { poly.use_fog = false; return; }
  poly.use_fog = true;

#ifdef USE_EXP_FOG
  if (!fog_exp_table)
    InitializeFogTable ();
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
    O = 1/poly.z_value;
  }
  else
  {
    inv_Dc = 1/Dc;
    M = -Ac*inv_Dc*inv_aspect;
    N = -Bc*inv_Dc*inv_aspect;
    O = -Cc*inv_Dc;
  }

  float shift_x = ctxt->icamera->GetShiftX ();
  float shift_y = ctxt->icamera->GetShiftY ();
  int i;
  for (i = 0 ; i < poly.num ; i++)
  {
    // Calculate the original 3D coordinate again (camera space).
    csVector3 v;
    v.z = 1. / (M * (poly.vertices[i].sx - shift_x) + N * (poly.vertices[i].sy - shift_y) + O);
    v.x = (poly.vertices[i].sx - shift_x) * v.z * inv_aspect;
    v.y = (poly.vertices[i].sy - shift_y) * v.z * inv_aspect;

    // Initialize fog vertex.
    poly.fog_info[i].r = 0;
    poly.fog_info[i].g = 0;
    poly.fog_info[i].b = 0;
    poly.fog_info[i].intensity = 0;

    // Consider a ray between (0,0,0) and v and calculate the thickness of every
    // fogged sector in between.
    csFogInfo* fi = ctxt->fog_info;
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
  if (!ctxt->fog_info || poly.num < 3) { poly.use_fog = false; return; }
  poly.use_fog = true;

#ifdef USE_EXP_FOG
  if (!fog_exp_table)
    InitializeFogTable ();
#endif

  float shift_x = ctxt->icamera->GetShiftX ();
  float shift_y = ctxt->icamera->GetShiftY ();
  float inv_aspect = ctxt->icamera->GetInvFOV ();

  int i;
  for (i = 0 ; i < poly.num ; i++)
  {
    // Calculate the original 3D coordinate again (camera space).
    csVector3 v;
    v.z = 1. / poly.vertices[i].z;
    v.x = (poly.vertices[i].sx - shift_x) * v.z * inv_aspect;
    v.y = (poly.vertices[i].sy - shift_y) * v.z * inv_aspect;

    // Initialize fog vertex.
    poly.fog_info[i].r = 0;
    poly.fog_info[i].g = 0;
    poly.fog_info[i].b = 0;
    poly.fog_info[i].intensity = 0;

    // Consider a ray between (0,0,0) and v and calculate the thickness of every
    // fogged sector in between.
    csFogInfo* fi = ctxt->fog_info;
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
      if (!ctxt->added_fog_info)
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
  if (!ctxt->fog_info) { mesh.do_fog = false; return; }
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
    csFogInfo* finfo = ctxt->fog_info;
    while (finfo)
    {
      float dist1, dist2;
      if (finfo->has_incoming_plane)
      {
	const csPlane3& pl = finfo->incoming_plane;
	float denom = pl.norm.x*v.x + pl.norm.y*v.y + pl.norm.z*v.z;
	//dist1 = v.Norm () * (-pl.DD / denom);
        dist1 = v.z * (-pl.DD / denom);
      }
      else
        dist1 = 0;
      //@@@ assume all FX polygons have no outgoing plane
      if (!ctxt->added_fog_info)
      {
        const csPlane3& pl = finfo->outgoing_plane;
        float denom = pl.norm.x*v.x + pl.norm.y*v.y + pl.norm.z*v.z;
        //dist2 = v.Norm () * (-pl.DD / denom);
        dist2 = v.z * (-pl.DD / denom);
      }
      else
        dist2 = v.Norm();

#ifdef USE_EXP_FOG
      // Implement semi-exponential fog (linearily approximated)
      UInt table_index = QRound ((100 * ABS (dist2 - dist1)) * finfo->fog->density);
      float I2;
      if (table_index < FOG_EXP_TABLE_SIZE)
        I2 = fog_exp_table [table_index];
      else
        I2 = fog_exp_table[FOG_EXP_TABLE_SIZE-1];
#else
      float I2 = ABS (dist2 - dist1) * finfo->fog->density;
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
	mesh.vertex_fog[i].r = (I2*finfo->fog->red + I1*mesh.vertex_fog[i].r + I1*I2*mesh.vertex_fog[i].r) * fact;
	mesh.vertex_fog[i].g = (I2*finfo->fog->green + I1*mesh.vertex_fog[i].g + I1*I2*mesh.vertex_fog[i].g) * fact;
	mesh.vertex_fog[i].b = (I2*finfo->fog->blue + I1*mesh.vertex_fog[i].b + I1*I2*mesh.vertex_fog[i].b) * fact;
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

bool csRenderView::ClipBBox (const csBox2& sbox, const csBox3& cbox,
    int& clip_portal, int& clip_plane)
{
  // Test against far plane if needed.
  csPlane3 far_plane;
  if (ctxt->icamera->GetFarPlane (far_plane))
  {
    // Ok, so this is not really a far plane clipping - we just dont draw this
    // object if no point of the camera_bounding box is closer than the D
    // part of the farplane.
    if (cbox.SquaredOriginDist () > far_plane.D ()*far_plane.D ())
      return false;	
  }
  
  // Test if there is a chance we must clip to current portal.
  int i;
  int box_class;
  box_class = ctxt->iview->ClassifyBox (sbox);
  if (box_class == -1) return false; // Not visible.
  if (box_class == 0) clip_portal = CS_CLIP_NEEDED;
  else clip_portal = CS_CLIP_NOT;

  // Test if there is a chance we must clip to current plane.
  // In addition test if we are fully behind the view plane.
  clip_plane = CS_CLIP_NOT;
  if (ctxt->do_clip_plane)
  {
    int cnt = 0;
    int cntz = 0;
    for (i = 0 ; i < 8 ; i++)
    {
      csVector3 c = cbox.GetCorner (i);
      if (c.z < SMALL_D)
      {
	cntz++;
	cnt++;
      }
      // @@@ TEST IF THIS IS THE RIGHT TEST!!!
      else if (ctxt->clip_plane.Classify (c) < 0)
	cnt++;
    }
    if (cnt == 8 || cntz == 8) return false;	// Object not visible.
    if (cnt > 0) clip_plane = CS_CLIP_NEEDED;
    else if (cntz > 0) clip_plane = CS_CLIP_TOPLEVEL;
  }
  else
  {
    // If we have no clip plane we still test against the view plane.
    int cntz = 0;
    for (i = 0 ; i < 8 ; i++)
    {
      csVector3 c = cbox.GetCorner (i);
      if (c.z < SMALL_D)
	cntz++;
    }
    if (cntz == 8) return false;	// Object not visible.
    if (cntz > 0) clip_plane = CS_CLIP_TOPLEVEL;
  }
 
  // If we don't need to clip to the current portal then we
  // test if we need to clip to the top-level portal.
  // Top-level clipping is always required unless we are totally
  // within the top-level frustum.
  // IF it is decided that we need to clip here then we still
  // clip to the inner portal. We have to do clipping anyway so
  // why not do it to the smallest possible clip area.
  if ((!ctxt->do_clip_frustum) || clip_portal != CS_CLIP_NEEDED)
  {
    box_class = iengine->GetTopLevelClipper ()->ClassifyBox (sbox);
    if (box_class == 0) clip_portal = CS_CLIP_TOPLEVEL;
  }

  return true;
}

void csRenderView::CreateRenderContext ()
{
  csRenderContext* old_ctxt = ctxt;
  // @@@ Use a pool for render contexts?
  // A pool would work very well here since the number of render contexts
  // is limited by recursion depth.
  ctxt = new csRenderContext ();
  *ctxt = *old_ctxt;
  if (ctxt->icamera) ctxt->icamera->IncRef ();
  if (ctxt->iview) ctxt->iview->IncRef ();
  ctxt->rcdata = NULL;
}

void csRenderView::RestoreRenderContext (csRenderContext* original)
{
  csRenderContext* old_ctxt = ctxt;
  ctxt = original;

  if (old_ctxt->icamera) old_ctxt->icamera->DecRef ();
  if (old_ctxt->iview) old_ctxt->iview->DecRef ();
  DeleteRenderContextData (old_ctxt);
  delete old_ctxt;
}

iCamera* csRenderView::CreateNewCamera ()
{
  // A pool for cameras?
  csCamera* newcam = new csCamera (ctxt->icamera->GetPrivateObject ());
  ctxt->icamera->DecRef ();
  ctxt->icamera = &newcam->scfiCamera;
  return ctxt->icamera;
}

void csRenderView::DeleteRenderContextData (csRenderContext* rc)
{
  if (!rc) return;
  while (rc->rcdata)
  {
    csRenderContextData* n = (csRenderContextData*)(rc->rcdata);
    rc->rcdata = n->next;
    if (n->data) n->data->DecRef ();
  }
}

void csRenderView::DeleteRenderContextData (void* key)
{
  csRenderContextData** prev = (csRenderContextData**)&(ctxt->rcdata);
  csRenderContextData* cd = (csRenderContextData*)(ctxt->rcdata);
  while (cd)
  {
    if (cd->key == key)
    {
      if (cd->data) cd->data->DecRef ();
      *prev = cd->next;
      delete cd;
      cd = *prev;
    }
    else
    {
      prev = &(cd->next);
      cd = cd->next;
    }
  }
}

void csRenderView::AttachRenderContextData (void* key, iBase* data)
{
  csRenderContextData* cd = new csRenderContextData ();
  cd->next = (csRenderContextData*)(ctxt->rcdata);
  ctxt->rcdata = cd;
  cd->key = key;
  cd->data = data;
}

iBase* csRenderView::FindRenderContextData (void* key)
{
  csRenderContextData* cd = (csRenderContextData*)(ctxt->rcdata);
  while (cd)
  {
    if (cd->key == key) return cd->data;
    cd = cd->next;
  }
  return NULL;
}

