/*
    Copyright (C) 1999 by Brandon Ehle <azverkan@yahoo.com>

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

#include <Python.h>
#include <stdio.h>

#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "csutil/syspath.h"
#include "csutil/csstring.h"
#include "csutil/util.h"
#include "iutil/cmdline.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"

#include "cspython.h"
#include "pytocs.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE(csPython)
  SCF_IMPLEMENTS_INTERFACE(iScript)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csPython::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csPython::eiEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY(csPython)


csPython* csPython::shared_instance = 0;

csPython::csPython(iBase *iParent) :
  object_reg(0), Mode(CS_REPORTER_SEVERITY_NOTIFY), use_debugger(false)
{
  SCF_CONSTRUCT_IBASE(iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiEventHandler);
  shared_instance = this;
}

csPython::~csPython()
{
  csRef<iEventQueue> queue = CS_QUERY_REGISTRY(object_reg, iEventQueue);
  if (queue.IsValid())
    queue->RemoveListener(&scfiEventHandler);
  Mode = CS_REPORTER_SEVERITY_BUG;
  Py_Finalize();
  object_reg = 0;
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiEventHandler);
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

extern "C"
{
  PyObject* csWrapTypedObject(void *objptr, const char *tagtype, int own);
  void SWIG_init_cspace();
}

static csString const& path_append(csString& path, char const* component)
{
  char c;
  size_t n = path.Length();
  if (n > 0 && (c = path[n - 1]) != '/' && c != '\\')
    path << '/';
  path << component;
  return path;
}

bool csPython::Initialize(iObjectRegistry* object_reg)
{
  csPython::object_reg = object_reg;

  csRef<iCommandLineParser> cmdline(
    CS_QUERY_REGISTRY(object_reg, iCommandLineParser));
  bool const reporter = cmdline->GetOption("python-enable-reporter") != 0;
  use_debugger = cmdline->GetOption("python-enable-debugger") != 0;

  Py_SetProgramName("Crystal Space -- Python");
  Py_Initialize();
  InitPytocs();

  if (!LoadModule ("sys")) return false;

  csString cmd;
  csRef<iVFS> vfs(CS_QUERY_REGISTRY(object_reg, iVFS));
  if (vfs.IsValid())
  {
    csRef<iStringArray> paths(vfs->GetRealMountPaths("/scripts"));
    for (size_t i = 0, n = paths->Length(); i < n; i++)
    {
      csString path = paths->Get(i);
      cmd << "sys.path.append('" << path_append(path, "python") << "')\n";
    }
  }

  csString cfg(csGetConfigPath());
  cmd << "sys.path.append('" << path_append(cfg, "scripts/python") << "')\n";
  if (!RunText (cmd)) return false;

  if (reporter && !LoadModule ("cshelper")) return false;
  if (use_debugger && !LoadModule ("pdb")) return false;
  if (!LoadModule ("cspace")) return false;

  Mode = CS_REPORTER_SEVERITY_NOTIFY;

  // Store the object registry pointer in 'cspace.object_reg'.
  Store("cspace.object_reg", object_reg, (void *) "iObjectRegistry *");

  csRef<iEventQueue> queue = CS_QUERY_REGISTRY(object_reg, iEventQueue);
  if (queue.IsValid())
    queue->RegisterListener(&scfiEventHandler, CSMASK_Broadcast);

  return true;
}

bool csPython::HandleEvent(iEvent& e)
{
  bool handled = false;
  if (e.Type == csevBroadcast && e.Command.Code == cscmdCommandLineHelp)
  {
#undef indent
#define indent "                     "
    printf("Options for csPython plugin:\n"
	   "  -python-enable-reporter\n"
	   indent "Redirect sys.stdout and sys.stderr to iReporter\n"
	   "  -python-enable-debugger\n"
	   indent "When Python exception is thrown, launch Python debugger\n");
#undef indent
    handled = true;
  }
  return handled;
}

void csPython::ShowError()
{
  if(PyErr_Occurred())
  {
    PyErr_Print();
    Print(true, "ERROR!\n");
  }
}

bool csPython::RunText(const char* Text)
{
  // Apparently, some Python installations have bad prototype, so const_cast<>.
  bool ok = !PyRun_SimpleString(CS_CONST_CAST(char*,Text));
  if(!ok && use_debugger)
    PyRun_SimpleString(CS_CONST_CAST(char*,"pdb.pm()"));
  ShowError();
  return ok;
}

bool csPython::Store(const char* name, void* data, void* tag)
{
  PyObject * obj = csWrapTypedObject(data, (const char*)tag, 0);
  char *mod_name = csStrNew(name);
  char * var_name = strrchr(mod_name, '.');
  if(!var_name)
    return false;
  *var_name = 0;
  ++var_name;
  PyObject * module = PyImport_ImportModule(mod_name);
  PyModule_AddObject(module, (char*)var_name, obj);

  delete[] mod_name;

  return true;
}

bool csPython::LoadModule(const char* name)
{
  csString s;
  s << "import " << name;
  return RunText(s);
}

void csPython::Print(bool Error, const char *msg)
{
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
  if (!rep)
    csPrintf ("%s\n", msg);
  else
  {
    if(Error)
      rep->Report (CS_REPORTER_SEVERITY_ERROR, "crystalspace.script.python",
      	"CrystalScript Error: %s", msg);
    else
      rep->Report (Mode, "crystalspace.script.python",
      	"%s", msg);
  }
}
