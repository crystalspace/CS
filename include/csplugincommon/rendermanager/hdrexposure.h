/*
    Copyright (C) 2008 by Frank Richter

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_HDREXPOSURE_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_HDREXPOSURE_H__

#include "csgfx/textureformatstrings.h"
#include "csutil/ref.h"

class csShaderVariable;
struct iDataBuffer;
struct iObjectRegistry;

namespace CS
{
  namespace RenderManager
  {
    class PostEffectManager;
  
    class CS_CRYSTALSPACE_EXPORT HDRExposureLinear
    {
      float exposure;
      csRef<csShaderVariable> svHDRScale;
      CS::StructuredTextureFormat readbackFmt;
      
      csRef<iDataBuffer> lastData;
      int lastW, lastH;
      csTicks lastTime;
    public:
      HDRExposureLinear () : exposure (1.0f), 
        readbackFmt (CS::TextureFormatStrings::ConvertStructured ("argb8")),
        lastTime (0)
      {}
    
      void Initialize (iObjectRegistry* objReg,
        PostEffectManager& postEffects);
      
      void ApplyExposure (PostEffectManager& postEffects);
    };
  
  } // namespace RenderManager
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_HDREXPOSURE_H__
