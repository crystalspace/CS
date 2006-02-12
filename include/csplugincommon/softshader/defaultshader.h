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

#ifndef __CS_CSPLUGINCOMMON_SOFTSHADER_DEFAULTSHADER_H__
#define __CS_CSPLUGINCOMMON_SOFTSHADER_DEFAULTSHADER_H__

/**\file
 * Interface to software renderer default shader plugin
 */

#include "scanline.h"

/**\addtogroup plugincommon
 * @{ */

namespace CS
{
namespace PluginCommon
{
  namespace SoftShader
  {
    /**
     * Software renderer default shader plugin
     */
    struct iDefaultShader : public virtual iBase
    {
    public:
      SCF_INTERFACE(iDefaultShader, 0, 0, 1);
    
      virtual iDefaultScanlineRenderer* GetDefaultRenderer() = 0;
    };
  } // namespace SoftShader
} // namespace PluginCommon
} // namespace CS

/** @} */

#endif // __CS_CSPLUGINCOMMON_SOFTSHADER_DEFAULTSHADER_H__
