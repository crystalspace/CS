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

#ifndef __GENMESHIFY_H__
#define __GENMESHIFY_H__

#include "csutil/refcount.h"

namespace genmeshify
{
  class App : public csRefCount
  {
    void CommandLineHelp();
  public:
    App (iObjectRegistry *objectRegistry);
    ~App ();

    // Initialize and load plugins we want
    bool Initialize ();

    // Report an error/warning, always returns false
    bool Report (const char* msg, ...);
    void Report (int severity, const char* msg, ...);

    // Parse the commandline and process any files specified
    bool ProcessFiles ();

    // Public members
    csRef<iDocumentSystem> docSystem;
    csRef<iEngine> engine;
    csRef<iImageIO> imageIO;
    csRef<iLoader> loader;
    csRef<iPluginManager> pluginManager;
    csRef<iVFS> vfs;
    csRef<iSyntaxService> synsrv;
    csRef<iSaver> saver;
    csRef<iShaderVarStringSet> svStrings;
    iObjectRegistry *objectRegistry;
  };
}

#endif // __GENMESHIFY_H__
