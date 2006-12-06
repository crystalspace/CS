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
#include "config.h"
#include "lighter.h"
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
  Statistics globalStats;

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

    // Initialize the TUI
    globalTUI.Redraw ();
    globalStats.SetTaskProgress ("Starting up", 0);

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

    lighter::globalStats.SetTaskProgress ("Starting up", 60);

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

    globalStats.SetTaskProgress ("Starting up", 100);
    globalStats.SetTotalProgress (5);
    return true;
  }

  bool Lighter::LightEmUp ()
  {
    // Have to load to have anything to light
    if (!LoadFiles ()) 
      return false;
    globalStats.SetTotalProgress (8);

    if (!scene->ParseEngine ()) 
      return false;
    globalStats.SetTotalProgress (10);
    

    globalStats.SetTaskProgress ("Lightmap layout", 0);
    // Calculate lightmapping coordinates
    LightmapUVFactoryLayouter *uvLayout = new SimpleUVFactoryLayouter (scene->GetLightmaps());

    float factoryProgress = 100.0f / scene->GetFactories ().GetSize ();
    ObjectFactoryHash::GlobalIterator factIt = 
      scene->GetFactories ().GetIterator ();
    while (factIt.HasNext ())
    {
      csRef<ObjectFactory> fact = factIt.Next ();
      fact->PrepareLightmapUV (uvLayout);
      globalStats.IncTaskProgress (factoryProgress);
    }

    // Initialize all objects
    globalStats.SetTotalProgress (15);
    globalStats.SetTaskProgress ("Initialize objects", 0);
    float sectorProgress = 100.0f / scene->GetSectors ().GetSize();
    SectorHash::GlobalIterator sectIt = 
      scene->GetSectors ().GetIterator ();
    while (sectIt.HasNext ())
    {
      csRef<Sector> sect = sectIt.Next ();
      sect->Initialize ();
      globalStats.IncTaskProgress (sectorProgress);
    }

    for (size_t i = 0; i < scene->GetLightmaps().GetSize(); i++)
    {
      Lightmap * lm = scene->GetLightmaps ()[i];
      lm->Initialize();
    }    
    
    // Progress 20
    globalStats.SetTotalProgress (20);

    // Shoot direct lighting
    if (globalConfig.GetLighterProperties ().doDirectLight)
    {
      DirectLighting::Initialize ();
      globalStats.SetTaskProgress ("Direct lighting", 0);
      sectIt.Reset ();
      while (sectIt.HasNext ())
      {
        csRef<Sector> sect = sectIt.Next ();
        DirectLighting::ShadeDirectLighting (sect, sectorProgress);
      }
    }

    globalStats.SetTotalProgress (80);

    //@@ DO OTHER LIGHTING

    // De-antialias the lightmaps
    sectIt.Reset ();
    globalStats.SetTaskProgress ("Postprocessing lightmaps", 0);
    while (sectIt.HasNext ())
    {
      csRef<Sector> sect = sectIt.Next ();
      float objProgress = sectorProgress / sect->allObjects.GetSize ();
      ObjectHash::GlobalIterator objIt = sect->allObjects.GetIterator ();
      while (objIt.HasNext ())
      {
        csRef<Object> obj = objIt.Next ();
        csArray<LightmapPtrDelArray*> allLightmaps (scene->GetAllLightmaps());
        obj->FixupLightmaps (allLightmaps);

        globalStats.IncTaskProgress (objProgress);
      }
    }

    globalStats.SetTotalProgress (90);
    //Save the result
    if (!scene->SaveFiles ()) return false;
    globalStats.SetTotalProgress (100);

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

  // Initialize it
  if (!lighter::globalLighter->Initialize ()) return 1;

  // Light em up!
  if (!lighter::globalLighter->LightEmUp ()) return 1;

  localLighter = 0;

  // Remove it
  csInitializer::DestroyApplication (object_reg);
  csPrintf (CS_ANSI_CLEAR_SCREEN);

  return 0;
}
