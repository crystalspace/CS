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

#ifndef __CS_CSPLUGINCOMMON_SOFTSHADER_TEXTURE_H__
#define __CS_CSPLUGINCOMMON_SOFTSHADER_TEXTURE_H__

/**\addtogroup plugincommon
 * @{ */

namespace CrystalSpace
{
  namespace SoftShader
  {
    struct SoftwareTexture
    {
      /// Width and height
      int w, h;
      /// log2(width) and log2(height)
      int shf_w, shf_h;
      /// (1 << log2(width)) - 1 and (1 << log2(height)) - 1
      int and_w, and_h;
      /// The bitmap
      uint32* bitmap;
    };
  } // namespace SoftShader
} // namespace CrystalSpace

/** @} */

#endif // __CS_CSPLUGINCOMMON_SOFTSHADER_TEXTURE_H__
