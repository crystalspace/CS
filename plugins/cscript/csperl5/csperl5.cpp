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

#include <stdlib.h>

extern "C" void xs_init (pTHX); // defined in csperlxs.c

SCF_IMPLEMENT_IBASE (csPerl5)
  SCF_IMPLEMENTS_INTERFACE (iScript)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csPerl5::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csPerl5)


csPerl5::csPerl5 (iBase *parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
}

bool csPerl5::Init (iObjectRegistry *objreg)
{
  reporter = CS_QUERY_REGISTRY (objreg, iReporter);
  if (! reporter) return false;

  csRef<iVFS> vfs = CS_QUERY_REGISTRY (objreg, iVFS);
  if (! vfs)
  {
    reporter->ReportError ("crystalspace.script.perl5.init.plugins",
      "Can't find VFS plugin");
    return false;
  }

  const char *vfsinc = "/scripts/perl5";
  csRef<iDataBuffer> incbuff = vfs->GetRealPath (vfsinc);
  if (! incbuff)
  {
    reporter->ReportError ("crystalspace.script.perl5.init.load",
      "VFS: Can't find %s directory; is it missing?", vfsinc);
    return false;
  }

  my_perl = perl_alloc ();
  if (my_perl == 0)
  {
    reporter->ReportError ("crystalspace.script.perl5.init.alloc",
      "perl5: Can't allocate memory for perl interpreter");
    return false;
  }
  perl_construct (my_perl);

  char *realinc = (char *) incbuff->GetData ();
  char *argv [] = { "perl5", "-T", "-I", realinc, "-e", "0" };
  int const argc = sizeof(argv) / sizeof(argv[0]);
  perl_parse (my_perl, xs_init, argc, argv, 0);
  perl_run (my_perl);
  return true;
}

csPerl5::~csPerl5 ()
{
  if (my_perl != 0)
  {
    perl_destruct (my_perl);
    perl_free (my_perl);
  }
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csPerl5::Initialize (iObjectRegistry *objreg)
{
  reporter->Report (CS_REPORTER_SEVERITY_WARNING,
    "cyrstalspace.script.perl5.init",
    "The iScript::Initialize() call is deprecated.");
  return true;
}

bool csPerl5::Store (const char *name, void *data, void *tag)
{
  reporter->Report (CS_REPORTER_SEVERITY_WARNING,
    "cyrstalspace.script.perl5.store",
    "This old-style iScript::Store() call is deprecated.");
  return false;
}

bool csPerl5::CheckError (const char *caller) const
{
  SV* error = get_sv ("@", FALSE);
  if (SvTRUE (error))
  {
    reporter->ReportError ("crystalspace.script.perl5.fatal",
      "perl: %s: %s", caller, SvPV_nolen (error));
    return false;
  }
  else
    return true;
}

bool csPerl5::RunText (const char *text)
{
  eval_pv (text, FALSE);
  return CheckError ("eval");
}

bool csPerl5::LoadModule (const char *name)
{
  char *text = (char *) malloc (strlen (name) + 5);
  strcpy (text, "use ");
  strcat (text, name);
  bool ok = RunText (text);
  free (text);
  return ok;
}

// Caller is responsible for decref'ing returned SV.
SV* csPerl5::CallV (const char *name, const char *fmt, va_list va, SV *self)
{
  dSP;
  ENTER;
  SAVETMPS;

  PUSHMARK(SP);

  if (self)
    XPUSHs (self);

  while (*fmt++ == '%') switch (*fmt++)
  {
    case 'c': case 'd': case 'i':
    XPUSHs (sv_2mortal (newSViv (va_arg (va, int))));
    break;

    case 'o': case 'u': case 'x': case 'X':
    XPUSHs (sv_2mortal (newSVuv (va_arg (va, unsigned))));
    break;

    case 'f': case 'e': case 'g': case 'E': case 'G':
    XPUSHs (sv_2mortal (newSVnv (va_arg (va, double))));
    break;

    case 's':
    XPUSHs (sv_2mortal (newSVpv (va_arg (va, char *), 0)));
    break;

    case 'p':
    XPUSHs (Query (va_arg (va, iScriptObject *))->self);
    break;

    case 'h': switch (*fmt++)
    {
      case 'd': case 'i':
      XPUSHs (sv_2mortal (newSViv (va_arg (va, int))));
      break;

      case 'o': case 'u': case 'x': case 'X':
      XPUSHs (sv_2mortal (newSVuv (va_arg (va, unsigned))));
      break;

      case 'f': case 'e': case 'g': case 'E': case 'G':
      XPUSHs (sv_2mortal (newSVnv (va_arg (va, double))));
      break;

      default:
      reporter->Report (CS_REPORTER_SEVERITY_WARNING,
        "crystalspace.script.perl5.call",
        "In format string, '%c' found where type specifier expected.", *fmt);
      break;
    }
    break;

    case 'l': switch (*fmt++)
    {
      case 'd': case 'i':
      XPUSHs (sv_2mortal (newSViv (va_arg (va, long))));
      break;

      case 'o': case 'u': case 'x': case 'X':
      XPUSHs (sv_2mortal (newSViv (va_arg (va, unsigned long))));
      break;

      case 'f': case 'e': case 'g': case 'E': case 'G':
      XPUSHs (sv_2mortal (newSVnv (va_arg (va, double))));
      break;

      default:
      reporter->Report (CS_REPORTER_SEVERITY_WARNING,
        "crystalspace.script.perl5.call",
        "In format string, '%c' found where type specifier expected.", *fmt);
      break;
    }
    break;

    case 'L': switch (*fmt++)
    {
      case 'd': case 'i':
      XPUSHs (sv_2mortal (newSViv (va_arg (va, int64))));
      break;

      case 'o': case 'u': case 'x': case 'X':
      XPUSHs (sv_2mortal (newSVuv (va_arg (va, uint64))));
      break;

      default:
      reporter->Report (CS_REPORTER_SEVERITY_WARNING,
        "crystalspace.script.perl5.call",
        "In format string, '%c' found where type specifier expected.", *fmt);
      break;
    }
    break;

    default:
    reporter->Report (CS_REPORTER_SEVERITY_WARNING,
      "crystalspace.script.perl5.call",
      "In format string, '%c' found where type specifier expected.", *fmt);
    break;
  }
  if (*--fmt)
    reporter->Report (CS_REPORTER_SEVERITY_WARNING,
      "crystalspace.script.perl5.call",
      "In format string, '%c' found where '%%' expected.", *fmt);
  PUTBACK;

  if (self)
    call_method (name, G_SCALAR | G_EVAL);
  else
    call_pv (name, G_SCALAR | G_EVAL);
  SPAGAIN;

  bool ok = CheckError ("call");
  SV *ret = POPs;
  SvREFCNT_inc (ret);

  PUTBACK;
  FREETMPS;
  LEAVE;

  if (ok)
    return ret;
  else
    return 0;
}

SCF_IMPLEMENT_IBASE (csPerl5::Object)
  SCF_IMPLEMENTS_INTERFACE (iScriptObject)
  SCF_IMPLEMENTS_INTERFACE (csPerl5Object)
SCF_IMPLEMENT_IBASE_END

csPerl5::Object::Object (csPerl5 *p, SV *s) :
  parent (p), my_perl(parent->my_perl), self(s)
{
  SCF_CONSTRUCT_IBASE (parent);
  SvREFCNT_inc (self);
}

csPerl5::Object::~Object ()
{
  SvREFCNT_dec (self);
  SCF_DESTRUCT_IBASE ();
}

csPerl5::Object* csPerl5::Query (iScriptObject *obj) const
{
  csRef<csPerl5Object> priv = SCF_QUERY_INTERFACE (obj, csPerl5Object);
  if (priv == 0)
    reporter->Report (CS_REPORTER_SEVERITY_WARNING,
      "crystalspace.script.perl5.call", "This iScriptObject isn't from Perl!");
  return CS_STATIC_CAST(csPerl5::Object*,(csPerl5Object*)priv);
}

csRef<iScriptObject> csPerl5::NewObject(const char *type, const char *fmt, ...)
{
  va_list va;
  va_start (va, fmt);
  SV *sv = CallV("new", fmt, va, sv_2mortal (newSVpv (type, 0)));
  va_end (va);

  csRef<iScriptObject> obj;
  if (sv)
  {
    obj.AttachNew(new Object (this, sv));
    SvREFCNT_dec(sv);
  }
  return obj;
}
