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

#include "video/canvas/openglcommon/glextmanager.h"

#include "glshader_fvp.h"
#include "glshader_fixed.h"

SCF_IMPLEMENT_IBASE(csGLShaderFVP)
  SCF_IMPLEMENTS_INTERFACE(iShaderProgram)
SCF_IMPLEMENT_IBASE_END

csGLShaderFVP::csGLShaderFVP (csGLShader_FIXED* shaderPlug)
{
  SCF_CONSTRUCT_IBASE (0);
  validProgram = true;
  csGLShaderFVP::shaderPlug = shaderPlug;
  csGLShaderFVP::object_reg = shaderPlug->object_reg;

  environment = ENVIRON_NONE;
  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
}

csGLShaderFVP::~csGLShaderFVP ()
{
  SCF_DESTRUCT_IBASE ();
}

void csGLShaderFVP::Activate ()
{
}

void csGLShaderFVP::Deactivate()
{
}

void csGLShaderFVP::SetupState (
  csRenderMesh *mesh, csArray<iShaderVariableContext*> &dynamicDomains)
{
  int i;

  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.renderer.stringset", iStringSet);

  if (dynamicVars.Length() > 0)
  {
    dynamicVars.PrepareFill ();
    for(i=0;i<dynamicDomains.Length();i++)
    {
      dynamicDomains[i]->FillVariableList (&dynamicVars);
    }
  }

  if (do_lighting)
  {
    glMatrixMode (GL_MODELVIEW_MATRIX);
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

      if (lights[i].positionVarRef)
      {
        csVector4 v;
        lights[i].positionVarRef->GetValue (v);
        glLightfv (GL_LIGHT0+l, GL_POSITION, (float*)&v);
      }
      else
      {
        csVector4 v (0);
        glLightfv (GL_LIGHT0+l, GL_POSITION, (float*)&v);
      }

      if (lights[i].diffuseVarRef)
      {
        csVector4 v;
        lights[i].diffuseVarRef->GetValue (v);
        glLightfv (GL_LIGHT0+l, GL_DIFFUSE, (float*)&v);
      }
      else
      {
        csVector4 v (0);
        glLightfv (GL_LIGHT0+l, GL_DIFFUSE, (float*)&v);
      }

      if (lights[i].specularVarRef)
      {
        csVector4 v;
        lights[i].specularVarRef->GetValue (v);
        glLightfv (GL_LIGHT0+l, GL_SPECULAR, (float*)&v);
      }
      else
      {
        csVector4 v (0);
        glLightfv (GL_LIGHT0+l, GL_SPECULAR, (float*)&v);
      }

      if (lights[i].attenuationVarRef)
      {
        csVector4 v;
        lights[i].attenuationVarRef->GetValue (v);
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

    if (ambientVarRef)
      ambientVarRef->GetValue (v);
    else
      v = csVector4 (0, 0, 0, 1);
    glLightModelfv (GL_LIGHT_MODEL_AMBIENT, (float*)&v);

    glEnable (GL_LIGHTING);
    glDisable (GL_COLOR_MATERIAL);
  }

  if (environment == ENVIRON_REFLECT_CUBE)
  {
    //setup for environmental cubemapping
    glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP_ARB);
    glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP_ARB);
    glTexGeni(GL_R,GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP_ARB);

    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);

    csReversibleTransform *t = &mesh->object2camera;
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

    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf(mAutoTextureMatrix);
  }
}

void csGLShaderFVP::ResetState ()
{
  int i;
  if (do_lighting)
  {
    for (i = 0; i < lights.Length(); ++i)
      glDisable (GL_LIGHT0+lights[i].lightnum);

    glDisable (GL_LIGHTING);
  }

  if (environment == ENVIRON_REFLECT_CUBE)
  {
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);

    glMatrixMode (GL_TEXTURE);
    glLoadIdentity ();
  }
}

void csGLShaderFVP::BuildTokenHash()
{
  xmltokens.Register("fixedvp",XMLTOKEN_FIXEDVP);
  xmltokens.Register("declare",XMLTOKEN_DECLARE);
  xmltokens.Register("light", XMLTOKEN_LIGHT);
  xmltokens.Register("ambient", XMLTOKEN_AMBIENT);
  xmltokens.Register("environment", XMLTOKEN_ENVIRONMENT);
  xmltokens.Register("reflect", XMLTOKEN_REFLECT);

  xmltokens.Register("integer", 100+csShaderVariable::INT);
  xmltokens.Register("float", 100+csShaderVariable::FLOAT);
  xmltokens.Register("vector3", 100+csShaderVariable::VECTOR3);
}

bool csGLShaderFVP::Load(iDocumentNode* program)
{
  if (!program)
    return false;

  do_lighting = false;
  ambientvar = csInvalidStringID;

  BuildTokenHash();

  csRef<iShaderManager> shadermgr = CS_QUERY_REGISTRY(
  	object_reg, iShaderManager);
  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
	object_reg, "crystalspace.renderer.stringset", iStringSet);

  csRef<iDocumentNode> variablesnode = program->GetNode("fixedvp");
  if(variablesnode)
  {
    csRef<iDocumentNodeIterator> it = variablesnode->GetNodes ();
    while(it->HasNext())
    {
      csRef<iDocumentNode> child = it->Next();
      if(child->GetType() != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
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
              char buf[40];
              sprintf (buf, "STANDARD_LIGHT_%d_POSITION", lights[i].lightnum);
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
        case XMLTOKEN_ENVIRONMENT:
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
                    environment = ENVIRON_REFLECT_CUBE;
                  }
                }
              }
            }
          }
          break;
        default:
          break;
        //return false;
      }
    }
  }

  return true;
}

bool csGLShaderFVP::Compile(csArray<iShaderVariableContext*> &staticdomains)
{
  shaderPlug->Open ();

  //go through the lights and get direct refs (if we can)
  dynamicVars.Empty ();
  int i, j;

  if ((environment == ENVIRON_REFLECT_CUBE) &&
    !shaderPlug->ext->CS_GL_ARB_texture_cube_map)
    return false;

  ambientVarRef = svContextHelper.GetVariable (ambientvar);
  if (!ambientVarRef)
  {
    for (j=0;j<staticdomains.Length();j++)
    {
      ambientVarRef = staticdomains[j]->GetVariable (ambientvar);
      if (ambientVarRef) break;
    }
  }
  if (!ambientVarRef)
    dynamicVars.InsertSorted (csShaderVariableProxy (ambientvar, 0,
    &ambientVarRef));

  for (i = 0; i < lights.Length (); i++)
  {
    lightingentry &ent = lights.Get (i);
    //query, it turn current and all levels above
    ent.positionVarRef = svContextHelper.GetVariable (ent.positionvar);
    if(!ent.positionVarRef)
    {
      for (j=0;j<staticdomains.Length();j++)
      {
        ent.positionVarRef = staticdomains[j]->GetVariable (ent.positionvar);
        if (ent.positionVarRef) break;
      }
    }
    if(!ent.positionVarRef)
      ent.dynVars.InsertSorted (csShaderVariableProxy(ent.positionvar, 0,
      &ent.positionVarRef));

    ent.diffuseVarRef = svContextHelper.GetVariable (ent.diffusevar);
    if(!ent.diffuseVarRef)
    {
      for (j=0;j<staticdomains.Length();j++)
      {
        ent.diffuseVarRef = staticdomains[j]->GetVariable (ent.diffusevar);
        if (ent.diffuseVarRef) break;
      }
    }
    if(!ent.diffuseVarRef)
      ent.dynVars.InsertSorted (csShaderVariableProxy(ent.diffusevar, 0,
      &ent.diffuseVarRef));

    ent.specularVarRef = svContextHelper.GetVariable (ent.specularvar);
    if(!ent.specularVarRef)
    {
      for (j=0;j<staticdomains.Length();j++)
      {
        ent.specularVarRef = staticdomains[j]->GetVariable (ent.specularvar);
        if (ent.specularVarRef) break;
      }
    }
    if(!ent.specularVarRef)
      ent.dynVars.InsertSorted (csShaderVariableProxy(ent.specularvar, 0,
      &ent.specularVarRef));

    ent.attenuationVarRef = svContextHelper.GetVariable (ent.attenuationvar);
    if(!ent.attenuationVarRef)
    {
      for (j=0;j<staticdomains.Length();j++)
      {
        ent.attenuationVarRef =
	  staticdomains[j]->GetVariable (ent.attenuationvar);
        if (ent.attenuationVarRef) break;
      }
    }
    if(!ent.attenuationVarRef)
      ent.dynVars.InsertSorted (csShaderVariableProxy(ent.attenuationvar, 0,
      &ent.attenuationVarRef));
  }

  return true;
}
