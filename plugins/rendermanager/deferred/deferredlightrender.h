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

#include "csutil/cfgacc.h"

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
     *     a /  | h             t is half the outer falloff angle (outer = cos(2t)),
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

    /* Use the half-angle formula to relate cos(a) to cos(a/2). */
    float houter = CosineHalfAngle (outer);

    float r = (range / houter) * sqrt (1 - houter * houter);

    csMatrix3 m (r, 0,      0,
                 0, 0, -range,
                 0, r,      0);
    csVector3 v (0, 0, range);

    return csReversibleTransform (m, v) * movable->GetFullTransform ();
  }

  /**
   * Creates a transform that will transform a 1x1x1 cube centered at the origin
   * to match the given bounding box (assumed to be in world space).
   */
  inline csReversibleTransform CreateBBoxTransform(const csBox3 &bbox)
  {
    csVector3 size = bbox.GetSize ();

    csMatrix3 scale (size.x,   0.0f,   0.0f,
                       0.0f, size.y,   0.0f,
                       0.0f,   0.0f, size.z);

    csReversibleTransform (scale, bbox.GetCenter ());
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
        /* TODO */
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
    iMovable *movable = light->GetMovable ();
    csVector3 pos = movable->GetFullPosition ();
    float range = light->GetCutoffDistance ();

    float inner, outer;
    light->GetSpotLightFalloff (inner, outer);

    // Gets the spot light direction.
    csReversibleTransform light2world = movable->GetFullTransform ();
    csVector3 d = csVector3::Unit (light2world.GetT2O () * csVector3 (0.0f, 0.0f, 1.0f));
    csVector3 u = p - pos;

    float dot_ud = u * d;
    if (dot_ud <= 0.0f || dot_ud >= range)
      return false;

    float cang = dot_ud / u.Norm ();

    /* Use the half-angle formula to relate cos(a) to cos(a/2). */
    float houter = CosineHalfAngle (outer);

    if (cang < houter)
      return false;

    return true;
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
        return true; // All points are affected by a directional light.
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

      /* Mesh used for drawing spot lights. Assumed to be a cone with a 
       * height and radius of 1, aligned with the positive y-axis, and its
       * base centered at the origin. */
      csRef<iMeshWrapper> coneMesh;
      csRef<iMaterialWrapper> coneMaterial;

      /* Mesh and material used for drawing point lights. */
      csRef<iMeshWrapper> pointMesh;
      csRef<iMaterialWrapper> pointMaterial;

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

        // Creates the spheres material.
        sphereMaterial = engine->CreateMaterial (
          "crystalspace.rendermanager.deferred.lightrender.sphere", 
          NULL);

        sphereMesh->GetMeshObject ()->SetMaterialWrapper (sphereMaterial);

        if (!loader->LoadShader ("/shader/deferred/point_light.xml"))
        {
          csReport (objRegistry, CS_REPORTER_SEVERITY_WARNING,
            messageID, "Could not load deferred_point_light shader");
        }

        iShader *pointLightShader = shaderManager->GetShader ("deferred_point_light");
        sphereMaterial->GetMaterial ()->SetShader (stringSet->Request ("gbuffer use"), pointLightShader);

        // Builds the cone.
        int coneDetail = cfg->GetInt ("RenderManager.Deferred.ConeDetail", 32);
        CS::Geometry::Cone conePrim (1.0f, 1.0f, coneDetail);

        csRef<iMeshFactoryWrapper> coneFactory = GeneralMeshBuilder::CreateFactory (engine, 
          "crystalspace.rendermanager.deferred.lightrender.cone", 
          &conePrim);

        coneMesh = coneFactory->CreateMeshWrapper ();

        // Creates the cone material.
        coneMaterial = engine->CreateMaterial (
          "crystalspace.rendermanager.deferred.lightrender.cone", 
          NULL);

        coneMesh->GetMeshObject ()->SetMaterialWrapper (coneMaterial);

        if (!loader->LoadShader ("/shader/deferred/spot_light.xml"))
        {
          csReport (objRegistry, CS_REPORTER_SEVERITY_WARNING,
            messageID, "Could not load deferred_spot_light shader");
        }

        iShader *spotLightShader = shaderManager->GetShader ("deferred_spot_light");
        coneMaterial->GetMaterial ()->SetShader (stringSet->Request ("gbuffer use"), spotLightShader);
      }

    };

    DeferredLightRenderer(iGraphics3D *g3d, 
                          iShaderManager *shaderMgr,
                          iStringSet *stringSet,
                          CS::RenderManager::RenderView *rview,
                          iTextureHandle *gBuffer0, 
                          iTextureHandle *gBuffer1,
                          iTextureHandle *gBuffer2,
                          iTextureHandle *gBufferDepth,
                          PersistentData &persistent)
      : 
    graphics3D(g3d),
    shaderMgr(shaderMgr),
    stringSet(stringSet),
    rview(rview),
    persistentData(persistent)
    {
      iShaderVarStringSet *svStringSet = shaderMgr->GetSVNameStringset ();

      csShaderVariable *gBuffer0SV = shaderMgr->GetVariableAdd (svStringSet->Request ("tex gbuffer 0"));
      csShaderVariable *gBuffer1SV = shaderMgr->GetVariableAdd (svStringSet->Request ("tex gbuffer 1"));
      csShaderVariable *gBuffer2SV = shaderMgr->GetVariableAdd (svStringSet->Request ("tex gbuffer 2"));
      csShaderVariable *gBufferDSV = shaderMgr->GetVariableAdd (svStringSet->Request ("tex gbuffer depth"));

      gBuffer0SV->SetValue (gBuffer0);
      gBuffer1SV->SetValue (gBuffer1);
      gBuffer2SV->SetValue (gBuffer2);
      gBufferDSV->SetValue (gBufferDepth);
    }

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
        csVector3 lightDir = movable->GetFullTransform ().This2Other (
          csVector3 (0.0f, 0.0f, 1.0f));
        lightDir = world2camera.This2Other (lightDir);

        csShaderVariable *lightDirSV = lightSVContext->GetVariableAdd (
          svStringSet->Request ("light direction view"));
        lightDirSV->SetValue (csVector3::Unit (lightDir));
      }
    }

    void RenderPointLight(iLight *light)
    {
      RenderLight (light,
                   persistentData.sphereMesh->GetMeshObject (),
                   persistentData.sphereMaterial->GetMaterial ());
    }

    void RenderSpotLight(iLight *light)
    {
      graphics3D->SetRenderState (G3DRENDERSTATE_EDGES, 1);

      RenderLight (light,
                   persistentData.coneMesh->GetMeshObject (),
                   persistentData.coneMaterial->GetMaterial ());
      
      graphics3D->SetRenderState (G3DRENDERSTATE_EDGES, 0);
    }

    void RenderDirectionalLight(iLight *light)
    {
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

      // Draw the point light mesh.
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

    iGraphics3D *graphics3D;
    iShaderManager *shaderMgr;
    iStringSet *stringSet;
    CS::RenderManager::RenderView *rview;

    PersistentData &persistentData;
  };

}
CS_PLUGIN_NAMESPACE_END(RMDeferred)

#endif // __DEFERREDLIGHTRENDER_H__
