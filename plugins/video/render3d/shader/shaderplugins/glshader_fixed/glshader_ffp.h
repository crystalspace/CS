/*
  Copyright (C) 2002 by Marten Svanfeldt
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

#ifndef __GLSHADER_FIXEDFP_H__
#define __GLSHADER_FIXEDFP_H__

#include "ivideo/shader/shader.h"
#include "csutil/weakref.h"
#include "csutil/leakguard.h"
#include "csgfx/shadervar.h"
#include "imap/services.h"
#include "csplugincommon/shader/shaderplugin.h"
#include "csplugincommon/shader/shaderprogram.h"

class csGLShader_FIXED;

class csGLShaderFFP : public csShaderProgram
{
private:
  csStringHash tokens;
#define CS_TOKEN_ITEM_FILE \
  "plugins/video/render3d/shader/shaderplugins/glshader_fixed/glshader_ffp.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE

  csWeakRef<iGraphics3D> g3d;
  csGLExtensionManager* ext;
  csGLShader_FIXED* shaderPlug;

  csGLStateCache* statecache;

  bool validProgram;

  enum FogMode
  {
    FogOff,
    FogLinear,
    FogExp,
    FogExp2
  };

  struct FogInfo
  {
    FogMode mode;

    ProgramParam density;
    ProgramParam start;
    ProgramParam end;
    ProgramParam color;

    FogInfo () : mode (FogOff) {}
  };
  friend struct FogInfo; // MSVC6: For access to csGLStateCache::FogOff.

  FogInfo fog;

  // Layers of multitexturing
  struct mtexlayer
  {
    //remember which vars to reset on deactivate
    bool doTexture;

    //texturenumber to use
    int texnum;
    csRef<iTextureHandle> texturehandle;

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
    float scale_rgb;
    // alpha scale
    float scale_alpha;
    mtexlayer()
    {
      texnum = -1;
      doTexture = false;

      colorsource[0] = GL_PREVIOUS_ARB;
      colorsource[1] = GL_TEXTURE;
      colorsource[2] = -1;
      colormod[0] = GL_SRC_COLOR;
      colormod[1] = GL_SRC_COLOR;
      colormod[2] = GL_SRC_COLOR;
      colorp = GL_MODULATE;

      alphasource[0] = GL_PREVIOUS_ARB;
      alphasource[1] = GL_TEXTURE;
      alphasource[2] = -1;
      alphamod[0] = GL_SRC_ALPHA;
      alphamod[1] = GL_SRC_ALPHA;
      alphamod[2] = GL_SRC_ALPHA;
      alphap = GL_MODULATE;

      scale_rgb = 1.0f;
      scale_alpha = 1.0f;
    }
  };

  //array of mtexlayers
  csArray<mtexlayer> texlayers;

  //maximum number of layers
  int maxlayers;

  void Report (int severity, const char* msg, ...);

  void UpdateValid();
  
  bool LoadLayer(mtexlayer* layer, iDocumentNode* node);
  bool LoadEnvironment(mtexlayer* layer, iDocumentNode* node);
  bool ParseFog (iDocumentNode* node, FogInfo& fog);

  void BuildTokenHash();
public:
  CS_LEAKGUARD_DECLARE (csGLShaderFFP);

  csGLShaderFFP(csGLShader_FIXED* shaderPlug);
  virtual ~csGLShaderFFP ();

  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgram
  ////////////////////////////////////////////////////////////////////

  /// Sets this program to be the one used when rendering
  virtual void Activate ();

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate ();

  /// Setup states needed for proper operation of the shader
  virtual void SetupState (const csRenderMesh* mesh,
    const csShaderVarStack &stacks);

  /// Reset states to original
  virtual void ResetState ();

  /// Loads from a document-node
  virtual bool Load(iDocumentNode* node);

  /// Loads from raw text
  virtual bool Load (const char* program, csArray<csShaderVarMapping> &mappings)
  { return false; }

  /// Compile a program
  virtual bool Compile(csArray<iShaderVariableContext*> &staticContexts);

  virtual int ResolveTextureBinding (const char* binding)
  { return -1; }
};


#endif //__GLSHADER_FIXEDFP_H__

