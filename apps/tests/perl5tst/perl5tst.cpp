/*
    Copyright (C) 2002 by Mat Sutcliffe <oktal@gmx.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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
#include "csutil/ref.h"
#include "cstool/initapp.h"
#include "csutil/cmdhelp.h"
#include "iutil/objreg.h"
#include "iutil/string.h"
#include "ivaria/script.h"
#include "ivaria/reporter.h"
#include "ivaria/stdrep.h"

#include <stdio.h>

CS_IMPLEMENT_APPLICATION

int main (int argc, char *argv[])
{
  iObjectRegistry* objreg = csInitializer::CreateEnvironment (argc, argv);
  if (! objreg)
  {
    csFPrintf (stderr, "Failed to create environment!\n");
    return 1;
  }

  bool plugins_ok = csInitializer::RequestPlugins (objreg,
    CS_REQUEST_REPORTER,
    CS_REQUEST_REPORTERLISTENER,
    CS_REQUEST_PLUGIN ("crystalspace.script.perl5", iScript),
    CS_REQUEST_END);
  if (! plugins_ok)
  {
    csFPrintf (stderr, "Failed to load plugins!\n");
    return 2;
  }

  if (csCommandLineHelper::CheckHelp (objreg))
  {
    csCommandLineHelper::Help (objreg);
    return 0;
  }

  {
    csRef<iScript> script = CS_QUERY_REGISTRY (objreg, iScript);
    if (! script)
    {
      csFPrintf (stderr, "Failed to find perl5 plugin!\n");
      return 3;
    }

    bool module_ok = script->LoadModule ("cspace");
    if (! module_ok)
    {
      csFPrintf (stderr, "Failed to load perl5 cspace module!\n");
      return 4;
    }

    csInitializer::OpenApplication (objreg);

    char const *text = "Hello, world!";
    csPrintf ("Testing Store:\n");
    script->Store ("text", text);
    csPrintf ("text = %s\n", text);

    csRef<iString> rettext;
    csPrintf ("Testing Retrieve:\n");
    script->Retrieve ("text", rettext);
    csPrintf ("text = %s\n", rettext->GetData());

    puts("");

    csPrintf ("Testing NewObject:\n");
    csRef<iScriptObject> obj =
      script->NewObject ("cspace::csVector3", "%g%g%g", 1.0f, 2.0f, 3.0f);
    csPrintf ("new csVector3 (1.0f, 2.0f, 3.0f)\n");

    puts("");

    csPrintf ("Testing Get:\n");
    float x = 0.0f, y = 0.0f, z = 0.0f;
    obj->Get ("x", x);
    obj->Get ("y", y);
    obj->Get ("z", z);
    csPrintf ("x=%g, y=%g, z=%g\n", x, y, z);

    puts("");

    csPrintf("Testing Set:\n");
    obj->Set ("x", 3.0f);
    obj->Set ("y", 2.0f);
    obj->Set ("z", 1.0f);

    obj->Get ("x", x);
    obj->Get ("y", y);
    obj->Get ("z", z);
    csPrintf ("x=%g, y=%g, z=%g\n", x, y, z);
  }

  csInitializer::DestroyApplication (objreg);
  return 0;
}
