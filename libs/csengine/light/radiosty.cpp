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
#include "iimage.h"
#include "ilghtmap.h"
#include "itexture.h"
#include <math.h>
#include "qint.h"

//--------------- csRadPoly --------------------------------------

IMPLEMENT_CSOBJTYPE(csRadPoly, csObject);

csRadPoly :: csRadPoly(csPolygon3D *original)
{
  polygon = original;
  polygon->ObjAdd(this); // attach to original
  area = original->GetArea();
  total_unshot_light = area;
  //lightmap = polygon->GetLightMapInfo()->GetLightMap(); // returns the
  // 'real' lightmap, which will later get dynamic lights included.
  // but we need the static lightmap
  csmap = polygon->GetLightMapInfo()->GetPolyTex()->GetCSLightMap();
  width = csmap->GetRealWidth();
  height = csmap->GetRealHeight();
  size = csmap->GetSize();
  lightmap = &csmap->GetStaticMap();
  deltamap = new csRGBLightMap();
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
  csPolyPlane* pl = polygon->GetPlane ();
  csMatrix3 *m_world2tex;
  csVector3 *v_world2tex;
  txt_pl->GetWorldToTexture(m_world2tex, v_world2tex);
  csMatrix3 m_t2w = m_world2tex->GetInverse ();
  csVector3 vv = *v_world2tex;
  float A = pl->GetWorldPlane ().A ();
  float B = pl->GetWorldPlane ().B ();
  float C = pl->GetWorldPlane ().C ();
  float D = pl->GetWorldPlane ().D ();
  float txt_A = A*m_t2w.m11 + B*m_t2w.m21 + C*m_t2w.m31;
  float txt_B = A*m_t2w.m12 + B*m_t2w.m22 + C*m_t2w.m32;
  float txt_C = A*m_t2w.m13 + B*m_t2w.m23 + C*m_t2w.m33;
  float txt_D = A*v_world2tex->x + B*v_world2tex->y + C*v_world2tex->z + D;

  csVector3 v1, v2;
  int lightcell_shift = csPolygon3D::lightcell_shift;
  int ru = x << lightcell_shift;
  int rv = y << lightcell_shift;
  int Imin_u = polytext->GetIMinU();
  int Imin_v = polytext->GetIMinV();
  v1.x = (float)(ru + Imin_u) * invww;
  v1.y = (float)(rv + Imin_v) * invhh;
  if (ABS (txt_C) < SMALL_EPSILON)
    v1.z = 0;
  else
    v1.z = - (txt_D + txt_A*v1.x + txt_B*v1.y) / txt_C;
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
  //@@@ should take in the fact that not all lumels are covered by polygon
  //but a bit of imprecision here doesn't matter too much, compute longer :)
  int sum = 0;
  for(int x=0; x<size*3; x++)
    sum += deltamap->GetMap()[x];
  float mean_lightval = sum / (float)(size*3);
  total_unshot_light = GetDiffuse() * area * mean_lightval;
  //CsPrintf(MSG_STDOUT, "RP %s, pri %g, area %g, sum %d, meanval %g ",
  //  polygon->GetName(), total_unshot_light, area, sum, mean_lightval);
  //CsPrintf(MSG_STDOUT, "w %d, h %d, size %d\n", width, height, size);
}

void csRadPoly :: AddDelta(csRadPoly *src, int suv, int ruv, float fraction,
  const csColor& filtercolor)
{
  int res;
  res = deltamap->GetRed()[ruv] + 
    QRound(fraction * src->deltamap->GetRed()[suv] * filtercolor.red);
  if(res > 255) res = 255;
  else if(res < 0) res = 0;
  deltamap->GetRed()[ruv] = res;

  res = deltamap->GetGreen()[ruv] + 
    QRound(fraction * src->deltamap->GetGreen()[suv] * filtercolor.green);
  if(res > 255) res = 255;
  else if(res < 0) res = 0;
  deltamap->GetGreen()[ruv] = res;

  res = deltamap->GetBlue()[ruv] + 
    QRound(fraction * src->deltamap->GetBlue()[suv] * filtercolor.blue);
  if(res > 255) res = 255;
  else if(res < 0) res = 0;
  deltamap->GetBlue()[ruv] = res;
}


void csRadPoly :: CopyAndClearDelta()
{
  int res;
  for(int uv=0; uv<size*3; uv++)
  {
    res = lightmap->GetMap()[uv] + deltamap->GetMap()[uv];
    if(res > 255) res = 255;
    else if(res < 0) res = 0;
    lightmap->GetMap()[uv] = res;
  }
  memset( deltamap->GetMap(), 0, size*3);
  total_unshot_light = 0.0;
}

void csRadPoly :: GetDeltaSums(int &red, int &green, int &blue)
{
  red = 0;
  green = 0;
  blue = 0;
  for(int uv=0; uv<size; uv++)
  {
    red += deltamap->GetRed()[uv];
    green += deltamap->GetGreen()[uv];
    blue += deltamap->GetBlue()[uv];
  }
}

void csRadPoly :: ApplyAmbient(int red, int green, int blue)
{
  int res;
  for(int uv=0; uv<size; uv++)
  {
    res = lightmap->GetRed()[uv] + red;
    if(res>255)res=255; else if(res<0) res=0;
    lightmap->GetRed()[uv] = res;

    res = lightmap->GetGreen()[uv] + green;
    if(res>255)res=255; else if(res<0) res=0;
    lightmap->GetGreen()[uv] = res;

    res = lightmap->GetBlue()[uv] + blue;
    if(res>255)res=255; else if(res<0) res=0;
    lightmap->GetBlue()[uv] = res;
  }
}


/// the map and the width to draw in for draw_lumel_cov
static int draw_lumel_cov_width = 0;
static float *draw_lumel_coverage_map = 0;
static void draw_lumel_cov( int x, int y, float density )
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
  float inv_lightcell_size = 1.0 / csPolygon3D::lightcell_size;
  for (i = 0; i < rpv; i++)
  {
    projector = (*m_world2tex) * (polygon->Vwor (i) - (*v_world2tex));
    rp [i].x = (projector.x * ww - Imin_u) * inv_lightcell_size + 0.5;
    rp [i].y = (projector.y * hh - Imin_v) * inv_lightcell_size + 0.5;
  }
  // draw the polygon in lumel space.
  csPolyTexture::SetupPolyFill( draw_lumel_cov );
  csPolyTexture::DoPolyFill( 0,0, width, height, rpv, rp);

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

  delete rp;
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

  int lightcell_size = csPolygon3D::lightcell_size;
  int lightcell_shift = csPolygon3D::lightcell_shift;

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

csRadiosity :: csRadiosity(csWorld *current_world)
{
  CsPrintf (MSG_INITIALIZATION, "\nPreparing radiosity...\n");
  iterations = 0;
  world = current_world;
  meter = new csProgressMeter(world->System, 1000);
  meter->SetGranularity(1);
  // copy data needed, create list and all radpolys
  list = new csRadList();
  // fill list
  csPolyIt *poly_it = new csPolyIt(world);
  csPolygon3D* poly;
  while ( (poly = poly_it->Fetch()) != NULL)
    if(poly->GetLightMapInfo()) // only for lightmapped polys
      if(poly->GetLightMapInfo()->GetPolyTex()->GetCSLightMap())
        list->InsertElement(new csRadPoly(poly));
  delete poly_it;
  /// remove ambient light from maps.
  RemoveAmbient();
  texturemap = 0;
}

csRadiosity :: ~csRadiosity()
{
  delete texturemap;
  delete meter;
  // remove data needed.
  delete list;
}

void csRadiosity :: DoRadiosity()
{
  csRadPoly *shoot;
  csRadPoly *dest; csPolygon3D *destpoly3d;

  CsPrintf (MSG_INITIALIZATION, "Calculating radiosity (%d lightmaps):\n",
    list->GetNumElements());
  CsPrintf(MSG_INITIALIZATION, "  ");
  meter->Restart();
  csProgressPulse *pulse = new csProgressPulse();
  shoot = list->PopHighest();
  if(shoot) {
    start_priority = shoot->GetPriority();
    list->InsertElement(shoot);
  }

  // do the work
  // Take RadPoly with highest unshot light amount, and distribute
  // the light to other polys (incrementing their unshot amount).
  // until stability, in theory, in practice some stop-condition.
  csPolyIt *poly_list = new csPolyIt(world);
  while( (shoot = FetchNext()) != NULL)
  {
    iterations++;
    // shoot the light off of RadPoly.
    CsPrintf(MSG_STDOUT, "(priority at %f).\n", shoot->GetPriority() );
    // make a list of polygons that might be partly visible.
    poly_list->Restart(); //@@@ could be smarter than 'all'.
    // prepare to shoot from source (visibility, precompute, etc)
    PrepareShootSource(shoot);
    while( (destpoly3d = poly_list->Fetch()) != NULL)
    {
      // If dest is wholly invisible from shoot, we can skip dest.
      dest = csRadPoly::GetRadPoly(*destpoly3d); // obtain radpoly
      if(!dest) continue; // polygon not lightmapped / radiosity rendered.
      if(shoot == dest) continue; // different polys required. or we 
      			//might requeue 'shoot', and corrupt the list.
      if(!VisiblePoly(shoot, dest)) continue;
      pulse->Step();
      ShootRadiosityToPolygon(dest);
      list->DeleteElement(dest); // get out of tree
      dest->ComputePriority(); // recompute dest's unshot light
      list->InsertElement(dest); // and requeue dest
    }
    // have shot all from shootrad.
    shoot->CopyAndClearDelta();
    list->InsertElement(shoot); 
    pulse->Erase();
  }
  ApplyDeltaAndAmbient();
  delete pulse;
}


csRadPoly* csRadiosity :: FetchNext()
{
  // you can define any stop moment you like. And stop here anytime you like
  // by returning NULL. The remaining unshot light will be added as
  // ambient light
  bool stop_now = false;
  char reason[80];
  float stop_value = 1.0000; // the amount of unshot light, where we can stop
  /// possibly a ''medium quality fast mode' could be:
  stop_value = start_priority / 1000.0;
  int max_iterations = 1000; // after this amount of shoot src's we stop

  csRadPoly *p = list->PopHighest();

  float nextpriority = p?p->GetPriority():0.0;
  float val = (start_priority - nextpriority ) / start_priority;
  if(val<0.0f) val=0.0f;
  val = pow(val, 15.0) * 0.98f;
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
  else if(iterations > max_iterations)
  {
    stop_now = true;
    sprintf(reason, "%d iterations reached", iterations);
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
  return p;
}


void csRadiosity :: ShootRadiosityToPolygon(csRadPoly* dest)
{
  // shoot from each lumel, also a radiosity patch, to each lumel on other.
  //CsPrintf(MSG_STDOUT, "Shooting from RadPoly %x to %x.\n", 
  //	(int)shoot, (int)dest);
  // prepare to send/receive light.
  PrepareShootDest(dest);

  int sx, sy, rx, ry; // shoot x,y, receive x,y
  int suv = 0, ruv = 0; // shoot uv index, receive uv index.
  for(sy=0; sy<shoot_src->GetHeight(); sy++)
   for(sx=0; sx<shoot_src->GetWidth(); sx++, suv++)
   {
     // if source lumel delta == 0 or lumel not visible, skip.
     if(shoot_src->DeltaIsZero(suv)) continue;
     if(shoot_src->LumelNotCovered(suv)) continue; 
     // get source lumel info
     PrepareShootSourceLumel(sx, sy, suv);
     ruv = 0;
     for(ry=0; ry<dest->GetHeight(); ry++)
       for(rx=0; rx<dest->GetWidth(); rx++, ruv++)
       {
         if(dest->LumelNotCovered(ruv)) continue;
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


void csRadiosity :: PrepareShootDest(csRadPoly *dest)
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
}


void csRadiosity :: PrepareShootSourceLumel(int sx, int sy, int suv)
{
  src_uv = suv;
  shoot_src->QuickLumel2World(src_lumel, sx, sy);
  /// use the size of a lumel in the source poly *
  /// the amount of the lumel visible to compute area of sender.
  source_patch_area = source_poly_lumel_area * 
    shoot_src->GetLumelCoverage(suv); 
  src_lumel_color.red = texturemap->GetRed()[suv] / 255.0;
  src_lumel_color.green = texturemap->GetGreen()[suv] / 255.0;
  src_lumel_color.blue = texturemap->GetBlue()[suv] / 255.0;
}


void csRadiosity :: ShootPatch(int rx, int ry, int ruv)
{
  // get dest lumel info
  shoot_dest->QuickLumel2World(dest_lumel, rx, ry);

  // check visibility
  float visibility = GetVisibility(shoot_src, src_lumel, shoot_dest, dest_lumel);
  if(visibility == 0.0) return;

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

  float totalfactor = factor * cossrcangle * cosdestangle * 
    source_patch_area * visibility / distance;

  if(0)
    CsPrintf(MSG_STDOUT, "totalfactor %g = factor %g * "
  	"cosshoot %g * cosdest %g * area %g * vis %g * sqdis %g\n", 
  	totalfactor, factor, cossrcangle, cosdestangle,
 	source_patch_area, visibility, distance);

  // add delta
  shoot_dest->AddDelta(shoot_src, src_uv, ruv, totalfactor, src_lumel_color);
}


float csRadiosity :: GetVisibility(csRadPoly *srcpoly, const csVector3& src, 
    csRadPoly *destpoly, const csVector3& dest)
{
  // Note: it would be smart to project the src polygon to the dest 
  // polygon earlier. Fill an array with poly_fill, and to do an array 
  // lookup here.

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


static csColor total_delta_color;
static float total_reflect = 0.0;
static float total_area = 0.0;

static void calc_ambient_func(csRadPoly *p)
{
  total_area += p->GetArea();
  total_reflect += p->GetDiffuse() * p->GetArea();
  int red, green, blue;
  p->GetDeltaSums(red, green, blue);
  total_delta_color.red += red * p->GetArea() / (float)p->GetSize();
  total_delta_color.green += green * p->GetArea() / (float)p->GetSize();
  total_delta_color.blue += blue * p->GetArea() / (float)p->GetSize();
}

static void apply_ambient_func(csRadPoly *p)
{
  p->ApplyAmbient(QRound(total_delta_color.red), 
    QRound(total_delta_color.green), QRound(total_delta_color.blue));
}

static void add_delta_func(csRadPoly *p)
{
  p->CopyAndClearDelta();
}

void csRadiosity :: RemoveAmbient()
{
  total_delta_color.Set(-csLight::ambient_red,
    -csLight::ambient_green, -csLight::ambient_blue);
  list->Traverse(apply_ambient_func);
}

void csRadiosity :: ApplyDeltaAndAmbient()
{
  /// first calculate the Ambient
  total_delta_color.Set(0,0,0);
  total_reflect = 0.0;
  total_area = 0.0;
  list->Traverse(calc_ambient_func);
  total_reflect /= total_area;
  total_reflect = 1.0 / (1.0 - total_reflect);
  total_delta_color *= total_reflect / total_area;
  // add deltamaps
  list->Traverse(add_delta_func);
  // add in the system ambient light settings.
  total_delta_color.red += csLight::ambient_red;
  total_delta_color.green += csLight::ambient_green;
  total_delta_color.blue += csLight::ambient_blue;
  //CsPrintf(MSG_INITIALIZATION, "Setting ambient (%g, %g, %g).\n",
  // total_delta_color.red, total_delta_color.green, total_delta_color.blue);

  /// then apply delta & ambient
  list->Traverse(apply_ambient_func);
}


bool csRadiosity :: VisiblePoly(csRadPoly *src, csRadPoly *dest)
{
  // for now check the center (u,v) of lightmap hitbeam.
  // thus if the center is shadowed it is still skipped, wrongly.
  csVector3 start, end;
  src->GetLumelWorldCoords(start, src->GetWidth()/2, src->GetHeight()/2);
  dest->GetLumelWorldCoords(end, dest->GetWidth()/2, dest->GetHeight()/2);
  return GetVisibility(src, start, dest, end) > 0.0;
}
