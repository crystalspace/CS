
#include "cssysdef.h"
#include "csutil/ref.h"
#include "cstool/initapp.h"
#include "csutil/cmdhelp.h"
#include "iutil/objreg.h"
#include "ivaria/script.h"

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

