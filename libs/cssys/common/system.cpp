/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "sysdef.h"
#include "cssys/common/system.h"
#include "cssys/common/sysdriv.h"
#include "csgeom/csrect.h"
#include "csutil/inifile.h"
#include "csinput/csinput.h"
#include "apps/support/console.h"	//@@@???
#include "isndrdr.h"
#include "inetdrv.h"
#include "inetman.h"
#include "iconfig.h"
#include "igraph3d.h"
#include "igraph2d.h"

// The global system variable
csSystemDriver *System = NULL;

// Make Shutdown static so that even if System has not been initialized,
// application can tell system driver to exit immediately
bool csSystemDriver::Shutdown = false;
bool csSystemDriver::ExitLoop = false;
bool csSystemDriver::DemoReady = false;

// Global configuration file
csIniFile *config = NULL;

// Forced driver name.
char override_driver[100] = { 0 };

// Global debugging level.
int csSystemDriver::debug_level = 0;

// the CLSID of the rendersystem to use.
static CLSID clsidRenderSystem;

// the CLSID of the network driver system to use.
static CLSID clsidNetworkDriverSystem;

// the CLSID of the network manager system to use.
static CLSID clsidNetworkManagerSystem;

// the CLSID of the sound render system to use.
static CLSID clsidSoundRenderSystem;

// IUnknown implementation for csSystemDriver
IMPLEMENT_UNKNOWN_NODELETE (csSystemDriver)

BEGIN_INTERFACE_TABLE (csSystemDriver)
    IMPLEMENTS_COMPOSITE_INTERFACE (System)
END_INTERFACE_TABLE()


void default_fatal_exit (int errorcode, bool canreturn)
{
  (void)canreturn;
  exit (errorcode);
}

void (*fatal_exit) (int errorcode, bool canreturn) = default_fatal_exit;

csSystemDriver::csSystemDriver()
{
  pprintf_init();
  System = this;
  FullScreen = false;
  Mouse = NULL;
  Keyboard = NULL;
  
  piG3D = NULL;
  piG2D = NULL;
  piGI = NULL;
  piNetDrv = NULL;
  piNetMan = NULL;
  piSound = NULL;

  EventQueue = NULL;
  Console = NULL;
  IsFocused = true;
  com_options = NULL;
  cfg_engine = NULL;

  csCoInitialize (0);
}

csSystemDriver::~csSystemDriver ()
{
  Close ();
  
  CHK (delete Mouse);
  CHK (delete Keyboard);
  CHK (delete EventQueue);
  CHK (delete Console);

  FINAL_RELEASE (piSound);
  FINAL_RELEASE (piG3D);
  FINAL_RELEASE (piGI);
  FINAL_RELEASE (piNetDrv);
  FINAL_RELEASE (piNetMan);

  csCoUninitialize();

  System = NULL;
  CHK (delete com_options);
}

bool csSystemDriver::Initialize (int argc, char *argv[], IConfig* cfg_engine)
{
  csSystemDriver::cfg_engine = cfg_engine;
  HRESULT hRes;

  SetSystemDefaults ();
  if (!ParseCmdLineDriver (argc, argv))
    return false;

  // DAN 9.29.98 - the DRIVER variable is now the ProgID for the comclass to use to
  // retrieve the IGraphics3D, IGraphics2D, and IGraphicsInfo interfaces. 
  char *pn = "crystalspace.graphics3d.software";
  if (*override_driver) 
    pn = override_driver;
  else if (config) 
    pn = config->GetStr ("VideoDriver", "DRIVER", pn);

  hRes = csCLSIDFromProgID (&pn, &clsidRenderSystem);
  if (FAILED(hRes))
  {
    Printf (MSG_FATAL_ERROR, "Bad value '%s' for DRIVER in configuration file\n", pn);
    return false;
  }

  pn = "crystalspace.network.driver.null";
  if (config) pn = config->GetStr ("Network", "NetDriver", pn);

  hRes = csCLSIDFromProgID (&pn, &clsidNetworkDriverSystem);
  if (FAILED(hRes))
  {
    Printf (MSG_FATAL_ERROR, "Bad value '%s' for network driver in configuration file\n", pn);
    return false;
  }

  pn = "crystalspace.network.manager.null";
  if (config) pn = config->GetStr ("Network", "NetManager", pn);

  hRes = csCLSIDFromProgID (&pn, &clsidNetworkManagerSystem);
  if (FAILED(hRes))
  {
    Printf(MSG_FATAL_ERROR, "Bad value '%s' for network manager in configuration file\n", pn);
    return false;
  }

  pn = "crystalspace.sound.render.null";
  if (config) pn = config->GetStr ("Sound", "SoundRender", pn);

  hRes = csCLSIDFromProgID (&pn, &clsidSoundRenderSystem);
  if (FAILED(hRes))
  {
    Printf(MSG_FATAL_ERROR, "Bad value '%s' for [Sound].SoundRender in configuration file\n", pn);
    return false;
  }

  CHK (EventQueue = new csEventQueue ());
  InitGraphics ();
  InitKeyboard ();
  InitMouse ();
  InitSound ();
  InitNetwork ();

  if (!ParseCmdLine (argc, argv))
  {
    Close ();
    return false;
  }
  
  piG3D->Initialize ();

  return true;
}

bool csSystemDriver::Open (char *Title)
{
  // the initialization order is crucial! do not reorder!
  if (FAILED (piG3D->Open (Title)))
    return false;
  if (!Keyboard->Open (EventQueue))
    return false;
  if (!Mouse->Open (GetISystemFromSystem (this), EventQueue))
    return false;
  if (piSound)
    if (FAILED (piSound->Open ()))
      return false;
  if (piNetDrv)
    if (FAILED (piNetDrv->Open ()))
      return false;
  if (piNetMan)
    if (FAILED (piNetMan->Open ()))
      return false;
  return true;
}

void csSystemDriver::Close(void)
{
  if (piSound) piSound->Close ();
  if (piNetMan) piNetMan->Close ();
  if (piNetDrv) piNetDrv->Close ();
  if (piG3D) piG3D->Close ();
  if (Keyboard) Keyboard->Close ();
  if (Mouse) Mouse->Close ();
}

bool csSystemDriver::InitGraphics ()
{
  HRESULT hRes;
  IGraphicsContextFactory* pFactory;

  hRes = csCoGetClassObject (clsidRenderSystem, CLSCTX_INPROC_SERVER, NULL, IID_IGraphicsContextFactory, (void**)&pFactory);
  if (FAILED(hRes)) goto OnError;

  hRes = pFactory->CreateInstance ((REFIID)IID_IGraphics3D, GetISystemFromSystem(this), (void**)&piG3D);
  if (FAILED(hRes)) goto OnError;

  hRes = piG3D->Get2dDriver (&piG2D);
  if (FAILED(hRes)) goto OnError;

  hRes = piG2D->QueryInterface ((REFIID)IID_IGraphicsInfo, (void**)&piGI);
  if (FAILED(hRes)) goto OnError;

OnError:
  
  if (FAILED (hRes))
  {
    Printf (MSG_FATAL_ERROR, "Error loading graphics context server\n");
    fatal_exit (0, false);
  }

  if (piG3D != NULL)
  {
    // Initialize default setting as given by the commandline.
    piG3D->SetRenderState (G3DRENDERSTATE_DEBUGENABLE, G3DSettings.do_debug);
    piG3D->SetRenderState (G3DRENDERSTATE_INTERLACINGENABLE, G3DSettings.cfg_interlacing);
    piG3D->SetCacheSize (G3DSettings.cache_size);
  }

  return (piG3D != NULL  &&  piG2D != NULL  &&  piGI != NULL);
}

bool csSystemDriver::InitKeyboard ()
{
  CHK (Keyboard = new SysKeyboardDriver());
  return (Keyboard != NULL);
}

bool csSystemDriver::InitMouse ()
{
  CHK (Mouse = new SysMouseDriver ());
  return (Mouse != NULL);
}

bool csSystemDriver::InitSound ()
{
  HRESULT hRes;
  ISoundRenderFactory* pFactory;

  hRes = csCoGetClassObject( clsidSoundRenderSystem, CLSCTX_INPROC_SERVER, NULL, IID_ISoundRenderFactory, (void**)&pFactory );
  if (FAILED (hRes))
  {
    Printf (MSG_FATAL_ERROR, "Error loading sound render context server\n");
    fatal_exit (0, false);
  }

  hRes = pFactory->CreateInstance( (REFIID)IID_ISoundRender, GetISystemFromSystem(this), (void**)&piSound );
  if (FAILED (hRes))
  {
    Printf (MSG_FATAL_ERROR, "Error creating sound render context server.");
    fatal_exit (0, false);
  }

  return (piSound != NULL);
}

bool csSystemDriver::InitNetwork ()
{
  HRESULT hRes;
  INetworkDriverFactory* pDriverFactory;
  INetworkManagerFactory* pManagerFactory;
  
  hRes = csCoGetClassObject( clsidNetworkDriverSystem, CLSCTX_INPROC_SERVER, NULL, IID_INetworkDriverFactory, (void**)&pDriverFactory );
  if (FAILED (hRes))
  {
    Printf (MSG_FATAL_ERROR, "Error loading network driver context server\n");
    fatal_exit (0, false);
  }

  hRes = pDriverFactory->CreateInstance( (REFIID)IID_INetworkDriver, GetISystemFromSystem(this), (void**)&piNetDrv );
  if (FAILED (hRes))
  {
    Printf (MSG_FATAL_ERROR, "Error loading network driver context server.");
    fatal_exit (0, false);
  }

  hRes = csCoGetClassObject( clsidNetworkManagerSystem, CLSCTX_INPROC_SERVER, NULL, IID_INetworkManagerFactory, (void**)&pManagerFactory );
  if (FAILED (hRes))
  {
    Printf (MSG_FATAL_ERROR, "Error loading network manager context server.");
    fatal_exit (0, false);
  }

  hRes = pManagerFactory->CreateInstance( (REFIID)IID_INetworkManager, GetISystemFromSystem(this), (void**)&piNetMan );
  if (FAILED (hRes))
  {
    Printf (MSG_FATAL_ERROR, "Error loading network manager context server.");
    fatal_exit (0, false);
  }

  return ((piNetDrv != NULL) && (piNetMan != NULL));
}

void csSystemDriver::NextFrame (long /*elapsed_time*/, long /*current_time*/)
{
  if (piSound) piSound->Update ();
}

void csSystemDriver::SetSystemDefaults ()
{
  FrameWidth = 640; if (config) FrameWidth = config->GetInt ("VideoDriver", "WIDTH", FrameWidth);
  FrameHeight = 480; if (config) FrameHeight = config->GetInt ("VideoDriver", "HEIGHT", FrameHeight);
  Depth = 8; if (config) Depth = config->GetInt ("VideoDriver", "DEPTH", Depth);
  G3DSettings.cfg_interlacing = false; if (config) G3DSettings.cfg_interlacing = config->GetYesNo ("VideoDriver", "INTERLACING", G3DSettings.cfg_interlacing);
  G3DSettings.cache_size = 5242880; if (config) G3DSettings.cache_size = config->GetInt ("TextureMapper", "CACHE", G3DSettings.cache_size);
  G3DSettings.do_debug = false;
}

bool csSystemDriver::ParseCmdLineDriver (int argc, char* argv[])
{
  int i;

  for (i = 1; i < argc; i++)
    if (!ParseArgDriver (argc, argv, i))
      return false;
  return true;
}

bool csSystemDriver::ParseArgDriver (int argc, char* argv[], int& i)
{
  if (strcasecmp ("-driver", argv[i]) == 0)
  {
    i++;
    if (i < argc)
    {
      sprintf (override_driver, "crystalspace.graphics3d.%s", argv[i]);
      Printf (MSG_INITIALIZATION, "Use 3D driver '%s'.\n", override_driver);
    }
  }
  return true;
}

csColOption* csSystemDriver::CollectOptions (IConfig* config, csColOption* already_collected)
{
  int i, num;
  csOptionDescription option;
  config->GetNumberOptions (num);
  char buf[100];
  for (i = 0 ; i < num ; i++)
  {
    config->GetOptionDescription (i, &option);
    switch (option.type)
    {
      case CSVAR_BOOL:
        CHK (already_collected = new csColOption (already_collected, option.type, option.id, option.name,
		true, config));
	strcpy (buf, "no");
	strcat (buf, option.name);
        CHK (already_collected = new csColOption (already_collected, option.type, option.id, buf,
		false, config));
	break;
      case CSVAR_CMD:
        CHK (already_collected = new csColOption (already_collected, option.type, option.id, option.name,
		true, config));
	break;
      case CSVAR_LONG:
      case CSVAR_FLOAT:
        CHK (already_collected = new csColOption (already_collected, option.type, option.id, option.name,
		false, config));
	break;
    }
  }
  return already_collected;
}

bool csSystemDriver::ParseCmdLine (int argc, char* argv[])
{
  CHK (delete com_options);
  com_options = NULL;
  int i;

  HRESULT hRes;
  IConfig* piConf;
  if (cfg_engine)
  {
    com_options = CollectOptions (cfg_engine, com_options);
  }
  if (piG3D)
  {
    hRes = piG3D->QueryInterface ((REFIID)IID_IConfig, (void**)&piConf);
    if (SUCCEEDED (hRes))
    {
      com_options = CollectOptions (piConf, com_options);
      piConf->Release ();
    }
  }
  if (piG2D)
  {
    hRes = piG2D->QueryInterface ((REFIID)IID_IConfig, (void**)&piConf);
    if (SUCCEEDED (hRes))
    {
      com_options = CollectOptions (piConf, com_options);
      piConf->Release ();
    }
  }

  for (i = 1; i < argc; i++)
    if (!ParseArg (argc, argv, i))
    {
      CHK (delete com_options);
      com_options = NULL;
      return false;
    }

  CHK (delete com_options);
  com_options = NULL;
  return true;
}

bool csSystemDriver::ParseArg (int argc, char* argv[], int& i)
{
  if (strcasecmp ("-help", argv[i]) == 0)
  {
    Help ();
    exit (0);
  }
  else if (strcasecmp ("-debug", argv[i]) == 0)
  {
    G3DSettings.do_debug = true;
    Printf (MSG_INITIALIZATION, "Debugging info enabled.\n");
  }
  else if (strcasecmp ("-nodebug", argv[i]) == 0)
  {
    G3DSettings.do_debug = false;
    Printf (MSG_INITIALIZATION, "Debugging info disabled.\n");
  }
  else if (strcasecmp ("-cache", argv[i]) == 0)
  {
    i++;
    if (i < argc) sscanf (argv[i], "%ld", &G3DSettings.cache_size);
  }
  else if (strcasecmp ("-mixing", argv[i]) == 0)
  {
  //@@@
    i++;
    //if (i < argc) if (!Textures::force_mixing (argv[i])) fatal_exit (0,false);
  }
  else if (strcasecmp ("-txtmode", argv[i]) == 0)
  {
    //@@@
    i++;
    //if (i < argc) if (!Textures::force_txtmode (argv[i])) fatal_exit (0,false);
  }
  else if (strcasecmp ("-mode", argv[i]) == 0)
  {
    if (++i < argc) SetMode (argv[i]);
  }
  else if (strcasecmp ("-depth", argv[i]) == 0)
  {
    if (++i < argc) Depth = atoi (argv[i]);
  }
  else if (strcasecmp ("-driver", argv[i]) == 0)
  {
    i++;
  }
  else if (argv[i][0] == '-')
  {
    csVariant val;
    csColOption* opt = com_options;
    while (opt)
    {
      if (strcasecmp (opt->option_name, argv[i]+1) == 0)
      {
	val.type = opt->type;
        switch (opt->type)
	{
	  case CSVAR_BOOL:
	  case CSVAR_CMD:
	    val.v.bVal = opt->option_value;
	    break;
	  case CSVAR_LONG:
    	    if (++i < argc) val.v.lVal = atol (argv[i]);
	    break;
	  case CSVAR_FLOAT:
    	    if (++i < argc) val.v.fVal = atof (argv[i]);
	    break;
	}
	opt->config->SetOption (opt->id, &val);
	break;
      }
      opt = opt->next;
    }
    if (!opt)
    {
      Printf (MSG_FATAL_ERROR, "Unknown commandline parameter '%s'!\n\
        (Use '-help' to get a list of options)\n", argv[i]);
      fatal_exit (0, false);
    }
  }
  return true;
}

void csSystemDriver::Help (IConfig* piConf)
{
  int i, num;
  csOptionDescription option;
  piConf->GetNumberOptions (num);
  for (i = 0 ; i < num ; i++)
  {
    piConf->GetOptionDescription (i, &option);
    char buf[120];
    strcpy (buf, "                                                                              ");
    csVariant def;
    piConf->GetOption (i, &def);
    switch (option.type)
    {
      case CSVAR_BOOL:
        sprintf (buf, "  -%s/no%s ", option.name, option.name);
	buf[strlen (buf)] = ' ';
	sprintf (buf+21, "%s (%s) ", option.description, def.v.bVal ? "yes" : "no");
	buf[strlen (buf)] = ' ';
	buf[78] = 0;
	break;
      case CSVAR_CMD:
        sprintf (buf, "  -%s ", option.name);
	buf[strlen (buf)] = ' ';
	sprintf (buf+21, "%s ", option.description);
	buf[strlen (buf)] = ' ';
	buf[78] = 0;
	break;
      case CSVAR_FLOAT:
        sprintf (buf, "  -%s <val> ", option.name);
	buf[strlen (buf)] = ' ';
	sprintf (buf+21, "%s (%f) ", option.description, def.v.fVal);
	buf[strlen (buf)] = ' ';
	buf[78] = 0;
	break;
      case CSVAR_LONG:
        sprintf (buf, "  -%s <val> ", option.name);
	buf[strlen (buf)] = ' ';
	sprintf (buf+21, "%s (%ld) ", option.description, def.v.lVal);
	buf[strlen (buf)] = ' ';
	buf[78] = 0;
	break;
    }
    Printf (MSG_STDOUT, "%s\n", buf);
  }
}

void csSystemDriver::Help ()
{
  HRESULT hRes;
  IConfig* piConf;
  if (cfg_engine)
  {
    Printf (MSG_STDOUT, "Options for 3D engine:\n");
    Help (cfg_engine);
  }
  if (piG3D)
  {
    hRes = piG3D->QueryInterface ((REFIID)IID_IConfig, (void**)&piConf);
    if (SUCCEEDED (hRes))
    {
      Printf (MSG_STDOUT, "Options for 3D rasterizer:\n");
      Help (piConf);
      piConf->Release ();
    }
  }
  if (piG2D)
  {
    hRes = piG2D->QueryInterface ((REFIID)IID_IConfig, (void**)&piConf);
    if (SUCCEEDED (hRes))
    {
      Printf (MSG_STDOUT, "Options for 2D rasterizer:\n");
      Help (piConf);
      piConf->Release ();
    }
  }
  Printf (MSG_STDOUT, "General options:\n");
  Printf (MSG_STDOUT, "  -help              this help\n");
  Printf (MSG_STDOUT, "  -debug/nodebug     keep more debugging info for crashes (default '%sdebug')\n", G3DSettings.do_debug ? "" : "no");
  Printf (MSG_STDOUT, "  -cache <size>      set the texture cache size (default=%d)\n", G3DSettings.cache_size);
  Printf (MSG_STDOUT, "  -mixing <mode>     set the mode used for mixing\n");
  Printf (MSG_STDOUT, "  -txtmode <mode>    set the texture mode\n");
  Printf (MSG_STDOUT, "  -mode <w>x<y>      set resolution (default=%dx%d)\n", FrameWidth, FrameHeight);
  Printf (MSG_STDOUT, "  -depth <d>         set depth (default=%d bpp)\n", Depth);
  Printf (MSG_STDOUT, "  -driver <s>        the 3D driver (opengl, glide, software, ...)\n");
}

void csSystemDriver::SetMode (const char* mode)
{
  int wres, hres;
  if (sscanf(mode, "%dx%d", &wres, &hres) != 2)
  {
    Printf (MSG_INITIALIZATION, "Mode %s unknown : assuming '-mode %dx%d'\n", mode,
      FrameWidth, FrameHeight);
  }
  else
  {
    FrameWidth = wres;
    FrameHeight = hres;
  }
}

void csSystemDriver::Alert (const char* msg)
{
  pprintf (msg);
  debugprintf (true, msg);
}

void csSystemDriver::Warn (const char* msg)
{
  pprintf (msg);
  debugprintf (true, msg);
}

void csSystemDriver::Printf (int mode, const char* str, ...)
{
  char buf[1024];
  va_list arg;

  va_start (arg, str);
  vsprintf (buf, str, arg);
  va_end (arg);

  switch (mode)
  {
    case MSG_INTERNAL_ERROR:
    case MSG_FATAL_ERROR:
      if (System)
        System->Alert (buf);
      else
      {
        pprintf ("%s", buf);
        debugprintf (true, "%s", buf);
      }
      break;

    case MSG_WARNING:
      if (System)
        System->Warn (buf);
      else
      {
        pprintf ("%s", buf);
        debugprintf (true, "%s", buf);
      }
      break;

    case MSG_INITIALIZATION:
      pprintf ("%s", buf);
      debugprintf (true, "%s", buf);
      if (System->DemoReady)
        System->DemoWrite (buf);
      break;

    case MSG_CONSOLE:
      if (System && System->Console)
        System->Console->PutText ("%s", buf);
      else
        pprintf ("%s", buf);
      break;

    case MSG_STDOUT:
      pprintf ("%s", buf);
      break;

    case MSG_DEBUG_0:
      debugprintf (false, "%s", buf);
      break;

    case MSG_DEBUG_1:
      if (debug_level >= 1)
        debugprintf (false, "%s", buf);
      break;

    case MSG_DEBUG_2:
      if (debug_level >= 2)
        debugprintf (false, "%s", buf);
      break;

    case MSG_DEBUG_0F:
      debugprintf (true, "%s", buf);
      break;

    case MSG_DEBUG_1F:
      if (debug_level >= 1)
        debugprintf (true, "%s", buf);
      break;

    case MSG_DEBUG_2F:
      if (debug_level >= 2)
        debugprintf (true, "%s", buf);
      break;

    case MSG_TICKER:
    {
      static print_work_counter=0;

      if(!strcasecmp(buf,"begin"))
        print_work_counter=1;
      if(!strcasecmp(buf,"end"))
        print_work_counter=0;

      if (System->DemoReady&&print_work_counter)
      {
        if (System->Console)
	{
	  System->Console->ShowWork ();
          // @@@ Don't know how to update console in another way. Change it!
          System->DemoWrite ("");
	}
      }
      break;
    }
  } /* endswitch */
}

void csSystemDriver::DemoWrite (const char* buf)
{
  if (Console)
  {
    
    if (piG2D)
    {
      piG2D->Clear (0);
    }

    Console->PutText ("%s", buf);
    csRect area;
    Console->Print (&area);
    
    if (piG2D)
    {
      piG2D->FinishDraw ();
      piG2D->Print (&area);
    }
  
  }
}

/*
* Print a message on the console or in stdout/debug.txt.
*/
void csSystemDriver::debugprintf (bool flush, const char *str, ...)
{
  char buf[1024];
  va_list arg;

  va_start (arg, str);
  vsprintf (buf, str, arg);
  va_end (arg);
  
  static FILE *f = NULL;
  if (!f)
    f = fopen ("debug.txt", "a+");
  if (f)
  {
    fwrite (buf, strlen (buf), 1, f);
    if (flush)
      fflush (f);
  }
}

void csSystemDriver::do_focus (int enable)
{
  if (enable == (int)IsFocused)
    return;
  IsFocused = enable;

  CHK (EventQueue->Put (new csEvent (Time(), csevBroadcast, cscmdFocusChanged, (void *)enable)));
  if (!enable)
  {
    Keyboard->Reset ();
    Mouse->Reset ();
  }
}

// COM implementation

IMPLEMENT_COMPOSITE_UNKNOWN_AS_EMBEDDED (csSystemDriver, System)

STDMETHODIMP csSystemDriver::XSystem::GetDepthSetting(int& retval)
{
    METHOD_PROLOGUE (csSystemDriver, System);
    retval = pThis->Depth;
    return S_OK;
}

STDMETHODIMP csSystemDriver::XSystem::GetFullScreenSetting(bool& retval)
{
    METHOD_PROLOGUE (csSystemDriver, System);
    retval = pThis->FullScreen;
    return S_OK;
}

STDMETHODIMP csSystemDriver::XSystem::GetHeightSetting(int& retval)
{
    METHOD_PROLOGUE (csSystemDriver, System);
    retval = pThis->FrameHeight;
    return S_OK;
}

STDMETHODIMP csSystemDriver::XSystem::GetWidthSetting(int& retval)
{
    METHOD_PROLOGUE (csSystemDriver, System);
    retval = pThis->FrameWidth;
    return S_OK;
}

STDMETHODIMP csSystemDriver::XSystem::Print(int mode, const char* string)
{
    METHOD_PROLOGUE (csSystemDriver, System);

    pThis->Printf(mode, "%s", string);
    return S_OK;
}

STDMETHODIMP csSystemDriver::XSystem::FOpen (const char* filename, 
                                             const char* mode, FILE** fp)
{
  METHOD_PROLOGUE (csSystemDriver, System);
  *fp = pThis->fopen (filename, mode);
  return S_OK;
}

STDMETHODIMP csSystemDriver::XSystem::FClose (FILE* fp)
{
  fclose (fp);
  return S_OK;
}

STDMETHODIMP csSystemDriver::XSystem::GetTime (time_t& time)
{
  time = csSystemDriver::Time ();
  return S_OK;
}

STDMETHODIMP csSystemDriver::XSystem::Shutdown ()
{
  METHOD_PROLOGUE (csSystemDriver, System);
  pThis->Shutdown = true;
  return S_OK;
}

STDMETHODIMP csSystemDriver::XSystem::GetShutdown (bool &Shutdown)
{
  METHOD_PROLOGUE (csSystemDriver, System);
  Shutdown = pThis->Shutdown;
  return S_OK;
}

STDMETHODIMP csSystemDriver::XSystem::GetSubSystemPtr(void **retval, int iSubSystemID)
{
  //METHOD_PROLOGUE (csSystemDriver, System);
  *retval = NULL;
//  (*retval)->AddRef ();
  return S_OK;
}


STDMETHODIMP csSystemDriver::XSystem::ConfigGetInt (char *Section, char *Key,
  int &Value, int Default)
{
  Value = Default; if (config) Value = config->GetInt (Section, Key, Default);
  return S_OK;
}

STDMETHODIMP csSystemDriver::XSystem::ConfigGetStr (char *Section, char *Key,
  char *&Value, char *Default)
{
  Value = Default; if (config) Value = config->GetStr (Section, Key, Default);
  return S_OK;
}

STDMETHODIMP csSystemDriver::XSystem::ConfigGetYesNo (char *Section, char *Key,
  bool &Value, bool Default)
{
  Value = Default; if (config) Value = config->GetYesNo (Section, Key, Default);
  return S_OK;
}
