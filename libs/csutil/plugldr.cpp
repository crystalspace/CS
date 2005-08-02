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

#include "cssysdef.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "csutil/scf.h"
#include "csutil/plugldr.h"
#include "csutil/util.h"
#include "csutil/snprintf.h"
#include "ivaria/reporter.h"
#include "iutil/objreg.h"
#include "iutil/cmdline.h"
#include "iutil/cfgmgr.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "iutil/comp.h"

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
 * The algorithm works as follows. First, a dependency matrix is built. Here
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
 *   <li> []                                  [iEngine]
 *   <li> []                                  [iEngine,iVFS]
 *   <li> [iVFS]                              [iEngine]
 *   <li> [iVFS]                              [iEngine,iGraphics3D]
 *   <li> [iVFS]                              [iEngine,iGraphics3D,iGraphics2D]
 *   <li> [iVFS,iGraphics2D]                  [iEngine,iGraphics3D]
 *   <li> [iVFS,iGraphics2D,iGraphics3D]      [iEngine]
 *   <li> [iVFS,iGraphics2D,iGraphics3D,iEngine] []
 * </ol></pre>
 * In this example we traversed all plugins in one go. If we didn't, we
 * just take the next one (iEngine, iVFS, iGraphics3D, iGraphics2D) and if
 * it is not already in the load list, recursively traverse it.
 */
bool csPluginList::Sort (iObjectRegistry* object_reg)
{
  size_t row, col, len = Length ();

  // Build the dependency matrix
  CS_ALLOC_STACK_ARRAY (bool, matrix, len * len);
  memset (matrix, 0, len * len * sizeof (bool));
  for (row = 0; row < len; row++)
  {
    const char *dep = iSCF::SCF->GetClassDependencies (Get (row)->ClassID);
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
         && (wildcard ? strncmp (tmp, Get (col)->ClassID, sl) :
             strcmp (tmp, Get (col)->ClassID)) == 0)
          matrix [row * len + col] = true;
      dep = comma;
      while (*dep == ',' || *dep == ' ' || *dep == '\t')
        dep++;
    }
  }

  // Go through dependency matrix and put all plugins into an array
  bool error = false;
  CS_ALLOC_STACK_ARRAY (size_t, order, len + 1);
  *order = 0;
  CS_ALLOC_STACK_ARRAY (size_t, loop, len + 1);
  *loop = 0;

  for (row = 0; row < len; row++)
    if (!RecurseSort (object_reg, row, order, loop, matrix))
      error = true;

  // Reorder plugin list according to "order" array
  csPluginLoadRec** newroot = new csPluginLoadRec*[len];
  for (row = 0; row < len; row++)
    newroot [row] = GetAndClear (order [row] - 1);
  for (row = 0; row < len; row++)
    Put (row, newroot[row]);
  delete[] newroot;

  return !error;
}

static size_t* strchr_int (size_t* str, size_t fnd)
{
  while (*str != fnd)
  {
    if (!*str) return 0;
    str++;
  }
  return str;
}

bool csPluginList::RecurseSort (iObjectRegistry *object_reg,
	size_t row, size_t* order, size_t* loop, bool *matrix)
{
  // If the plugin is already in the load list, skip it
  if (strchr_int (order, row + 1))
    return true;

  size_t len = Length ();
  bool *dep = matrix + row * len;
  bool error = false;
  size_t* loopp = strchr_int (loop, 0);
  *loopp++ = row + 1; *loopp = 0;
  size_t col, x;
  for (col = 0; col < len; col++)
    if (*dep++)
    {
      // If the plugin is already loaded, skip
      if (strchr_int (order, col + 1))
        continue;

      size_t* already = strchr_int (loop, col + 1);
      if (already)
      {
	csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	    "crystalspace.pluginloader.recursesort",
	    "Cyclic dependency detected!");
        size_t startx = already - loop;
        for (x = startx; loop [x]; x++)
	{
	  csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	    "crystalspace.pluginloader.recursesort",
            "   %s %s",
            x == startx ? "+->" : loop [x + 1] ? "| |" : "<-+",
            Get (loop [x] - 1)->ClassID);
	}
        error = true;
        break;
      }

      bool recurse_error = !RecurseSort (object_reg, col, order, loop, matrix);

      // Drop recursive loop dependency since it has already been ordered.
      *loopp = 0;

      if (recurse_error)
      {
        error = true;
        break;
      }
    }

  // Put current plugin into the list
  size_t* orderp = strchr_int (order, 0);
  *orderp++ = row + 1; *orderp = 0;

  return !error;
}

//--------------------------------------------------- The plugin loader -----//

csPluginLoader::csPluginLoader (iObjectRegistry* object_reg)
{
  csPluginLoader::object_reg = object_reg;
}

csPluginLoader::~csPluginLoader ()
{
}

bool csPluginLoader::LoadPlugins ()
{
  // Collect all options from command line
  csRef<iCommandLineParser> CommandLine (CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser));
  CS_ASSERT (CommandLine != 0);

  // The list of plugins
  csPluginList PluginList;

  // Now eat all common-for-plugins command-line switches
  bool g3d_override = false;
  const char *val = CommandLine->GetOption ("video");
  if (val)
  {
    // Alternate videodriver
    char temp [100];
    cs_snprintf (temp, sizeof(temp), "crystalspace.graphics3d.%s", val);
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
    	"crystalspace.pluginloader.loadplugins",
    	"Using alternative 3D driver: %s", temp);
    PluginList.Push (new csPluginLoadRec ("iGraphics3D", temp));
    g3d_override = true;
  }

  val = CommandLine->GetOption ("canvas");
  if (val)
  {
    if (!strchr (val, '.'))
    {
      char temp [100];
      cs_snprintf (temp, sizeof(temp), "crystalspace.graphics2d.%s", val);
      csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
    	  "crystalspace.pluginloader.loadplugins",
    	  "Using alternative 2D canvas: %s", temp);
      CommandLine->ReplaceOption ("canvas", temp);
    }
  }

  // Eat all --plugin switches specified on the command line
  size_t n = 0;
  while ((val = CommandLine->GetOption ("plugin", n++)))
  {
    size_t sl = strlen (val);
    char temp [100];
    if (sl >= sizeof (temp)) sl = sizeof (temp) - 1;
    memcpy (temp, val, sl); temp [sl] = 0;
    char *tag = strchr (temp, ':');
    if (tag) *tag++ = 0;
    if (g3d_override && tag && !strcmp ("iGraphics3D", tag)) continue;
    // If an ID isn't registered try to insert "crystalspace.utilities."
    // at the beginning. That makes it possible to specfiy e.g. 
    // '-plugin=bugplug' on the cmd line.
    if (!iSCF::SCF->ClassRegistered (temp))
    {
      char temp2 [100];
      cs_snprintf (temp2, sizeof(temp2), "crystalspace.utilities.%s", temp);
      PluginList.Push (new csPluginLoadRec (tag, temp2));
    }
    else
    {
      PluginList.Push (new csPluginLoadRec (tag, temp));
    }
  }

  // Now load and initialize all plugins
  csRef<iConfigManager> Config (CS_QUERY_REGISTRY (object_reg, iConfigManager));
  csRef<iConfigIterator> plugin_list (Config->Enumerate ("System.Plugins."));
  if (plugin_list)
  {
    while (plugin_list->Next ())
    {
      const char *tag = plugin_list->GetKey (true);
      // If -video was used to override 3D driver, then respect it.
      if (g3d_override && strcmp (tag, "iGraphics3D") == 0)
        continue;
      const char *classID = plugin_list->GetStr ();
      if (classID)
        PluginList.Push (new csPluginLoadRec (tag, classID));
    }
  }

  csRef<iVFS> VFS (CS_QUERY_REGISTRY (object_reg, iVFS));

  // Check all requested plugins and see if there is already
  // a plugin with that tag present. If not we add it.
  size_t i;
  for (i = 0 ; i < requested_plugins.Length () ; i++)
  {
    csPluginLoadRec* req_plugin = requested_plugins.Get (i);
    size_t j;
    bool present = false;
    for (j = 0 ; j < PluginList.Length () ; j++)
    {
      csPluginLoadRec* plugin = PluginList.Get (j);
      if (plugin->Tag && !strcmp (plugin->Tag, req_plugin->Tag))
      {
        present = true;
	break;
      }
    }
    if (!present)
    {
      PluginList.Push (new csPluginLoadRec (req_plugin->Tag,
      	req_plugin->ClassID));
    }
  }

  // Sort all plugins by their dependency lists
  if (!PluginList.Sort (object_reg))
  {
    return false;
  }

  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));

  // Load all plugins
  for (n = 0; n < PluginList.Length (); n++)
  {
    csPluginLoadRec* r = PluginList.Get(n);
    // If plugin is VFS then skip if already loaded earlier.
    r->plugin = 0;
    if (VFS && r->Tag && strcmp (r->Tag, "iVFS") == 0)
      continue;
    r->plugin.AttachNew(plugin_mgr->LoadPlugin (r->ClassID, true));
    if (r->plugin)
    {
      if (!object_reg->Register (r->plugin, r->Tag))
      {
	if (r->Tag)
	  csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	    "crystalspace.pluginloader.loadplugins",
	    "Duplicate tag '%s' found for plugin '%s'!", r->Tag, r->ClassID);
	else
	  csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	    "crystalspace.pluginloader.loadplugins",
	    "Could not register plugin '%s'!", r->ClassID);
	return false;
      }
    }
  }

  // flush all removed config files
  Config->FlushRemoved();

  return true;
}

void csPluginLoader::RequestPlugin (const char *pluginName,
	const char* tagName)
{
  requested_plugins.Push (new csPluginLoadRec (tagName, pluginName));
}

