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
#include "cssys/system.h"
#include "cssys/sysdriv.h"
#include "cssys/console.h"
#include "csutil/csrect.h"
#include "csutil/util.h"
#include "csutil/inifile.h"
#include "csutil/vfs.h"
#include "csinput/csinput.h"
#include "iplugin.h"
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
bool csSystemDriver::ConsoleReady = false;

// Forced driver name.
csStrVector extra_plugins (8, 8);

// Global debugging level.
int csSystemDriver::debug_level = 0;

// This is the default fatal exit function. The user can replace
// it by some other function that lonjumps somewhere, for example.
// The 'canreturn' indicates a error that can be ignored (i.e. a
// not-too-fatal error) and if it is 'true', the function can return.
// Returning when canreturn is 'false' will cause unpredictable behavior.
void default_fatal_exit (int errorcode, bool canreturn)
{
  (void)canreturn;
  exit (errorcode);
}

void (*fatal_exit) (int errorcode, bool canreturn) = default_fatal_exit;

//--- csPlugIn class ---//

csSystemDriver::csPlugIn::csPlugIn (iPlugIn *iObject, const char *iClassID)
{
  PlugIn = iObject;
  ClassID = strnew (iClassID);
  EventMask = 0;
}

csSystemDriver::csPlugIn::~csPlugIn ()
{
  delete [] ClassID;
  PlugIn->DecRef ();
}

//---------------------------------------------------- The System Driver -----//

csSystemDriver::csSystemDriver () : PlugIns (8, 8)
{
  console_open ();
  System = this;
  IsFocused = true;
  FullScreen = false;
  Mouse = NULL;
  Keyboard = NULL;
  
  G3D = NULL;
  G2D = NULL;
  NetDrv = NULL;
  NetMan = NULL;
  Sound = NULL;

  EventQueue = NULL;
  Console = NULL;
  OptionList = NULL;
  ConfigName = NULL;
}

csSystemDriver::~csSystemDriver ()
{
#ifdef DEBUG
  printf ("System driver is going to shut down now!\n");
#endif

  Close ();

  if (Config)
    CHKB (delete Config);
  if (ConfigName)
    CHKB (delete [] ConfigName);

  System = NULL;

  CHK (delete OptionList);

  // Decrement reference count for all known drivers
  if (Sound) Sound->DecRef ();
  if (NetDrv) NetDrv->DecRef ();
  if (NetMan) NetMan->DecRef ();
  if (G2D) G2D->DecRef ();
  if (G3D) G3D->DecRef ();
  if (VFS) VFS->DecRef ();

  // Free all plugins
  PlugIns.DeleteAll ();
  
  CHK (delete Console);
  CHK (delete Mouse);
  CHK (delete Keyboard);
  CHK (delete EventQueue);

  scfFinish ();
  console_close ();
}

bool csSystemDriver::Initialize (int argc, char *argv[], const char *iConfigName)
{
  // Increment our reference count to not get dumped when
  // someone will do an IncRef() and then an DecRef().
  IncRef ();

  // Initialize configuration file
  if (iConfigName)
    Config = new csIniFile (ConfigName = strnew (iConfigName));
  else
    Config = new csIniFile ();

  // Initialize Shared Class Facility
  csIniFile *SCFconfig = new csIniFile ("scf.cfg");
  scfInitialize (SCFconfig);
  delete SCFconfig;

  // Create the Event Queue
  CHK (EventQueue = new csEventQueue ());

  SetSystemDefaults (Config);
  if (!ParseCmdLineDriver (argc, argv))
    return false;

  // Now load and initialize all plugins
  csStrVector PluginList (8, 8);
  Config->EnumData ("PlugIns", &PluginList);
  while (extra_plugins.Length () || PluginList.Length ())
  {
    char *classID;
    if (extra_plugins.Length ())
      classID = (char *)extra_plugins.Get (0);
    else
      classID = Config->GetStr ("PlugIns", (char *)PluginList.Get (0));

    LoadPlugIn (classID, NULL, 0);

    if (extra_plugins.Length ())
      extra_plugins.Delete (0);
    else
      PluginList.Delete (0);
  }

  InitKeyboard ();
  InitMouse ();

  // Check if the minimal required drivers are loaded
  if (!CheckDrivers ())
    return false;

  if (!ParseCmdLine (argc, argv))
  {
    Close ();
    return false;
  }
  
  return true;
}

bool csSystemDriver::Open (const char *Title)
{
  if (G3D && !G3D->Open (Title))
    return false;
  // Query frame width/height/depth as it possibly has been adjusted after open
  FrameWidth = G2D->GetWidth ();
  FrameHeight = G2D->GetHeight ();
  Depth = G2D->GetPixelBytes ();
  Depth *= 8;

  if (!Keyboard->Open (EventQueue))
    return false;
  if (!Mouse->Open (this, EventQueue))
    return false;

  if (Sound)
    if (!Sound->Open ())
      return false;
  if (NetDrv)
    if (!NetDrv->Open ())
      return false;
  if (NetMan)
    if (!NetMan->Open ())
      return false;

  // Now pass the open event to all plugins
  csEvent e (GetTime (), csevBroadcast, cscmdSystemOpen);
  HandleEvent (e);

  return true;
}

void csSystemDriver::Close(void)
{
  // Warn all plugins the system is going down
  csEvent e (GetTime (), csevBroadcast, cscmdSystemClose);
  HandleEvent (e);

  if (Sound)
    Sound->Close ();
  if (NetMan)
    NetMan->Close ();
  if (NetDrv)
    NetDrv->Close ();
  if (G3D)
    G3D->Close ();
  if (Keyboard)
    Keyboard->Close ();
  if (Mouse)
    Mouse->Close ();
}

bool csSystemDriver::CheckDrivers ()
{
  return (VFS && G2D && G3D);
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

void csSystemDriver::NextFrame (time_t /*elapsed_time*/, time_t /*current_time*/)
{
  ProcessEvents ();
  if (Sound)
    Sound->Update ();
}

bool csSystemDriver::ProcessEvents ()
{
  csEvent *ev;
  bool did_some_work = false;
  while ((ev = EventQueue->Get ()))
  {
    did_some_work = true;
    HandleEvent (*ev);
    CHK (delete ev);
  }
  return did_some_work;
}

bool csSystemDriver::HandleEvent (csEvent &Event)
{
  int evmask = 1 << Event.Type;
  bool canstop = (Event.Type == csevBroadcast);
  for (int i = 0; i < PlugIns.Length (); i++)
  {
    csPlugIn *plugin = (csPlugIn *)PlugIns.Get (i);
    if (plugin->EventMask & evmask)
      if (plugin->PlugIn->HandleEvent (Event) && canstop)
        return true;
  }

  // If user switches away from our application, reset keyboard and mouse state
  if ((Event.Type == csevBroadcast)
   && (Event.Command.Code == cscmdFocusChanged)
   && (Event.Command.Info == NULL))
  {
    Keyboard->Reset ();
    Mouse->Reset ();
  }

  return false;
}

void csSystemDriver::SetSystemDefaults (csIniFile *Config)
{
  FrameWidth = Config->GetInt ("VideoDriver", "Width", 640);
  FrameHeight = Config->GetInt ("VideoDriver", "Height", 480);
  Depth = Config->GetInt ("VideoDriver", "Depth", 16);
  FullScreen = Config->GetYesNo ("VideoDriver", "FullScreen", false);
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
      char temp [100];
      if (strchr (argv [i], '.'))
      {
        strcpy (temp, argv [i]);
        Printf (MSG_INITIALIZATION, "Loading additional plug-in '%s'.\n", temp);
      }
      else
      {
        sprintf (temp, "crystalspace.graphics3d.%s", argv [i]);
        Printf (MSG_INITIALIZATION, "Using 3D driver '%s'.\n", temp);
      }
      extra_plugins.Push (strnew (temp));
    }
  }
  return true;
}

void csSystemDriver::CollectOptions (iConfig* Config, csColOption* already_collected)
{
  char buf[100];
  csOptionDescription option;
  int num = Config->GetOptionCount ();
  for (int i = 0 ; i < num ; i++)
  {
    Config->GetOptionDescription (i, &option);
    switch (option.type)
    {
      case CSVAR_BOOL:
        CHK (already_collected = new csColOption (already_collected, option.type,
          option.id, option.name, true, Config));
	strcpy (buf, "no");
	strcat (buf, option.name);
        CHK (already_collected = new csColOption (already_collected, option.type,
          option.id, buf, false, Config));
	break;
      case CSVAR_CMD:
        CHK (already_collected = new csColOption (already_collected, option.type,
          option.id, option.name, true, Config));
	break;
      case CSVAR_LONG:
      case CSVAR_FLOAT:
        CHK (already_collected = new csColOption (already_collected, option.type,
          option.id, option.name, false, Config));
	break;
    }
  }
}

bool csSystemDriver::ParseCmdLine (int argc, char* argv[])
{
  CHK (delete OptionList);
  OptionList = NULL;
  int i;

  for (i = 0; i < PlugIns.Length (); i++)
  {
    csPlugIn *plugin = (csPlugIn *)PlugIns.Get (i);
    iConfig *cfg = QUERY_INTERFACE (plugin->PlugIn, iConfig);
    if (cfg)
    {
      CollectOptions (cfg, OptionList);
      cfg->DecRef ();
    }
  }

  bool retcode = true;

  for (i = 1; i < argc; i++)
    if (!ParseArg (argc, argv, i))
    {
      retcode = false;
      break;
    }

  CHK (delete OptionList);
  OptionList = NULL;
  return retcode;
}

bool csSystemDriver::ParseArg (int argc, char* argv[], int& i)
{
  if (strcasecmp ("-help", argv[i]) == 0)
  {
    Help ();
    exit (0);
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
    csColOption* opt = OptionList;
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

void csSystemDriver::Help (iConfig* piConf)
{
  csOptionDescription option;
  int num = piConf->GetOptionCount ();
  for (int i = 0 ; i < num ; i++)
  {
    csVariant def;
    piConf->GetOptionDescription (i, &option);
    char opt [30], desc [80];
    piConf->GetOption (i, &def);
    switch (option.type)
    {
      case CSVAR_BOOL:
        sprintf (opt, "  -%s/no%s", option.name, option.name);
	sprintf (desc, "%s (%s) ", option.description, def.v.bVal ? "yes" : "no");
	break;
      case CSVAR_CMD:
        sprintf (opt, "  -%s", option.name);
	strcpy (desc, option.description);
	break;
      case CSVAR_FLOAT:
        sprintf (opt, "  -%s <val>", option.name);
	sprintf (desc, "%s (%f)", option.description, def.v.fVal);
	break;
      case CSVAR_LONG:
        sprintf (opt, "  -%s <val>", option.name);
	sprintf (desc, "%s (%ld)", option.description, def.v.lVal);
	break;
    }
    Printf (MSG_STDOUT, "%-21s%s\n", opt, desc);
  }
}

void csSystemDriver::Help ()
{
  for (int i = 0; i < PlugIns.Length (); i++)
  {
    csPlugIn *plugin = (csPlugIn *)PlugIns [i];
    iConfig *cfg = QUERY_INTERFACE (plugin->PlugIn, iConfig);
    if (cfg)
    {
      Printf (MSG_STDOUT, "Options for %s:\n", scfGetClassDescription (plugin->ClassID));
      Help (cfg);
      cfg->DecRef ();
    }
  }

  Printf (MSG_STDOUT, "General options:\n");
  Printf (MSG_STDOUT, "  -help              this help\n");
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
  console_out (msg);
  debug_out (true, msg);
}

void csSystemDriver::Warn (const char* msg)
{
  console_out (msg);
  debug_out (true, msg);
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
        console_out (buf);
        debug_out (true, buf);
      }
      break;

    case MSG_WARNING:
      if (System)
        System->Warn (buf);
      else
      {
        console_out (buf);
        debug_out (true, buf);
      }
      break;

    case MSG_INITIALIZATION:
      console_out (buf);
      debug_out (true, buf);
      if (System->ConsoleReady)
        System->DemoWrite (buf);
      break;

    case MSG_CONSOLE:
      if (System && System->Console)
        System->Console->PutText ("%s", buf);
      else
        console_out (buf);
      break;

    case MSG_STDOUT:
      console_out (buf);
      break;

    case MSG_DEBUG_0:
      debug_out (false, buf);
      break;

    case MSG_DEBUG_1:
      if (debug_level >= 1)
        debug_out (false, buf);
      break;

    case MSG_DEBUG_2:
      if (debug_level >= 2)
        debug_out (false, buf);
      break;

    case MSG_DEBUG_0F:
      debug_out (true, buf);
      break;

    case MSG_DEBUG_1F:
      if (debug_level >= 1)
        debug_out (true, buf);
      break;

    case MSG_DEBUG_2F:
      if (debug_level >= 2)
        debug_out (true, buf);
      break;
  } /* endswitch */
}

void csSystemDriver::DemoWrite (const char* buf)
{
  if (Console)
  {
    bool const ok2d = (!!G2D && G2D->BeginDraw());
    if (ok2d)
      G2D->Clear (0);

    Console->PutText ("%s", buf);
    csRect area;
    Console->Print (&area);

    if (area.xmax >= FrameWidth)
      area.xmax = FrameWidth-1;
    if (area.ymax >= FrameHeight)
      area.ymax = FrameHeight-1;

    if (ok2d)
    {
      G2D->FinishDraw ();
      G2D->Print (&area);
    }
  }
}

void csSystemDriver::debug_out (bool flush, const char *str)
{
  static FILE *f = NULL;
  if (!f)
    f = fopen ("debug.txt", "a+");
  if (f)
  {
    fputs (str, f);
    if (flush)
      fflush (f);
  }
}

//--------------------------------- iSystem interface for csSystemDriver -----//

void csSystemDriver::GetSettings (int &oWidth, int &oHeight, int &oDepth,
  bool &oFullScreen)
{
  oWidth = FrameWidth;
  oHeight = FrameHeight;
  oDepth = Depth;
  oFullScreen = FullScreen;
}

bool csSystemDriver::RegisterDriver (const char *iInterface, iPlugIn *iObject)
{
#define CHECK(Else, Object, Interface)				\
  Else if (!Object && strcmp (iInterface, #Interface) == 0)	\
    rc = !!(Object = QUERY_INTERFACE (iObject, Interface));

  bool rc = false;
  CHECK (;   , VFS,    iVFS)
  CHECK (else, G3D,    iGraphics3D)
  CHECK (else, G2D,    iGraphics2D)
  CHECK (else, Sound,  iSoundRender)
  CHECK (else, NetDrv, iNetworkDriver)
  CHECK (else, NetMan, iNetworkManager)
  return rc;

#undef CHECK
}

bool csSystemDriver::DeregisterDriver (const char *iInterface, iPlugIn *iObject)
{
#define CHECK(Else, Object, Interface)				\
  Else if (strcmp (iInterface, #Interface) == 0)		\
  {								\
    Interface *p = QUERY_INTERFACE (iObject, Interface);	\
    if (!p) return false;					\
    if (p != Object) { p->DecRef (); return false; }		\
    p->DecRef (); Object->DecRef (); Object = NULL;		\
    return true;						\
  }

  CHECK (;   , VFS,    iVFS)
  CHECK (else, G3D,    iGraphics3D)
  CHECK (else, G2D,    iGraphics2D)
  CHECK (else, Sound,  iSoundRender)
  CHECK (else, NetDrv, iNetworkDriver)
  CHECK (else, NetMan, iNetworkManager)
  return false;

#undef CHECK
}

iBase *csSystemDriver::LoadPlugIn (const char *iClassID, const char *iInterface, int iVersion)
{
  iPlugIn *p = CREATE_INSTANCE (iClassID, iPlugIn);
  if (!p)
    Printf (MSG_WARNING, "WARNING: could not load plugin `%s'\n", iClassID);
  else
  {
    PlugIns.Push (new csPlugIn (p, iClassID));
    if (p->Initialize (this))
    {
      iBase *ret;
      if (iInterface)
        ret = (iBase *)p->QueryInterface (iInterface, iVersion);
      else
        ret = p;
      if (ret)
        return p;
    }
    Printf (MSG_WARNING, "WARNING: failed to initialize plugin `%s'\n", iClassID);
    PlugIns.Delete (PlugIns.Length () - 1);
  }
  return NULL;
}

iBase *csSystemDriver::QueryPlugIn (const char *iInterface, int iVersion)
{
  for (int i = 0; i < PlugIns.Length (); i++)
  {
    iBase *ret = (iBase *)((csPlugIn *)PlugIns.Get (i))->PlugIn->
      QueryInterface (iInterface, iVersion);
    if (ret)
      return ret;
  }
  return NULL;
}

bool csSystemDriver::UnloadPlugIn (iPlugIn *iObject)
{
  int idx = PlugIns.FindKey (iObject);
  if (idx < 0)
    return false;

  DeregisterDriver ("iVFS", iObject);
  DeregisterDriver ("iGraphics3D", iObject);
  DeregisterDriver ("iGraphics2D", iObject);
  DeregisterDriver ("iSoundRender", iObject);
  DeregisterDriver ("iNetworkDriver", iObject);
  DeregisterDriver ("iNetworkManager", iObject);

  return PlugIns.Delete (idx);
}

bool csSystemDriver::CallOnEvents (iPlugIn *iObject, unsigned int iEventMask)
{
  int idx = PlugIns.FindKey (iObject);
  if (idx < 0)
    return false;

  csPlugIn *plugin = (csPlugIn *)PlugIns.Get (idx);
  plugin->EventMask = iEventMask;
  return true;
}

void csSystemDriver::Print (int mode, const char *string)
{
  Printf (mode, "%s", string);
}

time_t csSystemDriver::GetTime ()
{
  return Time ();
}

void csSystemDriver::StartShutdown ()
{
  Shutdown = true;
}

bool csSystemDriver::GetShutdown ()
{
  return Shutdown;
}

int csSystemDriver::ConfigGetInt (char *Section, char *Key, int Default)
{
  return Config->GetInt (Section, Key, Default);
}

char *csSystemDriver::ConfigGetStr (char *Section, char *Key, char *Default)
{
  return Config->GetStr (Section, Key, Default);
}

bool csSystemDriver::ConfigGetYesNo (char *Section, char *Key, bool Default)
{
  return Config->GetYesNo (Section, Key, Default);
}

float csSystemDriver::ConfigGetFloat (char *Section, char *Key, float Default)
{
  return Config->GetFloat (Section, Key, Default);
}

bool csSystemDriver::ConfigSetInt (char *Section, char *Key, int Value)
{
  return Config->SetInt (Section, Key, Value);
}

bool csSystemDriver::ConfigSetStr (char *Section, char *Key, char *Value)
{
  return Config->SetStr (Section, Key, Value);
}

bool csSystemDriver::ConfigSetFloat (char *Section, char *Key, float Value)
{
  return Config->SetFloat (Section, Key, Value);
}

bool csSystemDriver::ConfigSave ()
{
  return ConfigName ? Config->Save (ConfigName) : false;
}

void csSystemDriver::QueueKeyEvent (int KeyCode, bool Down)
{
  if (!KeyCode)
    return;

  time_t time = Time ();

  if (Down)
    Keyboard->do_keypress (time, KeyCode);
  else
    Keyboard->do_keyrelease (time, KeyCode);
}

void csSystemDriver::QueueMouseEvent (int Button, int Down, int x, int y, int ShiftFlags)
{
  time_t time = Time ();

  if (Button == 0)
    Mouse->do_mousemotion (time, x, y);
  else if (Down)
    Mouse->do_buttonpress (time, Button, x, y, ShiftFlags & CSMASK_SHIFT,
      ShiftFlags & CSMASK_ALT, ShiftFlags & CSMASK_CTRL);
  else
    Mouse->do_buttonrelease (time, Button, x, y);
}

void csSystemDriver::QueueFocusEvent (bool Enable)
{
  if (!EventQueue)
    return;
  if (Enable == (int)IsFocused)
    return;
  IsFocused = Enable;

  CHK (EventQueue->Put (new csEvent (Time(), csevBroadcast, cscmdFocusChanged,
    (void *)Enable)));
  if (!Enable)
  {
    Keyboard->Reset ();
    Mouse->Reset ();
  }
}

bool csSystemDriver::GetKeyState (int key)
{
  return Keyboard->GetKeyState (key);
}

bool csSystemDriver::GetMouseButton (int button)
{
  return Mouse->Button [button];
}

void csSystemDriver::GetMousePosition (int &x, int &y)
{
  x = Mouse->GetLastX ();
  y = Mouse->GetLastY ();
}
