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
#include "csgfx/symtable.h"

#include "iutil/event.h"
#include "iutil/eventh.h"

#include "iutil/string.h"
#include "iutil/comp.h"

#include "ivideo/render3d.h"
#include "ivideo/shader/shader.h"

// (These are undeffed at end of shadermgr.cpp. Ugly?)
#define STREAMMAX 16  // @@@ Hardcoded max streams to 16 
#define TEXMAX 16  // @@@ Hardcoded max texture units to 16

class csShaderWrapper : public iShaderWrapper
{
  csRef<iShader> shader;
  csSymbolTable symtab;
public:
  SCF_DECLARE_IBASE;

  csShaderWrapper (iShader* shader);
  virtual ~csShaderWrapper ();

  virtual iShader* GetShader();
  virtual void SelectMaterial(iMaterial* mat);
  virtual csSymbolTable* GetSymbolTable();
};

class csShaderManager : public iShaderManager
{
private:
  csRef<iObjectRegistry> objectreg;
  csRef<iVirtualClock> vc;

  csHashMap* variables;
  csRefArray<iShaderWrapper> shaders;

  int seqnumber;

  // standard variables
  // these are inited and updated by the shadermanager itself
  csRef<iShaderVariable> sv_time;
  void UpdateStandardVariables();

  csRefArray<iShaderProgramPlugin> pluginlist;
public:
  SCF_DECLARE_IBASE;

  csShaderManager(iBase* parent);
  virtual ~csShaderManager();

  //==================== iShaderManager ================//

  /// Create a empty shader
  virtual csPtr<iShader> CreateShader() ;
  /// Get a shader by name
  virtual iShader* GetShader(const char* name) ;
  /// Returns all shaders that have been created
  virtual const csRefArray<iShaderWrapper> &GetShaders () { return shaders; }
  /// Create a wrapper for a shader
  virtual csPtr<iShaderWrapper> CreateWrapper(iShader* shader);

  /// Create variable
  virtual csPtr<iShaderVariable> CreateVariable(const char* name) const;
  /// Add a variable to this context
  virtual bool AddVariable(iShaderVariable* variable) ;
  /// Get variable
  virtual iShaderVariable* GetVariable(int namehash)
  { return privateGetVariable (namehash); }
  /// Get all variable stringnames in this context (used when creatingthem)
  virtual csBasicVector GetAllVariableNames() const; 

  virtual csSymbolTable* GetSymbolTable();

  /// Private variable to get the variable without virtual call
  inline iShaderVariable* privateGetVariable (int namehash);

/// Create a shaderprogram
  virtual csPtr<iShaderProgram> CreateShaderProgram(const char* type);

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
      SCF_CONSTRUCT_IBASE (0);
      EventHandler::parent = parent;
    }
    
    SCF_DECLARE_IBASE;
    virtual bool HandleEvent (iEvent& ev) 
      { return parent->HandleEvent (ev); }
  } * scfiEventHandler;
};


class csShader : public iShader
{
private:
  csRef<iObjectRegistry> objectreg;
  csHashMap* variables;
  csBasicVector* techniques;
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

public:
  SCF_DECLARE_IBASE;

  csShaderManager* GetParent() {return parent;}

  csShader(csShaderManager* owner, iObjectRegistry* reg);
  csShader(const char* name, csShaderManager* owner, iObjectRegistry* reg);
  virtual ~csShader();

  //====================== iShader =====================//
  
  /// Check if valid (normaly a shader is valid if there is at least one valid technique)
  virtual bool IsValid() const;

  /// Set this shader's name
  virtual void SetName( const char* name ) 
  {
    delete csShader::name;
    csShader::name = new char[strlen(name)+1];
    strcpy(csShader::name, name);
  }
  /// Retrieve name of shader
  virtual const char* GetName() { return name; }
  
  /// Create a new technique
  virtual csPtr<iShaderTechnique> CreateTechnique();
  /// Get number of techniques
  virtual int GetTechniqueCount() const { return techniques->Length(); }
  /// Retrieve a technique
  virtual iShaderTechnique* GetTechnique( int technique );
  /// Retrieve the best technique in this shader
  virtual iShaderTechnique* GetBestTechnique();

  /// Add a variable to this context
  virtual bool AddVariable(iShaderVariable* variable);
  /// Get variable
  virtual iShaderVariable* GetVariable(int namehash)
  { return privateGetVariable (namehash); }
  /// Get all variable stringnames in this context (used when creatingthem)
  virtual csBasicVector GetAllVariableNames() const; 

  virtual csSymbolTable* GetSymbolTable();

  /// Private variable to get the variable without virtual call
  inline iShaderVariable* privateGetVariable (int namehash);


  /// Loads a shader from buffer
  virtual bool Load(iDataBuffer* program);

  /// Loads from a document-node
  virtual bool Load(iDocumentNode* node);

  /// Prepares the shader for usage. Must be called before the shader is assigned to a material
  virtual bool Prepare();
};

class csShaderTechnique : public iShaderTechnique
{
private:
  int priority;
  csBasicVector* passes;
  csShader* parent;
  csRef<iObjectRegistry> objectreg;

  //loading related
  enum
  {
    XMLTOKEN_PASS
  };

  csStringHash xmltokens;
  void BuildTokenHash();

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
  virtual int GetPassCount() const {return passes->Length(); }
  /// Retrieve a pass
  virtual iShaderPass* GetPass( int pass );

  /// Add a variable to this context
  virtual bool AddVariable(iShaderVariable* variable);
  /// Get variable
  virtual iShaderVariable* GetVariable(int namehash);
  /// Get all variable stringnames in this context (used when creatingthem)
  virtual csBasicVector GetAllVariableNames() const; 

  virtual csSymbolTable* GetSymbolTable();

  /// Check if valid (normaly a shader is valid if there is at least one valid technique)
  virtual bool IsValid() const; 

  /// Loads a shader from buffer
  virtual bool Load(iDataBuffer* program);

  /// Loads from a document-node
  virtual bool Load(iDocumentNode* node) ;

  /// Prepares the shader for usage. Must be called before the shader is assigned to a material
  virtual bool Prepare();
};

class csShaderPass : public iShaderPass
{
private:
  csRef<iShaderProgram> vp;
  csRef<iShaderProgram> fp;
  csRef<iObjectRegistry> objectreg;
  csShaderTechnique* parent;
  csHashMap variables;

  uint mixmode;

  csStringID streammapping[STREAMMAX];
  int texmappinglayer[TEXMAX];
  iTextureHandle* texmappingdirect[TEXMAX];

    //loading related
  enum
  {
    XMLTOKEN_DECLARE,
    XMLTOKEN_MIXMODE,
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
  
  csRef<iRender3D> r3d;
public:
  SCF_DECLARE_IBASE;

  iShaderTechnique* GetParent() {return parent;}

  csShaderPass(csShaderTechnique* owner, iObjectRegistry* reg)
  {
    SCF_CONSTRUCT_IBASE( 0 );
    r3d = CS_QUERY_REGISTRY (reg, iRender3D);
    vp = 0; fp = 0;
    parent = owner;
    objectreg = reg;
    mixmode = 0;
    int i;
    for (i=0; i<STREAMMAX; i++)
      streammapping[i] = csInvalidStringID;
    for (i=0; i<TEXMAX; i++)
    {
      texmappinglayer[i] = -1;
      texmappingdirect[i] = 0;
    }

    writemaskRed = true;
    writemaskGreen = true;
    writemaskBlue = true;
    writemaskAlpha = true;
  }
  virtual ~csShaderPass () {}

  /// Add a stream mapping
  virtual void AddStreamMapping (csStringID name, csVertexAttrib attribute);
  /// Get stream mapping for a certain attribute
  virtual csStringID GetStreamMapping (csVertexAttrib attribute) const;

  /// Add a texture mapping by name
  virtual void AddTextureMapping (const char* name, int unit);
  /// Add a texture mapping by material layer
  virtual void AddTextureMapping (int layer, int unit);
  /// Get texture mapping for a certain unit as a layer index
  virtual int GetTextureMappingAsLayer (int unit) const;
  /// Get texture mapping for a certain unit as a texture name
  virtual iTextureHandle* GetTextureMappingAsDirect (int unit);

  /// Get mixmode override
  virtual uint GetMixmodeOverride () const
  { return mixmode; }

  /// Get vertex-program
  virtual iShaderProgram* GetVertexProgram() {return vp; }

  /// Set vertex-program
  virtual void SetVertexProgram(iShaderProgram* program) { vp = program; }

  /// Get fragment-program
  virtual iShaderProgram* GetFragmentProgram() { return fp; }

  /// Set fragment-program
  virtual void SetFragmentProgram(iShaderProgram* program) { fp = program; }

  /// Check if valid
  virtual bool IsValid() const
  {
    bool valid = true;
    if(vp) valid = vp->IsValid();
    if(fp) valid = fp->IsValid();
    return valid;
  }

  /// Activate
  virtual void Activate(csRenderMesh* mesh);

  /// Deactivate
  virtual void Deactivate();

  /// Setup states needed for proper operation of the shader
  virtual void SetupState (csRenderMesh* mesh);

  /// Reset states to original
  virtual void ResetState ();

  /// Add a variable to this context
  virtual bool AddVariable(iShaderVariable* variable){return false;}
  /// Get variable
  virtual iShaderVariable* GetVariable(int namehash) 
  { return privateGetVariable (namehash); }
  /// Get all variable stringnames in this context (used when creatingthem)
  virtual csBasicVector GetAllVariableNames() const {return csBasicVector();}

  virtual csSymbolTable* GetSymbolTable();

  /// Private variable to get the variable without virtual call
  inline iShaderVariable* privateGetVariable (int namehash);

  /// Loads a shader from buffer
  virtual bool Load(iDataBuffer* program);

  /// Loads from a document-node
  virtual bool Load(iDocumentNode* node);

  /// Prepares the shader for usage. Must be called before the shader is assigned to a material
  virtual bool Prepare();
};

class csShaderVariable : iShaderVariable
{
private:
  char *Name;
  int NameHash;

  VariableType Type;

  int Int;
  float Float;
  csRef<iString> String;
  csVector3 Vector3;
  csVector4 Vector4;

public:
  SCF_DECLARE_IBASE;

  csShaderVariable (iBase *);
  virtual ~csShaderVariable ();

  virtual VariableType GetType() const { return Type; }
  virtual void SetType(VariableType t) { Type = t; }

  virtual int GetHash() const { return NameHash; }

  virtual void SetName(const char* n)
    { if (Name) free (Name); Name = strdup (n); }
  virtual const char* GetName() const { return Name; }

  virtual bool GetValue(int& value) const
    { CS_ASSERT(Type == INT); value = Int; return true; }
  virtual bool GetValue(float& value) const
    { CS_ASSERT(Type == FLOAT); value = Float; return true; }
  virtual bool GetValue(iString* value) const
    { CS_ASSERT(Type == STRING); value = String; return true; }
  virtual bool GetValue(csVector3& value) const
    { CS_ASSERT(Type == VECTOR3); value = Vector3; return true; }
  virtual bool GetValue(csVector4& value) const
    { CS_ASSERT(Type == VECTOR4); value = Vector4; return true; }

  virtual bool SetValue(int value)
    { Type = INT; Int = value; return true; }
  virtual bool SetValue(float value)
    { Type = FLOAT; Float = value; return true; }
  virtual bool SetValue(iString* value)
    { Type = STRING; String = value; return true; }
  virtual bool SetValue(const csVector3 &value)
    { Type = VECTOR3; Vector3 = value; return true; }
  virtual bool SetValue(const csVector4 &value)
    { Type = VECTOR4; Vector4 = value; return true; }
};

#endif //__SHADERMGR_H__

