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

#include <Python.h>
#include "cssysdef.h"
#include "csutil/csstring.h"
#include "cspython.h"

// The NextStep compiler does not allow C++ expressions in `extern "C"'
// functions.  This thin cover function works around that limitation.
static inline csPython* shared_cspython() { return csPython::shared_instance; }

extern "C" PyObject* pytocs_printout(PyObject *self, PyObject* args)
{
  char *command;

  (void)self;
  if (PyArg_ParseTuple(args, "s", &command))
    shared_cspython()->Print(false, command);

  Py_INCREF(Py_None);
  return Py_None;
}

extern "C" PyObject* pytocs_printerr(PyObject *self, PyObject* args)
{
  char *command;

  (void)self;
  if (PyArg_ParseTuple(args, "s", &command))
    shared_cspython()->Print(true, command);

  Py_INCREF(Py_None);
  return Py_None;
}

PyMethodDef PytocsMethods[] =
{
  { "printout", pytocs_printout, METH_VARARGS, "" },
  { "printerr", pytocs_printout, METH_VARARGS, "" },
  { 0, 0, 0, "" }
};

extern "C" void SWIG_init_cspace();

void InitPytocs()
{
  Py_InitModule("pytocs", PytocsMethods);
  SWIG_init_cspace();
}
