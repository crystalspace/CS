/*
    Copyright (C) 2001 by Jorrit Tyberghein

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
#include <stdio.h>
#include <string.h>
#include "csutil/cmdhelp.h"
#include "csutil/csevent.h"
#include "csutil/snprintf.h"
#include "csutil/sysfunc.h"
#include "iutil/objreg.h"
#include "iutil/cmdline.h"
#include "iutil/eventq.h"
#include "iutil/plugin.h"
#include "iutil/pluginconfig.h"


void csCommandLineHelper::Help (iPluginConfig* config)
{
  int i;
  for (i = 0 ; ; i++)
  {
    csOptionDescription option;
    if (!config->GetOptionDescription (i, &option))
      break;
    csString opt;
    csStringFast<80> desc;
    csVariant def;
    config->GetOption (i, &def);
    switch (option.type)
    {
      case CSVAR_BOOL:
        opt.Format ("  -[no]%s", option.name);
	desc.Format ("%s (%s) ", option.description, def.GetBool ()
		? "yes" : "no");
	break;
      case CSVAR_CMD:
        opt.Format ("  -%s", option.name);
	desc = option.description;
	break;
      case CSVAR_FLOAT:
        opt.Format ("  -%s=<val>", option.name);
	desc.Format ("%s (%g)", option.description, def.GetFloat ());
	break;
      case CSVAR_LONG:
        opt.Format ("  -%s=<val>", option.name);
	desc.Format ("%s (%ld)", option.description, def.GetLong ());
	break;
      case CSVAR_STRING:
        opt.Format ("  -%s=<val>", option.name);
	desc.Format ("%s (%s)", option.description, def.GetString ()
		? def.GetString () : "none");
	break;
    }
    //@@@????
    csPrintf ("%-21s%s\n", opt.GetData(), desc.GetData());
    //ReportSys (CS_MSG_STDOUT, "%-21s%s\n", opt, desc);
  }
}

void csCommandLineHelper::Help (iObjectRegistry* object_reg,
	iCommandLineParser* cmdline)
{
  csRef<iCommandLineParser> cmd = cmdline;
  if (!cmd)
    cmd = CS_QUERY_REGISTRY (object_reg, iCommandLineParser);
  CS_ASSERT (cmd != 0);

  // First send a global cscmdCommandLineHelp event.
  csRef<iEventQueue> evq (CS_QUERY_REGISTRY (object_reg, iEventQueue));
  if (evq)
  {
    iEventOutlet* evout = evq->GetEventOutlet ();
    CS_ASSERT (evout != 0);
    // We use ImmediateBroadcast here because after processing commandline
    // help the application usually exits. This means there is no chance
    // to actually process the event in the queue.
    evout->ImmediateBroadcast (cscmdCommandLineHelp, 0);
  }

  csRef<iPluginManager> plgmgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  csRef<iPluginIterator> it = plgmgr->GetPlugins ();
  while (it->HasNext ())
  {
    iBase* plug = it->Next ();
    csRef<iPluginConfig> config (SCF_QUERY_INTERFACE (plug, iPluginConfig));
    if (config)
    {
      csRef<iFactory> fact (SCF_QUERY_INTERFACE (plug, iFactory));
      if (fact)
        csPrintf ("Options for %s:\n", fact->QueryDescription ());
      else
        csPrintf ("Options for unknown plugin:\n");
      Help (config);
    }
  }

  //@@@???
  csPrintf (
"General options:\n"
"  -help              this help\n"
"  -video=<s>         the 3D rendering driver (opengl, software, ...)\n"
"  -canvas=<s>        the 2D canvas driver (asciiart, x2d, ...)\n"
"  -plugin=<s>        load the plugin after all others\n"
"  -verbose           be more verbose; print better diagonstic messages\n");
}

bool csCommandLineHelper::CheckHelp (iObjectRegistry* object_reg,
	iCommandLineParser* cmdline)
{
  csRef<iCommandLineParser> cmd = cmdline;
  if (!cmd)
    cmd = CS_QUERY_REGISTRY (object_reg, iCommandLineParser);
  CS_ASSERT (cmd != 0);
  bool rc = cmd->GetOption ("help") != 0;
  return rc;
}
