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

#ifndef __CS_IENGINE_RENDERMANAGER_H__
#define __CS_IENGINE_RENDERMANAGER_H__

#include "csutil/scf_interface.h"
#include "ivaria/view.h"

struct iTextureHandle;
struct iVisibilityCuller;

/**
 * Common render manager interface.
 */
struct iRenderManager : public virtual iBase
{
  SCF_INTERFACE(iRenderManager,2,0,1);

  /// Render the given view into the framebuffer.
  virtual bool RenderView (iView* view) = 0;

  /**
   * Render the given view into the framebuffer (special precache variant).
   * This method is used by the engine for a precache draw. Usually it's
   * behaviour differs from RenderView() and is thus unsuitable for normal
   * drawing.
   */
  virtual bool PrecacheView (iView* view) = 0;
};

/**
 * Interface for automatic view-to-texture rendering. Exposed by render
 * managers which support this functionality.
 */
struct iRenderManagerTargets : public virtual iBase
{
  SCF_INTERFACE(iRenderManagerTargets,1,0,1);

  /// Flags for target registration
  enum TargetFlags
  {
    /// Only render to the target once
    updateOnce = 1,
    /**
     * Assumes the target is used every frame - means it is rendered to
     * every frame.
     * \remark If this flag is set, but the texture is actually not used,
     *   this is a waste of cycles. Consider manual marking with MarkAsUsed()
     *   if the texture is only used some times.
     */
    assumeAlwaysUsed = 2,

    /// Clear the frame buffer before rendering to it.
    clearScreen = 4
  };
  /**
   * Register a texture and view that should be rendered to the texture.
   * The view is rendered automatically when the texture is used.
   * \param target The texture to render to.
   * \param view The view to render.
   * \param subtexture The subtexture. Typically the face of a cube map
   *   texture.
   * \param flags Combination of TargetFlags.
   * \remark If the combination \a target, \a subtexture was mapped to another
   * view before that mapping is removed.
   */
  virtual void RegisterRenderTarget (iTextureHandle* target, 
    iView* view, int subtexture = 0, uint flags = 0) = 0;
  /// Unregister a texture to automatically render to.
  virtual void UnregisterRenderTarget (iTextureHandle* target,
    int subtexture = 0) = 0;
  /**
   * Manually mark a texture as used.
   * Useful when the texture isn't used in the world itself (e.g. for HUD
   * rendering) and thus is not detected as used by the render manager.
   */
  virtual void MarkAsUsed (iTextureHandle* target) = 0;
};

/**
 * Interface to add post-effects layers
 */
struct iRenderManagerPostEffects : public virtual iBase
{
  SCF_INTERFACE(iRenderManagerPostEffects,1,0,0);

  virtual void ClearLayers() = 0;
  virtual bool AddLayersFromDocument (iDocumentNode* node) = 0;
  virtual bool AddLayersFromFile (const char* filename) = 0;
};

/**
 * Interface for render managers which implement a visculler.
 */
struct iRenderManagerVisCull : public virtual iBase
{
  SCF_INTERFACE(iRenderManagerVisCull,1,0,0);

  virtual csPtr<iVisibilityCuller> GetVisCuller () = 0;
};

/**
  * Interface for a render manager to provide access to the parameters of 
  * global illumination techniques.
  */
struct iRenderManagerGlobalIllum : public virtual iBase
{
  SCF_INTERFACE(iRenderManagerGlobalIllum, 1, 0, 0);
    
  virtual void EnableGlobalIllumination (bool enable) = 0;

  virtual void ChangeBufferResolution (const char *bufferResolution) = 0;

  virtual void EnableBlurPass (bool enableBlur) = 0;

  virtual void ChangeNormalsAndDepthResolution (const char *resolution) = 0;

  virtual csShaderVariable* GetGlobalIllumVariableAdd(const char *svName) = 0;

  virtual csShaderVariable* GetBlurVariableAdd(const char *svName) = 0;

  virtual csShaderVariable* GetCompositionVariableAdd(const char *svName) = 0;

  /*virtual void SetSamplingPatternSize (int samplingPatternSize) = 0;

  virtual void SetNumberOfSamples (int numberOfSamples) = 0;

  virtual void SetSampleRadius (float sampleRadius) = 0;

  virtual void SetOcclusionStrength (float occlusionStrength) = 0;

  virtual void SetDepthBias (float depthBias) = 0;

  virtual void SetMaxOccluderDistance (float maxOccluderDistance) = 0;

  virtual void SetLightRotationAngle (float lightRotation) = 0;

  virtual void SetBounceStrength (float bounceStrength) = 0;

  virtual void SetBlurKernelSize (int kernelSize) = 0;

  virtual void SetBlurPositionThreshold (float positionThreshold) = 0;

  virtual void SetBlurNormalThreshold (float normalThreshold) = 0;*/
};

#endif // __CS_IENGINE_RENDERMANAGER_H__
