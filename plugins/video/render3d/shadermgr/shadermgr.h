/*
    Copyright (C) 2002 by M�rten Svanfeldt
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

  int seqnumber;

  // standard variables
  // these are inited and updated by the shadermanager itself
  csRef<csShaderVariable> sv_time;
  void UpdateStandardVariables();

  csRefArray<iShaderProgramPlugin> pluginlist;

  csShaderVariableContextHelper svContextHelper;
public:
  SCF_DECLARE_IBASE;

  csShaderManager(iBase* parent);
  virtual ~csShaderManager();

  void Open ();
  void Close ();

  //==================== iShaderManager ================//

  /// Create a empty shader
  virtual csPtr<iShader> CreateShader() ;
  /// Get a shader by name
  virtual iShader* GetShader(const char* name) ;
  /// Returns all shaders that have been created
  virtual const csRefArray<iShader> &GetShaders () { return shaders; }

  virtual csPtr<csShaderVariable> CreateVariable(csStringID name) const
  {
    return new csShaderVariable(name);
  }

  /// Report a message.
  void Report (int severity, const char* msg, ...);

  /// Create a shaderprogram
  virtual csPtr<iShaderProgram> CreateShaderProgram(const char* type);

  /// Prepare all created shaders
  virtual void PrepareShaders ();

  //=================== iShaderVariableContext ================//
  /// Add a variable to this context
  virtual void AddVariable (csShaderVariable *variable)
    { svContextHelper.AddVariable (variable); }

  /// Get a named variable from this context
  virtual csShaderVariable* GetVariable (csStringID name) const
    { return svContextHelper.GetVariable (name); }

  /// Get a named variable from this context, and any context above/outer
  virtual csShaderVariable* GetVariableRecursive (csStringID name) const
  {
    csShaderVariable* var;
    var=GetVariable (name);
    if(var) return var;
    return 0;
  }

  /// Fill a csShaderVariableList
  virtual void FillVariableList (csShaderVariableList *list) const
    { svContextHelper.FillVariableList (list); }

  //==================== iComponent ====================//
  bool Initialize(iObjectRegistry* objreg);

  struct Component : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE( csShaderManager );
    virtual bool Initialize( iObjectRegistry* objectreg )
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
      SCF_DESTRUCT_IBASE();
    }
    virtual bool HandleEvent (iEvent& ev) 
    { return parent->HandleEvent (ev); }
  } * scfiEventHandler;
};


class csShader : public iShader
{
private:
  csRef<iObjectRegistry> objectreg;
  
  csRefArray<iShaderTechnique> techniques;
  csShaderManager* parent;
  char* name;

  //loading related
  enum
  {
    XMLTOKEN_SHADER,
    XMLTOKEN_TECHNIQUE,
    XMLTOKEN_DECLARE
  };

  csStringHash xmltokens;
  void BuildTokenHash();

  csShaderVariableContextHelper svContextHelper;
public:
  SCF_DECLARE_IBASE;

  csShaderManager* GetParent() {return parent;}

  csShader(csShaderManager* owner, iObjectRegistry* reg);
  csShader(const char* name, csShaderManager* owner, iObjectRegistry* reg);
  virtual ~csShader();

  //====================== iShader =====================//
  
  /**
   * Check if valid (normaly a shader is valid if there is at least
   * one valid technique)
   */
  virtual bool IsValid() const;

  /// Set this shader's name
  virtual void SetName( const char* newname ) 
  {
    delete[] name;
    name = csStrNew(newname);
  }
  /// Retrieve name of shader
  virtual const char* GetName() { return name; }
  
  /// Create a new technique
  virtual csPtr<iShaderTechnique> CreateTechnique();
  /// Get number of techniques
  virtual int GetTechniqueCount() const { return techniques.Length(); }
  /// Retrieve a technique
  virtual iShaderTechnique* GetTechnique( int technique );
  /// Retrieve the best technique in this shader
  virtual iShaderTechnique* GetBestTechnique();

  /// Loads a shader from buffer
  virtual bool Load(iDataBuffer* program);

  /// Loads from a document-node
  virtual bool Load(iDocumentNode* node);

  /**
   * Prepares the shader for usage. Must be called before the shader is
   * assigned to a material.
   */
  virtual bool Prepare();

  //=================== iShaderVariableContext ================//
  /// Add a variable to this context
  virtual void AddVariable (csShaderVariable *variable)
  { svContextHelper.AddVariable (variable); }

  /// Get a named variable from this context
  virtual csShaderVariable* GetVariable (csStringID name) const
  { return svContextHelper.GetVariable (name); }

  /// Fill a csShaderVariableList
  virtual void FillVariableList (csShaderVariableList *list) const
  { svContextHelper.FillVariableList (list); }

  /// Get a named variable from this context, and any context above/outer
  virtual csShaderVariable* GetVariableRecursive (csStringID name) const
  {
    csShaderVariable* var;
    var=GetVariable (name);
    if(var) return var;
    return parent->GetVariableRecursive (name);
  }
};

class csShaderTechnique : public iShaderTechnique
{
private:
  int priority;
  csRefArray<iShaderPass> passes;
  csShader* parent;
  csRef<iObjectRegistry> objectreg;

  //loading related
  enum
  {
    XMLTOKEN_PASS,
    XMLTOKEN_DECLARE
  };

  csStringHash xmltokens;
  void BuildTokenHash();

  csShaderVariableContextHelper svContextHelper;
public:
  SCF_DECLARE_IBASE;

  csShader* GetParent() {return parent;}
  
  csShaderTechnique(csShader* owner , iObjectRegistry* reg);
  virtual ~csShaderTechnique();
  

  //================== iShaderTechnique ============//

  /* Get technique priority. If there are several valid techniques
   * use the one with highest priority
   */
  virtual int GetPriority() const {return priority;}

  /// Set technique priority.
  virtual void SetPriority(int priority) {this->priority = priority;}

  /// Create a pass
  virtual csPtr<iShaderPass> CreatePass();
  /// Get number of passes
  virtual int GetPassCount() const {return passes.Length(); }
  /// Retrieve a pass
  virtual iShaderPass* GetPass( int pass );

  /**
   * Check if valid (normally a shader is valid if there is at least one
   * valid technique)
   */
  virtual bool IsValid() const; 

  /// Loads a shader from buffer
  virtual bool Load(iDataBuffer* program);

  /// Loads from a document-node
  virtual bool Load(iDocumentNode* node) ;

  /**
   * Prepares the shader for usage. Must be called before the shader is
   * assigned to a material.
   */
  virtual bool Prepare();

  //=================== iShaderVariableContext ================//
  /// Add a variable to this context
  virtual void AddVariable (csShaderVariable *variable)
  { svContextHelper.AddVariable (variable); }

  /// Get a named variable from this context
  virtual csShaderVariable* GetVariable (csStringID name) const
  { return svContextHelper.GetVariable (name); }

  /// Fill a csShaderVariableList
  virtual void FillVariableList (csShaderVariableList *list) const
  { svContextHelper.FillVariableList (list); }

  /// Get a named variable from this context, and any context above/outer
  virtual csShaderVariable* GetVariableRecursive (csStringID name) const
  {
    csShaderVariable* var;
    var=GetVariable (name);
    if(var) return var;
    return parent->GetVariableRecursive (name);
  }
};

class csShaderPass : public iShaderPass
{
private:
  csRef<iShaderProgram> vp;
  csRef<iShaderProgram> fp;
  csRef<iObjectRegistry> objectreg;
  csShaderTechnique* parent;


  // Optimization fields for SetupState.
  static int buffercount;
  int texturecount;
  static csVertexAttrib attribs[STREAMMAX*2];
  static iRenderBuffer* buffers[STREAMMAX*2];
  static csStringID buffernames[STREAMMAX*2];
  static iRenderBuffer* clear_buffers[STREAMMAX*2];	// For quick clearing...
  static int units[TEXMAX];
  static iTextureHandle* textures[TEXMAX];
  static iTextureHandle* clear_textures[TEXMAX];	// For quick clearing...
  static iTextureHandle* autoAlphaTex;

  uint mixmode;
  csAlphaMode alphaMode;

  csStringID streammapping[STREAMMAX];
  bool streammappinggeneric[STREAMMAX];
  csStringID texmapping[TEXMAX];
  csRef<csShaderVariable> texmappingRef[TEXMAX];
  csRef<csShaderVariable> autoAlphaTexRef;


  //loading related
  enum
  {
    XMLTOKEN_DECLARE,
    XMLTOKEN_MIXMODE,
    XMLTOKEN_ALPHAMODE,
    XMLTOKEN_STREAMMAPPING,
    XMLTOKEN_TEXTUREMAPPING,
    XMLTOKEN_VP,
    XMLTOKEN_FP,
    XMLTOKEN_WRITEMASK
  };

  csStringHash xmltokens;
  void BuildTokenHash();

  // writemask
  bool writemaskRed, writemaskGreen, writemaskBlue, writemaskAlpha;

  // writemask before we changed it, restor to this in deactivate
  bool OrigWMRed, OrigWMGreen, OrigWMBlue, OrigWMAlpha;
  
  csRef<iGraphics3D> g3d;

  csShaderVariableContextHelper svContextHelper;
  csShaderVariableList dynamicVars;
public:
  SCF_DECLARE_IBASE;

  iShaderTechnique* GetParent() {return parent;}

  csShaderPass (csShaderTechnique* owner, iObjectRegistry* reg);
  virtual ~csShaderPass ()
  {
    SCF_DESTRUCT_IBASE();
  }

  /// Add a stream mapping
  virtual void AddStreamMapping (csStringID name, csVertexAttrib attribute);
  /// Get stream mapping for a certain attribute
  virtual csStringID GetStreamMapping (csVertexAttrib attribute) const;

  /// Add a texture mapping
  virtual void AddTextureMapping (csStringID name, int unit);
  /// Get texture mapping for a certain unit
  virtual csStringID GetTextureMapping (int unit) const;

  /// Get mixmode override
  virtual uint GetMixmodeOverride () const
  { return mixmode; }

  /// Get vertex-program
  virtual iShaderProgram* GetVertexProgram() {return vp; }

  /// Set vertex-program
  virtual void SetVertexProgram(iShaderProgram* program) 
  { vp = program; }

  /// Get fragment-program
  virtual iShaderProgram* GetFragmentProgram() { return fp; }

  /// Set fragment-program
  virtual void SetFragmentProgram(iShaderProgram* program) 
  { fp = program; }

  /// Check if valid
  virtual bool IsValid() const
  {
    bool valid = false;
    if(vp) valid |= vp->IsValid();
    if(fp) valid &= fp->IsValid();
    return valid;
  }

  /// Activate
  virtual void Activate(csRenderMesh* mesh);

  /// Deactivate
  virtual void Deactivate();

  /// Setup states needed for proper operation of the shader
  virtual void SetupState (csRenderMesh* mesh, 
    csArray<iShaderVariableContext*> &dynamicDomains);

  /// Reset states to original
  virtual void ResetState ();

  /// Loads a shader from buffer
  virtual bool Load(iDataBuffer* program);

  /// Loads from a document-node
  virtual bool Load(iDocumentNode* node);

  /**
   * Prepares the shader for usage. Must be called before the shader is
   * assigned to a material.
   */
  virtual bool Prepare();

  virtual const csAlphaMode& GetAlphaMode ()
  { return alphaMode;  }

  //=================== iShaderVariableContext ================//
  /// Add a variable to this context
  virtual void AddVariable (csShaderVariable *variable)
  { svContextHelper.AddVariable (variable); }

  /// Get a named variable from this context
  virtual csShaderVariable* GetVariable (csStringID name) const
  { return svContextHelper.GetVariable (name); }

  /// Fill a csShaderVariableList
  virtual void FillVariableList (csShaderVariableList *list) const
  { svContextHelper.FillVariableList (list); }

  /// Get a named variable from this context, and any context above/outer
  virtual csShaderVariable* GetVariableRecursive (csStringID name) const
  {
    csShaderVariable* var;
    var=GetVariable (name);
    if(var) return var;
    return parent->GetVariableRecursive (name);
  }
};

#endif //__SHADERMGR_H__

