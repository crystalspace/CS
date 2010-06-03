/*
    Copyright (C) 2008 by Joe Forte

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __DEFERREDLIGHTRENDER_H__
#define __DEFERREDLIGHTRENDER_H__

#include "cssysdef.h"

#include "csgeom/vector3.h"
#include "ivideo/graph3d.h"
#include "cstool/genmeshbuilder.h"

CS_PLUGIN_NAMESPACE_BEGIN(RMDeferred)
{

  /**
   * Renderer for a single context where all lights are drawn
   * using information from a GBuffer.
   *
   * Usage: 
   *  1. Fill GBuffer with desired information.
   *  2. Attach accumulation buffer to receive lighting data.
   *  3. Iterate over each light in the context.
   *  4. Call FinishDraw()
   *
   * Example:
   * \code
   * // ... Fill GBuffer with data etc. ...
   *
   * {
   *   DeferredLightRenderer render (g3d, stringSet, rview
   *     shaderManager, persistentData);
   *
   *   ForEachLight (context, render);
   *   g3d->FinishDraw();
   * }
   *
   * // ... apply post processing ...
   * \endcode
   */
  class DeferredLightRenderer
  {
  public:

    /**
     * Data used by the light renderer that needs to persist over multiple 
     * frames. Render managers must store an instance of this class and 
     * provide it to the helper upon instantiation.
     */
    struct PersistentData
    {
      /* Mesh used for drawing point lights. Assumed to be center and the 
       * origin with a radius of 1. */
      csRef<iMeshWrapper> sphereMesh; 
      csRef<iMaterialWrapper> sphereMaterial;

      /* Mesh used for drawing spot lights. */
      csRef<iMeshWrapper> coneMesh;
      csRef<iMaterialWrapper> coneMaterial;

      /**
       * Initialize persistent data, must be called once before using the
       * light renderer.
       */
      void Initialize(iObjectRegistry *objRegistry)
      {
        using namespace CS::Geometry;

        const char *messageID = "crystalspace.rendermanager.deferred.lightrender";

        csRef<iEngine> engine = csQueryRegistry<iEngine> (objRegistry);
        csRef<iShaderManager> shaderManager = csQueryRegistry<iShaderManager> (objRegistry);
        csRef<iStringSet> stringSet = csQueryRegistryTagInterface<iStringSet> (objRegistry, 
          "crystalspace.shared.stringset");

        // Builds the sphere.
        csEllipsoid ellipsoid(csVector3 (0.0f, 0.0f, 0.0f), csVector3 (1.0f, 1.0f, 1.0f));
        int sphereDetail = 32; // TODO: make customizable.

        CS::Geometry::Sphere spherePrim (ellipsoid, sphereDetail);

        csRef<iMeshFactoryWrapper> sphereFactory = GeneralMeshBuilder::CreateFactory (engine, 
          "crystalspace.rendermanager.deferred.lightrender.sphere", 
          &spherePrim);

        sphereMesh = sphereFactory->CreateMeshWrapper ();

        // Creates the spheres material.
        sphereMaterial = engine->CreateMaterial (
          "crystalspace.rendermanager.deferred.lightrender.sphere", 
          NULL);

        sphereMesh->GetMeshObject ()->SetMaterialWrapper (sphereMaterial);

        // TODO: Replace with real deferred lighting shader.
        csRef<iLoader> loader = csQueryRegistry<iLoader> (objRegistry);
        if (!loader->LoadShader ("/shader/particles/basic_fixed.xml"))
        {
          csReport (objRegistry, CS_REPORTER_SEVERITY_WARNING,
            messageID, "Could not load basic_fixed shader");
        }

        sphereMaterial->GetMaterial ()->SetShader (stringSet->Request ("base"), 
          shaderManager->GetShader ("particles_basic_fixed"));
      }

    };

    DeferredLightRenderer(iGraphics3D *g3d, 
                          iShaderManager *shaderMgr,
                          iStringSet *stringSet,
                          CS::RenderManager::RenderView *rview, 
                          PersistentData &persistent)
      : 
    graphics3D(g3d),
    shaderMgr(shaderMgr),
    stringSet(stringSet),
    rview(rview),
    persistentData(persistent)
    {}

    ~DeferredLightRenderer() {}

    /**
     * Renders a single light.
     */
    void operator()(iLight *light)
    {
      switch (light->GetType ())
      {
      case CS_LIGHT_POINTLIGHT:
        RenderPointLight (light);
        break;
      case CS_LIGHT_DIRECTIONAL:
        RenderDirectionalLight(light);
        break;
      case CS_LIGHT_SPOTLIGHT:
        RenderSpotLight (light);
        break;
      default:
        CS_ASSERT(false);
      };
    }
   
  private:

    void RenderPointLight(iLight *light)
    {
      csShaderVariableStack &svStack = shaderMgr->GetShaderVariableStack ();

      csVector3 pos = light->GetMovable ()->GetFullPosition ();
      float range = light->GetCutoffDistance ();

      csMatrix3 scale (range, 0.0f,  0.0f,
                       0.0f,  range, 0.0f,
                       0.0f,  0.0f,  range);

      csReversibleTransform transform (scale, pos);

      iMeshObject *obj = persistentData.sphereMesh->GetMeshObject ();
      obj->SetColor (light->GetColor ());

      int num = 0;
      csRenderMesh **meshes = obj->GetRenderMeshes (num, 
        rview, 
        persistentData.sphereMesh->GetMovable (), 
        0);

      if (num == 0)
        return;

      iMaterial *mat = persistentData.sphereMaterial->GetMaterial ();
      iShader *shader = mat->GetShader (stringSet->Request ("base"));
      const size_t ticket = shader->GetTicket (*meshes[0], svStack);
      const size_t numPasses = shader->GetNumberOfPasses (ticket);

      for (size_t p = 0; p < numPasses; p++)
      {
        if (!shader->ActivatePass (ticket, p)) 
          continue;

        for (int i = 0; i < num; i++)
        {
          csRenderMesh *m = meshes[i];
          m->object2world = transform;

          if(!shader->SetupPass (ticket, m, *m, svStack))
            continue;

          graphics3D->DrawMesh (m, *m, svStack);
          
          shader->TeardownPass (ticket);
        }
        shader->DeactivatePass (ticket);
      }
    }

    void RenderSpotLight(iLight *light)
    {
    }

    void RenderDirectionalLight(iLight *light)
    {
    }

    iGraphics3D *graphics3D;
    iShaderManager *shaderMgr;
    iStringSet *stringSet;
    CS::RenderManager::RenderView *rview;

    PersistentData &persistentData;
  };

}
CS_PLUGIN_NAMESPACE_END(RMDeferred)

#endif // __DEFERREDLIGHTRENDER_H__
