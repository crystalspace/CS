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
#define SYSDEF_PATH
#include "cssysdef.h"
#include "cssys/system.h"
#include "cssys/sysdriv.h"
#include "cssys/csinput.h"
#include "cssys/csshlib.h"
#include "csutil/csrect.h"
#include "csutil/util.h"
#include "csutil/inifile.h"
#include "csutil/cfgfile.h"
#include "iplugin.h"
#include "ivfs.h"
#include "iconsole.h"
#include "isndrdr.h"
#include "inetdrv.h"
#include "iconfig.h"
#include "igraph3d.h"
#include "igraph2d.h"
#include "imotion.h"

// The global system variable
csSystemDriver *System = NULL;

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

//------------------------------------------------------- csPlugIn class -----//

csSystemDriver::csPlugIn::csPlugIn (iPlugIn *iObject, const char *iClassID,
  const char *iFuncID)
{
  PlugIn = iObject;
  ClassID = strnew (iClassID);
  FuncID = strnew (iFuncID);
  EventMask = 0;
}

csSystemDriver::csPlugIn::~csPlugIn ()
{
  delete [] ClassID;
  delete [] FuncID;
  PlugIn->DecRef ();
}

//----------------------- A private class used to keep a list of plugins -----//

struct csPluginLoadRec
{
  char *FuncID;
  char *ClassID;

  csPluginLoadRec (const char *iFuncID, const char *iClassID)
  { FuncID = strnew (iFuncID); ClassID = strnew (iClassID); }
  ~csPluginLoadRec ()
  { delete [] ClassID; delete [] FuncID; }
};

class csPluginList : public csVector
{
public:
  virtual ~csPluginList ()
  { DeleteAll (); }
  bool Sort (csSystemDriver *iSys);
  csPluginLoadRec &Get (int idx)
  { return *(csPluginLoadRec *)csVector::Get (idx); }
  virtual bool FreeItem (csSome Item)
  { delete (csPluginLoadRec *)Item; return true; }
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
 *                iEngine      iVFS     iGraphics3D iGraphics2D
 *             +-----------+-----------+-----------+-----------+
 * iEngine     |           |     X     |     X     |     X     |
 *             +-----------+-----------+-----------+-----------+
 * iVFS        |           |           |           |           |
 *             +-----------+-----------+-----------+-----------+
 * iGraphics3D |           |     X     |           |     X     |
 *             +-----------+-----------+-----------+-----------+
 * iGraphics2D |           |     X     |           |           |
 *             +-----------+-----------+-----------+-----------+
 * </pre>
 * Thus, we see that the iEngine plugin depends on iVFS, iGraphics3D and
 * iGraphics2D plugins (this is an abstract example, in reality the
 * things are simpler), iVFS does not depend on anything, iGraphics3D
 * wants the iVFS and the iGraphics2D plugins, and finally iGraphics2D
 * wants just the iVFS.
 * <p>
 * The sort algorithm works as follows: we take each plugin, one by one
 * starting from first (iEngine) and examine each of them. If some plugin
 * depends on others, we recursively launch this algorithm on those plugins.
 * If we don't have any more dependencies, we put the plugin into the
 * load list and return to the previous recursion level. To detect loops
 * we need to maintain an "recurse list", thus if we found that iEngine
 * depends on iGraphics3D, iGraphics3D depends on iGraphics2D and we're
 * examining iGraphics2D for dependencies, we have the following
 * loop-detection array: iEngine, iGraphics3D, iGraphics2D. If we find that
 * iGraphics2D depends on anyone that is in the loop array, we found a loop.
 * If we find that the plugin depends on anyone that is already in the load
 * list, its not a loop but just an already-fullfilled dependency.
 * Thus, the above table will be traversed this way (to the left is the
 * load list, to the right is the loop detection list):
 * <pre><ol>
 *   <li> []                                     [iEngine]
 *   <li> []                                     [iEngine,iVFS]
 *   <li> [iVFS]                                 [iEngine]
 *   <li> [iVFS]                                 [iEngine,iGraphics3D]
 *   <li> [iVFS]                                 [iEngine,iGraphics3D,iGraphics2D]
 *   <li> [iVFS,iGraphics2D]                     [iEngine,iGraphics3D]
 *   <li> [iVFS,iGraphics2D,iGraphics3D]         [iEngine]
 *   <li> [iVFS,iGraphics2D,iGraphics3D,iEngine] []
 * </ol></pre>
 * In this example we traversed all plugins in one go. If we didn't, we
 * just take the next one (iEngine, iVFS, iGraphics3D, iGraphics2D) and if
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
    const char *dep = scfGetClassDependencies (Get (row).ClassID);
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
         && (wildcard ? strncmp (tmp, Get (col).ClassID, sl) :
             strcmp (tmp, Get (col).ClassID)) == 0)
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
            Get (loop [x] - 1).ClassID);
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

IMPLEMENT_IBASE (csSystemDriver)
  IMPLEMENTS_INTERFACE (iSystem)
  IMPLEMENTS_EMBEDDED_INTERFACE (iSCF)
IMPLEMENT_IBASE_END

csSystemDriver::csSystemDriver () : PlugIns (8, 8), EventQueue (),
  OptionList (16, 16), CommandLine (16, 16), CommandLineNames (16, 16)
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiSCF);

  Keyboard.SetSystemDriver (this);
  Mouse.SetSystemDriver    (this);
  Joystick.SetSystemDriver (this);

  // Create the default system event outlet
  EventOutlets.Push (new csEventOutlet (NULL, this));

  System = this;
  FullScreen = false;

  VFS = NULL;
  G3D = NULL;
  G2D = NULL;
  NetDrv = NULL;
  Sound = NULL;
  MotionMan = NULL;

  Console = NULL;

  debug_level = 0;
  Shutdown = false;
  CurrentTime = cs_time (-1);

  ExitLoop = false;
}

csSystemDriver::~csSystemDriver ()
{
#ifdef CS_DEBUG
  printf ("System driver is going to shut down now!\n");
#endif

  Close ();

  // Free all plugin options (also decrefs their iConfig interfaces)
  OptionList.DeleteAll ();

  System = NULL;

  // Deregister all known drivers and plugins
  if (Console) Console->DecRef ();
  if (NetDrv) NetDrv->DecRef ();
  if (Sound) Sound->DecRef ();
  if (G2D) G2D->DecRef ();
  if (G3D) G3D->DecRef ();
  if (VFS) VFS->DecRef ();
  if (Config) Config->DecRef ();
  if (MotionMan) MotionMan->DecRef();

  // Free all plugins
  PlugIns.DeleteAll ();
  // Free the system event outlet
  EventOutlets.DeleteAll ();

  scfFinish ();
}

bool csSystemDriver::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  // Initialize Shared Class Facility|
  char scfconfigpath [MAXPATHLEN + 1];

#ifndef CS_STATIC_LINKED
  // Add both installpath and installpath/lib dirs to search for plugins
  GetInstallPath (scfconfigpath, sizeof (scfconfigpath));
  csAddLibraryPath (scfconfigpath);
  strcat (scfconfigpath, "lib");   
  int scfconfiglen = strlen(scfconfigpath);
  scfconfigpath[scfconfiglen] = PATH_SEPARATOR;
  scfconfigpath[scfconfiglen+1] = 0;
  csAddLibraryPath (scfconfigpath);
#endif

  // Find scf.cfg and initialize SCF
  GetInstallPath (scfconfigpath, sizeof (scfconfigpath));
  strcat (scfconfigpath, "scf.cfg");
  csConfigFile scfconfig (scfconfigpath);
  scfInitialize (&scfconfig);

  // @@@ This is ugly.  We need a better, more generalized way of doing this.
  // Hard-coding the name of the VFS plugin (crytalspace.kernel.vfs) is bad.
  // Then later ensuring that we skip over this same plugin when requested
  // by the client is even uglier.  The reason that the VFS plugin is required
  // this early is that both the application configuration file and the
  // configuration file for other plugins may (and almost always do) reside on
  // a VFS volume.
  Config = new csIniFile;
  VFS = LOAD_PLUGIN (this, "crystalspace.kernel.vfs", CS_FUNCID_VFS, iVFS);

  // Initialize configuration file
  if (iConfigName)
    if (!Config->Load (iConfigName, VFS))
      Printf (MSG_WARNING,
	"WARNING: Failed to load configuration file `%s'\n", iConfigName);

  // Collect all options from command line
  CollectOptions (argc, argv);

  // Analyse config and command line
  SetSystemDefaults (Config);

  // The list of plugins
  csPluginList PluginList;

  // Now eat all common-for-plugins command-line switches
  bool g3d_override = false;

  const char *val;
  if ((val = GetOptionCL ("video")))
  {
    // Alternate videodriver
    char temp [100];
    sprintf (temp, "crystalspace.graphics3d.%s", val);
    Printf (MSG_INITIALIZATION, "Using alternative 3D driver: %s\n", temp);
    PluginList.Push (new csPluginLoadRec (CS_FUNCID_VIDEO, temp));
    g3d_override = true;
  }
  if ((val = GetOptionCL ("canvas")))
  {
    char temp [100];
    if (!strchr (val, '.'))
    {
      sprintf (temp, "crystalspace.graphics2d.%s", val);
      ReplaceOptionCL ("canvas", temp);
    }
  }

  // Eat all --plugin switches specified on the command line
  int n = 0;
  while ((val = GetOptionCL ("plugin", n++)))
  {
    size_t sl = strlen (val);
    char temp [100];
    if (sl >= sizeof (temp)) sl = sizeof (temp) - 1;
    memcpy (temp, val, sl); temp [sl] = 0;
    char *func = strchr (temp, ':');
    if (func) *func++ = 0;
    PluginList.Push (new csPluginLoadRec (func, temp));
  }

  // Now load and initialize all plugins
  iConfigDataIterator *plugin_list = Config->EnumData ("PlugIns");
  if (plugin_list)
  {
    while (plugin_list->Next ())
    {
      const char *funcID = plugin_list->GetKey ();
      // If -video was used to override 3D driver, then respect it.
      if (g3d_override && strcmp (funcID, CS_FUNCID_VIDEO) == 0)
        continue;
      const char *classID = (const char *)plugin_list->GetData ();
      if (classID)
        PluginList.Push (new csPluginLoadRec (funcID, classID));
    }
    plugin_list->DecRef ();
  }

  // Sort all plugins by their dependency lists
  if (!PluginList.Sort (this))
    return false;

  // Load all plugins
  for (n = 0; n < PluginList.Length (); n++)
  {
    const csPluginLoadRec& r = PluginList.Get(n);
    // If plugin is VFS then skip if already loaded earlier.
    if (VFS && r.FuncID && strcmp (r.FuncID, CS_FUNCID_VFS) == 0)
      continue;
    LoadPlugIn (r.ClassID, r.FuncID, NULL, 0);
  }

  // See if user wants help
  if ((val = GetOptionCL ("help")))
  {
    Help ();
    exit (0);
  }

  /// Now find the drivers that are known by the system driver
  if (!VFS)
    VFS = QUERY_PLUGIN_ID (this, CS_FUNCID_VFS, iVFS);
  G3D = QUERY_PLUGIN_ID (this, CS_FUNCID_VIDEO, iGraphics3D);
  if (G3D)
    (G2D = G3D->GetDriver2D ())->IncRef ();
  else
    G2D = QUERY_PLUGIN_ID (this, CS_FUNCID_CANVAS, iGraphics2D);
  Sound = QUERY_PLUGIN_ID (this, CS_FUNCID_SOUND, iSoundRender);
  NetDrv = QUERY_PLUGIN_ID (this, CS_FUNCID_NETDRV, iNetworkDriver);
  Console = QUERY_PLUGIN_ID (this, CS_FUNCID_CONSOLE, iConsole);
  MotionMan = QUERY_PLUGIN_ID (this, CS_FUNCID_MOTION, iMotionManager);

  // Check if the minimal required drivers are loaded
  if (!CheckDrivers ())
    return false;

  return true;
}

bool csSystemDriver::Open (const char *Title)
{
  if ((G3D && !G3D->Open (Title))
   || (!G3D && G2D && !G2D->Open (Title)))
    return false;
  // Query frame width/height/depth as it possibly has been adjusted after open
  if (G2D)
  {
    FrameWidth = G2D->GetWidth ();
    FrameHeight = G2D->GetHeight ();
    Depth = G2D->GetPixelBytes ();
    Depth *= 8;
  }

  if (Sound)
    if (!Sound->Open ())
      return false;
  if (NetDrv)
    if (!NetDrv->Open ())
      return false;

  // Now pass the open event to all plugins
  csEvent Event (GetTime (), csevBroadcast, cscmdSystemOpen);
  HandleEvent (Event);

  return true;
}

void csSystemDriver::Close ()
{
  // Warn all plugins the system is going down
  csEvent Event (GetTime (), csevBroadcast, cscmdSystemClose);
  HandleEvent (Event);

  if (Sound)
    Sound->Close ();
  if (NetDrv)
    NetDrv->Close ();
  if (G3D)
    G3D->Close ();
}

bool csSystemDriver::CheckDrivers ()
{
  bool rc = true;
  if (!VFS)
  {
    Printf (MSG_FATAL_ERROR, "FATAL: No iVFS plug-in loaded!\n");
    rc = false;
  }
  if (!G3D)
  {
    Printf (MSG_FATAL_ERROR, "FATAL: No iGraphics3D plug-in loaded!\n");
    rc = false;
  }
  if (!G2D)
  {
    Printf (MSG_FATAL_ERROR, "FATAL: No iGraphics2D plug-in loaded!\n");
    rc = false;
  }

  return rc;
}

void csSystemDriver::NextFrame ()
{
  int i;

  // Update elapsed time first
  cs_time cur_time = Time ();
  ElapsedTime = (CurrentTime == cs_time (-1)) ? 0 : cur_time - CurrentTime;
  CurrentTime = cur_time;

  // See if any plugin wants to be called every frame
  for (i = 0; i < PlugIns.Length (); i++)
  {
    csPlugIn *plugin = PlugIns.Get (i);
    if (plugin->EventMask & CSMASK_Nothing)
    {
      csEvent Event (Time (), csevBroadcast, cscmdPreProcess);
      plugin->PlugIn->HandleEvent (Event);
    }
  }

  iEvent *ev;
  while ((ev = EventQueue.Get ()))
  {
    HandleEvent (*ev);
    ev->DecRef ();
  }

//@@@@@@@@@@@@@ TO DO @@@@@@@@@@@@@@@
// these plugins should really register for being called every
// frame - then they may do whatever they want inside their handleevent
// function
//@@@@@@@@@@@@@
  if (Sound)
    Sound->Update ();
//@@@@@@@@@@@@@@ END @@@@@@@@@@@@@@@@

  // If a plugin has set CSMASK_Nothing, it receives cscmdPostProcess events too
  for (i = 0; i < PlugIns.Length (); i++)
  {
    csPlugIn *plugin = PlugIns.Get (i);
    if (plugin->EventMask & CSMASK_Nothing)
    {
      csEvent Event (Time (), csevBroadcast, cscmdPostProcess);
      plugin->PlugIn->HandleEvent (Event);
    }
  }
}

void csSystemDriver::Loop ()
{
  while (!Shutdown && !ExitLoop)
    NextFrame ();
  ExitLoop = false;
}

bool csSystemDriver::HandleEvent (iEvent&Event)
{
  if (Event.Type == csevBroadcast)
    switch (Event.Command.Code)
    {
      case cscmdQuit:
        Shutdown = true;
        break;
      case cscmdQuitLoop:
        ExitLoop = true;
        break;
      case cscmdFocusChanged:
        // If user switches away from our application, reset
        // keyboard/mouse/joystick state
        if (!Event.Command.Info)
        {
          Keyboard.Reset ();
          Mouse.Reset ();
          Joystick.Reset ();
        }
        break;
    }

  int evmask = 1 << Event.Type;
  bool canstop = !(Event.Flags & CSEF_BROADCAST);
  for (int i = 0; i < PlugIns.Length (); i++)
  {
    csPlugIn *plugin = PlugIns.Get (i);
    if (plugin->EventMask & evmask)
      if (plugin->PlugIn->HandleEvent (Event) && canstop)
        return true;
  }

  return false;
}

void csSystemDriver::CollectOptions (int argc, const char* const argv[])
{
  for (int i = 1; i < argc; i++)
  {
    char *opt = (char *)argv [i];
    if (*opt == '-')
    {
      while (*opt == '-') opt++;
      char *arg = strchr (opt, '=');
      if (arg)
      {
        int n = arg - opt;
        char *newopt = new char [n + 1];
        memcpy (newopt, opt, n);
        (opt = newopt) [n] = 0;
        arg = strnew (arg + 1);
      }
      else
        opt = strnew (opt);
      CommandLine.Push (new csCommandLineOption (opt, arg));
    }
    else
      CommandLineNames.Push (strnew (opt));
  }
}

void csSystemDriver::SetSystemDefaults (iConfigFile *Config)
{
  // First look in .cfg file
  FrameWidth = Config->GetInt ("VideoDriver", "Width", 640);
  FrameHeight = Config->GetInt ("VideoDriver", "Height", 480);
  Depth = Config->GetInt ("VideoDriver", "Depth", 16);
  FullScreen = Config->GetYesNo ("VideoDriver", "FullScreen", false);

  // Now analyze command line
  const char *val;
  if ((val = GetOptionCL ("mode")))
  {
    int wres, hres;
    if (sscanf(val, "%dx%d", &wres, &hres) != 2)
    {
      Printf (MSG_INITIALIZATION, "Mode %s unknown : assuming '-mode %dx%d'\n", val,
        FrameWidth, FrameHeight);
    }
    else
    {
      FrameWidth = wres;
      FrameHeight = hres;
    }
  }

  if ((val = GetOptionCL ("depth")))
    Depth = atoi (val);

  if ((val = GetOptionCL ("fs")))
    if (!strcmp (val, "no"))
      FullScreen = false;
    else if (!val [0] || !strcmp (val, "yes"))
      FullScreen = true;
    else
      Printf (MSG_INITIALIZATION, "Unknown value `%s' for -fs switch: "
        "`yes' or `no' expected\n", val);

  Mouse.SetDoubleClickTime (
    Config->GetInt ("MouseDriver", "DoubleClickTime", 300),
    Config->GetInt ("MouseDriver", "DoubleClickDist", 2));
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
	sprintf (desc, "%s (%g)", option.description, def.v.f);
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
  csEvent HelpEvent (Time (), csevBroadcast, cscmdCommandLineHelp);
  for (int i = 0; i < PlugIns.Length (); i++)
  {
    csPlugIn *plugin = PlugIns.Get (i);
    iConfig *Config = QUERY_INTERFACE (plugin->PlugIn, iConfig);
    if (Config)
    {
      Printf (MSG_STDOUT, "Options for %s:\n", scfGetClassDescription (plugin->ClassID));
      Help (Config);
      Config->DecRef ();
    }
    plugin->PlugIn->HandleEvent (HelpEvent);
  }

  Printf (MSG_STDOUT, "General options:\n");
  Printf (MSG_STDOUT, "  -help              this help\n");
  Printf (MSG_STDOUT, "  -mode=<w>x<y>      set resolution (default=%dx%d)\n", FrameWidth, FrameHeight);
  Printf (MSG_STDOUT, "  -depth=<d>         set depth (default=%d bpp)\n", Depth);
  Printf (MSG_STDOUT, "  -[no]fs            use fullscreen videomode if available (default=%s)\n", FullScreen ? "yes" : "no");
  Printf (MSG_STDOUT, "  -video=<s>         the 3D rendering driver (opengl, glide, software, ...)\n");
  Printf (MSG_STDOUT, "  -canvas=<s>        the 2D canvas driver (asciiart, x2d, ...)\n");
  Printf (MSG_STDOUT, "  -plugin=<s>        load the plugin after all others\n");
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
      OptionList.Push (new csPluginOption (option.name, option.type, option.id,
        (option.type == CSVAR_BOOL) || (option.type == CSVAR_CMD), Config));
      if (option.type == CSVAR_BOOL)
      {
        char buf[100];
        strcpy (buf, "no");
        strcpy (buf + 2, option.name);
        OptionList.Push (new csPluginOption (buf, option.type, option.id,
          false, Config));
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

void csSystemDriver::RequestPlugin (const char *iPluginName)
{
  AddOptionCL ("plugin", iPluginName);
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

cs_time csSystemDriver::GetTime ()
{
  return Time ();
}

bool csSystemDriver::GetInstallPath (char *oInstallPath, size_t iBufferSize)
{
  char *path = getenv ("CRYSTAL");
  if (!path || !*path)
  {
    // Won't check here for the schisofrenic case when iBufferSize == 0;
    // Crystal Space is bloated enough without that...
    oInstallPath [0] = 0;
    return true;
  }

  size_t pl = strlen (path);
  // See if we have to add an ending path separator to the directory
  bool addsep = (path [pl - 1] != PATH_SEPARATOR)
#if defined (OS_DOS) || defined (OS_OS2) || defined (OS_WIN32)
             && (path [pl - 1] != '/')
#endif
    ;
  if (addsep)
    pl++;

  if (pl >= iBufferSize)
    pl = iBufferSize - 1;
  memcpy (oInstallPath, path, pl);
  if (addsep)
    oInstallPath [pl - 1] = PATH_SEPARATOR;
  oInstallPath [pl] = 0;
  return true;
}

void csSystemDriver::Printf (int mode, const char *format, ...)
{
  char buf[1024];
  va_list arg;

  va_start (arg, format);
  vsprintf (buf, format, arg);
  va_end (arg);

  switch (mode)
  {
    case MSG_INTERNAL_ERROR:
    case MSG_FATAL_ERROR:
      Alert (buf);
      break;

    case MSG_WARNING:
      Warn (buf);
      break;

    case MSG_INITIALIZATION:
      console_out (buf);
      debug_out (true, buf);
      if (Console)
        Console->PutText (MSG_INITIALIZATION, buf);
      break;

    case MSG_CONSOLE:
      if (Console)
        Console->PutText (MSG_CONSOLE, buf);
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

iBase *csSystemDriver::LoadPlugIn (const char *iClassID, const char *iFuncID,
  const char *iInterface, int iVersion)
{
  iPlugIn *p = CREATE_INSTANCE (iClassID, iPlugIn);
  if (!p)
    Printf (MSG_WARNING, "WARNING: could not load plugin `%s'\n", iClassID);
  else
  {
    int index = PlugIns.Push (new csPlugIn (p, iClassID, iFuncID));
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
    PlugIns.Delete (index);
  }
  return NULL;
}

bool csSystemDriver::RegisterPlugIn (const char *iClassID,
  const char *iFuncID, iPlugIn *iObject)
{
  int index = PlugIns.Push (new csPlugIn (iObject, iClassID, iFuncID));
  if (iObject->Initialize (this))
  {
    QueryOptions (iObject);
    iObject->IncRef ();
    return true;
  }
  else
  {
    Printf (MSG_WARNING, "WARNING: failed to initialize plugin `%s'\n", iClassID);
    PlugIns.Delete (index);
    return false;
  }
}

int csSystemDriver::GetNumPlugIns ()
{
  return PlugIns.Length ();
}

iBase* csSystemDriver::GetPlugIn (int idx)
{
  csPlugIn* pl = PlugIns.Get (idx);
  return pl->PlugIn;
}

iBase *csSystemDriver::QueryPlugIn (const char *iInterface, int iVersion)
{
  for (int i = 0; i < PlugIns.Length (); i++)
  {
    iBase *ret =
      (iBase *)PlugIns.Get (i)->PlugIn->QueryInterface (iInterface, iVersion);
    if (ret)
      return ret;
  }
  return NULL;
}

iBase *csSystemDriver::QueryPlugIn (const char *iFuncID, const char *iInterface,
  int iVersion)
{
  int idx = PlugIns.FindKey (iFuncID, 1);
  if (idx < 0)
    return NULL;

  return (iBase *)PlugIns.Get (idx)->PlugIn->QueryInterface (iInterface, iVersion);
}

iBase *csSystemDriver::QueryPlugIn (const char* iClassID, const char *iFuncID, const char *iInterface,
  int iVersion)
{
  int i;
  for (i = 0 ; i < PlugIns.Length () ; i++)
  {
    csPlugIn* pl = PlugIns.Get (i);
    if (pl->ClassID)
      if (pl->ClassID == iClassID || !strcmp (pl->ClassID, iClassID))
      {
	if (pl->FuncID)
	  if (pl->FuncID == iFuncID || !strcmp (pl->FuncID, iFuncID))
	  {
	    return (iBase *)PlugIns.Get (i)->PlugIn->QueryInterface (iInterface, iVersion);
	  }
      }
  }
  return NULL;
}

bool csSystemDriver::UnloadPlugIn (iPlugIn *iObject)
{
  int idx = PlugIns.FindKey (iObject);
  if (idx < 0)
    return false;

  iConfig *config = QUERY_INTERFACE (iObject, iConfig);
  if (config)
  {
    for (int i = OptionList.Length () - 1; i >= 0; i--) 
    {
      csPluginOption *pio = (csPluginOption *)OptionList.Get (i);
      if (pio->Config == config)
        OptionList.Delete (i);
    }
    config->DecRef ();
  }

  csPlugIn *p = PlugIns.Get (idx);

#define CHECK(Var,Func)						\
  if (!strcmp (p->FuncID, Func)) { Var->DecRef (); Var = NULL; }

  CHECK (VFS, CS_FUNCID_VFS)
  CHECK (G3D, CS_FUNCID_VIDEO)
  CHECK (G2D, CS_FUNCID_CANVAS)
  CHECK (Sound, CS_FUNCID_SOUND)
  CHECK (NetDrv, CS_FUNCID_NETDRV)
  CHECK (Console, CS_FUNCID_CONSOLE)

#undef CHECK

  return PlugIns.Delete (idx);
}

iConfigFile *csSystemDriver::GetConfig ()
{
  return Config;
}

iConfigFile *csSystemDriver::CreateConfig (const char *iFileName, bool iVFS)
{
  return new csIniFile (iFileName, iVFS ? VFS : NULL);
}

iConfigFileNew *csSystemDriver::CreateConfigNew (const char *iFileName, bool iVFS)
{
  return new csConfigFile (iFileName, iVFS ? VFS : NULL);
}

bool csSystemDriver::SaveConfig ()
{
  if (!Config->IsDirty ()) return true;
  return Config->Save ();
}

bool csSystemDriver::CallOnEvents (iPlugIn *iObject, unsigned int iEventMask)
{
  int idx = PlugIns.FindKey (iObject);
  if (idx < 0)
    return false;

  csPlugIn *plugin = PlugIns.Get (idx);
  plugin->EventMask = iEventMask;
  return true;
}

bool csSystemDriver::GetKeyState (int key)
{
  return Keyboard.GetKeyState (key);
}

bool csSystemDriver::GetMouseButton (int button)
{
  return Mouse.GetLastButton (button);
}

void csSystemDriver::GetMousePosition (int &x, int &y)
{
  x = Mouse.GetLastX ();
  y = Mouse.GetLastY ();
}

bool csSystemDriver::GetJoystickButton (int number, int button)
{
  return Joystick.GetLastButton (number, button);
}

void csSystemDriver::GetJoystickPosition (int number, int &x, int &y)
{
  x = Joystick.GetLastX (number);
  y = Joystick.GetLastY (number);
}

iEventOutlet *csSystemDriver::CreateEventOutlet (iEventPlug *iObject)
{
  if (!iObject)
    return NULL;

  csEventOutlet *outlet = new csEventOutlet (iObject, this);
  EventOutlets.Push (outlet);
  return outlet;
}

iEventCord *csSystemDriver::GetEventCord (int Category, int Subcategory)
{
  int idx = EventCords.Find (Category, Subcategory);
  if (idx != -1) 
    return EventCords.Get (idx);
  else
  {
    csEventCord *cord = new csEventCord (Category, Subcategory);
    EventCords.Push (cord);
    return cord;
  }
}

iEventOutlet *csSystemDriver::GetSystemEventOutlet ()
{
  return EventOutlets.Get (0);
}

csSystemDriver::csCommandLineOption *csSystemDriver::FindOptionCL (const char *iName, int iIndex)
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
    return (csCommandLineOption *)CommandLine.Get (idx);
  }
  return NULL;
}

bool csSystemDriver::ReplaceOptionCL (const char *iName, const char *iValue, int iIndex)
{
  csCommandLineOption *clo = FindOptionCL (iName, iIndex);
  if (clo)
  {
    delete [] clo->Value;
    clo->Value = strnew (iValue);
    return true;
  }
  else
    return false;
}

bool csSystemDriver::ReplaceNameCL (const char *iValue, int iIndex)
{
  if ((iIndex >= 0) && (iIndex < CommandLineNames.Length ()))
  {
    CommandLineNames.Replace (iIndex, strnew (iValue));
    return true;
  }
  else
    return false;
}

const char *csSystemDriver::GetOptionCL (const char *iName, int iIndex)
{
  csCommandLineOption *clo = FindOptionCL (iName, iIndex);
  return clo ? (clo->Value ? clo->Value : "") : NULL;
}

const char *csSystemDriver::GetNameCL (int iIndex)
{
  if ((iIndex >= 0) && (iIndex < CommandLineNames.Length ()))
    return (const char *)CommandLineNames.Get (iIndex);
  return NULL;
}

void csSystemDriver::AddOptionCL (const char *iName, const char *iValue)
{
  CommandLine.Push (new csCommandLineOption (strnew (iName), strnew (iValue)));
}

void csSystemDriver::AddNameCL (const char *iName)
{
  CommandLineNames.Push (strnew (iName));
}
