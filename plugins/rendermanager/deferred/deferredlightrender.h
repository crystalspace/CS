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
#include "csgeom/projections.h"
#include "ivideo/graph3d.h"
#include "cstool/genmeshbuilder.h"

#include "csutil/cfgacc.h"

#include "gbuffer.h"

CS_PLUGIN_NAMESPACE_BEGIN(RMDeferred)
{

  /**
   * Given the cosine of the angle a, returns the cosine of the angle a / 2.
   * 
   * cos(a/2) = +or- sqrt((1 + cos(a)) / 2) NOTE: The + form is returned.
   */
  inline float CosineHalfAngle(float c)
  {
    return sqrt ((1 + c) / 2.0f);
  }

   /**
    * Returns the given lights direction (only valid for spot and directional lights).
    */
  inline csVector3 GetLightDir(iLight *light)
  {
    iMovable *mov = light->GetMovable ();
    csVector3 d = mov->GetFullTransform ().GetT2O () * csVector3 (0.0f, 0.0f, 1.0f);

    return csVector3::Unit (d);
  }

  /**
   * Creates a transform that will transform a sphere centered at the origin 
   * with a radius of 1 to match the position and size of the given point light.
   */
  inline csReversibleTransform CreatePointLightTransform(iLight *light)
  {
    csVector3 pos = light->GetMovable ()->GetFullPosition ();
    float range = light->GetCutoffDistance ();

    csMatrix3 scale (range,  0.0f,  0.0f,
                      0.0f, range,  0.0f,
                      0.0f,  0.0f, range);

    return csReversibleTransform (scale, pos);
  }

  /**
   * Creates a transform that will transform a cone cone with a height and 
   * radius of 1, aligned with the positive y-axis, and its base centered at 
   * the origin.
   */
  inline csReversibleTransform CreateSpotLightTransform(iLight *light)
  {
    iMovable *movable = light->GetMovable ();
    csVector3 pos = movable->GetFullPosition ();
    float range = light->GetCutoffDistance ();

    float inner, outer;
    light->GetSpotLightFalloff (inner, outer);

    /* To compute the radius of the light cone we note the following diagram:
     *
     *         /|         Where r is the radius of the cone base,
     *        /t|               h is the height of the cone (h = range),
     *     a /  | h             t is half the outer falloff angle (outer = cos(t)),
     *      /___|               a is the length of the hypotenuse.
     *        r
     *
     *  From this diagram we know that cos(t) = h/a and, from pythagoras theorem, 
     *  a^2 = h^2 + r^2.
     *  
     *  From these equations we can solve for r:
     *    1. a^2 = h^2 + r^2 => r = sqrt(a^2 - h^2)
     *    2. cos(t) = h/a => a = h/cos(t)
     *
     *  Combining 1. and 2. we get:
     *    r = sqrt((h/cos(t))^2 - h^2)
     *      = sqrt(h^2*(1 - cos(t)^2) / cos(t)^2) 
     *      = h/cos(t) * sqrt(1 - cos(t)^2)
     */
    float r = (range / outer) * sqrt (1 - outer * outer);

    // Transforms the cone into light space.
    csMatrix3 m (r, 0,      0,
                 0, 0, -range,
                 0, r,      0);
    csVector3 v (0, 0,  range);

    return csReversibleTransform (m, v) * movable->GetFullTransform ();
  }

  /**
   * Creates a transform that will transform a 1x1x1 cube centered at the origin
   * to match the given directional light.
   */
  inline csReversibleTransform CreateDirectionalLightTransform(iLight *light)
  {
    iMovable *movable = light->GetMovable ();

    float z = light->GetCutoffDistance ();
    float r = light->GetDirectionalCutoffRadius ();

    csMatrix3 S (r, 0, 0,
                 0, z, 0,
                 0, 0, r);

    csMatrix3 T = S * movable->GetFullTransform ().GetO2T ();

    return csReversibleTransform (T, movable->GetFullPosition ());
  }

  /**
   * Creates a transform that will transform a 1x1x1 cube centered at the origin
   * to match the given bounding box (assumed to be in world space).
   */
  inline csReversibleTransform CreateLightTransform(iLight *light)
  {
      switch (light->GetType ())
      {
      case CS_LIGHT_POINTLIGHT:
        return CreatePointLightTransform (light);
        break;
      case CS_LIGHT_DIRECTIONAL:
        return CreateDirectionalLightTransform (light);
        break;
      case CS_LIGHT_SPOTLIGHT:
        return CreateSpotLightTransform (light);
        break;
      default:
        CS_ASSERT(false);
      };

      return csReversibleTransform ();
  }

  /**
   * Creates an orthographic projection suitable for drawing fullscreen quads.
   */
  inline CS::Math::Matrix4 CreateOrthoProj(iGraphics3D *graphics3D)
  {
    float w = graphics3D->GetDriver2D ()->GetWidth ();
    float h = graphics3D->GetDriver2D ()->GetHeight ();

    return CS::Math::Projections::Ortho (0, w, 0, h, -1.0f, 10.0f);
  }

  /**
   * Returns true if the given point is inside the volume of the given point light.
   */
  inline bool IsPointInsidePointLight(const csVector3 &p, iLight *light)
  {
    const float r = light->GetCutoffDistance ();
    const csVector3 c = light->GetMovable ()->GetFullPosition ();

    return (p - c).SquaredNorm () <= (r * r);
  }

  /**
   * Returns true if the given point is inside the volume of the given spot light.
   */
  inline bool IsPointInsideSpotLight(const csVector3 &p, iLight *light)
  {
    csVector3 pos = light->GetMovable ()->GetFullPosition ();
    float range = light->GetCutoffDistance ();

    float inner, outer;
    light->GetSpotLightFalloff (inner, outer);

    // Gets the spot light direction.
    csVector3 d = GetLightDir (light);
    csVector3 u = p - pos;

    float dot_ud = u * d;
    if (dot_ud <= 0.0f || dot_ud >= range)
      return false;

    float cang = dot_ud / u.Norm ();
    if (cang < outer)
      return true;

    return false;
  }

  /**
   * Returns true if the given point is inside the volume of the given directional light.
   */
  inline bool IsPointInsideDirectionalLight(const csVector3 &p, iLight *light)
  {
    csBox3 box;
    box.SetCenter (csVector3 (0.0f, 0.0f, 0.0f));
    box.SetSize (csVector3 (2.0f, 2.0f, 2.0f));

    // Transform the point to object space and test.
    csReversibleTransform obj2world = CreateDirectionalLightTransform (light);
    csVector3 objPos = obj2world.Other2This (p);

    return !box.In (objPos);
  }

  /**
   * Returns true if the given point is inside the volume of the given light.
   */
  inline bool IsPointInsideLight(const csVector3 &p, iLight *light)
  {
      switch (light->GetType ())
      {
      case CS_LIGHT_POINTLIGHT:
        return IsPointInsidePointLight (p, light);
        break;
      case CS_LIGHT_SPOTLIGHT:
        return IsPointInsideSpotLight (p, light);
        break;
      case CS_LIGHT_DIRECTIONAL:
        return IsPointInsideDirectionalLight (p, light);
        break;
      default:
        CS_ASSERT(false);
      };

      return false;
  }

  /**
   * Renderer for a single context where all lights are drawn
   * using information from a GBuffer.
   *
   * Usage: 
   *  1. Fill GBuffer with desired information.
   *  2. Attach accumulation buffer to receive lighting data.
   *  3. Output ambient light.
   *  4. Iterate over each light in the context.
   *  5. Call FinishDraw()
   *
   * Example:
   * \code
   * // ... Fill GBuffer with data etc. ...
   *
   * {
   *   DeferredLightRenderer render (...);
   *
   *   render.OutputAmbientLight();
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
      /* Mesh used for drawing point lights. Assumed to be center at the 
       * origin with a radius of 1. */
      csRef<iMeshWrapper> sphereMesh; 

      /* Mesh used for drawing spot lights. Assumed to be a cone with a 
       * height and radius of 1, aligned with the positive y-axis, and its
       * base centered at the origin. */
      csRef<iMeshWrapper> coneMesh;

      /* Mesh and material used for drawing directional lights. Assumed to be
       * a 1x1x1 box centered at the origin. */
      csRef<iMeshWrapper> boxMesh;

      /* Mesh and material used for drawing ambient light. Assumed to be in the
       * xy plane with the bottom left corner at the origin with a width and 
       * height of 1 along the positive x and y axes. */
      csRef<iMeshWrapper> quadMesh;

      /* Materials used for computing lighting results. */
      csRef<iMaterialWrapper> pointMaterial;
      csRef<iMaterialWrapper> spotMaterial;
      csRef<iMaterialWrapper> directionalMaterial;
      csRef<iMaterialWrapper> ambientMaterial;

      /**
       * Initialize persistent data, must be called once before using the
       * light renderer.
       */
      void Initialize(iObjectRegistry *objRegistry)
      {
        using namespace CS::Geometry;

        const char *messageID = "crystalspace.rendermanager.deferred.lightrender";

        csRef<iEngine> engine = csQueryRegistry<iEngine> (objRegistry);
        csRef<iLoader> loader = csQueryRegistry<iLoader> (objRegistry);
        csRef<iShaderManager> shaderManager = csQueryRegistry<iShaderManager> (objRegistry);
        csRef<iStringSet> stringSet = csQueryRegistryTagInterface<iStringSet> (objRegistry, 
          "crystalspace.shared.stringset");

        csConfigAccess cfg (objRegistry);

        // Builds the sphere.
        csEllipsoid ellipsoid(csVector3 (0.0f, 0.0f, 0.0f), csVector3 (1.0f, 1.0f, 1.0f));
        int sphereDetail = cfg->GetInt ("RenderManager.Deferred.SphereDetail", 32);

        CS::Geometry::Sphere spherePrim (ellipsoid, sphereDetail);

        csRef<iMeshFactoryWrapper> sphereFactory = GeneralMeshBuilder::CreateFactory (engine, 
          "crystalspace.rendermanager.deferred.lightrender.sphere", 
          &spherePrim);

        sphereMesh = sphereFactory->CreateMeshWrapper ();

        // Creates the point light material.
        pointMaterial = engine->CreateMaterial (
          "crystalspace.rendermanager.deferred.lightrender.point", 
          NULL);

        sphereMesh->GetMeshObject ()->SetMaterialWrapper (pointMaterial);

        if (!loader->LoadShader ("/shader/deferred/point_light.xml"))
        {
          csReport (objRegistry, CS_REPORTER_SEVERITY_WARNING,
            messageID, "Could not load deferred_point_light shader");
        }

        iShader *pointLightShader = shaderManager->GetShader ("deferred_point_light");
        pointMaterial->GetMaterial ()->SetShader (stringSet->Request ("gbuffer use"), pointLightShader);

        // Builds the cone.
        int coneDetail = cfg->GetInt ("RenderManager.Deferred.ConeDetail", 32);
        CS::Geometry::Cone conePrim (1.0f, 1.0f, coneDetail);

        csRef<iMeshFactoryWrapper> coneFactory = GeneralMeshBuilder::CreateFactory (engine, 
          "crystalspace.rendermanager.deferred.lightrender.cone", 
          &conePrim);

        coneMesh = coneFactory->CreateMeshWrapper ();

        // Creates the spot light material.
        spotMaterial = engine->CreateMaterial (
          "crystalspace.rendermanager.deferred.lightrender.spot", 
          NULL);

        coneMesh->GetMeshObject ()->SetMaterialWrapper (spotMaterial);

        if (!loader->LoadShader ("/shader/deferred/spot_light.xml"))
        {
          csReport (objRegistry, CS_REPORTER_SEVERITY_WARNING,
            messageID, "Could not load deferred_spot_light shader");
        }

        iShader *spotLightShader = shaderManager->GetShader ("deferred_spot_light");
        spotMaterial->GetMaterial ()->SetShader (stringSet->Request ("gbuffer use"), spotLightShader);

        // Builds the box.
        csBox3 box;
        box.SetCenter (csVector3 (0.0f, 0.0f, 0.0f));
        box.SetSize (csVector3 (2.0f, 2.0f, 2.0f));
        CS::Geometry::TesselatedBox boxPrim (box);

        csRef<iMeshFactoryWrapper> boxFactory = GeneralMeshBuilder::CreateFactory (engine, 
          "crystalspace.rendermanager.deferred.lightrender.box", 
          &boxPrim);

        boxMesh = boxFactory->CreateMeshWrapper ();

        // Creates the directional material.
        directionalMaterial = engine->CreateMaterial (
          "crystalspace.rendermanager.deferred.lightrender.directional", 
          NULL);

        boxMesh->GetMeshObject ()->SetMaterialWrapper (directionalMaterial);

        if (!loader->LoadShader ("/shader/deferred/directional_light.xml"))
        {
          csReport (objRegistry, CS_REPORTER_SEVERITY_WARNING,
            messageID, "Could not load deferred_directional_light shader");
        }

        iShader *directionalLightShader = shaderManager->GetShader ("deferred_directional_light");
        directionalMaterial->GetMaterial ()->SetShader (stringSet->Request ("gbuffer use"), 
          directionalLightShader);

        // Builds the quad.
        CS::Geometry::TesselatedQuad quadPrim (csVector3 (0.0f, 0.0f, 0.0f), 
                                               csVector3 (0.0f, 1.0f, 0.0f),
                                               csVector3 (1.0f, 0.0f, 0.0f));

        csRef<iMeshFactoryWrapper> quadFactory = GeneralMeshBuilder::CreateFactory (engine, 
          "crystalspace.rendermanager.deferred.lightrender.quad", 
          &quadPrim);

        quadMesh = quadFactory->CreateMeshWrapper ();

        // Creates the ambient material.
        ambientMaterial = engine->CreateMaterial (
          "crystalspace.rendermanager.deferred.lightrender.ambient", 
          NULL);

        quadMesh->GetMeshObject ()->SetMaterialWrapper (ambientMaterial);

        if (!loader->LoadShader ("/shader/deferred/ambient_light.xml"))
        {
          csReport (objRegistry, CS_REPORTER_SEVERITY_WARNING,
            messageID, "Could not load deferred_ambient_light shader");
        }

        iShader *ambientLightShader = shaderManager->GetShader ("deferred_ambient_light");
        ambientMaterial->GetMaterial ()->SetShader (stringSet->Request ("gbuffer use"), ambientLightShader);
      }

    };

    DeferredLightRenderer(iGraphics3D *g3d, 
                          iShaderManager *shaderMgr,
                          iStringSet *stringSet,
                          CS::RenderManager::RenderView *rview,
                          GBuffer &gbuffer,
                          PersistentData &persistent)
      : 
    graphics3D(g3d),
    shaderMgr(shaderMgr),
    stringSet(stringSet),
    rview(rview),
    persistentData(persistent)
    {
      gbuffer.UpdateShaderVars (shaderMgr);
    }

    ~DeferredLightRenderer() {}

    /**
     * Outputs the ambient light present in the gbuffer. Call once per-frame.
     */
    void OutputAmbientLight()
    {
      RenderAmbientLight ();
    }

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

    /**
     * Sets shader variables spacific to the given light.
     */
    void SetupLightShaderVars(iLight *light)
    {
      const csReversibleTransform &world2camera = graphics3D->GetWorldToCamera ();

      iShaderVarStringSet *svStringSet = shaderMgr->GetSVNameStringset ();
      iShaderVariableContext *lightSVContext = light->GetSVContext ();
      iMovable *movable = light->GetMovable ();
      csLightType type = light->GetType ();

      // Transform light position to view space.
      if (type == CS_LIGHT_POINTLIGHT || type == CS_LIGHT_SPOTLIGHT)
      {
        csVector3 lightPos = movable->GetFullPosition ();
        lightPos = world2camera.This2Other (lightPos);

        csShaderVariable *lightPosSV = lightSVContext->GetVariableAdd (
          svStringSet->Request ("light position view"));
        lightPosSV->SetValue (lightPos);
      }

      // Transform light direction to view space.
      if (type == CS_LIGHT_DIRECTIONAL || type == CS_LIGHT_SPOTLIGHT)
      {
        csVector3 lightDir = GetLightDir (light);
        lightDir = world2camera.GetT2O () * lightDir;

        csShaderVariable *lightDirSV = lightSVContext->GetVariableAdd (
          svStringSet->Request ("light direction view"));
        lightDirSV->SetValue (csVector3::Unit (lightDir));
      }
    }

    void RenderPointLight(iLight *light)
    {
      RenderLight (light,
                   persistentData.sphereMesh->GetMeshObject (),
                   persistentData.pointMaterial->GetMaterial ());
    }

    void RenderSpotLight(iLight *light)
    {
      RenderLight (light,
                   persistentData.coneMesh->GetMeshObject (),
                   persistentData.spotMaterial->GetMaterial ());
    }

    void RenderDirectionalLight(iLight *light)
    {
      RenderLight (light,
                   persistentData.boxMesh->GetMeshObject (),
                   persistentData.directionalMaterial->GetMaterial ());
    }

    void RenderAmbientLight()
    {
      iMaterial *mat = persistentData.ambientMaterial->GetMaterial ();

      // Update shader stack.
      csShaderVariableStack svStack = shaderMgr->GetShaderVariableStack ();
      iShader *shader = mat->GetShader (stringSet->Request ("gbuffer use"));

      svStack.Clear ();
      shaderMgr->PushVariables (svStack);
      shader->PushVariables (svStack);

      DrawFullscreenQuad (shader, svStack, CS_ZBUF_NONE);
    }

    /**
     * Renders a light mesh.
     */
    void RenderLight(iLight *light, 
                     iMeshObject *obj, 
                     iMaterial *mat)
    {
      int num = 0;
      csRenderMesh **meshes = obj->GetRenderMeshes (num, rview, 
        persistentData.sphereMesh->GetMovable (), 0);

      if (num <= 0)
        return;

      // Setup shader variables.
      SetupLightShaderVars (light);

      // Update shader stack.
      csShaderVariableStack svStack = shaderMgr->GetShaderVariableStack ();
      iShader *shader = mat->GetShader (stringSet->Request ("gbuffer use"));
      iShaderVariableContext *lightSVContext = light->GetSVContext ();

      svStack.Clear ();
      shaderMgr->PushVariables (svStack);
      lightSVContext->PushVariables (svStack);
      shader->PushVariables (svStack);

      // Draw the light mesh.
      iCamera *cam = rview->GetCamera ();
      csVector3 camPos = cam->GetTransform ().This2Other (csVector3 (0.0f, 0.0f, 1.0f));
      
      DrawLightMesh (meshes, 
                     num, 
                     CreateLightTransform (light), 
                     shader, 
                     svStack,
                     IsPointInsideLight (camPos, light));
    }

    /**
     * Draws the given light mesh using the supplied shader.
     */
    void DrawLightMesh(csRenderMesh **meshes, 
                       int n, 
                       const csReversibleTransform &transform,
                       iShader *shader,
                       csShaderVariableStack &svStack,
                       bool insideLight)
    {
      CS_ASSERT (n > 0);

      const size_t ticket = shader->GetTicket (*meshes[0], svStack);
      const size_t numPasses = shader->GetNumberOfPasses (ticket);

      CS::Graphics::MeshCullMode cullMode = CS::Graphics::cullNormal;
      if (insideLight)
        cullMode = CS::Graphics::cullFlipped;

      for (size_t p = 0; p < numPasses; p++)
      {
        if (!shader->ActivatePass (ticket, p)) 
          continue;

        for (int i = 0; i < n; i++)
        {
          csRenderMesh *m = meshes[i];
          
          if (!shader->SetupPass (ticket, m, *m, svStack))
            continue;

          // Use additive blending so we do not loose the contributions from other lights.
          m->mixmode = CS_FX_ADD;
          m->cullMode = cullMode;
          m->object2world = transform;

          graphics3D->DrawMesh (m, *m, svStack);

          shader->TeardownPass (ticket);
        }
        shader->DeactivatePass (ticket);
      }
    }

    /**
     * Draws a fullscreen quad using the supplied shader.
     */
    void DrawFullscreenQuad(iShader *shader,
                            csShaderVariableStack &svStack,
                            csZBufMode zmode)
    {
      iMeshObject *obj = persistentData.quadMesh->GetMeshObject ();

      int num = 0;
      csRenderMesh **meshes = obj->GetRenderMeshes (num, rview, 
        persistentData.quadMesh->GetMovable (), 0);

      if (num <= 0)
        return;

      // Switches to using orthographic projection. 
      csReversibleTransform oldView = graphics3D->GetWorldToCamera ();
      CS::Math::Matrix4 oldProj = graphics3D->GetProjectionMatrix ();

      graphics3D->SetWorldToCamera (csReversibleTransform ());
      graphics3D->SetProjectionMatrix (CreateOrthoProj (graphics3D));

      // Create quad transform.
      float w = graphics3D->GetDriver2D ()->GetWidth ();
      float h = graphics3D->GetDriver2D ()->GetHeight ();

      csMatrix3 S (w, 0, 0,
                   0, h, 0,
                   0, 0, 1);

      csReversibleTransform T (S, csVector3 (0.0f, 0.0f, 0.0f));

      const size_t ticket = shader->GetTicket (*meshes[0], svStack);
      const size_t numPasses = shader->GetNumberOfPasses (ticket);

      for (size_t p = 0; p < numPasses; p++)
      {
        if (!shader->ActivatePass (ticket, p)) 
          continue;

        for (int i = 0; i < num; i++)
        {
          csRenderMesh *m = meshes[i];

          if (!shader->SetupPass (ticket, m, *m, svStack))
            continue;

          // Use additive blending so we do not loose the contributions from other lights.
          m->z_buf_mode = zmode;
          m->object2world = T;

          graphics3D->DrawMesh (m, *m, svStack);

          shader->TeardownPass (ticket);
        }
        shader->DeactivatePass (ticket);
      }

      // Restores old transforms.
      graphics3D->SetWorldToCamera (oldView);
      graphics3D->SetProjectionMatrix (oldProj);

      // Needed for the change in projection matrix to take effect. 
      graphics3D->BeginDraw (CSDRAW_3DGRAPHICS);
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
