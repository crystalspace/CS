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
#include "csperl5.h"
#include "csutil/scf.h"
#include "iutil/objreg.h"
#include "csutil/csstring.h"
#include "iutil/vfs.h"
#include "iutil/databuff.h"

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
  scfParent->reporter = CS_QUERY_REGISTRY (objreg, iReporter);

  if (scfParent->reporter) return true;
  else return false;
}

bool csPerl5::Initialize (iObjectRegistry *objreg)
{
  csRef<iVFS> vfs = CS_QUERY_REGISTRY (objreg, iVFS);
  if (! vfs)
  {
    reporter->ReportError ("crystalspace.perl5.init.plugins",
      "Can't find VFS plugin");
    return false;
  }

  const char *vfsinc = "this/scripts/perl5";
  csRef<iDataBuffer> incbuff = vfs->GetRealPath (vfsinc);
  if (! incbuff)
  {
    reporter->ReportError ("crystalspace.perl5.init.load",
      "VFS: Can't find %s directory; is it missing?", vfsinc);
    return false;
  }

  perl = perl_alloc ();
  if (! perl)
  {
    reporter->ReportError ("crystalspace.perl5.init.alloc",
      "perl5: Can't allocate memory for perl interpreter");
    return false;
  }
  perl_construct (perl);

  char *realinc = (char *) incbuff->GetData ();
  char *argv [] = { "perl5", "-T", "-I", realinc, "-e", "0" };
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

  SV* error = get_sv ("@", FALSE);
  if (SvTRUE (error))
  {
    reporter->ReportError ("crystalspace.perl5.run.fatal",
      "perl: %s", SvPV_nolen (error));
    return false;
  }
  else
    return true;
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

