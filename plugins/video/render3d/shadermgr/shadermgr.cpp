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


#include "cssysdef.h"
#include "cstypes.h"
#include "csutil/scf.h"
#include "csutil/objreg.h"
#include "csgeom/vector3.h"
#include "csutil/hashmap.h"

#include "igeom/clip2d.h"
#include "iutil/vfs.h"

#include "ivideo/shader/shader.h"
#include "ivideo/render3d.h"
#include "shadermgr.h"


// Pluginstuff
CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE( csShaderManager )
  SCF_IMPLEMENTS_INTERFACE( iShaderManager )
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE( iComponent )
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE( csShaderManager::Component )
  SCF_IMPLEMENTS_INTERFACE( iComponent )
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY( csShaderManager )

SCF_IMPLEMENT_IBASE( csShader )
  SCF_IMPLEMENTS_INTERFACE( iShader )
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE( csShaderVariable )
  SCF_IMPLEMENTS_INTERFACE( iShaderVariable )
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE( csShaderTechnique )
  SCF_IMPLEMENTS_INTERFACE( iShaderTechnique )
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE( csShaderPass )
  SCF_IMPLEMENTS_INTERFACE( iShaderPass )
SCF_IMPLEMENT_IBASE_END

SCF_EXPORT_CLASS_TABLE( shadermgr )
  SCF_EXPORT_CLASS( csShaderManager, "crystalspace.video.shader.manager", "Shader manager" )
SCF_EXPORT_CLASS_TABLE_END

//=================== csShaderManager ================//

// General stuff
csShaderManager::csShaderManager(iBase* parent)
{
  SCF_CONSTRUCT_IBASE( parent );
  SCF_CONSTRUCT_EMBEDDED_IBASE( scfiComponent );

  // alloc variables-hash
  variables = new csHashMap();

  shaders = new csBasicVector();

  seqnumber = 0;
}

csShaderManager::~csShaderManager()
{
  //Clear variables
  csHashIterator cIter( variables);

  while(cIter.HasNext() )
  {
    iShaderVariable* i = (iShaderVariable*)cIter.Next();
    i->DecRef();
  }

  variables->DeleteAll();
  delete variables;

  //clear all shaders
  while(shaders->Length() > 0)
  {
    delete (iShader*)shaders->Pop();
  }
  delete shaders;
}

bool csShaderManager::Initialize(iObjectRegistry *objreg)
{
  objectreg = objreg;
  return true;
}

//Variable handling
bool csShaderManager::AddVariable(iShaderVariable* variable)
{
  variable->IncRef();
  variables->Put( csHashCompute(variable->GetName()), variable);
  return true;
}

csPtr<iShaderVariable> csShaderManager::CreateVariable(const char* name)
{
  csShaderVariable* myVar = new csShaderVariable();
  myVar->SetName(name);
  return (iShaderVariable*)myVar;
}

iShaderVariable* csShaderManager::GetVariable(const char* name)
{
  csHashIterator cIter(variables, csHashCompute(name));

  if( cIter.HasNext() )
  {
    return (iShaderVariable*)cIter.Next();
  }

  return NULL;
}

csBasicVector csShaderManager::GetAllVariableNames()
{
  csBasicVector vReturnValue;

  csHashIterator cIter (variables);
  while(cIter.HasNext())
  {
    vReturnValue.Push( (csSome) ((iShaderVariable*)cIter.Next())->GetName() );
  }

  return vReturnValue;
}

// Shader handling
csPtr<iShader> csShaderManager::CreateShader()
{
  char* name = new char[10];
  sprintf(name, "effect%2d", seqnumber);
  seqnumber++;

  csShader* cshader = new csShader(name);
  cshader->IncRef();

  shaders->Push(cshader);
  
  return (iShader*)cshader;
}

csPtr<iShader> csShaderManager::CreateShader(const char* filename)
{
  return NULL;
}

iShader* csShaderManager::GetShader(const char* name)
{
  int i;
  for( i = 0; i < shaders->Length(); ++i)
  {
    if( strcasecmp(((iShader*)shaders->Get(i))->GetName() ,name) == 0)
      return (iShader*)shaders->Get(i);
  }
  return NULL;
}

csPtr<iShaderProgram> csShaderManager::CreateShaderProgramFromFile(const char* filename, const char* type)
{
  csRef<iVFS> vfs = CS_QUERY_REGISTRY(objectreg, iVFS);

  csRef<iFile> shaderfile (vfs->Open(filename, VFS_FILE_READ));

  char* shadercontent = new char[shaderfile->GetSize()];
  shaderfile->Read(shadercontent, shaderfile->GetSize());

  csPtr<iShaderProgram> pm = CreateShaderProgramFromString(shadercontent, type);
  
  delete shadercontent;
  return pm;
}

csPtr<iShaderProgram> csShaderManager::CreateShaderProgramFromString(const char* string, const char* type)
{
  csRef<iRender3D> render3d = CS_QUERY_REGISTRY (objectreg, iRender3D);
  if( render3d )
  {
    csRef<iShaderRenderInterface> shi = SCF_QUERY_INTERFACE (render3d, iShaderRenderInterface);
    if( shi)
    {
      return shi->CreateShaderProgram(string, NULL, type);
    }
  }
  return NULL;
}

//===================== csShader ====================//
csShader::csShader()
{
  variables = new csHashMap();
}

csShader::csShader(const char* name)
{
  csShader();
  SetName(name);
}

csShader::~csShader()
{
  if(name)
    delete name;

  //Clear variables
  csHashIterator cIter( variables);

  while(cIter.HasNext() )
  {
    iShaderVariable* i = (iShaderVariable*)cIter.Next();
    i->DecRef();
  }

  variables->DeleteAll();
  delete variables;
}

bool csShader::IsValid()
{
  return false;
}

//Variable handling
bool csShader::AddVariable(iShaderVariable* variable)
{
  variable->IncRef();
  variables->Put( csHashCompute(variable->GetName()), variable);
  return true;
}

iShaderVariable* csShader::GetVariable(const char* name)
{
  csHashIterator cIter(variables, csHashCompute(name));

  if( cIter.HasNext() )
  {
    return (iShaderVariable*)cIter.Next();
  }

  return NULL;
}

csBasicVector csShader::GetAllVariableNames()
{
  csBasicVector vReturnValue;

  csHashIterator cIter (variables);
  while(cIter.HasNext())
  {
    vReturnValue.Push( (csSome) ((iShaderVariable*)cIter.Next())->GetName() );
  }

  return vReturnValue;
}

//technique-related
csPtr<iShaderTechnique> csShader::CreateTechnique()
{
  iShaderTechnique* mytech = new csShaderTechnique();
  mytech->IncRef();

  techniques->Push(mytech);
  return mytech;
}

iShaderTechnique* csShader::GetTechnique(int technique)
{
  if( technique >= techniques->Length()) return NULL;

  return (iShaderTechnique*)techniques->Get(technique);
}

iShaderTechnique* csShader::GetBestTechnique()
{
  int i;
  int maxpriority = 0;
  iShaderTechnique* tech = NULL;

  for (i = 0; i < techniques->Length(); ++i)
  {
    if( ((iShaderTechnique*)techniques->Get(i))->IsValid() &&
        ((iShaderTechnique*)techniques->Get(i))->GetPriority() > maxpriority)
    {
      tech = ((iShaderTechnique*)techniques->Get(i));
      maxpriority = tech->GetPriority();
    }
  }
  return tech;
}

void csShader::MapStream(int mapid, const char* streamname)
{
}


//================= csShaderTechnique ============//

csPtr<iShaderPass> csShaderTechnique::CreatePass()
{
  iShaderPass* mpass = new csShaderPass();
  mpass->IncRef();

  passes->Push(mpass);
  return mpass;
}

iShaderPass* csShaderTechnique::GetPass(int pass)
{
  if( pass >= passes->Length()) return NULL;

  return (iShaderPass*)passes->Get(pass);
}

void csShaderTechnique::MapStream(int mapped_id, const char* streamname)
{
}

bool csShaderTechnique::IsValid()
{
  return true;
}