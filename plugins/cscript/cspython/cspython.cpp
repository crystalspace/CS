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
#include <stdio.h>
//extern "C" {
#include "Python.h"
//}
#include "cssysdef.h"
#include "cssys/sysfunc.h"
#include "cspython.h"
#include "csutil/csstring.h"
#include "ivaria/reporter.h"
#include "iutil/objreg.h"

CS_IMPLEMENT_PLATFORM_PLUGIN

SCF_IMPLEMENT_IBASE(csPython)
  SCF_IMPLEMENTS_INTERFACE(iScript)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csPython::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY(csPython)

SCF_EXPORT_CLASS_TABLE(cspython)
  SCF_EXPORT_CLASS(csPython, "crystalspace.script.python",
    "Crystal Space Script Python")
SCF_EXPORT_CLASS_TABLE_END

csPython* csPython::shared_instance = NULL;

csPython::csPython(iBase *iParent) :object_reg(NULL),
	Mode(CS_REPORTER_SEVERITY_NOTIFY)
{
  SCF_CONSTRUCT_IBASE(iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  shared_instance = this;
}

csPython::~csPython()
{
  Mode=CS_REPORTER_SEVERITY_BUG;
  Py_Finalize();
  object_reg=NULL;
}

extern "C" {
  extern void SWIG_MakePtr(char *_c, const void *_ptr, char *type);
}

bool csPython::Initialize(iObjectRegistry* object_reg)
{
  csPython::object_reg = object_reg;

  Py_SetProgramName("Crystal Space -- Python");
  Py_Initialize();
  InitPytocs();

  char path[256];
  csGetInstallPath (path, 255);
  if (path[0] == 0) strcpy (path, "./");

  if (!LoadModule ("sys")) return false;
  csString cmd;
  cmd << "sys.path.append('" << path << "scripts/python/')";
  if (!RunText (cmd)) return false;
#if 0 // Enable this to send python script prints to the crystal space console.
  if (!LoadModule ("cshelper")) return false;
#endif
  if (!LoadModule ("pdb")) return false;
  if (!LoadModule ("cspacec")) return false;
  if (!LoadModule ("cspace")) return false;

  Mode = CS_REPORTER_SEVERITY_NOTIFY;

  // Store the object registry pointer in 'cspace.object_reg'.
  Store("cspace.object_reg_ptr", object_reg, (void*)"_iObjectRegistry_p");
  RunText("cspace.object_reg=cspace.iObjectRegistryPtr(cspace.object_reg_ptr)");

  return true;
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
  csString str(Text);
  bool worked=!PyRun_SimpleString(str.GetData());
  if(!worked)
    PyRun_SimpleString("pdb.pm()");
  ShowError();
  return worked;
}

bool csPython::Store(const char* name, void* data, void* tag)
{
  char command[256];
  char sysPtr[100];
  SWIG_MakePtr (sysPtr, data, (char*)tag);
  sprintf (command, "%s=\"%s\"", name, sysPtr);
  RunText (command);

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
  iReporter* rep = CS_QUERY_REGISTRY (object_reg, iReporter);
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
    rep->DecRef ();
  }
}
