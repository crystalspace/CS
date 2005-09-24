/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __CS_IENGINE_HALO_H__
#define __CS_IENGINE_HALO_H__

/**\file
 */
/**
 * \addtogroup engine3d_light
 * @{ */

#include "csutil/scf.h"

struct iMaterialWrapper;

/**
 * The possible halo-types.
 */
enum csHaloType
{
  /// Cross halo
  cshtCross,
  /// Nova halo
  cshtNova,
  /// Flare halo
  cshtFlare
};

/**
 * This is the basic interface for all types of halos.
 */
struct iBaseHalo : public virtual iBase
{
  SCF_INTERFACE(iBaseHalo,2,0,0);
  /// Set intensity.
  virtual void SetIntensity (float i) = 0;
  /// Get intensity.
  virtual float GetIntensity () = 0;
  /// Get halo type.
  virtual csHaloType GetType () = 0;
};

/**
 * This is a halo which resembles a cross.
 */
struct iCrossHalo : public virtual iBase
{
  SCF_INTERFACE(iCrossHalo,2,0,0);
  /// Set intensity factor.
  virtual void SetIntensityFactor (float i) = 0;
  /// Get intensity factor.
  virtual float GetIntensityFactor () = 0;
  /// Set cross factor.
  virtual void SetCrossFactor (float i) = 0;
  /// Get cross factor.
  virtual float GetCrossFactor () = 0;
};

/**
 * This is a halo which resembles a nova.
 */
struct iNovaHalo : public virtual iBase
{
  SCF_INTERFACE(iNovaHalo,2,0,0);
  /// Set random seed for generating the halo.
  virtual void SetRandomSeed (int s) = 0;
  /// Get random seed.
  virtual int GetRandomSeed () = 0;
  /// Set the number of halo spokes.
  virtual void SetSpokeCount (int s) = 0;
  /// Get the number of halo spokes.
  virtual int GetSpokeCount () = 0;
  /// Set the roundness factor.
  virtual void SetRoundnessFactor (float r) = 0;
  /// Get the roundness factor.
  virtual float GetRoundnessFactor () = 0;
};

/**
 * This is a halo which resembles a (solar) flare.
 */
struct iFlareHalo : public virtual iBase
{
  SCF_INTERFACE(iFlareHalo,2,0,0);
  /**
   * Add a visual component to the flare.
   * give position, size, image and mixmode.
   * The component is added at the end of the list - to be displayed last.
   */
  virtual void AddComponent (float pos, float w, float h, uint mode,
    iMaterialWrapper *image) = 0;
};

/** @} */

#endif // __CS_IENGINE_HALO_H__
