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

#ifndef __CS_HALO_H__
#define __CS_HALO_H__

#include "csgeom/math3d.h"
#include "csengine/light.h"
#include "ihalo.h"

class csEngine;

enum csHaloType
{
  cshtCross,
  cshtNova
};

/** 
 * This is the basic class for all types of halos.
 * The csLight class contains a pointer to a object derived from this class.
 */
class csHalo
{
  /// the current intensity of the attached halo; if >0 halo is in halo queue.
  float Intensity;

public:
  /// The type of halo
  csHaloType Type;

  /// Constructor
  csHalo (csHaloType iType);
  /// Destructor
  virtual ~csHalo ();

  /// Generate the alphamap for this halo of size Size x Size
  virtual unsigned char *Generate (int Size) = 0;

  /// Get halo intensity
  float GetIntensity ()
  { return Intensity; }
  /// Set halo intensity
  void SetIntensity (float iInt)
  { Intensity = iInt; }
};

/**
 * This is a halo which ressembles a cross.
 */
class csCrossHalo : public csHalo
{
public:
  /// Halo intensity factor
  float IntensityFactor;
  /// Halo cross-ressemblance factor
  float CrossFactor;

  /// Create a halo object
  csCrossHalo (float intensity_factor, float cross_factor);

  /// Generate the alphamap for this halo of size Size x Size
  virtual unsigned char *Generate (int Size);
};

class csNovaHalo : public csHalo
{
public:
  /// Random seed for generating halo
  int Seed;
  /// Number of halo spokes
  int NumSpokes;
  /// The "roundness" factor
  float Roundness;

  /// Create a halo object
  csNovaHalo (int seed, int num_spokes, float roundness);

  /// Generate the alphamap for this halo of size Size x Size
  virtual unsigned char *Generate (int Size);
};

/**
 * This is used to keep track of halos.<p>
 * When the engine detects that a light that is marked to have an halo
 * is directly visible, an object of this type is created and put into
 * a global queue maintained within the engine object. The light starts
 * to brighten until it reaches maximal intensity; when the halo becomes
 * obscured by something or goes out of view the intensity starts to
 * decrease until it reaches zero; upon this event the halo object is
 * destroyed and removed from halo queue.
 */
class csLightHalo
{
public:
  /// The light this halo is attached to
  csLight *Light;

  /// Halo handle as returned by 3D rasterizer
  iHalo *Handle;

  /// Last time we were updated
  cs_time LastTime;

  /// Create an light halo object
  csLightHalo (csLight *iLight, iHalo *iHandle);

  /// Destroy the light halo object
  ~csLightHalo ();

  /**
   * Process a light halo. The function changes halo brightness depending
   * whenever the halo is obscured or not and returns "false" if halo has
   * reached zero intensity and should be removed from halo queue.
   * The function also actually projects, clips and draws the halo.
   */
  bool Process (cs_time ElapsedTime, csEngine const&);
};

#endif // __CS_HALO_H__
