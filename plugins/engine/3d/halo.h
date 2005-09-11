/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
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

#ifndef __CS_HALO_H__
#define __CS_HALO_H__

#include "csgeom/math3d.h"
#include "iengine/halo.h"
#include "ivideo/halo.h"

class csLight;
class csEngine;
struct iCamera;
struct iMaterialWrapper;

/**
 * This is the basic class for all types of halos.
 * The csLight class contains a pointer to a object derived from this class.
 */
class csHalo : public iBaseHalo
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
  virtual float GetIntensity () { return Intensity; }
  /// Set halo intensity
  virtual void SetIntensity (float iInt) { Intensity = iInt; }
  /// Get halo type.
  virtual csHaloType GetType () { return Type; }

  SCF_DECLARE_IBASE;
};

/**
 * This is a halo which resembles a cross.
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

  /// Destructor.
  virtual ~csCrossHalo();

  /// Generate the alphamap for this halo of size Size x Size
  virtual unsigned char *Generate (int Size);

  //------------------------ iCrossHalo ------------------------------------
  SCF_DECLARE_IBASE_EXT (csHalo);
  /// iCrossHalo implementation.
  struct CrossHalo : public iCrossHalo
  {
    SCF_DECLARE_EMBEDDED_IBASE (csCrossHalo);
    virtual void SetIntensityFactor (float i)
    { scfParent->IntensityFactor = i; }
    virtual float GetIntensityFactor ()
    { return scfParent->IntensityFactor; }
    virtual void SetCrossFactor (float i)
    { scfParent->CrossFactor = i; }
    virtual float GetCrossFactor ()
    { return scfParent->CrossFactor; }
  } scfiCrossHalo;
  friend struct CrossHalo;
};

/** This halo has a center with a number of spokes */
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

  /// Destructor.
  virtual ~csNovaHalo();

  /// Generate the alphamap for this halo of size Size x Size
  virtual unsigned char *Generate (int Size);

  //------------------------ iNovaHalo ------------------------------------
  SCF_DECLARE_IBASE_EXT (csHalo);
  /// iNovaHalo implementation.
  struct NovaHalo : public iNovaHalo
  {
    SCF_DECLARE_EMBEDDED_IBASE (csNovaHalo);
    virtual void SetRandomSeed (int s)
    { scfParent->Seed = s; }
    virtual int GetRandomSeed ()
    { return scfParent->Seed; }
    virtual void SetSpokeCount (int s)
    { scfParent->NumSpokes = s; }
    virtual int GetSpokeCount ()
    { return scfParent->NumSpokes; }
    virtual void SetRoundnessFactor (float r)
    { scfParent->Roundness = r; }
    virtual float GetRoundnessFactor ()
    { return scfParent->Roundness; }
  } scfiNovaHalo;
  friend struct NovaHalo;
};


/** structure used to keep flare component information */
struct csFlareComponent 
{
  /// position, (0.= at light, 1.=center)
  float position;
  /// width and height (1.0 gives size of a normal halo)
  float width, height;
  /// visual image of component
  iMaterialWrapper *image;
  /// mixmode for drawing
  uint mixmode;
  /// next component to draw
  csFlareComponent *next;
};

/**
 * This halo is used for (solar)flares
 */
class csFlareHalo : public csHalo
{
private:
  /// List of the flare components. in drawing order.
  csFlareComponent *components;
  /// Last flare component to make adding efficient.
  csFlareComponent *last;
public:
  /// Create an (empty) flare
  csFlareHalo();
  /// Destructor.
  ~csFlareHalo();
  /**
   * Add a visual component to the flare.
   * give position, size, image and mixmode.
   * The component is added at the end of the list - to be displayed last.
   */
  void AddComponent(float pos, float w, float h, uint mode,
    iMaterialWrapper *image);
  /// Get the list of component
  csFlareComponent *GetComponents() const {return components;}
  /**
   * Generate this halo's alpha map.
   * Not used for this halo (returns 0) since the halo consists of
   * multiple images.
   */
  virtual unsigned char *Generate (int Size);

  //------------------------ iFlareHalo ------------------------------------
  SCF_DECLARE_IBASE_EXT (csHalo);
  /// iFlareHalo implementation.
  struct FlareHalo : public iFlareHalo
  {
    SCF_DECLARE_EMBEDDED_IBASE (csFlareHalo);
    virtual void AddComponent (float pos, float w, float h, uint mode,
      iMaterialWrapper *image)
    {
      scfParent->AddComponent (pos, w, h, mode, image);
    }
  } scfiFlareHalo;
  friend struct FlareHalo;
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
  csTicks LastTime;

  /// Create an light halo object
  csLightHalo (csLight *iLight, iHalo *iHandle);

  /// Destroy the light halo object
  virtual ~csLightHalo ();

  /**
   * Process a light halo. The function changes halo brightness depending
   * whenever the halo is obscured or not and returns "false" if halo has
   * reached zero intensity and should be removed from halo queue.
   * The function also actually projects, clips and draws the halo.
   */
  virtual bool Process (csTicks ElapsedTime, iCamera* camera, 
    csEngine* engine);

  /**
   * see if camera position is visible, returns it projected onto screen.
   * Called by Process.
   */
  bool IsVisible(iCamera* camera, csEngine* engine, csVector3& v);

  /**
   * add up elapsed time, and returns new intensity, given the current
   * halo intensity. Give whether halo is currently visible for fading in
   * and out. returns true if intensity >0, returns false if intensity <= 0,
   * when the halo is completely faded out.
   * Called by Process.
   */
  bool ComputeNewIntensity(csTicks ElapsedTime, float& hintensity,
    bool halo_vis);
};

/**
 * This class is used to keep track of visible Flares.
 * It is a subclass of csLightHalo, and - to the engine - behaves the same.
 */
class csLightFlareHalo : public csLightHalo
{
  /// size of halos on the screen
  int halosize;
  /// the flare description
  csFlareHalo *flare;
public:
  /**
   * create. pass light, and flareHalo, the halosize is the size that
   * halos have on the screen - it is used to scale the flare.
   */
  csLightFlareHalo(csLight *light, csFlareHalo *halo, int iHaloSize);
  /// remove
  virtual ~csLightFlareHalo();

  /// process the halo.
  virtual bool Process (csTicks ElapsedTime, iCamera* camera, 
    csEngine* engine);
  /// process a flare component (clip and draw it).
  void ProcessFlareComponent(csEngine const& engine, csFlareComponent *comp,
    csVector2 const& start, csVector2 const& deltapos);
};

#endif // __CS_HALO_H__
