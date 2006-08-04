/*
  Copyright (C) 2005 by Marten Svanfeldt
            (C) 2006 by Frank Richter

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

#include "crystalspace.h"

#include "genmeshify.h"
#include "processor.h"

CS_IMPLEMENT_APPLICATION

namespace genmeshify
{
  App::App (iObjectRegistry *objectRegistry)
    : objectRegistry (objectRegistry)
  {
  }

  App::~App ()
  {
  }

  bool App::Initialize ()
  {
    // Load config
    if (!csInitializer::SetupConfigManager (objectRegistry,0))
      return Report ("Cannot setup config manager!");

    // Get plugins
    if (!csInitializer::RequestPlugins (objectRegistry,
            CS_REQUEST_ENGINE,
            CS_REQUEST_IMAGELOADER,
            CS_REQUEST_LEVELLOADER,
            CS_REQUEST_NULL3D,            
            CS_REQUEST_VFS,
            CS_REQUEST_END))
      return Report ("Cannot load plugins!");

    // Check for commandline help.
    if (csCommandLineHelper::CheckHelp (objectRegistry))
    {
      csCommandLineHelper::Help (objectRegistry);
      return true;
    }

    // Get the plugins wants to use
    engine = csQueryRegistry<iEngine> (objectRegistry);
    if (!engine) return Report ("No iEngine!");
    engine->SetSaveableFlag (true);

    imageIO = csQueryRegistry<iImageIO> (objectRegistry);
    if (!imageIO) return Report ("No iImageIO!");

    loader = csQueryRegistry<iLoader> (objectRegistry);
    if (!loader) return Report ("No iLoader!");
    
    synsrv = csQueryRegistry<iSyntaxService> (objectRegistry);
    if (!synsrv) return Report ("No iSyntaxService!");

    saver = csQueryRegistryOrLoad<iSaver> (objectRegistry,
      "crystalspace.level.saver");
    if (!saver) return Report ("No iSaver!");
    
    vfs = csQueryRegistry<iVFS> (objectRegistry);
    if (!vfs) return Report ("No iVFS!");

    strings = csQueryRegistryTagInterface<iStringSet> (
      objectRegistry, "crystalspace.shared.stringset");
    if (!strings) return Report ("No shared string set!");

    // Open the systems
    if (!csInitializer::OpenApplication (objectRegistry))
      return Report ("Error opening system!");

    // For now, force the use of TinyXML to be able to write
    docSystem.AttachNew (new csTinyDocumentSystem);

    return true;
  }

  bool App::Report (const char* msg, ...)
  {
    va_list arg;
    va_start (arg, msg);
    csReportV (objectRegistry, CS_REPORTER_SEVERITY_ERROR, 
        "crystalspace.application.genmeshify", msg, arg);
    va_end (arg);
    return false;
  }

  bool App::ProcessFiles ()
  {
    //Parse cmd-line
    csRef<iCommandLineParser> cmdline = csQueryRegistry<iCommandLineParser>
      (objectRegistry);

    int cmd_idx = 0;
    int map_idx = 0;
    while (true)
    {
      const char *val = cmdline->GetName (cmd_idx++);
      if (!val)
      {
        if (map_idx > 0)
          break;
        else
          return Report ("Please specify a level (either zip or VFS dir)!");
      }

      map_idx++;

      Processor processor (this, val);
      if (!processor.Process ()) return false;
      //scene->AddFile (val);
    }
    return true;
  }

}

int main (int argc, char* argv[])
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return 1;

  {
    // Load up the global object
    csRef<genmeshify::App> app;
    app.AttachNew (new genmeshify::App (object_reg));

    // Initialize it
    if (!app->Initialize ()) return 1;

    // Run it
    if (!app->ProcessFiles ()) return 1;
  }

  // Remove it
  csInitializer::DestroyApplication (object_reg);

  return 0;
}
