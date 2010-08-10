/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Copyright (C) 2007 by Pablo Martin <caedes@grupoikusnet.com>
    Originally written by Brandon Ehle

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

#ifndef __CS_CSPYTHON_H__
#define __CS_CSPYTHON_H__

#include <Python.h>
#include "ivaria/script.h"
#include "ivaria/reporter.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/csinput.h"
#include "csplugincommon/script/scriptcommon.h"

CS_PLUGIN_NAMESPACE_BEGIN(cspython)
{

PyObject* csWrapTypedObject(void* objectptr, const char *typetag,
  int own);

struct csPythonValue : public virtual iBase
{
  SCF_INTERFACE (csPythonValue,1,0,0);
  virtual ~csPythonValue() {}
};

struct csPythonObject : public virtual iBase
{
  SCF_INTERFACE (csPythonObject,1,0,0);
  virtual ~csPythonObject() {}
};

class csPython : public scfImplementationExt2<csPython,
					      csScriptCommon,
					      iComponent,
                                              iEventHandler> 
{
protected:
  class Object;

  class Value : public scfImplementation2<csPython::Value,iScriptValue,csPythonValue>
  {
    csPython *parent;
  protected:
    friend class csPython;
    friend class csPython::Object;
    PyObject *self;
  public:
    Value (csPython *p, PyObject *s, bool incref = true)
    : scfImplementationType (this), parent(p), self(s)
    {
      if (incref && self)
      {
	Py_INCREF (self); 
      }
    }
    virtual ~Value ()
    { 
      if (self)
      {
	Py_DECREF (self); 
      }
    }

    iScript* GetScript() { return parent; }

    unsigned GetTypes () const;
    int GetInt () const;
    double GetDouble () const;
    float GetFloat () const;
    bool GetBool () const;
    const csRef<iString> GetString () const;
    csRef<iScriptObject> GetObject () const;
  }; // csPython::Value

  class Object : public scfImplementationExt1<csPython::Object, csScriptObjectCommon, csPythonObject>
  {
    csPython *parent;

  protected:
    friend class csPython;
    friend class csPython::Value;
    
    PyObject *self;

  public:
    Object (csPython *p, PyObject *s, bool incref = true)
    : scfImplementationType (this), parent(p), self(s)
    { 
      if (incref && self) 
      {
	Py_INCREF (self); 
      }
    }
    virtual ~Object () 
    { 
      if (self) 
      {
	Py_DECREF (self); 
      }
    }

    iScript* GetScript() { return parent; }

    const csRef<iString> GetClass () const;
    bool IsA (const char *type) const;

    void* GetPointer ();

    csPtr<iScriptValue> Call (const char *name, const csRefArray<iScriptValue> &args = csRefArray<iScriptValue> ());

    bool Set (const char *name, iScriptValue *value);
    csPtr<iScriptValue> Get (const char *name);
  }; // csPython::Object

  friend class Value;
  friend class Object;
  
  Object* Query (iScriptObject *obj) const;
  Value* Query (iScriptValue *val) const;

  csPtr<iScriptValue> CallBody (PyObject *func,
    const csRefArray<iScriptValue> &args, bool bound_func = false);
  PyObject* py_main;
  // FindObject doesnt incref the returned PyObject
  PyObject *FindObject(const char *url, bool do_errors=true);
  // difference of FindFunction with FindObject is it returns an
  // increfed PyObject
  PyObject *FindFunction(const char *url, bool do_errors=true);
  void Test();
  // Augment pythonpath with cspython specific paths
  bool AugmentPythonPath();
  // Add a vfs path to pythonpath, with an optional subpath to search for
  bool AddVfsPythonPath(const char *vfspath, const char *subpath=0);
  // Add a real path to pythonpath, with an optional subpath to search for
  bool AddRealPythonPath(csString &path, const char *subpath=0);
  // Load configuration options
  void LoadConfig();
  void LoadComponents();
public:
  csPython(iBase *iParent);
  virtual ~csPython();

  static csPython* shared_instance;
  iObjectRegistry* object_reg;
  csRef<iReporter> reporter;
  int Mode;
  bool use_debugger;

  virtual bool Initialize(iObjectRegistry*); // implements iComponent

  virtual bool RunText(const char *Text);
  virtual bool LoadModule(const char *Text);
  virtual bool LoadModule(const char *path, const char *name);
  virtual bool LoadModuleNative(const char *path, const char *name);

  virtual bool Store (const char *name, iScriptValue *value);
  virtual bool Store (const char *name, csPtr<iScriptValue> value) // not from iScript
  {
    return Store(name,(iScriptValue*)csRef<iScriptValue>(value));
  }
  virtual csPtr<iScriptValue> Retrieve (const char *name);
  virtual bool Remove (const char *name);

  virtual csPtr<iScriptValue> Call (const char *name,
	const csRefArray<iScriptValue> &args = csRefArray<iScriptValue> ());
  virtual csPtr<iScriptObject> New (const char *type,
	const csRefArray<iScriptValue> &args = csRefArray<iScriptValue> ());

  virtual csPtr<iScriptValue> RValue (int value) 
  { return csPtr<iScriptValue> (new Value (this, PyLong_FromLong((long)value), false)); }
  virtual csPtr<iScriptValue> RValue (float value) 
  { return csPtr<iScriptValue> (new Value (this, PyFloat_FromDouble((float)value), false)); }
  virtual csPtr<iScriptValue> RValue (double value) 
  { return csPtr<iScriptValue> (new Value (this, PyFloat_FromDouble(value), false)); }
  virtual csPtr<iScriptValue> RValue (const char *value) 
  { return csPtr<iScriptValue> (new Value (this, PyString_FromString(value), false)); }
  virtual csPtr<iScriptValue> RValue (bool value) 
  { return csPtr<iScriptValue> (new Value (this, PyBool_FromLong(value ? 1 : 0), false)); }
  virtual csPtr<iScriptValue> RValue (iScriptObject *value) 
  { return csPtr<iScriptValue> (new Value (this, Query(value)->self, false)); }

  // following are not from iScript
  virtual csPtr<iScriptValue> RValue (PyObject *obj) 
  { return csPtr<iScriptValue> (new Value (this, obj, false)); }
  virtual csPtr<iScriptValue> RValue (void* data, const char* tag) 
  { return csPtr<iScriptValue> (new Value (this, csWrapTypedObject(data, (const char*)tag, 0), false)); }

  // error printing
  void ShowError();
  void Print(bool Error, const char *msg);

  // functions for printing python objects
  void Print(csRef<iScriptValue> value);
  void Print(csRef<iScriptObject> obj);
  void Print(PyObject *py_obj);

  // Implement iEventHandler interface.
  virtual bool HandleEvent(iEvent&);
  CS_EVENTHANDLER_NAMES("crystalspace.cspython")
  CS_EVENTHANDLER_NIL_CONSTRAINTS

  csRef<iEventHandler> weakeh_open;
};

}
CS_PLUGIN_NAMESPACE_END(cspython)

#endif // __CS_CSPYTHON_H__
