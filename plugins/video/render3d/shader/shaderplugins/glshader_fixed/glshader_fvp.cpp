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

#include "cssysdef.h"

#include "csutil/hashmap.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/scfstr.h"
#include "csutil/csmd5.h"
#include "csgeom/vector3.h"
#include "csutil/xmltiny.h"

#include "iutil/document.h"
#include "iutil/string.h"
#include "iutil/strset.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"
#include "ivideo/rendermesh.h"
#include "ivideo/shader/shader.h"

#include "plugins/video/canvas/openglcommon/glextmanager.h"
#include "plugins/video/canvas/openglcommon/glstates.h"

#include "glshader_fvp.h"
#include "glshader_fixed.h"

CS_LEAKGUARD_IMPLEMENT (csGLShaderFVP)

csGLShaderFVP::csGLShaderFVP (csGLShader_FIXED* shaderPlug) :
  csShaderProgram (shaderPlug->object_reg)
{
  validProgram = true;
  csGLShaderFVP::shaderPlug = shaderPlug;

  init_token_table (tokens);
}

csGLShaderFVP::~csGLShaderFVP ()
{
}

void csGLShaderFVP::Activate ()
{
}

void csGLShaderFVP::Deactivate()
{
}

void csGLShaderFVP::SetupState (const csRenderMesh *mesh, 
				const csShaderVarStack &stacks)
{
  size_t i;

  csRef<csShaderVariable> var;

  if (do_lighting)
  {
    statecache->SetMatrixMode (GL_MODELVIEW_MATRIX);
    glPushMatrix ();
    glLoadIdentity ();

    for(i = 0; i < lights.Length(); ++i)
    {

      //fix vars for this light
      /*for(j = 0; j < lights[i].dynVars.Length (); j++)
      {
        *((csRef<csShaderVariable>*)lights[i].dynVars.Get(j).userData) = 
          lights[i].dynVars.Get(j).shaderVariable;
        lights[i].dynVars.Get(j).shaderVariable = 0;
      }*/

      int l = lights[i].lightnum;
      glEnable (GL_LIGHT0+l);

      var = lights[i].positionVarRef;
      if (!var && lights[i].positionvar < (csStringID)stacks.Length ()
          && stacks[lights[i].positionvar].Length () > 0)
        var = stacks[lights[i].positionvar].Top ();
      if (var)
      {
        csVector4 v;
        var->GetValue (v);
        glLightfv (GL_LIGHT0+l, GL_POSITION, (float*)&v);
      }
      else
      {
        csVector4 v (0);
        glLightfv (GL_LIGHT0+l, GL_POSITION, (float*)&v);
      }

      var = lights[i].diffuseVarRef;
      if (!var && lights[i].diffusevar < (csStringID)stacks.Length ()
          && stacks[lights[i].diffusevar].Length () > 0)
        var = stacks[lights[i].diffusevar].Top ();
      if (var)
      {
        csVector4 v;
        var->GetValue (v);
        glLightfv (GL_LIGHT0+l, GL_DIFFUSE, (float*)&v);
      }
      else
      {
        csVector4 v (0);
        glLightfv (GL_LIGHT0+l, GL_DIFFUSE, (float*)&v);
      }

      var = lights[i].specularVarRef;
      if (!var && lights[i].specularvar < (csStringID)stacks.Length ()
          && stacks[lights[i].diffusevar].Length () > 0)
        var = stacks[lights[i].diffusevar].Top ();
      if (var)
      {
        csVector4 v;
        var->GetValue (v);
        glLightfv (GL_LIGHT0+l, GL_SPECULAR, (float*)&v);
      }
      else
      {
        csVector4 v (0);
        glLightfv (GL_LIGHT0+l, GL_SPECULAR, (float*)&v);
      }

      var = lights[i].attenuationVarRef;
      if (!var && lights[i].attenuationvar < (csStringID)stacks.Length ()
          && stacks[lights[i].attenuationvar].Length () > 0)
        var = stacks[lights[i].attenuationvar].Top ();
      if (var)
      {
        csVector4 v;
        var->GetValue (v);
        glLightf (GL_LIGHT0+l, GL_CONSTANT_ATTENUATION, v.x);
        glLightf (GL_LIGHT0+l, GL_LINEAR_ATTENUATION, v.y);
        glLightf (GL_LIGHT0+l, GL_QUADRATIC_ATTENUATION, v.z);
      }
      else
      {
        csVector4 v (1, 0, 0, 0);
        glLightf (GL_LIGHT0+l, GL_CONSTANT_ATTENUATION, v.x);
        glLightf (GL_LIGHT0+l, GL_LINEAR_ATTENUATION, v.y);
        glLightf (GL_LIGHT0+l, GL_QUADRATIC_ATTENUATION, v.z);
      }
    }

    glPopMatrix ();

    csVector4 v (1);
    glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, (float*)&v);
    glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, (float*)&v);

    var = ambientVarRef;
    if (!var && ambientvar < (csStringID)stacks.Length ()
        && stacks[ambientvar].Length () > 0)
      var = stacks[ambientvar].Top ();
    if (var)
      var->GetValue (v);
    else
      v = csVector4 (0, 0, 0, 1);
    glLightModelfv (GL_LIGHT_MODEL_AMBIENT, (float*)&v);

    statecache->Enable_GL_LIGHTING ();
    glDisable (GL_COLOR_MATERIAL);
  }

  for (i=0; i<layers.Length (); i++)
  {
    statecache->SetActiveTU (i);
    if (layers[i].texgen == TEXGEN_REFLECT_CUBE)
    {
      //setup for environmental cubemapping
      glTexGeni (GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);
      glTexGeni (GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);
      glTexGeni (GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);

      statecache->Enable_GL_TEXTURE_GEN_S ();
      statecache->Enable_GL_TEXTURE_GEN_T ();
      statecache->Enable_GL_TEXTURE_GEN_R ();

      const csReversibleTransform *t = mesh->camera_transform;
      const csMatrix3 &orientation = t->GetO2T();

      float mAutoTextureMatrix[16];
      // Transpose 3x3 in order to invert matrix (rotation)
      // Note that we need to invert the Z _before_ the rotation
      // No idea why we have to invert the Z at all, but reflection
      // is wrong without it
      mAutoTextureMatrix[0] = orientation.m11; 
      mAutoTextureMatrix[1] = orientation.m12; 
      mAutoTextureMatrix[2] = orientation.m13;

      mAutoTextureMatrix[4] = orientation.m21;
      mAutoTextureMatrix[5] = orientation.m22;
      mAutoTextureMatrix[6] = orientation.m23;

      mAutoTextureMatrix[8] = orientation.m31; 
      mAutoTextureMatrix[9] = orientation.m32; 
      mAutoTextureMatrix[10] = orientation.m33;

      mAutoTextureMatrix[3] =
        mAutoTextureMatrix[7] = mAutoTextureMatrix[11] = 0.0f;
      mAutoTextureMatrix[12] =
        mAutoTextureMatrix[13] = mAutoTextureMatrix[14] = 0.0f;
      mAutoTextureMatrix[15] = 1.0f;  

      statecache->SetMatrixMode (GL_TEXTURE);
      glLoadMatrixf (mAutoTextureMatrix);
    }
	else
    if (layers[i].texgen == TEXGEN_REFLECT_SPHERE)
    {
      //setup for environmental spheremapping
      glTexGeni (GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
      glTexGeni (GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

      statecache->Enable_GL_TEXTURE_GEN_S ();
      statecache->Enable_GL_TEXTURE_GEN_T ();

	  //TODO 
	  // need a correct texture matrix

	  /*
      const csReversibleTransform *t = mesh->camera_transform;
      const csMatrix3 &orientation = t->GetO2T();

      float mAutoTextureMatrix[16];

	  mAutoTextureMatrix[0] = orientation.m11; 
      mAutoTextureMatrix[1] = orientation.m12; 
      mAutoTextureMatrix[2] = orientation.m13;

      mAutoTextureMatrix[4] = orientation.m21;
      mAutoTextureMatrix[5] = orientation.m22;
      mAutoTextureMatrix[6] = orientation.m23;

      mAutoTextureMatrix[8] = orientation.m31; 
      mAutoTextureMatrix[9] = orientation.m32; 
      mAutoTextureMatrix[10] = orientation.m33;

      mAutoTextureMatrix[3] =
        mAutoTextureMatrix[7] = mAutoTextureMatrix[11] = 0.0f;
      mAutoTextureMatrix[12] =
        mAutoTextureMatrix[13] = mAutoTextureMatrix[14] = 0.0f;
      mAutoTextureMatrix[15] = 1.0f;  

      statecache->SetMatrixMode (GL_TEXTURE);
      glLoadMatrixf (mAutoTextureMatrix);
	  */
    }


    if (layers[i].texMatrixOps.Length() > 0)
    {
      statecache->SetMatrixMode (GL_TEXTURE);

      for (size_t j = 0; j < layers[i].texMatrixOps.Length(); j++)
      {
	TexMatrixOp& op = layers[i].texMatrixOps[j];

	RetrieveParamValue (op.param, stacks);

	switch (op.type)
	{
	  case TexMatrixScale:
	    {
	      glScalef (op.param.vectorValue.x, op.param.vectorValue.y, 
		op.param.vectorValue.z);
	    }
	    break;
	  case TexMatrixRotate:
	    {
	      glRotatef (op.param.vectorValue.x, 1.0f, 0.0f, 0.0f);
	      glRotatef (op.param.vectorValue.y, 0.0f, 1.0f, 0.0f);
	      glRotatef (op.param.vectorValue.z, 0.0f, 0.0f, 1.0f);
	    }
	    break;
	  case TexMatrixTranslate:
	    {
	      glTranslatef (op.param.vectorValue.x, op.param.vectorValue.y, 
		op.param.vectorValue.z);
	    }
	    break;
	  case TexMatrixMatrix:
	    {
	      const csMatrix3& m = op.param.matrixValue;

	      float matrix[16];
	      matrix[0] = m.m11; 
	      matrix[1] = m.m21; 
	      matrix[2] = m.m31;

	      matrix[4] = m.m12;
	      matrix[5] = m.m22;
	      matrix[6] = m.m32;

	      matrix[8] = m.m13; 
	      matrix[9] = m.m23; 
	      matrix[10] = m.m33;

	      matrix[3] = matrix[7] = matrix[11] = 0.0f;
	      matrix[12] = matrix[13] = matrix[14] = 0.0f;
	      matrix[15] = 1.0f;  

	      glMultMatrixf (matrix);
	    }
	    break;
	}
      }
    }


    if (layers[i].constcolor.valid)
    {
      RetrieveParamValue (layers[i].constcolor, stacks);
      glTexEnvfv (GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, 
	&layers[i].constcolor.vectorValue.x);
    }
  }

  statecache->SetActiveTU (0);

  var = primcolVarRef;
  if (!var && primcolvar != csInvalidStringID &&
      primcolvar < (csStringID)stacks.Length () && 
      stacks[primcolvar].Length () > 0)
    var = stacks[primcolvar].Top ();

  if (var)
  {
    csVector4 col;
    var->GetValue (col);
    glColor4f (col.x, col.y, col.z, col.w);
  }
}

void csGLShaderFVP::ResetState ()
{
  size_t i;
  if (do_lighting)
  {
    for (i = 0; i < lights.Length(); ++i)
      glDisable (GL_LIGHT0+lights[i].lightnum);

    statecache->Disable_GL_LIGHTING ();
  }

  for (i=0; i<layers.Length (); i++)
  {
    statecache->SetActiveTU (i);
    if ((layers[i].texgen != TEXGEN_NONE) ||
      (layers[i].texMatrixOps.Length() > 0))
    {
      statecache->Disable_GL_TEXTURE_GEN_S ();
      statecache->Disable_GL_TEXTURE_GEN_T ();
      statecache->Disable_GL_TEXTURE_GEN_R ();

      statecache->SetMatrixMode (GL_TEXTURE);
      glLoadIdentity ();
    }
  }
  statecache->SetActiveTU (0);
}

bool csGLShaderFVP::ParseTexMatrixOp (iDocumentNode* node, 
				      TexMatrixOp& op, bool matrix)
{
  const char* type = node->GetAttributeValue ("type");
  if (type == 0)
  {
    synsrv->Report ("crystalspace.graphics3d.shader.glfixed",
      CS_REPORTER_SEVERITY_WARNING,
      node,
      "No 'type' attribute");
    return false;
  }
  if (!ParseProgramParam (node, op.param,
    matrix ? ParamMatrix : ParamFloat | ParamVector2 | ParamVector3 |
    ParamVector4))
    return false;

  return true;
}

bool csGLShaderFVP::ParseTexMatrix (iDocumentNode* node, 
				    csArray<TexMatrixOp>& matrixOps)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while(it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next();
    if(child->GetType() != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = tokens.Request (value);
    switch(id)
    {
      case XMLTOKEN_SCALE:
	{
	  TexMatrixOp newOp (1.0f);
	  newOp.type = TexMatrixScale;
	  if (!ParseTexMatrixOp (child, newOp))
	    return false;
	  matrixOps.Push (newOp);
	}
	break;
      case XMLTOKEN_ROTATE:
	{
	  TexMatrixOp newOp (0.0f);
	  newOp.type = TexMatrixRotate;
	  if (!ParseTexMatrixOp (child, newOp))
	    return false;
	  matrixOps.Push (newOp);
	}
	break;
      case XMLTOKEN_TRANSLATE:
	{
	  TexMatrixOp newOp (0.0f);
	  newOp.type = TexMatrixTranslate;
	  if (!ParseTexMatrixOp (child, newOp))
	    return false;
	  matrixOps.Push (newOp);
	}
	break;
      case XMLTOKEN_MATRIX:
	{
	  TexMatrixOp newOp (0.0f);
	  newOp.type = TexMatrixMatrix;
	  if (!ParseTexMatrixOp (child, newOp, true))
	    return false;
	  matrixOps.Push (newOp);
	}
	break;
      default:
	{
	  synsrv->ReportBadToken (child);
	  return false;
	}
	break;
    }
  }
  return true;
}

bool csGLShaderFVP::Load(iDocumentNode* program)
{
  if (!program)
    return false;

  do_lighting = false;
  ambientvar = csInvalidStringID;
  primcolvar = csInvalidStringID;

  csRef<iShaderManager> shadermgr = CS_QUERY_REGISTRY(
  	objectReg, iShaderManager);

  csRef<iDocumentNode> variablesnode = program->GetNode("fixedvp");
  if(variablesnode)
  {
    csRef<iDocumentNodeIterator> it = variablesnode->GetNodes ();
    while(it->HasNext())
    {
      csRef<iDocumentNode> child = it->Next();
      if(child->GetType() != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = tokens.Request (value);
      switch(id)
      {
        case XMLTOKEN_LIGHT:
          {
            do_lighting = true;
            lights.Push (lightingentry ());
            int i = lights.Length ()-1;

            const char* str;

            lights[i].lightnum = child->GetAttributeValueAsInt("num");

            if ((str = child->GetAttributeValue("position")))
              lights[i].positionvar = strings->Request (str);
            else
            {
              char buf[64];
              sprintf (buf, "STANDARD_LIGHT_%d_POSITION_WORLD", lights[i].lightnum);
              lights[i].positionvar = strings->Request (buf);
            }

            if ((str = child->GetAttributeValue("diffuse")))
              lights[i].diffusevar = strings->Request (str);
            else
            {
              char buf[40];
              sprintf (buf, "STANDARD_LIGHT_%d_DIFFUSE", lights[i].lightnum);
              lights[i].diffusevar = strings->Request (buf);
            }

            if ((str = child->GetAttributeValue("specular")))
              lights[i].specularvar = strings->Request (str);
            else
            {
              char buf[40];
              sprintf (buf, "STANDARD_LIGHT_%d_SPECULAR", lights[i].lightnum);
              lights[i].specularvar = strings->Request (buf);
            }

            if ((str = child->GetAttributeValue("attenuation")))
              lights[i].attenuationvar = strings->Request (str);
            else
            {
              char buf[40];
              sprintf (buf, "STANDARD_LIGHT_%d_ATTENUATION",
	      	lights[i].lightnum);
              lights[i].attenuationvar = strings->Request (buf);
            }
          }
          break;
        case XMLTOKEN_VERTEXCOLOR:
          {
	    // @@@ Realize as var mapping?
            const char* str;
            if ((str = child->GetContentsValue ()) != 0)
              primcolvar = strings->Request (str);
          }
          break;
        case XMLTOKEN_CONSTANTCOLOR:
          {
	    // @@@ Realize as var mapping?
            int layer = child->GetAttributeValueAsInt ("layer");
            if (layers.Length ()<= (size_t)layer)
              layers.SetLength (layer+1);
	    if (!ParseProgramParam (child, layers[layer].constcolor, ParamFloat | 
	      ParamVector3 | ParamVector4))
	      return false;
          }
          break;
        case XMLTOKEN_AMBIENT:
          {
            const char* str;
            if ((str = child->GetAttributeValue("color")))
              ambientvar = strings->Request (str);
            else
              ambientvar = strings->Request ("STANDARD_LIGHT_AMBIENT");
          
            do_lighting = true;
          }
          break;
        case XMLTOKEN_TEXGEN:
          {
            const char* str;
            if ((str = child->GetAttributeValue ("type")))
            {
              if (!strcasecmp(str, "reflection"))
              {
                if ((str = child->GetAttributeValue ("mapping")))
                {
                  if (!strcasecmp(str, "cube"))
                  {
                    int layer = child->GetAttributeValueAsInt ("layer");
                    if (layers.Length () <= (size_t)layer)
                      layers.SetLength (layer+1);
                    layers[layer].texgen = TEXGEN_REFLECT_CUBE;
                  }
				  else
                  if (!strcasecmp(str, "sphere"))
                  {
                    int layer = child->GetAttributeValueAsInt ("layer");
                    if (layers.Length () <= (size_t)layer)
                      layers.SetLength (layer+1);
                    layers[layer].texgen = TEXGEN_REFLECT_SPHERE;
                  }
                }
              }
            }
          }
          break;
	case XMLTOKEN_TEXMATRIX:
	  {
	    const char* dest = child->GetAttributeValue ("destination");
	    if (!dest)
	    {
	      synsrv->Report ("crystalspace.graphics3d.shader.glfixed",
		CS_REPORTER_SEVERITY_WARNING,
		child,
		"No 'destination' attribute");
	      return false;
	    }
	    int unit = 0;
	    if (sscanf (dest, "unit %d", &unit) != 1)
	    {
	      synsrv->Report ("crystalspace.graphics3d.shader.glfixed",
		CS_REPORTER_SEVERITY_WARNING,
		child,
		"Unknown destination '%s'", dest);
	      return false;
	    }
	    /*
	     @@@ Would be nice to somehow utilize the 'Resolve TU' function 
	         of the FP.
	     */
            if (layers.Length () <= (size_t)unit)
              layers.SetLength (unit + 1);
	    if (!ParseTexMatrix (child, layers[unit].texMatrixOps))
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
  }

  return true;
}

bool csGLShaderFVP::Compile(csArray<iShaderVariableContext*> &staticContexts)
{
  shaderPlug->Open ();

  //get a statecache
  csRef<iGraphics2D> g2d = CS_QUERY_REGISTRY (objectReg, iGraphics2D);
  g2d->PerformExtension ("getstatecache", &statecache);

  size_t i, j;

  for (i=0; i<layers.Length (); i++)
    if ((layers[i].texgen == TEXGEN_REFLECT_CUBE) &&
      !shaderPlug->ext->CS_GL_ARB_texture_cube_map)
      return false;

  for (j=0; j<staticContexts.Length(); j++)
  {
    ambientVarRef = staticContexts[j]->GetVariable (ambientvar);
    if (ambientVarRef) break;
  }

  for (i = 0; i < layers.Length (); i++)
  {
    ResolveParamStatic (layers[i].constcolor, staticContexts);
  }

  for (i = 0; i < lights.Length (); i++)
  {
    lights[i].positionVarRef = 0;
    lights[i].diffuseVarRef = 0;
    lights[i].specularVarRef = 0;
    lights[i].attenuationVarRef = 0;
    for (j=0; j<staticContexts.Length(); j++)
    {
      if (!lights[i].positionVarRef)
        lights[i].positionVarRef = 
          staticContexts[j]->GetVariable (lights[i].positionvar);
      if (!lights[i].diffuseVarRef)
        lights[i].diffuseVarRef = 
          staticContexts[j]->GetVariable (lights[i].diffusevar);
      if (!lights[i].specularVarRef)
        lights[i].specularVarRef = 
          staticContexts[j]->GetVariable (lights[i].specularvar);
      if (!lights[i].attenuationVarRef)
        lights[i].attenuationVarRef = 
          staticContexts[j]->GetVariable (lights[i].attenuationvar);
    }
  }

  primcolVarRef = 0;
  if (primcolvar != csInvalidStringID)
    for (j=0; j<staticContexts.Length(); j++)
    {
      if (!primcolVarRef)
        primcolVarRef = 
          staticContexts[j]->GetVariable (primcolvar);
    }

  return true;
}
