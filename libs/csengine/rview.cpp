/*
    Copyright (C) 2000-2001 by Andrew Zabolotny
    Copyright (C) 2001 by Jorrit Tyberghein

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
#include "ivideo/igraph3d.h"
#include "igeom/iclip2.h"
#include "iengine/icamera.h"

csFrustumView::csFrustumView () : light_frustum (NULL), callback (NULL),
  callback_data (NULL)
{
  memset (this, 0, sizeof (csFrustumView));
  shadows = new csShadowBlockList ();
  shared = false;
}

csFrustumView::csFrustumView (const csFrustumView &iCopy)
{
  // hehe. kind of trick.
  memcpy (this, &iCopy, sizeof (csFrustumView));
  // Leave cleanup actions alone to original copy
  cleanup = NULL;
  shared = true;
}

csFrustumView::~csFrustumView ()
{
  while (cleanup)
  {
    csFrustumViewCleanup *next = cleanup->next;
    cleanup->action (this, cleanup);
    cleanup = next;
  }
  if (light_frustum) light_frustum->DecRef ();
  if (!shared) delete shadows;
}

bool csFrustumView::DeregisterCleanup (csFrustumViewCleanup *action)
{
  csFrustumViewCleanup **pcur = &cleanup;
  csFrustumViewCleanup *cur = cleanup;
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

void csFrustumView::StartNewShadowBlock ()
{
  if (!shared) delete shadows;
  shadows = new csShadowBlockList ();
  shared = false;
}

//---------------------------------------------------------------------------

csShadowBlock::csShadowBlock (csSector* sect, int draw_busy,
	int max_shadows, int delta) : shadows (max_shadows, delta)
{
  sector = sect;
  draw_busy = draw_busy;
}

csShadowBlock::csShadowBlock (int max_shadows, int delta) :
	shadows (max_shadows, delta)
{
  sector = NULL;
  draw_busy = -1;
}

csShadowBlock::~csShadowBlock ()
{
  DeleteShadows ();
}

void csShadowBlock::AddRelevantShadows (csShadowBlock* source,
    	csTransform* trans)
{
  csShadowIterator* shadow_it = source->GetShadowIterator ();
  while (shadow_it->HasNext ())
  {
    csShadowFrustum* csf = (csShadowFrustum*)shadow_it->Next ();
    if (csf->IsRelevant ())
    {
      if (trans)
      {
	csShadowFrustum* copycsf = new csShadowFrustum (*csf);
	copycsf->Transform (trans);
	shadows.Push (copycsf);
      }
      else
      {
        csf->IncRef ();
        shadows.Push (csf);
      }
    }
  }
  delete shadow_it;
}

void csShadowBlock::AddRelevantShadows (csShadowBlockList* source)
{
  csShadowIterator* shadow_it = source->GetShadowIterator ();
  while (shadow_it->HasNext ())
  {
    csShadowFrustum* csf = (csShadowFrustum*)shadow_it->Next ();
    if (csf->IsRelevant ())
    {
      csf->IncRef ();
      shadows.Push (csf);
    }
  }
  delete shadow_it;
}

void csShadowBlock::AddAllShadows (csShadowBlockList* source)
{
  csShadowIterator* shadow_it = source->GetShadowIterator ();
  while (shadow_it->HasNext ())
  {
    csShadowFrustum* csf = (csShadowFrustum*)shadow_it->Next ();
    csf->IncRef ();
    shadows.Push (csf);
  }
  delete shadow_it;
}

void csShadowBlock::AddUniqueRelevantShadows (csShadowBlockList* source)
{
  int i;
  int cnt = shadows.Length ();

  csShadowIterator* shadow_it = source->GetShadowIterator ();
  while (shadow_it->HasNext ())
  {
    csShadowFrustum* csf = (csShadowFrustum*)shadow_it->Next ();
    if (csf->IsRelevant ())
    {
      for (i = 0 ; i < cnt ; i++)
	if (((csShadowFrustum*)shadows[i]) == csf)
	  break;
      if (i >= cnt)
      {
        csf->IncRef ();
        shadows.Push (csf);
      }
    }
  }
  delete shadow_it;
}

csFrustum* csShadowBlock::AddShadow (const csVector3& origin, void* userData,
    	int num_verts, csPlane3& backplane)
{
  csShadowFrustum* sf = new csShadowFrustum (origin, num_verts);
  sf->SetBackPlane (backplane);
  sf->SetUserData (userData);
  shadows.Push (sf);
  return (csFrustum*)sf;
}

void csShadowBlock::UnlinkShadow (int idx)
{
  csShadowFrustum* sf = (csShadowFrustum*)shadows[idx];
  sf->DecRef ();
  shadows.Delete (idx);
}

//---------------------------------------------------------------------------

csShadowFrustum::csShadowFrustum (const csShadowFrustum& orig)
	: csFrustum ((const csFrustum&)orig)
{
  this->userData = orig.userData;
  this->relevant = orig.relevant;
}

//---------------------------------------------------------------------------

csFrustum* csShadowIterator::Next ()
{
  if (!cur) { cur_shad = NULL; return NULL; }
  csShadowFrustum* s;
  if (i >= 0 && i < cur_num)
    s = (csShadowFrustum*)cur->GetShadow (i);
  else
    s = NULL;
  i += dir;
  if (i < 0 || i >= cur_num)
  {
    if (onlycur) cur = NULL;
    else if (dir == 1) cur = cur->next;
    else cur = cur->prev;
    if (cur) cur_num = cur->GetNumShadows ();
    if (dir == 1) i = 0;
    else i = cur_num-1;
  }
  cur_shad = s;
  return s;
}

csShadowBlock* csShadowIterator::GetCurrentShadowBlock ()
{
  if (dir == -1)
  {
    if (i < cur_num-1) return cur;
    else if (onlycur || !cur->next) return NULL;
    else return cur->next;
  }
  else
  {
    if (i > 0) return cur;
    else if (onlycur || !cur->prev) return NULL;
    else return cur->prev;
  }
}

void csShadowIterator::DeleteCurrent ()
{
  if (dir == -1)
  {
    if (i < cur_num-1)
    {
      // Delete the previous element in the current list.
      cur->UnlinkShadow (i+1);
      cur_num--;
    }
    else if (onlycur || !cur || !cur->next)
    {
      // We are at the very first element of the iterator. Nothing to do.
      return;
    }
    else
    {
      // We are the first element of this list (last since we do reverse)
      // so we delete the last element (first) of the previous (next) list.
      cur->next->UnlinkShadow (0);
    }
  }
  else
  {
    if (i > 0)
    {
      // Delete the previous element in the current list.
      i--;
      cur->UnlinkShadow (i);
      cur_num--;
    }
    else if (onlycur || !cur || !cur->prev)
    {
      // We are at the very first element of the iterator. Nothing to do.
      return;
    }
    else
    {
      // We are the first element of this list so we delete the last
      // element of the previous list.
      cur->prev->UnlinkShadow (cur->prev->GetNumShadows ()-1);
    }
  }
}

//---------------------------------------------------------------------------

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
  if (ctxt && ctxt->icamera) ctxt->icamera->DecRef ();
  if (iengine) iengine->DecRef ();
  delete ctxt;
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
  float inv_aspect = poly.inv_aspect;

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
    bool& do_clip)
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
  
  // Test if we need and should clip to the current portal.
  int box_class;
  box_class = ctxt->iview->ClassifyBox (sbox);
  if (box_class == -1) return false; // Not visible.
  do_clip = false;
  if (ctxt->do_clip_plane || ctxt->do_clip_frustum)
  {
    if (box_class == 0) do_clip = true;
  }

  // If we don't need to clip to the current portal then we
  // test if we need to clip to the top-level portal.
  // Top-level clipping is always required unless we are totally
  // within the top-level frustum.
  // IF it is decided that we need to clip here then we still
  // clip to the inner portal. We have to do clipping anyway so
  // why not do it to the smallest possible clip area.
  if (!do_clip)
  {
    box_class = iengine->GetTopLevelClipper ()->ClassifyBox (sbox);
    if (box_class == 0) do_clip = true;
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
}

void csRenderView::RestoreRenderContext (csRenderContext* original)
{
  csRenderContext* old_ctxt = ctxt;
  ctxt = original;

  if (old_ctxt->icamera) old_ctxt->icamera->DecRef ();
  if (old_ctxt->iview) old_ctxt->iview->DecRef ();
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


