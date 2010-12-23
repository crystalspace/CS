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

#include "deferreddemo.h"

#include "csutil/custom_new_disable.h"
#include <CEGUI.h>
#include <CEGUIWindowManager.h>
#include <CEGUILogger.h>
#include "csutil/custom_new_enable.h"

const char *DEFAULT_CFG_WORLDDIR = "/lev/castle";
const char *DEFAULT_CFG_LOGOFILE = "/lib/std/cslogo2.png";

//----------------------------------------------------------------------
DeferredDemo::DeferredDemo()
:
viewRotX(0.0f),
viewRotY(0.0f),
shouldShutdown(false)
{
  SetApplicationName ("CrystalSpace.DeferredDemo");

  // Sets default cfg values.
  cfgWorldDir = DEFAULT_CFG_WORLDDIR;
  cfgWorldFile = "world";

  cfgDrawLogo = true;
  cfgUseDeferredShading = true;
}

//----------------------------------------------------------------------
DeferredDemo::~DeferredDemo()
{}

//----------------------------------------------------------------------
bool DeferredDemo::OnInitialize(int argc, char *argv[])
{
  if (!csInitializer::RequestPlugins (GetObjectRegistry(),
      CS_REQUEST_VFS,
      CS_REQUEST_OPENGL3D,
      CS_REQUEST_ENGINE,
      CS_REQUEST_FONTSERVER,
      CS_REQUEST_IMAGELOADER,
      CS_REQUEST_LEVELLOADER,
      CS_REQUEST_REPORTER,
      CS_REQUEST_REPORTERLISTENER,
      CS_REQUEST_PLUGIN ("crystalspace.cegui.wrapper", iCEGUI),
      CS_REQUEST_END))
  {
    return ReportError("Failed to initialize plugins!");
  }

  csBaseEventHandler::Initialize (GetObjectRegistry());
  csEventID events[] = 
  {
    /* List of events to listen to. */

    csevQuit (GetObjectRegistry()),
    csevFrame (GetObjectRegistry()),
    csevKeyboardEvent (GetObjectRegistry()),
    csevCommandLineHelp (GetObjectRegistry()),

    CS_EVENTLIST_END
  };

  if (!RegisterQueue (GetObjectRegistry(), events))
  {
    return ReportError("Failed to set up event handler!");
  }

  // Setup default values.
  viewRotX = 0.0f;
  viewRotY = 0.0f;

  shouldShutdown = false;
  
  // Setup chached event names.
  quitEventID = csevQuit (GetObjectRegistry());
  cmdLineHelpEventID = csevCommandLineHelp (GetObjectRegistry());

  // Load deferred demo config file.
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (GetObjectRegistry());
  csRef<iConfigManager> cfg = csQueryRegistry<iConfigManager> (GetObjectRegistry());
  cfg->AddDomain ("/config/deferreddemo.cfg", vfs, iConfigManager::ConfigPriorityPlugin);

  csRef<iBugPlug> bugPlug = csQueryRegistry<iBugPlug> (GetObjectRegistry());

  return true;
}

//----------------------------------------------------------------------
bool DeferredDemo::SetupModules()
{
  eventQueue = csQueryRegistry<iEventQueue> (GetObjectRegistry());
  if (!eventQueue) 
    return ReportError("Failed to locate Event Queue!");

  graphics2D = csQueryRegistry<iGraphics2D> (GetObjectRegistry());
  if (!graphics2D) 
    return ReportError("Failed to locate 2D renderer!");

  graphics3D = csQueryRegistry<iGraphics3D> (GetObjectRegistry());
  if (!graphics3D) 
    return ReportError("Failed to locate 3D renderer!");

  engine = csQueryRegistry<iEngine> (GetObjectRegistry());
  if (!engine) 
    return ReportError("Failed to locate 3D engine!");

  vc = csQueryRegistry<iVirtualClock> (GetObjectRegistry());
  if (!vc) 
    return ReportError("Failed to locate Virtual Clock!");

  kbd = csQueryRegistry<iKeyboardDriver> (GetObjectRegistry());
  if (!kbd) 
    return ReportError("Failed to locate Keyboard Driver!");

  md = csQueryRegistry<iMouseDriver> (GetObjectRegistry());
  if (!md) 
    return ReportError("Failed to locate Mouse Driver!");

  loader = csQueryRegistry<iLoader> (GetObjectRegistry());
  if (!loader) 
    return ReportError("Failed to locate Loader!");

  cegui = csQueryRegistry<iCEGUI> (GetObjectRegistry());
  if (!loader) 
    return ReportError("Failed to locate CEGUI!");

  /* NOTE: Config settings for render managers are stored in 'engine.cfg' 
   * and are needed when loading a render manager. Normally these settings 
   * are added by the engine when it loads a render manager. However, since
   * we are loading the deferred render manager manually we must also manually
   * add the proper config file. */
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (GetObjectRegistry());
  csRef<iConfigManager> cfg = csQueryRegistry<iConfigManager> (GetObjectRegistry());
  cfg->AddDomain ("/config/engine.cfg", vfs, iConfigManager::ConfigPriorityPlugin);

  rm = csLoadPlugin<iRenderManager> (GetObjectRegistry(), "crystalspace.rendermanager.deferred");
  if (!rm)
    return ReportError("Failed to load deferred Render Manager!");

  cfg->RemoveDomain ("/config/engine.cfg");

  rm_debug = scfQueryInterface<iDebugHelper> (rm);
  if (!rm_debug)
    return ReportError("Failed to query the deferred Render Manager debug helper!");

  rm_default = engine->GetRenderManager ();

  return true;
}

//----------------------------------------------------------------------
bool DeferredDemo::LoadScene()
{
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (GetObjectRegistry());
  if (!vfs->ChDir (cfgWorldDir))
    return ReportError("Could not navigate to level directory %s!",
		       CS::Quote::Single (cfgWorldDir.GetDataSafe ()));

  if (!loader->LoadMapFile (cfgWorldFile))
     return ReportError("Could not load level!");

  return true;
}

//----------------------------------------------------------------------
bool DeferredDemo::LoadAppData()
{
  LoadLogo ();

  return true;
}

//----------------------------------------------------------------------
bool DeferredDemo::LoadSettings()
{
  const char *val = NULL;

  csRef<iConfigManager> cfg = csQueryRegistry<iConfigManager> (GetObjectRegistry());
  csRef<iCommandLineParser> cmdline = csQueryRegistry<iCommandLineParser> (GetObjectRegistry());

  val = cmdline->GetOption ("worlddir");
  if (val)
    cfgWorldDir = val;
  else
    cfgWorldDir = cfg->GetStr ("Deferreddemo.Settings.WorldDirectory", DEFAULT_CFG_WORLDDIR);

  cfgWorldFile = cfg->GetStr ("Deferreddemo.Settings.WorldFile", "world");
  cfgLogoFile = cfg->GetStr ("Deferreddemo.Settings.LogoFile", DEFAULT_CFG_LOGOFILE);


  if (cmdline->GetOption ("nologo"))
    cfgDrawLogo = false;
  else
    cfgDrawLogo = cfg->GetBool ("Deferreddemo.Settings.DrawLogo", true);

  if (cmdline->GetOption ("forward"))
    cfgUseDeferredShading = false;
  else
    cfgUseDeferredShading = !cfg->GetBool ("Deferreddemo.Settings.UseForward", false);

  cfgShowGui = cfg->GetBool ("Deferreddemo.Settings.ShowGui", true);

  return true;
}

//----------------------------------------------------------------------
bool DeferredDemo::SetupGui(bool reload)
{
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (GetObjectRegistry());

  if (!reload)
  {
    cegui->Initialize ();
    cegui->GetLoggerPtr ()->setLoggingLevel (CEGUI::Informative);

    // Load the ice skin.
    vfs->ChDir ("/cegui/");
    cegui->GetSchemeManagerPtr ()->create ("ice.scheme");
    cegui->GetSystemPtr ()->setDefaultMouseCursor ("ice", "MouseArrow");

    cegui->GetFontManagerPtr ()->createFreeTypeFont ("DejaVuSans", 10, true, "/fonts/ttf/DejaVuSans.ttf");
  }

  // Load layout and set as root
  CEGUI::WindowManager *winMgr = cegui->GetWindowManagerPtr ();

  if (reload)
  {
    winMgr->getWindow ("root")->destroy ();
  }
  
  vfs->ChDir ("/data/deferreddemo/");
  cegui->GetSystemPtr ()->setGUISheet (winMgr->loadWindowLayout("deferreddemo.layout"));

  guiRoot             = winMgr->getWindow ("root");
  guiDeferred         = static_cast<CEGUI::RadioButton*>(winMgr->getWindow ("Deferred"));
  guiForward          = static_cast<CEGUI::RadioButton*>(winMgr->getWindow ("Forward"));
  guiShowGBuffer      = static_cast<CEGUI::Checkbox*>(winMgr->getWindow ("ShowGBuffer"));
  guiDrawLightVolumes = static_cast<CEGUI::Checkbox*>(winMgr->getWindow ("DrawLightVolumes"));
  guiDrawLogo         = static_cast<CEGUI::Checkbox*>(winMgr->getWindow ("DrawLogo"));

  if (!guiRoot || 
      !guiDeferred || 
      !guiForward || 
      !guiShowGBuffer || 
      !guiDrawLightVolumes || 
      !guiDrawLogo)
  {
    return ReportError("Could not load GUI!");
  }

  if (cfgUseDeferredShading)
    guiDeferred->setSelected (true);
  else
    guiForward->setSelected (true);

  guiShowGBuffer->setSelected (false);
  guiDrawLightVolumes->setSelected (false);
  guiDrawLogo->setSelected (cfgDrawLogo);

  showGBuffer = false;
  drawLightVolumes = false;

  return true;
}

//----------------------------------------------------------------------
bool DeferredDemo::SetupScene()
{
  // Setup camera
  csRef<iSector> room;
  csVector3 pos (0, 0, 0);
  if (engine->GetCameraPositions ()->GetCount () > 0)
  {
    iCameraPosition* campos = engine->GetCameraPositions ()->Get (0);
    room = engine->GetSectors ()->FindByName (campos->GetSector ());
    pos = campos->GetPosition ();
  }
  else
  {
    // There are no valid starting point for the camera so we just start at the origin.
    room = engine->GetSectors ()->FindByName ("room");
    pos = csVector3 (0, 0, 0);
  }

  if (!room)
    return ReportError("Can't find a valid starting position!");

  view.AttachNew ( new csView (engine, graphics3D) );
  view->SetRectangle (0, 0, graphics2D->GetWidth (), graphics2D->GetHeight ());
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (pos);

  // Checks for support of at least 4 color buffer attachment points.
  const csGraphics3DCaps *caps = graphics3D->GetCaps();
  if (caps->MaxRTColorAttachments < 3)
    return ReportError("Graphics3D does not support at least 3 color buffer attachments!");
  else
    ReportInfo("Graphics3D supports %d color buffer attachments.", caps->MaxRTColorAttachments);

  engine->Prepare ();

  return true;
}

//----------------------------------------------------------------------
void DeferredDemo::Help()
{
  csRef<iConfigManager> cfg (csQueryRegistry<iConfigManager> (object_reg));

  csPrintf ("Options for DeferredDemo:\n");
  csPrintf ("  -nologo            do not draw logo.\n");
  csPrintf ("  -forward           use forward rendering on startup.\n");
  csPrintf ("  -world=<file>      use given world file instead of %s\n",
	    CS::Quote::Single ("world"));
  csPrintf ("  -worlddir=<path>   load map from VFS <path> (default %s)\n", 
    CS::Quote::Single (cfg->GetStr ("Walktest.Settings.WorldFile",
				    DEFAULT_CFG_WORLDDIR)));
}

//----------------------------------------------------------------------
bool DeferredDemo::Application()
{
  if (!OpenApplication (GetObjectRegistry()))
  {
    return ReportError("Error opening system!");
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (GetObjectRegistry()))
  {
    csCommandLineHelper::Help (GetObjectRegistry());
    return true;
  }

  if (SetupModules() && 
      LoadSettings() && 
      LoadAppData() &&
      LoadScene() &&
      SetupGui() &&
      SetupScene())
  {
    RunDemo();
  }

  return true;
}

//----------------------------------------------------------------------
void DeferredDemo::RunDemo()
{
  // Fetch the minimal elapsed ticks per second.
  csConfigAccess cfgacc (GetObjectRegistry(), "/config/system.cfg");
  csTicks min_elapsed = (csTicks)cfgacc->GetInt ("System.MinimumElapsedTicks", 0);

  while (!ShouldShutdown())
  {
    vc->Advance ();

    csTicks previous = csGetTicks ();

    eventQueue->Process ();

    // Limit fps.
    csTicks elapsed = csGetTicks () - previous;
    if (elapsed < min_elapsed)
      csSleep (min_elapsed - elapsed);
  }
}

//----------------------------------------------------------------------
void DeferredDemo::UpdateCamera()
{
  const float MOVE_SPEED = 5.0f;
  const float ROTATE_SPEED = 2.0f;

  float dt = (float)(vc->GetElapsedTicks () / 1000.0f);

  // Handles camera movement.
  iCamera *c = view->GetCamera ();
  if (kbd->GetKeyState (CSKEY_SHIFT))
  {
    if (kbd->GetKeyState (CSKEY_RIGHT))
      c->Move (CS_VEC_RIGHT * MOVE_SPEED * dt);
    if (kbd->GetKeyState (CSKEY_LEFT))
      c->Move (CS_VEC_LEFT * MOVE_SPEED * dt);
    if (kbd->GetKeyState (CSKEY_UP))
      c->Move (CS_VEC_UP * MOVE_SPEED * dt);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_DOWN * MOVE_SPEED * dt);
  }
  else
  {
    if (kbd->GetKeyState (CSKEY_RIGHT))
      viewRotY += ROTATE_SPEED * dt;
    if (kbd->GetKeyState (CSKEY_LEFT))
      viewRotY -= ROTATE_SPEED * dt;
    if (kbd->GetKeyState (CSKEY_PGUP))
      viewRotX += ROTATE_SPEED * dt;
    if (kbd->GetKeyState (CSKEY_PGDN))
      viewRotX -= ROTATE_SPEED * dt;
    if (kbd->GetKeyState (CSKEY_UP))
      c->Move (CS_VEC_FORWARD * MOVE_SPEED * dt);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_BACKWARD * MOVE_SPEED * dt);
  }

  csMatrix3 Rx = csXRotMatrix3 (viewRotX);
  csMatrix3 Ry = csYRotMatrix3 (viewRotY);
  csOrthoTransform V (Rx * Ry, c->GetTransform ().GetOrigin ());

  c->SetTransform (V);
}

//----------------------------------------------------------------------
void DeferredDemo::UpdateGui()
{
  guiRoot->setVisible (cfgShowGui);

  cfgDrawLogo = guiDrawLogo->isSelected ();

  if (showGBuffer != guiShowGBuffer->isSelected ())
  {
    showGBuffer = !showGBuffer;
    rm_debug->DebugCommand ("toggle_visualize_gbuffer");
  }

  if (drawLightVolumes != guiDrawLightVolumes->isSelected ())
  {
    drawLightVolumes = !drawLightVolumes;
    rm_debug->DebugCommand ("toggle_visualize_lightvolumes");
  }

  if (cfgUseDeferredShading != guiDeferred->isSelected ())
  {
     cfgUseDeferredShading = guiDeferred->isSelected ();
  } 
}

//----------------------------------------------------------------------
bool DeferredDemo::LoadLogo()
{
  logoTex = loader->LoadTexture (cfgLogoFile.GetDataSafe (), CS_TEXTURE_2D, NULL);

  if (!logoTex.IsValid ())
  {
    return ReportError("Could not load logo %s!",
		       CS::Quote::Single (cfgLogoFile.GetDataSafe ()));
  }

  return true;
}

//----------------------------------------------------------------------
void DeferredDemo::DrawLogo()
{
  if (!cfgDrawLogo || !logoTex.IsValid ())
    return;

  int w, h;
  logoTex->GetRendererDimensions (w, h);

  int screenW = graphics2D->GetWidth ();

  // Margin to the edge of the screen, as a fraction of screen width
  const float marginFraction = 0.01f;
  const int margin = (int)screenW * marginFraction;

  // Width of the logo, as a fraction of screen width
  const float widthFraction = 0.3f;
  const int width = (int)screenW * widthFraction;
  const int height = width * h / w;

  graphics3D->BeginDraw (CSDRAW_2DGRAPHICS);
  graphics3D->DrawPixmap (logoTex, 
                          screenW - width - margin, 
                          margin,
                          width,
                          height,
                          0,
                          0,
                          w,
                          h,
                          0);
}

//----------------------------------------------------------------------
void DeferredDemo::Frame ()
{
  UpdateCamera ();
  UpdateGui ();

  if (cfgUseDeferredShading)
    engine->SetRenderManager (rm);
  else
    engine->SetRenderManager (rm_default);

  view->Draw ();

  cegui->Render ();

  DrawLogo ();

  graphics3D->FinishDraw ();
  graphics3D->Print (NULL);
}

//----------------------------------------------------------------------
bool DeferredDemo::OnUnhandledEvent (iEvent &event)
{
  if (event.Name == quitEventID)
  {
    return OnQuit (event);
  }
  else if (event.Name == cmdLineHelpEventID)
  {
    Help ();
    return true;
  }

  return false;
}

//----------------------------------------------------------------------
bool DeferredDemo::OnKeyboard(iEvent &event)
{
  csKeyEventType eventtype = csKeyEventHelper::GetEventType (&event);
  if (eventtype == csKeyEventTypeDown)
  {
    utf32_char code = csKeyEventHelper::GetCookedCode(&event);
    if (code == CSKEY_ESC)
    {
      if (eventQueue.IsValid ()) 
      {
        eventQueue->GetEventOutlet ()->Broadcast( csevQuit(GetObjectRegistry()) );
        return true;
      }
    }
    else if (code == 'f')
    {
      cfgUseDeferredShading = false;
      guiForward->setSelected (true);
    }
    else if (code == 'd')
    {
      cfgUseDeferredShading = true;
      guiDeferred->setSelected (true);
    }
    else if (code == 'g')
    {
      cfgShowGui = !cfgShowGui;
    }
#ifdef CS_DEBUG
    else if (code == 'r')
    {
      SetupGui (true);
    }
#endif
  }

  return false;
}

//----------------------------------------------------------------------
bool DeferredDemo::OnQuit (iEvent &event)
{
  shouldShutdown = true;
  return true;
}
