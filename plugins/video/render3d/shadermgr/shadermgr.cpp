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
#include <limits.h>
#include "cstypes.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/scfstrset.h"
#include "csgeom/vector3.h"
#include "csutil/objreg.h"
#include "csutil/scfstr.h"
#include "csgeom/vector4.h"
#include "csutil/hashmap.h"
#include "csutil/xmltiny.h"

#include "iengine/engine.h"
#include "iengine/texture.h"
#include "iengine/material.h"
#include "igeom/clip2d.h"
#include "imap/services.h"
#include "iutil/document.h"
#include "iutil/vfs.h"
#include "iutil/eventq.h"
#include "iutil/virtclk.h"
#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "ivideo/texture.h"

#include "ivideo/shader/shader.h"
#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"
#include "ivideo/rendermesh.h"
#include "shadermgr.h"

// Pluginstuff
CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csShaderManager)
  SCF_IMPLEMENTS_INTERFACE (iShaderManager)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csShaderManager::Component)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csShaderManager::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csShaderManager)

SCF_IMPLEMENT_IBASE (csShader)
  SCF_IMPLEMENTS_INTERFACE (iShader)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csShaderTechnique)
  SCF_IMPLEMENTS_INTERFACE (iShaderTechnique)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csShaderPass)
  SCF_IMPLEMENTS_INTERFACE (iShaderPass)
SCF_IMPLEMENT_IBASE_END

//=================== csShaderWrapper ================//

SCF_IMPLEMENT_IBASE (csShaderWrapper)
  SCF_IMPLEMENTS_INTERFACE (iShaderWrapper)
SCF_IMPLEMENT_IBASE_END

//=================== csShaderManager ================//

// General stuff
csShaderManager::csShaderManager(iBase* parent)
{
  SCF_CONSTRUCT_IBASE(parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  scfiEventHandler = 0;

  seqnumber = 0;
}

csShaderManager::~csShaderManager()
{
  //clear all shaders
  /*while(shaders.Length() > 0)
  {
    delete (csShader*)shaders->Pop();
  }
  delete shaders;*/
  shaders.DeleteAll ();
  if (scfiEventHandler) scfiEventHandler->DecRef();
}

void csShaderManager::Report (int severity, const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  csReportV (objectreg, severity, "crystalspace.graphics3d.shadermgr", 
    msg, args);
  va_end (args);
}

bool csShaderManager::Initialize(iObjectRegistry *objreg)
{
  objectreg = objreg;
  vc = CS_QUERY_REGISTRY (objectreg, iVirtualClock);
  txtmgr = CS_QUERY_REGISTRY (objectreg, iTextureManager);

  if (!scfiEventHandler)
    scfiEventHandler = new EventHandler (this);

  csRef<iEventQueue> q = CS_QUERY_REGISTRY (objectreg, iEventQueue);
  if (q)
    q->RegisterListener (scfiEventHandler,
	CSMASK_Broadcast);

  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY  (objectreg,
	iPluginManager);

  csRef<iStringArray> classlist = csPtr<iStringArray> 
    (iSCF::SCF->QueryClassList("crystalspace.graphics3d.shader."));
  int const nmatches = classlist->Length();
  if (nmatches != 0)
  {
    int i;
    for (i = 0; i < nmatches; ++i)
    {
      const char* classname = classlist->Get(i);
      csRef<iShaderProgramPlugin> plugin = 
	CS_LOAD_PLUGIN (plugin_mgr, classname, iShaderProgramPlugin);
      if (plugin)
      {
        Report (CS_REPORTER_SEVERITY_NOTIFY, "Loaded plugin %s", classname);
        pluginlist.Push (plugin);
	// @@@ Really, shouldn't that be called in Open()?
        plugin->Open ();
      }
    }
  }
  else
  {
    Report (CS_REPORTER_SEVERITY_WARNING, "No shader plugins found!");
  }

  return true;
}

void csShaderManager::Open ()
{
}

void csShaderManager::Close ()
{
}

bool csShaderManager::HandleEvent(iEvent& event)
{
  if (event.Type == csevBroadcast)
  {
    switch(event.Command.Code)
    {
      case cscmdPreProcess:
  //      UpdateStandardVariables();
	return false;
      case cscmdSystemOpen:
	{
	  Open ();
	  return true;
	}
      case cscmdSystemClose:
	{
	  Close ();
	  return true;
	}
    }
  }
  return false;
}

// Shader handling
csPtr<iShader> csShaderManager::CreateShader()
{
  char* name = new char[10];
  sprintf(name, "effect%2d", seqnumber);
  seqnumber++;

  csShader* cshader = new csShader(name, this, objectreg);
  //cshader->IncRef();

  csRef<iShaderWrapper> wrap = CreateWrapper (cshader);
  shaders.Push(wrap);
  
  return csPtr<iShader> (cshader);
}

iShaderWrapper* csShaderManager::GetShader(const char* name)
{
  int i;
  for (i = 0; i < shaders.Length(); ++i)
  {
    iShaderWrapper* shader = shaders.Get(i);
    if (strcasecmp(shader->GetShader()->GetName(), name) == 0)
      return shader;
  }
  return 0;
}

csPtr<iShaderProgram> csShaderManager::CreateShaderProgram(const char* type)
{
  int i;

  if(!type) return 0;

  for (i = 0; i < pluginlist.Length(); ++i)
  {
    if (pluginlist.Get(i)->SupportType(type))
      return pluginlist.Get(i)->CreateProgram(type);
  }
  return 0;
}

void csShaderManager::PrepareShaders ()
{
  int i;
  for (i = 0; i < shaders.Length(); ++i)
    shaders[i]->GetShader ()->Prepare ();
}



//===================== csShader ====================//
csShader::csShader (csShaderManager* owner, iObjectRegistry* reg)
{
  SCF_CONSTRUCT_IBASE( 0 );
  this->name = 0;
  variables = new csHashMap();
  techniques = new csArray<iShaderTechnique*>();
  parent = owner;
  objectreg = reg;

  symtabs.SetLength (1, csSymbolTable());
}

csShader::csShader (const char* name, csShaderManager* owner,
	iObjectRegistry* reg)
{
  SCF_CONSTRUCT_IBASE( 0 );
  csShader::name = 0;
  variables = new csHashMap();
  techniques = new csArray<iShaderTechnique*>();
  parent = owner;
  objectreg = reg;
  SetName(name);

  symtabs.SetLength (1, csSymbolTable());
  symtab = symtabs[0];
}

csShader::~csShader()
{
  delete name;

  //Clear variables
  csGlobalHashIterator cIter (variables);

  while (cIter.HasNext())
  {
    csShaderVariable* i = (csShaderVariable*)cIter.Next();
    i->DecRef();
  }

  variables->DeleteAll();
  delete variables;

  while (techniques->Length() > 0)
  {
    delete (csShaderTechnique*)techniques->Pop();
  }
  delete techniques;
}

bool csShader::IsValid() const
{
  //is valid if there are at least one valid technique
  for(int i = 0; i < techniques->Length(); ++i)
  {
    iShaderTechnique* t = techniques->Get(i);
    if(t->IsValid())
      return true;
  }
  return false;
}

csShaderVariable* csShader::privateGetVariable (int namehash)
{
  csShaderVariable* var;
  csHashIterator cIter(variables, namehash);

  if (cIter.HasNext())
  {
    var = (csShaderVariable*)cIter.Next();
    return var;
  }

  if (parent)
  {
    return parent->privateGetVariable (namehash);
  }

  return 0;
}

//technique-related
csPtr<iShaderTechnique> csShader::CreateTechnique()
{
  iShaderTechnique* mytech = new csShaderTechnique (this, objectreg);
  mytech->IncRef();

  techniques->Push(mytech);
  AddChild (mytech);
  return mytech;
}

iShaderTechnique* csShader::GetTechnique(int technique)
{
  if( technique >= techniques->Length()) return 0;

  return techniques->Get(technique);
}

iShaderTechnique* csShader::GetBestTechnique()
{
  int i;
  int maxpriority = INT_MIN;
  iShaderTechnique* tech = 0;

  for (i = 0; i < techniques->Length(); ++i)
  {
    if( techniques->Get(i)->IsValid() &&
        techniques->Get(i)->GetPriority() > maxpriority)
    {
      tech = techniques->Get(i);
      maxpriority = tech->GetPriority();
    }
  }
  return tech;
}

void csShader::BuildTokenHash()
{
  xmltokens.Register ("shader", XMLTOKEN_SHADER);
  xmltokens.Register ("technique", XMLTOKEN_TECHNIQUE);
  xmltokens.Register ("declare", XMLTOKEN_DECLARE);

  xmltokens.Register("integer", 100+csShaderVariable::INT);
  xmltokens.Register("float", 100+csShaderVariable::FLOAT);
  xmltokens.Register("string", 100+csShaderVariable::STRING);
  xmltokens.Register("vector3", 100+csShaderVariable::VECTOR3);
}

bool csShader::Load (iDocumentNode* node)
{
  if (!node) 
    return false;

  BuildTokenHash();

  csRef<iStringSet> strings = 
    CS_QUERY_REGISTRY_TAG_INTERFACE (objectreg, 
    "crystalspace.renderer.stringset", iStringSet);

  if(node)
  {
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    //get the name, if any. otherwise don't set
    const char* name = node->GetAttributeValue("name");
    if(name && strlen(name))
      SetName(name);


    while(it->HasNext())
    {
      csRef<iDocumentNode> child = it->Next();
      if(child->GetType() != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch(id)
      {
      case XMLTOKEN_TECHNIQUE:
        {
          csRef<iShaderTechnique> tech = CreateTechnique();
          tech->Load(child);
        }
        break;
      case XMLTOKEN_DECLARE:
        {
          //create a new variable
          csRef<csShaderVariable> var = 
            parent->CreateVariable (
            strings->Request(child->GetAttributeValue ("name")));

          // @@@ Will leak! Should do proper refcounting.
          var->IncRef ();

          csStringID idtype = xmltokens.Request( child->GetAttributeValue("type") );
          idtype -= 100;
          var->SetType( (csShaderVariable::VariableType) idtype);
          switch(idtype)
          {
          case csShaderVariable::INT:
            var->SetValue( child->GetAttributeValueAsInt("default") );
            break;
          case csShaderVariable::FLOAT:
            var->SetValue( child->GetAttributeValueAsFloat("default") );
            break;
          case csShaderVariable::STRING:
            var->SetValue(new scfString( child->GetAttributeValue("default")) );
            break;
          case csShaderVariable::VECTOR3:
            const char* def = child->GetAttributeValue("default");
            csVector3 v;
            sscanf(def, "%f,%f,%f", &v.x, &v.y, &v.z);
            var->SetValue( v );
            break;
          }
          AddVariable (var);
        }
        break;
      }
    }
  }
  return true;
}

bool csShader::Load (iDataBuffer* program)
{
  csRef<iDocumentSystem> xml (CS_QUERY_REGISTRY (objectreg, iDocumentSystem));
  if (!xml) xml = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
  csRef<iDocument> doc = xml->CreateDocument ();
  const char* error = doc->Parse (program);
  if (error != 0)
  { 
    parent->Report (CS_REPORTER_SEVERITY_ERROR, 
      "Error parsing document: %s", error);
    return false;
  }
  return (Load (doc->GetRoot ()->GetNode ("shader")));
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
    primap[i].priority = techniques->Get(i)->GetPriority();
  }

  if (techniques->Length()>1)
    qsort(primap, techniques->Length()-1, sizeof(priority_mapping), pricompare);

  bool isPrep = false;
  //int prepNr;

  csArray<iShaderTechnique*>* newTArr = new csArray<iShaderTechnique*>;

  for(i = 0; i < techniques->Length() && !isPrep; ++i)
  {
    iShaderTechnique* t = techniques->Get(primap[i].technique);
    if ( t->Prepare() )
    {
      t->IncRef();
      newTArr->Push(t);
    }
  }
  
  while(techniques->Length() > 0)
  {
    (techniques->Pop())->DecRef();
  }
  delete techniques;

  techniques = newTArr;

  return true;
}

//==================== csShaderPass ==============//
csVertexAttrib csShaderPass::attribs[STREAMMAX*2];
iRenderBuffer* csShaderPass::buffers[STREAMMAX*2];
iRenderBuffer* csShaderPass::clear_buffers[STREAMMAX*2];
int csShaderPass::units[TEXMAX];
iTextureHandle* csShaderPass::textures[TEXMAX];
iTextureHandle* csShaderPass::clear_textures[TEXMAX];

void csShaderPass::Activate(csRenderMesh* mesh)
{
  if(vp) vp->Activate(this, mesh);
  if(fp) fp->Activate(this, mesh);
}

void csShaderPass::Deactivate()
{
  if(vp) vp->Deactivate(this);
  if(fp) fp->Deactivate(this);

  g3d->SetTextureState (units, clear_textures, TEXMAX);
  g3d->SetBufferState (attribs, clear_buffers, STREAMMAX*2);
}

void csShaderPass::SetupState (csRenderMesh *mesh)
{
  int i;
  memset (buffers, 0, sizeof (iRenderBuffer*)*STREAMMAX*2);
  for (i=0; i<STREAMMAX; i++)
  {
    if (streammapping[i] != csInvalidStringID)
    {
      iRenderBuffer* buf  = 
        mesh->buffersource->GetRenderBuffer (streammapping[i]);
      if (buf)
      {
        bool gen = streammappinggeneric[i];
        buffers[(i<<1)+(gen?0:1)] = buf;
      }
    }
  }
  g3d->SetBufferState (attribs, buffers, STREAMMAX*2);

  //iMaterialHandle* mathandle =
    //(mesh->material) ? (mesh->material->GetMaterialHandle()) : 0;

  memset (textures, 0, sizeof (iTextureHandle*)*TEXMAX);
  for (i=0; i<TEXMAX; i++)
  {
    if (texmapping[i] != csInvalidStringID)
    {
      csShaderVariable* var = GetVariable (texmapping[i]);
      if (var)
        var->GetValue (textures[i]);
    }
  }
  g3d->SetTextureState (units, textures, TEXMAX);

  g3d->GetWriteMask (OrigWMRed, OrigWMGreen, OrigWMBlue, OrigWMAlpha);
  g3d->SetWriteMask (writemaskRed, writemaskGreen, writemaskBlue, writemaskAlpha);
  
  if (vp) vp->SetupState (this, mesh);
  if (fp) fp->SetupState (this, mesh);
}

void csShaderPass::ResetState ()
{
  //int i;
  /*for (i=0; i<STREAMMAX; i++)
  {
    if (streammapping[i] != csInvalidStringID)
    {
      g3d->DeactivateBuffer ((csVertexAttrib)i);
    }
  }*/
  /*
  for (i=TEXMAX-1; i>=0; i--)
  {
    if (texmappingdirect[i] || texmappinglayer[i] != -1)
    {
      g3d->DeactivateTexture (i);
    }
  }*/
  if (vp) vp->ResetState ();
  if (fp) fp->ResetState ();

  g3d->SetWriteMask (OrigWMRed, OrigWMGreen, OrigWMBlue, OrigWMAlpha);
}

void csShaderPass::AddStreamMapping (csStringID name, csVertexAttrib attribute)
{
  int a = attribute<100?attribute:attribute-100;
  streammapping[a] = name;
  streammappinggeneric[a] = attribute<100;
}

csStringID csShaderPass::GetStreamMapping (csVertexAttrib attribute) const
{
  int a = attribute<100?attribute:attribute-100;
  //csStringID s = streammapping[a];
  if ((attribute<100 && streammappinggeneric[a]) ||
      (attribute>=100 && !streammappinggeneric[a]))
    return streammapping[a];
  else return csInvalidStringID;
}

void csShaderPass::AddTextureMapping (csStringID name, int unit)
{
  texmapping[unit] = name;
}

csStringID csShaderPass::GetTextureMapping (int unit) const
{
  return texmapping[unit];
}

void csShaderPass::BuildTokenHash()
{
  xmltokens.Register ("declare", XMLTOKEN_DECLARE);
  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("streammapping", XMLTOKEN_STREAMMAPPING);
  xmltokens.Register ("texturemapping", XMLTOKEN_TEXTUREMAPPING);
  xmltokens.Register ("vp", XMLTOKEN_VP);
  xmltokens.Register ("fp", XMLTOKEN_FP);
  xmltokens.Register ("writemask", XMLTOKEN_WRITEMASK);

  xmltokens.Register("integer", 100+csShaderVariable::INT);
  xmltokens.Register("float", 100+csShaderVariable::FLOAT);
  xmltokens.Register("string", 100+csShaderVariable::STRING);
  xmltokens.Register("vector3", 100+csShaderVariable::VECTOR3);
}

bool csShaderPass::Load (iDocumentNode* node)
{

  if (!node) 
    return false;

  BuildTokenHash();
  csRef<iShaderManager> shadermgr = CS_QUERY_REGISTRY(objectreg, iShaderManager);
  csRef<iSyntaxService> synserv = 
    CS_QUERY_REGISTRY (objectreg, iSyntaxService);

  csRef<iStringSet> strings = 
    CS_QUERY_REGISTRY_TAG_INTERFACE (objectreg, 
    "crystalspace.renderer.stringset", iStringSet);

  if(node)
  {
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while(it->HasNext())
    {
      csRef<iDocumentNode> child = it->Next();
      if(child->GetType() != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch(id)
      {
      case XMLTOKEN_MIXMODE:
        {
          synserv->ParseMixmode (child, mixmode);
        }
        break;
      case XMLTOKEN_STREAMMAPPING:
        {
          const char* dest = child->GetAttributeValue ("destination");
          csVertexAttrib attribute;
          bool found = false;
          int i;
          for (i = 0; i<16; i++)
          {
            char str[13];
            sprintf (str, "attribute %d", i);
            if (strcmp (str, dest) == 0)
            {
              attribute = (csVertexAttrib)(CS_VATTRIB_0 + i);
              found = true;
              break;
            }
          }
          if (!found)
          {
            if (strcmp (dest, "position") == 0)
            {
              attribute = CS_VATTRIB_POSITION;
              found = true;
            }
            else if (strcmp (dest, "normal") == 0)
            {
              attribute = CS_VATTRIB_NORMAL;
              found = true;
            }
            else if (strcmp (dest, "color") == 0)
            {
              attribute = CS_VATTRIB_COLOR;
              found = true;
            }
            else if (strcmp (dest, "primary color") == 0)
            {
              attribute = CS_VATTRIB_PRIMARY_COLOR;
              found = true;
            }
            else if (strcmp (dest, "secondary color") == 0)
            {
              attribute = CS_VATTRIB_SECONDARY_COLOR;
              found = true;
            }
            else if (strcmp (dest, "texture coordinate") == 0)
            {
              attribute = CS_VATTRIB_TEXCOORD;
              found = true;
            }
            else if (strcmp (dest, "texture coordinate 0") == 0)
            {
              attribute = CS_VATTRIB_TEXCOORD0;
              found = true;
            }
            else if (strcmp (dest, "texture coordinate 1") == 0)
            {
              attribute = CS_VATTRIB_TEXCOORD1;
              found = true;
            }
            else if (strcmp (dest, "texture coordinate 2") == 0)
            {
              attribute = CS_VATTRIB_TEXCOORD2;
              found = true;
            }
            else if (strcmp (dest, "texture coordinate 3") == 0)
            {
              attribute = CS_VATTRIB_TEXCOORD3;
              found = true;
            }
          }
          if (found)
          {
            AddStreamMapping (
              strings->Request (child->GetAttributeValue ("stream")),
              attribute);
          }
        }
        break;
      case XMLTOKEN_TEXTUREMAPPING:
        {
          if (child->GetAttribute ("name"))
          {
            AddTextureMapping (strings->Request (
              child->GetAttributeValue ("name")),
              child->GetAttributeValueAsInt ("unit"));
          }
        }
        break;
      case XMLTOKEN_VP:
        {
          csRef<iShaderProgram> vp = shadermgr->CreateShaderProgram(child->GetAttributeValue("type"));
          if(vp)
          {
            if(child->GetAttribute("file"))
            {
              csRef<iVFS> vfs = CS_QUERY_REGISTRY(objectreg, iVFS);
              //load from file
              vp->Load( csRef<iDataBuffer>(vfs->ReadFile(child->GetAttributeValue("file"))));
            }
            else
            {
              vp->Load(child);
            }
            SetVertexProgram(vp);
          }
        }
        break;
      case XMLTOKEN_FP:
        {
          csRef<iShaderProgram> fp = shadermgr->CreateShaderProgram(child->GetAttributeValue("type"));
          if(fp)
          {
            if(child->GetAttribute("file"))
            {
              csRef<iVFS> vfs = CS_QUERY_REGISTRY(objectreg, iVFS);
              //load from file
              fp->Load( csRef<iDataBuffer>(vfs->ReadFile(child->GetAttributeValue("file"))));
            }
            else
            {
              fp->Load(child);
            }
            SetFragmentProgram(fp);
          }
        }
        break;
      case XMLTOKEN_DECLARE:
        {
          //create a new variable
          csRef<csShaderVariable> var = 
            shadermgr->CreateVariable (
            strings->Request (child->GetAttributeValue ("name")));

          // @@@ Will leak! Should do proper refcounting.
          var->IncRef ();

          csStringID idtype = xmltokens.Request( child->GetAttributeValue("type") );
          idtype -= 100;
          var->SetType( (csShaderVariable::VariableType) idtype);
          switch(idtype)
          {
          case csShaderVariable::INT:
            var->SetValue( child->GetAttributeValueAsInt("default") );
            break;
          case csShaderVariable::FLOAT:
            var->SetValue( child->GetAttributeValueAsFloat("default") );
            break;
          case csShaderVariable::STRING:
            var->SetValue(new scfString( child->GetAttributeValue("default")) );
            break;
          case csShaderVariable::VECTOR3:
            const char* def = child->GetAttributeValue("default");
            csVector3 v;
            sscanf(def, "%f,%f,%f", &v.x, &v.y, &v.z);
            var->SetValue( v );
            break;
          }
          AddVariable (var);
        }
        break;
      case XMLTOKEN_WRITEMASK:
        {
          if (strcasecmp(child->GetAttributeValue ("r"), "true")==0)
            writemaskRed = true;
          else if (strcasecmp(child->GetAttributeValue ("r"), "false")==0)
            writemaskRed = false;
          
          if (strcasecmp(child->GetAttributeValue ("g"), "true")==0)
            writemaskGreen = true;
          else if (strcasecmp(child->GetAttributeValue ("g"), "false")==0)
            writemaskGreen = false;

          if (strcasecmp(child->GetAttributeValue ("b"), "true")==0)
            writemaskBlue = true;
          else if (strcasecmp(child->GetAttributeValue ("b"), "false")==0)
            writemaskBlue = false;

          if (strcasecmp(child->GetAttributeValue ("a"), "true")==0)
            writemaskAlpha = true;
          else if (strcasecmp(child->GetAttributeValue ("a"), "false")==0)
            writemaskAlpha = false;
        }
        break;
      }
    }
  }
  return true;
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
csShaderTechnique::csShaderTechnique(csShader* owner, iObjectRegistry* reg)
{
  SCF_CONSTRUCT_IBASE( 0 );
  passes = new csArray<iShaderPass*>();
  parent = owner;
  objectreg = reg;

  symtabs.SetLength (1, csSymbolTable());
}

csShaderTechnique::~csShaderTechnique()
{
  while(passes->Length() > 0)
  {
    delete (csShaderPass*)passes->Pop();
  }
  delete passes;
}

csPtr<iShaderPass> csShaderTechnique::CreatePass()
{
  iShaderPass* mpass = new csShaderPass(this, objectreg);
  mpass->IncRef();

  passes->Push(mpass);
  AddChild (mpass);
  return mpass;
}

iShaderPass* csShaderTechnique::GetPass(int pass)
{
  if( pass >= passes->Length()) return 0;

  return passes->Get(pass);
}

bool csShaderTechnique::IsValid() const
{
  bool valid = false;
  //returns true if all passes are valid
  for(int i = 0; i < passes->Length(); ++i)
  {
    iShaderPass* p = passes->Get(i);
    valid = p->IsValid();
  }
  
  return valid;
}

void csShaderTechnique::BuildTokenHash()
{
  xmltokens.Register ("pass", XMLTOKEN_PASS);
  xmltokens.Register ("declare", XMLTOKEN_DECLARE);

  xmltokens.Register("integer", 100+csShaderVariable::INT);
  xmltokens.Register("float", 100+csShaderVariable::FLOAT);
  xmltokens.Register("string", 100+csShaderVariable::STRING);
  xmltokens.Register("vector3", 100+csShaderVariable::VECTOR3);

}

bool csShaderTechnique::Load (iDocumentNode* node)
{
  if (!node) 
    return false;

  BuildTokenHash();

  csRef<iShaderManager> shadermgr = CS_QUERY_REGISTRY (objectreg,
	iShaderManager);
  csRef<iStringSet> strings = 
    CS_QUERY_REGISTRY_TAG_INTERFACE (objectreg, 
    "crystalspace.renderer.stringset", iStringSet);

  if(node)
  {
    priority = node->GetAttributeValueAsInt("priority");
    SetPriority(priority);
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext())
    {
      csRef<iDocumentNode> child = it->Next();
      if(child->GetType() != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch(id)
      {
        case XMLTOKEN_PASS:
          {
            csRef<iShaderPass> pass = CreatePass();
            pass->Load(child);
		    // CreatePass already pushes this
            // passes->Push(pass);
          }
          break;
        case XMLTOKEN_DECLARE:
          {
            //create a new variable
            csRef<csShaderVariable> var = shadermgr->CreateVariable (
              strings->Request(child->GetAttributeValue ("name")));

            // @@@ Will leak! Should do proper refcounting.
            var->IncRef ();

            csStringID idtype = xmltokens.Request (
			child->GetAttributeValue("type") );
            idtype -= 100;
            var->SetType ((csShaderVariable::VariableType) idtype);
            switch(idtype)
            {
              case csShaderVariable::INT:
                var->SetValue (child->GetAttributeValueAsInt("default"));
                break;
              case csShaderVariable::FLOAT:
                var->SetValue (child->GetAttributeValueAsFloat("default"));
                break;
              case csShaderVariable::STRING:
                var->SetValue (new scfString (
			child->GetAttributeValue("default")));
                break;
              case csShaderVariable::VECTOR3:
                const char* def = child->GetAttributeValue("default");
                csVector3 v;
                sscanf(def, "%f,%f,%f", &v.x, &v.y, &v.z);
                var->SetValue( v );
                break;
            }
            AddVariable (var);
          }
          break;
      }
    }
  }
  return true;
}

bool csShaderTechnique::Load(iDataBuffer* program)
{
  return false;//don't load a technique from file
}

bool csShaderTechnique::Prepare()
{
  for (int i = 0; i < passes->Length(); ++i)
  {
    iShaderPass* p = passes->Get(i);
    if (!p->Prepare())
      return false;
  }
  return true;
}

//@@@: What's the point of #undef'ing these? It's the end of the file!
#undef STREAMMAX
#undef TEXTUREMAX

