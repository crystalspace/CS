/*
    Copyright (C) 1998 by Ayal Zwi Pinkus
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
#include "qint.h"
#include "qsqrt.h"
#include "csengine/bezier.h"
#include "csengine/curve.h"
#include "csengine/light.h"
#include "csengine/polytext.h"
#include "csengine/polygon.h"
#include "csengine/thing.h"
#include "csengine/engine.h"
#include "csengine/lppool.h"

unsigned long csCurve::last_curve_id = 0;

// Initialize the cache for Bezier curves
static csBezier2 bezierCache;

IMPLEMENT_CSOBJTYPE (csCurve,csObject);
IMPLEMENT_CSOBJTYPE (csCurveTemplate,csPObject);
IMPLEMENT_CSOBJTYPE (csBezierCurve,csCurve);
IMPLEMENT_CSOBJTYPE (csBezierTemplate,csCurveTemplate);

IMPLEMENT_IBASE_EXT (csCurve)
  IMPLEMENTS_EMBEDDED_INTERFACE (iCurve)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csCurve::Curve)
  IMPLEMENTS_INTERFACE (iCurve)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_IBASE_EXT (csCurveTemplate)
  IMPLEMENTS_EMBEDDED_INTERFACE (iCurveTemplate)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csCurveTemplate::CurveTemplate)
  IMPLEMENTS_INTERFACE (iCurveTemplate)
IMPLEMENT_EMBEDDED_IBASE_END

csCurveTesselated::csCurveTesselated (int num_v, int num_t)
{
  num_vertices = num_v;
  object_coords = new csVector3[num_v];
  txt_coords = new csVector2[num_v];
  controls = new csVector2[num_v];
  colors = new csColor[num_v];
  num_triangles = num_t;
  triangles = new csTriangle[num_t];
  colors_valid = false;
}

csCurveTesselated::~csCurveTesselated ()
{
  delete[] object_coords;
  delete[] txt_coords;
  delete[] controls;
  delete[] colors;
  delete[] triangles;
}

void csCurveTesselated::UpdateColors (csLightMap* lightmap)
{
  csRGBLightMap& map = lightmap->GetRealMap ();
  UByte* mapR = map.GetRed ();
  UByte* mapG = map.GetGreen ();
  UByte* mapB = map.GetBlue ();
  int lm_width = lightmap->GetWidth ();
  int lm_height = lightmap->GetWidth ();

  int j;
  for (j = 0 ; j < GetNumTriangles () ; j++)
  {
    csTriangle& ct = triangles[j];
    int lm_idx;
    int cx, cy;
    cx = QInt (controls[ct.a].x * (lm_width - 1));
    cy = QInt (controls[ct.a].y * (lm_height - 1));
    lm_idx = cy*lm_width + cx;
    colors[ct.a].red = ((float)mapR[lm_idx])/256.;
    colors[ct.a].green = ((float)mapG[lm_idx])/256.;
    colors[ct.a].blue = ((float)mapB[lm_idx])/256.;
    cx = QInt (controls[ct.b].x * (lm_width - 1));
    cy = QInt (controls[ct.b].y * (lm_height - 1));
    lm_idx = cy*lm_width + cx;
    colors[ct.b].red = ((float)mapR[lm_idx])/256.;
    colors[ct.b].green = ((float)mapG[lm_idx])/256.;
    colors[ct.b].blue = ((float)mapB[lm_idx])/256.;
    cx = QInt (controls[ct.c].x * (lm_width - 1));
    cy = QInt (controls[ct.c].y * (lm_height - 1));
    lm_idx = cy*lm_width + cx;
    colors[ct.c].red = ((float)mapR[lm_idx])/256.;
    colors[ct.c].green = ((float)mapG[lm_idx])/256.;
    colors[ct.c].blue = ((float)mapB[lm_idx])/256.;
  }

  colors_valid = true;
}


csCurveTesselated* csBezierCurve::Tesselate (int res)
{
  if (res<2)
    res=2;
  else if (res>9)
    res=9;
  
  if (res == previous_resolution && previous_tesselation)
    return previous_tesselation;

  previous_resolution = res;
  delete previous_tesselation;

  previous_tesselation = 
       new csCurveTesselated ((res+1)*(res+1), 2*res*res);

  double *controls[9] = 
  {
    cpt[0], cpt[1], cpt[2], cpt[3], cpt[4],
    cpt[5], cpt[6], cpt[7], cpt[8],
  };

  int i,j;

  for (i=0;i<=res;i++)
  for (j=0;j<=res;j++)
  {
    int idx = i+(res+1)*j;

    csVector3* vtx_coord = previous_tesselation->GetVertices ()+idx;
    csVector2* vtx_txtcoord = previous_tesselation->GetTxtCoords ()+idx;
    csVector2* vtx_control = previous_tesselation->GetControlPoints ()+idx;
    *vtx_coord = csBezier2::GetPoint (controls, i, j, res);
    //
    *vtx_txtcoord = csBezier2::GetTextureCoord (controls, i, j, res);
    //
    vtx_control->x = ((float)i)/(float)res;
    vtx_control->y = ((float)j)/(float)res;
  }

  for (i=0;i<res;i++)
  {
    for (j=0;j<res;j++)
    {
      csTriangle& up = previous_tesselation->GetTriangle (2*(i+j*res));
      csTriangle& down = previous_tesselation->GetTriangle (2*(i+j*res)+1);
      
      int tl = i+(res+1)*j;
      int tr = i+(res+1)*j+1;

      int bl = i+(res+1)*(j+1);
      int br = i+(res+1)*(j+1)+1;
      up.a = tl;
      up.b = br;
      up.c = tr;

      down.a = br;
      down.b = tl;
      down.c = bl;
    }
  }

  return previous_tesselation;
}

void csCurve::GetCameraBoundingBox (const csTransform& obj2cam, csBox3& cbox)
{
  csBox3 box;
  GetObjectBoundingBox (box);
  cbox.StartBoundingBox (obj2cam * box.GetCorner (0));
  cbox.AddBoundingVertexSmart (obj2cam * box.GetCorner (1));
  cbox.AddBoundingVertexSmart (obj2cam * box.GetCorner (2));
  cbox.AddBoundingVertexSmart (obj2cam * box.GetCorner (3));
  cbox.AddBoundingVertexSmart (obj2cam * box.GetCorner (4));
  cbox.AddBoundingVertexSmart (obj2cam * box.GetCorner (5));
  cbox.AddBoundingVertexSmart (obj2cam * box.GetCorner (6));
  cbox.AddBoundingVertexSmart (obj2cam * box.GetCorner (7));
}

/// Calculate inverse perspective corrected point for this camera.
static void Perspective (const csVector3& v, csVector2& p,
	float aspect, float shift_x, float shift_y)
{
  float iz = aspect / v.z;
  p.x = v.x * iz + shift_x;
  p.y = v.y * iz + shift_y;
}

float csCurve::GetScreenBoundingBox (const csTransform& obj2cam,
	iCamera* camera,
	csBox2& boundingBox)
{
  csVector2   oneCorner;
  csBox3      cbox;

  float aspect = camera->GetFOV ();
  float shift_x = camera->GetShiftX ();
  float shift_y = camera->GetShiftY ();

  // @@@ Note. The bounding box created by this function greatly
  // exagerates the real bounding box. However, this function
  // needs to be fast. I'm not sure how to do this more accuratelly.

  GetCameraBoundingBox (obj2cam, cbox);

  // if the entire bounding box is behind the camera, we're done
  if ((cbox.MinZ () < 0) && (cbox.MaxZ () < 0))
    return -1;

  // Transform from camera to screen space.
  if (cbox.MinZ () <= 0)
  {
    // Sprite is very close to camera.
    // Just return a maximum bounding box.
    boundingBox.Set (-10000, -10000, 10000, 10000);
  }
  else
  {
    Perspective (cbox.Max (), oneCorner, aspect, shift_x, shift_y);
    boundingBox.StartBoundingBox (oneCorner);
    csVector3 v (cbox.MinX (), cbox.MinY (), cbox.MaxZ ());
    Perspective (v, oneCorner, aspect, shift_x, shift_y);
    boundingBox.AddBoundingVertexSmart (oneCorner);
    Perspective (cbox.Min (), oneCorner, aspect, shift_x, shift_y);
    boundingBox.AddBoundingVertexSmart (oneCorner);
    v.Set (cbox.MaxX (), cbox.MaxY (), cbox.MinZ ());
    Perspective (v, oneCorner, aspect, shift_x, shift_y);
    boundingBox.AddBoundingVertexSmart (oneCorner);
  }

  return cbox.MaxZ ();
}

void csBezierCurve::GetObjectBoundingBox (csBox3& bbox)
{
  // @@@ This algo uses the control points to compute
  // the bounding box. Is this right?
  if (!valid_bbox)
  {
    valid_bbox = true;
    object_bbox.StartBoundingBox ();
    int i,j;
    for (i=0 ; i<3 ; i++)
      for (j=0 ; j<3 ; j++)
        object_bbox.AddBoundingVertex (points[i][j]);
  }
  bbox = object_bbox;
}

csCurve::~csCurve ()
{
  while (lightpatches)
    csEngine::current_engine->lightpatch_pool->Free (lightpatches);
  delete _o2w;
  delete lightmap;
  delete[] _uv2World;
  delete[] _uv2Normal;
}

// Default IsLightable returns false, because we don't know how to calculate
// x,y,z and normals for the curve by default
bool csCurve::IsLightable()
{
  return false;
}

// Default PosInSpace does nothing
void csCurve::PosInSpace (csVector3& /*vec*/, double /*u*/, double /*v*/)
{
  return;
}

// Default Normal does nothing
void csCurve::Normal (csVector3& /*vec*/, double /*u*/, double /*v*/)
{
  return;
}

// #define CURVE_LM_SIZE 32
#define CURVE_LM_SIZE (8 - 2) /*this is the real value - 2*/

void csCurve::InitLightMaps (bool do_cache)
{
  if (!IsLightable ()) return;
  lightmap = new csLightMap ();

  // Allocate space for the lightmap and initialize it to ambient color.
  int r, g, b;
  r = csLight::ambient_red;
  g = csLight::ambient_green;
  b = csLight::ambient_blue;
  lightmap->Alloc (CURVE_LM_SIZE*csLightMap::lightcell_size, 
                   CURVE_LM_SIZE*csLightMap::lightcell_size, r, g, b);

  if (!do_cache) { lightmap_up_to_date = false; return; }
  if (csEngine::do_force_relight) lightmap_up_to_date = false;
  else 
    if (!lightmap->ReadFromCache ( CURVE_LM_SIZE*csLightMap::lightcell_size,
           CURVE_LM_SIZE*csLightMap::lightcell_size, this, false, 
           csEngine::current_engine))
    lightmap_up_to_date = true;
  else lightmap_up_to_date = true;
}

void csCurve::MakeDirtyDynamicLights () 
{ 
  lightmap_up_to_date = false;
  lightmap->MakeDirtyDynamicLights (); 
};


void csCurve::AddLightPatch(csLightPatch* lp)
{
  lp->next_poly = lightpatches;
  lp->prev_poly = NULL;

  if (lightpatches) 
    lightpatches->prev_poly = lp;
  
  lightpatches = lp;
  lp->polygon = NULL;
  lp->curve = this;

  /// set the dynamic lights to dirty
  lightmap->MakeDirtyDynamicLights ();
}

void csCurve::UnlinkLightPatch (csLightPatch* lp)
{
  if (lp->next_poly) lp->next_poly->prev_poly = lp->prev_poly;
  if (lp->prev_poly) lp->prev_poly->next_poly = lp->next_poly;
  else lightpatches = lp->next_poly;
  lp->prev_poly = lp->next_poly = NULL;
  lp->curve = NULL;
  lightmap->MakeDirtyDynamicLights ();
}

bool csCurve::RecalculateDynamicLights ()
{
  // first combine the static and pseudo-dynamic lights
  if (!lightmap || !lightmap->UpdateRealLightMap() )
    return false;

  //---
  // Now add all dynamic lights.
  //---
  csLightPatch* lp = lightpatches;
  while (lp)
  {
    ShineDynLight (lp);
    lp = lp->GetNextPoly ();
  }

  return true;
}

void csCurve::ShineDynLight (csLightPatch* lp)
{
  CS_ASSERT(_o2w);
  CS_ASSERT(_uv2World);
  CS_ASSERT(_uv2Normal);

  int lm_width = lightmap->GetWidth ();
  int lm_height = lightmap->GetHeight ();

  csDynLight *light = lp->light;

  iShadowIterator* shadow_it = lp->shadows.GetShadowIterator ();
  bool has_shadows = shadow_it->HasNext ();

  csColor color = light->GetColor() * NORMAL_LIGHT_LEVEL;

  UByte *mapR = lightmap->GetRealMap().GetRed();
  UByte *mapG = lightmap->GetRealMap().GetGreen();
  UByte *mapB = lightmap->GetRealMap().GetBlue();

  int lval;
  float cosfact = csPolyTexture::cfg_cosinus_factor;

  // now add to the map
  csVector3 pos;
  csVector3 normal;
  float d;
  int ui, vi;
  int uv;
  for (ui = 0 ; ui < lm_width; ui++)
  {
    for (vi = 0 ; vi < lm_height; vi++)
    {
      uv = vi*lm_width + ui;

      pos = _uv2World[uv];

      // is the point contained within the light frustrum? 
      if (!lp->light_frustum->Contains(pos - lp->light_frustum->GetOrigin()))
        // No, skip it
        continue;

      // if we have any shadow frustrumsq
      if (has_shadows)
      {
        shadow_it->Reset ();
	bool shad = false;
	while (shadow_it->HasNext ())
	{
          csFrustum* csf = shadow_it->Next ();
          // is this point in shadow
          if (csf->Contains(pos - csf->GetOrigin()))
	  {
	    shad = true;
            break;
	  }
        }
                  
        // if it was found in shadow skip it
        if (shad)
          continue;
      }

      d = csSquaredDist::PointPoint (light->GetCenter (), pos);
      if (d >= light->GetSquaredRadius ()) continue;
      d = qsqrt (d);
      normal = _uv2Normal[uv];
      float cosinus = (pos-light->GetCenter ())*normal;
      cosinus /= d;
      cosinus += cosfact;
      if (cosinus < 0) cosinus = 0;
      else if (cosinus > 1) cosinus = 1;

      float brightness = cosinus * light->GetBrightnessAtDistance (d);

      if (color.red > 0)
      {
        lval = mapR[uv] + QRound (color.red * brightness);
        if (lval > 255) lval = 255;
        mapR[uv] = lval;
      }
      if (color.green > 0)
      {
        lval = mapG[uv] + QRound (color.green * brightness);
        if (lval > 255) lval = 255;
        mapG[uv] = lval;
      }
      if (color.blue > 0)
      {
        lval = mapB[uv] + QRound (color.blue * brightness);
        if (lval > 255) lval = 255;
        mapB[uv] = lval;
      }
    }
  }

  shadow_it->DecRef ();
}

void csCurve::GetCoverageMatrix (csFrustumView& lview, 
                                 csCoverageMatrix &cm) const
{
  csVector3 pos;
  int uv;

  iShadowIterator* shadow_it = lview.GetFrustumContext ()->
  	GetShadows ()->GetShadowIterator ();
  bool has_shadows = shadow_it->HasNext ();
  
  int lm_width = lightmap->GetWidth ();
  int lm_height = lightmap->GetHeight ();

  cm.height = lm_height;
  cm.width = lm_width;

  for (int ui = 0 ; ui < lm_width; ui++)
  {
    for (int vi = 0 ; vi < lm_height; vi++)
    {
      uv = vi*lm_width + ui;
      pos = _uv2World[uv];

      csFrustumContext* ctxt = lview.GetFrustumContext ();
      // is the point contained within the light frustrum? 
      if (!ctxt->GetLightFrustum ()->Contains (pos-
      		ctxt->GetLightFrustum ()->GetOrigin ()))
        // No, skip it
        continue;

      // if we have any shadow frustrums
      if (has_shadows)
      {
        csFrustum* csf = NULL;
	shadow_it->Reset ();
	while (shadow_it->HasNext ())
	{
	  csf = shadow_it->Next ();
          // is this point in shadow
          if (csf->Contains(pos - csf->GetOrigin()))
            break;
        }
              
        // if it was found in shadow skip it
        if (csf != NULL)
          continue;
      }

      cm.coverage[uv] = 1.0;
    }
  }

  shadow_it->DecRef ();
}

void csCurve::CalculateLighting (csFrustumView& lview)
{
  csLightingInfo& linfo = lview.GetFrustumContext ()->GetLightingInfo ();
  if (lview.IsDynamic ())
  {
    // We are working for a dynamic light. In this case we create
    // a light patch for this polygon.
    csLightPatch* lp = csEngine::current_engine->lightpatch_pool->Alloc ();

    AddLightPatch (lp);
  
    csDynLight* dl = (csDynLight*)lview.GetUserData ();
    dl->AddLightpatch (lp);

    // This light patch has exactly 4 vertices because it fits around our
    // lightmap
    lp->Initialize (4);

    // Copy shadow frustums.
    lp->shadows.DeleteShadows ();
    // @@@: It would be nice if we could optimize earlier 
    // to determine relevant shadow frustums in curves and use
    // AddRelevantShadows instead.
    lp->shadows.AddAllShadows (lview.GetFrustumContext ()->GetShadows ());

    lp->light_frustum = new csFrustum(*lview.GetFrustumContext ()->
    	GetLightFrustum ());

    MakeDirtyDynamicLights ();
  }
  else
  {
    // if our UV buffers have not been intialized, create them
    if (!_uv2World)
    {
      CalcUVBuffers();
    }

    if (!lightmap || lightmap_up_to_date) 
      return;

    int lm_width = lightmap->GetWidth ();
    int lm_height = lightmap->GetHeight ();

    csStatLight *light = (csStatLight *)lview.GetUserData ();

    bool dyn = light->IsDynamic();

    UByte *mapR, *mapG, *mapB;
    csShadowMap* smap;

    /* initialize color to something to avoid compiler warnings */
    csColor color(0,0,0);

    if (dyn)
    {
      smap = lightmap->FindShadowMap( light );
      if (!smap)
      {
        smap = lightmap->NewShadowMap(light, CURVE_LM_SIZE, CURVE_LM_SIZE );
      }
  
      mapR = smap->map;
      mapG = NULL;
      mapB = NULL;
    }
    else
    {
      mapR = lightmap->GetStaticMap ().GetRed ();
      mapG = lightmap->GetStaticMap ().GetGreen ();
      mapB = lightmap->GetStaticMap ().GetBlue ();
      color = linfo.GetColor () * NORMAL_LIGHT_LEVEL;
    }

    int lval;

    float cosfact = csPolyTexture::cfg_cosinus_factor;

    // get our coverage matrix
    csCoverageMatrix *shadow_matrix = new csCoverageMatrix(lm_width,lm_height);
    GetCoverageMatrix(lview, *shadow_matrix);

    // calculate the static lightmap
    csVector3 pos;
    csVector3 normal;
    float d;
    int ui, vi;
    int uv;
    for (ui = 0 ; ui < lm_width; ui++)
    {
      for (vi = 0 ; vi < lm_height; vi++)
      {
        uv = vi*lm_width + ui;

        // is the point lit by our current light
        if (shadow_matrix->coverage[uv] <= SMALL_EPSILON)
          // No, go to next point then
          continue;

        // what are the world coordinates of this texel
        pos = _uv2World[uv];

        d = csSquaredDist::PointPoint (light->GetCenter (), pos);
        if (d >= light->GetSquaredRadius ()) continue;
        d = qsqrt (d);

        normal = _uv2Normal[uv];
        float cosinus = (pos-light->GetCenter ())*normal;
        cosinus /= d;
        cosinus += cosfact;

        if (cosinus < 0) 
          cosinus = 0;
        else if (cosinus > 1) 
          cosinus = 1;

        float brightness = cosinus * light->GetBrightnessAtDistance (d);

        if (dyn)
        {
          lval = mapR[uv] + QRound (NORMAL_LIGHT_LEVEL * brightness);
          if (lval > 255) lval = 255;
          mapR[uv] = lval;
        }
        else
        {
	  csColor& col = linfo.GetColor ();
          if (col.red > 0)
          {
            lval = mapR[uv] + QRound (color.red * brightness);
            if (lval > 255) lval = 255;
            mapR[uv] = lval;
          }
          if (col.green > 0 && mapG)
          {
            lval = mapG[uv] + QRound (color.green * brightness);
            if (lval > 255) lval = 255;
            mapG[uv] = lval;
          }
          if (col.blue > 0 && mapB)
          {
            lval = mapB[uv] + QRound (color.blue * brightness);
            if (lval > 255) lval = 255;
            mapB[uv] = lval;
          }
        }
      }
    }

    delete shadow_matrix;
  }
}

void csCurve::HardTransform (const csReversibleTransform& trans)
{
  return;//@@@
  if (_uv2World)
  {
    int lm_width = lightmap->GetWidth ();
    int lm_height = lightmap->GetHeight ();
    int uv;
    // transform our uv2World buffer
    for (int ui=0; ui < lm_width; ui++)
    {
      for (int vi=0; vi < lm_height; vi++)
      {
        uv = vi*lm_width + ui;

        _uv2World[uv] = trans.Other2This (_uv2World[uv]);
      }
    }
  }
}

void csCurve::SetObject2World (csReversibleTransform* o2w) 
{ 
  if (!lightmap) return;	// Return if there is no lightmap yet.
  int lm_width = lightmap->GetWidth ();
  int lm_height = lightmap->GetHeight ();

  // current index into buffers
  int uv;
  
  // if there was already an object to world transform specified
  if (_o2w && _uv2World) 
  {
    // untransform our buffers
    for(int ui=0; ui < lm_width; ui++)
    {
      for(int vi=0; vi < lm_height; vi++)
      {
        uv = vi*lm_width + ui;

        _uv2World[uv] = _o2w->This2Other(_uv2World[uv]);
      }
    }
    
    delete _o2w; 
  }
  
  // intialize the new transform
  _o2w = new csReversibleTransform(*o2w);

  if (_uv2World)
  {
    // transform our uv2World buffer
    for(int ui=0; ui < lm_width; ui++)
    {
      for(int vi=0; vi < lm_height; vi++)
      {
        uv = vi*lm_width + ui;

        _uv2World[uv] = _o2w->Other2This(_uv2World[uv]);
      }
    }
  }
}

void csCurve::CacheLightMaps ()
{
  if (!lightmap) return;
  if (!lightmap_up_to_date)
  {
    lightmap_up_to_date = true;
    lightmap->Cache (NULL, this, csEngine::current_engine);
  }
  lightmap->ConvertToMixingMode ();
}

float csCurve::GetArea()
{
  float area = 0.0;

  // Tesselate at resolution 10000 to get the best approximation of area
  csCurveTesselated* ct = Tesselate( 10000 );

  csVector3 *vertex = ct->GetVertices ();
  csTriangle t;

  // loop through all of our triangles and sum thier areas
  for(int i=0; i<ct->GetNumTriangles(); i++)
  {
    t = ct->GetTriangle(i);
    area += ABS(csMath3::Area3 (vertex[t.a], vertex[t.b], vertex[t.c]) );
  }

  return area / 2.0;
}

void csCurve::CalcUVBuffers()
{
  if (_uv2World)
    delete[] _uv2World;
  if (_uv2Normal)
    delete[] _uv2Normal;

  int lm_width = lightmap->GetWidth ();
  int lm_height = lightmap->GetHeight ();

  _uv2World = new csVector3[lm_width * lm_height];
  _uv2Normal = new csVector3[lm_width * lm_height];

  // our texture space coordinates
  float u,v;
  int uv;

  // now loop over every texel in the lightmap
  for (int ui = 0 ; ui < lm_width; ui++)
  {
    // calculate the real u coordinate in texture space
    u = ((float)ui + 0.5F)/(float)lm_width; // offset 0.5 for texel center
    for (int vi = 0 ; vi < lm_height; vi++)
    {
      uv = vi*lm_width + ui;

      // calculate the real v coordinate in texture space
      v = ((float)vi + 0.5F)/(float)lm_height;// offset 0.5 for texel center

      // ask our curve to find the object space coordinate at u,v
      PosInSpace (_uv2World[uv], u, v);
      
      // ask our curve to find the normal at u,v
      Normal (_uv2Normal[uv], u, v);

      // if we have an object to world transformation, use it
      if (_o2w)
      {
        _uv2World[uv] = _o2w->Other2This(_uv2World[uv]);
      }
    }
  }
}

csBezierCurve::csBezierCurve (csBezierTemplate* parent_tmpl) :
  csCurve (parent_tmpl) 
{
  int i,j;
  for (i=0 ; i<3 ; i++)
    for (j=0 ; j<3 ; j++)
    {
      texture_coords[i][j].x = (0.5*i);
      texture_coords[i][j].y = (0.5*j);
    }
  previous_tesselation = NULL;
  previous_resolution = -1;
  valid_bbox = false;
}

csBezierCurve::~csBezierCurve ()
{
  delete previous_tesselation;
}

void csBezierCurve::SetControlPoint (int index, int control_id)
{
  GetControlPoint(index) = parent->CurveVertex (control_id);
  GetTextureCoord(index) = parent->CurveTexel (control_id);
  cpt[index][0] = GetControlPoint(index).x;
  cpt[index][1] = GetControlPoint(index).y;
  cpt[index][2] = GetControlPoint(index).z;
  cpt[index][3] = GetTextureCoord(index).x;
  cpt[index][4] = GetTextureCoord(index).y;
}


bool csBezierCurve::IsLightable ()
{
  return true;
}
void csBezierCurve::PosInSpace (csVector3& vec, double u, double v)
{
  double *controls[9] = 
  {
    cpt[0], cpt[1], cpt[2], cpt[3], cpt[4],
    cpt[5], cpt[6], cpt[7], cpt[8],
  };

  vec = csBezier2::GetPoint (controls,  u, v);
}

void csBezierCurve::Normal (csVector3& vec, double u, double v)
{
  double *controls[9] = 
  {
    cpt[0], cpt[1], cpt[2], cpt[3], cpt[4],
    cpt[5], cpt[6], cpt[7], cpt[8],
  };
  vec = csBezier2::GetNormal (controls, u, v);
}

void csBezierCurve::HardTransform (const csReversibleTransform& trans)
{
  csCurve::HardTransform (trans);
  valid_bbox = false;
  int i, j;
  for (i = 0 ; i < 3 ; i++)
    for (j = 0 ; j < 3 ; j++)
      points[i][j] = trans.This2Other (points[i][j]);
}

//------------------------------------------------------------------

csCurve::csCurve (csCurveTemplate* parent_tmpl) : csObject (),
    cstxt (NULL), parent_template (parent_tmpl), lightpatches (NULL), 
    _o2w (NULL), _uv2World (NULL), _uv2Normal (NULL), parent (NULL),
    lightmap (NULL), lightmap_up_to_date (false)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiCurve);
  curve_id = last_curve_id++;
} 

iCurveTemplate* csCurve::Curve::GetParentTemplate ()
{
  return &(scfParent->GetParentTemplate ()->scfiCurveTemplate);
}

void csCurve::Curve::SetMaterial (iMaterialWrapper* mat)
{
  scfParent->SetMaterialWrapper (mat->GetPrivateObject ());
}

iMaterialWrapper* csCurve::Curve::GetMaterial ()
{
  return &(scfParent->GetMaterialWrapper ()->scfiMaterialWrapper);
}

csCurveTemplate::csCurveTemplate () : csPObject ()
{
  CONSTRUCT_EMBEDDED_IBASE (scfiCurveTemplate);
  csEngine::current_engine->AddToCurrentRegion (this);
}

void csCurveTemplate::CurveTemplate::SetMaterial (iMaterialWrapper* mat)
{
  scfParent->SetMaterialWrapper (mat->GetPrivateObject ());
}

iMaterialWrapper* csCurveTemplate::CurveTemplate::GetMaterial ()
{
  return &(scfParent->GetMaterialWrapper ()->scfiMaterialWrapper);
}

iCurve* csCurveTemplate::CurveTemplate::MakeCurve ()
{
  csCurve* curve = scfParent->MakeCurve ();
  return &(curve->scfiCurve);
}

//------------------------------------------------------------------

csBezierTemplate::csBezierTemplate ()
  : csCurveTemplate () 
{
  int i;
  for (i=0 ; i<9 ; i++)
    ver_id[i] = 0;
}

void csBezierTemplate::SetVertex (int index, int ver_ind)
{
  ver_id[index] = ver_ind;
}
int csBezierTemplate::GetVertex (int index) 
{
  return ver_id[index];
}

int csBezierTemplate::NumVertices ()
{
  return 9;
}

csCurve* csBezierTemplate::MakeCurve ()
{
  csBezierCurve* p = new csBezierCurve (this);
  p->SetMaterialWrapper (cstxt);
  return p;
}

