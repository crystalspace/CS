/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein
    Copyright (C) 1999 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __IENGINE_LIGHT_H__
#define __IENGINE_LIGHT_H__

#include "csutil/scf.h"

class csLight;
class csColor;
struct iSector;
struct iObject;
struct iCrossHalo;
struct iNovaHalo;
struct iFlareHalo;

/// Light level that is used when there is no light on the texture.
#define DEFAULT_LIGHT_LEVEL 20
/// Light level that corresponds to a normally lit texture.
#define NORMAL_LIGHT_LEVEL 128

/**
 * Attenuation controls how the brightness of a light fades with distance.
 * There are four attenuation formulas:
 * <ul>
 *   <li> no attenuation = light * 1
 *   <li> linear attenuation = light * (radius - distance) / radius
 *   <li> inverse attenuation = light * (radius / distance)
 *   <li> realistic attenuation = light * (radius^2 / distance^2)
 * </ul>
 */
#define CS_ATTN_NONE      0
#define CS_ATTN_LINEAR    1
#define CS_ATTN_INVERSE   2
#define CS_ATTN_REALISTIC 3

SCF_VERSION (iLight, 0, 0, 5);

/**
 * The iLight interface is the SCF interface for the csLight class. 
 */
struct iLight : public iBase
{
  /// Get private pointer to light object. UGLY
  virtual csLight* GetPrivateObject () = 0;
  /// Get the iObject for this light.
  virtual iObject *QueryObject() = 0;
  /// Get the position of this light.
  virtual csVector3& GetCenter () = 0;
  /// Get the squared radius.
  virtual float GetSquaredRadius () const = 0;
  /// Get the color of this light.
  virtual csColor& GetColor () = 0;
  /// Set the color of this light.
  virtual void SetColor (const csColor& col) = 0;
  /// Set the sector for this light.
  virtual void SetSector (iSector* sector) = 0;
  /// Get the brightness of a light at a given distance.
  virtual float GetBrightnessAtDistance (float d) = 0;
  /// Create a cross halo for this light.
  virtual iCrossHalo* CreateCrossHalo (float intensity, float cross) = 0;
  /// Create a nova halo for this light.
  virtual iNovaHalo* CreateNovaHalo (int seed, int num_spokes,
  	float roundness) = 0;
  /// Create a flare halo for this light.
  virtual iFlareHalo* CreateFlareHalo () = 0;
  /// Return current attenuation mode
  virtual int GetAttenuation () = 0;
  /// Set attenuation mode
  virtual void SetAttenuation (int a) = 0;
};

#endif // __IENGINE_LIGHT_H__
