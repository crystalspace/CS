/*
    Common routines for X-Windows drivers
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

#include "cssysdef.h"
#include "x11comm.h"
#include "csutil/cfgacc.h"
#include "iutil/cmdline.h"

void GetX11Settings (iSystem *iSys, int &oSimDepth, bool &oUseSHM,
  bool &oHardwareCursor)
{
  csConfigAccess Config(iSys, "/config/video.cfg");
  oSimDepth = Config->GetInt ("Video.SimulateDepth", 0);
  oUseSHM = Config->GetBool ("Video.XSHM", true);
  oHardwareCursor = Config->GetBool ("Video.SystemMouseCursor", true);

  const char *val;
  if ((val = iSys->GetCommandLine ()->GetOption ("shm")))
    oUseSHM = true;

  if ((val = iSys->GetCommandLine ()->GetOption ("noshm")))
    oUseSHM = false;

  if ((val = iSys->GetCommandLine ()->GetOption ("sdepth")))
  {
    oSimDepth = atol (val);
    if (oSimDepth != 8 && oSimDepth != 15 && oSimDepth != 16 && oSimDepth != 32)
    {
      iSys->Printf (CS_MSG_WARNING, "Crystal Space can't run in this simulated depth! (use 8, 15, 16, or 32)!\n");
      oSimDepth = 0;
    }
  }

  if (iSys->GetCommandLine ()->GetOption ("sysmouse"))
    oHardwareCursor = true;
  if (iSys->GetCommandLine ()->GetOption ("nosysmouse"))
    oHardwareCursor = false;
}
