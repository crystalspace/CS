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

#include <math.h>

#include "sysdef.h"
#include "qint.h"
#include "csengine/sysitf.h"
#include "csengine/polytext.h"
#include "csengine/polyplan.h"
#include "csengine/polygon.h"
#include "csengine/light.h"
#include "csengine/sector.h"
#include "csengine/world.h"
#include "csengine/lghtmap.h"
#include "csobject/nameobj.h"
#include "igraph3d.h"
#include "itexture.h"

// Option variable: do accurate lighting of things (much slower)?
bool csPolyTexture::do_accurate_things = true;
// Option variable: cosinus factor.
float csPolyTexture::cfg_cosinus_factor = 0;

int csPolyTexture::subtex_size = DEFAULT_SUBTEX_SIZE;
bool csPolyTexture::subtex_dynlight = true;

//---------------------------------------------------------------------------

IMPLEMENT_UNKNOWN_NODELETE( csPolyTexture )

BEGIN_INTERFACE_TABLE( csPolyTexture )
  IMPLEMENTS_COMPOSITE_INTERFACE( PolygonTexture )
END_INTERFACE_TABLE()
  
csPolyTexture::csPolyTexture ()
{
  dyn_dirty = true;
  dirty_matrix = NULL;
  lm = NULL;
  tcache_data = NULL;
}

csPolyTexture::~csPolyTexture ()
{
  CHK (delete [] dirty_matrix);
}

void csPolyTexture::SetMipmapSize (int mm)
{
  mipmap_size = mm;
  switch (mipmap_size)
  {
    case 2: mipmap_shift = 1; break;
    case 4: mipmap_shift = 2; break;
    case 8: mipmap_shift = 3; break;
    case 16: mipmap_shift = 4; break;
    case 32: mipmap_shift = 5; break;
    case 64: mipmap_shift = 6; break;
    default: mipmap_size = 2; mipmap_shift = 1; break;
  }
}

#if 0
bool verbose = false;
#  define DB(x) if (verbose) CsPrintf##x
#  define DBCHECK(sector,poly) verbose = !strcmp (polygon->get_name (), poly) && !strcmp (((PolygonSet*)(polygon->get_parent ()))->get_name (), sector)
#else
#  define DB(x)
#  define DBCHECK(sector,poly)
#endif

void csPolyTexture::CreateBoundingTextureBox ()
{
  DBCHECK ("moor", "southU");
  DB ((MSG_DEBUG_0, "---------------------------------------------\n"));

  // First we compute the bounding box in 2D texture space (uv-space).
  float min_u = 1000000000.;
  float min_v = 1000000000.;
  float max_u = -1000000000.;
  float max_v = -1000000000.;

  csPolyPlane* pl = polygon->GetPlane ();

  int i;
  csVector3 v1, v2;
  DB ((MSG_DEBUG_0, "  Vertices in world-space:\n"));
  for (i = 0 ; i < polygon->GetNumVertices () ; i++)
  {
    v1 = polygon->Vwor (i);   // Coordinates of vertex in world space.
    DB ((MSG_DEBUG_0, "      %d:(%f,%f,%f)\n", i, v1.x, v1.y, v1.z));
    v1 -= pl->v_world2tex;
    v2 = (pl->m_world2tex) * v1;  // Coordinates of vertex in texture space.
    if (v2.x < min_u) min_u = v2.x;
    if (v2.x > max_u) max_u = v2.x;
    if (v2.y < min_v) min_v = v2.y;
    if (v2.y > max_v) max_v = v2.y;
  }

  DB ((MSG_DEBUG_0, "  min_u=%f max_u=%f min_v=%f max_v=%f\n", min_u, max_u, min_v, max_v));

  int ww, hh;
  txt_handle->GetMipMapDimensions (mipmap_level, ww, hh);
  Imin_u = QRound (min_u*ww);
  Imin_v = QRound (min_v*hh);
  Imax_u = QRound (max_u*ww);
  Imax_v = QRound (max_v*hh);
  
  DB ((MSG_DEBUG_0, "  ww=%d hh=%d\n", ww, hh));
  DB ((MSG_DEBUG_0, "  Imin_u=%d Imax_u=%d Imin_v=%d Imax_v=%d\n", Imin_u, Imax_u, Imin_v, Imax_v));

  // DAN: used in hardware accel drivers
  Fmin_u = min_u;
  Fmax_u = max_u;
  Fmin_v = min_v;
  Fmax_v = max_v;

  h = Imax_v-Imin_v;
  w_orig = Imax_u-Imin_u;
  w = 1;
  shf_u = 0;
  and_u = 0;
  while (true)
  {
    if (w_orig <= w) break;
    w <<= 1;
    shf_u++;
    and_u = (and_u<<1)+1;
  }

  fdu = min_u*ww;
  fdv = min_v*hh;
  du = QInt16 (fdu);
  dv = QInt16 (fdv);

  DB ((MSG_DEBUG_0, "  w_orig=%d w=%d h=%d\n", w_orig, w, h));
  DB ((MSG_DEBUG_0, "  fdu=%f fdv=%f du=%d dv=%d\n", fdu, fdv, du, dv));
  DB ((MSG_DEBUG_0, "  and_u=%d shf_u=%d\n", and_u, shf_u));

  // The size of the whole texture is extended by an upper and lower
  // margin to prevent overflow in the texture mapper.
  size = w*(H_MARGIN+H_MARGIN+h);

  DB ((MSG_DEBUG_0, "  size=%d\n", size));
}

void csPolyTexture::MakeDirtyDynamicLights ()
{
  dyn_dirty = true;
}

bool csPolyTexture::RecalcDynamicLights ()
{
  if (!dyn_dirty) return false;
  if (!lm) return false;

  dyn_dirty = false;
  bool dm = csPolyTexture::subtex_size && csPolyTexture::subtex_dynlight && dirty_matrix;

  //---
  // First copy the static lightmap to the real lightmap.
  // Remember the real lightmap first so that we can see if
  // there were any changes.
  //---
  long lm_size = lm->GetSize ();
  csRGBLightMap& stmap = lm->GetStaticMap ();
  csRGBLightMap& remap = lm->GetRealMap ();
  csRGBLightMap oldmap;

  if (dm) { oldmap.AllocRed (lm_size); memcpy (oldmap.mapR, remap.mapR, lm_size); }
  memcpy (remap.mapR, stmap.mapR, lm_size);

  if (remap.mapG)
  {
    if (dm) { oldmap.AllocGreen (lm_size); memcpy (oldmap.mapG, remap.mapG, lm_size); }
    memcpy (remap.mapG, stmap.mapG, lm_size);
  }

  if (remap.mapB)
  {
    if (dm) { oldmap.AllocBlue (lm_size); memcpy (oldmap.mapB, remap.mapB, lm_size); }
    memcpy (remap.mapB, stmap.mapB, lm_size);
  }

  //---
  // Then add all pseudo-dynamic lights.
  //---
  csLight* light;
  unsigned char* mapR, * mapG, * mapB;
  float red, green, blue;
  unsigned char* p, * last_p;
  int l, s;

  if (lm->first_smap)
  {
    csShadowMap* smap = lm->first_smap;

    //@@@
    //if (Textures::mixing != MIX_NOCOLOR)
    {
      // Color mode.
      do
      {
        mapR = remap.mapR;
        mapG = remap.mapG;
        mapB = remap.mapB;
        light = smap->light;
        red = light->GetColor ().red;
        green = light->GetColor ().green;
        blue = light->GetColor ().blue;
        csLight::CorrectForNocolor (&red, &green, &blue);
  	p = smap->map;
  	last_p = p+lm_size;
  	do
  	{
    	  s = *p++;
          l = *mapR + QRound (red * s);
    	  if (l > 255) l = 255;
          *mapR++ = l;
          l = *mapG + QRound (green * s); if (l > 255) l = 255;
    	  if (l > 255) l = 255;
    	  *mapG++ = l;
          l = *mapB + QRound (blue * s); if (l > 255) l = 255;
    	  if (l > 255) l = 255;
          *mapB++ = l;
  	}
        while (p < last_p);

        smap = smap->next;
      }
      while (smap);
    }
//@@@
#if 0
    else
    {
      // NOCOLOR mode.
      do
      {
        mapR = remap.mapR;
        light = smap->light;
        red = light->get_red ();
        green = light->get_green ();
        blue = light->get_blue ();
        csLight::mixing_dependent_strengths (&red, &green, &blue);
  	p = smap->map;
  	last_p = p+lm_size;
  	do
  	{
          l = *mapR + QRound (red * (*p++));
    	  if (l > 255) l = 255;
          *mapR++ = l;
  	}
  	while (p < last_p);

        smap = smap->next;
      }
      while (smap);
    }
#endif
  }

  //---
  // Now add all dynamic lights.
  //---
  csLightPatch* lp = polygon->GetLightpatches ();
  while (lp)
  {
    ShineDynLightmap (lp);
    lp = lp->GetNextPoly ();
  }

  if (dm)
  {
    //---
    // Now compare the old map with the new one and
    // see where there are changes. Tag the corresponding
    // sub-textures as dirty.
    //---
    int lw = lm->GetWidth ();
    int lh = lm->GetHeight ();
    int ru, rv, idx;
    int lv, lu, luv, luv_v;
    int num = csPolyTexture::subtex_size >> mipmap_shift; // Horiz/vert number of lightmap boxes in one sub-texture
    int numu, numv;
    int uu, vv;

    idx = 0;
    for (rv = 0 ; rv < dirty_h ; rv++)
    {
      lv = (rv * csPolyTexture::subtex_size) >> mipmap_shift;
      luv_v = lv * lw;
      if (lv+num >= lh) numv = lh-lv-1;
      else numv = num;

      for (ru = 0 ; ru < dirty_w ; ru++, idx++)
      {
        // If already dirty we don't need to check.
        if (!dirty_matrix[idx])
  	{
          lu = (ru * csPolyTexture::subtex_size) >> mipmap_shift;
    	  luv = luv_v + lu;
          if (lu+num >= lw-1) numu = lw-lu-1;
    	  else numu = num;
    	  // <= num here because I want to include the boundaries of the next
    	  // sub-texture in the test as well.
    	  for (vv = 0 ; vv <= numv ; vv++)
    	  {
      
      	    for (uu = 0 ; uu <= numu ; uu++)
      	    {
              // If we find a difference this sub-texture is dirty. I would like to have
              // a multi-break statement but unfortunatelly C++ does not have this. That's
              // why I use the goto. Yes I know! goto's are EVIL!
              if ((oldmap.mapR[luv] != remap.mapR[luv]) ||
                  (oldmap.mapG && (oldmap.mapG[luv] != remap.mapG[luv])) ||
                  (oldmap.mapB && (oldmap.mapB[luv] != remap.mapB[luv])))
              {
          	dirty_matrix[idx] = 1;
    		dirty_cnt++;
    		goto stop;
              }
              luv++;
            }
            luv += lw-num-1;
          }
          stop: ;
        }
      }
    }
  }

  return true;
}

void csPolyTexture::InitLightmaps ()
{
}

/*
 * Added by Denis Dmitriev for correct lightmaps shining. This code above draws perfectly (like perfect
 * texture mapping -- I mean most correctly) anti-aliased polygon on lightmap and adjusts it according
 * to the actual polygon shape on the texture
 */
#define EPS   0.0001

float calc_area (int n, csVector2 *p)
{
  float area=0;

  for (int i=0,j;i<n;i++)
  {
    if (i!=n-1)
      j = i+1;
    else
      j = 0;

    area += (p[i].y+p[j].y)*(p[i].x-p[j].x);
  }

  area /= 2.0;
  return fabs (area);
}

static int __texture_width;
static float *__texture;
static unsigned char *__mark;

struct __rect
{
  float left, right;
  float top, bottom;
};

static void (*__draw_func)(int, int, float);

FILE *fo;

static void lixel_intensity (int x, int y, float density)
{
  int addr=x+y*__texture_width;

  if (density>=1.0)
    density=1.0;

  __texture[addr] = density;
}

static void correct_results (int x, int y, float density)
{
  if (density<EPS||density>=1-EPS)
    return;

  int addr=x+y*__texture_width;
  float res=__texture[addr]/density;

  if (res>1)
    res=1;

  __texture[addr]=res;
  __mark[addr]=1;
}

/* I was interested in these values */
static int max_depth=0,depth=0;

static void poly_fill (int n, csVector2 *p2d, __rect &visible)
{
  depth++;

  if (depth>max_depth)
    max_depth=depth;

  // how_to_divide:
  //   0 -- horizontal
  //   1 -- vertical

  //@@@Note from Jorrit, I tried to use QInt() here but this didn't work?
  //@@@Answer from A.Z: there was a BIGBUG{tm} in QInt - 1.0 was rounded to 0.
  int height = QInt (visible.bottom - visible.top);
  int width = QInt (visible.right - visible.left);

  float a=calc_area (n,p2d);
  if (fabs (a-height*width)<EPS)
  {
    // this area is completely covered

    int x = QInt (visible.left), y = QInt (visible.top);
    for (int i=0 ; i<height ; i++)
      for (int j=0 ; j<width ; j++)
    __draw_func (j+x, i+y, 1);

    depth--;
    return;
  }
  else
    if (fabs (a)<EPS)
    {
      // this area is hollow
      depth--;
      return;
    }

  if (height==1&&width==1)
  {
    int x = QInt (visible.left), y = QInt (visible.top);
    __draw_func (x, y, a);

    depth--;
    return;
  }

  int sub_x = QInt (visible.left) + width / 2;
  int sub_y = QInt (visible.top) + height / 2;

  int how_to_divide = (height > width) ? 0 : 1;

  int n2[2];
  csVector2 *p2[2];

  CHK (p2[0] = new csVector2[n+1]);
  CHK (p2[1] = new csVector2[n+1]);

  n2[0]=n2[1]=0;

  if (how_to_divide)
  {
    // dividing vertically
    // (p[0] -- left poly, p[1] -- right poly)

    int i=0,where_are_we=p2d[0].x>sub_x;

    p2[where_are_we][n2[where_are_we]++]=p2d[0];

    for (int _=1,prev=0;_<=n;_++)
    {
      if (_==n) i=0;
      else i=_;

      int now_we_are=p2d[i].x>sub_x;

      if (now_we_are==where_are_we)
      {
  	if (i)
    	p2[where_are_we][n2[where_are_we]++]=p2d[i];
      }
      else
      {
  	float y=(p2d[prev].y*(p2d[i].x-sub_x)+p2d[i].y*(sub_x-p2d[prev].x))/(p2d[i].x-p2d[prev].x);
  	csVector2 p(sub_x,y);

  	p2[0][n2[0]++]=p2[1][n2[1]++]=p;

  	if (i) p2[now_we_are][n2[now_we_are]++]=p2d[i];
      }

      where_are_we=now_we_are;
      prev=i;
    }

    __rect v;
    v.left=visible.left;
    v.right=sub_x;
    v.top=visible.top;
    v.bottom=visible.bottom;

    poly_fill (n2[0],p2[0],v);

    v.left=sub_x;
    v.right=visible.right;

    poly_fill (n2[1],p2[1],v);
  }
  else
  {
    // dividing horizontally
    // (p[0] -- top poly, p[1] -- bottom poly)

    int i=0,where_are_we=p2d[0].y>sub_y;

    p2[where_are_we][n2[where_are_we]++]=p2d[0];

    for (int _=1,prev=0;_<=n;_++)
    {
      if (_==n) i=0;
      else i=_;

      int now_we_are=p2d[i].y>sub_y;

      if (now_we_are==where_are_we)
      {
  	if (i)
    	p2[where_are_we][n2[where_are_we]++]=p2d[i];
      }
      else
      {
  	float x=(p2d[prev].x*(p2d[i].y-sub_y)+p2d[i].x*(sub_y-p2d[prev].y))/(p2d[i].y-p2d[prev].y);
  	csVector2 p(x, sub_y);

  	p2[0][n2[0]++]=p2[1][n2[1]++]=p;

  	if (i) p2[now_we_are][n2[now_we_are]++]=p2d[i];
      }

      where_are_we=now_we_are;
      prev=i;
    }

    __rect v;
    v.left=visible.left;
    v.right=visible.right;
    v.top=visible.top;
    v.bottom=sub_y;

    poly_fill (n2[0],p2[0],v);

    v.top=sub_y;
    v.bottom=visible.bottom;

    poly_fill (n2[1],p2[1],v);
  }

  CHK (delete[] p2[0]);
  CHK (delete[] p2[1]);

  depth--;
}

/* Modified by me to add nice lightmaps recalculations -- D.D. */
void csPolyTexture::FillLightmap (csLightView& lview)
{
  if (!lm) return;
  DBCHECK ("room4", "southU");
  csStatLight* light = (csStatLight*)lview.l;
  DB ((MSG_DEBUG_0, "#### shine_lightmaps #### light:(%f,%f,%f)\n", light->GetCenter ().x, light->GetCenter ().y, light->GetCenter ().z));

  int lw = lm->GetWidth (); // @@@ DON'T NEED TO GO TO PO2 SIZES
  int lh = lm->GetHeight ();

  int u, uv;

  int l1, l2 = 0, l3 = 0;
  float d, dl;

  int ww, hh;
  txt_handle->GetMipMapDimensions (mipmap_level, ww, hh);

  DB ((MSG_DEBUG_0, "lw=%d lh=%d ww=%d hh=%d mipmap_shift=%d mipmap_size=%d\n", lw, lh, ww, hh, mipmap_shift, mipmap_size));

  csPolyPlane* pl = polygon->GetPlane ();
  float cosfact = polygon->GetCosinusFactor ();
  if (cosfact == -1) cosfact = cfg_cosinus_factor;

  // From: T = Mwt * (W - Vwt)
  // ===>
  // Mtw * T = W - Vwt
  // Mtw * T + Vwt = W
  csMatrix3 m_t2w = pl->m_world2tex.GetInverse ();
  csVector3 vv = pl->v_world2tex;

  // From: Ax+By+Cz+D = 0
  // From: T = Mwt * (W - Vwt)
  // ===>
  // Mtw * T + Vwt = W
  // Get 'w' from 'u' and 'v' (using u,v,w plane).
  float A = pl->GetWorldPlane ().A ();
  float B = pl->GetWorldPlane ().B ();
  float C = pl->GetWorldPlane ().C ();
  float D = pl->GetWorldPlane ().D ();
  float txt_A = A*m_t2w.m11 + B*m_t2w.m21 + C*m_t2w.m31;
  float txt_B = A*m_t2w.m12 + B*m_t2w.m22 + C*m_t2w.m32;
  float txt_C = A*m_t2w.m13 + B*m_t2w.m23 + C*m_t2w.m33;
  float txt_D = A*pl->v_world2tex.x + B*pl->v_world2tex.y + C*pl->v_world2tex.z + D;

  DB ((MSG_DEBUG_0, "(A,B,C,D)=(%f,%f,%f,%f) txt_(A,B,C,D)=(%f,%f,%f,%f)\n", A, B, C, D, txt_A, txt_B, txt_C, txt_D));

  csVector3 v1, v2;

  int ru, rv;
  float invww, invhh;
  invww = 1. / (float)ww;
  invhh = 1. / (float)hh;

  bool hit;     // Set to true if there is a hit
  bool first_time = false;  // Set to true if this is the first pass for the dynamic light
  int dyn;

  unsigned char* mapR;
  unsigned char* mapG;
  unsigned char* mapB;
  csShadowMap* smap;

  int i;
  smap = NULL;
  hit = false;

  dyn = light->IsDynamic ();
  if (dyn)
  {
    smap = lm->FindShadowMap (light);
    if (!smap) { smap = lm->NewShadowMap (light, w, h, mipmap_size); first_time = true; }
    else first_time = false;
    mapR = smap->map;
    mapG = NULL;
    mapB = NULL;
  }
  else
  {
    mapR = lm->GetStaticMap ().mapR;
    mapG = lm->GetStaticMap ().mapG;
    mapB = lm->GetStaticMap ().mapB;
  }
  long lm_size = lm->lm_size;

  float miny = 1000000, maxy = -1000000;
  int MaxIndex = -1, MinIndex = -1;

  // Calculate the uv's for all points of the frustrum (the
  // frustrum is actually a clipped version of the polygon).
  csVector2* f_uv = NULL;

  // Our polygon on its own texture space. Weird, isn't it? ;)
  csVector2* rp = NULL;
  int rpv=0;

  csFrustrum* light_frustrum = lview.light_frustrum;
  int num_frustrum = light_frustrum->GetNumVertices ();
  csVector3* frustrum = light_frustrum->GetVertices ();
  int mi;
  CHK (f_uv = new csVector2 [num_frustrum]);

  rpv=polygon->GetNumVertices();
  CHK (rp = new csVector2 [rpv]);

  csVector3 projector;

  for(i=0;i<rpv;i++)
  {
    projector=pl->m_world2tex * (polygon->Vcam(i) + light_frustrum->GetOrigin () - pl->v_world2tex);
    rp[i].x=(projector.x*ww-Imin_u) / (mipmap_size)+0.5;
    rp[i].y=(projector.y*hh-Imin_v) / (mipmap_size)+0.5;
  }

  for (i = 0 ; i < num_frustrum ; i++)
  {
    if (lview.mirror) mi = num_frustrum-i-1;
    else mi = i;

    // T = Mwt * (W - Vwt)
    v1 = pl->m_world2tex * (frustrum[mi] + light_frustrum->GetOrigin () - pl->v_world2tex);
    f_uv[i].x = (v1.x*ww-Imin_u) / (mipmap_size) + 0.5;
    f_uv[i].y = (v1.y*hh-Imin_v) / (mipmap_size) + 0.5;
    if (f_uv[i].y < miny) miny = f_uv[MinIndex = i].y;
    if (f_uv[i].y > maxy) maxy = f_uv[MaxIndex = i].y;

    DB ((MSG_DEBUG_0, "    %d: frust:(%f,%f,%f) f_uv:(%f,%f) ", i, frustrum[mi].x, frustrum[mi].y, frustrum[mi].z, f_uv[i].x, f_uv[i].y));
    DB ((MSG_DEBUG_0, "frust+lcenter:(%f,%f,%f)\n", frustrum[mi].x+light_frustrum->GetOrigin ().x, frustrum[mi].y+light_frustrum->GetOrigin ().y, frustrum[mi].z+light_frustrum->GetOrigin ().z));
  }

  float r200d = lview.r * NORMAL_LIGHT_LEVEL / light->GetRadius ();
  float g200d = lview.g * NORMAL_LIGHT_LEVEL / light->GetRadius ();
  float b200d = lview.b * NORMAL_LIGHT_LEVEL / light->GetRadius ();

  __texture_width=lw;
  __texture=(float*)calloc(lh*lw,sizeof(float));
  __mark=(unsigned char*)calloc(lh,lw);

  __rect vis={0,lw,0,lh};

  //fo=fopen("light.txt","at");

  __draw_func=lixel_intensity;
  poly_fill(num_frustrum,f_uv,vis);
  __draw_func=correct_results;
  poly_fill(rpv,rp,vis);

  uv=0;
  for (int sy=0; sy < lh; sy++)
  {
    for (u=0;u<lw;u++,uv++)
    {
      //@@@ (Note from Jorrit): The following test should not be needed
      // but it appears to be anyway. 'uv' can get too large.
      if (uv >= lm_size) continue;

        float usual_value=1.0;

//      __uv=uv;
        float lightintensity=__texture[uv];

        if(!lightintensity)
        {
          usual_value=0.0;

          if(u&&__mark[uv-1])
          {
            lightintensity+=__texture[uv-1];
            usual_value++;
          }

          if(sy&&__mark[uv-lw])
          {
            lightintensity+=__texture[uv-lw];
            usual_value++;
          }

          if((u!=lw-1)&&__mark[uv+1])
          {
            lightintensity+=__texture[uv+1];
            usual_value++;
          }

          if((sy!=lh-1)&&__mark[uv+lw])
          {
            lightintensity+=__texture[uv+lw];
            usual_value++;
          }

          if(!lightintensity)
            continue;
        }

        float lightness=lightintensity/usual_value;
//      if(lightness>1||lightness<0)
//        fprintf(fo,"(%d, %d) -> lightness=%.2f/%.2f=%.2f\n",u,sy,lightintensity,usual_value,lightness);

        ru = u  << mipmap_shift;
        rv = sy << mipmap_shift;

        bool rc = false;
        int tst;
        static int shift_u[5] = { 0, 2, 0, -2, 0 };
        static int shift_v[5] = { 0, 0, 2, 0, -2 };
        for (tst = 0 ; tst < 5 ; tst++)
        {
          v1.x = (float)(ru+shift_u[tst]+Imin_u)*invww;
          v1.y = (float)(rv+shift_v[tst]+Imin_v)*invhh;
          if (ABS (txt_C) < SMALL_EPSILON)
            v1.z = 0;
          else
            v1.z = - (txt_D + txt_A*v1.x + txt_B*v1.y) / txt_C;
          v2 = vv + m_t2w * v1;

	  // Check if the point on the polygon is shadowed. To do this
	  // we traverse all shadow frustrums and see if it is contained in any of them.
	  csShadowFrustrum* shadow_frust;
	  shadow_frust = lview.shadows.GetFirst ();
	  bool shadow = false;
	  while (shadow_frust)
	  {
	    if (shadow_frust->relevant && shadow_frust->polygon != polygon)
	      if (shadow_frust->Contains (v2-shadow_frust->GetOrigin ()))
		{ shadow = true; break; }
	    shadow_frust = shadow_frust->next;
	  }
	  if (!shadow) { rc = false; break; }

	  if (!do_accurate_things) break;
	  rc = true;
        }

        if (!rc)
        {
    	  //@@@ I think this is wrong and the next line is right!
          //d = csSquaredDist::PointPoint (light->GetCenter (), v2);
          d = csSquaredDist::PointPoint (lview.light_frustrum->GetOrigin (), v2);
          DB ((MSG_DEBUG_0, "    -> In viewing frustrum (distance %f compared with radius %f)\n", sqrt (d), light->GetRadius ()));

	  if (d >= light->GetSquaredRadius ()) continue;
	  d = sqrt (d);
          DB ((MSG_DEBUG_0, "    -> *** HIT ***\n"));

	  hit = true;

	  l1 = mapR[uv];

	  //@@@ I think this is wrong and the next line is right!
	  //float cosinus = (v2-light->GetCenter ())*polygon->GetPolyPlane ()->Normal ();
	  float cosinus = (v2-lview.light_frustrum->GetOrigin ())*polygon->GetPolyPlane ()->Normal ();
	  cosinus /= d;
	  cosinus += cosfact;
	  if (cosinus < 0) cosinus = 0;
	  else if (cosinus > 1) cosinus = 1;
	  if (dyn)
	  {
	    dl = NORMAL_LIGHT_LEVEL/light->GetRadius ();
	    l1 = l1 + QInt (lightness*QRound (cosinus * (NORMAL_LIGHT_LEVEL - d*dl)));
	    if (l1 > 255) l1 = 255;
	    mapR[uv] = l1;
	  }
	  else
	  {
	    if (lview.r > 0)
	    {
	      l1 = l1 + QInt (lightness*QRound (cosinus * r200d*(light->GetRadius () - d)));
	      if (l1 > 255) l1 = 255;
	      mapR[uv] = l1;
	    }
	    if (lview.g > 0 && mapG)
	    {
	      l2 = mapG[uv] + QInt (lightness*QRound (cosinus * g200d*(light->GetRadius () - d)));
	      if (l2 > 255) l2 = 255;
	      mapG[uv] = l2;
	    }
	    if (lview.b > 0 && mapB)
	    {
	      l3 = mapB[uv] + QInt (lightness*QRound (cosinus * b200d*(light->GetRadius () - d)));
	      if (l3 > 255) l3 = 255;
	      mapB[uv] = l3;
	    }
	  }
        }
      //mapR[uv] = 128;
      //mapG[uv] = 128;
      //mapB[uv] = 128;
      //if (u == 0 && (v & 1)) { mapR[uv] = 255; mapG[uv] = 0; mapB[uv] = 0; }
      //else if (v == 0 && (u & 1)) { mapR[uv] = 0; mapG[uv] = 255; mapB[uv] = 0; }
      //else if (u == lw-1 && (v & 1)) { mapR[uv] = 0; mapG[uv] = 0; mapB[uv] = 255; }
      //else if (v == lh-1 && (u & 1)) { mapR[uv] = 255; mapG[uv] = 0; mapB[uv] = 255; }
      //else if (u == v) { mapR[uv] = 255; mapG[uv] = 255; mapB[uv] = 0; }
      //else if (u == lh-1-v) { mapR[uv] = 0; mapG[uv] = 255; mapB[uv] = 255; }
    }
  }

  CHK (delete [] f_uv);
  CHK (delete [] rp);

  free(__texture);
  free(__mark);

  //fclose(fo);

  if (dyn && first_time)
  {
    if (!hit)
    {
      // There was no hit. Just remove the dynamic light map from the polygon
      // unless it was not allocated this turn.
      lm->DelShadowMap (smap);
    }
    else
    {
      // There was a hit. Register this polygon with the light.
      light->RegisterPolygon (polygon);
    }
  }
}

/* Modified by me to correct some lightmap's border problems -- D.D. */
void csPolyTexture::ShineDynLightmap (csLightPatch* lp)
{
  int lw = (w>>mipmap_shift)+2;   // @@@ DON'T NEED TO GO TO 'W', 'W_ORIG' SHOULD BE SUFFICIENT!
  int lh = (h>>mipmap_shift)+2;

  int u, uv;

  int l1, l2 = 0, l3 = 0;
  float d;

  int ww, hh;
  txt_handle->GetMipMapDimensions (mipmap_level, ww, hh);

  csPolyPlane* pl = polygon->GetPlane ();
  float cosfact = polygon->GetCosinusFactor ();
  if (cosfact == -1) cosfact = cfg_cosinus_factor;

  // From: T = Mwt * (W - Vwt)
  // ===>
  // Mtw * T = W - Vwt
  // Mtw * T + Vwt = W
  csMatrix3 m_t2w = pl->m_world2tex.GetInverse();
  csVector3 vv = pl->v_world2tex;

  // From: Ax+By+Cz+D = 0
  // From: T = Mwt * (W - Vwt)
  // ===>
  // Mtw * T + Vwt = W
  // Get 'w' from 'u' and 'v' (using u,v,w plane).
  float A = pl->GetWorldPlane ().A ();
  float B = pl->GetWorldPlane ().B ();
  float C = pl->GetWorldPlane ().C ();
  float D = pl->GetWorldPlane ().D ();
  float txt_A = A*m_t2w.m11 + B*m_t2w.m21 + C*m_t2w.m31;
  float txt_B = A*m_t2w.m12 + B*m_t2w.m22 + C*m_t2w.m32;
  float txt_C = A*m_t2w.m13 + B*m_t2w.m23 + C*m_t2w.m33;
  float txt_D = A*pl->v_world2tex.x + B*pl->v_world2tex.y + C*pl->v_world2tex.z + D;

  csVector3 v1, v2;

  int ru, rv;
  float invww, invhh;
  invww = 1. / (float)ww;
  invhh = 1. / (float)hh;

  csRGBLightMap& remap = lm->GetRealMap ();
  csDynLight* light = (csDynLight*)(lp->light);
  unsigned char* mapR = remap.mapR;
  unsigned char* mapG = remap.mapG;
  unsigned char* mapB = remap.mapB;
  long lm_size = lm->lm_size;

  int i;
  float miny = 1000000, maxy = -1000000;
  int MaxIndex = -1, MinIndex = -1;

  // Calculate the uv's for all points of the frustrum (the
  // frustrum is actually a clipped version of the polygon).
  csVector2* f_uv = NULL;
  if (lp->vertices)
  {
    int mi;
    CHK (f_uv = new csVector2 [lp->num_vertices]);
    for (i = 0 ; i < lp->num_vertices ; i++)
    {
      //if (lview.mirror) mi = lview.num_frustrum-i-1;
      //else mi = i;
      mi = i;

      // T = Mwt * (W - Vwt)
      //v1 = pl->m_world2tex * (lp->vertices[mi] + lp->center - pl->v_world2tex);
      //@@@ This is only right if we don't allow reflections on dynamic lights
      v1 = pl->m_world2tex * (lp->vertices[mi] + light->GetCenter () - pl->v_world2tex);
      f_uv[i].x = (v1.x*ww-Imin_u) / mipmap_size;
      f_uv[i].y = (v1.y*hh-Imin_v) / mipmap_size;
      if (f_uv[i].y < miny) miny = f_uv[MinIndex = i].y;
      if (f_uv[i].y > maxy) maxy = f_uv[MaxIndex = i].y;
    }
  }

  //float r200d = lview.r * NORMAL_LIGHT_LEVEL / light->GetRadius ();
  //float g200d = lview.g * NORMAL_LIGHT_LEVEL / light->GetRadius ();
  //float b200d = lview.b * NORMAL_LIGHT_LEVEL / light->GetRadius ();
  float r = light->GetColor ().red;
  float g = light->GetColor ().green;
  float b = light->GetColor ().blue;
  float r200d = r * NORMAL_LIGHT_LEVEL / light->GetRadius ();
  float g200d = g * NORMAL_LIGHT_LEVEL / light->GetRadius ();
  float b200d = b * NORMAL_LIGHT_LEVEL / light->GetRadius ();

  int new_lw = lm->GetWidth ();

  int scanL1, scanL2, scanR1, scanR2;   // scan vertex left/right start/final
  float sxL, sxR, dxL, dxR;             // scanline X left/right and deltas
  int sy, fyL, fyR;                     // scanline Y, final Y left, final Y right
  int xL, xR;

  sxL = sxR = dxL = dxR = 0;            // avoid GCC warnings about "uninitialized variables"
  scanL2 = scanR2 = MaxIndex;
//  sy = fyL = fyR = (QRound (f_uv[scanL2].y)>lh-1)?lh-1:QRound (f_uv[scanL2].y);
  sy = fyL = fyR = (QRound (ceil(f_uv[scanL2].y))>lh-1)?lh-1:QRound (ceil(f_uv[scanL2].y));

  for ( ; ; )
  {
    //-----
    // We have reached the next segment. Recalculate the slopes.
    //-----
    bool leave;
    do
    {
      leave = true;
      if (sy <= fyR)
      {
        // Check first if polygon has been finished
a:      if (scanR2 == MinIndex) goto finish;
        scanR1 = scanR2;
        scanR2 = (scanR2 + 1) % lp->num_vertices;

        if(fabs(f_uv[scanR2].y-f_uv[MaxIndex].y)<EPS)
        {
          // oops! we have a flat bottom!
          goto a;
        }
/*      if (scanR2 == MinIndex) goto finish;
        scanR1 = scanR2;
        scanR2 = (scanR2 + 1) % lp->num_vertices;
*/
        fyR = QRound(floor(f_uv[scanR2].y));
        float dyR = (f_uv[scanR1].y - f_uv[scanR2].y);
  sxR = f_uv[scanR1].x;
        if (dyR != 0)
        {
          dxR = (f_uv[scanR2].x - sxR) / dyR;
    // horizontal pixel correction
          sxR += dxR * (f_uv[scanR1].y - ((float)sy));
        }
  else dxR = 0;
        leave = false;
      }
      if (sy <= fyL)
      {
b:      if (scanL2 == MinIndex) goto finish;
        scanL1 = scanL2;
        scanL2 = (scanL2 - 1 + lp->num_vertices) % lp->num_vertices;

        if(fabs(f_uv[scanL2].y-f_uv[MaxIndex].y)<EPS)
        {
          // oops! we have a flat bottom!
          goto b;
        }

/*      scanL1 = scanL2;
        scanL2 = (scanL2 - 1 + lp->num_vertices) % lp->num_vertices;
*/
        fyL = QRound(floor(f_uv[scanL2].y));
        float dyL = (f_uv[scanL1].y - f_uv[scanL2].y);
  sxL = f_uv[scanL1].x;
        if (dyL != 0)
        {
          dxL = (f_uv[scanL2].x - sxL) / dyL;
          // horizontal pixel correction
          sxL += dxL * (f_uv[scanL1].y - ((float)sy));
        }
  else dxL = 0;
        leave = false;
      }
    }
    while (!leave);

    // Find the trapezoid top (or bottom in inverted Y coordinates)
    int fin_y;
    if (fyL > fyR) fin_y = fyL;
    else fin_y = fyR;

    while (sy >= fin_y)
    {
      // Compute the rounded screen coordinates of horizontal strip
      float _l=sxL,_r=sxR;

      if(_r>_l) {float _=_r; _r=_l; _l=_;}
    
      xL = QRound (ceil(_l))+1;
      xR = QRound (floor(_r));

/*    xL = QRound (sxL)+1;
      xR = QRound (sxR);
      if (xR > xL) { int xswap = xR; xR = xL; xL = xswap; }
*/
      if (xR < 0) xR = 0;
      if (xL > lw) xL = lw;

      for (u = xR; u < xL ; u++)
      {
        uv = sy*new_lw+u;

  //@@@ (Note from Jorrit): The following test should not be needed
  // but it appears to be anyway. 'uv' can get both negative and too large.
  if (uv < 0 || uv >= lm_size) continue;

        ru = u  << mipmap_shift;
        rv = sy << mipmap_shift;

        v1.x = (float)(ru+Imin_u)*invww;
        v1.y = (float)(rv+Imin_v)*invhh;
        if (ABS (txt_C) < SMALL_EPSILON)
          v1.z = 0;
        else
          v1.z = - (txt_D + txt_A*v1.x + txt_B*v1.y) / txt_C;
        v2 = vv + m_t2w * v1;

  // Check if the point on the polygon is shadowed. To do this
  // we traverse all shadow frustrums and see if it is contained in any of them.
  csShadowFrustrum* shadow_frust;
  shadow_frust = lp->shadows.GetFirst ();
  bool shadow = false;
  while (shadow_frust)
  {
    if (shadow_frust->relevant && shadow_frust->polygon != polygon)
      if (shadow_frust->Contains (v2-shadow_frust->GetOrigin ()))
        { shadow = true; break; }
    shadow_frust = shadow_frust->next;
  }

  if (!shadow)
        {
    //@@@ This is only right if we don't allow reflections for dynamic lights
          d = csSquaredDist::PointPoint (light->GetCenter (), v2);

    if (d >= light->GetSquaredRadius ()) continue;
    d = sqrt (d);

    //@@@ This is only right if we don't allow reflections for dynamic lights
    float cosinus = (v2-light->GetCenter ())*polygon->GetPolyPlane ()->Normal ();
    cosinus /= d;
    cosinus += cosfact;
    if (cosinus < 0) cosinus = 0;
    else if (cosinus > 1) cosinus = 1;
    if (r > 0)
    {
      l1 = QRound (cosinus*r200d*(light->GetRadius () - d));
      if (l1)
      {
        l1 += mapR[uv];
        if (l1 > 255) l1 = 255;
        mapR[uv] = l1;
      }
    }
    if (g > 0 && mapG)
    {
      l2 = QRound (cosinus*g200d*(light->GetRadius () - d));
      if (l2)
      {
        l2 += mapG[uv];
        if (l2 > 255) l2 = 255;
        mapG[uv] = l2;
      }
    }
    if (b > 0 && mapB)
    {
      l3 = QRound (cosinus*b200d*(light->GetRadius () - d));
      if (l3)
      {
        l3 += mapB[uv];
        if (l3 > 255) l3 = 255;
        mapB[uv] = l3;
      }
    }
        }
      }

      if(!sy) goto finish;
      sxL += dxL;
      sxR += dxR;
      sy--;
    }
  }

finish:

  CHK (delete [] f_uv);
}

void csPolyTexture::CreateDirtyMatrix ()
{
  int dw = w/csPolyTexture::subtex_size + 1;
  int dh = h/csPolyTexture::subtex_size + 1;
  if (!dirty_matrix || dw != dirty_w || dh != dirty_h)
  {
    // Dirty matrix does not exist or the size is not correct
    CHK (delete [] dirty_matrix);
    dirty_w = dw;
    dirty_h = dh;
    dirty_size = dw*dh;
    CHK (dirty_matrix = new UByte [dirty_size]);
    MakeAllDirty ();
  }
}

void csPolyTexture::MakeAllDirty ()
{
  if (dirty_matrix)
  {
    memset (dirty_matrix, 1, dirty_size);
    dirty_cnt = dirty_size;
  }
}

//---------------------------------------------------------------------------
