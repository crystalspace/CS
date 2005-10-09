/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Written by Michael Dale Long.

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
#include "csutil/scf.h"
#include "csutil/csevcord.h"
#include "iutil/eventh.h"

csEventCord::csEventCord(int cat, int subcat) 
  : scfImplementationType (this),
  category(cat), subcategory(subcat)
{
  plugins = 0;
  // By default, only pass along category 0, subcategory 0 events to the queue.
  pass = (category == 0 && subcategory == 0);
  SpinLock = 0;
}

csEventCord::~csEventCord()
{
}

int csEventCord::Insert(iEventHandler *plugin, int priority)
{
  Lock ();
  // Increment the plugin reference count
  plugin->IncRef ();
  int retval = 0;
  if (plugins)
  {
    // Add a new record
    PluginData *last = 0, *curr = plugins;
    while (curr)
    {
      if (priority > curr->priority)
        break;
      last = curr;
      curr = curr->next;
      retval++;
    }
    if (last)
    {
      // Insert in the middle or at the end
      last->next = new PluginData;
      last->next->plugin = plugin;
      last->next->priority = priority;
      last->next->next = curr;
    }
    else
    {
      // Insert at the top
      plugins = new PluginData;
      plugins->plugin = plugin;
      plugins->priority = priority;
      plugins->next = curr;
    }
  }
  else
  {
    // Create first record
    plugins = new PluginData;
    plugins->plugin = plugin;
    plugins->priority = priority;
    plugins->next = 0;
  }

  Unlock ();
  return retval;
}

void csEventCord::Remove (iEventHandler *plugin)
{
  Lock ();
  PluginData *last = 0, *curr = plugins;
  // Walk the list until you find the plugin
  while (curr)
  {
    if (curr->plugin == plugin)
      if (last)
      {
        // Unlink the plugin from the list and delete it's record
        last->next = curr->next;
        curr->plugin->DecRef ();
        delete curr;
        break;
      }
    last = curr;
    curr = curr->next;
  }

  Unlock ();
}

bool csEventCord::Post (iEvent *event)
{
  Lock ();

  PluginData *cur = plugins;
  // Walk the list until a plugin returns true
  while (cur)
  {
    if (cur->plugin->HandleEvent (*event))
    {
      Unlock ();
      return true;
    }
    cur = cur->next;
  }

  Unlock ();

  // If we pass events along to the queue, then we return
  // false so it will process the event.
  return !pass;
}
