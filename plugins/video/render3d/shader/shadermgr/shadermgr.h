/*
    Copyright (C) 2002 by Marten Svanfeldt
                          Anders Stenberg

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

#ifndef __SHADERMGR_H__
#define __SHADERMGR_H__

#include "csutil/ref.h"
#include "csutil/refarr.h"
#include "csutil/scf.h"
#include "csutil/objreg.h"
#include "csutil/csstring.h"
#include "csutil/util.h"
#include "csutil/parray.h"

#include "iutil/event.h"
#include "iutil/eventh.h"

#include "iutil/string.h"
#include "iutil/comp.h"

#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"
#include "ivideo/material.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"
#include "csgfx/shadervar.h"
#include "csgfx/shadervarcontext.h"

// (These are undeffed at end of shadermgr.cpp. Ugly?)
#define STREAMMAX 16  // @@@ Hardcoded max streams to 16 
#define TEXMAX 16  // @@@ Hardcoded max texture units to 16


class csShaderManager : public iShaderManager
{
private:
  csRef<iObjectRegistry> objectreg;
  csRef<iVirtualClock> vc;
  csRef<iTextureManager> txtmgr;

  csRefArray<iShader> shaders;
  csRefArray<iShaderCompiler> compilers;

  int seqnumber;

  // standard variables
  // these are inited and updated by the shadermanager itself
  csRef<csShaderVariable> sv_time;
  void UpdateStandardVariables();

  csShaderVariableContext svcontext;
  CS_SHADERVAR_STACK shaderVarStack;

public:
  SCF_DECLARE_IBASE;

  csShaderManager(iBase* parent);
  virtual ~csShaderManager();

  void Open ();
  void Close ();

  //==================== iShaderManager ================//

  /**
   * Register a shader to the shadermanager. Compiler should register all
   * shaders.
   */
  virtual void RegisterShader (iShader* shader);
  /// Get a shader by name
  virtual iShader* GetShader (const char* name);
  /// Returns all shaders that have been created
  virtual const csRefArray<iShader> &GetShaders () { return shaders; }


  /// Register a compiler to the manager
  virtual void RegisterCompiler (iShaderCompiler* compiler);
  /// Get a shadercompiler by name
  virtual iShaderCompiler* GetCompiler (const char* name);

  /// Report a message.
  void Report (int severity, const char* msg, ...);

  /// Get the shadervariablestack used to handle shadervariables on rendering
  virtual CS_SHADERVAR_STACK& GetShaderVariableStack ()
  {
    return shaderVarStack;
  }

  //=================== iShaderVariableContext ================//

  /// Add a variable to this context
  void AddVariable (csShaderVariable *variable)
    { svcontext.AddVariable (variable); }

  /// Get a named variable from this context
  csShaderVariable* GetVariable (csStringID name) const
    { return svcontext.GetVariable (name); }

  /**
  * Push the variables of this context onto the variable stacks
  * supplied in the "stacks" argument
  */
  void PushVariables (CS_SHADERVAR_STACK &stacks) const
    { svcontext.PushVariables (stacks); }

  /**
  * Pop the variables of this context off the variable stacks
  * supplied in the "stacks" argument
  */
  void PopVariables (CS_SHADERVAR_STACK &stacks) const
    { svcontext.PopVariables (stacks); }

  //==================== iComponent ====================//
  bool Initialize (iObjectRegistry* objreg);

  struct Component : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csShaderManager);
    virtual bool Initialize (iObjectRegistry* objectreg)
    {
      return scfParent->Initialize( objectreg );
    }
  } scfiComponent;

  ////////////////////////////////////////////////////////////////////
  //                         iEventHandler
  ////////////////////////////////////////////////////////////////////
  
  bool HandleEvent (iEvent& Event);

  struct EventHandler : public iEventHandler
  {
  private:
    csShaderManager* parent;
  public:
    SCF_DECLARE_IBASE;

    EventHandler (csShaderManager* parent)
    {
      SCF_CONSTRUCT_IBASE (0);
      EventHandler::parent = parent;
    }
    virtual ~EventHandler ()
    {
      SCF_DESTRUCT_IBASE ();
    }
    
    virtual bool HandleEvent (iEvent& ev) 
    { return parent->HandleEvent (ev); }
  } * scfiEventHandler;
};

#endif //__SHADERMGR_H__
