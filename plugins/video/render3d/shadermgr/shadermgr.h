/*
    Copyright (C) 2002 by Mårten Svanfeldt
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
#include "csutil/scf.h"
#include "csutil/objreg.h"
#include "csutil/csstring.h"

#include "iutil/event.h"
#include "iutil/eventh.h"

#include "iutil/string.h"
#include "iutil/comp.h"

#include "ivideo/shader/shader.h"

class csShaderManager : public iShaderManager
{
private:
  csRef<iObjectRegistry> objectreg;
  csRef<iVirtualClock> vc;

  csHashMap* variables;
  csBasicVector* shaders;

  int seqnumber;

  // standard variables
  // these are inited and updated by the shadermanager itself
  csRef<iShaderVariable> sv_time;
  void UpdateStandardVariables();
public:
  SCF_DECLARE_IBASE;

  csShaderManager(iBase* parent);
  virtual ~csShaderManager();

  //==================== iShaderManager ================//

  /// Creates a shader from specified file
  virtual csPtr<iShader> CreateShader(const char* filename) ;
  /// Create a empty shader
  virtual csPtr<iShader> CreateShader() ;
  /// Get a shader by name
  virtual iShader* GetShader(const char* name) ;

  /// Create variable
  virtual csPtr<iShaderVariable> CreateVariable(const char* name) ;
  /// Add a variable to this context
  virtual bool AddVariable(iShaderVariable* variable) ;
  /// Get variable
  virtual iShaderVariable* GetVariable(const char* string);
  /// Get all variable stringnames added to this context (used when creatingthem)
  virtual csBasicVector GetAllVariableNames() ; 

  /// Create a shaderpgoram from a shaderfile
  virtual csPtr<iShaderProgram> CreateShaderProgramFromFile(const char* programfile, const char* type);
  /// Create a shaderprogram from a string describing it
  virtual csPtr<iShaderProgram> CreateShaderProgramFromString(const char* programstring, const char* type);

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
    EventHandler (csShaderManager* parent)
    {
      SCF_CONSTRUCT_IBASE (NULL);
      EventHandler::parent = parent;
    }
    
    SCF_DECLARE_IBASE;
    virtual bool HandleEvent (iEvent& ev) 
      { return parent->HandleEvent (ev); }
  } * scfiEventHandler;
};

/*class csShaderVariable : public iShaderVariable
{
private:
  csString name;

  int type;

  int intval; //0
  csRef<iString> istring; //1
  csVector3 v3; //2

public:
  SCF_DECLARE_IBASE;

  csShaderVariable()
  {
    type = 0;
  }
  virtual void SetName(const char* name) { csShaderVariable::name = csString(name); }
  virtual const char* GetName() { return name; }
  virtual bool GetValue(int& value) { if(type==0) {value = intval; return true;} return false; }
  virtual bool GetValue(iString* value) { if(type==1) {value = istring; return true;} return false; }
  virtual bool GetValue(csVector3& value) { if(type==2) {value = v3; return true;} return false; }
//  virtual bool GetValue(csVector4* value);
  virtual bool SetValue(int value) { type=0; intval = value; return true; }
  virtual bool SetValue(iString* value) { type=1; istring = value; return true; }
  virtual bool SetValue(csVector3 value) { type=2; v3 = value; return true; }
//  virtual bool SetValue(csVector4* value);
};
*/
class csShader : public iShader
{
private:
  csHashMap* variables;
  csBasicVector* techniques;
  csShaderManager* parent;
  char* name;
public:
  SCF_DECLARE_IBASE;

  csShaderManager* GetParent() {return parent;}

  csShader(csShaderManager* owner);
  csShader(const char* name, csShaderManager* owner);
  virtual ~csShader();

  //====================== iShader =====================//
  
  /// Check if valid (normaly a shader is valid if there is at least one valid technique)
  virtual bool IsValid();

  /// Sets a stream-mapping
  virtual void MapStream( int mapped_id, const char* streamname);

  /// Set this shader's name
  virtual void SetName( const char* name ) 
  {
    if(this->name) delete this->name;
    this->name = new char[strlen(name)+1];
    strcpy(this->name, name);
  }
  /// Retrieve name of shader
  virtual const char* GetName() { return name; }
  
  /// Create a new technique
  virtual csPtr<iShaderTechnique> CreateTechnique();
  /// Get number of techniques
  virtual int GetTechniqueCount() { return techniques->Length(); }
  /// Retrieve a technique
  virtual iShaderTechnique* GetTechnique( int technique );
  /// Retrieve the best technique in this shader
  virtual iShaderTechnique* GetBestTechnique();

  /// Add a variable to this context
  virtual bool AddVariable(iShaderVariable* variable);
  /// Get variable
  virtual iShaderVariable* GetVariable(const char* string);
  /// Get all variable stringnames in this context (used when creatingthem)
  virtual csBasicVector GetAllVariableNames(); 
};

class csShaderTechnique : public iShaderTechnique
{
private:
  int priority;
  csBasicVector* passes;
  csShader* parent;
public:
  SCF_DECLARE_IBASE;

  csShader* GetParent() {return parent;}
  
  csShaderTechnique(csShader* owner);
  virtual ~csShaderTechnique();
  

  //================== iShaderTechnique ============//
  /// Sets a stream-mapping
  virtual void MapStream( int mapped_id, const char* streamname);

  /* Get technique priority. If there are several valid techniques
   * use the one with highest priority
   */
  virtual int GetPriority() {return priority;}

  /// Set technique priority.
  virtual void SetPriority(int priority) {this->priority = priority;}

  /// Create a pass
  virtual csPtr<iShaderPass> CreatePass();
  /// Get number of passes
  virtual int GetPassCount() {return passes->Length(); }
  /// Retrieve a pass
  virtual iShaderPass* GetPass( int pass );

  /// Check if valid (normaly a shader is valid if there is at least one valid technique)
  virtual bool IsValid(); 
};

class csShaderPass : public iShaderPass
{
private:
  csRef<iShaderProgram> vp;
  csRef<iShaderProgram> fp;

  csShaderTechnique* parent;
  csHashMap variables;
public:
  SCF_DECLARE_IBASE;

  iShaderTechnique* GetParent() {return parent;}

  csShaderPass(csShaderTechnique* owner)
  {
    SCF_CONSTRUCT_IBASE( NULL );
    vp = 0; fp = 0;
    parent = owner;
  }

  /// Get vertex-program
  virtual iShaderProgram* GetVertexProgram() {return vp; }

  /// Set vertex-program
  virtual void SetVertexProgram(iShaderProgram* program) { vp = program; }

  /// Get fragment-program
  virtual iShaderProgram* GetFragmentProgram() { return fp; }

  /// Set fragment-program
  virtual void SetFragmentProgram(iShaderProgram* program) { fp = program; }

  /// Check if valid
  virtual bool IsValid()
  {
    if( vp || fp )
      return true;
    return false;
  }

  /// Activate
  virtual void Activate()
  {
    if(vp) vp->Activate(this);
    if(fp) fp->Activate(this);
  }

  /// Deactivate
  virtual void Deactivate()
  {
    if(vp) vp->Deactivate(this);
    if(fp) fp->Deactivate(this);
  }

  /// Add a variable to this context
  virtual bool AddVariable(iShaderVariable* variable){return false;}
  /// Get variable
  virtual iShaderVariable* GetVariable(const char* string);
  /// Get all variable stringnames in this context (used when creatingthem)
  virtual csBasicVector GetAllVariableNames() {return csBasicVector();}
};

#endif //__SHADERMGR_H__