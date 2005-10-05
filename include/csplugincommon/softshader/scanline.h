/*
    Copyright (C) 2005 by Jorrit Tyberghein
              (C) 2005 by Frank Richter

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

#ifndef __CS_CSPLUGINCOMMON_SOFTSHADER_SCANLINE_H__
#define __CS_CSPLUGINCOMMON_SOFTSHADER_SCANLINE_H__

/**\file
 * Software renderer scanline rendering
 */

#include "ivideo/rendermesh.h"

#include "texture.h"
#include "types.h"

/**\addtogroup plugincommon
 * @{ */

namespace CrystalSpace
{
  namespace SoftShader
  {
    /**
     * Software renderer scanline renderer.
     */
    struct iScanlineRenderer : public virtual iBase
    {
    public:
      SCF_INTERFACE(iScanlineRenderer, 0, 0, 1);
    
      /**
       * Scanline rendering function.
       * \param _This The scanline renderer that provided the function.
       * \param L Interpolator for the left edge
       * \param R Interpolator for the right edge
       * \param ipolStep Interpolation step for approximated perspective-
       *   correct interpolation.
       * \param ipolShift Interpolation shift for approximated perspective-
       *   correct interpolation.
       * \param dest Start of pixels to write.
       * \param len Number of pixels to write.
       * \param zbuff Pointer into Z buffer.
       */
      typedef void (*ScanlineProc) (iScanlineRenderer* _This,
	InterpolateEdgePersp& L, InterpolateEdgePersp& R, 
	int ipolStep, int ipolShift,
	void* dest, uint len, uint32 *zbuff);
  
      /**
       * Information for settin up rendering, filled by the scanline renderer
       */
      struct RenderInfo
      {
	/// The renderer object to pass to the scanline function
	iScanlineRenderer* renderer;
	/// Scanline function
	ScanlineProc proc;
	/// Factors for buffer denormalization
	csVector4* denormFactors;
	/// Buffers to denormalize
	BuffersMask denormBuffers;
	/// Buffers the scanline function expects
	BuffersMask desiredBuffers;
	/// Numbers of components in buffers
	const size_t* bufferComps;
      };

      /**
       * Return an appropriate scanline function based on the provided
       * parameters (and further more, user options).
       */
      virtual bool Init (SoftwareTexture** textures,
	const csRenderMeshModes& modes,
	BuffersMask availableBuffers,
	RenderInfo& renderInfo) = 0;
    };
  } // namespace SoftShader
} // namespace CrystalSpace

/** @} */

#endif // __CS_CSPLUGINCOMMON_SOFTSHADER_SCANLINE_H__
