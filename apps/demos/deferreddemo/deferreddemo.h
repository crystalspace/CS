/*
  Copyright (C) 2010 by Joe Forte

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

#ifndef __DEFERREDDEMO_H__
#define __DEFERREDDEMO_H__

#include "crystalspace.h"

#include "ivaria/icegui.h"

struct iEngine;
struct iObjectRegistry;
struct iGraphics3D;
struct iGraphics2D;

/**
 * The main class that runs the deferred shading demo application.
 */
class DeferredDemo : public csApplicationFramework, csBaseEventHandler
{
public:

  /// Constructor.
  DeferredDemo();

  /// Destructor.
  ~DeferredDemo();

  /// Initialize the deferred shading demo application.
  virtual bool OnInitialize(int argc, char *argv[]);

  /// Main entry point to the application.
  virtual bool Application();

  /// Returns true if the application should shutdown after the current frame.
  bool ShouldShutdown() const { return shouldShutdown; }

  /// Prints help text for the demo app.
  void Help();

protected:

  /// The deferred shading demos run method.
  void RunDemo();

  /// Loads settings for the demo.
  bool LoadSettings();

  /// Loads the logo.
  bool LoadLogo();

  /// Draws the logo if possible.
  void DrawLogo();

  /// Takes a screenshot.
  bool TakeScreenShot();

  /// Updates the cameras position and rotation.
  void UpdateCamera(float deltaTime);

  /// Loads modules needed by the application.
  bool SetupModules();

  /// Loads the demo scene.
  bool LoadScene();

  /// Loads application data.
  bool LoadAppData();

  /// Setup the demo scene.
  bool SetupScene();

  /// Sets up the demo GUI.
  bool SetupGui(bool reload = false);

  /// Updates the GUI.
  void UpdateGui();

  /// Sets up the dynamics system
  bool SetupDynamicsSystem(iPluginManager *pluginManager);

  /// Creates the colliders for the dynamics system
  void CreateColliders();

  void CreateMeshColliders(const char *baseMeshName, int numMeshes);
  void CreateMeshBBoxCollider(const char *meshName);

  /// Updates the dynamics system
  void UpdateDynamics(float deltaTime);

  /// Spawns a sphere
  void SpawnSphere(bool attachLight = false);

  /// Handles an event not being Handled by csBaseEventHandler.
  virtual bool OnUnhandledEvent(iEvent &event);

  /// Handles a keyboard event.
  virtual bool OnKeyboard(iEvent &event);

  /// Handles a quit event.
  virtual bool OnQuit(iEvent &event);

  /// Handles the Frame event.
  virtual void Frame();

protected:

  csRef<iEventQueue> eventQueue;

  csRef<iEngine> engine;

  csRef<iGraphics2D> graphics2D;
  csRef<iGraphics3D> graphics3D;

  csRef<iRenderManager> rm;
  csRef<iRenderManager> rm_default;
  csRef<iDebugHelper> rm_debug;
  csRef<iRenderManagerGlobalIllum> rmGlobalIllum;

  csRef<iLoader> loader;

  csRef<iKeyboardDriver> kbd;
  csRef<iMouseDriver> md;

  csRef<iVirtualClock> vc;

  csRef<FramePrinter> printer;

  csRef<iCEGUI> cegui;
  csRef<CS::Utility::iHUDManager> hudManager;
  csRef<iConfigListener> configEventNotifier;

  csRef<iDynamics> dynamics;
  csRef<iDynamicSystem> dynamicSystem;
  csRef<CS::Physics::Bullet::iDynamicSystem> bulletDynamicSystem;
  csRef<CS::Debug::iDynamicsDebuggerManager> dynamicsDebuggerManager;
  csRef<CS::Debug::iDynamicSystemDebugger> dynamicsDebugger;

  csRef<iSector> room;

  csRef<iMeshFactoryWrapper> ballFact[6];
  csColor ballColors[3];

  csRef<iLight> light;

  csRef<iShaderVarStringSet> svStringSet;

  CS::NumberedFilenameHelper screenshotHelper;
  csString screenshotFormat;

  // Cache event names.
  csEventID quitEventID;
  csEventID cmdLineHelpEventID;

protected:

  csRef<iView> view;

  float viewRotX;
  float viewRotY;   

protected:

  CEGUI::Window *guiRoot;
  CEGUI::RadioButton *guiDeferred;
  CEGUI::RadioButton *guiForward;
  CEGUI::Checkbox *guiShowGBuffer;
  CEGUI::Checkbox *guiDrawLightVolumes;
  CEGUI::Checkbox *guiDrawLogo;
  CEGUI::Checkbox *guiEnableAO;
  CEGUI::Checkbox *guiEnableIndirectLight;
  CEGUI::Checkbox *guiEnableBlur;
  CEGUI::Checkbox *guiEnableDetailSamples;
  CEGUI::Checkbox *guiEnableGlobalIllum;

  bool showGBuffer;
  bool drawLightVolumes;
  bool showAmbientOcclusion;
  bool showGlobalIllumination;
  bool enableGlobalIllum;
  bool downsampleNormalsDepth;

  csString globalIllumResolution;
  csString depthNormalsResolution;

  bool isBulletEnabled;
  bool doBulletDebug;

  // Variables for the global illumination effects
  float occlusionStrength;
  float sampleRadius;
  float detailSampleRadius;
  int aoPasses;
  float maxOccluderDistance;  
  float selfOcclusion;
  float occAngleBias;
  float bounceStrength;
  int blurKernelSize;
  float blurPositionThreshold;
  float blurNormalThreshold;

  csRef<iEventHandler> occlusionStrengthListener;
  csRef<iEventHandler> sampleRadiusListener;
  csRef<iEventHandler> detailSampleRadiusListener;
  csRef<iEventHandler> aoPassesListener;
  csRef<iEventHandler> maxOccluderDistListener;
  csRef<iEventHandler> patternSizeListener;
  csRef<iEventHandler> selfOcclusionListener;
  csRef<iEventHandler> occAngleBiasListener;
  csRef<iEventHandler> bounceStrengthListener;
  csRef<iEventHandler> blurKernelSizeListener;
  csRef<iEventHandler> blurPositionThresholdListener;
  csRef<iEventHandler> blurNormalThresholdListener;

protected:

  csString cfgWorldDir;
  csString cfgWorldFile;
  csString cfgLogoFile;

  bool cfgDrawLogo;
  bool cfgUseDeferredShading;

  bool cfgShowGui;

  // The logo texture.
  csRef<iTextureHandle> logoTex;

private:

  bool shouldShutdown;
};

#endif //__DEFERREDDEMO_H__
