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

#include "cssysdef.h"

#include "csutil/hashmap.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/csmd5.h"
#include "csutil/scfstr.h"
#include "csgeom/vector3.h"
#include "csutil/xmltiny.h"

#include "iutil/document.h"
#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "ivideo/material.h"
#include "ivaria/reporter.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"
#include "ivideo/shader/shader.h"

#include "video/canvas/openglcommon/glextmanager.h"
#include "video/canvas/openglcommon/glstates.h"

#include "glshader_ffp.h"
#include "glshader_fixed.h"


////////////////////////////////////////////////////////////////////
//                          csGLShaderFFP
////////////////////////////////////////////////////////////////////

SCF_IMPLEMENT_IBASE(csGLShaderFFP)
  SCF_IMPLEMENTS_INTERFACE(iShaderProgram)
SCF_IMPLEMENT_IBASE_END

csGLShaderFFP::csGLShaderFFP(csGLShader_FIXED* shaderPlug)
{
  SCF_CONSTRUCT_IBASE (0);

  csGLShaderFFP::shaderPlug = shaderPlug;
  object_reg = shaderPlug->object_reg;
  ext = shaderPlug->ext;
  programstring = 0;
  validProgram = true;

  SyntaxService = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
}

void csGLShaderFFP::Report (int severity, const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  csReportV (object_reg, severity, "crystalspace.graphics3d.shader.fixed.fp", 
    msg, args);
  va_end (args);
}

void csGLShaderFFP::BuildTokenHash()
{
  xmltokens.Register ("alphaoperation", XMLTOKEN_ALPHAOP);
  xmltokens.Register ("alphasource", XMLTOKEN_ALPHASOURCE);
  xmltokens.Register ("coloroperation", XMLTOKEN_COLOROP);
  xmltokens.Register ("colorsource", XMLTOKEN_COLORSOURCE);
  xmltokens.Register ("layer", XMLTOKEN_LAYER);
  xmltokens.Register ("environment", XMLTOKEN_ENVIRONMENT);

  xmltokens.Register("integer", 100+csShaderVariable::INT);
  xmltokens.Register("float", 100+csShaderVariable::FLOAT);
  xmltokens.Register("string", 100+csShaderVariable::STRING);
  xmltokens.Register("vector3", 100+csShaderVariable::VECTOR3);

  xmltokens.Register("primary color", GL_PRIMARY_COLOR);
  xmltokens.Register("texture", GL_TEXTURE);
  xmltokens.Register("constant color", GL_CONSTANT_ARB);
  xmltokens.Register("previous layer", GL_PREVIOUS_ARB);
  
  xmltokens.Register("color", GL_SRC_COLOR);
  xmltokens.Register("invertcolor", GL_ONE_MINUS_SRC_COLOR);
  xmltokens.Register("alpha", GL_SRC_ALPHA);
  xmltokens.Register("invertalpha", GL_ONE_MINUS_SRC_ALPHA);

  xmltokens.Register("replace", GL_REPLACE);
  xmltokens.Register("modulate", GL_MODULATE);
  xmltokens.Register("add", GL_ADD);
  xmltokens.Register("add signed", GL_ADD_SIGNED_ARB);
  xmltokens.Register("interpolate", GL_INTERPOLATE_ARB);
  xmltokens.Register("subtract", GL_SUBTRACT_ARB);
  xmltokens.Register("dot3", GL_DOT3_RGB_ARB);
  xmltokens.Register("dot3 alpha", GL_DOT3_RGBA_ARB);
}


////////////////////////////////////////////////////////////////////
//                          iShaderProgram
////////////////////////////////////////////////////////////////////

bool csGLShaderFFP::Load(iDocumentNode* node)
{
  if(!node)
    return false;

  BuildTokenHash();

  csRef<iDocumentNode> mtexnode = node->GetNode("fixedfp");
  if(mtexnode)
  {
    csRef<iDocumentNodeIterator> it = mtexnode->GetNodes();
    while(it->HasNext())
    {
      csRef<iDocumentNode> child = it->Next();
      if(child->GetType() != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch(id)
      {
        case XMLTOKEN_LAYER:
          {
            mtexlayer* ml = new mtexlayer();
            if(!LoadLayer(ml, child))
              return false;
            texlayers.Push (*ml);
          }
          break;
      }
    } 
  }
  return true;
}

bool csGLShaderFFP::LoadLayer(mtexlayer* layer, iDocumentNode* node)
{
  if(layer == 0 || node == 0)
    return false;

  csRef<iDocumentNodeIterator> it = node->GetNodes();

  while(it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next();
    if(child->GetType() != CS_NODE_ELEMENT) continue;
    csStringID id = xmltokens.Request(child->GetValue());
    switch (id)
    {
      case XMLTOKEN_COLORSOURCE:
        {
          int num = child->GetAttributeValueAsInt("num");

          if(num < 0 || num > 3 ) continue;

          const char* str = child->GetAttributeValue("source");
          if (str)
          {
            int i = xmltokens.Request(str);
            if(i == GL_PRIMARY_COLOR_ARB||i == GL_TEXTURE
	    	||i == GL_CONSTANT_ARB||i==GL_PREVIOUS_ARB)
            {
              layer->colorsource[num] = i;
            }
            else
            {
              SyntaxService->Report ("crystalspace.graphics3d.shader.fixed",
                CS_REPORTER_SEVERITY_WARNING,
                child, "Invalid color source: %s", str);
            }
          }

          str = child->GetAttributeValue("modifier");
          if (str)
          {
            int m = xmltokens.Request(str);
            if(m == GL_SRC_COLOR ||m == GL_ONE_MINUS_SRC_COLOR
	    	||m == GL_SRC_ALPHA||m == GL_ONE_MINUS_SRC_ALPHA)
            {
              layer->colormod[num] = m;
            }
            else
            {
              SyntaxService->Report ("crystalspace.graphics3d.shader.fixed",
                CS_REPORTER_SEVERITY_WARNING,
                child, "Invalid color modifier: %s", str);
            }
          }
        }
        break;
      case XMLTOKEN_ALPHASOURCE:
        {
          int num = child->GetAttributeValueAsInt("num");

          if(num < 0 || num > 3 )
            continue;

          int i = xmltokens.Request(child->GetAttributeValue("source"));
          if(i == GL_PRIMARY_COLOR_ARB||i == GL_TEXTURE
	  	||i == GL_CONSTANT_ARB||i==GL_PREVIOUS_ARB)
            layer->alphasource[num] = i;

          int m = xmltokens.Request(child->GetAttributeValue("modifier"));
          if(m == GL_SRC_ALPHA||m == GL_ONE_MINUS_SRC_ALPHA)
            layer->alphamod[num] = m;
        }
        break;
      case XMLTOKEN_COLOROP:
        {
          int o = xmltokens.Request(child->GetAttributeValue("operation"));
          if(o == GL_REPLACE|| o == GL_MODULATE||o == GL_ADD
	  	||o == GL_ADD_SIGNED_ARB|| o == GL_INTERPOLATE_ARB
		||o == GL_SUBTRACT_ARB||o == GL_DOT3_RGB_ARB
		||o == GL_DOT3_RGBA_ARB)
            layer->colorp = o;
          if(child->GetAttribute("scale") != 0)
            layer->scale_rgb = child->GetAttributeValueAsFloat ("scale");
        }
        break;
      case XMLTOKEN_ALPHAOP:
        {
          int o = xmltokens.Request(child->GetAttributeValue("operation"));
          if(o == GL_REPLACE|| o == GL_MODULATE||o == GL_ADD
	  	||o == GL_ADD_SIGNED_ARB|| o == GL_INTERPOLATE_ARB
		||o == GL_SUBTRACT_ARB||o == GL_DOT3_RGB_ARB
		||o == GL_DOT3_RGBA_ARB)
            layer->alphap = o;
          if(child->GetAttribute("scale") != 0)
            layer->scale_alpha = child->GetAttributeValueAsFloat ("scale");
        }
        break;
    }
  }
  return true;
}

bool csGLShaderFFP::Load(iDataBuffer* program)
{
  csRef<iDocumentSystem> xml (CS_QUERY_REGISTRY (object_reg, iDocumentSystem));
  if (!xml) xml = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
  csRef<iDocument> doc = xml->CreateDocument ();
  const char* error = doc->Parse (program);
  if (error != 0)
  { 
    Report (CS_REPORTER_SEVERITY_ERROR, "Document error '%s'!", error);
    return false;
  }
  return Load(doc->GetRoot());
}

bool csGLShaderFFP::Prepare()
{
  maxlayers = shaderPlug->texUnits;

  //get a statecache
  csRef<iGraphics2D> g2d = CS_QUERY_REGISTRY (object_reg, iGraphics2D);
  g2d->PerformExtension("getstatecache", &statecache);

  //get extension-object
  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  csRef<iShaderRenderInterface> sri = 
    SCF_QUERY_INTERFACE (g3d, iShaderRenderInterface);

  ext = (csGLExtensionManager*) sri->GetPrivateObject ("ext");

  if (texlayers.Length () > maxlayers)
    return false;

  return true;
}

void csGLShaderFFP::Activate(iShaderPass* current, csRenderMesh* mesh)
{
  for(int i = 0; i < MIN(maxlayers, texlayers.Length()); ++i)
  {
    mtexlayer* layer = &texlayers[i];
    ext->glActiveTextureARB(GL_TEXTURE0_ARB + i);
    ext->glClientActiveTextureARB(GL_TEXTURE0_ARB + i);

    if(ext->CS_GL_ARB_texture_env_combine || 
      ext->CS_GL_EXT_texture_env_combine)
    {
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
      glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);

      glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, layer->colorsource[0]);
      glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, layer->colormod[0]);
      glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, layer->colorsource[1]);
      glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, layer->colormod[1]);
      if (layer->colorsource[2] != -1)
      {
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, layer->colorsource[2]);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, layer->colormod[2]);
      }

      if( (layer->colorp != GL_DOT3_RGB_ARB)
      		&& (layer->colorp  != GL_DOT3_RGBA_ARB))
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, layer->colorp );
      else if (ext->CS_GL_ARB_texture_env_dot3
      		|| ext->CS_GL_EXT_texture_env_dot3)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, layer->colorp );

      glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, layer->scale_rgb);

      glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, layer->alphasource[0]);
      glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, layer->alphamod[0]);
      glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, layer->alphasource[1]);
      glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, layer->alphamod[1]);
      if (layer->alphasource[2] != -1)
      {
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_ARB, layer->alphasource[2]);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_ARB, layer->alphamod[2]);
      }

      if( (layer->colorp != GL_DOT3_RGB_ARB)
      		&& (layer->colorp  != GL_DOT3_RGBA_ARB))
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, layer->alphap);
      else if (ext->CS_GL_ARB_texture_env_dot3
      		|| ext->CS_GL_EXT_texture_env_dot3)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, layer->alphap);

      glTexEnvf(GL_TEXTURE_ENV, GL_ALPHA_SCALE, layer->scale_alpha);
    }
  }
}

void csGLShaderFFP::Deactivate(iShaderPass* current)
{
  for (int i=maxlayers-1; i>=0; --i)
  {
    ext->glActiveTextureARB(GL_TEXTURE0_ARB+i);
    ext->glClientActiveTextureARB(GL_TEXTURE0_ARB+i);
    glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  }
}

void csGLShaderFFP::SetupState (iShaderPass *current, csRenderMesh *mesh)
{
}

void csGLShaderFFP::ResetState ()
{
}
