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

extern "C" {
#include "Python.h"
}
#include "cssysdef.h"
#include "cspython.h"
#include "csutil/csstring.h"

CS_IMPLEMENT_PLUGIN

IMPLEMENT_IBASE(csPython)
  IMPLEMENTS_INTERFACE(iScript)
  IMPLEMENTS_INTERFACE(iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY(csPython)

EXPORT_CLASS_TABLE(cspython)
  EXPORT_CLASS(csPython, "crystalspace.script.python",
    "Crystal Space Script Python")
EXPORT_CLASS_TABLE_END

csPython *thisclass=NULL;

csPython::csPython(iBase *iParent) :Sys(NULL), Mode(MSG_INITIALIZATION)
{
  CONSTRUCT_IBASE(iParent);
}

csPython::~csPython()
{
  Mode=MSG_INTERNAL_ERROR;
  Py_Finalize();
  Sys=NULL;
  thisclass=NULL;
}

bool csPython::Initialize(iSystem* iSys)
{
  Sys=iSys;
  thisclass=this;

  Py_SetProgramName("Crystal Space -- Python");
  Py_Initialize();
  InitPytocs();

  char path[256];
  iSys->GetInstallPath (path, 255);
  if (path[0] == 0) strcpy (path, "./");

  if(!LoadModule("sys"))
    return false;
  csString cmd;
  cmd << "sys.path.append('" << path << "scripts/python/')";
  if(!RunText(cmd))
    return false;
#if 0 // Enable this to send python script prints to the crystal space console.
  if(!LoadModule("cshelper"))
    return false;
#endif   
  if(!LoadModule("pdb"))
    return false;
  if(!LoadModule("cspacec"))
    return false;
  if(!LoadModule("cspace"))
    return false;

  Mode=MSG_STDOUT;
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

bool csPython::Store(const char* type, const char* name, void* data)
{
  Storage=data;
  csString s;
  s << name << "=cspacec.ptrcast(cspacec.GetMyPtr(), '" << type << "')";
  RunText(s);
  Storage=NULL;
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
  if(Error)
    Sys->Printf(MSG_FATAL_ERROR, "CrystalScript Error: %s\n", msg);
  else
    Sys->Printf(Mode, "%s", msg);
}
