/*
    Copyright (C) 2007 by Frank Richter

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

#include "cssysdef.h"

#include "ivideo/shader/shader.h"
#include "csutil/objreg.h"

#include "csplugincommon/rendermanager/renderlayers.h"

namespace CS
{
  namespace RenderManager
  {
    void AddDefaultBaseLayers (iObjectRegistry* objectReg,
      MultipleRenderLayer& layers, uint flags)
    {
      csRef<iShaderManager> shaderManager = 
        csQueryRegistry<iShaderManager> (objectReg);
      csRef<iStringSet> stringSet = csQueryRegistryTagInterface<iStringSet> (
        objectReg, "crystalspace.shared.stringset");

      SingleRenderLayer baseLayer (
        shaderManager->GetShader ("std_lighting"));
      baseLayer.AddShaderType (stringSet->Request("base"));
      baseLayer.AddShaderType (stringSet->Request("ambient"));
      if (!(flags & defaultlayerNoTerrain))
        baseLayer.AddShaderType (stringSet->Request("splatting ambient"));
      baseLayer.AddShaderType (stringSet->Request("standard"));
      layers.AddLayers (baseLayer);
      
      if (!(flags & defaultlayerNoTerrain))
      {
        SingleRenderLayer splatLayer;
        splatLayer.AddShaderType (stringSet->Request("terrain splat"));
        layers.AddLayers (splatLayer);
      }
    }

  } // namespace RenderManager
} // namespace CS
