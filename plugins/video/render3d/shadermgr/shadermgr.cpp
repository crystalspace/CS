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
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/strset.h"
#include "csutil/objreg.h"
#include "csgeom/vector3.h"
#include "csutil/hashmap.h"

#include "igeom/clip2d.h"
#include "iutil/vfs.h"
#include "iutil/eventq.h"
#include "iutil/virtclk.h"
#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"

#include "ivideo/shader/shader.h"
#include "ivideo/shader/shadervar.h"
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

SCF_IMPLEMENT_IBASE (csShaderManager::EventHandler)
SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY( csShaderManager )

SCF_IMPLEMENT_IBASE( csShader )
  SCF_IMPLEMENTS_INTERFACE( iShader )
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE( csShaderTechnique )
  SCF_IMPLEMENTS_INTERFACE( iShaderTechnique )
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE( csShaderPass )
  SCF_IMPLEMENTS_INTERFACE( iShaderPass )
SCF_IMPLEMENT_IBASE_END

SCF_EXPORT_CLASS_TABLE( shadermgr )
  SCF_EXPORT_CLASS( csShaderManager, "crystalspace.render3d.shadermanager", "Shader manager" )
SCF_EXPORT_CLASS_TABLE_END

//=================== csShaderManager ================//

// General stuff
csShaderManager::csShaderManager(iBase* parent)
{
  SCF_CONSTRUCT_IBASE( parent );
  SCF_CONSTRUCT_EMBEDDED_IBASE( scfiComponent );
  scfiEventHandler = NULL;

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

  int i;
  for(i = 0; i < pluginlist.Length(); ++i)
  {
    iShaderProgramPlugin* sp = (iShaderProgramPlugin*)pluginlist.Pop();
    sp->DecRef();
  }
}

bool csShaderManager::Initialize(iObjectRegistry *objreg)
{
  objectreg = objreg;
  vc = CS_QUERY_REGISTRY(objectreg, iVirtualClock);

  if (!scfiEventHandler)
    scfiEventHandler = new EventHandler (this);

  csRef<iEventQueue> q (CS_QUERY_REGISTRY(objectreg, iEventQueue));
  if (q)
    q->RegisterListener (scfiEventHandler, CSMASK_Broadcast || CSMASK_FrameProcess  );

  
  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY(objectreg, iPluginManager);

  iStrVector* classlist = iSCF::SCF->QueryClassList("crystalspace.render3d.shader.");
  int const nmatches = classlist->Length();
  if(nmatches != 0)
  {
    int i;
    for(i = 0; i < nmatches; ++i)
    {
      const char* classname = classlist->Get(i);
      csRef<iShaderProgramPlugin> plugin = CS_LOAD_PLUGIN(plugin_mgr, classname, iShaderProgramPlugin);
      if(plugin)
      {
        csReport( objectreg,  CS_REPORTER_SEVERITY_NOTIFY,"crystalspace.render3d.shadermgr", "Loaded plugin %s", classname);
        pluginlist.Push(plugin);
        plugin->IncRef();
        plugin->Open();
      }
    }
  }

  //create standard-variables
  sv_time = CreateVariable("STANDARD_TIME");
  AddVariable(sv_time);
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

bool csShaderManager::HandleEvent(iEvent& event)
{
  if (event.Type == csevBroadcast)
  {
    switch(event.Command.Code)
    {
    case cscmdPreProcess:
      UpdateStandardVariables();
      return false;
    }
  }
  return false;
}

void csShaderManager::UpdateStandardVariables()
{
  //time
  if(sv_time && vc)
  {
    float sec = vc->GetCurrentTicks()/1000.0f;
    sv_time->SetValue(sec);
  }
}

// Shader handling
csPtr<iShader> csShaderManager::CreateShader()
{
  char* name = new char[10];
  sprintf(name, "effect%2d", seqnumber);
  seqnumber++;

  csShader* cshader = new csShader(name, this);
  cshader->IncRef();

  shaders->Push(cshader);
  
  return (iShader*)cshader;
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

csPtr<iShaderProgram> csShaderManager::CreateShaderProgram(const char* type)
{
  int i;
  for(i = 0; i < pluginlist.Length(); ++i)
  {
    if( ((iShaderProgramPlugin*)pluginlist.Get(i))->SupportType(type))
      return ((iShaderProgramPlugin*)pluginlist.Get(i))->CreateProgram();
  }
  return NULL;
}

//===================== csShader ====================//
csShader::csShader(csShaderManager* owner)
{
  SCF_CONSTRUCT_IBASE( NULL );
  this->name = 0;
  variables = new csHashMap();
  techniques = new csBasicVector();
  parent = owner;
}

csShader::csShader(const char* name, csShaderManager* owner)
{
  SCF_CONSTRUCT_IBASE( NULL );
  this->name = 0;
  variables = new csHashMap();
  techniques = new csBasicVector();
  parent = owner;
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

  while(techniques->Length() > 0)
  {
    delete (iShaderTechnique*)techniques->Pop();
  }

  delete techniques;
}

bool csShader::IsValid()
{
  //is valid if there are at least one valid technique
  for(int i = 0; i < techniques->Length(); ++i)
  {
    iShaderTechnique* t = (iShaderTechnique*) techniques->Get(i);
    if(t->IsValid())
      return true;
  }
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
  iShaderVariable* var;
  csHashIterator cIter(variables, csHashCompute(name));

  if( cIter.HasNext() )
  {
    var = (iShaderVariable*)cIter.Next();

    return var;
  }

  if(parent)
  {
    return parent->GetVariable(name);
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
  iShaderTechnique* mytech = new csShaderTechnique(this);
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

bool csShader::Load(iDocumentNode* node)
{
  return false;
}

bool csShader::Load(iDataBuffer* program)
{
  return false;
}


struct priority_mapping
{
  int technique;
  int priority;
};

int pricompare( const void *arg1, const void *arg2 )
{
  priority_mapping* p1 = (priority_mapping*)arg1;
  priority_mapping* p2 = (priority_mapping*)arg2;
  if(p1->priority < p2->priority) return -1;
  if(p1->priority > p2->priority) return -1;
  return 0;
}

bool csShader::Prepare()
{
  int i;

  //go through the technques in priority order
  //fill priority struct
  priority_mapping* primap = new priority_mapping[techniques->Length()];
  
  for(i = 0; i < techniques->Length(); ++i)
  {
    primap[i].technique = i;
    primap[i].priority = ((iShaderTechnique*)techniques->Get(i))->GetPriority();
  }

  qsort(primap, techniques->Length()-1, sizeof(priority_mapping), pricompare);

  bool isPrep = false;
  int prepNr;

  csBasicVector* newTArr = new csBasicVector;

  for(i = 0; i < techniques->Length() && !isPrep; ++i)
  {
    iShaderTechnique* t = (iShaderTechnique*)techniques->Get(primap[i].technique);
    if ( t->Prepare() )
    {
      t->IncRef();
      newTArr->Push(t);
    }
  }
  
  while(techniques->Length() > 0)
  {
    ((iShaderTechnique*)techniques->Pop())->DecRef();
  }

  techniques = newTArr;

  return true;
}

//==================== csShaderPass ==============//
iShaderVariable* csShaderPass::GetVariable(const char* string)
{
  iShaderVariable* var;
  csHashIterator c(&variables, csHashCompute(string));

  if(c.HasNext())
  {
    var = (iShaderVariable*)c.Next();
    
    return var;
  }

  if (parent && parent->GetParent())
  {
    return parent->GetParent()->GetVariable(string);
  }

  return NULL;
}

bool csShaderPass::Load(iDocumentNode* node)
{
  return false;
}

bool csShaderPass::Load(iDataBuffer* program)
{
  return false;
}

bool csShaderPass::Prepare()
{
  if(vp) 
    if(!vp->Prepare())
      return false;

  if(fp)
    if(!fp->Prepare())
      return false;

  return true;
}

//================= csShaderTechnique ============//
csShaderTechnique::csShaderTechnique(csShader* owner)
{
  SCF_CONSTRUCT_IBASE( NULL );
  passes = new csBasicVector();
  parent = owner;
}

csShaderTechnique::~csShaderTechnique()
{
  while(passes->Length() > 0)
  {
    delete (iShaderPass*)passes->Pop();
  }
  delete passes;
}

csPtr<iShaderPass> csShaderTechnique::CreatePass()
{
  iShaderPass* mpass = new csShaderPass(this);
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
  bool valid = false;
  //returns true if all passes are valid
  for(int i = 0; i < passes->Length(); ++i)
  {
    iShaderPass* p = (iShaderPass*)passes->Get(i);
    valid = p->IsValid();
  }
  
  return valid;
}

bool csShaderTechnique::Load(iDocumentNode* node)
{
  return false;
}

bool csShaderTechnique::Load(iDataBuffer* program)
{
  return false;
}

bool csShaderTechnique::Prepare()
{
  for(int i = 0; i < passes->Length(); ++i)
  {
    iShaderPass* p = (iShaderPass*)passes->Get(i);
    if(!p->Prepare())
      return false;
  }
  return true;
}