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
#include "ivaria/script.h"
#include "ivaria/reporter.h"
#include "ivaria/stdrep.h"

#include <stdio.h>

CS_IMPLEMENT_APPLICATION

iObjectRegistry *objreg;

char testscript [] =
"  $text = 'Jorrit does hail from all places';\n"
"  $v = new csVector3 (4, 2, 0);\n"
"  print join (' ', (split (' ', $text)) [$v->{x}, $v->{y}, $v->{z}]);\n";

int main (int argc, char *argv[], char *env[])
{
  objreg = csInitializer::CreateEnvironment (argc, argv);
  if (! objreg)
  {
    fprintf (stderr, "Failed to create environment!\n");
    return 1;
  }

  bool plugins_ok = csInitializer::RequestPlugins (objreg,
    CS_REQUEST_REPORTER,
    CS_REQUEST_REPORTERLISTENER,
    CS_REQUEST_PLUGIN ("crystalspace.script.perl5", iScript),
    CS_REQUEST_END);
  if (! plugins_ok)
  {
    fprintf (stderr, "Failed to load plugins!\n");
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
      fprintf (stderr, "Failed to find perl5 plugin!\n");
      return 3;
    }

    bool script_ok = script->Initialize (objreg);
    if (! script_ok)
    {
      fprintf (stderr, "Failed to initialize perl5 plugin!\n");
      return 4;
    }

    bool module_ok = script->LoadModule ("cspace");
    if (! module_ok)
    {
      fprintf (stderr, "Failed to load perl5 cspace module!\n");
      return 5;
    }

    csInitializer::OpenApplication (objreg);

    printf ("\nRunning perl script:\n%s\n", testscript);
    script->RunText (testscript);
    printf ("\n\nTest script finished.\n\n");
  }

  csInitializer::DestroyApplication (objreg);
  return 0;
}

