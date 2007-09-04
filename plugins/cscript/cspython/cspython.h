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

#ifndef __CS_CSPYTHON_H__
#define __CS_CSPYTHON_H__

#include "ivaria/script.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/csinput.h"
#include "csplugincommon/script/scriptcommon.h"

CS_PLUGIN_NAMESPACE_BEGIN(cspython)
{

class csPython : public scfImplementationExt1<csPython,
					      csScriptCommon,
					      iComponent> 
{
public:
  csPython(iBase *iParent);
  virtual ~csPython();

  static csPython* shared_instance;
  iObjectRegistry* object_reg;
  int Mode;
  bool use_debugger;

  virtual bool Initialize(iObjectRegistry*); // implements iComponent

  virtual bool RunText(const char *Text);
  virtual bool LoadModule(const char *Text);
  virtual bool LoadModule(const char *path, const char *name);
  virtual bool LoadModuleNative(const char *path, const char *name);

  //@@@ The following 11 methods are new additions to iScript and not yet
  //@@@ implemented.
  virtual bool Store (const char *name, iScriptValue *value) { return false; }
  virtual csPtr<iScriptValue> Retrieve (const char *name) { return 0; }
  virtual bool Remove (const char *name) { return false; }
  virtual csPtr<iScriptValue> Call (const char *name,
	const csRefArray<iScriptValue> &args) { return 0; }
  virtual csPtr<iScriptObject> New (const char *type,
	const csRefArray<iScriptValue> &args) { return 0; }
  virtual csPtr<iScriptValue> RValue (int value) { return 0; }
  virtual csPtr<iScriptValue> RValue (float value) { return 0; }
  virtual csPtr<iScriptValue> RValue (double value) { return 0; }
  virtual csPtr<iScriptValue> RValue (const char *value) { return 0; }
  virtual csPtr<iScriptValue> RValue (bool value) { return 0; }
  virtual csPtr<iScriptValue> RValue (iScriptObject *value) { return 0; }

  bool HandleEvent(iEvent&);
  bool Store(const char* name, void* data, void* tag);
  void ShowError();
  void Print(bool Error, const char *msg);

  // Implement iEventHandler interface.
  struct EventHandler : public scfImplementation1<EventHandler, iEventHandler>
  {
    csPython* parent;

    EventHandler (csPython* parent) : scfImplementationType (this), 
      parent (parent) {}
    virtual ~EventHandler() {}
    virtual bool HandleEvent (iEvent& e) { return parent->HandleEvent(e); }

    CS_EVENTHANDLER_NAMES("crystalspace.cspython")
    CS_EVENTHANDLER_NIL_CONSTRAINTS
  };
  csRef<EventHandler> eventHandler;
};

}
CS_PLUGIN_NAMESPACE_END(cspython)

#endif // __CS_CSPYTHON_H__
