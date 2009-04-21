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
#include "prsky.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "csqsqrt.h"
#include "csqint.h"
#include "csgeom/matrix3.h"
#include "iutil/objreg.h"

//---------- csProcSkyTexture ------------------------------------
csProcSkyTexture::csProcSkyTexture(iTextureFactory* p,csProcSky *par)
: csProcTexture(p)
{
  sky = par;
  mat_w = 256;
  mat_h = 256;
  isect = 0;
  forcerender = false;
  next = sky->GetFirstSky();
  sky->SetFirstSky(this);

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

  texFlags = CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS;
}

csProcSkyTexture::~csProcSkyTexture()
{
  delete[] isect;
}

bool csProcSkyTexture::PrepareAnim ()
{
  if (anim_prepared) return true;
  if (!csProcTexture::PrepareAnim ()) return false;
  /// initialise further (using the g2d, txtmgr and so on)
  return true;
}


void csProcSkyTexture::Animate (csTicks current_time)
{
  sky->DrawToTexture (this, current_time, object_reg);
}

//---------- csProcSky -------------------------------------------

csProcSky::csProcSky()
{
  firstsky = 0;
  int i;
  radius = 20000000.0f; /// 20 000 km
  center.Set(0.0f, -radius + 100000.0f, 0.0f); // sky is 100 km high
  cam.Set(0.0f, 0.0f, 0.0f);

  sunpos.Set(0.0f, 0.0f, radius);  // at north  point
  //// try 0.95(edge of sun) -- 1.0(at top) for elevation.
  float sunelevation = 0.99f * HALF_PI; // pi/2 is top.
  float sunazimuth = 0.0f; // 0=north,+z
  csXRotMatrix3 elev(-sunelevation);
  sunpos = elev * sunpos;
  csYRotMatrix3 compassdir(sunazimuth);
  sunpos = compassdir * sunpos;
  sunpos += center;
  suncolor.Set(1.0f, 1.0f, 0.6f);
  maxhaze.Set(130, 150, 255);

  nr_octaves = 5;
  octsize = 32; // octave is octsize x octsize
  octaves = new uint8 [octsize*octsize*nr_octaves];
  enlarged = new uint8* [nr_octaves];
  for(i=0 ; i<nr_octaves; i++)
  {
    /// scale size of this octave
    int sz = 1 << (nr_octaves - i - 1);
    enlarged[i] = new uint8 [sz * sz * octsize * octsize ];
  }

  animated= true;
  old_time = 0;
  startoctaves = new uint8 [octsize*octsize*nr_octaves];
  endoctaves = new uint8 [octsize*octsize*nr_octaves];
  periods = new int [nr_octaves];
  curposition = new int [nr_octaves];
  int aperiod = 10*1000;
  for(i=0 ; i<nr_octaves; i++)
  {
    periods[i] = aperiod;
    curposition[i] = 0;
    aperiod = aperiod*2/3;
  }
  windpos.Set(0.0,0.0);
  winddir.Set(10.,10.);

  Initialize();
}

csProcSky::~csProcSky()
{
  delete[] octaves;
  int i;
  for(i=0 ; i<nr_octaves; i++)
    delete[] enlarged[i];
  delete[] enlarged;
  delete[] startoctaves;
  delete[] endoctaves;
  delete[] periods;
  delete[] curposition;
}


void csProcSky::SetAnimated (iObjectRegistry* object_reg,
	bool anim, csTicks current_time)
{
  animated = anim;
  if(animated && (current_time != 0)) {
    old_time = current_time;
  }
  if(!animated)
  {
    /// force rerender of all skytextures (to bring to equal time)
    csProcSkyTexture *p = firstsky;
    while(p)
    {
      if (p->AnimPrepared())
      {
        p->ForceRerender();
        DrawToTexture (p, current_time, object_reg);
      }
      p = p->GetNextSky();
    }
  }
}

void csProcSky::Initialize()
{
  /// init every octave
  int i;
  for(i=0 ; i< nr_octaves; i++)
  {
    InitOctave(startoctaves, i);
    InitOctave(endoctaves, i);
    CopyOctave(startoctaves, i, octaves, i);
    SmoothOctave(octaves, i, 2);
    Enlarge( enlarged[i], octaves + octsize*octsize*i, nr_octaves - i - 1, i);
  }
}

void csProcSky::InitOctave(uint8 *octs, int nr)
{
  int sz = octsize*octsize;
  uint8* myoct = octs + nr*sz;
  int i;
  for(i=0; i<sz; i++)
    *myoct++ = (uint8)( rand()&0xFF );
}


void csProcSky::SmoothOctave(uint8 *octs, int nr, int smoothpower)
{
  int sz = octsize*octsize;
  uint8* myoct = new uint8 [sz];
  /// make a backup for perfect smoothing
  memcpy(myoct, octs+sz*nr, sz);
  /// smooth
  int sm = smoothpower, y, x, ix, iy, tot=0;
  for(y=0; y<octsize; y++)
    for(x=0; x<octsize; x++)
    {
      for(ix=-sm; ix<=+sm; ix++)
        for(iy=-sm; iy<=+sm; iy++)
          tot += myoct[(x+ix+octsize)%octsize +
	    ((y+iy+octsize)%octsize)*octsize];
      tot /= (2*sm+1)*(2*sm+1);
      SetOctave(octs, nr, x, y, tot);
    }
  delete[] myoct;
}


void csProcSky::Enlarge(uint8* dest, uint8* src, int factor, int rshift)
{
  int srcsize = octsize;
  int zoom = 1<<factor;
  int destsize = srcsize*zoom;
  //// smooth in squares of zoom x zoom
  int sy, sx;
  for(sy = 0; sy <srcsize; sy++)
    for(sx = 0; sx <srcsize; sx++)
    {
      /// the topleft/topright/botleft/botright values of the square
      int topleft = src[sy*srcsize + sx] << 6;
      int botleft = src[ ( (sy+1)%srcsize )*srcsize + sx] << 6;
      int topright = src[sy*srcsize + (sx+1)%srcsize] << 6;
      int botright = src[ ( (sy+1)%srcsize )*srcsize + (sx+1)%srcsize] << 6;
      /*
      int topleft = GetOctave(0, sx, sy) << 6;
      int topright = GetOctave(0, (sx+1)%srcsize, sy) << 6;
      int botleft = GetOctave(0, sx, (sy+1)%srcsize) << 6;
      int botright = GetOctave(0, (sx+1)%srcsize, (sy+1)%srcsize) << 6;
      */

      int leftinc = (botleft - topleft) >> factor;
      int rightinc = (botright - topright) >> factor;
      int leftval = topleft;  /// these will walk with y
      int rightval = topright;
      /// the start of the horizontal line in dest;
      uint8* dypos= dest + sy*zoom*destsize;
      /// smooth 1 square onto dest
	  int dy, dx;
      for(dy = 0; dy<zoom; dy++)
      {
        int horinc = (rightval - leftval) >> factor;
		int horval = leftval;
		uint8* dxpos = dypos + sx*zoom;
		for(dx = 0; dx<zoom; dx++)
		{
		  *dxpos++ = horval >> (6+rshift);
		  horval += horinc;
		}
		leftval += leftinc;
		rightval += rightinc;
		dypos += destsize;
      }
    }
}


void csProcSky::CopyOctave(uint8 *srcocts, int srcnr, uint8 *destocts,
  int destnr)
{
  memcpy( destocts + octsize*octsize*destnr, srcocts + octsize*octsize*srcnr,
    octsize*octsize );
}


void csProcSky::Combine(uint8 *dest, uint8 *start, uint8 *end, int pos,
  int max, int nr)
{
  int sz = octsize * octsize;
  uint8 *dp = dest + sz * nr;
  uint8 *sp = start + sz * nr;
  uint8 *ep = end + sz * nr;
  int epow = max - pos; // end power, reverse position
  /// pos = strength of end
  /// epow = strength of start
  /// max = total strength
  int i;
  for(i=0; i<sz; i++)
  {
    //float res = float(pos) * float(*ep++) + float(epow)*float(*sp++);
    //*dp++ = int(res / float(max));
    *dp++ = (pos*int(*ep++) + epow*int(*sp++))/max;
  }
}


void csProcSky::AnimOctave(int nr, int elapsed)
{
  curposition[nr] += elapsed;
  if(curposition[nr] >= periods[nr])
  {
    // next cycle
    curposition[nr] -= periods[nr];
    if(curposition[nr] > periods[nr])
    {
      // wow - very long elapsed time, more than two periods
      // totally new start & end points
      InitOctave(startoctaves, nr);
      InitOctave(endoctaves, nr);
      curposition[nr] %= periods[nr]; // position in new period
    }
    else
    {
      // end copied to start, make new random end point.
      CopyOctave(endoctaves, nr, startoctaves, nr);
      InitOctave(endoctaves, nr);
    }
  }
  //// start & end are OK, we know the current position < period.
  Combine(octaves, startoctaves, endoctaves, curposition[nr], periods[nr], nr);
  /// smooth resulting octave
  SmoothOctave(octaves, nr, 2);
  /// pre-enlarge the octave
  Enlarge(enlarged[nr], octaves + octsize*octsize*nr, nr_octaves - nr - 1, nr);
}


bool csProcSky::SphereIntersect(const csVector3& point, csVector3& isect)
{
  //csVector3 pt = point - center;  /// texture point
  /// center sphere now at 0,0,0.
  csVector3 pd = point - cam; /// diff point
  csVector3 pc = cam - center; /// camera point

  float a = pd.SquaredNorm();
  float b = 2.0f * pc * pd;
  float c = center.SquaredNorm() - 2.0f * (cam*center) +
    cam.SquaredNorm() - radius*radius;
  float discrim = b * b - 4.0f * a * c;
  if(discrim < 0.0f) return false;
  float div = 1.0f / (2.0f * a); /// do the div only once
  float sqdis = csQsqrt(discrim); /// and the sqrt only once.
  /// the positive mu solution is in the direction viewed.
  float mu = div * (-b + sqdis);
  if(mu < 0.0f) mu = div * (-b - sqdis);

  isect = cam + mu * pd;
  return true;
}


float csProcSky::GetSundist(const csVector3& spot)
{
  float rad = (radius - (cam.y - center.y));
  float sundist = (spot-sunpos).SquaredNorm()/ (rad*rad);
  sundist /= 5.;
  return sundist;
}


csRGBcolor csProcSky::GetSkyBlue(const csVector3& spot, float& haze,
  float sundist, bool& below)
{
  csRGBcolor res;
  float r, g, b;

  if (spot.y < cam.y)
  {
    haze = 1.0f;   /// ground
    below = true;
    float d = (cam.y-spot.y) / (radius - (cam.y-center.y));
    float mirrorplace = 0.0f;
    if(d > mirrorplace){
      haze = 1.0f - (d-mirrorplace) / 200.0f;
      if(haze < 0.0f) haze = 0.0f;
      //haze += float(rand()&0xFF)/256./10.;
      //if(haze>1.0)haze=1.0;
    }
    if (d > 1.0f) d = 1.0f;
    r = maxhaze.red - d * 50.0f;
    g = maxhaze.green - d * 40.0f;
    b = maxhaze.blue - d * 30.0f;
    float sunmirror = 1.0f;
    if (sundist < sunmirror)
    {
      float m = (sunmirror-sundist) / sunmirror;
      r += suncolor.red * m * 30.0f;
      g += suncolor.green * m * 30.0f;
      b += suncolor.blue * m * 30.0f;
    }
    if (r > 255.0) r = 255.0;
    if (g > 255.0) g = 255.0;
    if (b > 255.0) b = 255.0;
    res.Set(csQround(r), csQround(g), csQround(b));
    return res;
  }
  haze = (spot.y-cam.y) / (radius - (cam.y - center.y));
  haze = 1.0f - haze*haze;

  /// check sun
  if (sundist > 1.0f) sundist = 1.0f;
  sundist = 1.0f - sundist;
  //float glaremoment = 0.5;
  //if(sundist > glaremoment) haze = (sundist-glaremoment)/(1.0-glaremoment);

  float elev = haze; //- sundist;
  //if(elev<0.0)elev=0.0;
  b = maxhaze.blue;
  g = elev * maxhaze.green; 
  r = elev * maxhaze.red;
  elev = 1.0f - elev;
  b -= (elev * 70.0f);

  r += suncolor.red * sundist * 255.0f;
  g += suncolor.green * sundist * 255.0f;
  b += suncolor.blue * sundist * 255.0f;

  //csPrintf("color %d %d %d\n", res.red, res.green, res.blue);
    if (r > 255.0) r = 255.0;
    if (g > 255.0) g = 255.0;
    if (b > 255.0) b = 255.0;
    res.Set(csQround(r), csQround(g), csQround(b));
  return res;
}

uint8 csProcSky::GetCloudVal(int x, int y)
{
  int res = 0;
  int div = 1;
  int i;
  int revdiv = div << (nr_octaves-1);
  int thesize = revdiv*octsize;
  for(i=0; i<nr_octaves; i++)
  {
    /// add octave value div (2**octavenr).
    /// later octaves count for less.  (lessened is precalced)
    /// the x,y is divided by revdiv - later octaves divided less.
    /// later octaves give more precision.
    /// and tiled into the octsize.

    int add = int( enlarged[i][ thesize*(y%thesize)+ x%thesize ]);
    thesize>>=1;

    /*
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
    */
    res += add ;
    //div <<= 1;
    //revdiv >>=1;
  }
  //res>>=1;  /// do not do this
  //res = int(GetOctave(0, (x/8)%octsize, (y/8)%octsize )) ;

  /// now overcast sky cloud value.
  /// lessen cloudyness.
  /// res in (200-300);
#if 0
  int lessen = 180;
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
#endif
  int cloudmin = 180;
  int cloudlen = 400 - cloudmin;
  res = (res-cloudmin)*255/cloudlen;
  res = (res*res)>>7; //// square the cloudyness

  if(res>255) res=255;
  return (uint8)res;
}

void csProcSky::DrawToTexture (csProcSkyTexture *skytex, csTicks current_time,
  iObjectRegistry*)
{
  int i;
  csVector3 txtorig, txtu, txtv;
  skytex->GetTextureSpace(txtorig, txtu, txtv);
  int width = skytex->GetWidth();
  int height = skytex->GetHeight();

  /// if it already has a texture cache (it has been drawn to in the past)
  /// and we do not animate, and no rerender is forced,
  /// then nothing needs to be done
  if(!skytex->MustRender() && skytex->GetIntersect() && !animated) return;

  // if the texture has no cache, make one
  if(!skytex->GetIntersect()) MakeIntersectCache(skytex);
  /// animate the octaves
  if(animated)
  {
    int elapsed_time = int(current_time) - int(old_time);
    if(elapsed_time > 0)
    {
      for(i=0; i<nr_octaves; i++)
        AnimOctave(i, elapsed_time);
      windpos += winddir * (float(elapsed_time)*.001);
    }
    old_time = current_time;
  }

  iGraphics3D* g3d = skytex->GetG3D ();
  iGraphics2D* g2d = skytex->GetG2D ();
  g3d->SetRenderTarget (skytex->GetTextureWrapper ()->GetTextureHandle (),
  	true);
  if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS)) return;

  csVector3 texelu = txtu / float(width);
  csVector3 texelv = txtv / float(height);
  txtorig += 0.5f*(texelu+texelv);
  csVector3 isect;
  // csVector3 spot;
  int y, x;
  for(y=0; y<height; y++)
    for(x=0; x<width; x++)
    {
      /*    // 20 msec
      /// get texel pt in 3d space
      spot = txtorig + texelu*float(x) + texelv*float(y);
      if(!SphereIntersect(spot, isect))
      {
        //csPrintf("no intersection!\n");
	isect = center; isect.z += radius;
      }
      */
      isect = skytex->GetIntersect()[ y*width+x ];

      float sundist = GetSundist(isect); /* 5 msec */
      bool below = false;
      float haze=0.0;
      csRGBcolor blue = GetSkyBlue(isect, haze, sundist, below); /* 35 msec */
      ///
      int cloud;
      if(!below)
      {
        float cloudx = 1024.+(isect.x - center.x)/radius*255.*40.+windpos.x;
        float cloudy = 1024.+(isect.z - center.z)/radius*255.*40.+windpos.y;
        if(cloudx<0.0)cloudx = -cloudx;
        if(cloudy<0.0)cloudy = -cloudy;
        cloud = GetCloudVal((int)cloudx, (int)cloudy ); /* 80 msec */
	//cloud = 0;
        //cloud = int(cloudx)%256 + int(cloudy)%256; /// nice patterns :)
        //cloud = GetCloudVal(x,y); // debug
      }
      else cloud = 0;
      csRGBcolor clcol(cloud, cloud, cloud);
      if(sundist>3.)
      {
        int sunshadow = cloud - int(sundist)/3;
	if(sunshadow<0)sunshadow=0;
	clcol.Set(sunshadow,sunshadow,sunshadow);
      }
      int hazefact = int(haze*255.);
      //if(cloud<64) hazefact=255;
      if(hazefact<255-cloud) hazefact = 255-cloud;

      int r,g,b;
      r = (blue.red + clcol.red*(255-hazefact)/64);
      g = (blue.green+ clcol.green*(255-hazefact)/64);
      b = (blue.blue+ clcol.blue*(255-hazefact)/64);
      int col = g2d->FindRGB(r, g, b);
      g2d->DrawPixel(x,height-y-1,col);
    }

  g3d->FinishDraw();

  /// did a rendering
  skytex->UnsetForceRender();
}


void csProcSky::MakeIntersectCache(csProcSkyTexture *skytex)
{
  if(skytex->GetIntersect() != 0)
  {
    delete[] skytex->GetIntersect();
    skytex->SetIntersect(0);
  }
  int width = skytex->GetWidth();
  int height = skytex->GetHeight();
  csVector3 *cache = new csVector3 [ width*height ] ;
  skytex->SetIntersect(cache);

  /// fill the cache
  csVector3 txtorig, txtu, txtv;
  skytex->GetTextureSpace(txtorig, txtu, txtv);
  csVector3 texelu = txtu / float(width);
  csVector3 texelv = txtv / float(height);
  txtorig += 0.5f*(texelu+texelv);
  csVector3 spot, isect;
  int y, x;
  for(y=0; y<height; y++)
    for(x=0; x<height; x++)
    {
      /// get texel pt in 3d space
      spot = txtorig + texelu*float(x) + texelv*float(y);
      if(!SphereIntersect(spot, isect))
      {
        //csPrintf("no intersection!\n");
	isect = center; isect.z += radius;
      }
      cache [ y*width+x ] = isect;
    }
}

