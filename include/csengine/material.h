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

#ifndef __CS_MATERIAL_H__
#define __CS_MATERIAL_H__

#include "csutil/cscolor.h"

class csTextureHandle;


/**
 * A material class.
 */

class csMaterial
{
private:
  /// flat shading color
  csColor flat_color;
  /// the texture of the material (can be NULL)
  csTextureHandle *texture;

  /// The diffuse reflection value of the material
  float diffuse;
  /// The ambient lighting of the material
  float ambient;
  /// The reflectiveness of the material
  float reflection;


public:
  /**
   * create an empty material
   */
  csMaterial ();
  /**
   * create a material with only the texture given.
   */
  csMaterial (csTextureHandle *txt);

  /**
   * destroy material
   */
  ~csMaterial ();


  /// Get the flat shading color
  inline const csColor& GetFlatColor () const {return flat_color;}
  /// Set the flat shading color
  inline void SetFlatColor (const csColor& col) {flat_color = col;}

  /// Get the texture (if none NULL is returned)
  inline csTextureHandle *GetTextureHandle () const {return texture;}
  /// Set the texture (pass NULL to set no texture)
  inline void SetTextureHandle (csTextureHandle *tex) {texture = tex;}

  /// Get diffuse reflection constant for the material
  inline float GetDiffuse () const { return diffuse; }
  /// Set diffuse reflection constant for the material
  inline void SetDiffuse (float val) { diffuse = val; }

  /// Get ambient lighting for the material
  inline float GetAmbient () const { return ambient; }
  /// Set ambient lighting for the material
  inline void SetAmbient (float val) { ambient = val; }

  /// Get reflection of the material
  inline float GetReflection () const { return reflection; }
  /// Set reflection of the material
  inline void SetReflection (float val) { reflection = val; }
};

#endif // __CS_MATERIAL_H__
