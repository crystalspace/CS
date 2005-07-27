/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef __CS_IVIDEO_MATERIAL_H__
#define __CS_IVIDEO_MATERIAL_H__

/**\file
 * Material interface
 */
/**
 * \addtogroup gfx3d
 * @{ */
 
#include "csutil/scf.h"
#include "csutil/strset.h"

#include "ivideo/shader/shader.h"


/// Default material `diffuse' parameter
#define CS_DEFMAT_DIFFUSE 0.7f
/// Default material `ambient' parameter
#define CS_DEFMAT_AMBIENT 0.0f
/// Default material `reflection' parameter
#define CS_DEFMAT_REFLECTION  0.0f

/// Name string for the material "diffuse" shader var
#define CS_MATERIAL_VARNAME_DIFFUSE		"mat diffuse"
/// Name string for the material "ambient" shader var
#define CS_MATERIAL_VARNAME_AMBIENT		"mat ambient"
/// Name string for the material "reflection" shader var
#define CS_MATERIAL_VARNAME_REFLECTION		"mat reflection"
/// Name string for the material "flat color" shader var
#define CS_MATERIAL_VARNAME_FLATCOLOR		"mat flatcolor"
/// Name string for the material "diffuse" texture
#define CS_MATERIAL_TEXTURE_DIFFUSE		"tex diffuse"

struct iTextureHandle;
struct csRGBpixel;

SCF_VERSION (iMaterial, 0, 1, 0);

/**
 * This class represents a material as seen from the engine
 * view. Because it is also a shader variable context it can be used directly 
 * for rendering.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iEngine::CreateBaseMaterial()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iMaterialWrapper::GetMaterial()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>3D renderer implementations (iGraphics3D).
 *   </ul>
 */
struct iMaterial : public iShaderVariableContext
{
  /**
   * Associate a shader with a shader type
   */
  virtual void SetShader (csStringID type, iShader* shader) = 0;

  /**
   * Get shader associated with a shader type
   */
  virtual iShader* GetShader (csStringID type) = 0;

  /**
   * Get all Shaders.
   */
  virtual const csHash<csRef<iShader>, csStringID>& GetShaders() const =0;

  /**
   * Get the base texture from the material.
   */
  virtual iTextureHandle *GetTexture () = 0;

  /**
   * Get a texture from the material.
   */
  virtual iTextureHandle* GetTexture (csStringID name) = 0;

  /**
   * Get the flat color. If the material has a texture assigned, this
   * will return the mean texture color.
   * \deprecated Use the shader variable system instead.
   */
  virtual void GetFlatColor (csRGBpixel &oColor,
    bool useTextureMean = true) = 0;
  /**
   * Set the flat shading color.
   * \deprecated Use the shader variable system instead.
   */
  virtual void SetFlatColor (const csRGBcolor& col) = 0;

  /**
   * Get light reflection parameters for this material.
   * \deprecated Use the shader variable system instead.
   */
  virtual void GetReflection (
    float &oDiffuse, float &oAmbient, float &oReflection) = 0;
  /**
   * Set the reflection parameters.
   * \deprecated Use the shader variable system instead.
   */
  virtual void SetReflection (float oDiffuse, float oAmbient,
    float oReflection) = 0;
};

/** @} */

#endif // __CS_IVIDEO_MATERIAL_H__
