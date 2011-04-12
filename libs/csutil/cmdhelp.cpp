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

#include "csutil/cmdhelp.h"
#include "csutil/csevent.h"
#include "csutil/snprintf.h"
#include "csutil/sysfunc.h"
#include "iutil/cmdline.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include <../plugins/utilities/movierecorder/lzoconf.h>


csCommandLineHelper::csCommandLineHelper ()
{
  // Create the default 'application' section
  CommandSection section ("");
  commandSections.Push (section);
}

void csCommandLineHelper::PrintTitle (const char* title, unsigned int level)
{
  csString txt = title;
  if (!level) txt = txt.Upcase ();
  csString underline;
  underline.PadRight (txt.Length(), level > 1 ? '-' : '=');
  csPrintf ("\n%s\n%s\n\n", txt.GetData (), underline.GetData());
}

void csCommandLineHelper::PrintOption (const char* name, const char* description, const csVariant& value)
{
  csOptionDescription opdescription;
  opdescription.name = name;
  opdescription.description = description;
  opdescription.type = value.GetType ();

  PrintOption (opdescription, value);
}

void csCommandLineHelper::PrintOption (const csOptionDescription& option, const csVariant& value)
{
  csString opt;
  csStringFast<80> desc;

  // TODO: format nicely the description on several lines

  switch (option.type)
    {
    case CSVAR_BOOL:
      opt.Format ("  -[no]%s", option.name.GetData ());
      desc.Format ("%s (%s) ", option.description.GetData (), value.GetBool ()
		   ? "yes" : "no");
      break;
    case CSVAR_CMD:
      opt.Format ("  -%s", option.name.GetData ());
      desc = option.description.GetData ();
      break;
    case CSVAR_FLOAT:
      opt.Format ("  -%s=<float>", option.name.GetData ());
      desc.Format ("%s (%g)", option.description.GetData (), value.GetFloat ());
      break;
    case CSVAR_LONG:
      opt.Format ("  -%s=<int>", option.name.GetData ());
      desc.Format ("%s (%ld)", option.description.GetData (), value.GetLong ());
      break;
    case CSVAR_STRING:
      opt.Format ("  -%s=<string>", option.name.GetData ());
      if (value.GetString () && strlen (value.GetString ()))
	desc.Format ("%s (%s)", option.description.GetData (), value.GetString ());
      else
	desc.Format ("%s", option.description.GetData ());
      break;
    }

  //@@@????
  csPrintf ("%-21s%s\n", opt.GetData(), desc.GetData());
  //ReportSys (CS_MSG_STDOUT, "%-21s%s\n", opt, desc);
}

void csCommandLineHelper::Help (iObjectRegistry* object_reg,
	iCommandLineParser* cmdline)
{
  csRef<iCommandLineParser> cmd = cmdline;
  if (!cmd)
    cmd = csQueryRegistry<iCommandLineParser> (object_reg);
  CS_ASSERT (cmd != 0);

  // First send a global csevCommandLineHelp event.
  csRef<iEventQueue> evq (csQueryRegistry<iEventQueue> (object_reg));
  if (evq)
  {
    iEventOutlet* evout = evq->GetEventOutlet ();
    CS_ASSERT (evout != 0);
    // We use ImmediateBroadcast here because after processing commandline
    // help the application usually exits. This means there is no chance
    // to actually process the event in the queue.
    evout->ImmediateBroadcast (csevCommandLineHelp(object_reg), 0);
  }

  // Print the options of all plugins exhibiting the iPluginConfig interface
  csRef<iPluginManager> plgmgr = csQueryRegistry<iPluginManager> (object_reg);
  csRef<iPluginIterator> it = plgmgr->GetPluginInstances ();
  while (it->HasNext ())
  {
    iBase* plug = it->Next ();
    csRef<iPluginConfig> config (scfQueryInterface<iPluginConfig> (plug));
    if (config)
    {
      csRef<iFactory> fact (scfQueryInterface<iFactory> (plug));
      if (fact)
	PrintTitle (fact->QueryDescription (), 1);
      else
	PrintTitle ("Options for unknown plugin", 1);

      int i;
      for (i = 0 ; ; i++)
      {
	csOptionDescription option;
	if (!config->GetOptionDescription (i, &option))
	  break;

	csVariant value;
	config->GetOption (i, &value);

	PrintOption (option, value);
      }
    }
  }

  // Print general options for CS
  PrintTitle ("General options", 1);
  PrintOption ("help", "Print this help", csVariant ());
  PrintOption ("cfgfile", "Load a configuration file", csVariant (""));
  PrintOption ("cfgset", "Specify a configuration setting", csVariant (""));
  PrintOption ("plugin", "Load the plugin after all others", csVariant (""));
  PrintOption ("verbose", "Be more verbose; print better diagnostic messages", csVariant ());
}

bool csCommandLineHelper::CheckHelp (iObjectRegistry* object_reg,
	iCommandLineParser* cmdline)
{
  csRef<iCommandLineParser> cmd = cmdline;
  if (!cmd)
    cmd = csQueryRegistry<iCommandLineParser> (object_reg);
  CS_ASSERT (cmd != 0);
  bool rc = cmd->GetOption ("help") != 0;
  return rc;
}

size_t csCommandLineHelper::AddCommandLineSection (const char* name)
{
  CommandSection section (name);
  return commandSections.Push (section);
}

void csCommandLineHelper::AddCommandLineOption (csOptionDescription& description, csVariant& value,
						size_t section)
{
  Option option;
  option.value = value;
  option.description = description;
  commandSections[section].commandOptions.Push (option);
}

void csCommandLineHelper::AddCommandLineOption (const char* name, const char* description,
						csVariant value, size_t section)
{
  csOptionDescription opdescription;
  opdescription.name = name;
  opdescription.description = description;
  opdescription.type = value.GetType ();

  Option option;
  option.value = value;
  option.description = opdescription;
  commandSections[section].commandOptions.Push (option);
}

void csCommandLineHelper::AddCommandLineExample (const char* example)
{
  examples.Push (example);
}

void csCommandLineHelper::PrintApplicationHelp (iObjectRegistry* registry,
						const char* command,
						const char* usage,
						const char* description) const
{
  // Print main usage of the application
  PrintTitle ("Description");
  // TODO: format the description nicely with word wrapping at the end of the line
  csPrintf ("%s\n", description);

  PrintTitle ("Usage");
  csPrintf ("\t%s\n", usage);

  if (examples.GetSize ())
  {
    PrintTitle ("Usage examples");
    for (size_t i = 0; i < examples.GetSize (); i++)
      csPrintf ("\t%s\n", examples[i]);
  }

  // Print specific options for the application
  PrintTitle ("Available options");
  for (size_t i = 0; i < commandSections.GetSize (); i++)
  {
    const CommandSection& section = commandSections[i];

    if (section.commandOptions.GetSize ())
    {
      if (!i)
	PrintTitle (csString ().Format ("Specific options for %s", command), 1);
      else
	PrintTitle (section.name.GetData (), 2);
    }

    for (csArray<Option>::ConstIterator it = section.commandOptions.GetIterator ();
	 it.HasNext (); )
    {
      const Option& option = it.Next ();
      PrintOption (option.description, option.value);
    }
  }

  // Print plugin specific options
  Help (registry);
}
