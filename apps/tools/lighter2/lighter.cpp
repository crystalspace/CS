/*
  Copyright (C) 2005 by Marten Svanfeldt

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

#include "crystalspace.h"

#include "common.h"
#include "lighter.h"
#include "scene.h"
#include "lightmapuv.h"
#include "raytracer.h"
#include "raygenerator.h"
#include "radprimitive.h"


CS_IMPLEMENT_APPLICATION;

// used below
CS_SPECIALIZE_TEMPLATE
class csHashComputer<lighter::RadPrimitive*> : public csHashComputerIntegral<lighter::RadPrimitive*> {};

namespace lighter
{
  RadSettings globalSettings;
  Lighter* globalLighter;

  Lighter::Lighter (iObjectRegistry *objectRegistry)
    : objectRegistry (objectRegistry), scene (new Scene)
  {
  }

  Lighter::~Lighter ()
  {
    delete scene;
  }

  bool Lighter::Initialize ()
  {
    // Load config
    if (!csInitializer::SetupConfigManager (objectRegistry,0))
      return Report ("Cannot setup config manager!");

    // Get plugins
    if (!csInitializer::RequestPlugins (objectRegistry,
            CS_REQUEST_ENGINE,
            CS_REQUEST_IMAGELOADER,
            CS_REQUEST_LEVELLOADER,
            CS_REQUEST_NULL3D,
            CS_REQUEST_REPORTER,
            CS_REQUEST_REPORTERLISTENER,
            CS_REQUEST_VFS,
            CS_REQUEST_END))
      return Report ("Cannot load plugins!");

    csRef<iStandardReporterListener> repl =
  	csQueryRegistry<iStandardReporterListener> (objectRegistry);
    if (repl)
    {
      // tune the reporter to be a bit more chatty
      repl->SetMessageDestination (
  	    CS_REPORTER_SEVERITY_BUG, false, true, true, true, true);
      repl->SetMessageDestination (
  	    CS_REPORTER_SEVERITY_ERROR, false, true, true, true, true);
      repl->SetMessageDestination (
  	    CS_REPORTER_SEVERITY_WARNING, true, false, true, false, true);
      repl->SetMessageDestination (
  	    CS_REPORTER_SEVERITY_NOTIFY, true, false, true, false, true);
      repl->SetMessageDestination (
  	    CS_REPORTER_SEVERITY_DEBUG, true, false, true, false, true);
      repl->ShowMessageID (CS_REPORTER_SEVERITY_WARNING, true);
      repl->ShowMessageID (CS_REPORTER_SEVERITY_NOTIFY, true);
    }

    // Check for commandline help.
    if (csCommandLineHelper::CheckHelp (objectRegistry))
    {
      csCommandLineHelper::Help (objectRegistry);
      return true;
    }

    // Get the plugins wants to use
    reporter = csQueryRegistry<iReporter> (objectRegistry);
    if (!reporter) return Report ("Cannot get a reporter");

    engine = csQueryRegistry<iEngine> (objectRegistry);
    if (!engine) return Report ("No iEngine!");

    imageIO = csQueryRegistry<iImageIO> (objectRegistry);
    if (!imageIO) return Report ("No iImageIO!");

    loader = csQueryRegistry<iLoader> (objectRegistry);
    if (!loader) return Report ("No iLoader!");

    pluginManager = csQueryRegistry<iPluginManager> (objectRegistry);
    if (!pluginManager) return Report ("No iPluginManager!");

    vfs = csQueryRegistry<iVFS> (objectRegistry);
    if (!vfs) return Report ("No iVFS!");

    // Open the systems
    if (!csInitializer::OpenApplication (objectRegistry))
      return Report ("Error opening system!");

    // For now, force the use of TinyXML to be able to write
    docSystem.AttachNew (new csTinyDocumentSystem);

    return true;
  }

  bool Lighter::LightEmUp ()
  {
    // Have to load to have anything to light
    if (!LoadFiles ()) return false;
    if (!scene->ParseEngine ()) return false;

    // Calculate lightmapping coordinates
    LightmapUVLayouter *uvLayout = new SimpleUVLayouter;

    RadObjectFactoryHash::GlobalIterator factIt = 
      scene->GetFactories ().GetIterator ();
    while (factIt.HasNext ())
    {
      csRef<RadObjectFactory> fact = factIt.Next ();
      fact->ComputeLightmapUV (uvLayout);
    }

    // Initialize all objects
    SectorHash::GlobalIterator sectIt = 
      scene->GetSectors ().GetIterator ();
    while (sectIt.HasNext ())
    {
      csRef<Sector> sect = sectIt.Next ();
      sect->Initialize ();
    }

    // Shoot direct lighting
    sectIt.Reset ();
    while (sectIt.HasNext ())
    {
      csRef<Sector> sect = sectIt.Next ();
      ShootDirectLighting (sect);
    }

    //@@ DO OTHER LIGHTING

    // De-antialias the lightmaps
    sectIt.Reset ();
    while (sectIt.HasNext ())
    {
      csRef<Sector> sect = sectIt.Next ();
      RadObjectHash::GlobalIterator objIt = sect->allObjects.GetIterator ();
      while (objIt.HasNext ())
      {
        csRef<RadObject> obj = objIt.Next ();
        obj->FixupLightmaps ();
      }
    }

    //Save the result
    if (!scene->SaveFiles ()) return false;
    
    return true;  
  }

  bool Lighter::Report (const char* msg, ...)
  {
    va_list arg;
    va_start (arg, msg);
    if (reporter)
    {
      reporter->ReportV (CS_REPORTER_SEVERITY_ERROR, 
        "crystalspace.application.lighter2", msg, arg);
    }
    else
    {
      csPrintfV (msg, arg);
      csPrintf ("\n");
    }
    return false;
  }

  bool Lighter::LoadFiles ()
  {
    //Parse cmd-line
    csRef<iCommandLineParser> cmdline = csQueryRegistry<iCommandLineParser>
      (objectRegistry);

    int cmd_idx = 0;
    int map_idx = 0;
    while (true)
    {
      const char *val = cmdline->GetName (cmd_idx++);
      if (!val)
      {
        if (map_idx > 0)
          break;
        else
          return Report ("Please specify a level (either zip or VFS dir)!");
      }

      map_idx++;
      scene->AddFile (val);
    }
    
    // Load the files
    return scene->LoadFiles ();
  }

  void Lighter::ShootDirectLighting (Sector* sector)
  {
    RandomRayListGenerator<PseudoRandomRaygenerator> rayGenerator;

    // Need a raytracer
    Raytracer rayTracer (sector->kdTree);

    // Iterate all lights
    LightRefArray::Iterator lightIt = sector->allLights.GetIterator ();
    while (lightIt.HasNext ())
    {
      csRef<Light> radLight = lightIt.Next ();
      radLight->freeEnergy = radLight->color * 8000.0f; //@scale

      // Generate and shoot rays
      csArray<Ray> rays = rayGenerator (0xFFF, radLight->position);

      typedef csSet<RadPrimitive*> RadPrimSet;
      RadPrimSet primsToLight;
      for (unsigned int rayIdx = 0; rayIdx < rays.GetSize (); rayIdx++)
      {
        const Ray& ray = rays[rayIdx];
        HitPoint hit;
        if (rayTracer.TraceClosestHit (ray, hit))
        {
          // YAY! do something with it..
          //printf("HAVE HIT! %s\n", hit.hitPoint.Description ().GetData ());
          primsToLight.Add (hit.primitive);
        }
      }
      
      RadPrimSet::GlobalIterator it = primsToLight.GetIterator ();;
      while (it.HasNext ())
      {
        RadPrimitive *p = it.Next ();
        ShadeRadPrimitive (rayTracer, *p, radLight);
      }

      radLight->freeEnergy.Clamp (0,0,0);
    }
  }

  float Do5RayVistest (Raytracer &tracer, RadPrimitive &prim, csVector3 elCenter, csVector3 oPoint)
  {
    //Radtest midpoint and midpoint +- 0.5*u/vXformfacctor

    float vis = 1.0f;

    // Ec

     //@@ THIS NEEDS INVESTIGATION. WHY -? .. 
#define RAYTEST(rayOrig, visDecr)\
    {\
      const csVector3 o = (rayOrig)-prim.GetPlane ().GetNormal ()*0.01f;\
      const csVector3 dir = (oPoint-o);\
      Ray ray; HitPoint hit;\
      ray.origin = o; ray.minLength = FLT_EPSILON*10; ray.maxLength = dir.Norm ();\
      ray.direction = dir / ray.maxLength; \
      if (tracer.TraceAnyHit (ray, hit)) vis -= (visDecr);\
    }

    csVector3 halfU = prim.GetuFormVector () * 0.5f;
    csVector3 halfV = prim.GetvFormVector () * 0.5f;

    RAYTEST(elCenter, 0.2f);
    RAYTEST(elCenter + halfU + halfV, 0.2f);
    RAYTEST(elCenter - halfU + halfV, 0.2f);
    RAYTEST(elCenter + halfU - halfV, 0.2f);
    RAYTEST(elCenter - halfU - halfV, 0.2f);

#undef RAYTEST
    return vis;
  }

  void Lighter::ShadeRadPrimitive (Raytracer &tracer, RadPrimitive &prim, Light* light)
  {    
    //@@HACK, assume visible
    prim.PrepareNoPatches ();

    const float primArea = prim.GetArea ();
    const float totalArea = (prim.GetuFormVector () % prim.GetvFormVector ()).Norm ();

    csVector3 elementCenter = prim.GetMinCoord () + prim.GetuFormVector () * 0.5f + prim.GetvFormVector () * 0.5f;

    int minU, minV, maxU, maxV;
    prim.ComputeMinMaxUV (minU, maxU, minV, maxV);
    
    uint findex = 0;

    for (int v = minV; v <= maxV; v++)
    {
      csVector3 ec = elementCenter;
      for (int u = minU; u <= maxU; u++, findex++, ec += prim.GetuFormVector ())
      {
        const float elemArea = prim.GetElementAreas ()[findex];
        if (elemArea <= 0.0f) continue; //need an area

        csVector3 jiVec = light->position - ec;

        float distSq = jiVec.SquaredNorm ();
        jiVec.Normalize ();

        float cosTheta_j = - (prim.GetPlane ().GetNormal () * jiVec);
        
        if (cosTheta_j <= 0.0f) continue; //backface culling

        // Do a 5 ray visibility test
        float visFact = Do5RayVistest (tracer, prim, ec, light->position);

        //
        float Hij = elemArea / totalArea;
        float dAj = totalArea;
        float Fij = cosTheta_j / (distSq) * Hij * dAj;

        // energy
        csColor energy = light->freeEnergy * Fij * visFact;
        csColor reflected = energy;
        //blah
        //store stats

        Lightmap * lm = prim.GetRadObject ()->GetLightmaps ()[prim.GetLightmapID ()];
        lm->GetData ()[v*lm->GetWidth ()+u] += reflected;

        uint patchIndex = (v-minV)/globalSettings.vPatchResolution * prim.GetuPatches ()+(u-minU)/globalSettings.uPatchResolution;
        RadPatch &patch = prim.GetPatches ()[patchIndex];
        patch.energy += reflected;

      }
      elementCenter += prim.GetvFormVector ();
    }
  }
}

int main (int argc, char* argv[])
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return 1;

  // Load up the global object
  lighter::globalLighter = new lighter::Lighter (object_reg);

  // Initialize it
  if (!lighter::globalLighter->Initialize ()) return 1;

  // Light em up!
  if (!lighter::globalLighter->LightEmUp ()) return 1;

  // Remove it
  delete lighter::globalLighter;
  csInitializer::DestroyApplication (object_reg);

  return 0;
}
