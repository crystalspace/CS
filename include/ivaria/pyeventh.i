/*
  Copyright (C) 2003 Rene Jager <renej_frog@users.sourceforge.net>

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

#ifdef SWIGPYTHON

#ifndef CS_MICRO_SWIG

/*
struct _csPyEventHandler : public iEventHandler
{
  SCF_DECLARE_IBASE;
  _csPyEventHandler (PyObject * obj);
  virtual ~_csPyEventHandler ();
  virtual bool HandleEvent (iEvent &);
};
*/

%inline %{

  struct _csPyEventHandler : public iEventHandler
  {
    SCF_DECLARE_IBASE;
    _csPyEventHandler (PyObject * obj) : _pySelf(obj)
    {
      SCF_CONSTRUCT_IBASE(0);
      IncRef();
    }
    virtual ~_csPyEventHandler ()
    {
      SCF_DESTRUCT_IBASE();
      DecRef();
    }
    virtual bool HandleEvent (iEvent & event)
    {
      PyObject * event_obj = SWIG_NewPointerObj(
        (void *) &event, SWIG_TypeQuery("iEvent *"), 0
      );
      PyObject * result = PyObject_CallMethod(_pySelf, "HandleEvent", "(O)",
        event_obj
      );
      Py_DECREF(event_obj);
      if (!result)
      {
        return false;
      }
      bool res = PyInt_AsLong(result);
      Py_DECREF(result);
      return res;
    }
  private:
    PyObject * _pySelf;
  };

%}

%{
  SCF_IMPLEMENT_IBASE(_csPyEventHandler)
  SCF_IMPLEMENT_IBASE_END
%}

%pythoncode %{

  class csPyEventHandler (_csPyEventHandler):
    """Python version of iEventHandler implementation.
       This class can be used as base class for event handlers in Python.
       Call csPyEventHandler.__init__(self) in __init__ of derived class.
    """
    def __init__ (self):
      _csPyEventHandler.__init__(self, self)

  class _EventHandlerFuncWrapper (csPyEventHandler):
    def __init__ (self, func):
      csPyEventHandler.__init__(self)
      self._func = func
      # Make sure a reference keeps to this wrapper instance.
      self._func._cs_event_handler_wrapper = self
    def HandleEvent (self, event):
      return self._func(event)

  def _csInitializer_SetupEventHandler (reg, obj,
      mask=(CSMASK_FrameProcess|CSMASK_Input|CSMASK_Broadcast)):
    """Replacement of C++ versions."""
    if callable(obj):
      # obj is a function
      hdlr = _EventHandlerFuncWrapper(obj)
      hdlr.thisown = 1
    else:
      # assume it is a iEventHandler
      hdlr = obj
    return csInitializer._SetupEventHandler(reg, hdlr, mask)

  csInitializer.SetupEventHandler = \
    staticmethod(_csInitializer_SetupEventHandler)

%}

#ifdef USE_DIRECTORS

%feature("director") iEventHandler;

#endif // USE_DIRECTORS

#endif // CS_MICRO_SWIG

#endif // SWIGPYTHON
