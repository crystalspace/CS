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

#ifndef __CS_PROCSKYTEX_H__
#define __CS_PROCSKYTEX_H__

#include "csextern.h"

#include "csgeom/math3d.h"
#include "csgfx/rgbpixel.h"
#include "csutil/cscolor.h"
#include "cstool/proctex.h"

class csProcSky;

/**
 * A polygon of a sky.
 */
class CS_CSTOOL_EXPORT csProcSkyTexture : public csProcTexture
{
  /// the sky this is a part of
  csProcSky *sky;
  /// next procskytexture in this sky
  csProcSkyTexture *next;

  /// texture orig,udir,vdir (in world coordinates as used in the sky)
  csVector3 txtorig, txtu, txtv;
  /// the cached intersection points
  csVector3 *isect;
  /// must be rerendered next frame
  bool forcerender;

public:
  /// create, given a sky it belongs to.
  csProcSkyTexture(csProcSky *par);
  ///
  virtual ~csProcSkyTexture();

  void SetNextSky(csProcSkyTexture *n) {next = n;}
  csProcSkyTexture *GetNextSky() const {return next;}

  virtual bool PrepareAnim ();

  /// Draw the next frame.
  virtual void Animate (csTicks current_time);

  /// get the width of this texture
  int GetWidth() const {return mat_w;}
  /// get the height of this texture
  int GetHeight() const {return mat_h;}
  /// get texturespace values
  void GetTextureSpace(csVector3& orig, csVector3& u, csVector3& v) const
  { orig = txtorig; u = txtu; v = txtv; }
  /// set cached isects cache array
  void SetIntersect(csVector3 *icache) {isect = icache;}
  /// get cached isects cache array
  csVector3 *GetIntersect() const {return isect;}

  /// force the texture to be re-rendered next frame (by the prSky)
  void ForceRerender() {forcerender = true;}
  /// see if the texture must be re-rendered
  bool MustRender() const {return forcerender;}
  /// unset the forced rendering.
  void UnsetForceRender() {forcerender = false;}
  /// has been prepared ?
  bool AnimPrepared () {return anim_prepared;}
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
};


/**
 * a sky, this represents a whole sphere of sky - so multiple polygons
 * can be rendered to.
 */
class CS_CSTOOL_EXPORT csProcSky
{
  /// the proc sky textures of this sky
  csProcSkyTexture *firstsky;

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
  /// color at the maximum hazyness point
  csRGBcolor maxhaze;

  /// nr of octaves of the clouds.
  int nr_octaves;
  /// size of an octave
  int octsize;
  /// the octaves, for the current state
  uint8 *octaves;
  /// the enlarged octaves
  uint8** enlarged;


  /// is it animation (if not - no recalculation is performed)
  bool animated;
  /// periods for each octave - total new random after this many msec
  int *periods;
  /// current time position (in msec) per octaves
  int *curposition;
  /**
   * start and end images for each octave; (like octaves but start and
   * end positions of this period of animation
   */
  uint8 *startoctaves, *endoctaves;
  /// the previous time of animated frame
  csTicks old_time;
  /// current wind 'position', of the origin of the clouds
  csVector2 windpos;
  /// direction and speed of wind
  csVector2 winddir;

  /// init the texture
  void Initialize();
  /// init an octave with new random/smoothed content
  void InitOctave(uint8 *octs, int nr);
  /// smooth an octave contents , smoothpower (0=none, 1=medium, 2=much)
  void SmoothOctave(uint8 *octs, int nr, int smoothpower);
  /// enlarge an octave, size is scaled by 2**factor, values >> rshift;
  void Enlarge(uint8 *dest, uint8 *src, int factor, int rshift);
  /// take weighted average of start&end into dest, pos(0=start) of max(=end).
  void Combine(uint8 *dest, uint8 *start, uint8 *end, int pos, int max, int nr);
  /// animate octave nr, given elapsed time (msec);
  void AnimOctave(int nr, int elapsed);
  /// octave value get/set
  uint8& GetOctave(uint8 *octaves, int oct, int x, int y)
  { return octaves [ oct*octsize*octsize + y*octsize + x ]; }
  void SetOctave(uint8 *octaves, int oct, int x, int y, uint8 val)
  { octaves[ oct*octsize*octsize + y*octsize + x ] = val; }
  /// copy one octave to another
  void CopyOctave(uint8 *srcocts, int srcnr, uint8 *destocts, int destnr);

  /// get the intersection with sphere (false = no intersection)
  bool SphereIntersect(const csVector3& point, csVector3& isect);
  /// get sky bluishness at a point on the sphere. below==the ground
  csRGBcolor GetSkyBlue(const csVector3& spot, float& haze, float sundist,
    bool& below);
  /// get combined octave value, cloudval.
  uint8 GetCloudVal(int x, int y);
  /// get sundistance value
  float GetSundist(const csVector3& spot);

public:
  csProcSky();
  ~csProcSky();

  /// do a nextframe like drawing update
  void DrawToTexture (csProcSkyTexture *skytex, csTicks current_time,
  	iObjectRegistry* object_reg);

  /// Make intersection point cache in a texture
  void MakeIntersectCache(csProcSkyTexture *skytex);

  /**
   * Enable or disable sky animation. Sky animation is very slow.
   * If you wish to continue the animated where it left off
   * (seamlessly) pass the current time,
   * if you wish to continue where the animated would be if
   * it had been running all the time - pass 0 for time.
   * current_time has no meaning when disabling the animation.
   */
  void SetAnimated (iObjectRegistry* object_reg,
  	bool anim=true, csTicks current_time=0);
  /// See if the prsky is animated
  bool GetAnimated() const {return animated;}
  /// get first sky texture with this sky
  csProcSkyTexture *GetFirstSky() const {return firstsky;}
  /// set first sky texture with this sky
  void SetFirstSky(csProcSkyTexture *s) {firstsky = s;}
};

#endif // __CS_PROCSKYTEX_H__
