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

#define SYSDEF_ALLOCA
#include "sysdef.h"
#include "cssys/system.h"
#include "cssys/sysdriv.h"
#include "csutil/csrect.h"
#include "csutil/util.h"
#include "csutil/inifile.h"
#include "csutil/vfs.h"
#include "csinput/csinput.h"
#include "iplugin.h"
#include "iconsole.h"
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
  CHK (delete [] ClassID);
  PlugIn->DecRef ();
}

/// A private class used to keep a list of plugins
class csPluginList : public csStrVector
{
public:
  csPluginList () : csStrVector (8, 8) {}
  bool Sort (csSystemDriver *iSys);
private:
  bool RecurseSort (csSystemDriver *iSys, int row, char *order, char *loop, bool *matrix);
};

/**
 * Since every plugin can depend on another one, the plugin loader should be
 * able to sort them by their preferences. Thus, if some plugin A wants some
 * other plugins B and C to be loaded before him, the plugin loader should
 * sort the list of loaded plugins such that plugin A comes after B and C.
 * <p>
 * Of course it is possible that some plugin A depends on B and B depends on A,
 * or even worse A on B, B on C and C on A. The sort algorithm should detect
 * this case and type an error message if it is detected.
 * <p>
 * The alogorithm works as follows. First, a dependency matrix is built. Here
 * is a example of a simple dependency matrix:
 * <pre>
 *                iWorld       iVFS     iGraphics3D iGraphics2D
 *             +-----------+-----------+-----------+-----------+
 * iWorld      |           |     X     |     X     |     X     |
 *             +-----------+-----------+-----------+-----------+
 * iVFS        |           |           |           |           |
 *             +-----------+-----------+-----------+-----------+
 * iGraphics3D |           |     X     |           |     X     |
 *             +-----------+-----------+-----------+-----------+
 * iGraphics2D |           |     X     |           |           |
 *             +-----------+-----------+-----------+-----------+
 * </pre>
 * Thus, we see that the iWorld plugin depends on iVFS, iGraphics3D and
 * iGraphics2D plugins (this is an abstract example, in reality the
 * things are simpler), iVFS does not depend on anything, iGraphics3D
 * wants the iVFS and the iGraphics2D plugins, and finally iGraphics2D
 * wants just the iVFS.
 * <p>
 * The sort algorithm works as follows: we take each plugin, one by one
 * starting from first (iWorld) and examine each of them. If some plugin
 * depends on others, we recursively launch this algorithm on those plugins.
 * If we don't have any more dependencies, we put the plugin into the
 * load list and return to the previous recursion level. To detect loops
 * we need to maintain an "recurse list", thus if we found that iWorld
 * depends on iGraphics3D, iGraphics3D depends on iGraphics2D and we're
 * examining iGraphics2D for dependencies, we have the following
 * loop-detection array: iWorld, iGraphics3D, iGraphics2D. If we find that
 * iGraphics2D depends on anyone that is in the loop array, we found a loop.
 * If we find that the plugin depends on anyone that is already in the load
 * list, its not a loop but just an already-fullfilled dependency.
 * Thus, the above table will be traversed this way (to the left is the
 * load list, to the right is the loop detection list):
 * <pre><ol>
 *   <li> []                                    [iWorld]
 *   <li> []                                    [iWorld,iVFS]
 *   <li> [iVFS]                                [iWorld]
 *   <li> [iVFS]                                [iWorld,iGraphics3D]
 *   <li> [iVFS]                                [iWorld,iGraphics3D,iGraphics2D]
 *   <li> [iVFS,iGraphics2D]                    [iWorld,iGraphics3D]
 *   <li> [iVFS,iGraphics2D,iGraphics3D]        [iWorld]
 *   <li> [iVFS,iGraphics2D,iGraphics3D,iWorld] []
 * </ol></pre>
 * In this example we traversed all plugins in one go. If we didn't, we
 * just take the next one (iWorld, iVFS, iGraphics3D, iGraphics2D) and if
 * it is not already in the load list, recursively traverse it.
 */
bool csPluginList::Sort (csSystemDriver *iSys)
{
  int row, col, len = Length ();

  // We'll use char for speed reasons
  if (len > 255)
  {
    iSys->Printf (MSG_FATAL_ERROR, "PLUGIN LOADER: Too many plugins requested (%d, max 255)\n", len);
    return false;
  }

  // Build the dependency matrix
  bool *matrix = (bool *)alloca (len * len * sizeof (bool));
  memset (matrix, 0, len * len * sizeof (bool));
  for (row = 0; row < len; row++)
  {
    const char *dep = scfGetClassDependencies ((char *)Get (row));
    while (dep && *dep)
    {
      char tmp [100];
      const char *comma = strchr (dep, ',');
      if (!comma)
        comma = strchr (dep, 0);
      size_t sl = comma - dep;
      if (sl >= sizeof (tmp))
        sl = sizeof (tmp) - 1;
      memcpy (tmp, dep, sl);
      while (sl && ((tmp [sl - 1] == ' ') || (tmp [sl - 1] == '\t')))
        sl--;
      tmp [sl] = 0;
      if (!sl)
        break;
      bool wildcard = tmp [sl - 1] == '.';
      for (col = 0; col < len; col++)
        if ((col != row)
         && (wildcard ? strncmp (tmp, (char *)Get (col), sl) :
             strcmp (tmp, (char *)Get (col))) == 0)
          matrix [row * len + col] = true;
      dep = comma;
      while (*dep == ',' || *dep == ' ' || *dep == '\t')
        dep++;
    }
  }

  // Go through dependency matrix and put all plugins into an array
  bool error = false;
  char *order = (char *)alloca (len + 1);
  *order = 0;
  char *loop = (char *)alloca (len + 1);
  *loop = 0;

  for (row = 0; row < len; row++)
    if (!RecurseSort (iSys, row, order, loop, matrix))
      error = true;

  // Reorder plugin list according to "order" array
  csSome *newroot = (csSome *)malloc (len * sizeof (csSome));
  for (row = 0; row < len; row++)
    newroot [row] = root [order [row] - 1];
  free (root); root = newroot;

  return !error;
}

bool csPluginList::RecurseSort (csSystemDriver *iSys, int row, char *order,
  char *loop, bool *matrix)
{
  // If the plugin is already in the load list, skip it
  if (strchr (order, row + 1))
    return true;

  int len = Length ();
  bool *dep = matrix + row * len;
  bool error = false;
  char *loopp = strchr (loop, 0);
  *loopp++ = row + 1; *loopp = 0;
  for (int col = 0; col < len; col++)
    if (*dep++)
    {
      // If the plugin is already loaded, skip
      if (strchr (order, col + 1))
        continue;

      char *already = strchr (loop, col + 1);
      if (already)
      {
        iSys->Printf (MSG_FATAL_ERROR, "PLUGIN LOADER: Cyclic dependency detected!\n");
        int startx = int (already - loop);
        for (int x = startx; loop [x]; x++)
          iSys->Printf (MSG_FATAL_ERROR, "   %s %s\n",
            x == startx ? "+->" : loop [x + 1] ? "| |" : "<-+",
            (char *)Get (loop [x] - 1));
        error = true;
        break;
      }

      bool recurse_error = !RecurseSort (iSys, col, order, loop, matrix);

      // Drop recursive loop dependency since it has been already moved to order
      *loopp = 0;

      if (recurse_error)
      {
        error = true;
        break;
      }
    }

  // Put current plugin into the list
  char *orderp = strchr (order, 0);
  *orderp++ = row + 1; *orderp = 0;

  return !error;
}

//---------------------------------------------------- The System Driver -----//

csSystemDriver::csSystemDriver () : PlugIns (8, 8), OptionList (16, 16),
  CommandLine (16, 16), CommandLineNames (16, 16)
{
  console_open ();
  System = this;
  IsFocused = true;
  FullScreen = false;
  Mouse = NULL;
  Keyboard = NULL;

  VFS = NULL;
  G3D = NULL;
  G2D = NULL;
  NetDrv = NULL;
  NetMan = NULL;
  Sound = NULL;

  EventQueue = NULL;
  Console = NULL;
  ConfigName = NULL;

  scf=NULL;
}

csSystemDriver::~csSystemDriver ()
{
#ifdef DEBUG
  printf ("System driver is going to shut down now!\n");
#endif

  Close ();

  CHKB (delete Config);
  CHKB (delete [] ConfigName);
  // Free all plugin options (also decrefs their iConfig interfaces)
  OptionList.DeleteAll ();

  System = NULL;

  // Deregister all known drivers
#define DEREGISTER_DRIVER(Object, Interface)			\
  if (Object)							\
  {								\
    iPlugIn *plugin = QUERY_INTERFACE (Object, iPlugIn);	\
    if (plugin)							\
    {								\
      DeregisterDriver (#Interface, plugin);			\
      plugin->DecRef ();					\
    }								\
  }

  DEREGISTER_DRIVER (VFS, iVFS);
  DEREGISTER_DRIVER (G3D, iGraphics3D);
  DEREGISTER_DRIVER (G2D, iGraphics2D);
  DEREGISTER_DRIVER (Sound, iSoundRender);
  DEREGISTER_DRIVER (NetDrv, iNetworkDriver);
  DEREGISTER_DRIVER (NetMan, iNetworkManager);
  DEREGISTER_DRIVER (Console, iConsole);

#undef DEREGISTER_DRIVER

  // Free all plugins
  PlugIns.DeleteAll ();

  CHK (delete Mouse);
  CHK (delete Keyboard);
  CHK (delete EventQueue);

//TODO Azverkan this should be a scf->Release()
  if(scf)
    CHK(delete scf);

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
    CHKB (Config = new csIniFile (ConfigName = strnew (iConfigName)))
  else
    CHKB (Config = new csIniFile ());

  // Initialize Shared Class Facility
  CHK (csIniFile *scfconfig = new csIniFile ("scf.cfg"));
  scfInitialize (scfconfig);
  CHK (delete scfconfig);

  // Create the Event Queue
  CHK (EventQueue = new csEventQueue ());

  // Initialize keyboard and mouse
  InitKeyboard ();
  InitMouse ();

  // Collect all options from command line
  CollectOptions (argc, argv);

  // Analyse config and command line
  SetSystemDefaults (Config);

  // The list of plugins
  csPluginList PluginList;

  // Now eat all common-for-plugins command-line switches
  static const char g3d_str[] = "crystalspace.graphics3d.";
  const int g3d_len = sizeof(g3d_str) / sizeof(g3d_str[0]) - 1;
  bool g3d_override = false;

  const char *val;
  if ((val = GetOptionCL ("video")))
  {
    // Alternate videodriver
    char temp [100];
    sprintf (temp, "crystalspace.graphics3d.%s", val);
    Printf (MSG_INITIALIZATION, "Using alternative 3D driver: %s\n", temp);
    PluginList.Push (strnew (temp));
    g3d_override = true;
  }

  // Eat all --plugin switches specified on the command line
  int n = 0;
  while ((val = GetOptionCL ("plugin", n++)))
    PluginList.Push (strnew (val));

  // Now load and initialize all plugins
  n = PluginList.Length ();
  Config->EnumData ("PlugIns", &PluginList);
  while (n < PluginList.Length ())
  {
    const char *classID = Config->GetStr ("PlugIns", (char *)PluginList.Get (n));
    // If -video was used to override 3D driver, then respect it.
    if (g3d_override && strncmp(classID, g3d_str, g3d_len) == 0)
      PluginList.Delete(n);
    else
    {
      PluginList.Replace (n, strnew (classID));
      n++;
    }
  }

  // Sort all plugins by their dependency lists
  if (!PluginList.Sort (this))
    return false;

  // Load all plugins
  for (n = 0; n < PluginList.Length (); n++)
    LoadPlugIn ((char *)PluginList.Get (n), NULL, 0);

  // See if user wants help
  if ((val = GetOptionCL ("help")))
  {
    Help ();
    exit (0);
  }

  // Check if the minimal required drivers are loaded
  if (!CheckDrivers ())
    return false;

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

void csSystemDriver::Close ()
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
  CHK (Keyboard = new SysKeyboardDriver ());
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

void csSystemDriver::CollectOptions (int argc, char *argv[])
{
  for (int i = 1; i < argc; i++)
  {
    char *opt = argv [i];
    if (*opt == '-')
    {
      while (*opt == '-') opt++;
      opt = strnew (opt);
      char *arg = strchr (opt, '=');
      if (arg) *arg++ = 0; else arg = opt + strlen (opt);
      CHK (CommandLine.Push (new csCommandLineOption (opt, arg)));
    }
    else
      CommandLineNames.Push (strnew (opt));
  }
}

void csSystemDriver::SetSystemDefaults (csIniFile *Config)
{
  // First look in .cfg file
  FrameWidth = Config->GetInt ("VideoDriver", "Width", 640);
  FrameHeight = Config->GetInt ("VideoDriver", "Height", 480);
  Depth = Config->GetInt ("VideoDriver", "Depth", 16);
  FullScreen = Config->GetYesNo ("VideoDriver", "FullScreen", false);

  // Now analyze command line
  const char *val;
  if ((val = GetOptionCL ("mode")))
    SetMode (val);

  if ((val = GetOptionCL ("depth")))
    Depth = atoi (val);
}

void csSystemDriver::Help (iConfig* Config)
{
  for (int i = 0; ; i++)
  {
    csOptionDescription option;
    if (!Config->GetOptionDescription (i, &option))
      break;
    char opt [30], desc [80];
    csVariant def;
    Config->GetOption (i, &def);
    switch (option.type)
    {
      case CSVAR_BOOL:
        sprintf (opt, "  -[no]%s", option.name);
	sprintf (desc, "%s (%s) ", option.description, def.v.b ? "yes" : "no");
	break;
      case CSVAR_CMD:
        sprintf (opt, "  -%s", option.name);
	strcpy (desc, option.description);
	break;
      case CSVAR_FLOAT:
        sprintf (opt, "  -%s=<val>", option.name);
	sprintf (desc, "%s (%f)", option.description, def.v.f);
	break;
      case CSVAR_LONG:
        sprintf (opt, "  -%s=<val>", option.name);
	sprintf (desc, "%s (%ld)", option.description, def.v.l);
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
    iConfig *Config = QUERY_INTERFACE (plugin->PlugIn, iConfig);
    if (Config)
    {
      Printf (MSG_STDOUT, "Options for %s:\n", scfGetClassDescription (plugin->ClassID));
      Help (Config);
      Config->DecRef ();
    }
  }

  Printf (MSG_STDOUT, "General options:\n");
  Printf (MSG_STDOUT, "  -help              this help\n");
  Printf (MSG_STDOUT, "  -mode=<w>x<y>      set resolution (default=%dx%d)\n", FrameWidth, FrameHeight);
  Printf (MSG_STDOUT, "  -depth=<d>         set depth (default=%d bpp)\n", Depth);
  Printf (MSG_STDOUT, "  -video=<s>         the 3D driver (opengl, glide, software, ...)\n");
  Printf (MSG_STDOUT, "  -plugin=<s>        load the plugin after all others\n");
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
        System->Console->PutText (buf);
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
    bool const ok2d = ((G2D!=NULL) && G2D->BeginDraw());
    if (ok2d)
      G2D->Clear (0);

    Console->PutText (buf);
    csRect area;
    Console->Draw (&area);

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

void csSystemDriver::QueryOptions (iPlugIn *iObject)
{
  iConfig *Config = QUERY_INTERFACE (iObject, iConfig);
  if (Config)
  {
    int on = OptionList.Length ();
    for (int i = 0 ; ; i++)
    {
      csOptionDescription option;
      if (!Config->GetOptionDescription (i, &option))
        break;
      CHK (OptionList.Push (new csPluginOption (option.name, option.type, option.id,
        (option.type == CSVAR_BOOL) || (option.type == CSVAR_CMD), Config)));
      if (option.type == CSVAR_BOOL)
      {
        char buf[100];
        strcpy (buf, "no");
        strcpy (buf + 2, option.name);
        CHK (OptionList.Push (new csPluginOption (buf, option.type, option.id,
          false, Config)));
      }
    } /* endfor */

    // Parse the command line for plugin options and pass them to plugin
    for (; on < OptionList.Length (); on++)
    {
      csPluginOption *pio = (csPluginOption *)OptionList.Get (on);
      const char *val;
      if ((val = GetOptionCL (pio->Name)))
      {
        csVariant optval;
        optval.type = pio->Type;
        switch (pio->Type)
        {
          case CSVAR_BOOL:
          case CSVAR_CMD:
            optval.v.b = pio->Value;
            break;
          case CSVAR_LONG:
            if (!val) continue;
            optval.v.l = atol (val);
            break;
          case CSVAR_FLOAT:
            if (!val) continue;
            optval.v.f = atof (val);
            break;
        }
        pio->Config->SetOption (pio->ID, &optval);
      }
    }
    Config->DecRef ();
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
  CHECK (else, Console, iConsole)
  return rc;

#undef CHECK
}

bool csSystemDriver::DeregisterDriver (const char *iInterface, iPlugIn *iObject)
{
#define CHECK(Else, Object, Interface)					\
  Else if (strcmp (iInterface, #Interface) == 0)			\
  {									\
    Interface *p = QUERY_INTERFACE (iObject, Interface);		\
    if (!p) return false;						\
    if (p != Object) { p->DecRef (); return false; }			\
    p->DecRef (); iBase* t = Object; Object = NULL; t->DecRef ();	\
    return true;							\
  }

  CHECK (;   , VFS,    iVFS)
  CHECK (else, G3D,    iGraphics3D)
  CHECK (else, G2D,    iGraphics2D)
  CHECK (else, Sound,  iSoundRender)
  CHECK (else, NetDrv, iNetworkDriver)
  CHECK (else, NetMan, iNetworkManager)
  CHECK (else, Console, iConsole)
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
    CHK (PlugIns.Push (new csPlugIn (p, iClassID)));
    if (p->Initialize (this))
    {
      iBase *ret;
      if (iInterface)
        ret = (iBase *)p->QueryInterface (iInterface, iVersion);
      else
        ret = p;
      if (ret)
      {
        QueryOptions (p);
        return p;
      }
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

const char *csSystemDriver::GetOptionCL (const char *iName, int iIndex)
{
  int idx = CommandLine.FindKey (iName);
  if (idx >= 0)
  {
    while (iIndex)
    {
      idx++; 
      if (idx >= CommandLine.Length ())
        return NULL;
      if (CommandLine.CompareKey (CommandLine.Get (idx), iName, 0) == 0)
        iIndex--;
    }
    return ((csCommandLineOption *)CommandLine.Get (idx))->Value;
  }
  return NULL;
}

const char *csSystemDriver::GetNameCL (int iIndex)
{
  if ((iIndex >= 0) && (iIndex < CommandLineNames.Length ()))
    return (const char *)CommandLineNames.Get (iIndex);
  return NULL;
}

void csSystemDriver::AddOptionCL (const char *iName, const char *iValue)
{
  CHK (CommandLine.Push (new csCommandLineOption (strnew (iName), strnew (iValue))));
}

void csSystemDriver::AddNameCL (const char *iName)
{
  CommandLineNames.Push (strnew (iName));
}

iSCF* csSystemDriver::GetSCF() {
  if(!scf)
    CHK(scf=new csSCF());
  return scf;
}
