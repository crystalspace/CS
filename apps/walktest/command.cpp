/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

/*
 * Command processor. There are now several sources of commands:
 * the console and the keyboard. This class ignores the sources but
 * just executes the commands. The respective source handlers should
 * then do whatever they need to recognize the command and send the
 * command to this class.
 */

#include <string.h>
#include "cssysdef.h"
#include "csver.h"
#include "csengine/polytext.h"
#include "command.h"
#include "csutil/scanstr.h"
#include "walktest.h"
#include "qint.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivaria/conout.h"
#include "isys/event.h"
#include "imesh/object.h"
#include "iengine/mesh.h"

extern WalkTest* Sys;

// Static csCommandProcessor variables
iEngine* csCommandProcessor::engine = NULL;
iCamera* csCommandProcessor::camera = NULL;
iGraphics3D* csCommandProcessor::g3d = NULL;
iConsoleOutput* csCommandProcessor::console = NULL;
iSystem* csCommandProcessor::system = NULL;
iFile* csCommandProcessor::script = NULL;
// Additional command handler
csCommandProcessor::CmdHandler csCommandProcessor::ExtraHandler = NULL;

void csCommandProcessor::Initialize (iEngine* engine, iCamera* camera,
  iGraphics3D* g3d, iConsoleOutput* console, iSystem* system)
{
  csCommandProcessor::engine = engine;
  csCommandProcessor::camera = camera;
  csCommandProcessor::g3d = g3d;
  csCommandProcessor::console = console;
  csCommandProcessor::system = system;
}

bool csCommandProcessor::PerformLine (const char* line)
{
  return perform_line (line);
}

void csCommandProcessor::perform_callback (void *, const char *command)
{
  perform_line (command);
}

static int value_choice (const char* arg, int old_value, const char* const* choices, int num)
{
  if (!arg) return -1;
  int i = 0;
  if (!strcasecmp (arg, "next")) return (old_value+1)%num;
  if (!strcasecmp (arg, "prev")) return (old_value-1+num)%num;
  while (choices[i])
  {
    if (!strcasecmp (choices[i], arg)) return i;
    i++;
  }
  CsPrintf (CS_MSG_CONSOLE, "Expected one of the following:\n");
  i = 0;
  while (choices[i])
  {
    CsPrintf (CS_MSG_CONSOLE, "    %s%s\n", choices[i], i == old_value ? " (current)" : "");
    i++;
  }
  CsPrintf (CS_MSG_CONSOLE, "    or 'next' or 'prev'\n");
  return -1;
}

static bool yes_or_no (const char* arg, bool old_value)
{
  if (!arg) return false;
  if (*arg == '0' && *(arg+1) == 0) return false;
  if (*arg == '1' && *(arg+1) == 0) return true;
  if (!strcasecmp (arg, "yes") || !strcasecmp (arg, "true") || !strcasecmp (arg, "on")) return true;
  if (!strcasecmp (arg, "no") || !strcasecmp (arg, "false") || !strcasecmp (arg, "off")) return false;
  if (!strcasecmp (arg, "toggle")) return !old_value;
  CsPrintf (CS_MSG_CONSOLE, "Expected: yes, true, on, 1, no, false, off, 0, or toggle!\n");
  return false;
}

static const char* say_on_or_off (int arg)
{
  if (arg) return "on";
  return "off";
}

/*
 * Standard processing to change/display a boolean value setting.
 */
void csCommandProcessor::change_boolean (const char* arg, bool* value, const char* what)
{
  if (arg)
  {
    // Change value
    int v = yes_or_no (arg, *value);
    if (v != -1)
    {
      *value = v;
      CsPrintf (CS_MSG_CONSOLE, "Set %s %s\n", what, say_on_or_off (*value));
    }
  }
  else
  {
    // Show value
    CsPrintf (CS_MSG_CONSOLE, "Current %s is %s\n", what, say_on_or_off (*value));
  }
}

/*
 * Standard processing to change/display a multi-value setting.
 */
void csCommandProcessor::change_choice (const char* arg, int* value, const char* what, const char* const* choices, int num)
{
  if (arg)
  {
    // Change value
    int v = value_choice (arg, *value, choices, num);
    if (v != -1)
    {
      *value = v;
      CsPrintf (CS_MSG_CONSOLE, "Set %s %s\n", what, choices[*value]);
    }
  }
  else
  {
    // Show value
    CsPrintf (CS_MSG_CONSOLE, "Current %s is %s\n", what, choices[*value]);
  }
}

/*
 * Standard processing to change/display a floating point setting.
 * Return true if value changed.
 */
bool csCommandProcessor::change_float (const char* arg, float* value, const char* what, float min, float max)
{
  if (arg)
  {
    // Change value.
    float g;
    if ((*arg == '+' || *arg == '-') && *(arg+1) == *arg)
    {
      float dv;
      sscanf (arg+1, "%f", &dv);
      g = *value+dv;
    }
    else sscanf (arg, "%f", &g);
    if (g < min || g > max) CsPrintf (CS_MSG_CONSOLE, "Bad value for %s (%f <= value <= %f)!\n", what, min, max);
    else
    {
      *value = g;
      CsPrintf (CS_MSG_CONSOLE, "Set %s to %f\n", what, *value);
      return true;
    }
  }
  else
  {
    // Show value.
    CsPrintf (CS_MSG_CONSOLE, "Current %s is %f\n", what, *value);
  }
  return false;
}

/*
 * Standard processing to change/display an integer setting.
 * Return true if value changed.
 */
bool csCommandProcessor::change_int (const char* arg, int* value, const char* what, int min, int max)
{
  if (arg)
  {
    // Change value.
    int g;
    if ((*arg == '+' || *arg == '-') && *(arg+1) == *arg)
    {
      int dv;
      sscanf (arg+1, "%d", &dv);
      g = *value+dv;
    }
    else sscanf (arg, "%d", &g);
    if (g < min || g > max) CsPrintf (CS_MSG_CONSOLE, "Bad value for %s (%d <= value <= %d)!\n", what, min, max);
    else
    {
      *value = g;
      CsPrintf (CS_MSG_CONSOLE, "Set %s to %d\n", what, *value);
      return true;
    }
  }
  else
  {
    // Show value.
    CsPrintf (CS_MSG_CONSOLE, "Current %s is %d\n", what, *value);
  }
  return false;
}

/*
 * Standard processing to change/display a long setting.
 * Return true if value changed.
 */
bool csCommandProcessor::change_long (const char* arg, long* value, const char* what, long min, long max)
{
  if (arg)
  {
    // Change value.
    long g;
    if ((*arg == '+' || *arg == '-') && *(arg+1) == *arg)
    {
      long dv;
      sscanf (arg+1, "%ld", &dv);
      g = *value+dv;
    }
    else sscanf (arg, "%ld", &g);
    if (g < min || g > max) CsPrintf (CS_MSG_CONSOLE, "Bad value for %s (%ld <= value <= %ld)!\n", what, min, max);
    else
    {
      *value = g;
      CsPrintf (CS_MSG_CONSOLE, "Set %s to %ld\n", what, *value);
      return true;
    }
  }
  else
  {
    // Show value.
    CsPrintf (CS_MSG_CONSOLE, "Current %s is %ld\n", what, *value);
  }
  return false;
}

bool csCommandProcessor::perform_line (const char* line)
{
  char cmd[512], arg[255];
  if (*line == ';') return true;        // Comment
  if (*line == 0) return true;          // Empty line
  strcpy (cmd, line);
  char* space = strchr (cmd, ' ');
  if (space) { *space = 0; strcpy (arg, space+1); }
  else *arg = 0;
  return perform (cmd, *arg ? arg : (char*)NULL);
}

extern bool GetConfigOption (iBase* plugin, const char* optName, csVariant& optValue);
extern void SetConfigOption (iBase* plugin, const char* optName, const char* optValue);
extern void SetConfigOption (iBase* plugin, const char* optName, csVariant& optValue);

bool csCommandProcessor::perform (const char* cmd, const char* arg)
{
  if (ExtraHandler)
  {
    static bool inside = false;
    if (!inside)
    {
      inside = true;
      bool ret = ExtraHandler (cmd, arg);
      inside = false;
      if (ret) return true;
    }
  }

  if (!strcasecmp (cmd, "quit"))
    csEngine::System->GetSystemEventOutlet ()->Broadcast (cscmdQuit);
  else if (!strcasecmp (cmd, "help"))
  {
    CsPrintf (CS_MSG_CONSOLE, "-*- General commands -*-\n");
    CsPrintf (CS_MSG_CONSOLE, " about, version, quit, help\n");
    CsPrintf (CS_MSG_CONSOLE, " debug, db_maxpol, db_procpol\n");
    CsPrintf (CS_MSG_CONSOLE, " console, facenorth, facesouth, faceeast\n");
    CsPrintf (CS_MSG_CONSOLE, " facewest, faceup, facedown, turn, activate\n");
    CsPrintf (CS_MSG_CONSOLE, " cls, exec, dnl, cmessage, dmessage\n");
    CsPrintf (CS_MSG_CONSOLE, " portals, cosfact\n");
    CsPrintf (CS_MSG_CONSOLE, " extension, lod, sprlight, coordset\n");
  }
  else if (!strcasecmp (cmd, "about"))
  {
    CsPrintf (CS_MSG_CONSOLE, "Crystal Space version %s (%s).\n", CS_VERSION, CS_RELEASE_DATE);
  }
  else if (!strcasecmp (cmd, "version"))
    CsPrintf (CS_MSG_CONSOLE, "%s\n", CS_VERSION);
  else if (!strcasecmp (cmd, "extension"))
  {
    iGraphics2D* g2d = g3d->GetDriver2D ();
    if (!g2d->PerformExtension (arg))
      CsPrintf (CS_MSG_CONSOLE, "Extension '%s' not supported!\n", arg);
  }
  else if (!strcasecmp (cmd, "db_maxpol"))
  {
    long val = g3d->GetRenderState (G3DRENDERSTATE_MAXPOLYGONSTODRAW);
    int ival = (int)val;
    change_int (arg, &ival, "maximum polygons", 0, 2000000000);
    g3d->SetRenderState (G3DRENDERSTATE_MAXPOLYGONSTODRAW, (long)ival);
  }
  else if (!strcasecmp (cmd, "db_procpol"))
  {
    int val = csEngine::GetMaxProcessPolygons ();
    change_int (arg, &val, "maximum process polygons", 0, 2000000000);
    csEngine::SetMaxProcessPolygons (val);
  }
  else if (!strcasecmp (cmd, "cmessage"))
  {
    if (arg) CsPrintf (CS_MSG_CONSOLE, arg);
    else CsPrintf (CS_MSG_CONSOLE, "Argument expected!\n");
  }
  else if (!strcasecmp (cmd, "dmessage"))
  {
    if (arg) CsPrintf (CS_MSG_DEBUG_0, arg);
    else CsPrintf (CS_MSG_CONSOLE, "Argument expected!\n");
  }
  else if (!strcasecmp (cmd, "cosfact"))
    change_float (arg, &csPolyTexture::cfg_cosinus_factor, "cosinus factor", -1, 1);
  else if (!strcasecmp (cmd, "lod"))
  {
    iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (Sys, "crystalspace.mesh.object.sprite.3d",
      	  "MeshObj", iMeshObjectType);
    csVariant lod_level;
    GetConfigOption (type, "sprlod", lod_level);
    change_float (arg, &lod_level.v.f, "LOD detail", -1, 1000000);
    SetConfigOption (type, "sprlod", lod_level);
  }
  else if (!strcasecmp (cmd, "sprlight"))
  {
    iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (Sys, "crystalspace.mesh.object.sprite.3d",
      	  "MeshObj", iMeshObjectType);
    csVariant lqual;
    GetConfigOption (type, "sprlq", lqual);
    change_long (arg, &lqual.v.l, "sprite lighting quality", 0, 3);
    SetConfigOption (type, "sprlq", lqual);
  }
  else if (!strcasecmp (cmd, "dnl"))
    CsPrintf (CS_MSG_DEBUG_0, "\n");
  else if (!strcasecmp (cmd, "exec"))
  {
    if (arg)
    {
      if (start_script (arg) && console)
        console->SetVisible (true);
    }
    else CsPrintf (CS_MSG_CONSOLE, "Please specify the name of the script!\n");
  }
  else if (!strcasecmp (cmd, "portals"))
    change_boolean (arg, &csSector::do_portals, "portals");
  else if (!strcasecmp (cmd, "cls"))
  {
    if (console)
      console->Clear ();
  }
  else if (!strcasecmp (cmd, "console"))
  {
    if (console)
    {
      bool active = console->GetVisible ();
      change_boolean (arg, &active, "console");
      if (active != console->GetVisible ())
        console->SetVisible (active);
    }
  }
  else if (!strcasecmp (cmd, "turn"))
    camera->GetTransform ().RotateThis (VEC_ROT_RIGHT, M_PI);
  else if (!strcasecmp (cmd, "activate"))
  {
    CsPrintf (CS_MSG_CONSOLE, "OBSOLETE\n");
  }
  else if (!strcasecmp (cmd, "coordset"))
  {
    if (!arg)
    {
      CsPrintf (CS_MSG_CONSOLE, "Expected argument!\n");
      return false;
    }
    char sect[100];
    float x, y, z;
    if (csScanStr (arg, "%s,%f,%f,%f", sect, &x, &y, &z) != 4)
    {
      CsPrintf (CS_MSG_CONSOLE, "Expected sector,x,y,z. Got something else!\n");
      return false;
    }
    iSector* s = engine->FindSector (sect);
    if (!s)
    {
      CsPrintf (CS_MSG_CONSOLE, "Can't find this sector!\n");
      return false;
    }
    camera->SetSector (s);
    camera->GetTransform ().SetOrigin (csVector3(x,y,z));
  }
  else if (!strcasecmp (cmd, "facenorth"))
   camera->GetTransform ().SetO2T (csMatrix3() /* identity */ );
  else if (!strcasecmp (cmd, "facesouth"))
   camera->GetTransform ().SetO2T ( csMatrix3 ( -1,  0,  0,
                               0,  1,  0,
                               0,  0, -1 ) );
  else if (!strcasecmp (cmd, "facewest"))
   camera->GetTransform ().SetO2T ( csMatrix3 (  0,  0,  1,
                               0,  1,  0,
                              -1,  0,  0 ) );
  else if (!strcasecmp (cmd, "faceeast"))
   camera->GetTransform ().SetO2T ( csMatrix3 (  0,  0, -1,
                               0,  1,  0,
                              1,  0,  0 ) );
  else if (!strcasecmp (cmd, "facedown"))
   camera->GetTransform ().SetO2T ( csMatrix3 (  1,  0,  0,
                               0,  0,  1,
                               0, -1,  0 ) );
  else if (!strcasecmp (cmd, "faceup"))
   camera->GetTransform ().SetO2T ( csMatrix3 (  1,  0,  0,
                               0,  0, -1,
                               0,  1,  0 ) );
  else
  {
    CsPrintf (CS_MSG_CONSOLE, "Unknown command: `%s'\n", cmd);
    return false;
  }
  return true;
}

bool csCommandProcessor::start_script (const char* scr)
{
  bool ok = false;
  iVFS* v = CS_QUERY_PLUGIN_ID (system, CS_FUNCID_VFS, iVFS);
  if (v)
  {
    if (v->Exists (scr))
    {
      iFile* f = v->Open (scr, VFS_FILE_READ);
      if (!f)
        CsPrintf (CS_MSG_CONSOLE, "Could not open script file '%s'!\n", scr);
      else
      {
        // Replace possible running script with this one.
        if (script)
          script->DecRef();
        script = f;
        ok = true;
      }
    }
    v->DecRef();
  }
  return ok;
}

bool csCommandProcessor::get_script_line (char* buf, int nbytes)
{
  if (!script)
    return false;

  char c = '\n';
  while (c == '\n' || c == '\r')
    if (!script->Read(&c, 1))
      break;

  if (script->AtEOF())
  {
    script->DecRef();
    script = NULL;
    return false;
  }

  char* p = buf;
  const char* plim = p + nbytes - 1;
  while (p < plim)
  {
    if (c == '\n' || c == '\r')
      break;
    *p++ = c;
    if (!script->Read(&c, 1))
      break;
  }
  *p = '\0';
  return true;
}
