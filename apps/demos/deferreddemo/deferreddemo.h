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

#include "cstool/demoapplication.h"
#include "iengine/rendermanager.h"
#include "iutil/cfgnotifier.h"
#include "iutil/dbghelp.h"
#include "ivaria/bullet.h"
#include "ivaria/dynamics.h"
#include "ivaria/dynamicsdebug.h"
#include "ivaria/icegui.h"

/**
 * The main class that runs the deferred shading demo application.
 */
class DeferredDemo : public CS::Utility::DemoApplication
{
public:

  /// Constructor.
  DeferredDemo ();

  /// Destructor.
  ~DeferredDemo ();

  //-- CS::Utility::DemoApplication
  void PrintHelp ();
  void Frame ();
  const char* GetApplicationConfigFile ()
  { return "/config/deferreddemo.cfg"; }

  //-- csApplicationFramework
  bool OnInitialize (int argc, char *argv[]);
  bool Application  ();

  bool OnKeyboard (iEvent &event);
  bool OnMouseDown (iEvent &event);
  bool OnMouseUp (iEvent &event);

protected:

  /// Loads settings for the demo.
  bool LoadSettings ();

  /// Loads modules needed by the application.
  bool SetupModules ();

  /// Loads the demo scene.
  bool LoadScene ();

  /// Setup the demo scene.
  bool SetupScene ();

  /// Sets up the demo GUI.
  bool SetupGui (bool reload = false);

  /// Updates the GUI.
  void UpdateGui ();

  /// Sets up the dynamics system
  bool SetupDynamicsSystem ();

  /// Creates the colliders for the dynamics system
  void CreateColliders ();  

  /// Updates the dynamics system
  void UpdateDynamics (float deltaTime);

  /// Spawns a sphere
  void SpawnSphere (bool attachLight = false);

protected:

  csRef<iRenderManager> rm;
  csRef<iRenderManager> rm_default;
  csRef<iDebugHelper> rm_debug;
  csRef<iRenderManagerGlobalIllum> rmGlobalIllum;

  csRef<iDynamics> dynamics;
  csRef<iDynamicSystem> dynamicSystem;
  csRef<CS::Physics::Bullet::iDynamicSystem> bulletDynamicSystem;
  csRef<CS::Debug::iDynamicsDebuggerManager> dynamicsDebuggerManager;
  csRef<CS::Debug::iDynamicSystemDebugger> dynamicsDebugger;

  csRef<iMeshFactoryWrapper> ballFact[6];
  csColor ballColors[3];

  csRef<iLight> light;

  csRef<iShaderVarStringSet> svStringSet;

protected:

  csRef<iCEGUI> cegui;

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

  csRef<iConfigListener> configEventNotifier;
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

  bool cfgDrawLogo;
  bool cfgUseDeferredShading;

  bool cfgShowGui;
  bool cfgShowHUD;

private:

  // Event management

  /// Whether or not there is currently a mouse interaction with the camera
  bool mouseMove;

  CS_EVENTHANDLER_NAMES ("crystalspace.deferreddemo")
  
  virtual const csHandlerID * GenericPrec (csRef<iEventHandlerRegistry> &r1, 
    csRef<iEventNameRegistry> &r2, csEventID event) const 
  {
    // The CeGUI window has precedence in the mouse events iff
    // there are no current mouse interaction with the camera
    if (!mouseMove)
    {
      static csHandlerID precConstraint[2];
    
      precConstraint[0] = r1->GetGenericID("crystalspace.cegui");
      precConstraint[1] = CS_HANDLERLIST_END;
      return precConstraint;
    }

    else
    {
      static csHandlerID precConstraint[1];
    
      precConstraint[0] = CS_HANDLERLIST_END;
      return precConstraint;
    }
  }

  virtual const csHandlerID * GenericSucc (csRef<iEventHandlerRegistry> &r1, 
    csRef<iEventNameRegistry> &r2, csEventID event) const 
  {
    // The CeGUI window has precedence in the mouse events iff
    // there are no current mouse interaction with the camera
    if (mouseMove)
    {
      static csHandlerID precConstraint[2];
    
      precConstraint[0] = r1->GetGenericID("crystalspace.cegui");
      precConstraint[1] = CS_HANDLERLIST_END;
      return precConstraint;
    }

    else
    {
      static csHandlerID precConstraint[1];
    
      precConstraint[0] = CS_HANDLERLIST_END;
      return precConstraint;
    }
  }

  CS_EVENTHANDLER_DEFAULT_INSTANCE_CONSTRAINTS
};

#endif //__DEFERREDDEMO_H__
