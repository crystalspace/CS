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
#include "directlight.h"
#include "sampler.h"

CS_IMPLEMENT_APPLICATION


namespace lighter
{
  Lighter* globalLighter;

  Lighter::Lighter (iObjectRegistry *objectRegistry)
    : objectRegistry (objectRegistry), scene (new Scene),
      progStartup ("Starting up", 5),
      progLoadFiles ("Loading files", 2),
      progLightmapLayout ("Lightmap layout", 5),
      progSaveFactories ("Saving mesh factories", 7),
      progInitializeMain ("Initialize objects", 10),
        progInitialize (0, 3, &progInitializeMain),
        progInitializeLightmaps ("Lightmaps", 3, &progInitializeMain),
          progInitializeLM (0, 3, &progInitializeLightmaps),
          progPrepareLighting (0, 5, &progInitializeLightmaps),
            progPrepareLightingUVL (0, 95, &progPrepareLighting),
            progPrepareLightingSector (0, 5, &progPrepareLighting),
        progSaveMeshesMain ("Saving mesh objects", 3, &progInitializeMain),
          progSaveMeshes (0, 99, &progSaveMeshesMain),
          progSaveFinish (0, 1, &progSaveMeshesMain),
        progBuildKDTree ("Building KD-Tree", 10, &progInitializeMain),
      progDirectLighting ("Direct lighting", 60),
      progPostproc ("Postprocessing lightmaps", 10),
        progPostprocSector (0, 50, &progPostproc),
        progPostprocLM (0, 50, &progPostproc),
      progSaveResult ("Saving result", 2),
      progSaveMeshesPostLight ("Updating meshes", 1),
      progApplyWorldChanges ("Updating world files", 1),
      progCleanup ("Cleanup", 1),
      progFinished ("Finished!", 0)
  {
  }

  Lighter::~Lighter ()
  {
    CleanUp ();
  }

  int Lighter::Run ()
  {
    // Initialize it
    if (!Initialize ()) return 1;

    // Common baby light my fire
    if (!LightEmUp ()) return 1;

    return 0;
  }

  bool Lighter::Initialize ()
  {
    cmdLine = csQueryRegistry<iCommandLineParser> (objectRegistry);
    if (!cmdLine) return Report ("Cannot get a commandline helper");

    // Check for commandline help.
    if (csCommandLineHelper::CheckHelp (objectRegistry, cmdLine))
    {
      CommandLineHelp ();
      return true;
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
    globalTUI.Initialize ();
    {
      int maxSwapSize = configMgr->GetInt ("lighter2.swapcachesize", 
        200)*1024*1024;
      swapManager = new SwapManager (maxSwapSize);
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

    imageIO = csQueryRegistry<iImageIO> (objectRegistry);
    if (!imageIO) return Report ("No iImageIO!");

    loader = csQueryRegistry<iLoader> (objectRegistry);
    if (!loader) return Report ("No iLoader!");

    vfs = csQueryRegistry<iVFS> (objectRegistry);
    if (!vfs) return Report ("No iVFS!");

    strings = csQueryRegistryTagInterface<iStringSet> (
      objectRegistry, "crystalspace.shared.stringset");
    if (!strings) return Report ("No shared string set!");

    // Open the systems
    if (!csInitializer::OpenApplication (objectRegistry))
      return Report ("Error opening system!");

    // For now, force the use of TinyXML to be able to write
    docSystem.AttachNew (new csTinyDocumentSystem);

    progStartup.SetProgress (100);
    return true;
  }

  void Lighter::CleanUp ()
  {
    delete scene; scene = 0;
    delete swapManager; swapManager = 0;
  }

  bool Lighter::LightEmUp ()
  {
    // Have to load to have anything to light
    if (!LoadFiles (progLoadFiles)) 
      return false;

    size_t updateFreq, u;
    float progressStep;

    progLightmapLayout.SetProgress (0);
    // Calculate lightmapping coordinates
    csRef<LightmapUVFactoryLayouter> uvLayout;
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

    if (!scene->SaveWorldFactories (progSaveFactories)) return false;
    scene->GetFactories ().DeleteAll();

    // Initialize all objects
    progInitialize.SetProgress (0);
    progressStep = 1.0f / scene->GetSectors ().GetSize();
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

    progInitializeLM.SetProgress (0);
    u = updateFreq = progInitializeLM.GetUpdateFrequency (
      scene->GetLightmaps().GetSize());
    progressStep = updateFreq * (1.0f / scene->GetLightmaps().GetSize());
    for (size_t i = 0; i < scene->GetLightmaps().GetSize(); i++)
    {
      Lightmap * lm = scene->GetLightmaps ()[i];
      lm->Initialize();
      if (--u == 0)
      {
        progInitializeLM.IncProgress (progressStep);
        u = updateFreq;
      }
    }
    progInitializeLM.SetProgress (1);

    uvLayout->PrepareLighting (progPrepareLightingUVL);
    uvLayout.Invalidate();
    
    progPrepareLightingSector.SetProgress (0);
    progressStep = 1.0f / scene->GetSectors ().GetSize();
    sectIt.Reset();
    while (sectIt.HasNext ())
    {
      csRef<Sector> sect = sectIt.Next ();

      Statistics::Progress* progSector = 
        progPrepareLightingSector.CreateProgress (progressStep);
      sect->PrepareLighting (*progSector);
      delete progSector;
    }
    progPrepareLightingSector.SetProgress (1);

    if (!scene->SaveWorldMeshes (progSaveMeshes)) return false;
    if (!scene->FinishWorldSaving (progSaveFinish)) return false;

    /* TODO: the global lightmaps' subrect allocators are not needed any
	     more, discard contents. */

    progBuildKDTree.SetProgress (0);
    progressStep = 1.0f / scene->GetSectors ().GetSize();
    sectIt.Reset();
    while (sectIt.HasNext ())
    {
      csRef<Sector> sect = sectIt.Next ();

      Statistics::Progress* progSector = 
        progBuildKDTree.CreateProgress (progressStep);
      sect->BuildKDTree (*progSector);
      delete progSector;
    }
    progBuildKDTree.SetProgress (1);
   
    // Shoot direct lighting
    if (globalConfig.GetLighterProperties ().doDirectLight)
    {
      DirectLighting::Initialize ();
      progDirectLighting.SetProgress (0);
      float sectorProgress = 1.0f / scene->GetSectors ().GetSize();
      sectIt.Reset ();
      while (sectIt.HasNext ())
      {
        csRef<Sector> sect = sectIt.Next ();
        Statistics::Progress* lightProg = 
          progDirectLighting.CreateProgress (sectorProgress);
        DirectLighting::ShadeDirectLighting (sect, *lightProg);
        delete lightProg;
      }
      progDirectLighting.SetProgress (1);
    }

    //@@ DO OTHER LIGHTING

    progPostproc.SetProgress (0);
    // De-antialias the lightmaps
    {
      LightmapMaskArray lmMasks;
      LightmapPtrDelArray::Iterator lmIt = scene->GetLightmaps ().GetIterator ();
      while (lmIt.HasNext ())
      {
        const Lightmap* lm = lmIt.Next ();
        lmMasks.Push (LightmapMask (*lm));
      }

      progPostprocSector.SetProgress (0);
      float sectorProgress = 1.0f / scene->GetSectors ().GetSize();
      sectIt.Reset ();
      while (sectIt.HasNext ())
      {
        csRef<Sector> sect = sectIt.Next ();
        Statistics::Progress* progSector = 
          progPostprocSector.CreateProgress (sectorProgress);

        u = updateFreq = 
          progSector->GetUpdateFrequency (sect->allObjects.GetSize());
        progressStep = updateFreq * (1.0f / sect->allObjects.GetSize());
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
        u = updateFreq = progLM->GetUpdateFrequency (lightmaps.GetSize());
        progressStep = updateFreq * (1.0f / lightmaps.GetSize());

        for (size_t lmI = 0; lmI < lightmaps.GetSize (); ++lmI)
        {
          lightmaps[lmI]->FixupLightmap (lmMasks[lmI]);
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

    //Save the result
    if (!scene->SaveLightmaps (progSaveResult)) return false;
    if (!scene->SaveMeshesPostLighting (progSaveMeshesPostLight)) return false;
    if (!scene->ApplyWorldChanges (progApplyWorldChanges)) return false;

    progCleanup.SetProgress (0);
    CleanUp ();
    progCleanup.SetProgress (1);

    progFinished.SetProgress (1);

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

  void Lighter::CommandLineHelp () const
  {
    csPrintf ("Syntax:\n");
    csPrintf ("  lighter2 [options] <file> [file] [file] ...\n\n");
    csPrintf ("Options:\n");
    
    csPrintf (" --[no]simpletui\n");
    csPrintf ("  Use simplified text ui for output. Recommended/needed\n"
              "  for platforms without ANSI console handling such as msys.\n");

    csPrintf (" --swapcachesize=<megabyte>\n");
    csPrintf ("  Set the size of the in memory swappable data cache in number "
      "of megabytes\n");

    csPrintf (" --[no]directlight\n");
    csPrintf ("  Calculate direct lighting using per lumel/vertex sampling\n");

    csPrintf (" --[no]directlightrandom\n");
    csPrintf ("  Use random sampling for direct lighting instead of sampling\n"
              "  every light source.\n");

    csPrintf (" --utexelperunit=<number>\n");
    csPrintf ("  Set scaling between world space units and lightmap pixels\n"
              "  in lightmap u-mapping direction\n");

    csPrintf (" --vtexelperunit=<number>\n");
    csPrintf ("  Set scaling between world space units and lightmap pixels\n"
              "  in lightmap v-mapping direction\n");
   
    csPrintf (" --maxlightmapu=<number>\n");
    csPrintf ("  Set maximum lightmap size in u-mapping direction\n");

    csPrintf (" --maxlightmapv=<number>\n");
    csPrintf ("  Set maximum lightmap size in v-mapping direction\n\n");
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
