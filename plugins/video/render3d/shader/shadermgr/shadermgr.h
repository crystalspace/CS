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

#include "csutil/cfgacc.h"
#include "csutil/csstring.h"
#include "csutil/objreg.h"
#include "csutil/parray.h"
#include "csutil/ref.h"
#include "csutil/refarr.h"
#include "csutil/scf.h"
#include "csutil/scfarray.h"
#include "csutil/util.h"
#include "csutil/weakref.h"

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

CS_PLUGIN_NAMESPACE_BEGIN(ShaderManager)
{

class PlexHierarchicalCache;

typedef csHash<csRef<iShaderVariableAccessor>,csStringBase> csSVAHash;

class csShaderManager : 
  public scfImplementation3<csShaderManager,
			    iShaderManager,
			    iEventHandler,
			    iComponent>,
  public CS::ShaderVariableContextImpl
{
private:
  iObjectRegistry* objectreg;
  csRef<iVirtualClock> vc;
  csRef<iTextureManager> txtmgr;
  csRef<iStringSet> strings;
  csRef<iShaderVarStringSet> stringsSvName;
  csRef<iEventHandler> weakEventHandler;
  csRef<PlexHierarchicalCache> shaderCache;

  bool do_verbose;

  csRefArray<iShader> shaders;
  csRefArray<iShaderCompiler> compilers;

  csConfigAccess config;

  int seqnumber;

  // standard variables
  // these are inited and updated by the shadermanager itself
  csRef<csShaderVariable> sv_time;
  void UpdateStandardVariables();

  csShaderVariableStack shaderVarStack;

  csSet<csStringID> neutralTags;
  csSet<csStringID> forbiddenTags;
  csSet<csStringID> requiredTags;
  struct TagInfo
  {
    csShaderTagPresence presence;
    int priority;
  };
  csSet<csStringID>& GetTagSet (csShaderTagPresence presence);
  csHash<TagInfo, csStringID> tagInfo;

  // We maintain a hash of shader variable accessors.
  csSVAHash sva_hash;

  csEventID Frame;
  csEventID SystemOpen;
  csEventID SystemClose;

#define CS_TOKEN_ITEM_FILE \
  "plugins/video/render3d/shader/shadermgr/shadermgr.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE
  csStringHash xmltokens;
    
  void AddDefaultVariables();
  void LoadDefaultVariables();
public:
  csShaderManager(iBase* parent);
  virtual ~csShaderManager();

  void Open ();
  void Close ();

  /**\name iShaderManager implementation
   * @{ */

  /**
   * Register a shader to the shadermanager. Compiler should register all
   * shaders.
   */
  virtual void RegisterShader (iShader* shader);
  virtual void UnregisterShader (iShader* shader);
  virtual void UnregisterShaders ();
  /// Get a shader by name
  virtual iShader* GetShader (const char* name);
  /// Returns all shaders that have been created
  virtual const csRefArray<iShader> &GetShaders () { return shaders; }


  /// Register a compiler to the manager
  virtual void RegisterCompiler (iShaderCompiler* compiler);
  /// Get a shadercompiler by name
  virtual iShaderCompiler* GetCompiler (const char* name);

  virtual void RegisterShaderVariableAccessor (const char* name,
      iShaderVariableAccessor* accessor);
  virtual void UnregisterShaderVariableAccessor (const char* name,
      iShaderVariableAccessor* accessor);
  virtual iShaderVariableAccessor* GetShaderVariableAccessor (
      const char* name);
  virtual void UnregisterShaderVariableAcessors ();

  /// Report a message.
  void Report (int severity, const char* msg, ...);

  /// Get the shadervariablestack used to handle shadervariables on rendering
  virtual csShaderVariableStack& GetShaderVariableStack ()
  {
    return shaderVarStack;
  }

  virtual void SetTagOptions (csStringID tag, csShaderTagPresence presence, 
    int priority = 0);
  virtual void GetTagOptions (csStringID tag, csShaderTagPresence& presence, 
    int& priority);

  virtual const csSet<csStringID>& GetTags (csShaderTagPresence presence,
    int& count);

  virtual iShaderVarStringSet* GetSVNameStringset () const
  {
    return stringsSvName;
  }
  
  iHierarchicalCache* GetShaderCache();
  
  void AddSubShaderCache (iHierarchicalCache* cache,
    int priority = cachePriorityApp);
  iHierarchicalCache* AddSubCacheDirectory (const char* cacheDir,
    int priority = cachePriorityApp, bool readOnly = false);
  void RemoveSubShaderCache (iHierarchicalCache* cache);
  void RemoveAllSubShaderCaches ();
  /** @} */

  /**\name iComponent implementation
   * @{ */
  bool Initialize (iObjectRegistry* objreg);
  /** @} */


  /**\name iEventHandler implementation
   * @{ */
  bool HandleEvent (iEvent& Event);

  CS_EVENTHANDLER_NAMES("crystalspace.graphics3d.shadermgr")
  
  virtual const csHandlerID * GenericPrec (
    csRef<iEventHandlerRegistry> &, csRef<iEventNameRegistry> &,
    csEventID) const { return 0; }

  virtual const csHandlerID * GenericSucc (
    csRef<iEventHandlerRegistry> &r1, csRef<iEventNameRegistry> &r2,
    csEventID event) const 
  { 
    /// \todo Create signposts for the SystemOpen event
    if (event != csevFrame(r2))
      return 0;
    static csHandlerID succConstraint[6] = {
      FrameSignpost_Logic3D::StaticID(r1),
      FrameSignpost_3D2D::StaticID(r1),
      FrameSignpost_2DConsole::StaticID(r1),
      FrameSignpost_ConsoleDebug::StaticID(r1),
      FrameSignpost_DebugFrame::StaticID(r1),
      CS_HANDLERLIST_END
    };
    return succConstraint; 
  }
  
  CS_EVENTHANDLER_DEFAULT_INSTANCE_CONSTRAINTS
  /** @} */
};

}
CS_PLUGIN_NAMESPACE_END(ShaderManager)

#endif //__SHADERMGR_H__
