/*
    Copyright (C) 2008 by Frank Richter

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

#include "csplugincommon/rendermanager/debugcommon.h"

namespace CS
{
  namespace RenderManager
  {
    RMDebugCommonBase::RMDebugCommonBase() : wantDebugLockLines (false) {}
    
    bool RMDebugCommonBase::DebugCommand (const char* _cmd)
    {
      csString cmd (_cmd);
      csString args;
      size_t space = cmd.FindFirst (' ');
      if (space != (size_t)-1)
      {
	cmd.SubString (args, space+1);
	cmd.Truncate (space);
      }

      if (strcmp (cmd, "toggle_debug_lines_lock") == 0)
      {
	csPrintf ("%p got toggle_debug_lines_lock: ", this);
	if (HasDebugLockLines())
	{
	  DeleteDebugLockLines();
	  csPrintf ("unlocked\n");
	}
	else
	{
	  wantDebugLockLines = !wantDebugLockLines;
	  csPrintf ("%slocked\n", wantDebugLockLines ? "" : "un");
	}
	return true;
      }
      else if (strcmp (cmd, "toggle_debug_flag") == 0)
      {
	RenderTreeBase::DebugPersistent& persist = GetDebugPersistent ();
	uint flag = persist.QueryDebugFlag (args);
	if (flag != (uint)-1)
	{
	  persist.EnableDebugFlag (flag, !persist.IsDebugFlagEnabled (flag));
	}
	return true;
      }
      return false;
    }
  
  } // namespace RenderManager
} // namespace CS
