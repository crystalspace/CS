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

#include "common.h"

#include "config.h"
#include "lighter.h"
#include "lightmap.h"
#include "lightmapuv.h"
#include "lightmapuv_simple.h"
#include "primitive.h"
#include "raygenerator.h"
#include "raytracer.h"
#include "scene.h"
#include "statistics.h"
#include "tui.h"
#include "lightcalculator.h"
#include "directlight.h"
#include "globalillumination.h"
#include "sampler.h"
#include <csutil/floatrand.h>

CS_IMPLEMENT_APPLICATION


namespace lighter
{
  Lighter* globalLighter;

  Lighter::Lighter (iObjectRegistry *objectRegistry)
    : objectRegistry (objectRegistry), swapManager (0), scene (new Scene),
      progStartup ("Starting up", 5),
      progLoadFiles ("Loading files", 2),
      progLightmapLayout ("Lightmap layout", 5),
      progSaveFactories ("Saving mesh factories", 7),
      progInitializeMain ("Initialize objects", 10),
        progInitialize (0, 3, &progInitializeMain),
        progInitializeLightmaps ("Lightmaps", 3, &progInitializeMain),
          progPrepareLighting (0, 5, &progInitializeLightmaps),
            progPrepareLightingUVL (0, 95, &progPrepareLighting),
            progPrepareLightingSector (0, 5, &progPrepareLighting),
        progSaveMeshesMain ("Saving mesh objects", 3, &progInitializeMain),
          progSaveMeshes (0, 99, &progSaveMeshesMain),
          progSaveFinish (0, 1, &progSaveMeshesMain),
        progBuildKDTree ("Building KD-Tree", 10, &progInitializeMain),
      progDirectLighting ("Direct lighting", 60),
      progPhotonEmission ("Emitting Photons", 50),
      progFinalGather ("Splatting / Final Gather", 80),
      progPostproc ("Postprocessing lightmaps", 10),
        progPostprocSector (0, 50, &progPostproc),
        progPostprocLM (0, 50, &progPostproc),
      progSaveMeshesPostLight ("Updating meshes", 1),
      progSpecMaps ("Generating specular direction maps", 10),
      progSaveResult ("Saving result", 2),
      progCleanLightingData ("Cleanup", 1),
      progApplyWorldChanges ("Updating world files", 1),
      progCleanup ("Cleanup", 1),
      progFinished ("Finished!", 0)
  {
  }

  Lighter::~Lighter ()
  {
    Statistics::Progress progDummy (0, 0);
    CleanUp (progDummy);
  }

  int Lighter::Run ()
  {
    // Initialize it
    if (!Initialize ()) return 1;

    // Come on baby light my fire
    if (!LightEmUp ()) return 1;

    // We couldn't get much higher
    return 0;
  }

  bool Lighter::Initialize ()
  {
    cmdLine = csQueryRegistry<iCommandLineParser> (objectRegistry);
    if (!cmdLine) return Report ("Cannot get a commandline helper");

    // Check for commandline help.
    if (csCommandLineHelper::CheckHelp (objectRegistry, cmdLine))
    {
      CommandLineHelp (cmdLine->GetOption ("expert", 0) != 0);
      return false;
    }

    // Load config
    const char* configFile = cmdLine->GetOption ("config");

    if (!csInitializer::SetupConfigManager (objectRegistry,
      configFile ? configFile : "/config/lighter2.cfg", "crystalspace.lighter2"))
      return Report ("Cannot setup config manager!");

    configMgr = csQueryRegistry<iConfigManager> (objectRegistry);
    if (!configMgr) return Report ("Cannot get a configuration manager");

    LoadConfiguration ();
    globalConfig.Initialize ();
    globalTUI.Initialize (objectRegistry);
    {
      // Attempt to detect physical memory installed.
      size_t maxSwapSize = CS::Platform::GetPhysicalMemorySize();
      if(maxSwapSize)
      {
        // Convert physical memory to megabytes, and use 3/4 of memory as the limit.
        maxSwapSize /= size_t (1024/0.75f);
      }
      else
      {
          maxSwapSize = 200;
      }
      // Check for override.
      int maxSwapConfig = configMgr->GetInt ("lighter2.swapcachesize", (int)maxSwapSize);
      if ((maxSwapConfig >= 0) && (maxSwapConfig <= SIZE_MAX/(1024*1024)))
        maxSwapSize = size_t (maxSwapConfig)*1024*1024;
      swapManager = new SwapManager (maxSwapSize);
    }

    rayDebug.SetFilterExpression (globalConfig.GetDebugProperties().rayDebugRE);

    // Setup the job manager
    if (globalConfig.GetLighterProperties ().numThreads <= 1)
    {
      jobManager.AttachNew (new CS::Utility::SimpleJobQueue);
    }
    else
    {
      jobManager.AttachNew (new CS::Threading::ThreadedJobQueue (
        globalConfig.GetLighterProperties ().numThreads));
    }

    // Initialize the TUI
    globalTUI.Redraw ();
    progStartup.SetProgress (0);

    // Setup reporter
    {
      pluginManager = csQueryRegistry<iPluginManager> (objectRegistry);
      if (!pluginManager) return Report ("No iPluginManager!");

      csRef<iReporter> rep = csQueryRegistryOrLoad<iReporter> (objectRegistry, 
        "crystalspace.utilities.reporter");
      if (!rep) return false;

      // Set ourselves up as a reporterlistener
      rep->AddReporterListener (&globalTUI);
      
      csRef<iStandardReporterListener> stdrep = 
        csQueryRegistryOrLoad<iStandardReporterListener> (objectRegistry, 
        "crystalspace.utilities.stdrep");
      // Set up standard listener to also report to lighter2.log
      stdrep->SetDebugFile ("/this/lighter2.log");
      stdrep->SetMessageDestination (CS_REPORTER_SEVERITY_BUG, false, false, 
        false, false, true, false);
      stdrep->SetMessageDestination (CS_REPORTER_SEVERITY_ERROR, false, false, 
        false, false, true, false);
      stdrep->SetMessageDestination (CS_REPORTER_SEVERITY_WARNING, false, false, 
        false, false, true, false);
      stdrep->SetMessageDestination (CS_REPORTER_SEVERITY_NOTIFY, false, false, 
        false, false, true, false);
      stdrep->SetMessageDestination (CS_REPORTER_SEVERITY_DEBUG, false, false, 
        false, false, true, false);
    }
    
    // Get plugins
    if (!csInitializer::RequestPlugins (objectRegistry,
            CS_REQUEST_ENGINE,
            CS_REQUEST_IMAGELOADER,
            CS_REQUEST_LEVELLOADER,
            CS_REQUEST_NULL3D,            
            CS_REQUEST_VFS,
            CS_REQUEST_END))
      return Report ("Cannot load plugins!");

    progStartup.SetProgress (0.6f);

    // Get the plugins wants to use
    reporter = csQueryRegistry<iReporter> (objectRegistry);
    if (!reporter) return Report ("Cannot get a reporter");

    engine = csQueryRegistry<iEngine> (objectRegistry);
    if (!engine) return Report ("No iEngine!");
    engine->SetSaveableFlag (true);
    engine->SetDefaultKeepImage (true);

    imageIO = csQueryRegistry<iImageIO> (objectRegistry);
    if (!imageIO) return Report ("No iImageIO!");

    loader = csQueryRegistry<iLoader> (objectRegistry);
    if (!loader) return Report ("No iLoader!");

    vfs = csQueryRegistry<iVFS> (objectRegistry);
    if (!vfs) return Report ("No iVFS!");

    syntaxService = csQueryRegistry<iSyntaxService> (objectRegistry);
    if (!syntaxService) return Report ("No iSyntaxService!");

    svStrings = csQueryRegistryTagInterface<iShaderVarStringSet> (
      objectRegistry, "crystalspace.shader.variablenameset");
    if (!svStrings) return Report ("No SV names string set!");

    // Open the systems
    if (!csInitializer::OpenApplication (objectRegistry))
      return Report ("Error opening system!");

    docSystem = csQueryRegistry<iDocumentSystem> (objectRegistry);
    if (!docSystem) 
      docSystem.AttachNew (new csTinyDocumentSystem);

    progStartup.SetProgress (1);
    return true;
  }

  void Lighter::CleanUp (Statistics::Progress& progress)
  {
    static const int cleanupSteps = 3;
    float progressStep = 1.0f/cleanupSteps;
  
    progress.SetProgress (0);
  
    Statistics::Progress* progCleanupScene =
      progress.CreateProgress (progressStep*0.9f);
    if (scene) scene->CleanUp (*progCleanupScene);
    delete progCleanupScene;
    delete scene; scene = 0;
    progress.SetProgress (1*progressStep);
    
    delete swapManager; swapManager = 0;
    progress.SetProgress (2*progressStep);
    
    engine.Invalidate ();
    progress.SetProgress (3*progressStep);
  
    progress.SetProgress (1);
}

  bool Lighter::LightEmUp ()
  {
    // Have to load to have anything to light
    if (!LoadFiles (progLoadFiles)) 
      return false;

    // Calculate lightmapping coordinates
    CalculateLightmaps ();
   
    if (!scene->SaveWorldFactories (progSaveFactories)) 
      return false;
    scene->GetFactories ().DeleteAll();

    // Initialize all objects
    InitializeObjects ();    
    
    // Prepare lighting of all objects
    PrepareLighting ();

    // Save all data to the meshes
    if (!scene->SaveWorldMeshes (progSaveMeshes)) 
      return false;
    if (!scene->FinishWorldSaving (progSaveFinish)) 
      return false;

    /* TODO: the global lightmaps' subrect allocators are not needed any
	     more, discard contents. */

    // Build the KD-trees
    BuildKDTrees ();

    // Build Photon Maps
    BuildPhotonMap();
   
    // Fill the photon maps
    ComputeLighting();

    // Shoot direct lighting
//    DoDirectLighting ();   

    //@@ DO OTHER LIGHTING
//    DoIndirectIllumination();

    // Postprocessing of ligthmaps
    PostprocessLightmaps ();

    //Save the result
    if (!scene->SaveMeshesPostLighting (progSaveMeshesPostLight)) 
      return false;
    if (!scene->GenerateSpecularDirectionMaps (progSpecMaps))
      return false;
    if (!scene->SaveLightmaps (progSaveResult)) 
      return false;
    scene->CleanLightingData (progCleanLightingData);
    if (!scene->ApplyWorldChanges (progApplyWorldChanges)) 
      return false;

    CleanUp (progCleanup);

    progFinished.SetProgress (1);

    globalTUI.FinishDraw ();

    return true;  
  }

  bool Lighter::Report (const char* msg, ...)
  {
    va_list arg;
    va_start (arg, msg);
    csReportV(objectRegistry, CS_REPORTER_SEVERITY_ERROR, 
      "crystalspace.application.lighter2", msg, arg);
    va_end (arg);
    return false;
  }

  bool Lighter::LoadFiles (Statistics::Progress& progress)
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
    return scene->LoadFiles (progress);
  }

  void Lighter::CalculateLightmaps ()
  {
    size_t updateFreq, u;
    float progressStep;

    progLightmapLayout.SetProgress (0);
    // Calculate lightmapping coordinates  
    uvLayout.AttachNew (new SimpleUVFactoryLayouter (scene->GetLightmaps()));

    u = updateFreq = progLightmapLayout.GetUpdateFrequency (
      scene->GetFactories ().GetSize ());
    progressStep = updateFreq * (1.0f / scene->GetFactories ().GetSize ());
    ObjectFactoryHash::GlobalIterator factIt = 
      scene->GetFactories ().GetIterator ();
    while (factIt.HasNext ())
    {
      csRef<ObjectFactory> fact = factIt.Next ();
      fact->PrepareLightmapUV (uvLayout);
      if (--u == 0)
      {
        progLightmapLayout.IncProgress (progressStep);
        u = updateFreq;
      }
    }
    progLightmapLayout.SetProgress (1);
  }

  void Lighter::BuildPhotonMap()
  {
    // Indicate 0% progress
    progPhotonEmission.SetProgress(0);

    // Check configuration to see if indirect illumination is enabled.
    if (!globalConfig.GetLighterProperties().indirectLMs)
    {
      return;
    }

    // Compute step size for progress updates
    const float progressStep = 1.0f / scene->GetSectors ().GetSize();

    // Create global illumination object
    lighter::GlobalIllumination 
      lighting(globalConfig.GetIndirectProperties());

    // Retrieve sector iterator
    SectorHash::GlobalIterator sectIt = 
      scene->GetSectors ().GetIterator ();

    // Iterator over all sectors
    size_t sectorIdx = 0;
    while (sectIt.HasNext ())
    {
      // Retrieve next sector
      csRef<Sector> sect = sectIt.Next ();
      
      // Setup to report progress
      Statistics::Progress* progPhoton =
        progPhotonEmission.CreateProgress(progressStep);

      // Emit photons in this sector
      lighting.EmitPhotons(sect, *progPhoton);

      // Cleanup
      delete progPhoton;

      sectorIdx++;
    }

    // Indicate 100% progress
    progPhotonEmission.SetProgress(1);
  }

  void Lighter::InitializeObjects ()
  {
    progInitialize.SetProgress (0);
    const float progressStep = 1.0f / scene->GetSectors ().GetSize();
    SectorHash::GlobalIterator sectIt = 
      scene->GetSectors ().GetIterator ();
    while (sectIt.HasNext ())
    {
      csRef<Sector> sect = sectIt.Next ();

      Statistics::Progress* progSector = 
        progInitialize.CreateProgress (progressStep);
      sect->Initialize (*progSector);
      delete progSector;
    }
    progInitialize.SetProgress (1);
  }

  void Lighter::PrepareLighting ()
  {
    uvLayout->PrepareLighting (progPrepareLightingUVL);
    uvLayout.Invalidate();

    progPrepareLightingSector.SetProgress (0);
    const float progressStep = 1.0f / scene->GetSectors ().GetSize();
    SectorHash::GlobalIterator sectIt = 
      scene->GetSectors ().GetIterator ();
    while (sectIt.HasNext ())
    {
      csRef<Sector> sect = sectIt.Next ();

      Statistics::Progress* progSector = 
        progPrepareLightingSector.CreateProgress (progressStep);
      sect->PrepareLighting (*progSector);
      delete progSector;
    }
    
    progPrepareLightingSector.SetProgress (1);
  }

  void Lighter::BuildKDTrees ()
  {
    progBuildKDTree.SetProgress (0);
    const float progressStep = 1.0f / scene->GetSectors ().GetSize();
    SectorHash::GlobalIterator sectIt = 
      scene->GetSectors ().GetIterator ();
    while (sectIt.HasNext ())
    {
      csRef<Sector> sect = sectIt.Next ();

      Statistics::Progress* progSector = 
        progBuildKDTree.CreateProgress (progressStep);
      sect->BuildKDTree (*progSector);
      delete progSector;
    }
    progBuildKDTree.SetProgress (1);
  }

  void Lighter::DoDirectLighting ()
  {
    //progDirectLighting.SetProgress (0);
    //if (globalConfig.GetLighterProperties ().doDirectLight)
    //{
    //  int numDLPasses = 
    //    globalConfig.GetLighterProperties().directionalLMs ? 4 : 1;
    //  const csVector3 bases[4] =
    //  {
    //    csVector3 (0, 0, 1),
    //    csVector3 (/* -1/sqrt(6) */ -0.408248f, /* 1/sqrt(2) */ 0.707107f, /* 1/sqrt(3) */ 0.577350f),
    //    csVector3 (/* sqrt(2/3) */ 0.816497f, 0, /* 1/sqrt(3) */ 0.577350f),
    //    csVector3 (/* -1/sqrt(6) */ -0.408248f, /* -1/sqrt(2) */ -0.707107f, /* 1/sqrt(3) */ 0.577350f)
    //  };
    //  float sectorProgress = 
    //    1.0f / (numDLPasses * scene->GetSectors ().GetSize());
    //  for (int p = 0; p < numDLPasses; p++)
    //  {
    //    DirectLighting lighting (bases[p], p);

    //    SectorHash::GlobalIterator sectIt = 
    //      scene->GetSectors ().GetIterator ();
    //    while (sectIt.HasNext ())
    //    {
    //      csRef<Sector> sect = sectIt.Next ();
    //      Statistics::Progress* lightProg = 
    //        progDirectLighting.CreateProgress (sectorProgress);
    //      lighting.ShadeDirectLighting (sect, *lightProg);
    //      delete lightProg;
    //    }
    //  }
    //  progDirectLighting.SetProgress (1);
    //}
  }

  void Lighter::ComputeLighting ()
  {
    // Set task progress to 0%
    progDirectLighting.SetProgress (0);

    int numPasses = 
      globalConfig.GetLighterProperties().directionalLMs ? 4 : 1;

    const csVector3 bases[4] =
    {
      csVector3 (0, 0, 1),
      csVector3 (/* -1/sqrt(6) */ -0.408248f, /* 1/sqrt(2) */ 0.707107f, /* 1/sqrt(3) */ 0.577350f),
      csVector3 (/* sqrt(2/3) */ 0.816497f, 0, /* 1/sqrt(3) */ 0.577350f),
      csVector3 (/* -1/sqrt(6) */ -0.408248f, /* -1/sqrt(2) */ -0.707107f, /* 1/sqrt(3) */ 0.577350f)
    };

    // What portion of main task does each sub-task complete
    float sectorProgress = 
      1.0f / (numPasses * scene->GetSectors ().GetSize());

    // Loop through lighting calculation for directional dependencies
    for (int p = 0; p < numPasses; p++)
    {
      // Construct a light calculator
      LightCalculator lighting (bases[p], p);

      // Add components to the light calculator
      DirectLighting directComponent (bases[p], p);
      lighting.addComponent(&directComponent, 1.0);

//      IndirectLighting indirectComponent (bases[p], p);
//      lighting.addComponent(&indirectComponent);

      // Iterate overl all scene sectors
      SectorHash::GlobalIterator sectIt = 
        scene->GetSectors ().GetIterator ();
      while (sectIt.HasNext ())
      {
        // Get the next sector
        csRef<Sector> sect = sectIt.Next ();

        // Create a sub-task progress
        Statistics::Progress* lightProg = 
          progDirectLighting.CreateProgress (sectorProgress);

        // Compute the lighting
        lighting.ComputeSectorStaticLighting (sect, *lightProg);

        // Clean up
        delete lightProg;
      }
    }

    // Set task progress to 100%
    progDirectLighting.SetProgress (1);
  }

  void Lighter::DoIndirectIllumination()
  {
    // Indicate 0% progress
    progFinalGather.SetProgress(0);

    // Check configuration to see if indirect illumination is enabled.
    if (!globalConfig.GetLighterProperties().indirectLMs)
    {
      return;
    }

    // Retrieve sector iterator
    SectorHash::GlobalIterator sectIt = 
      scene->GetSectors().GetIterator();

    // Compute step size for progress updates
    const float progressStep = 1.0f / scene->GetSectors ().GetSize();

    // Iterate over all sectors
    while (sectIt.HasNext())
    {
      // Move to next sector
      csRef<Sector> sect = sectIt.Next();

      // Setup to report progress
      Statistics::Progress* lightProg =
        progFinalGather.CreateProgress(progressStep);

      // Get object iterator for current sector
      ObjectHash::GlobalIterator gitr = sect->allObjects.GetIterator();

      // Create global illumination object
      lighter::GlobalIllumination lighting(globalConfig.GetIndirectProperties());

      // Shade light maps with indirect light from photon maps
      lighting.ShadeIndirectLighting(sect, *lightProg);

      // Clean up
      delete lightProg;
    }

    // Set progress to 100%
    progFinalGather.SetProgress(1);
  }

  void Lighter::PostprocessLightmaps ()
  {
    size_t realNumLMs = scene->GetLightmaps ().GetSize ();
    if (globalConfig.GetLighterProperties().directionalLMs)
      realNumLMs /= 4;
    progPostproc.SetProgress (0);
    // De-antialias the lightmaps
    LightmapMaskPtrDelArray lmMasks;
    for (size_t l = 0; l < realNumLMs; l++)
    {
      const Lightmap* lm = scene->GetLightmaps ()[l];
      lmMasks.Push (new LightmapMask (*lm));
    }

    progPostprocSector.SetProgress (0);
    const float sectorProgress = 1.0f / scene->GetSectors ().GetSize();
    SectorHash::GlobalIterator sectIt = 
      scene->GetSectors ().GetIterator ();

    while (sectIt.HasNext ())
    {
      csRef<Sector> sect = sectIt.Next ();
      Statistics::Progress* progSector = 
        progPostprocSector.CreateProgress (sectorProgress);

      size_t u, updateFreq;
      u = updateFreq = 
        progSector->GetUpdateFrequency (sect->allObjects.GetSize());
      const float progressStep = updateFreq * (1.0f / sect->allObjects.GetSize());
      ObjectHash::GlobalIterator objIt = sect->allObjects.GetIterator ();
      while (objIt.HasNext ())
      {
        csRef<Object> obj = objIt.Next ();
        obj->FillLightmapMask (lmMasks);
        if (--u == 0)
        {
          progSector->IncProgress (progressStep);
          u = updateFreq;
        }
      }
      progSector->SetProgress (1);
      delete progSector;
    }
    progPostprocSector.SetProgress (1);

    progPostprocLM.SetProgress (0);
    csArray<LightmapPtrDelArray*> allLightmaps (scene->GetAllLightmaps());
    float lightmapStep = 1.0f / allLightmaps.GetSize();
    for (size_t li = 0; li < allLightmaps.GetSize (); ++li)
    {
      LightmapPtrDelArray& lightmaps = *allLightmaps[li];

      Statistics::Progress* progLM = 
        progPostprocLM.CreateProgress (lightmapStep);
      size_t u, updateFreq;
      u = updateFreq = progLM->GetUpdateFrequency (lightmaps.GetSize());
      const float progressStep = updateFreq * (1.0f / lightmaps.GetSize());

      for (size_t lmI = 0; lmI < lightmaps.GetSize (); ++lmI)
      {
        // Might have empty lightmap entries for non-created lightmaps
        if (!lightmaps[lmI])
          continue;

        lightmaps[lmI]->FixupLightmap (*(lmMasks[lmI % realNumLMs]));
        if (--u == 0)
        {
          progLM->IncProgress (progressStep);
          u = updateFreq;
        }
      }
      progLM->SetProgress (1);
      delete progLM;
    }
    progPostprocLM.SetProgress (1);    
  }

  void Lighter::LoadConfiguration ()
  {
    //Merge configuration settings from command line into config
    csRef<csConfigFile> cmdLineConfig;
    cmdLineConfig.AttachNew (new csConfigFile);

    const char* option;
    const char* optionValue;
    size_t index = 0;
    csString buffer;
    while ((option = cmdLine->GetOptionName (index)) != 0)
    {
      optionValue = cmdLine->GetOption (index);

      if (!optionValue)
      {
        // No option value, a boolean setting, either true or false
        if (option[0] == 'n' && option[1] == 'o')
        {
          option += 2;
          optionValue = "false";
        }
        else
        {
          optionValue = "true";
        }
      }

      buffer << "lighter2." << option << " = " << optionValue << "\n";

      index++;
    }

    cmdLineConfig->LoadFromBuffer (buffer.GetDataSafe (), true);
    configMgr->AddDomain (cmdLineConfig, iConfigManager::ConfigPriorityUserApp);
  }

  void Lighter::CommandLineHelp (bool expert) const
  {
    csPrintf ("Syntax:\n");
    csPrintf ("  lighter2 [options] <file> [file] [file] ...\n\n");
    csPrintf ("Options:\n");
    
    csPrintf (" --[no]simpletui\n");
    csPrintf ("  Use simplified text ui for output. Recommended/needed\n"
              "  for platforms without ANSI console handling such as msys.\n\n");

    csPrintf (" --swapcachesize=<megabyte>\n");
    csPrintf ("  Set the size of the in memory swappable data cache in number "
      "of megabytes\n");
    csPrintf ("   Default: 200\n\n");

    csPrintf (" --[no]directlight\n");
    csPrintf ("  Calculate direct lighting using per lumel/vertex sampling\n");
    csPrintf ("   Default: True\n\n");

    csPrintf (" --indirectlight\n");
    csPrintf ("  Calculates indirect lighting using photon mapping\n");
    csPrintf ("    Default: %s\n\n", (globalConfig.GetLighterProperties ().indirectLMs?"True":"False"));

    csPrintf (" --[no]directlightrandom\n");
    csPrintf ("  Use random sampling for direct lighting instead of sampling\n"
              "  every light source.\n");
    csPrintf ("   Default: False\n\n");

    csPrintf (" --lmdensity=<number>\n");
    csPrintf ("  Set scaling between world space units and lightmap pixels\n");
    csPrintf ("   Default: %f\n\n", globalConfig.GetLMProperties ().lmDensity);

    csPrintf (" --maxlightmapu=<number>\n");
    csPrintf ("  Set maximum lightmap size in u-mapping direction\n");
    csPrintf ("   Default: %d\n\n", globalConfig.GetLMProperties ().maxLightmapU);

    csPrintf (" --maxlightmapv=<number>\n");
    csPrintf ("  Set maximum lightmap size in v-mapping direction\n");
    csPrintf ("   Default: %d\n\n", globalConfig.GetLMProperties ().maxLightmapV);

    csPrintf (" --blackthreshold=<threshold>\n");
    csPrintf ("  Set the normalized threshold for lightmap pixels to be "
                "considered black.\n");
    csPrintf ("   Default: %f\n\n", globalConfig.GetLMProperties ().blackThreshold);

    csPrintf (" --normalstolerance=<angle>\n");
    csPrintf ("  Set the angle between two normals to be considered equal by "
                "the\n");
    csPrintf ("  lightmap layouter.\n");
    csPrintf ("   Default: 1\n\n");

    csPrintf (" --maxterrainlightmapu=<number>\n");
    csPrintf ("  Set maximum terrain lightmap size in u-mapping direction\n");
    csPrintf ("   Default: value for non-terrain lightmaps\n\n");

    csPrintf (" --maxterrainlightmapv=<number>\n");
    csPrintf ("  Set maximum terrain lightmap size in v-mapping direction\n");
    csPrintf ("   Default: value for non-terrain lightmaps\n\n");

    csPrintf (" --bumplms\n");
    csPrintf ("  Generate directional lightmaps needed for normalmapping static\n");
    csPrintf ("  lit surfaces\n");
    csPrintf ("   Default: False\n\n");
    
    csPrintf (" --nospecmaps\n");
    csPrintf ("  Don't generate maps for specular lighting on static lit surfaces\n\n");
    
    csPrintf (" --expert\n");
    csPrintf ("  Display advanced command line options\n\n");

    if (expert)
    {
      /*
      csPrintf (" --numthreads=<N>\n");
      csPrintf ("  Number of threads to use\n");
      csPrintf ("   Default: number of processors in the system\n");
      */
      csPrintf (" --debugOcclusionRays=<regexp>\n");
      csPrintf ("  Write a visualization of rays and their occlusions to\n"
                "  meshes matching <regexp>\n\n");

      csPrintf (" --[no]binary\n");
      csPrintf ("  Whether to save buffers in binary format. Default: True\n\n");

      csPrintf (" --savePhotonMap\n");
      csPrintf ("  Save the contents of the photon map as a flat array in a\n"
                "  binary file for external use.\n\n");
    }

    csPrintf (" --numphotons=<number>\n");
    csPrintf ("  Sets the number of photons to emit in each sector (you should change this).\n");
    csPrintf ("   Default: %d\n\n", globalConfig.GetIndirectProperties ().numPhotons);

    csPrintf (" --photonspersample=<number>\n");
    csPrintf ("  Sets the number of photons to sample for density calculations (you should change this).\n");
    csPrintf ("   Default: %d\n\n", globalConfig.GetIndirectProperties ().numPerSample);
    
    csPrintf (" --maxrecursiondepth=<number>\n");
    csPrintf ("  Sets the maximum number of times a photon can scatter during\n"
              "  indirect illumination (set to -1 to use a purely random cutoff).\n");
    csPrintf ("   Default: %d\n\n", globalConfig.GetIndirectProperties ().maxRecursionDepth);

    csPrintf (" --sampledistance=<threshold>\n");
    csPrintf ("  Sets the max distance to search for photons when sampling\n");
    csPrintf ("   Default: %f\n\n", globalConfig.GetIndirectProperties ().sampleDistance);

    csPrintf (" --finalGather\n");
    csPrintf ("  Turns on final gather for indirect lighting calculations\n");
    csPrintf ("   Default: %s\n\n", (globalConfig.GetIndirectProperties ().finalGather?"True":"False"));

    csPrintf (" --numfgrays=<number>\n");
    csPrintf ("  Sets the number of Final Gather rays to average from\n");
    csPrintf ("   Default: %d\n", globalConfig.GetIndirectProperties ().numFinalGatherRays);

    csPrintf ("\n");
  }

}

int main (int argc, char* argv[])
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return 1;

  // Load up the global object
  csRef<lighter::Lighter> localLighter;
  localLighter.AttachNew (new lighter::Lighter (object_reg));
  lighter::globalLighter = localLighter;

  int ret = lighter::globalLighter->Run ();

  localLighter = 0;
  lighter::globalLighter = 0;

  // Remove it
  csInitializer::DestroyApplication (object_reg);

  return ret;
}
