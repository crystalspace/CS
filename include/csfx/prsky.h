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

#ifndef __PROCSKYTEX_H__
#define __PROCSKYTEX_H__

#include "csgeom/math3d.h"
#include "csgfxldr/rgbpixel.h"
#include "csutil/cscolor.h"
#include "csfx/proctex.h"

class csProcSky;

/** 
 * A polygon of a sky.
*/
class csProcSkyTexture : public csProcTexture {
  /// the sky this is a part of
  csProcSky *sky;

  /// texture orig,udir,vdir (in world coordinates as used in the sky)
  csVector3 txtorig, txtu, txtv;

public:
  /// create, given a sky it belongs to.
  csProcSkyTexture(csProcSky *par);
  ///
  virtual ~csProcSkyTexture();

  virtual bool PrepareAnim ();

  /// Draw the next frame.
  virtual void Animate (cs_time current_time);

  /// methods for the Sky parent - get the g2d
  iGraphics2D* GetG2D() const {return ptG2D;}
  /// get the g3d
  iGraphics3D* GetG3D() const {return ptG3D;}
  /// get the texture manager (used for encoding colors)
  iTextureManager* GetTextureManager() const {return ptTxtMgr;}
  /// get the width of this texture
  int GetWidth() const {return mat_w;}
  /// get the height of this texture
  int GetHeight() const {return mat_h;}
  /// get texturespace values
  void GetTextureSpace(csVector3& orig, csVector3& u, csVector3& v) const
  { orig = txtorig; u = txtu; v = txtv; }

  /** 
   * Set the texturemapping of the sky onto this texture
   * txtorig is a corner point of the polygon (say the topleft point)
   *   (in world coordinates)
   * txtu is the vector towards the right for the length of the polygon
   *   (thus txtorig+txtu is the topright point)
   * txtv is the vector towards the bottom for the length of the polygon
   *   (thus txtorig+txtv is the bottomleft point)
  */
  void SetTextureSpace(const csVector3& tex_orig, const csVector3& total_u,
    const csVector3& total_v) {txtorig=tex_orig; txtu=total_u; txtv=total_v;}

  CSOBJTYPE;
};


/**
 * a sky, this represents a whole sphere of sky - so multiple polygons
 * can be rendered to.
*/
class csProcSky {
  /// sphere radius
  float radius;
  /// sphere center
  csVector3 center;
  /// camera point
  csVector3 cam;
  /// sun position
  csVector3 sunpos;
  /// sun's color
  csColor suncolor;

  /// nr of octaves of the clouds.
  int nr_octaves;
  /// size of an octave
  int octsize;
  /// the octaves
  uint8 *octaves;

  /// init the texture
  void Initialize();
  /// init an octave with new random/smoothed content
  void InitOctave(int nr);
  /// octave value get/set
  uint8& GetOctave(int oct, int x, int y) 
  { return octaves [ oct*octsize*octsize + y*octsize + x ]; }
  void SetOctave(int oct, int x, int y, uint8 val) 
  { octaves[ oct*octsize*octsize + y*octsize + x ] = val; }
  /// get the intersection with sphere (false = no intersection)
  bool SphereIntersect(const csVector3& point, csVector3& isect);
  /// get sky bluishness at a point on the sphere.
  csRGBcolor GetSkyBlue(const csVector3& spot, float& haze, float sundist);
  /// get combined octave value, cloudval.
  uint8 GetCloudVal(int x, int y);
  /// get sundistance value
  float GetSundist(const csVector3& spot);

public:
  csProcSky();
  ~csProcSky();

  void DrawToTexture(csProcSkyTexture *skytex);
};

#endif // __PROCSKYTEX_H__
