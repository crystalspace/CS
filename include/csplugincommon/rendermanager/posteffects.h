/*
    Copyright (C) 2007-2008 by Marten Svanfeldt

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_POSTEFFECTS_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_POSTEFFECTS_H__

#include "csutil/ref.h"
#include "csutil/array.h"
#include "ivideo/shader/shader.h"

struct iShader;
struct iTextureHandle;
struct iView;
struct iGraphics3D;

namespace CS
{
namespace RenderManager
{


  /**
   * Helper for "simple" post effects usage in render manager.
   * Provides a simple way to render the screen to a texture and then use
   * a number of full screen passes with settable shader to get the desired
   * effect.
   */
  class CS_CRYSTALSPACE_EXPORT PostEffectManager
  {
  public:
    PostEffectManager ();
    ~PostEffectManager ();

    void Initialize (iObjectRegistry* objectReg);

    void SetupView (iView* view);

    iTextureHandle* GetScreenTarget ();

    void DrawPostEffects ();

    void AddLayer (iShader* shader)
    {
      PostEffectLayer layer;
      layer.effectShader = shader;

      postLayers.Push (layer);
    }

  private:
    void SetupScreenQuad (unsigned int width, unsigned int height);
    void KillScreenQuad ();

    void AllocatePingpongTextures ();
    
    csRef<iGraphics3D> graphics3D;

    csSimpleRenderMesh fullscreenQuad;
    csVector3* screenQuadVerts;
    csVector2* screenQuadTex;

    unsigned int currentWidth, currentHeight;
    float textureCoordinateX, textureCoordinateY, textureOffsetX, textureOffsetY;

    csRef<iTextureHandle> textures[2];
    unsigned int activeTexture;

    struct PostEffectLayer
    {
      csRef<iShader> effectShader;
    };

    csArray<PostEffectLayer> postLayers;
  };

}
}

#endif
