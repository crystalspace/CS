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

#ifndef __CS_CSPLUGINCOMMON_SOFTSHADER_RENDERINTERFACE_H__
#define __CS_CSPLUGINCOMMON_SOFTSHADER_RENDERINTERFACE_H__

/**\file
 * Software renderer shader interface
 */
 
/**\addtogroup plugincommon
 * @{ */

namespace CrystalSpace
{
  namespace SoftShader
  {
    struct iScanlineRenderer;
    
    /**
     * Interface specific to the software renderer to allow shaders to
     * communicate with it.
     */
    struct iSoftShaderRenderInterface : public virtual iBase
    {
    public:
      SCF_INTERFACE(iSoftShaderRenderInterface, 0, 0, 1);
    
      virtual void SetScanlineRenderer (iScanlineRenderer* sr) = 0;
    };
   
  } // namespace SoftShader
} // namespace CrystalSpace

/** @} */

#endif // __CS_CSPLUGINCOMMON_SOFTSHADER_RENDERINTERFACE_H__
