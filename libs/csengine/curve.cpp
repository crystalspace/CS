/*
    Copyright (C) 1998 by Ayal Zwi Pinkus
  
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

#include "sysdef.h"
#include "qint.h"
#include "csengine/bezier.h"
#include "csengine/curve.h"
#include "csengine/basic/polyset.h"
#include "csengine/light/light.h"
#include "csengine/polygon/polytext.h"
#include "csengine/polygon/polygon.h"
#include "csengine/world.h"

static csBezierCache theBezierCache;

CSOBJTYPE_IMPL(csCurve,csObject);
CSOBJTYPE_IMPL(csCurveTemplate,csObject);
CSOBJTYPE_IMPL(csBezier,csCurve);
CSOBJTYPE_IMPL(csBezierTemplate,csCurveTemplate);

csCurveTesselated::csCurveTesselated (int num_v, int num_t)
{
  num_vertices = num_v;
  CHK (vertices = new csCurveVertex[num_v]);
  num_triangles = num_t;
  CHK (triangles = new csCurveTriangle[num_t]);
}

csCurveTesselated::~csCurveTesselated ()
{
  CHK (delete[] vertices);
  CHK (delete[] triangles);
}

csCurveTesselated* csBezier::Tesselate (int res)
{
  if (res<2)
    res=2;
  else if (res>9)
    res=9;
  
  if (res == previous_resolution && previous_tesselation)
    return previous_tesselation;

  previous_resolution = res;
  CHK(delete previous_tesselation);

  CHK( previous_tesselation = 
       new csCurveTesselated ((res+1)*(res+1), 2*res*res));

  TDtDouble *controls[9] = 
  {
    cpt[0], cpt[1], cpt[2], cpt[3], cpt[4],
    cpt[5], cpt[6], cpt[7], cpt[8],
  };

  int i,j;

// Remove this code. Handled by set_control_point
/*
  for (i=0;i<3;i++)
    for (j=0;j<3;j++)
      {
	      controls[i*3+j][0] = points[i][j].x;
	      controls[i*3+j][1] = points[i][j].y;
	      controls[i*3+j][2] = points[i][j].z;

	      controls[i*3+j][3] = texture_coords[i][j].x; 
	      controls[i*3+j][4] = texture_coords[i][j].y; 
      }
*/

  for (i=0;i<=res;i++)
  for (j=0;j<=res;j++)
  {
    TDtDouble point[5];
    BezierPoint(point, controls, i, j,res,BinomiumMap());
    csCurveVertex& vtx = previous_tesselation->GetVertex(i+(res+1)*j);
    vtx.object_coord.x = point[0];
    vtx.object_coord.y = point[1];
    vtx.object_coord.z = point[2];
    //
    vtx.txt_coord.x    = point[3];
    vtx.txt_coord.y    = point[4];
    //
    vtx.control.x      = ((float)i)/(float)res;
    vtx.control.y      = ((float)j)/(float)res;
  }

  for (i=0;i<res;i++)
  {
    for (j=0;j<res;j++)
    {
      csCurveTriangle& up = 
	previous_tesselation->GetTriangle (2*(i+j*res));
      csCurveTriangle& down = 
	previous_tesselation->GetTriangle (2*(i+j*res)+1);
      int tl = i+(res+1)*j;
      int tr = i+(res+1)*j+1;

      int bl = i+(res+1)*(j+1);
      int br = i+(res+1)*(j+1)+1;
      up.i1 = tl;
      up.i2 = br;
      up.i3 = tr;

      down.i1 = br;
      down.i2 = tl;
      down.i3 = bl;
    }
  }

  return previous_tesselation;
}

csCurve::~csCurve ()
{
  CHK (delete lightmap);
}

// Default IsLightable returns false, because we don't know how to calculate
// x,y,z and normals for the curve by default
bool csCurve::IsLightable()
{
  return false;
}

// Default PosInSpace does nothing
void csCurve::PosInSpace (csVector3& vec, double u, double v)
{
  return;
}

// Default Normal does nothing
void csCurve::Normal (csVector3& vec, double u, double v)
{
  return;
}

void csCurve::InitLightmaps (csPolygonSet* owner, bool do_cache, int index)
{
  if (!IsLightable ()) return;
  CHK (lightmap = new csLightMap ());
  lightmap->Alloc (256, 256, 16, NULL);
  if (!do_cache) { lightmap_up_to_date = false; return; }
  if (csPolygon3D::do_force_recalc) lightmap_up_to_date = false;
  else if (!lightmap->ReadFromCache (256, 256, 16, owner, NULL, index, csWorld::current_world))
    lightmap_up_to_date = true;
  else lightmap_up_to_date = true;
}

void csCurve::ShineLightmaps (csLightView& lview)
{
  if (!lightmap) return;
  if (lightmap_up_to_date) return;
  int lmw, lmh;
  lmw = lightmap->GetWidth ()-2;
  lmh = lightmap->GetHeight ()-2;
  csRGBLightMap& map = lightmap->GetStaticMap ();
  UByte* mapR = map.mapR;
  UByte* mapG = map.mapG;
  UByte* mapB = map.mapB;
  csStatLight* light = (csStatLight*)lview.l;

  float r200d = lview.r * NORMAL_LIGHT_LEVEL / light->GetRadius ();
  float g200d = lview.g * NORMAL_LIGHT_LEVEL / light->GetRadius ();
  float b200d = lview.b * NORMAL_LIGHT_LEVEL / light->GetRadius ();
  int l1, l2, l3;

  float cosfact = csPolyTexture::cfg_cosinus_factor;

  csVector3 pos;
  csVector3 normal;
  float d;
  float u, v;
  int ui, vi;
  int uv;
  for (ui = 0 ; ui <= lmw ; ui++)
  {
    u = ((float)ui)/(float)lmw;
    for (vi = 0 ; vi <= lmh ; vi++)
    {
      v = ((float)vi)/(float)lmh;
      uv = vi*(lmw+2)+ui;
      PosInSpace (pos, u, v);
      d = csSquaredDist::PointPoint (light->GetCenter (), pos);
      if (d >= light->GetSquaredRadius ()) continue;
      d = sqrt (d);
      Normal (normal, u, v);
      float cosinus = (pos-light->GetCenter ())*normal;
      cosinus /= d;
      cosinus += cosfact;
      if (cosinus < 0) cosinus = 0;
      else if (cosinus > 1) cosinus = 1;
      if (lview.r > 0)
      {
        l1 = mapR[uv] + QRound (cosinus * r200d*(light->GetRadius () - d));
        if (l1 > 255) l1 = 255;
        mapR[uv] = l1;
      }
      if (lview.g > 0 && mapG)
      {
        l2 = mapG[uv] + QRound (cosinus * g200d*(light->GetRadius () - d));
        if (l2 > 255) l2 = 255;
        mapG[uv] = l2;
      }
      if (lview.b > 0 && mapB)
      {
        l3 = mapB[uv] + QRound (cosinus * b200d*(light->GetRadius () - d));
        if (l3 > 255) l3 = 255;
        mapB[uv] = l3;
      }
    }
  }
}

void csCurve::CacheLightmaps (csPolygonSet* owner, int index)
{
  if (!lightmap) return;
  if (!lightmap_up_to_date)
  {
    lightmap_up_to_date = true;
    lightmap->Cache (owner, NULL, index, csWorld::current_world);
  }
  lightmap->ConvertToMixingMode ();
}




csBezier::csBezier()
  : csCurve () 
{
  int i,j;
  for (i=0;i<3;i++)
    for (j=0;j<3;j++)
      {
	texture_coords[i][j].x = (0.5*i);
	texture_coords[i][j].y = (0.5*j);
      }
  previous_tesselation = NULL;
  previous_resolution = -1;
}

csBezier::~csBezier ()
{
  CHK(delete previous_tesselation);
}

void csBezier::SetControlPoint(int index, int control_id)
{
  GetControlPoint(index) = parent->CurveVertex (control_id);
  GetTextureCoord(index) = parent->CurveTexel (control_id);
	cpt[index][0] = GetControlPoint(index).x;
	cpt[index][1] = GetControlPoint(index).y;
	cpt[index][2] = GetControlPoint(index).z;
	cpt[index][3] = GetTextureCoord(index).x;
	cpt[index][4] = GetTextureCoord(index).y;
}


bool csBezier::IsLightable ()
{
  return true;
}
void csBezier::PosInSpace (csVector3& vec, double u, double v)
{
  TDtDouble point[3];
  TDtDouble *controls[9] = 
  {
    cpt[0], cpt[1], cpt[2], cpt[3], cpt[4],
    cpt[5], cpt[6], cpt[7], cpt[8],
  };
  BezierPoint(point, controls,  u, v,bfact );
  vec.x = point[0];
  vec.y = point[1];
  vec.z = point[2];
}

void csBezier::Normal (csVector3& vec, double u, double v)
{
  TDtDouble point[3];
  TDtDouble *controls[9] = 
  {
    cpt[0], cpt[1], cpt[2], cpt[3], cpt[4],
    cpt[5], cpt[6], cpt[7], cpt[8],
  };
  BezierNormal(point, controls, u, v);
  vec.x = point[0];
  vec.y = point[1];
  vec.z = point[2];
}


csBezierTemplate::csBezierTemplate ()
  : csCurveTemplate () 
{
  parent = NULL;
  int i;
  for (i=0;i<9;i++)
    {
      ver_id[i]=0;
    }
}

void csBezierTemplate::SetVertex (int index, int ver_ind)
{
  ver_id[index]=ver_ind;
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
  CHK (csBezier* p = new csBezier ());
  p->SetTextureHandle (cstxt);
  return p;
}


