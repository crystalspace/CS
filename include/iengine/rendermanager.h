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

struct iRenderManager : public virtual iBase
{
  SCF_INTERFACE(iRenderManager,2,0,0);

  virtual bool RenderView (iView* view) = 0;
};

struct iRenderManagerTargets : public virtual iBase
{
  SCF_INTERFACE(iRenderManagerTargets,1,0,0);

  enum TargetFlags
  {
    updateOnce = 1
  };
  virtual void RegisterRenderTarget (iTextureHandle* target, 
    iView* view, int subtexture = 0, uint flags = 0) = 0;
  virtual void UnregisterRenderTarget (iTextureHandle* target,
    int subtexture = 0) = 0;
};

struct iRenderManagerPostEffects : public virtual iBase
{
  SCF_INTERFACE(iRenderManagerPostEffects,1,0,0);

  virtual void ClearLayers() = 0;
  virtual bool AddLayersFromDocument (iDocumentNode* node) = 0;
  virtual bool AddLayersFromFile (const char* filename) = 0;
};

#endif // __CS_IENGINE_RENDERMANAGER_H__
