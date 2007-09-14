/*
    Copyright (C) 2007 by Marten Svanfeldt

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_RENDERLAYERS_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_RENDERLAYERS_H__

#include "csutil/strset.h"

struct iShader;

namespace CS
{
namespace RenderManager
{

  class SingleRenderLayer
  {
  public:
    SingleRenderLayer (const csStringID shaderName, iShader* defaultShader = 0)
      : shaderName (shaderName), defaultShader (defaultShader)
    {
    }

    size_t GetLayerCount () const
    {
      return 1;
    }

    const csStringID GetShaderName (size_t layer) const
    {
      CS_ASSERT(layer == 0);

      return shaderName;
    }

    iShader* GetDefaultShader (size_t layer) const
    {
      CS_ASSERT(layer == 0);

      return defaultShader;
    }

  private:
    const csStringID shaderName;
    iShader* defaultShader;
  };

  class MultipleRenderLayer : private CS::NonCopyable
  {
  public:
    MultipleRenderLayer (size_t numLayers, const csStringID* shaderName, 
      iShader** defaultShader)
      : numLayers (numLayers), shaderName (0), defaultShader (0)
    {
      this->shaderName = static_cast<csStringID*> (
        cs_malloc (sizeof(csStringID)*numLayers));
      this->defaultShader = static_cast<iShader**> (
        cs_malloc (sizeof(iShader*)*numLayers));

      memcpy(this->shaderName, shaderName, sizeof(const csStringID)*numLayers);
      memcpy(this->defaultShader, defaultShader, sizeof(iShader*)*numLayers);
    }

    ~MultipleRenderLayer ()
    {
      cs_free (shaderName);
      cs_free (defaultShader);
    }

    size_t GetLayerCount () const
    {
      return numLayers;
    }

    const csStringID GetShaderName (size_t layer) const
    {
      CS_ASSERT(layer < numLayers);

      return shaderName[layer];
    }

    iShader* GetDefaultShader (size_t layer) const
    {
      CS_ASSERT(layer < numLayers);

      return defaultShader[layer];
    }

  private:
    size_t numLayers;
    csStringID* shaderName;
    iShader** defaultShader;
  };

}
}

#endif
