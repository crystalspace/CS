/*
Copyright (C) 2002 by Mårten Svanfeldt
                      Anders Stenberg

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __GLSHADER_MTEX_H__
#define __GLSHADER_MTEX_H__

#include "ivideo/shader/shader.h"

class csGLRender3D;

class csGLShader_MTEX : public iShaderProgramPlugin
{
private:

  csGLExtensionManager* ext;
  csRef<iObjectRegistry> object_reg;
  


public:
  SCF_DECLARE_IBASE;
  
  csGLShader_MTEX (iBase *parent);
  virtual ~csGLShader_MTEX ();


  
  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgramPlugin
  ////////////////////////////////////////////////////////////////////
  virtual csPtr<iShaderProgram> CreateProgram(const char* type);

  virtual bool SupportType(const char* type);

  virtual void Open();

  virtual csPtr<iString> GetProgramID(const char* programstring);

  ////////////////////////////////////////////////////////////////////
  //                          iComponent
  ////////////////////////////////////////////////////////////////////

  bool Initialize (iObjectRegistry* reg);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGLShader_MTEX);
    virtual bool Initialize (iObjectRegistry* reg)
      { return scfParent->Initialize (reg); }
  } scfiComponent;
};

class csShaderGLMTEX : public iShaderProgram
{
private:

  iGLTextureCache* txtcache;
  csGLExtensionManager* ext;
  csRef<iObjectRegistry> object_reg;

  csGLStateCache* statecache;

  char* programstring;
  
  csHashMap variables;
  bool validProgram;

  // programloading stuff
  enum
  {
    XMLTOKEN_GLMTEX = 1,
    XMLTOKEN_LAYER,
    XMLTOKEN_COLORSOURCE,
    XMLTOKEN_TEXTURESOURCE,
    XMLTOKEN_TEXCOORDSOURCE,
    XMLTOKEN_ENVIRONMENT,
    XMLTOKEN_ALPHASOURCE,
    XMLTOKEN_COLOROP,
    XMLTOKEN_ALPHAOP,
    XMLTOKEN_COLORSCALE,
    XMLTOKEN_ALPHASCALE,
  };

  csStringHash xmltokens;

  void BuildTokenHash();

  // Layers of multitexturing
  enum COLORSOURCE
  {
    CS_COLORSOURCE_MESH,
    CS_COLORSOURCE_STREAM,
    CS_COLORSOURCE_NONE
  };

  enum TEXCOORDSOURCE
  {
    CS_TEXCOORDSOURCE_MESH,
    CS_TEXCOORDSOURCE_STREAM,
    CS_TEXCOORDSOURCE_NONE
  };

  struct mtexlayer
  {
    //remember which vars to reset on deactivate
    bool doCArr, doTCArr, doTexture;

    //texturenumber to use
    int texnum;

    //where to get color
    COLORSOURCE ccsource;
    char* colorstream;

    //where to get texturecoords
    TEXCOORDSOURCE tcoordsource;
    char* tcoordstream;

    //colorsource
    int colorsource[3];
    //color modifier
    int colormod[3];
    //color operation
    int colorp;

    //alphasource
    int alphasource[4];
    //alpha modifier
    int alphamod[4];
    //alpha operation
    int alphap;

    // rgb scale
    float scale_rgb[3];
    // alpha scale
    float scale_alpha;
    mtexlayer()
    {
      texnum = -1;
      ccsource = CS_COLORSOURCE_MESH;
      tcoordsource = CS_TEXCOORDSOURCE_MESH;
      colorsource[0] = GL_PREVIOUS_ARB; colorsource[1] = GL_TEXTURE; colorsource[2] = -1;
      colormod[0] = GL_SRC_COLOR; colormod[1] = GL_SRC_COLOR; colormod[2] = GL_SRC_COLOR;
      colorp = GL_MODULATE;

      alphasource[0] = GL_PREVIOUS_ARB; alphasource[1] = GL_TEXTURE; alphasource[2] = -1;
      alphamod[0] = GL_SRC_ALPHA; alphamod[1] = GL_SRC_ALPHA; alphamod[2] = GL_SRC_ALPHA;
      alphap = GL_MODULATE;

      scale_rgb[0] = scale_rgb[1] = scale_rgb[2] = 1.0f;
      scale_alpha = 1.0f;

      colorstream = tcoordstream = NULL;
      doCArr = doTCArr = doTexture = false;
    }

    ~mtexlayer()
    {
      if(colorstream) delete colorstream;
      if(tcoordstream) delete tcoordstream;
    }
  };

  //array of mtexlayers
  csBasicVector texlayers;

  //maximum number of layers
  int maxlayers;

  void UpdateValid();
  
  bool LoadLayer(mtexlayer* layer, iDocumentNode* node);
  bool LoadEnvironment(mtexlayer* layer, iDocumentNode* node);
public:
  SCF_DECLARE_IBASE;

  csShaderGLMTEX(iObjectRegistry* objreg);

  virtual ~csShaderGLMTEX ()
  {
    Deactivate(NULL, NULL);
    if(programstring) delete programstring;
  }

  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgram
  ////////////////////////////////////////////////////////////////////

  virtual csPtr<iString> GetProgramID();

  /// Sets this program to be the one used when rendering
  virtual void Activate(iShaderPass* current, csRenderMesh* mesh);

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate(iShaderPass* current, csRenderMesh* mesh);

  /* Propertybag - get property, return false if no such property found
   * Which properties there is is implementation specific
   */
  virtual bool GetProperty(const char* name, iString* string) {return false;};
  virtual bool GetProperty(const char* name, int* string) {return false;};
  virtual bool GetProperty(const char* name, csVector3* string) {return false;};
//  virtual bool GetProperty(const char* name, csVector4* string) {};

  /* Propertybag - set property.
   * Which properties there is is implementation specific
   */
  virtual bool SetProperty(const char* name, iString* string) {return false;};
  virtual bool SetProperty(const char* name, int* string) {return false;};
  virtual bool SetProperty(const char* name, csVector3* string) {return false;};
//  virtual bool SetProperty(const char* name, csVector4* string) {return false;};

  /// Add a variable to this context
  virtual bool AddVariable(iShaderVariable* variable) 
    { /*do not allow externals to add variables*/ return false; };
  /// Get variable
  virtual iShaderVariable* GetVariable(const char* string);
  /// Get all variable stringnames added to this context (used when creatingthem)
  virtual csBasicVector GetAllVariableNames(); 

  /// Check if valid
  virtual bool IsValid() { return validProgram;} 

  /// Loads shaderprogram from buffer
  virtual bool Load(iDataBuffer* program);

  /// Loads from a document-node
  virtual bool Load(iDocumentNode* node);

  /// Prepares the shaderprogram for usage. Must be called before the shader is assigned to a material
  virtual bool Prepare();
};


#endif //__GLSHADER_MTEX_H__

