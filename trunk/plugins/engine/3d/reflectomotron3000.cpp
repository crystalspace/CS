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

#include "iengine/rendermanager.h"
#include "ivaria/view.h"
#include "ivideo/txtmgr.h"

#include "cstool/csview.h"

#include "camera.h"
#include "engine.h"
#include "meshobj.h"
#include "reflectomotron3000.h"

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{
  namespace EnvTex
  {
    void Holder::Clear ()
    {
      usedTextures.DeleteAll ();
      freeTextures.DeleteAll ();
    }

    void Holder::NextFrame ()
    {
      size_t i = 0;
      csTicks currentTime = csGetTicks();
      while (i < usedTextures.GetSize())
      {
        if (currentTime > usedTextures[i].deathTime)
        {
          freeTextures.Push (usedTextures[i]);
          usedTextures.DeleteIndexFast (i);
        }
        else
          i++;
      }
    }

    iTextureHandle* Holder::QueryUnusedTexture (int size, 
                                                csTicks lifetime)
    {
      HeldTexture tex;
      if (freeTextures.GetSize() > 0)
      {
        tex = freeTextures.Pop ();
        tex.deathTime = csGetTicks() + lifetime;
        usedTextures.Push (tex);
        return tex.texture;
      }
      tex.texture = engine->G3D->GetTextureManager()->CreateTexture (size, size,
        csimgCube, "rgb8", CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS);
        tex.deathTime = csGetTicks() + lifetime;
      usedTextures.Push (tex);
      return tex.texture;
    }

    //-----------------------------------------------------------------------

    static const int updateInterval = 15;

    Accessor::Accessor (csMeshWrapper* meshwrap) : 
      scfImplementationType (this), meshwrap (meshwrap),
      envTex (0), lastUpdateFrame ((uint)~0), lastUpdateTime (0)
    {
    }

    void Accessor::PreGetValue (csShaderVariable *variable)
    {
      csEngine* engine = meshwrap->engine;
      csTicks currentTime = csGetTicks();
      if ((engine->GetCurrentFrameNumber() == lastUpdateFrame)
        || (currentTime - updateInterval < lastUpdateTime))
      {
        variable->SetValue (envTex);
        return;
      }
      
      csRef<iRenderManagerTargets> rmTargets =
        scfQueryInterface<iRenderManagerTargets> (engine->renderManager);
      if (!rmTargets) return;
      
      lastUpdateFrame = engine->GetCurrentFrameNumber();
      lastUpdateTime = currentTime;
      iMovable* movable = meshwrap->GetMovable();

      const int textureSize = 256;
      envTex = engine->envTexHolder.QueryUnusedTexture (textureSize,
        updateInterval);
      variable->SetValue (envTex);

      static const csVector3 lookAtVecs[12] = {
        csVector3 ( 1.0f,  0.0f,  0.0f), csVector3 (0.0f, 1.0f,  0.0f), 
        csVector3 (-1.0f,  0.0f,  0.0f), csVector3 (0.0f, 1.0f,  0.0f), 
        csVector3 ( 0.0f,  1.0f,  0.0f), csVector3 (0.0f, 0.0f, -1.0f), 
        csVector3 ( 0.0f, -1.0f,  0.0f), csVector3 (0.0f, 0.0f, -1.0f),
        csVector3 ( 0.0f,  0.0f,  1.0f), csVector3 (0.0f, 1.0f,  0.0f), 
        csVector3 ( 0.0f,  0.0f, -1.0f), csVector3 (0.0f, 1.0f,  0.0f) 
      };

      iSectorList* meshSectors = movable->GetSectors ();
      iSector* meshSector = meshSectors->Get (0);

      csBox3 bbox = meshwrap->GetObjectModel ()->GetObjectBoundingBox ();
      csVector3 meshPos = movable->GetFullPosition();
      meshPos += bbox.GetCenter();

      for (int i = 0; i < 6; i++)
      {
        csRef<iView> sideView;
        sideView.AttachNew (new csView (engine, engine->G3D));
        iPerspectiveCamera* cam = sideView->GetPerspectiveCamera();
        cam->GetCamera()->SetViewportSize (textureSize, textureSize);
        cam->GetCamera()->SetSector (meshSector);
        cam->GetCamera()->GetTransform().SetOrigin (meshPos);
        cam->SetFOVAngle (90, textureSize);
        cam->SetPerspectiveCenter (textureSize / 2, textureSize/ 2);
        sideView->SetRectangle (0, 0, textureSize, textureSize);
        sideView->GetMeshFilter ().AddFilterMesh (meshwrap);

        csOrthoTransform& camTF = cam->GetCamera()->GetTransform();
        camTF.LookAt (lookAtVecs[i*2], lookAtVecs[i*2 + 1]);

        rmTargets->RegisterRenderTarget (envTex, sideView, i, 
          iRenderManagerTargets::updateOnce);
      }
    }
  } // namespace EnvTex
}
CS_PLUGIN_NAMESPACE_END(Engine)
