/*
  Copyright (C) 2002-2005 by Marten Svanfeldt
			     Anders Stenberg
			     Frank Richter

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
#include "csgeom/vector3.h"
#include "csplugincommon/opengl/glextmanager.h"
#include "csplugincommon/opengl/glstates.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "iutil/comp.h"
#include "iutil/document.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/shader/shader.h"
#include "glshader_ffp.h"
#include "glshader_fixed.h"

////////////////////////////////////////////////////////////////////
//                          csGLShaderFFP
////////////////////////////////////////////////////////////////////

CS_LEAKGUARD_IMPLEMENT (csGLShaderFFP);

csGLShaderFFP::csGLShaderFFP(csGLShader_FIXED* shaderPlug) :
  csShaderProgram (shaderPlug->object_reg)
{
  csGLShaderFFP::shaderPlug = shaderPlug;
  validProgram = false;

  BuildTokenHash();
}

csGLShaderFFP::~csGLShaderFFP ()
{
}

void csGLShaderFFP::Report (int severity, const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  csReportV (objectReg, severity, "crystalspace.graphics3d.shader.fixed.fp", 
    msg, args);
  va_end (args);
}

void csGLShaderFFP::BuildTokenHash()
{
  InitTokenTable (tokens);

  tokens.Register("primary color", GL_PRIMARY_COLOR);
  tokens.Register("texture", GL_TEXTURE);
  tokens.Register("constant color", GL_CONSTANT_ARB);
  tokens.Register("previous layer", GL_PREVIOUS_ARB);
  
  tokens.Register("color", GL_SRC_COLOR);
  tokens.Register("invertcolor", GL_ONE_MINUS_SRC_COLOR);
  tokens.Register("one minus color", GL_ONE_MINUS_SRC_COLOR);
  tokens.Register("alpha", GL_SRC_ALPHA);
  tokens.Register("invertalpha", GL_ONE_MINUS_SRC_ALPHA);
  tokens.Register("one minus alpha", GL_ONE_MINUS_SRC_ALPHA);

  tokens.Register("replace", GL_REPLACE);
  tokens.Register("modulate", GL_MODULATE);
  tokens.Register("add", GL_ADD);
  tokens.Register("add signed", GL_ADD_SIGNED_ARB);
  tokens.Register("interpolate", GL_INTERPOLATE_ARB);
  tokens.Register("subtract", GL_SUBTRACT_ARB);
  tokens.Register("dot3", GL_DOT3_RGB_ARB);
  tokens.Register("dot3 alpha", GL_DOT3_RGBA_ARB);
}


////////////////////////////////////////////////////////////////////
//                          iShaderProgram
////////////////////////////////////////////////////////////////////

bool csGLShaderFFP::Load (iShaderTUResolver*, iDocumentNode* node)
{
  if(!node)
    return false;

  csRef<iDocumentNode> mtexnode = node->GetNode("fixedfp");
  if(mtexnode)
  {
    csRef<iDocumentNodeIterator> it = mtexnode->GetNodes();
    while(it->HasNext())
    {
      csRef<iDocumentNode> child = it->Next();
      if(child->GetType() != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = tokens.Request (value);
      switch(id)
      {
        case XMLTOKEN_LAYER:
          {
            mtexlayer ml;
	    const char* name = child->GetAttributeValue ("name");
            if (!LoadLayer(&ml, child))
              return false;
            size_t idx = texlayers.Push (ml);
	    if (name != 0)
	      layerNames.Put (name, (int)idx);
          }
          break;
	case XMLTOKEN_FOG:
	  {
	    if (!ParseFog (child, fog))
	      return false;
	  }
	  break;
        default:
	  {
	    switch (commonTokens.Request (value))
	    {
	      case XMLTOKEN_PROGRAM:
	      case XMLTOKEN_VARIABLEMAP:
	      case XMLTOKEN_SHADERVAR:
		// Don't want those
		synsrv->ReportBadToken (child);
		return false;
		break;
	      default:
		if (!ParseCommon (child))
		  return false;
	    }
	  }
      }
    }
    CompactLayers();
    texlayers.ShrinkBestFit();
  }
  else
  {
    synsrv->ReportError ("crystalspace.graphics3d.shader.fixed.fp",
      node, "<fixedfp> node missing");
    return false;
  }
  return true;
}

bool csGLShaderFFP::LoadLayer (mtexlayer* layer, iDocumentNode* node)
{
  if(layer == 0 || node == 0)
    return false;

  csRef<iDocumentNodeIterator> it = node->GetNodes();

  while(it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next();
    if(child->GetType() != CS_NODE_ELEMENT) continue;
    csStringID id = tokens.Request(child->GetValue());
    switch (id)
    {
      case XMLTOKEN_COLORSOURCE:
        {
          int num = child->GetAttributeValueAsInt("num");

          if(num < 0 || num > 3 ) continue;

          const char* str = child->GetAttributeValue("source");
          if (str)
          {
            int i = tokens.Request(str);
            if(i == GL_PRIMARY_COLOR_ARB||i == GL_TEXTURE
	    	||i == GL_CONSTANT_ARB||i==GL_PREVIOUS_ARB)
            {
              layer->color.source[num] = i;
            }
            else
            {
              synsrv->Report ("crystalspace.graphics3d.shader.fixed",
                CS_REPORTER_SEVERITY_WARNING,
                child, "Invalid color source: %s", str);
            }
          }

          str = child->GetAttributeValue("modifier");
          if (str)
          {
            int m = tokens.Request(str);
            if(m == GL_SRC_COLOR ||m == GL_ONE_MINUS_SRC_COLOR
	    	||m == GL_SRC_ALPHA||m == GL_ONE_MINUS_SRC_ALPHA)
            {
              layer->color.mod[num] = m;
            }
            else
            {
              synsrv->Report ("crystalspace.graphics3d.shader.fixed",
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

          int i = tokens.Request(child->GetAttributeValue("source"));
          if(i == GL_PRIMARY_COLOR_ARB||i == GL_TEXTURE
	  	||i == GL_CONSTANT_ARB||i==GL_PREVIOUS_ARB)
            layer->alpha.source[num] = i;

          int m = tokens.Request(child->GetAttributeValue("modifier"));
          if(m == GL_SRC_ALPHA||m == GL_ONE_MINUS_SRC_ALPHA)
            layer->alpha.mod[num] = m;
        }
        break;
      case XMLTOKEN_COLOROPERATION:
        {
          int o = tokens.Request(child->GetAttributeValue("operation"));
          if(o == GL_REPLACE|| o == GL_MODULATE||o == GL_ADD
	  	||o == GL_ADD_SIGNED_ARB|| o == GL_INTERPOLATE_ARB
		||o == GL_SUBTRACT_ARB||o == GL_DOT3_RGB_ARB
		||o == GL_DOT3_RGBA_ARB)
            layer->color.op = o;
          if(child->GetAttribute("scale") != 0)
            layer->color.scale = child->GetAttributeValueAsFloat ("scale");
        }
        break;
      case XMLTOKEN_ALPHAOPERATION:
        {
          int o = tokens.Request(child->GetAttributeValue("operation"));
          if(o == GL_REPLACE|| o == GL_MODULATE||o == GL_ADD
	  	||o == GL_ADD_SIGNED_ARB|| o == GL_INTERPOLATE_ARB
		||o == GL_SUBTRACT_ARB||o == GL_DOT3_RGB_ARB
		||o == GL_DOT3_RGBA_ARB)
	    layer->alpha.op = o;
          if(child->GetAttribute("scale") != 0)
	    layer->alpha.scale = child->GetAttributeValueAsFloat ("scale");
        }
        break;
      default:
	synsrv->ReportBadToken (child);
        return false;
    }
  }
  return true;
}

bool csGLShaderFFP::ParseFog (iDocumentNode* node, FogInfo& fog)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes();

  while(it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next();
    if(child->GetType() != CS_NODE_ELEMENT) continue;
    csStringID id = tokens.Request(child->GetValue());
    switch (id)
    {
      case XMLTOKEN_MODE:
	{
	  const char* type = child->GetContentsValue ();
	  if (type == 0)
	  {
	    synsrv->Report ("crystalspace.graphics3d.shader.glfixed",
	      CS_REPORTER_SEVERITY_WARNING,
	      child,
	      "Node has no contents");
	    return false;
	  }
	  if (strcmp (type, "linear") == 0)
	  {
	    fog.mode = FogLinear;
	  }
	  else if (strcmp (type, "exp") == 0)
	  {
	    fog.mode = FogExp;
	  }
	  else if (strcmp (type, "exp2") == 0)
	  {
	    fog.mode = FogExp2;
	  }
	  else
	  {
	    synsrv->Report ("crystalspace.graphics3d.shader.glfixed",
	      CS_REPORTER_SEVERITY_WARNING,
	      child,
	      "Invalid fog mode %s", type);
	    return false;
	  }
	}
	break;
      case XMLTOKEN_DENSITY:
	{
	  if (!ParseProgramParam (child, fog.density, ParamFloat))
	    return false;
	}
	break;
      case XMLTOKEN_START:
	{
	  if (!ParseProgramParam (child, fog.start, ParamFloat))
	    return false;
	}
	break;
      case XMLTOKEN_END:
	{
	  if (!ParseProgramParam (child, fog.end, ParamFloat))
	    return false;
	}
	break;
      case XMLTOKEN_FOGCOLOR:
	{
	  if (!ParseProgramParam (child, fog.start, ParamFloat | ParamVector3 |
	    ParamVector4))
	    return false;
	}
	break;
      default:
	synsrv->ReportBadToken (child);
        return false;
    }
  }
  return true;
}

//#define DUMP_LAYERS

#ifdef DUMP_LAYERS
#include "csplugincommon/opengl/glenum_identstrs.h"
#endif

void csGLShaderFFP::DumpTexFunc (const mtexlayer::TexFunc& tf)
{
#ifdef DUMP_LAYERS
  csString srcStr, modStr, opStr;
  int j;
  for (j = 0; j < 2; j++)
  {
    srcStr = csOpenGLEnums.StringForIdent (tf.source[j]);
    if (srcStr.IsEmpty()) srcStr.Format ("%.4x", tf.source[j]);
    modStr = csOpenGLEnums.StringForIdent (tf.mod[j]);
    if (modStr.IsEmpty()) modStr.Format ("%.4x", tf.mod[j]);
    csPrintf (" %-23s %-23s\n", srcStr.GetData(), modStr.GetData());
  }
  opStr = csOpenGLEnums.StringForIdent (tf.op);
  if (opStr.IsEmpty()) opStr.Format ("%.4x", tf.op);
  csPrintf (" %s %f\n\n", opStr.GetData(), tf.scale);
#endif
}

static int GetUsedLayersCount (GLenum op)
{
  switch (op)
  {
    case GL_REPLACE:
      return 1;
    case GL_MODULATE:
    case GL_ADD:
    case GL_ADD_SIGNED_ARB:
    case GL_SUBTRACT_ARB:
    case GL_DOT3_RGB_ARB:
    case GL_DOT3_RGBA_ARB:
      return 2;
    case GL_INTERPOLATE_ARB:
      return 3;
    default:
      return 0;
  }
}

static uint LayerSourceToFlag (GLenum source)
{
  switch (source)
  {
    case GL_TEXTURE:		return 1;
    case GL_CONSTANT_ARB:	return 2;
    //case GL_PRIMARY_COLOR_ARB:  return 4;
    default:			return 0;
  }
}

void csGLShaderFFP::CompactLayers()
{
  if (texlayers.Length() >= 2)
  {
    CS_ALLOC_STACK_ARRAY(uint, layerUseFlags, texlayers.Length());
    CS_ALLOC_STACK_ARRAY(int, layerMap, texlayers.Length());
    memset (layerUseFlags, 0, sizeof (uint) * texlayers.Length());
    size_t p;
    for (p = 0; p < texlayers.Length(); p++)
    {
      const mtexlayer& tl = texlayers[p];
      int i;
      for (i = 0; i < GetUsedLayersCount (tl.color.op); i++)
      {
	layerUseFlags[p] |= LayerSourceToFlag (tl.color.source[i]);
      }
      for (i = 0; i < GetUsedLayersCount (tl.alpha.op); i++)
      {
	layerUseFlags[p] |= LayerSourceToFlag (tl.alpha.source[i]);
      }
    }

    csArray<mtexlayer> newlayers;
    mtexlayer nextlayer = texlayers[0];
    p = 0;
    size_t layerOfs = 0;
    while (p+1 < texlayers.Length())
    {
      // Check if used resources overlap
      if ((layerUseFlags[p] & layerUseFlags[p+1]) == 0)
      {
	// No, possibility to merge
	const mtexlayer& tl1 = nextlayer;
	const mtexlayer& tl2 = texlayers[p+1];
	mtexlayer newlayer;
	bool newColor = TryMergeTexFuncs (newlayer.color, tl1.color, 
	  tl2.color);
	bool newAlpha = TryMergeTexFuncs (newlayer.alpha, tl1.alpha, 
	  tl2.alpha);

	if (newColor && newAlpha)
	{
	  nextlayer = newlayer;
	  layerMap[p] = (int)(p - layerOfs);
	  p++;
	  layerOfs++;
	  continue;
	}
      }

      newlayers.Push (nextlayer);
      layerMap[p] = (int)(p - layerOfs);
      p++;
      nextlayer = texlayers[p];
    }
    newlayers.Push (nextlayer);
    layerMap[p] = (int)(p - layerOfs);
    texlayers = newlayers;

    csHash<int, csStrKey, csConstCharHashKeyHandler>::GlobalIterator layerNameIt 
      = layerNames.GetIterator();
    while (layerNameIt.HasNext())
    {
      csStrKey key;
      int layerNum = layerNameIt.Next (key);
      layerNames.PutUnique (key, layerMap[layerNum]);
    }
  }

#ifdef DUMP_LAYERS
  {
    for (size_t i = 0; i < texlayers.Length(); i++)
    {
      csPrintf ("Layer %zu:\n", i);
      const mtexlayer& tl = texlayers[i];
      DumpTexFunc (tl.color);
      DumpTexFunc (tl.alpha);
    }
  }
#endif
}

bool csGLShaderFFP::TryMergeTexFuncs (mtexlayer::TexFunc& newTF, 
				      const mtexlayer::TexFunc& tf1, 
				      const mtexlayer::TexFunc& tf2)
{
  if ((tf2.op == GL_REPLACE) 
    && (tf2.source[0] == GL_PREVIOUS_ARB))
  {
    newTF = tf1;
    return true;
  }
  else if ((tf1.op == GL_REPLACE) 
    && (fabsf (tf1.scale - 1.0f) < EPSILON))
  {
    int prevLayIdx = -1;
    for (int i = 0; i < GetUsedLayersCount (tf2.op); i++)
    {
      if (tf2.source[i] == GL_PREVIOUS_ARB)
      {
	prevLayIdx = i;
	break;
      }
    }
    if (prevLayIdx != -1)
    {
      newTF = tf2;
      newTF.source[prevLayIdx] = tf1.source[0];
      return true;
    }
  }
  return false;
}

bool csGLShaderFFP::Compile (csArray<iShaderVariableContext*> &staticContexts)
{
  shaderPlug->Open ();
  ext = shaderPlug->ext;

  maxlayers = shaderPlug->texUnits;

  //get a statecache
  csRef<iGraphics2D> g2d = CS_QUERY_REGISTRY (objectReg, iGraphics2D);
  g2d->PerformExtension ("getstatecache", &statecache);

  //get extension-object
  g3d = CS_QUERY_REGISTRY (objectReg, iGraphics3D);
  csRef<iShaderRenderInterface> sri = 
    SCF_QUERY_INTERFACE (g3d, iShaderRenderInterface);
  if (!sri) return false;

  ext = (csGLExtensionManager*) sri->GetPrivateObject ("ext");

  if (texlayers.Length () > (size_t)maxlayers)
    return false;

  // Don't support layers if the COMBINE ext isn't present
  if ((!shaderPlug->enableCombine) && (texlayers.Length() > 0))
    return false;

  const bool hasDOT3 = ext->CS_GL_ARB_texture_env_dot3 || 
    ext->CS_GL_EXT_texture_env_dot3;

  for(size_t i = 0; i < texlayers.Length(); ++i)
  {
    const mtexlayer& layer = texlayers[i];
    if (((layer.color.op == GL_DOT3_RGB_ARB) || 
        (layer.color.op == GL_DOT3_RGBA_ARB)) && 
        !(hasDOT3))
      return false;
    if (((layer.alpha.op == GL_DOT3_RGB_ARB) || 
        (layer.alpha.op == GL_DOT3_RGBA_ARB)) && 
        !(hasDOT3))
      return false;
  }

  ResolveParamStatic (fog.density, staticContexts);
  ResolveParamStatic (fog.start, staticContexts);
  ResolveParamStatic (fog.end, staticContexts);
  ResolveParamStatic (fog.color, staticContexts);

  validProgram = true;

  return true;
}

void csGLShaderFFP::ActivateTexFunc (const mtexlayer::TexFunc& tf, 
				     GLenum sourceP, GLenum operandP, 
				     GLenum combineP, GLenum scaleP)
{
  for (int i = 0; i < 3; i++)
  {
    if (tf.source[i] != -1)
    {
      glTexEnvi (GL_TEXTURE_ENV, sourceP + i, tf.source[i]);
      glTexEnvi (GL_TEXTURE_ENV, operandP + i, tf.mod[i]);
    }
  }
  glTexEnvi (GL_TEXTURE_ENV, combineP, tf.op);
  glTexEnvf (GL_TEXTURE_ENV, scaleP, tf.scale);
}

void csGLShaderFFP::Activate ()
{
  for(size_t i = 0; i < texlayers.Length(); ++i)
  {
    statecache->SetActiveTU ((int)i);
    statecache->ActivateTU ();

    if (shaderPlug->enableCombine)
    {
      const mtexlayer& layer = texlayers[i];

      ActivateTexFunc (layer.color, GL_SOURCE0_RGB_ARB, GL_OPERAND0_RGB_ARB,
	GL_COMBINE_RGB_ARB, GL_RGB_SCALE_ARB);
      ActivateTexFunc (layer.alpha, GL_SOURCE0_ALPHA_ARB, GL_OPERAND0_ALPHA_ARB,
	GL_COMBINE_ALPHA_ARB, GL_ALPHA_SCALE);
    }
  }
  if (fog.mode != FogOff)
  {
    statecache->Enable_GL_FOG ();
  }
}

void csGLShaderFFP::Deactivate()
{
  statecache->SetActiveTU (0);
  statecache->ActivateTU ();
  if (shaderPlug->enableCombine)
  {
    glTexEnvi  (GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
    glTexEnvi  (GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
    glTexEnvi  (GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PRIMARY_COLOR);
    glTexEnvi  (GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
    glTexEnvi  (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
    glTexEnvi  (GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);

    glTexEnvi  (GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
    glTexEnvi  (GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
    glTexEnvi  (GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, GL_PRIMARY_COLOR);
    glTexEnvi  (GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, GL_SRC_ALPHA);
    glTexEnvi  (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_MODULATE);
    glTexEnvi  (GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);
  }

  if (fog.mode != FogOff)
  {
    statecache->Disable_GL_FOG ();
  }
}

void csGLShaderFFP::SetupState (const csRenderMesh *mesh, 
                                csRenderMeshModes& modes,
                                const csShaderVarStack &stacks)
{
  if (fog.mode != FogOff)
  {
    RetrieveParamValue (fog.density, stacks);
    RetrieveParamValue (fog.start, stacks);
    RetrieveParamValue (fog.end, stacks);
    RetrieveParamValue (fog.color, stacks);
    glFogfv (GL_FOG_COLOR, (float*)&fog.color.vectorValue);

    switch (fog.mode)
    {
      case FogLinear:
	{
	  glFogi (GL_FOG_MODE, GL_LINEAR);
	  glFogf (GL_FOG_START, fog.start.vectorValue.x);
	  glFogf (GL_FOG_END, fog.end.vectorValue.x);
	}
	break;
      case FogExp:
	{
	  glFogi (GL_FOG_MODE, GL_EXP);
	  glFogf (GL_FOG_DENSITY, fog.density.vectorValue.x);
	}
	break;
      case FogExp2:
	{
	  glFogi (GL_FOG_MODE, GL_EXP2);
	  glFogf (GL_FOG_DENSITY, fog.density.vectorValue.x);
	}
	break;
      default:
	break;
    }
  }
}

void csGLShaderFFP::ResetState ()
{
}
