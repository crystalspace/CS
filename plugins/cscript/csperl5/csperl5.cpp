
#include "cssysdef.h"
#include "csperl5.h"
#include "csutil/scf.h"
#include "iutil/objreg.h"
#include "csutil/csstring.h"
#include "cssys/sysfunc.h"

#include <string.h>

extern "C" void xs_init (); // defined in csperlxs.c

SCF_IMPLEMENT_IBASE (csPerl5)
  SCF_IMPLEMENTS_INTERFACE (iScript)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csPerl5::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csPerl5)

SCF_EXPORT_CLASS_TABLE (csperl5)
  SCF_EXPORT_CLASS (csPerl5,
  "crystalspace.script.perl5",
  "Crystal Space Perl v5 Scripting Plugin")
SCF_EXPORT_CLASS_TABLE_END

csPerl5::csPerl5 (iBase *parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
}

bool csPerl5::eiComponent::Initialize (iObjectRegistry *objreg)
{
  return true;
}

bool csPerl5::Initialize (iObjectRegistry *objreg)
{
  perl = perl_alloc ();
  if (! perl) return false;
  perl_construct (perl);

  char incdir [128];
  csGetInstallPath (incdir, 96);
  strcat (incdir, "scripts/perl5");

  char *argv [] = { "perl5", "-I", incdir, "-e", "0" };
  int argc = 5;
  perl_parse (perl, xs_init, argc, argv, NULL);
  perl_run (perl);
  return true;
}

csPerl5::~csPerl5 ()
{
  perl_destruct (perl);
  perl_free (perl);
}

bool csPerl5::RunText (const char *text)
{
  eval_pv (text, FALSE);

  return true; //TODO: add error checking
}

bool csPerl5::LoadModule (const char *name)
{
  return RunText ((csString ("require ") + name).GetData ());
}

bool csPerl5::Store (const char *name, void *data, void *tag)
{
  switch (* (const char *) tag)
  {
    case 'i':
    sv_setiv (get_sv (name, TRUE), * (int *) data);
    break;

    case 'f':
    sv_setnv (get_sv (name, TRUE), * (float *) data);
    break;

    case 's':
    sv_setpv (get_sv (name, TRUE), (const char *) data);
    break;

    default:
    return false;
  }
  return true;
}

