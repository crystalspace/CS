/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Brandon Ehle

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

#include "cssysdef.h"
#include "csutil/csstring.h"
#include "cspython.h"

extern "C" {
#include "Python.h"
}

extern "C" PyObject* pytocs_printout(PyObject *self, PyObject* args) {
  char *command;

  (void)self;
  if (PyArg_ParseTuple(args, "s", &command))
    csPython::shared_instance->Print(0, command);
  
  Py_INCREF(Py_None);
  return Py_None;
}

extern "C" PyObject* pytocs_printerr(PyObject *self, PyObject* args) {
  char *command;

  (void)self;
  if (PyArg_ParseTuple(args, "s", &command))
    csPython::shared_instance->Print(1, command);
  
  Py_INCREF(Py_None);
  return Py_None;
}

PyMethodDef PytocsMethods[]={
  {"printout", pytocs_printout, METH_VARARGS, ""},
  {"printerr", pytocs_printout, METH_VARARGS, ""},
  {NULL, NULL, 0, ""}
};

extern "C" void initcspacec();

//TODO temporary
//#include "cssys/system.h"

void InitPytocs() {
  Py_InitModule("pytocs", PytocsMethods);
  initcspacec();
//TODO temporary
  //System=NULL;
}

