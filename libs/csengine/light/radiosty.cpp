/*
    Copyright (C) 2000 by W.C.A. Wijngaards

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
#include "csgeom/polyaa.h"
#include "csengine/radiosty.h"
#include "csengine/world.h"
#include "csengine/sector.h"
#include "csengine/light.h"
#include "csengine/lghtmap.h"
#include "csengine/polytext.h"
#include "csengine/cspmeter.h"
#include "csengine/csppulse.h"
#include "csengine/polytmap.h"
#include "csengine/texture.h"
#include "csengine/rview.h"
#include "csgeom/math3d.h"
#include "csgeom/vector3.h"
#include "csgeom/frustrum.h"
#include "iimage.h"
#include "ilghtmap.h"
#include "itexture.h"
#include <math.h>
#include "qint.h"

//------------ Utility ------------------------------------
float FastPow2(float x, const int y)
// computes x ** (2 ** y), using only y multiplies.
{
  float res = x;
  for(int i=1; i<y; i++)
    res *= res;
  return res;
}

//--------------- csRadPoly --------------------------------------

IMPLEMENT_CSOBJTYPE(csRadPoly, csObject);

csRadPoly :: csRadPoly(csPolygon3D *original)
{
  polygon = original;
  polygon->ObjAdd(this); // attach to original
  area = original->GetArea();
  total_unshot_light = area;
  last_shoot_priority = 0.0f;
  num_repeats = 0;
  iter_cookie = 0;
  //lightmap = polygon->GetLightMapInfo()->GetLightMap(); // returns the
  // 'real' lightmap, which includes dynamic lights.
  // but we need the static lightmap
  csmap = polygon->GetLightMapInfo()->GetPolyTex()->GetCSLightMap();
  width = csmap->GetRealWidth();
  height = csmap->GetRealHeight();
  size = csmap->GetSize();
  lightmap = &csmap->GetStaticMap();
  deltamap = new csRGBFloatLightMap();
  // all light in static map is unshot now, add it to delta. clear lightmap.
  deltamap->Copy(*lightmap, size);
  memset( lightmap->GetMap(), 0, size*3);
  // get lumel coverage
  lumel_coverage_map = ComputeLumelCoverage();
  SetupQuickLumel2World();

  ComputePriority();
}

csRadPoly :: ~csRadPoly()
{
  if(lumel_coverage_map) delete[] lumel_coverage_map;
  delete deltamap;
  polygon->ObjRelease(this); // detach from original
}


void csRadPoly :: GetLumelWorldCoords(csVector3& res, int x, int y)
{
  // see polytext.cpp for more info.
  int ww=0, hh=0;
  polygon->GetTextureHandle()->GetMipMapDimensions (0, ww, hh);
  float invww = 1. / (float)ww;
  float invhh = 1. / (float)hh;

  csPolyTexture *polytext = polygon->GetLightMapInfo()->GetPolyTex();
  csPolyTxtPlane* txt_pl = polygon->GetLightMapInfo ()->GetTxtPlane ();
  csMatrix3 *m_world2tex;
  csVector3 *v_world2tex;
  txt_pl->GetWorldToTexture(m_world2tex, v_world2tex);
  csMatrix3 m_t2w = m_world2tex->GetInverse ();
  csVector3 vv = *v_world2tex;

  csVector3 v1(0), v2;
  int lightcell_shift = csLightMap::lightcell_shift;
  int ru = x << lightcell_shift;
  int rv = y << lightcell_shift;
  int Imin_u = polytext->GetIMinU();
  int Imin_v = polytext->GetIMinV();
  v1.x = (float)(ru + Imin_u) * invww;
  v1.y = (float)(rv + Imin_v) * invhh;
  v2 = vv + m_t2w * v1;

  res = v2;
}


void csRadPoly :: SetupQuickLumel2World()
{
  // setup quick conversion to lumel space.
  // for flat polygons only.
  // curved surfaces will have to use the slow method, but are
  // not supported yet anyway.
  GetLumelWorldCoords(lumel_origin, 0, 0);
  GetLumelWorldCoords(lumel_x_axis, 1, 0);
  GetLumelWorldCoords(lumel_y_axis, 0, 1);
  one_lumel_area = ABS( csMath3::Area3(lumel_origin, lumel_x_axis, 
    lumel_y_axis) );
  lumel_x_axis -= lumel_origin;
  lumel_y_axis -= lumel_origin;

}


void csRadPoly :: ComputePriority()
{
  float sum = 0;
  for(int x=0; x<size*3; x++)
    sum += deltamap->GetMap()[x] * lumel_coverage_map[x%size];
  float mean_lightval = sum / (float)(size*3);
  total_unshot_light = GetDiffuse() * area * mean_lightval;
  // to prevent loops, polygons with several repeats (at the same
  // priority) are ignored.
  if(num_repeats > 5)
    total_unshot_light = 0.0;
  //CsPrintf(MSG_STDOUT, "RP %s, pri %g, area %g, sum %d, meanval %g ",
  //  polygon->GetName(), total_unshot_light, area, sum, mean_lightval);
  //CsPrintf(MSG_STDOUT, "w %d, h %d, size %d\n", width, height, size);
}

void csRadPoly :: AddDelta(csRadPoly *src, int suv, int ruv, float fraction,
  const csColor& filtercolor)
{
  deltamap->GetRed()[ruv] +=
    (fraction * src->deltamap->GetRed()[suv] * filtercolor.red);
  deltamap->GetGreen()[ruv] +=
    (fraction * src->deltamap->GetGreen()[suv] * filtercolor.green);
  deltamap->GetBlue()[ruv] +=
    (fraction * src->deltamap->GetBlue()[suv] * filtercolor.blue);
}


void csRadPoly :: CopyAndClearDelta()
{
  int res;
  int uv;
  for(uv=0; uv<size*3; uv++)
  {
    if(lumel_coverage_map[uv%size] == 0.0) continue;
    res = lightmap->GetMap()[uv] + QRound(deltamap->GetMap()[uv]);
    if(res > 255) res = 255;
    else if(res < 0) res = 0;
    lightmap->GetMap()[uv] = res;
  }
  for(uv=0; uv<size*3; uv++)
    deltamap->GetMap()[uv] = 0.0;
  total_unshot_light = 0.0;
}

void csRadPoly :: GetDeltaSums(float &red, float &green, float &blue)
{
  red = 0.0;
  green = 0.0;
  blue = 0.0;
  for(int uv=0; uv<size; uv++)
  {
    if(lumel_coverage_map[uv] == 0.0f) continue;
    red += deltamap->GetRed()[uv];
    green += deltamap->GetGreen()[uv];
    blue += deltamap->GetBlue()[uv];
  }
}

void csRadPoly :: ApplyAmbient(int red, int green, int blue)
{
  float res;
  for(int uv=0; uv<size; uv++)
  {
    if(lumel_coverage_map[uv] == 0.0) continue;

    res = deltamap->GetRed()[uv] + red;
    if(res>255)res=255; else if(res<0) res=0;
    deltamap->GetRed()[uv] = res;

    res = deltamap->GetGreen()[uv] + green;
    if(res>255)res=255; else if(res<0) res=0;
    deltamap->GetGreen()[uv] = res;

    res = deltamap->GetBlue()[uv] + blue;
    if(res>255)res=255; else if(res<0) res=0;
    deltamap->GetBlue()[uv] = res;
  }
}


/// the map and the width to draw in for draw_lumel_cov
static volatile int draw_lumel_cov_width = 0;
static volatile float *draw_lumel_coverage_map = 0;
static void draw_lumel_cov( int x, int y, float density, void * )
{
  int addr = x + y * draw_lumel_cov_width;
  draw_lumel_coverage_map[addr] = density;
}


float* csRadPoly :: ComputeLumelCoverage()
{
  int i;
  // allocate the coverage map.
  float *covmap = new float[size];
  draw_lumel_coverage_map = covmap;
  draw_lumel_cov_width = width;
  for(i=0; i<size; i++)
    covmap[i] = 0.0;

  // get polygon projected to texture space
  int ww, hh;
  polygon->GetTextureHandle()->GetMipMapDimensions (0, ww, hh);
  csPolyTxtPlane* txt_pl = polygon->GetLightMapInfo ()->GetTxtPlane ();
  csPolyTexture *polytext = polygon->GetLightMapInfo()->GetPolyTex();
  int Imin_u = polytext->GetIMinU();
  int Imin_v = polytext->GetIMinV();
  csMatrix3 *m_world2tex;
  csVector3 *v_world2tex;
  txt_pl->GetWorldToTexture(m_world2tex, v_world2tex);

  int rpv= polygon->GetVertices ().GetNumVertices ();
  csVector2* rp = new csVector2[rpv];
  csVector3 projector;
  float inv_lightcell_size = 1.0 / csLightMap::lightcell_size;
  for (i = 0; i < rpv; i++)
  {
    projector = (*m_world2tex) * (polygon->Vwor (i) - (*v_world2tex));
    rp [i].x = (projector.x * ww - Imin_u) * inv_lightcell_size;
    rp [i].y = (projector.y * hh - Imin_v) * inv_lightcell_size;
  }
  // draw the polygon in lumel space.
  csAntialiasedPolyFill (rp, rpv, NULL, draw_lumel_cov);

  //printf("Coverage map for %x (w=%d, h=%d)\n", (int)this, width, height);
  //printf("coords (%d): ", rpv);
  //for(i=0; i<rpv; i++)
  //  printf("(%4.2f, %4.2f) ", rp[i].x, rp[i].y);
  //printf("\n");
  //i = 0;
  //for(int y=0;y <height ; y++)
  //{
  //  for(int x=0; x<width; x++, i++)
  //    printf("%4.2f ", covmap[i]);
  //  printf("\n");
  //}

  FixCoverageMap(covmap, width, height);

  delete[] rp;
  return covmap;
}


csRGBLightMap * csRadPoly :: ComputeTextureLumelSized()
{
  int uv;
  csRGBLightMap *map = new csRGBLightMap();
  map->Alloc(size);
  // fill map with flat color
  int flatr = QRound(polygon->GetFlatColor().red * 255.0);
  if(flatr > 255) flatr = 255; else if (flatr < 0) flatr = 0;
  int flatg = QRound(polygon->GetFlatColor().green * 255.0);
  if(flatg > 255) flatg = 255; else if (flatg < 0) flatg = 0;
  int flatb = QRound(polygon->GetFlatColor().blue * 255.0);
  if(flatb > 255) flatb = 255; else if (flatb < 0) flatb = 0;
  for(uv=0; uv<size; uv++)
  {
    map->GetRed()[uv] = flatr;
    map->GetGreen()[uv] = flatg;
    map->GetBlue()[uv] = flatb;
  }

  // get texture of polygon
  csTextureHandle* txthandle = polygon->GetCsTextureHandle();
  if(txthandle == NULL) // no texture: flatcol is enough.
    return map;
  int transr, transg, transb; // transparent color
  txthandle->GetTransparent(transr, transg, transb);
  iImage *txtimage = txthandle->GetImageFile();
  iImage *rgbimage = txtimage->Clone(); 
  rgbimage->SetFormat(CS_IMGFMT_TRUECOLOR); // get rgb
  int txtw = rgbimage->GetWidth();
  int txth = rgbimage->GetHeight();
  RGBPixel *rgb = (RGBPixel *) rgbimage->GetImageData();

  int lightcell_size = csLightMap::lightcell_size;
  int lightcell_shift = csLightMap::lightcell_shift;

  // scale down texture
  // map each lumel to the texture and scan the lightcellsize x lightcellsize
  // region of the texture map.
  int texelsperlumel_shift = lightcell_shift * 2;
  int lumel_uv = 0;
  for(int lumel_y = 0; lumel_y < height; lumel_y ++)
    for(int lumel_x = 0; lumel_x < width; lumel_x++, lumel_uv++)
    {
      /// these ints are only large enough for a lightcellsize < 4096
      int sumr = 0;
      int sumg = 0;
      int sumb = 0;
      // in texture coords the lumel is:
      int txt_start_x = (lumel_x << lightcell_shift) % txtw;
      int txt_start_y = (lumel_y << lightcell_shift) % txth;
      for(int dy = 0; dy < lightcell_size; dy++)
        for(int dx = 0; dx < lightcell_size; dx++)
	{
	  int txt_x = (dx + txt_start_x) % txtw; // modulo to wrap around
	  int txt_y = (dy + txt_start_y) % txth; // and make texture tile
	  int txt_idx = txt_y * txtw + txt_x;
	  sumr += rgb[txt_idx].red;
	  sumg += rgb[txt_idx].green;
	  sumb += rgb[txt_idx].blue;
	}
       // store averages
       map->GetRed()[lumel_uv] = sumr >> texelsperlumel_shift;
       map->GetGreen()[lumel_uv] = sumg >> texelsperlumel_shift;
       map->GetBlue()[lumel_uv] = sumb >> texelsperlumel_shift;
    }

  /*
  printf("Map for %s, %s\n", polygon->GetName(), rgbimage->GetName());
  uv = 0;
  for(int y=0; y<height; y++)
  {
    for(int x=0; x<width; x++, uv++)
      printf("%2.2x%2.2x%2.2x ", map->GetRed()[uv], map->GetGreen()[uv],
        map->GetBlue()[uv]);
    printf("\n");
  }
  */

  // get rid of rgbimage
  rgbimage->DecRef();
  return map;
}


void csRadPoly :: FixCoverageMap(float* map, int width, int height)
{
  int x,y, uv = 0;
  float fix_val = 0.00001;
  for(y=0; y<height; y++)
    for(x=0; x<width; x++,uv++)
    {
      if(map[uv] > 0.0f) continue;
      if(x>0 && map[uv-1] > fix_val)
        map[uv] = fix_val;
      else if(y>0 && map[uv-width] > fix_val)
        map[uv] = fix_val;
      else if(x>0 && y>0 && map[uv-width-1] > fix_val)
        map[uv] = fix_val;
    }
}


csRadPoly* csRadPoly :: GetRadPoly(csPolygon3D &object)
{ // we are attached to the original polygon as child.
  csObject *o = object.GetChild (csRadPoly::Type);
  if (o) return (csRadPoly*) o;
  return NULL;
}


//--------------- csRadTree --------------------------------------
/**
 *  Note: the right subtree contains all elements >= the current.
 *  The left subtree all elements < the current.
 */

void csRadTree::Insert(csRadPoly *p)
{
  // find spot
  csRadTree *spot = this;
  csRadTree *parent = NULL;
  while(spot != NULL)
  {
    parent = spot;
    if(p->GetPriority() >= spot->GetPriority())
      spot = spot->right;
    else spot = spot->left;
  }
  // spot is 0, parent can insert new leaf now.
  if(p->GetPriority() >= parent->GetPriority())
    parent->right = new csRadTree(p, 0, 0);
  else parent->left = new csRadTree(p, 0, 0);
  // done.
}


void csRadTree :: DelNode()
{
  // deletes this node with both left & right subtrees
  // so left != NULL && right != NULL.
  // because tree has >= elements in its right subtree, we must
  // switch this element with the smallest el. of the right subtree.
  // Since that element may be equal in priority to this one.

  csRadTree *parent = 0;
  csRadTree *replacement = right->FindLeftMost(parent);
  // swap contents of replacement and this
  element = replacement->element;
  // get rid of replacment. replacement.left == NULL.
  if(parent==NULL)
  {
    // I am parent
    right = replacement->right;
    replacement->right = NULL;
    delete replacement;
    return;
  }
  parent->left = replacement->right;
  replacement->right = NULL;
  delete replacement;
}

csRadTree* csRadTree::Delete(csRadPoly *p)
{
  // find spot containing p, and its parent
  csRadTree *spot = this;
  csRadTree *parent = NULL;
  while(spot != NULL && spot->element != p)
  {
    parent = spot;
    if(p->GetPriority() >= spot->GetPriority())
      spot = spot->right;
    else spot = spot->left;
  }
  if(spot==NULL) // no such element
    return this;
  // spot is the element containing p, parent is parent of spot.
  if(parent == NULL) // no parent, this el. contains things.
  {
    csRadTree *newroot = 0;
    if(left==NULL)
    {
      newroot = right;
      right = NULL;
      delete this;
      return newroot;
    }
    if(right==NULL)
    {
      newroot = left;
      left = NULL;
      delete this;
      return newroot;
    }
    // this node has a left & a right subtree
    DelNode();
    return this;
  }
  // spot has a parent
  if(spot->left!=NULL && spot->right!=NULL)
  {
    // spot has both subtrees
    spot->DelNode();
    return this;
  }
  // one subtree is missing, substitute the remaining one for spot.
  // remaining one may be NULL.
  csRadTree *replacement = 0;
  float spotpri = spot->GetPriority();
  if(spot->left == NULL)
  {
    replacement = spot->right;
    spot->right = NULL;
    delete spot;
  }
  else // else! do not do both.
    if(spot->right == NULL) 
  {
    replacement = spot->left;
    spot->left = NULL;
    delete spot;
  }
  if(spotpri >= parent->GetPriority() )
    parent->right = replacement;
  else parent->left = replacement;
  return this;
}


csRadTree* csRadTree::PopHighest(csRadPoly*& p)
{
  csRadTree *parent = 0;
  csRadTree *spot = FindRightMost(parent);
  p = spot->element;
  if(parent)
  {
    parent->right = spot->left;
    spot->left = NULL;
    delete spot;
    return this;
  }
  else
  { // this node is deleted, and no right subtree
    csRadTree* newroot = spot->left;
    spot->left = NULL;
    delete spot;
    return newroot;
  }
}


csRadTree* csRadTree::FindLeftMost(csRadTree*& parent)
{
  parent = NULL;
  csRadTree *spot = this;
  while(spot->left != NULL)
  {
    parent = spot;
    spot = spot->left;
  }
  return spot;
}

csRadTree* csRadTree::FindRightMost(csRadTree*& parent)
{
  parent = NULL;
  csRadTree *spot = this;
  while(spot->right != NULL)
  {
    parent = spot;
    spot = spot->right;
  }
  return spot;
}

void csRadTree::TraverseInOrder( void (*func)( csRadPoly * ) )
{
  if(left) left->TraverseInOrder( func );
  func(element);
  if(right) right->TraverseInOrder( func );
}

//--------------- csRadList --------------------------------------
csRadList :: csRadList()
{
  tree = 0;
  num = 0;
}

static void deletefunc (csRadPoly* p)
{
  delete p;
}

csRadList :: ~csRadList()
{
  if(tree){
    tree->TraverseInOrder( deletefunc );
    delete tree;
  }
}


void csRadList :: InsertElement(csRadPoly *p)
{
  if(!tree) tree = new csRadTree(p, 0, 0);
  else tree->Insert(p);
  num++;
}


void csRadList :: DeleteElement(csRadPoly *p)
{
  if(tree) 
  {
    tree = tree->Delete(p);
    num--;
  }
}


csRadPoly * csRadList :: PopHighest()
{
  csRadPoly *p;
  if(!tree) return NULL;
  tree = tree->PopHighest(p);
  num--;
  return p;
}

static void print_func( csRadPoly *p )
{
  CsPrintf(MSG_STDOUT, "csRadList: csRadpoly %x, pri %f \n",
    (int)p, p->GetPriority() );
}

void csRadList :: Print()
{
  // for debug purposes
  CsPrintf(MSG_STDOUT, "csRadList Print().\n");
  if(tree) tree->TraverseInOrder( print_func );
  else CsPrintf(MSG_STDOUT, "csRadList empty.\n");
  CsPrintf(MSG_STDOUT, "csRadList Print() end.\n");
}

//--------------- csRadiosity --------------------------------------

// configuration defaults
bool  csRadiosity::do_static_specular = false;
float csRadiosity::static_specular_amount = 0.70;
int   csRadiosity::static_specular_tightness = 2;
float csRadiosity::colour_bleed = 1.0;
float csRadiosity::stop_priority = 0.1;
float csRadiosity::stop_improvement = 10000.0;
int   csRadiosity::stop_iterations = 1000;

csRadiosity :: csRadiosity(csWorld *current_world)
{
  CsPrintf (MSG_INITIALIZATION, "\nPreparing radiosity...\n");
  iterations = 0;
  world = current_world;
  meter = new csProgressMeter(world->System, 1000);
  meter->SetGranularity(1);
  pulse = new csProgressPulse();
  // copy data needed, create list and all radpolys
  list = new csRadList();
  // fill list
  csPolyIt *poly_it = new csPolyIt(world);
  csPolygon3D* poly;
  while ( (poly = poly_it->Fetch()) != NULL)
    if(poly->GetUnsplitPolygon() == NULL // only for original polygons 
      &&           // in the list, not the split children also in list.
     poly->GetLightMapInfo()) // only for lightmapped polys
      if(poly->GetLightMapInfo()->GetPolyTex()->GetCSLightMap())
        list->InsertElement(new csRadPoly(poly));
  delete poly_it;
  /// remove ambient light from maps.
  RemoveAmbient();
  texturemap = 0;
  shadow_matrix = 0;
}

csRadiosity :: ~csRadiosity()
{
  delete texturemap;
  delete shadow_matrix;
  delete pulse;
  delete meter;
  // remove data needed.
  delete list;
}

void csRadiosity :: DoRadiosity()
{
  csRadPoly *shoot;

  CsPrintf (MSG_INITIALIZATION, "Calculating radiosity (%d lightmaps):\n",
    list->GetNumElements());
  CsPrintf(MSG_INITIALIZATION, "  ");
  meter->Restart();
  shoot = list->PopHighest();
  if(shoot) {
    start_priority = shoot->GetPriority();
    list->InsertElement(shoot);
  }

  // do the work
  // Take RadPoly with highest unshot light amount, and distribute
  // the light to other polys (incrementing their unshot amount).
  // until stability, in theory, in practice some stop-condition.

  while( (shoot = FetchNext()) != NULL)
  {
    iterations++;
    // shoot the light off of RadPoly.
    //CsPrintf(MSG_STDOUT, "(priority at %f).\n", shoot->GetPriority() );
    pulse->Step();
    // prepare to shoot from source (visibility, precompute, etc)
    PrepareShootSource(shoot);

    // start the frustum calcs.
    StartFrustum();
    // have shot all from shootrad.
    shoot->CopyAndClearDelta();
    list->InsertElement(shoot); 
    pulse->Erase();
  }
  ApplyDeltaAndAmbient();
}


csRadPoly* csRadiosity :: FetchNext()
{
  // you can define any stop moment you like. And stop here anytime you like
  // by returning NULL. The remaining unshot light will be added as
  // ambient light
  bool stop_now = false;
  char reason[80];
  float stop_value = 0.1000; // the amount of unshot light, where we can stop.
  /// take first stop moment, do the least work expected.
  if(stop_priority > start_priority / stop_improvement)
    stop_value = stop_priority;
  else stop_value = start_priority / stop_improvement;
  int max_repeats = 10; // after n loops, stop. Should never be necessary.
    // due to loop detection in csRadPoly. For robustness.

  csRadPoly *p = list->PopHighest();

  float nextpriority = p?p->GetPriority():0.0;
  float val = (start_priority - nextpriority ) / start_priority;
  if(val<0.0f) val=0.0f;
  val = pow(val, 20.0) * 0.98f;
  int ticks_now = QRound( val * meter->GetTotal());
  //CsPrintf(MSG_STDOUT, "New value %g, ticks at %d / %d\n",
  //  val, ticks_now, meter->GetTotal());
  while(meter->GetCurrent() < ticks_now)
    meter->Step();

  if(p==NULL)
  {
    stop_now = true;
    sprintf(reason, "no polygons to light");
  }
  else if(p->GetPriority() < stop_value)
  {
    stop_now = true;
    sprintf(reason, "priority down to %g", p->GetPriority());
  }
  else if(iterations > stop_iterations)
  {
    stop_now = true;
    sprintf(reason, "%d iterations reached", iterations);
  }
  else if(p->GetNumRepeats() > max_repeats)
  {
    stop_now = true;
    sprintf(reason, "loop detected");
  }
  /// more stop conditions can be put here.

  if(stop_now)
  {
    while(meter->GetCurrent() < meter->GetTotal())
      meter->Step();
    CsPrintf(MSG_INITIALIZATION, "\n");
    CsPrintf(MSG_INITIALIZATION, "Finished radiosity (%s).\n", reason);
    list->InsertElement(p); // to prevent memory leak.
    return NULL;
  }

  if(nextpriority == p->GetLastShootingPriority())
    p->IncNumRepeats();
  p->SetLastShootingPriority(nextpriority);
  return p;
}

static void frustum_polygon_report_func (csObject *obj, csFrustumView* lview);
static void frustum_curve_report_func (csObject *obj, csFrustumView* lview);
static csVector3 plane_origin, plane_v1, plane_v2;
void csRadiosity :: StartFrustum()
{
  csFrustumView *lview = new csFrustumView();
  lview->userdata = (void*) this;
  lview->curve_func = frustum_curve_report_func;
  lview->poly_func = frustum_polygon_report_func;
  lview->radius = 10000000.0; // should be enough
  lview->sq_radius = lview->radius * lview->radius;
  lview->things_shadow = csPolyTexture::do_accurate_things; 
  lview->mirror = false;
  lview->gouraud_only = false;
  lview->gouraud_color_reset = false;
  lview->r = 1.0; // the resulting color can be used as a filter
  lview->g = 1.0;
  lview->b = 1.0;
  lview->dynamic = false;
  csVector3 center; // start from the center of the shooting poly.
  // this will lead to inaccuracy as each lumel of the shooting
  // poly is it's own center. But that is too slow.
  // And this leads to sharper shadows as well.
  shoot_src->QuickLumel2World(center, shoot_src->GetWidth()/2.,
    shoot_src->GetHeight()/2.);
  lview->light_frustum = new csFrustum (center);
  lview->light_frustum->MakeInfinite ();
  // add a backplane to frustum to clip to it... But which plane?
  //csPlane3 *src_plane = shoot_src->GetPolygon3D()->GetPolyPlane();
  //lview->light_frustum->SetBackPlane(* src_plane);
  
  /// setup some vectors so we can test on plane location
  plane_origin = shoot_src->GetPolygon3D()->Vwor(0);
  plane_v1 = shoot_src->GetPolygon3D()->Vwor(1) - plane_origin;
  plane_v2 = shoot_src->GetPolygon3D()->Vwor(2) - plane_origin;
  shoot_src->GetPolygon3D()->GetSector()->CheckFrustum (*lview);

  delete lview;
}


static void frustum_curve_report_func (csObject *obj, csFrustumView* lview)
{ 
  (void)obj;
  (void)lview;
  // empty for now
}

static void frustum_polygon_report_func (csObject *obj, csFrustumView* lview)
{
  // radiosity works with the base, unsplit polygon.
  csPolygon3D *destpoly3d = ((csPolygon3D*)obj)->GetBasePolygon();
  csRadPoly *dest = csRadPoly::GetRadPoly(*destpoly3d); // obtain radpoly
  if(!dest) return; // polygon not lightmapped / radiosity rendered.
  // check poly -- on right side of us?
  csVector3 center;
  dest->QuickLumel2World(center, dest->GetWidth()/2., dest->GetHeight()/2.);
  if( csMath3::WhichSide3D(center - plane_origin, plane_v1, plane_v2) >= 0)
    return; // when on the plane or behind, skip.
  // use poly
  csRadiosity *rad = (csRadiosity*)lview->userdata;
  rad->ProcessDest(dest, lview);
}


void csRadiosity :: ProcessDest(csRadPoly *dest, csFrustumView *lview)
{
  if(shoot_src == dest) return; // different polys required. or we 
    			//might requeue 'shoot', and corrupt the list.
  //if(!VisiblePoly(shoot_src, dest)) continue; // done by frustum already

  // check if dest poly has already been shot at this turn.
  // as several split polyes can be hit, returning the original poly.
  if(dest->GetIterCookie() == iterations) return;
  dest->SetIterCookie(iterations);
  // prepare to send/receive light.
  PrepareShootDest(dest, lview);
  ShootRadiosityToPolygon(dest);
  printf("deleteing matrix %x\n", (int)shadow_matrix);
  delete shadow_matrix; 
  shadow_matrix = 0;
  list->DeleteElement(dest); // get out of tree
  dest->ComputePriority(); // recompute dest's unshot light
  list->InsertElement(dest); // and requeue dest
}


void csRadiosity :: ShootRadiosityToPolygon(csRadPoly* dest)
{
  // shoot from each lumel, also a radiosity patch, to each lumel on other.
  //CsPrintf(MSG_STDOUT, "Shooting from RadPoly %x to %x.\n", 
  //	(int)shoot, (int)dest);

  int sx, sy, rx, ry; // shoot x,y, receive x,y
  int suv = 0, ruv = 0; // shoot uv index, receive uv index.
  for(sy=0; sy<shoot_src->GetHeight(); sy++)
   for(sx=0; sx<shoot_src->GetWidth(); sx++, suv++)
   {
     // if source lumel delta == 0 or lumel not visible, skip.
     if(shoot_src->DeltaIsZero(suv)) continue;
     //if(shoot_src->LumelNotCovered(suv)) continue; 
     // get source lumel info
     PrepareShootSourceLumel(sx, sy, suv);
     ruv = 0;
     for(ry=0; ry<dest->GetHeight(); ry++)
       for(rx=0; rx<dest->GetWidth(); rx++, ruv++)
       {
         //if(dest->LumelNotCovered(ruv)) continue;
         ShootPatch(rx, ry, ruv);
       }
   }
}


void csRadiosity :: PrepareShootSource(csRadPoly *src)
{
  shoot_src = src;
  // shoot_normal points to the shooting direction.
  src_normal = - shoot_src->GetNormal();
  source_poly_lumel_area = shoot_src->GetOneLumelArea();
  delete texturemap;
  texturemap = shoot_src->ComputeTextureLumelSized();
}


void csRadiosity :: PrepareShootDest(csRadPoly *dest, csFrustumView *lview)
{
  shoot_dest = dest;
  // compute the factor for the light getting through. The same for
  // every lumel. This factor is expanded to be the formfactor * areafraction

  // * diffusereflection.
  factor = shoot_dest->GetDiffuse() / (float)PI;

  // note that the Normal's must be UnitLength for this to work.
  // to do: factor *= cos(shootangle) * cos(destangle) * H(i,j) * src_area
  //                  ----------------------------------------------------
  //                      dist * dist

  // Note: when bumpmaps arrive, you can make radiosity take them
  // into account. Simply have GetNormal() return the normal vector
  // for the specific lumel-coords. This will shade according to
  // the bump-map.
  // The same sort of thing could be used to make curved surfaces look
  // smoother, i.e. compute the normal for the location again.
  // The Normal must have length 1, in order to work here.

  dest_normal = - shoot_dest->GetNormal();
  // use filter colour from lview
  trajectory_color.Set(lview->r, lview->g, lview->b);

  // use shadows and light from lview
  // gets coverage matrix from polytext.cpp, so the code is shared
  // between regular lighting and radiosity lighting, prevents bugs.
  shadow_matrix = new csPolyTexture::csCoverageMatrix(
    dest->GetWidth(), dest->GetHeight());
  printf("created matrix %x\n", (int)shadow_matrix);
  //for(int i=0; i<dest->GetSize(); i++)
  //  static_shadow_map[i] = 0.0; // start with none visible.
  dest->GetPolygon3D()->GetLightMapInfo()->GetPolyTex()->GetCoverageMatrix(
    *lview, *shadow_matrix);
}


void csRadiosity :: PrepareShootSourceLumel(int sx, int sy, int suv)
{
  src_uv = suv;
  shoot_src->QuickLumel2World(src_lumel, sx, sy);
  /// use the size of a lumel in the source poly *
  /// the amount of the lumel visible to compute area of sender.
  /// factor is included, which saves a lot of multiplications.
  source_patch_area = factor * source_poly_lumel_area;
    // * shoot_src->GetLumelCoverage(suv); 
  // color is texture color filtered by trajectory (i.e. half-transparent-
  // portals passed by from source to dest poly)
  src_lumel_color.red = texturemap->GetRed()[suv] / 255.0f 
    * trajectory_color.red * colour_bleed;
  src_lumel_color.green = texturemap->GetGreen()[suv] / 255.0f 
    * trajectory_color.green * colour_bleed;
  src_lumel_color.blue = texturemap->GetBlue()[suv] / 255.0f 
    * trajectory_color.blue * colour_bleed;
  // cap source delta to prevent explosions of light
  float cap = 512.0f;
  if(shoot_src->GetDeltaMap()->GetRed()[suv] > cap)
    shoot_src->GetDeltaMap()->GetRed()[suv] = cap;
  if(shoot_src->GetDeltaMap()->GetGreen()[suv] > cap)
    shoot_src->GetDeltaMap()->GetGreen()[suv] = cap;
  if(shoot_src->GetDeltaMap()->GetBlue()[suv] > cap)
    shoot_src->GetDeltaMap()->GetBlue()[suv] = cap;
}


void csRadiosity :: ShootPatch(int rx, int ry, int ruv)
{
  // check visibility
  // old use was: GetVisibility(shoot_src, src_lumel, shoot_dest, dest_lumel);
  float visibility = shadow_matrix->coverage[ruv];
  if(visibility == 0.0) return;

  // prepare dest lumel info
  shoot_dest->QuickLumel2World(dest_lumel, rx, ry);

  // compute formfactors.
  csVector3 path = dest_lumel - src_lumel;
  float distance = path.Norm();
  if(distance < 0.000001) return; // too close together
  path /= distance ; //otherwise dot product will not return cos(angle).
  distance *= distance;
  float cossrcangle = src_normal * path;
  if(cossrcangle < 0.0) return;
  float cosdestangle = - (dest_normal * path);
  if(cosdestangle < 0.0) return; // facing away, negative light is not good.

  float totalfactor = cossrcangle * cosdestangle * 
    source_patch_area * visibility / distance;
  if(totalfactor > 1.0f) totalfactor = 1.0f;

#if 0
    CsPrintf(MSG_STDOUT, "totalfactor %g = "
  	"cosshoot %g * cosdest %g * area %g * vis %g / sqdis %g.  "
	"srclumelcolor (%g, %g, %g)\n", 
  	totalfactor, cossrcangle, cosdestangle,
 	source_patch_area, visibility, distance,
	src_lumel_color.red, src_lumel_color.green, src_lumel_color.blue);
#endif

  shoot_dest->AddDelta(shoot_src, src_uv, ruv, totalfactor, src_lumel_color);
  // specular gloss
  // direction of the 'light' on dest is -path
  // normal at this destlumel is dest_normal
  // viewing direction, below, equal to normal (look at polygon frontally)
  if(!csRadiosity::do_static_specular) 
  {
    return;
  }
  csVector3 viewdir(0.577, 0.577, 0.577);
  viewdir = dest_normal;
  csVector3 reflectdir = (2.0f * dest_normal * (cosdestangle) - -path) * 
    dest_normal;
  double val = ( reflectdir * viewdir );
  if(val<0.0f) return;
  if(val>1.0) val=1.0;
  //alternative specular computation
  //csVector3 halfdir = (-path + viewdir);
  //halfdir.Normalize();
  //double val = ABS( halfdir * dest_normal );
  //float gloss = spec_amt * pow(val, spec_tightness);
  float gloss = static_specular_amount * 
    FastPow2(val, static_specular_tightness);
  
#if 0
    CsPrintf(MSG_STDOUT, "Gloss %g, val=%g, reflect %g,%g,%g\n",
      gloss, val, reflectdir.x, reflectdir.y, reflectdir.z);
#endif

  // add delta using both gloss and totalfactor
  //shoot_dest->AddDelta(shoot_src, src_uv, ruv, gloss*totalfactor, src_lumel_color);

  gloss *= source_patch_area * visibility / distance;
  // add gloss seperately -- too much light this way
  shoot_dest->AddDelta(shoot_src, src_uv, ruv, gloss, trajectory_color );
}


float csRadiosity :: GetVisibility(csRadPoly *srcpoly, const csVector3& src, 
    csRadPoly *destpoly, const csVector3& dest)
{
  // change path slightly, to take floatingpoint inaccuracies into account.
  csVector3 path = dest-src;
  // aim slightly behind destination so we won't miss.
  csVector3 corr_src = src + path * 0.001; // so we won't hit src polygon.
  csVector3 corr_dest = src + path * 1.5; 

  csPolygon3D *hit = srcpoly->GetPolygon3D()->GetSector()->HitBeam(
    corr_src, corr_dest);
  if( hit == destpoly->GetPolygon3D() )
    return 1.0;
  else return 0.0;
}


static volatile float total_delta_color_red;
static volatile float total_delta_color_green;
static volatile float total_delta_color_blue;
static volatile float total_reflect = 0.0;
static volatile float total_area = 0.0;

static void calc_ambient_func(csRadPoly *p)
{
  total_area += p->GetArea();
  total_reflect += p->GetDiffuse() * p->GetArea();
  float red, green, blue;
  p->GetDeltaSums(red, green, blue);
  total_delta_color_red += red * p->GetArea() / (float)p->GetSize();
  total_delta_color_green += green * p->GetArea() / (float)p->GetSize();
  total_delta_color_blue += blue * p->GetArea() / (float)p->GetSize();
}

static void apply_ambient_func(csRadPoly *p)
{
  p->ApplyAmbient(QRound((float)total_delta_color_red), 
    QRound((float)total_delta_color_green), 
    QRound((float)total_delta_color_blue));
}

static void add_delta_func(csRadPoly *p)
{
  p->CopyAndClearDelta();
}

void csRadiosity :: RemoveAmbient()
{
  total_delta_color_red = -csLight::ambient_red;
  total_delta_color_green = -csLight::ambient_green;
  total_delta_color_blue = -csLight::ambient_blue;
  list->Traverse(apply_ambient_func);
}

void csRadiosity :: ApplyDeltaAndAmbient()
{
  /// first calculate the Ambient
  total_delta_color_red = 0.0;
  total_delta_color_green = 0.0;
  total_delta_color_blue = 0.0;
  total_reflect = 0.0;
  total_area = 0.0;
  list->Traverse(calc_ambient_func);
  total_reflect /= total_area;
  total_reflect = 1.0 / (1.0 - total_reflect);
  float mult = total_reflect / total_area;
  total_delta_color_red *= mult;
  total_delta_color_green *= mult;
  total_delta_color_blue *= mult;

  // add in the system ambient light settings.
  total_delta_color_red += csLight::ambient_red;
  total_delta_color_green += csLight::ambient_green;
  total_delta_color_blue += csLight::ambient_blue;
  CsPrintf(MSG_INITIALIZATION, "Setting ambient (%g, %g, %g).\n",
   total_delta_color_red, total_delta_color_green, total_delta_color_blue);

  /// then apply ambient to deltamaps
  list->Traverse(apply_ambient_func);
  /// add deltamaps
  list->Traverse(add_delta_func);
}


bool csRadiosity :: VisiblePoly(csRadPoly *src, csRadPoly *dest)
{
  // for now check the center (u,v) of lightmap hitbeam.
  // thus if the center is shadowed it is still skipped, wrongly.
  csVector3 start, end;
  src->QuickLumel2World(start, src->GetWidth()/2., src->GetHeight()/2.);
  dest->QuickLumel2World(end, dest->GetWidth()/2., dest->GetHeight()/2.);
  return GetVisibility(src, start, dest, end) > 0.0;
}


