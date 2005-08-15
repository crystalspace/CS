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

#include "csgeom/vector3.h"
#include "csplugincommon/opengl/glextmanager.h"
#include "csplugincommon/opengl/glstates.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"

#include "imap/services.h"
#include "iutil/document.h"
#include "iutil/string.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/rendermesh.h"
#include "ivideo/shader/shader.h"

#include "glshader_fvp.h"
#include "glshader_fixed.h"

CS_LEAKGUARD_IMPLEMENT (csGLShaderFVP);

csGLShaderFVP::csGLShaderFVP (csGLShader_FIXED* shaderPlug) :
  csShaderProgram (shaderPlug->object_reg)
{
  validProgram = true;
  csGLShaderFVP::shaderPlug = shaderPlug;

  InitTokenTable (tokens);
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
                                csRenderMeshModes& modes,
				const csShaderVarStack &stacks)
{
  size_t i;

  csRef<csShaderVariable> var;

  if (do_lighting)
  {
    statecache->SetMatrixMode (GL_MODELVIEW);
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

      var = csGetShaderVariableFromStack (stacks, lights[i].positionvar);
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

      var = csGetShaderVariableFromStack (stacks, lights[i].diffusevar);
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

      var = csGetShaderVariableFromStack (stacks, lights[i].specularvar);
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

      var = csGetShaderVariableFromStack (stacks, lights[i].attenuationvar);
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


    var = csGetShaderVariableFromStack (stacks, ambientvar);
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
    statecache->SetActiveTU ((int)i);
    statecache->ActivateTU ();
    if (layers[i].texgen == TEXGEN_REFLECT_CUBE)
    {

      csShaderVariable* sv = csGetShaderVariableFromStack (stacks, string_world2camera);
      if (!sv) continue;

      //setup for environmental cubemapping
      glTexGeni (GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);
      glTexGeni (GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);
      glTexGeni (GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);

      statecache->Enable_GL_TEXTURE_GEN_S ();
      statecache->Enable_GL_TEXTURE_GEN_T ();
      statecache->Enable_GL_TEXTURE_GEN_R ();
      
      csReversibleTransform t;
      sv->GetValue (t);
      const csMatrix3 &orientation = t.GetT2O();

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
    else if (layers[i].texgen == TEXGEN_REFLECT_SPHERE)
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
    else if (layers[i].texgen == TEXGEN_FOG)
    {
      csRef<csShaderVariable> planeVar;
      csRef<csShaderVariable> densityVar;

      planeVar = csGetShaderVariableFromStack (stacks, layers[i].fogplane);
      densityVar = csGetShaderVariableFromStack (stacks, layers[i].fogdensity);

      if (planeVar && densityVar)
      {
        // Initialize some stuff
        csVector4 fogplane;
        float density;
        planeVar->GetValue (fogplane);
        densityVar->GetValue (density);
        statecache->SetMatrixMode (GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity ();
        glTexGeni (GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
        glTexGeni (GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
        statecache->Enable_GL_TEXTURE_GEN_S ();
        statecache->Enable_GL_TEXTURE_GEN_T ();

        // This is where the magic starts
        // If you understand this initially (or at all) I salute you

        // First we intialize our texgen planes with the case where
        // viewplane & fogplane are equal, i.e we want U==Z and
        // V==1. (The fog lookup texture pretty much does "min(U, V)")
        // We also offset the fogplane a bit, to avoid ugly clipping 
        // when fogged portals cuts the near plane.
        GLfloat coeff1[] = {0, 0, density, -0.1*density};
        GLfloat coeff2[] = {0, 0, 0, 1};
        
        // Then we see if we're looking away from the fogplane
        // If we are, we can just go with the viewplane==fogplane fog,
        // but if we aren't we're in trouble...
        if (fogplane.z>0) 
        {
          // Okay, so we weren't. Tough luck. Time to try to do the
          // best out of things. We start by calculating a blend factor
          // to blend to viewplane==fogplane fog when we approach the
          // fogplane, to avoid some ugly skipping. The "pow" is there
          // tweak the curve to get a bit smoother blend.
          float blend = 1.0f - pow (1.0f + fogplane.w*0.2f, 5);
          blend = MIN (MAX (blend, 0), 1);

          // Okay, here's the serious magic. Basically what we want to do
          // is to calculate the distance from the vertex to the viewplane
          // along Z,and the distance from the vertex to the fogplane, 
          // and use the smallest.
          
          /*
                             fogplane
                ||||||||||||/ 
                ||||fog||||/   ^ 
                ||||||||||/    |z+
                ---------/-----O------------ viewplane
                        /
                       /
          */

          // The distance to the viewplane is just the Z value, so that's easy
          // The distance to the fogplane (along Z) is the normal of the 
          // fogplane <dot> the vertex (i.e. the perpendicular distance), 
          // divided by the Z component of the fogplane normal. Finally
          // it's scaled by "density" to get that into the equation too.
          // The actual MIN part is done by the "standardtex fog" texture.

          // This one is already the viewplane distance, i.e. Z.
          // We throw in a little offset, to cope with the
          // fact that during the blend we haven't actually reached
          // the fogplane yet. Nothing scientific, but does the job.
          const float dist =  fogplane.w*density;
          coeff1[3] = (MIN (MAX (dist, -1.0f), 0) - 0.1f*density);

          // This one is the fogplane distance, i.e. 
          // vertex<dot>fogplane/fogplane.z. We blend this to just a
          // fixed 1 as we approach the fogplane.
          coeff2[0] = fogplane.x*density/fogplane.z;
          coeff2[1] = fogplane.y*density/fogplane.z;
          coeff2[2] = density;
          coeff2[3] = (1-blend)+(fogplane.w/fogplane.z-0.1)*density;
        }

        glTexGenfv (GL_S, GL_EYE_PLANE, (GLfloat*)&coeff1);
        glTexGenfv (GL_T, GL_EYE_PLANE, (GLfloat*)&coeff2);
        glPopMatrix();
      }
    }

    if (layers[i].texMatrixOps.Length() > 0)
    {
      statecache->SetMatrixMode (GL_TEXTURE);

      for (size_t j = 0; j < layers[i].texMatrixOps.Length(); j++)
      {
	TexMatrixOp& op = layers[i].texMatrixOps[j];

        csRef<csShaderVariable> var;

        var = csGetShaderVariableFromStack (stacks, op.param.name);
        if (!var.IsValid ())
          var = op.param.var;

        // If var is null now we have no const nor any passed value, ignore it
        if (!var.IsValid ())
          continue;

        csVector4 vectorVal;
        var->GetValue (vectorVal);

	switch (op.type)
	{
	  case TexMatrixScale:
	    {
	      glScalef (vectorVal.x, vectorVal.y, vectorVal.z);
	    }
	    break;
	  case TexMatrixRotate:
	    {
	      glRotatef (vectorVal.x, 1.0f, 0.0f, 0.0f);
	      glRotatef (vectorVal.y, 0.0f, 1.0f, 0.0f);
	      glRotatef (vectorVal.z, 0.0f, 0.0f, 1.0f);
	    }
	    break;
	  case TexMatrixTranslate:
	    {
	      glTranslatef (vectorVal.x, vectorVal.y, vectorVal.z);
	    }
	    break;
	  case TexMatrixMatrix:
	    {
	      //const csMatrix3& m = op.param.matrixValue;
              csMatrix3 m;
              var->GetValue (m);

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
      csRef<csShaderVariable> var;

      var = csGetShaderVariableFromStack (stacks, layers[i].constcolor.name);
      if (!var.IsValid ())
        var = layers[i].constcolor.var;

      if (!var.IsValid ())
        continue;

      csVector4 vectorVal;
      var->GetValue (vectorVal);

      glTexEnvfv (GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, 
	&vectorVal.x);
    }
  }

  statecache->SetActiveTU (0);
  statecache->ActivateTU ();

  var = csGetShaderVariableFromStack (stacks, primcolvar);
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
    statecache->SetActiveTU ((int)i);
    statecache->ActivateTU ();
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
  statecache->ActivateTU ();
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
    ParamVector4 | ParamShaderExp))
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

static int ParseLayerParam (iDocumentNode* node, iShaderTUResolver* tuResolve)
{
  const char* layerName = node->GetAttributeValue ("layer");
  if (layerName == 0) return -1;

  int layer = 
    tuResolve ? tuResolve->ResolveTextureBinding (layerName) : -1;
  if (layer < 0) layer = node->GetAttributeValueAsInt ("layer");
  return layer;
}

bool csGLShaderFVP::Load(iShaderTUResolver* tuResolve, iDocumentNode* program)
{
  if (!program)
    return false;

  do_lighting = false;
  ambientvar = csInvalidStringID;
  primcolvar = csInvalidStringID;
  string_world2camera = strings->Request ("world2camera transform");

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
            size_t i = lights.Length ()-1;

            const char* str;

            lights[i].lightnum = child->GetAttributeValueAsInt("num");

            if ((str = child->GetAttributeValue("position")))
              lights[i].positionvar = strings->Request (str);
            else
            {
              csString buf;
              buf.Format ("light %d position world", 
		lights[i].lightnum);
              lights[i].positionvar = strings->Request (buf);
            }

            if ((str = child->GetAttributeValue("diffuse")))
              lights[i].diffusevar = strings->Request (str);
            else
            {
              csString buf;
              buf.Format ("light %d diffuse", lights[i].lightnum);
              lights[i].diffusevar = strings->Request (buf);
            }

            if ((str = child->GetAttributeValue("specular")))
              lights[i].specularvar = strings->Request (str);
            else
            {
              csString buf;
              buf.Format ("light %d specular", lights[i].lightnum);
              lights[i].specularvar = strings->Request (buf);
            }

            if ((str = child->GetAttributeValue("attenuation")))
              lights[i].attenuationvar = strings->Request (str);
            else
            {
              csString buf;
              buf.Format ("light %d attenuation",
	      	lights[i].lightnum);
              lights[i].attenuationvar = strings->Request (buf);
            }
          }
          break;
        case XMLTOKEN_VERTEXCOLOR:
          {
            const char* str;
            if ((str = child->GetContentsValue ()) != 0)
              primcolvar = strings->Request (str);
          }
          break;
        case XMLTOKEN_CONSTANTCOLOR:
          {
            int layer = ParseLayerParam (child, tuResolve);
	    if (layer < 0)
	    {
	      synsrv->ReportError ("crystalspace.graphics3d.shader.fixed.vp",
		variablesnode, "'layer' attribute invalid");
	      return false;
	    }
            if (layers.Length ()<= (size_t)layer)
              layers.SetLength (layer+1);
	    if (!ParseProgramParam (child, layers[layer].constcolor, ParamFloat | 
	      ParamVector3 | ParamVector4 | ParamShaderExp))
	      return false;
          }
          break;
        case XMLTOKEN_AMBIENT:
          {
            const char* str;
            if ((str = child->GetAttributeValue("color")))
              ambientvar = strings->Request (str);
            else
              ambientvar = strings->Request ("light ambient");
          
            do_lighting = true;
          }
          break;
        case XMLTOKEN_TEXGEN:
          {
            int layer = ParseLayerParam (child, tuResolve);
	    if (layer < 0)
	    {
	      synsrv->ReportError ("crystalspace.graphics3d.shader.fixed.vp",
		variablesnode, "'layer' attribute invalid");
	      return false;
	    }
            if (layers.Length () <= (size_t)layer)
              layers.SetLength (layer+1);
            const char* str;
            if ((str = child->GetAttributeValue ("type")))
            {
              if (!strcasecmp(str, "reflection"))
              {
                if ((str = child->GetAttributeValue ("mapping")))
                {
                  if (!strcasecmp(str, "cube"))
                  {
                    layers[layer].texgen = TEXGEN_REFLECT_CUBE;
                  }
                  else if (!strcasecmp(str, "sphere"))
                  {
                    layers[layer].texgen = TEXGEN_REFLECT_SPHERE;
                  }
		  else
		  {
		    synsrv->ReportError ("crystalspace.graphics3d.shader.fixed.vp",
		      variablesnode, "invalid mapping '%s'", str);
		    return false;
		  }
                }
              }
              else if (!strcasecmp(str, "fog"))
              {
                layers[layer].texgen = TEXGEN_FOG;

                if ((str = child->GetAttributeValue("plane")))
                  layers[layer].fogplane = strings->Request (str);
                else
                  layers[layer].fogplane = strings->Request ("fogplane");

                if ((str = child->GetAttributeValue("density")))
                  layers[layer].fogdensity = strings->Request (str);
                else
                  layers[layer].fogdensity = strings->Request ("fog density");
              }
	      else
	      {
		synsrv->ReportError ("crystalspace.graphics3d.shader.fixed.vp",
		  variablesnode, "invalid type '%s'", str);
		return false;
	      }
            }
          }
          break;
	case XMLTOKEN_TEXMATRIX:
	  {
            int layer = ParseLayerParam (child, tuResolve);
	    if (layer < 0)
	    {
	      synsrv->ReportError ("crystalspace.graphics3d.shader.fixed.vp",
		variablesnode, "'layer' attribute invalid");
	      return false;
	    }
            if (layers.Length () <= (size_t)layer)
              layers.SetLength (layer+1);
	    if (!ParseTexMatrix (child, layers[layer].texMatrixOps))
	      return false;
	  }
	  break;
        default:
	  {
	    switch (commonTokens.Request (value))
	    {
	      case XMLTOKEN_PROGRAM:
	      case XMLTOKEN_VARIABLEMAP:
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
  else
  {
    synsrv->ReportError ("crystalspace.graphics3d.shader.fixed.vp",
      variablesnode, "<fixedvp> node missing");
  }

  return true;
}

bool csGLShaderFVP::Compile()
{
  shaderPlug->Open ();

  g3d = CS_QUERY_REGISTRY (objectReg, iGraphics3D);

  //get a statecache
  csRef<iGraphics2D> g2d = CS_QUERY_REGISTRY (objectReg, iGraphics2D);
  g2d->PerformExtension ("getstatecache", &statecache);

  size_t i;

  for (i=0; i<layers.Length (); i++)
    if ((layers[i].texgen == TEXGEN_REFLECT_CUBE) &&
      !shaderPlug->ext->CS_GL_ARB_texture_cube_map)
      return false;

  return true;
}
