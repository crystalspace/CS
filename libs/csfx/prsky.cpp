/*
    Copyright (C) 2001 by W.C.A. Wijngaards

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
#include "csfx/prsky.h"
#include "igraph2d.h"
#include "igraph3d.h"
#include "itxtmgr.h"
#include "qsqrt.h"
#include "csgeom/matrix3.h"

//---------- csProcSkyTexture ------------------------------------

IMPLEMENT_CSOBJTYPE (csProcSkyTexture, csProcTexture);

csProcSkyTexture::csProcSkyTexture(csProcSky *par) : csProcTexture()
{
  sky = par;
  mat_w = 256;
  mat_h = 256;

  if(0)
  {
  /// looking towards +y
  txtorig.Set(-500.,50.,-500.); // topleft point
  txtu.Set(1000.,0.,0.); // right dir 
  txtv.Set(0.,0.,1000.); // down dir 
  }
  else
  {
  /// looking towards +z
  txtorig.Set(-500.,500.,2000.); // topleft point
  txtu.Set(1000.,0.,0.); // right dir 
  txtv.Set(0.,-1000.,0.); // down dir 
  }

  texFlags = CS_TEXTURE_3D | CS_TEXTURE_PROC | CS_TEXTURE_NOMIPMAPS | 
    CS_TEXTURE_PROC_ALONE_HINT;
}

csProcSkyTexture::~csProcSkyTexture() 
{
}

bool csProcSkyTexture::PrepareAnim ()
{
  if (!csProcTexture::PrepareAnim ()) return false;
  /// initialise further (using the g2d, txtmgr and so on)
  return true;
}


void csProcSkyTexture::Animate (cs_time current_time)
{
  (void)current_time;
  sky->DrawToTexture(this);
}

//---------- csProcSky -------------------------------------------

csProcSky::csProcSky()
{
  radius = 20000000.; /// 20 000 km
  center.Set(0., -radius + 100000.0, 0.); // sky is 100 km high
  cam.Set(0.,0.,0.);

  sunpos.Set(0,0,radius);  // at north  point
  //// try 0.95(edge of sun) -- 1.0(at top) for elevation.
  float sunelevation = 0.99 * PI/2.0; // pi/2 is top.
  float sunazimuth = 0.0; // 0=north,+z
  csXRotMatrix3 elev(-sunelevation);
  sunpos = elev * sunpos;
  csYRotMatrix3 compassdir(sunazimuth);
  sunpos = compassdir * sunpos;
  sunpos += center;
  suncolor.Set(1.0,1.0,0.6);

  nr_octaves = 4;
  octsize = 32; // octave is octsize x octsize
  octaves = new uint8 [octsize*octsize*nr_octaves];

  Initialize();
}

csProcSky::~csProcSky()
{
  delete[] octaves;
}


void csProcSky::Initialize()
{
  /// init every octave
  int i;
  for(i=0 ; i< nr_octaves; i++)
    InitOctave(i);
}

void csProcSky::InitOctave(int nr)
{
  int sz = octsize*octsize;
  uint8* myoct = new uint8[sz];
  int i;
  for(i=0; i<sz; i++)
    myoct[i] = (uint8)( rand()&0xFF );
  int sm = 2;
  for(int y=0; y<octsize; y++)
    for(int x=0; x<octsize; x++)
    {
      int tot= 0;
      for(int ix=-sm; ix<=+sm; ix++)
        for(int iy=-sm; iy<=+sm; iy++)
          tot += myoct[(x+ix+octsize)%octsize + 
	    ((y+iy+octsize)%octsize)*octsize];
      tot /= (2*sm+1)*(2*sm+1);
      SetOctave(nr,x,y,tot);
    }
  delete myoct;
}

bool csProcSky::SphereIntersect(const csVector3& point, csVector3& isect)
{
  //csVector3 pt = point - center;  /// texture point
  /// center sphere now at 0,0,0.
  csVector3 pd = point - cam; /// diff point
  csVector3 pc = cam - center; /// camera point

  float a = pd.SquaredNorm();
  float b = 2.0f * pc * pd;
  float c = center.SquaredNorm() - 2.0*(cam*center) +
    cam.SquaredNorm() - radius*radius;
  float discrim = b*b - 4.0*a*c;
  if(discrim < 0.0) return false;
  float div = 1.0 / (2.0 * a); /// do the div only once
  float sqdis = qsqrt(discrim); /// and the sqrt only once.
  /// the positive mu solution is in the direction viewed.
  float mu = div * (-b + sqdis);
  if(mu < 0.0) mu = div * (-b - sqdis);

  isect = cam + mu * pd;
  return true;
}


float csProcSky::GetSundist(const csVector3& spot)
{
  float sundist = (spot-sunpos).Norm()/ (radius - (cam.y-center.y));
  sundist /=10.;
  return sundist;
}


csRGBcolor csProcSky::GetSkyBlue(const csVector3& spot, float& haze,
  float sundist)
{
  csRGBcolor res;
  int r,g,b;
  csRGBcolor maxhaze(130,150,255);

  if(spot.y < cam.y) 
  {
    haze = 1.0;   /// ground
    float d = (cam.y-spot.y)/(radius - (cam.y-center.y));
    float mirrorplace = 0.0;
    if(d>mirrorplace){
      haze = 1.0-(d-mirrorplace)/200. ;
      if(haze<0.0)haze=0.0;
      haze += float(rand()&0xFF)/256./10.;
      if(haze>1.0)haze=1.0;
    }
    if(d>1.0)d=1.0;
    r = int( (maxhaze.red) - d*50.);
    g = int( (maxhaze.green) - d*40.);
    b = int( (maxhaze.blue) - d*30.);
    float sunmirror = 1.0;
    if(sundist < sunmirror)
    {
      float m = (sunmirror-sundist)/sunmirror;
      r += int( (suncolor.red)*m*30.);
      g += int( (suncolor.green)*m*30.);
      b += int( (suncolor.blue)*m*30.);
    }
    if(r>255)r=255;
    if(g>255)g=255;
    if(b>255)b=255;
    res.Set(r,g,b);
    return res;
  }
  haze = (spot.y-cam.y)/(radius - (cam.y-center.y));
  haze = 1.0 - haze*haze;

  /// check sun
  if(sundist> 1.0) sundist = 1.0;
  sundist = 1.0-sundist;
  //float glaremoment = 0.5;
  //if(sundist > glaremoment) haze = (sundist-glaremoment)/(1.0-glaremoment);

  float elev = haze; //- sundist;
  //if(elev<0.0)elev=0.0;

  b = maxhaze.blue;
  g = (int)( elev*float(maxhaze.green) );
  r = (int)( elev*float(maxhaze.red) );
  elev = 1.0 - elev;
  b -= (int)( elev*70.0 );

  r += (int)( suncolor.red*sundist*255.);
  g += (int)( suncolor.green*sundist*255.);
  b += (int)( suncolor.blue*sundist*255.);

  //printf("color %d %d %d\n", res.red, res.green, res.blue);
  if(r>255)r=255;
  if(g>255)g=255;
  if(b>255)b=255;
  res.red = r;
  res.green = g;
  res.blue =b;
  return res;
}

uint8 csProcSky::GetCloudVal(int x, int y)
{
  int res = 0;
  int div = 1;
  int i;
  int revdiv = div << (nr_octaves-1);
  for(i=0; i<nr_octaves; i++)
  {
    /// add octave value div (2**octavenr).
    /// later octaves count for less.
    /// the x,y is divided by revdiv - later octaves divided less.
    /// later octaves give more precision.
    /// and tiled into the octsize.
    int add = 0 ;
    ///olddirect:int(GetOctave(i, (x/revdiv)%octsize, (y/revdiv)%octsize ))>>i;
    /// get interpolated adding value.
    int add1 = (revdiv-x%revdiv)*
      (int(GetOctave(i, (x/revdiv)%octsize, (y/revdiv)%octsize ))>>i)
      + (x%revdiv)*
      (int(GetOctave(i, (x/revdiv + 1)%octsize, (y/revdiv)%octsize ))>>i);
    int add2 = (revdiv-x%revdiv)*
      (int(GetOctave(i, (x/revdiv)%octsize, (y/revdiv+1)%octsize ))>>i)
      + (x%revdiv)*
      (int(GetOctave(i, (x/revdiv + 1)%octsize, (y/revdiv+1)%octsize ))>>i);
    add = add1*(revdiv-y%revdiv) + add2*(y%revdiv);
    add /= revdiv*revdiv;
    res += add ;
    div <<= 1;
    revdiv >>=1;
  }
  //res>>=1;  /// do not do this
  //res = int(GetOctave(0, (x/8)%octsize, (y/8)%octsize )) ;

  /// now overcast sky cloud value.
  /// lessen cloudyness.
  /// res in (200-300);
  int lessen = 20;
  res -= lessen;
  if(res<0)res=0;
  int cloudyness = 270;
  if(res<cloudyness)
  {
    int minres = res-200;
    if(minres<0)minres=0;
    res = minres*res/50;
  }
  // res*res/cloudyness;
  else res = res;

  if(res>255) res=255;
  return (uint8)res;
}

void csProcSky::DrawToTexture(csProcSkyTexture *skytex)
{
  csVector3 txtorig, txtu, txtv;
  skytex->GetTextureSpace(txtorig, txtu, txtv);
  int width = skytex->GetWidth();
  int height = skytex->GetHeight();
  iGraphics2D *g2d = skytex->GetG2D();
  iTextureManager *txtmgr = skytex->GetTextureManager();

  if (!skytex->GetG3D()->BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  csVector3 texelu = txtu / float(width);
  csVector3 texelv = txtv / float(height);
  csVector3 spot, isect;
  for(int y=0; y<height; y++)
    for(int x=0; x<width; x++)
    {
      /// get texel pt in 3d space
      spot = txtorig + texelu*float(x) + texelv*float(y);
      if(!SphereIntersect(spot, isect))
      {
        //printf("no intersection!\n");
	isect = center; isect.z += radius;
      }

      float sundist = GetSundist(isect);
      /// 
      float cloudx = 1024.+(isect.x - center.x)/radius*255.*20.;
      float cloudy = 1024.+(isect.z - center.z)/radius*255.*20.;
      if(cloudx<0.0)cloudx = -cloudx;
      if(cloudy<0.0)cloudy = -cloudy;
      int cloud = GetCloudVal((int)cloudx, (int)cloudy );
      //cloud = int(cloudx)%256 + int(cloudy)%256; /// nice patterns :)
      //cloud = GetCloudVal(x,y); // debug
      csRGBcolor clcol(cloud, cloud, cloud);
      if(sundist>3.)
      {
        int sunshadow = cloud - int(sundist)/3;
	if(sunshadow<0)sunshadow=0;
	clcol.Set(sunshadow,sunshadow,sunshadow);
      }
      float haze=0.0;
      csRGBcolor blue = GetSkyBlue(isect, haze, sundist);
      int hazefact = int(haze*255.);
      if(cloud<64) hazefact=255;
      if(hazefact<255-cloud) hazefact = 255-cloud;

      int r,g,b;
      r = (blue.red + clcol.red*(255-hazefact)/64);
      g = (blue.green+ clcol.green*(255-hazefact)/64);
      b = (blue.blue+ clcol.blue*(255-hazefact)/64);
      int col = txtmgr->FindRGB(r, g, b);
      g2d->DrawPixel(x,y,col);
    }

  skytex->GetG3D()->FinishDraw();
  skytex->GetG3D()->Print(NULL);
}

