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

#ifdef SWIGCSHARP

#ifndef CS_MINI_SWIG

%ignore _csSharpEventHandler::_HandleEvent;
%ignore _csSharpEventHandler::_Connect;
%ignore _csSharpEventHandler::m_handler;
%ignore EventHandler_t;

%template(scfSharpEventHandler) scfImplementation1<_csSharpEventHandler, iEventHandler>;
%inline %{
  typedef int (*EventHandler_t)(void *ev);

  struct _csSharpEventHandler: public scfImplementation1<_csSharpEventHandler,
							iEventHandler>
  {
    _csSharpEventHandler(): scfImplementationType(this)
    {
    }

    virtual ~_csSharpEventHandler()
    {
    }
		
    virtual bool HandleEvent (iEvent &ev)
    {
      return _HandleEvent(ev);
    }

    bool _HandleEvent(iEvent &ev)
    {
      int ret;
      if(m_handler)
      {
      ret = m_handler((void*)&ev);
      if(ret)
	return true;
      }
      return false;
    }
		
    void _Connect(EventHandler_t handler)
    {
      m_handler = handler;
    }

    CS_EVENTHANDLER_NAMES("crystalspace.cspacesharp")
    CS_EVENTHANDLER_NIL_CONSTRAINTS

    EventHandler_t m_handler;
  };
%}

%pragma(csharp) imclasscode=%{
  //We declare this delegate in cspacePINVOKE for callbacks
  public delegate int EventHandler_t(IntPtr eventHandler);

  [DllImport("$dllimport", EntryPoint="ConnectSharpEventHandler")]
  public static extern void ConnectSharpEventHandler(IntPtr self, EventHandler_t handler);

  [DllImport("$dllimport", EntryPoint="csMalloc")]
  public static extern IntPtr csMalloc(int size);

  [DllImport("$dllimport", EntryPoint="csFree")]
  public static extern void csFree(IntPtr ptr);
%}

#ifdef USE_DIRECTORS

%feature("director") iEventHandler;

#endif //USE_DIRECTORS

%{
  extern "C"{
    void ConnectSharpEventHandler(void *self, EventHandler_t handler);
    void *csMalloc(int size);
    void csFree(void *ptr);
  }

  void ConnectSharpEventHandler(void *self, EventHandler_t handler)
  {
    if(self!=NULL)
      ((_csSharpEventHandler*)self)->_Connect(handler);
  }

  void *csMalloc(int size)
  {
    return malloc(size);
  }

  void csFree(void *ptr)
  {
    free(ptr);
  }
%}

#endif //CS_MINI_SWIG
#endif SWIGCSHARP

