/*
  Copyright (C) 2005-2006 by Marten Svanfeldt

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

#ifndef __LIGHTER_H__
#define __LIGHTER_H__

#include "csutil/refcount.h"

#include "raydebug.h"
#include "statistics.h"


namespace lighter
{
  class ElementAreasAlloc;
  class Light_old;
  class LightmapUVFactoryLayouter;
  class Primitive;
  class Raytracer;
  class Scene;
  class Sector;
  class SwapManager;
  

  class Lighter : public csRefCount
  {
  public:
    Lighter (iObjectRegistry *objectRegistry);
    ~Lighter ();

    int Run ();

    // Initialize and load plugins we want
    bool Initialize ();

    // Do the fancy lighting
    bool LightEmUp ();

    // Report an error/warning, always returns false
    bool Report (const char* msg, ...);

    // Public members
    csRef<iDocumentSystem> docSystem;
    csRef<iEngine> engine;
    csRef<iImageIO> imageIO;
    csRef<iLoader> loader;
    csRef<iPluginManager> pluginManager;
    csRef<iReporter> reporter;
    csRef<iVFS> vfs;
    csRef<iCommandLineParser> cmdLine;
    csRef<iConfigManager> configMgr;
    iObjectRegistry *objectRegistry;
    csRef<iShaderVarStringSet> svStrings;
    csRef<iJobQueue> jobManager;
    csRef<iSyntaxService> syntaxService;

    SwapManager* swapManager;
    RayDebugHelper rayDebug;

  protected:
    // Cleanup and prepare for shutdown
    void CleanUp (Statistics::Progress& progress);

    // Parse the commandline and load any files specified
    bool LoadFiles (Statistics::Progress& progress);

    // Calculate lightmapping
    void CalculateLightmaps ();

    // Build the photon map
    void BuildPhotonMaps();
    void BalancePhotonMaps();

    // Adjust light attenuation
    void ForceRealisticAttenuation();

    // Initialize objects after LM construction
    void InitializeObjects ();

    // Prepare for lighting
    void PrepareLighting ();

    // Build per-sector KD-tree
    void BuildKDTrees ();

    // Compute all lighting components (Fill the lightmaps)
    void ComputeLighting (bool enableRaytracer, bool enablePhotonMapper);

    // Post-process all lightmaps
    void PostprocessLightmaps ();

    // Load configuration from config file & command line
    void LoadConfiguration ();

    // Print command line help
    void CommandLineHelp (bool expert, bool raytraceopts, bool pmopts) const;

    Scene *scene;

    csRef<LightmapUVFactoryLayouter> uvLayout;

    Statistics::Progress progStartup;
    Statistics::Progress progLoadFiles;
    Statistics::Progress progLightmapLayout;
    Statistics::Progress progSaveFactories;
    Statistics::Progress progInitializeMain;
    Statistics::Progress progInitialize;
    Statistics::Progress progInitializeLightmaps;
    Statistics::Progress progPrepareLighting;
    Statistics::Progress progPrepareLightingUVL;
    Statistics::Progress progPrepareLightingSector;
    Statistics::Progress progSaveMeshesMain;
    Statistics::Progress progSaveMeshes;
    Statistics::Progress progSaveFinish;
    Statistics::Progress progBuildKDTree;
    Statistics::Progress progPhotonEmission;
    Statistics::Progress progPhotonBalancing;
    Statistics::Progress progCalcLighting;
    Statistics::Progress progPostproc;
    Statistics::Progress progPostprocSector;
    Statistics::Progress progPostprocLM;
    Statistics::Progress progSaveMeshesPostLight;
    Statistics::Progress progSpecMaps;
    Statistics::Progress progSaveResult;
    Statistics::Progress progCleanLightingData;
    Statistics::Progress progApplyWorldChanges;
    Statistics::Progress progCleanup;
    Statistics::Progress progFinished;
  };

  // Global lighter
  extern Lighter* globalLighter;
}

#endif

