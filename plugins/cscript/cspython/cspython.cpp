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

IMPLEMENT_FACTORY(csPython)

EXPORT_CLASS_TABLE(cspython)
	EXPORT_CLASS(csPython, "crystalspace.script.python", "Crystal Space Script Python")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE(csPython)
	IMPLEMENTS_INTERFACE(iScript)
IMPLEMENT_IBASE_END

csPython *thisclass=NULL;

csPython::csPython(iBase *iParent)
    :Sys(NULL), Mode(MSG_INITIALIZATION) {
			CONSTRUCT_IBASE(iParent);
    }

csPython::~csPython() {
  Mode=MSG_INTERNAL_ERROR;

  Py_Finalize();

  Sys=NULL;
  thisclass=NULL;
}

bool csPython::Initialize(iSystem* iSys) {
  Sys=iSys;
  thisclass=this;

  Py_SetProgramName("Crystal Space -- Python");
  Py_Initialize();
  InitPytocs();

  if(!LoadModule("sys"))
    return 0;
  if(!RunText("sys.path.append('./scripts/python/')"))
    return 0;
  if(!LoadModule("cshelper"))
    return 0;
  if(!LoadModule("pdb"))
    return 0;
  if(!LoadModule("cspacec"))
    return 0;
  if(!LoadModule("cspace"))
    return 0;

  Mode=MSG_CONSOLE;

  return 1;
}

void csPython::ShowError() {
  if(PyErr_Occurred()) {
    PyErr_Print();
    Print(1, "ERROR!\n");
  }
}

bool csPython::RunText(const char* Text) {
  csString str(Text);
  bool worked=!PyRun_SimpleString(str.GetData());
  if(!worked) 
    PyRun_SimpleString("pdb.pm()");
    
  ShowError();
    
  return worked;
}

bool csPython::Store(const char* type, const char* name, void* data) {
	Storage=data;
	RunText(csString(name)+"=cspacec.ptrcast(cspacec.GetMyPtr(), '"+type+"')");
	Storage=NULL;
	return 1;
}

bool csPython::LoadModule(const char* name) {
  return RunText(csString("import ")+name);
}

void csPython::Print(bool Error, const char *msg) {
  if(Error) {
    Sys->Printf(MSG_WARNING, csString("CrystalScript Error: ")+msg);
  } else {
    Sys->Printf(Mode, msg);
  }
}

