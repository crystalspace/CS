/*
    Copyright (C) 2010 by Frank Richter

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_POSTEFFECTSSUPPORT_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_POSTEFFECTSSUPPORT_H__

/**\file
 * Drop-in post effects support for render managers
 */

#include "iengine/rendermanager.h"

#include "csplugincommon/rendermanager/posteffects.h"

namespace CS
{
  namespace RenderManager
  {
    /**
     * Add post effects support to a render manager.
     * Usage:
     * - Must be a base class of the render manager implementation.
     * - Render manager implementation should add
     *   <tt>iRenderManagerPostEffects<iDebugHelper></tt> to their SCF implementation
     *   base class.
     * - Access to the post processing effects manager is available via the member
     *   `postEffects'.
     */
    class CS_CRYSTALSPACE_EXPORT PostEffectsSupport : public virtual iRenderManagerPostEffects
    {
      CS::RenderManager::PostEffectLayersParser* postEffectsParser;
    protected:
      CS::RenderManager::PostEffectManager       postEffects;
    public:
      PostEffectsSupport();
      virtual ~PostEffectsSupport();
      
      /**
       * Initialize post processing effects support.
       * \param objectReg Object registry.
       * \param configKey Configuration key for initial effects to load.
       *   Will read a file name for a post effects layers file from the config
       *   key <tt>&lt;configKey&gt;.Effects</tt>.
       */
      void Initialize (iObjectRegistry* objectReg, const char* configKey);
    
      /**\name iRenderManagerPostEffects implementation
      * @{ */
      void ClearLayers() { postEffects.ClearLayers(); }
      bool AddLayersFromDocument (iDocumentNode* node);
      bool AddLayersFromFile (const char* filename);
      /** @} */
    };
    
  } // namespace RenderManager
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_POSTEFFECTSSUPPORT_H__
