/*
  Copyright (C) 2003 by Marten Svanfeldt

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
#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "iutil/cmdline.h"
#include "imap/services.h"
#include "ivaria/reporter.h"
#include "ivideo/rendermesh.h"
#include "csutil/util.h"
#include "csutil/scfstr.h"
#include "csutil/scfstrset.h"
#include "csutil/xmltiny.h"
#include "xmlshader.h"

CS_LEAKGUARD_IMPLEMENT (csXMLShaderTech)
CS_LEAKGUARD_IMPLEMENT (csXMLShader)
CS_LEAKGUARD_IMPLEMENT (csXMLShaderCompiler)

CS_IMPLEMENT_PLUGIN

//---------------------------------------------------------------------------

iRenderBuffer* csXMLShaderTech::last_buffers[shaderPass::STREAMMAX*2];
iRenderBuffer* csXMLShaderTech::clear_buffers[shaderPass::STREAMMAX*2];
int csXMLShaderTech::lastBufferCount;

iTextureHandle* csXMLShaderTech::last_textures[shaderPass::TEXTUREMAX];
iTextureHandle* csXMLShaderTech::clear_textures[shaderPass::TEXTUREMAX];
int csXMLShaderTech::textureUnits[shaderPass::TEXTUREMAX];
int csXMLShaderTech::lastTexturesCount;

csXMLShaderTech::csXMLShaderTech (csXMLShader* parent) : 
  passes(0), passesCount(0), currentPass(~0),
  xmltokens (parent->compiler->xmltokens)
{
  csXMLShaderTech::parent = parent;

  do_verbose = parent->compiler->do_verbose;

  int i;
  for (i = 0; i < shaderPass::TEXTUREMAX; i++)
    textureUnits[i] = i;
}

csXMLShaderTech::~csXMLShaderTech()
{
  delete[] passes;
}

static inline bool IsDestalphaMixmode (uint mode)
{
  return (mode == CS_FX_DESTALPHAADD);
}

bool csXMLShaderTech::LoadPass (iDocumentNode *node, shaderPass *pass)
{
  iSyntaxService* synldr = parent->compiler->synldr;
  iStringSet* strings = parent->compiler->strings;

  //Load shadervar block
  csRef<iDocumentNode> varNode = node->GetNode(
    xmltokens.Request (csXMLShaderCompiler::XMLTOKEN_SHADERVARS));
 
  if (varNode)
    LoadSVBlock (varNode, &pass->svcontext);

  //load vp
  csRef<iDocumentNode> programNode;
  csRef<iShaderProgram> program;
  programNode = node->GetNode(xmltokens.Request (
    csXMLShaderCompiler::XMLTOKEN_VP));

  if (programNode)
  {
    program = LoadProgram (programNode, pass);
    if (program)
    {
      pass->vp = program;
    }
    else
    {
      if (do_verbose)
        SetFailReason ("vertex program failed to load");
      else
	parent->compiler->Report (CS_REPORTER_SEVERITY_NOTIFY, 
	  "Vertex Program for shader '%s' failed to load",
	  parent->GetName ());
      return false;
    }
  }

  programNode = node->GetNode (xmltokens.Request (
    csXMLShaderCompiler::XMLTOKEN_FP));

  if (programNode)
  {
    program = LoadProgram (programNode, pass);
    if (program)
      pass->fp = program;
    else
    {
      if (do_verbose)
        SetFailReason ("fragment program failed to load");
      else
        parent->compiler->Report (CS_REPORTER_SEVERITY_NOTIFY, 
	  "Fragment Program for shader '%s' failed to load",
	  parent->GetName ());
      return false;
    }
  }

  {
    csRef<iDocumentNode> nodeMixMode = node->GetNode ("mixmode");
    if (nodeMixMode != 0)
    {
      uint mm;
      if (synldr->ParseMixmode (nodeMixMode, mm, true))
	pass->mixMode = mm;
    }

    const csGraphics3DCaps* caps = parent->g3d->GetCaps();
    if (IsDestalphaMixmode (pass->mixMode) && !caps->DestinationAlpha)
    {
      if (do_verbose)
        SetFailReason ("destination alpha not supported by renderer");
      return false;
    }

    csRef<iDocumentNode> nodeAlphaMode = node->GetNode ("alphamode");
    if (nodeAlphaMode != 0)
    {
      csAlphaMode am;
      if (synldr->ParseAlphaMode (nodeAlphaMode, strings, am))
      {
	pass->alphaMode = am;
      }
    }

    csRef<iDocumentNode> nodeZmode = node->GetNode ("zmode");
    if (nodeZmode != 0)
    {
      csRef<iDocumentNode> node;
      csRef<iDocumentNodeIterator> it;
      it = nodeZmode->GetNodes ();
      while (it->HasNext ())
      {
	csRef<iDocumentNode> child = it->Next ();
	if (child->GetType () != CS_NODE_ELEMENT) continue;
	node = child;
	break;
      }

      if (synldr->ParseZMode (node, pass->zMode, true))
      {
	pass->overrideZmode = true;
      }
      else
      {
	synldr->ReportBadToken (node);
      }
    }

    pass->wmRed = true;
    pass->wmGreen = true;
    pass->wmBlue = true;
    pass->wmAlpha = true;
    csRef<iDocumentNode> nodeWM = node->GetNode ("writemask");
    if (nodeWM != 0)
    {
      if (nodeWM->GetAttributeValue ("r") != 0)
	pass->wmRed = !strcasecmp (nodeWM->GetAttributeValue ("r"), "true");
      if (nodeWM->GetAttributeValue ("g") != 0)
	pass->wmGreen = !strcasecmp (nodeWM->GetAttributeValue ("g"), "true");
      if (nodeWM->GetAttributeValue ("b") != 0)
	pass->wmBlue = !strcasecmp (nodeWM->GetAttributeValue ("b"), "true");
      if (nodeWM->GetAttributeValue ("a") != 0)
	pass->wmAlpha = !strcasecmp (nodeWM->GetAttributeValue ("a"), "true");
    }
  }
  if (pass->alphaMode.autoAlphaMode)
  {
    if (pass->alphaMode.autoModeTexture != csInvalidStringID)
    {
      csShaderVariable *var;
      var = pass->svcontext.GetVariable (pass->alphaMode.autoModeTexture);

      if(!var)
	var = svcontext.GetVariable (pass->alphaMode.autoModeTexture);
      if (var)
	pass->autoAlphaTexRef = var;
    }
  }

  //if we got this far, load buffermappings
  csRef<iDocumentNodeIterator> it;
  it = node->GetNodes (xmltokens.Request (csXMLShaderCompiler::XMLTOKEN_BUFFER));
  pass->bufferCount = 0;
  while(it->HasNext ())
  {
    csRef<iDocumentNode> mapping = it->Next ();
    if (mapping->GetType () != CS_NODE_ELEMENT) continue;
    
    const char* dest = mapping->GetAttributeValue ("destination");
    csVertexAttrib attrib = CS_VATTRIB_0;
    bool found = false;
    int i;
    for(i=0;i<16;i++)
    {
      char str[13];
      sprintf (str, "attribute %d", i);
      if (strcasecmp(str, dest)==0)
      {
        found = true;
        attrib = (csVertexAttrib)(CS_VATTRIB_0 + i);
        break;
      }
    }
    if (!found)
    {
      if (strcasecmp (dest, "position") == 0)
      {
        attrib = CS_VATTRIB_POSITION;
        found = true;
      }
      else if (strcasecmp (dest, "normal") == 0)
      {
        attrib = CS_VATTRIB_NORMAL;
        found = true;
      }
      else if (strcasecmp (dest, "color") == 0)
      {
        attrib = CS_VATTRIB_COLOR;
        found = true;
      }
      else if (strcasecmp (dest, "primary color") == 0)
      {
        attrib = CS_VATTRIB_PRIMARY_COLOR;
        found = true;
      }
      else if (strcasecmp (dest, "secondary color") == 0)
      {
        attrib = CS_VATTRIB_SECONDARY_COLOR;
        found = true;
      }
      else if (strcasecmp (dest, "texture coordinate") == 0)
      {
        attrib = CS_VATTRIB_TEXCOORD;
        found = true;
      }
      else
      {
	static const char mapName[] = "texture coordinate ";
	if (strncasecmp (dest, mapName, sizeof (mapName) - 1) == 0)
	{
	  const char* target = dest + sizeof (mapName) - 1;

	  for (int u = 0; u < 8; u++)
	  {
	    char buf[2];
	    sprintf (buf, "%d", u);
	    if (strcmp (target, buf) == 0)
	    {
	      attrib = (csVertexAttrib)((int)CS_VATTRIB_TEXCOORD0 + u);
	      found = true;
	      break;
	    }
	  }
	  if (!found && pass->fp)
	  {
	    int texUnit = pass->fp->ResolveTextureBinding (target);
	    if (texUnit >= 0)
	    {
	      attrib = (csVertexAttrib)((int)CS_VATTRIB_TEXCOORD0 + texUnit);
	      found = true;
	    }
	  }
	}
      }
    }
    if (found)
    {
      csStringID varID = strings->Request (mapping->GetAttributeValue("name"));
      pass->bufferID[pass->bufferCount] = varID; //MUST HAVE STRINGS
      pass->bufferGeneric[pass->bufferCount] = CS_VATTRIB_IS_GENERIC (attrib);

      csShaderVariable *varRef=0;
      varRef = pass->svcontext.GetVariable(pass->bufferID[pass->bufferCount]);

      if(!varRef)
        varRef = svcontext.GetVariable (pass->bufferID[pass->bufferCount]);

      if (varRef)
	pass->bufferRef[pass->bufferCount] = varRef;
      pass->vertexattributes[pass->bufferCount] = attrib;
      pass->bufferCount++;
    }
    else
    {
      parent->compiler->Report (CS_REPORTER_SEVERITY_WARNING,
	"Shader '%s', pass %d: invalid buffer destination '%s'",
	parent->GetName (), GetPassNumber (pass), dest);
    }
  }

  if (pass->bufferCount == 0)
  {
    parent->compiler->Report (CS_REPORTER_SEVERITY_WARNING,
      "Shader '%s', pass %d has no buffer mappings",
      parent->GetName (), GetPassNumber (pass));
  }

  //get texturemappings
  pass->textureCount = 0;
  it = node->GetNodes (xmltokens.Request (
    csXMLShaderCompiler::XMLTOKEN_TEXTURE));
  while(it->HasNext ())
  {
    csRef<iDocumentNode> mapping = it->Next ();
    if (mapping->GetType() != CS_NODE_ELEMENT) continue;
    const char* dest = mapping->GetAttributeValue ("destination");
    if (mapping->GetAttribute("name") && dest)
    {
      char unitName[8];
      int i;
      int texUnit = -1;
      for (i = 0; i < shaderPass::TEXTUREMAX; i++)
      {
	sprintf (unitName, "unit %d", i);
	if (strcasecmp (unitName, dest) == 0)
	{
	  texUnit = i;
	  break;
	}
      }

      if ((texUnit < 0) && pass->fp)
	texUnit = pass->fp->ResolveTextureBinding (dest);

      if (texUnit < 0) continue;
      csStringID varID = strings->Request (mapping->GetAttributeValue("name"));
      pass->textureID[texUnit] = varID;

      csShaderVariable *varRef=0;
      varRef = pass->svcontext.GetVariable (varID);

      if(!varRef)
        varRef = svcontext.GetVariable (varID);

      if (varRef)
	pass->textureRef[texUnit] = varRef;

      pass->textureCount = MAX(pass->textureCount, texUnit + 1);
    }
  }

  if (pass->textureCount == 0)
  {
    parent->compiler->Report (CS_REPORTER_SEVERITY_WARNING,
      "Shader '%s', pass %d has no texture mappings",
      parent->GetName (), GetPassNumber (pass));
    
  }

  return true;
}

bool csXMLShaderTech::LoadSVBlock (iDocumentNode *node,
  iShaderVariableContext *context)
{
  iStringSet* strings = parent->compiler->strings;
  iSyntaxService* synldr = parent->compiler->synldr;

  csRef<csShaderVariable> svVar;
  
  csRef<iDocumentNodeIterator> it = node->GetNodes ("shadervar");
  while (it->HasNext ())
  {
    csRef<iDocumentNode> var = it->Next ();
    svVar.AttachNew (new csShaderVariable (
      strings->Request(var->GetAttributeValue ("name"))));

    if (synldr->ParseShaderVar (var, *svVar))
      context->AddVariable(svVar);
  }

  return true;
}

csPtr<iShaderProgram> csXMLShaderTech::LoadProgram (iDocumentNode *node, 
						    shaderPass *pass)
{
  if (node->GetAttributeValue("plugin") == 0)
  {
    parent->compiler->Report (CS_REPORTER_SEVERITY_ERROR,
      "No shader program plugin specified for <%s> in shader '%s'",
      node->GetValue (), parent->GetName ());
    return 0;
  }

  csRef<iShaderProgram> program;

  const char *pluginprefix = "crystalspace.graphics3d.shader.";
  char *plugin = new char[strlen(pluginprefix) + 255 + 1];
  strcpy (plugin, pluginprefix);

  strncat (plugin, node->GetAttributeValue("plugin"), 255);
  // @@@ Also check if 'plugin' is a full class ID

  //load the plugin
  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (
    parent->compiler->objectreg, iPluginManager);

  csRef<iShaderProgramPlugin> plg;
  plg = CS_QUERY_PLUGIN_CLASS(plugin_mgr, plugin, iShaderProgramPlugin);
  if(!plg)
  {
    plg = CS_LOAD_PLUGIN(plugin_mgr, plugin, iShaderProgramPlugin);
    if (!plg)
    {
      parent->compiler->Report (CS_REPORTER_SEVERITY_ERROR,
	"Couldn't retrieve shader plugin '%s' for <%s> in shader '%s'",
	plugin, node->GetValue (), parent->GetName ());
      delete[] plugin;
      return 0;
    }
  }

  delete[] plugin;
  const char* programType = node->GetAttributeValue("type");
  if (programType == 0)
    programType = node->GetValue ();
  program = plg->CreateProgram (programType);
  if (program == 0)
    return 0;
  csRef<iDocumentNode> programNode;
  if (node->GetAttributeValue ("file") != 0)
  {
    csRef<iVFS> VFS = CS_QUERY_REGISTRY(parent->compiler->objectreg, 
      iVFS);
    csRef<iFile> programFile = VFS->Open (node->GetAttributeValue ("file"),
      VFS_FILE_READ);
    if (!programFile)
    {
      parent->compiler->Report (CS_REPORTER_SEVERITY_ERROR,
        "Unable to load shader program file '%s'",
        node->GetAttributeValue ("file"));
      return 0;
    }
    csRef<iDocumentSystem> docsys (
      CS_QUERY_REGISTRY(parent->compiler->objectreg, iDocumentSystem));
    if (docsys == 0)
      docsys.AttachNew (new csTinyDocumentSystem ());

    csRef<iDocument> programDoc = docsys->CreateDocument ();
    programDoc->Parse (programFile);
    /*csRef<iDocumentNodeIterator> it = programDoc->GetRoot ()->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      programNode = child;
      break;
    }*/
    programNode = programDoc->GetRoot ();
  }
  else
    programNode = node;
  if (!program->Load (programNode))
    return 0;

  csArray<iShaderVariableContext*> staticContexts;
  staticContexts.Push (&pass->svcontext);
  staticContexts.Push (&svcontext);
  if (!program->Compile (staticContexts))
    return 0;

  return csPtr<iShaderProgram> (program);
}

bool csXMLShaderTech::Load (iDocumentNode* node, iDocumentNode* parentSV)
{
  if ((node->GetType() != CS_NODE_ELEMENT) || 
    (xmltokens.Request (node->GetValue()) 
    != csXMLShaderCompiler::XMLTOKEN_TECHNIQUE))
  {
    if (do_verbose) SetFailReason ("Node is not a well formed technique");
    return 0;
  }

  iStringSet* strings = parent->compiler->strings;
  iShaderManager* shadermgr = parent->compiler->shadermgr;

  int requiredCount;
  const csSet<csStringID>& requiredTags = 
    shadermgr->GetTags (TagRequired, requiredCount);
  int forbiddenCount;
  const csSet<csStringID>& forbiddenTags = 
    shadermgr->GetTags (TagForbidden, forbiddenCount);

  int requiredPresent = 0;
  csRef<iDocumentNodeIterator> it = node->GetNodes (
    xmltokens.Request (csXMLShaderCompiler::XMLTOKEN_TAG));
  while (it->HasNext ())
  {
    csRef<iDocumentNode> tag = it->Next ();

    const char* tagName = tag->GetContentsValue ();
    csStringID tagID = strings->Request (tagName);
    if (requiredTags.In (tagID))
    {
      requiredPresent++;
    }
    else if (forbiddenTags.In (tagID))
    {
      if (do_verbose) SetFailReason ("Shader tag '%s' is forbidden", 
	tagName);
      return 0;
    }
  }

  if ((requiredCount != 0) && (requiredPresent == 0))
  {
    if (do_verbose) SetFailReason ("No required shader tag is present");
    return 0;
  }

  //count passes
  passesCount = 0;
  it = node->GetNodes (xmltokens.Request (
    csXMLShaderCompiler::XMLTOKEN_PASS));
  while(it->HasNext ())
  {
    it->Next ();
    passesCount++;
  }

  //load shadervariable definitions
  if (parentSV)
  {
    csRef<iDocumentNode> varNode = parentSV->GetNode(
      xmltokens.Request (csXMLShaderCompiler::XMLTOKEN_SHADERVARS));
    if (varNode)
      LoadSVBlock (varNode, &svcontext);
  }

  csRef<iDocumentNode> varNode = node->GetNode(
    xmltokens.Request (csXMLShaderCompiler::XMLTOKEN_SHADERVARS));

  if (varNode)
    LoadSVBlock (varNode, &svcontext);

  //alloc passes
  passes = new shaderPass[passesCount];
  uint i;
  for (i = 0; i < passesCount; i++)
  {
    shaderPass& pass = passes[i];
    pass.alphaMode.autoAlphaMode = true;
    pass.alphaMode.autoModeTexture = 
      strings->Request (CS_MATERIAL_TEXTURE_DIFFUSE);
  }


  //first thing we load is the programs for each pass.. if one of them fail,
  //fail the whole technique
  int currentPassNr = 0;
  it = node->GetNodes (xmltokens.Request (csXMLShaderCompiler::XMLTOKEN_PASS));
  while (it->HasNext ())
  {
    csRef<iDocumentNode> passNode = it->Next ();
    passes[currentPassNr].owner = this;
    if (!LoadPass (passNode, &passes[currentPassNr++]))
    {
      return false;
    }
  }

  return true;
}

bool csXMLShaderTech::ActivatePass (unsigned int number)
{
  if(number>=passesCount)
    return false;

  currentPass = number;

  shaderPass *thispass = &passes[currentPass];
  if(thispass->vp)
    thispass->vp->Activate ();
  if(thispass->fp)
    thispass->fp->Activate ();
  
  iGraphics3D* g3d = parent->g3d;
  if (thispass->overrideZmode)
  {
    oldZmode = g3d->GetZMode ();
    g3d->SetZMode (thispass->zMode);
  }

  g3d->GetWriteMask (orig_wmRed, orig_wmGreen, orig_wmBlue, orig_wmAlpha);
  g3d->SetWriteMask (thispass->wmRed, thispass->wmGreen, thispass->wmBlue,
    thispass->wmAlpha);

  return true;
}

bool csXMLShaderTech::DeactivatePass ()
{
  if(currentPass>=passesCount)
    return false;
  shaderPass *thispass = &passes[currentPass];
  currentPass = ~0;

  if(thispass->vp) thispass->vp->Deactivate ();
  if(thispass->fp) thispass->fp->Deactivate ();

  iGraphics3D* g3d = parent->g3d;
  g3d->SetBufferState(thispass->vertexattributes, clear_buffers, 
    lastBufferCount);
  lastBufferCount=0;

  g3d->SetTextureState(textureUnits, clear_textures, 
    lastTexturesCount);
  lastTexturesCount=0;
  
  if (thispass->overrideZmode)
    g3d->SetZMode (oldZmode);

  g3d->SetWriteMask (orig_wmRed, orig_wmGreen, orig_wmBlue, orig_wmAlpha);

  return true;
}

bool csXMLShaderTech::SetupPass (const csRenderMesh *mesh, 
			     csRenderMeshModes& modes,
			     const csShaderVarStack &stacks)
{
  if(currentPass>=passesCount)
    return false;

  iGraphics3D* g3d = parent->g3d;
  shaderPass *thispass = &passes[currentPass];

  //now map our buffers. all refs should be set
  int i;
  for (i = 0; i < thispass->bufferCount; i++)
  {
    if (thispass->bufferRef[i] != 0)
      thispass->bufferRef[i]->GetValue(last_buffers[i]);
    else if (thispass->bufferID[i] < (csStringID)stacks.Length ())
    {
      csShaderVariable* var = 0;
      if (stacks[thispass->bufferID[i]].Length () > 0)
        var = stacks[thispass->bufferID[i]].Top ();
      if (var)
        var->GetValue(last_buffers[i]);
      else
        last_buffers[i] = 0;
    }
    else
      last_buffers[i] = 0;
  }
  g3d->SetBufferState (thispass->vertexattributes, last_buffers, 
    thispass->bufferCount);
  lastBufferCount = thispass->bufferCount;

  //and the textures
  for (i = 0; i < thispass->textureCount; i++)
  {
    if (thispass->textureRef[i] != 0)
    {
      iTextureWrapper* wrap;
      thispass->textureRef[i]->GetValue(wrap);
      if (wrap)
      {
        wrap->Visit ();
        last_textures[i] = wrap->GetTextureHandle ();
      }
      else
        last_textures[i] = 0;
    }
    else if (thispass->textureID[i] < (csStringID)stacks.Length ())
    {
      csShaderVariable* var = 0;
      if (stacks[thispass->textureID[i]].Length () > 0)
        var = stacks[thispass->textureID[i]].Top ();
      if (var)
      {
        iTextureWrapper* wrap;
        var->GetValue(wrap);
        if (wrap) 
        {
          wrap->Visit ();
          last_textures[i] = wrap->GetTextureHandle ();
        } else 
          var->GetValue(last_textures[i]);
      } else
        last_textures[i] = 0;
    }
    else
      last_textures[i] = 0;
  }
  g3d->SetTextureState (textureUnits, last_textures, 
    thispass->textureCount);
  lastTexturesCount = thispass->textureCount;

  modes = *mesh;
  if (thispass->alphaMode.autoAlphaMode)
  {
    iTextureHandle* tex = 0;
    if (thispass->autoAlphaTexRef != 0)
      thispass->autoAlphaTexRef->GetValue (tex);
    else if (thispass->alphaMode.autoModeTexture != csInvalidStringID)
    {
      if (thispass->alphaMode.autoModeTexture < (csStringID)stacks.Length ())
      {
        csShaderVariable* var = 0;
        if (stacks[thispass->alphaMode.autoModeTexture].Length ()>0)
          var = stacks[thispass->alphaMode.autoModeTexture].Top ();
        if (var)
          var->GetValue (tex);
      }
    }
    if (tex != 0)
      modes.alphaType = tex->GetAlphaType ();
    else
      modes.alphaType = csAlphaMode::alphaNone;
  }
  else
    modes.alphaType = thispass->alphaMode.alphaType;
  // Override mixmode, if requested
  if ((thispass->mixMode & CS_FX_MASK_MIXMODE) == CS_FX_MESH)
  {
    if ((modes.mixmode & CS_FX_MASK_MIXMODE) == CS_FX_ALPHA)
      modes.alphaType = csAlphaMode::alphaSmooth;
  }
  else
    modes.mixmode = thispass->mixMode;

  if(thispass->vp) thispass->vp->SetupState (mesh, stacks);
  if(thispass->fp) thispass->fp->SetupState (mesh, stacks);

  return true;
}

bool csXMLShaderTech::TeardownPass ()
{
  shaderPass *thispass = &passes[currentPass];

  if(thispass->vp) thispass->vp->ResetState ();
  if(thispass->fp) thispass->fp->ResetState ();

  return true;
}

int csXMLShaderTech::GetPassNumber (shaderPass* pass)
{
  if ((pass >= passes) && (pass < passes + passesCount))
  {
    return pass - passes;
  }
  else
    return -1;
}

void csXMLShaderTech::SetFailReason (const char* reason, ...)
{
  va_list args;
  va_start (args, reason);
  fail_reason.FormatV (reason, args);
  va_end (args);
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY (csXMLShaderCompiler)

SCF_IMPLEMENT_IBASE(csXMLShaderCompiler)
  SCF_IMPLEMENTS_INTERFACE(iComponent)
  SCF_IMPLEMENTS_INTERFACE(iShaderCompiler)
SCF_IMPLEMENT_IBASE_END

csXMLShaderCompiler::csXMLShaderCompiler(iBase* parent)
{
  SCF_CONSTRUCT_IBASE(parent);
  init_token_table (xmltokens);
}

csXMLShaderCompiler::~csXMLShaderCompiler()
{
  SCF_DESTRUCT_IBASE();
}

void csXMLShaderCompiler::Report (int severity, const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  csReportV (objectreg, severity, 
    "crystalspace.graphics3d.shadercompiler.xmlshader", msg, args);
  va_end (args);
}

bool csXMLShaderCompiler::Initialize (iObjectRegistry* object_reg)
{
  objectreg = object_reg;

  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (
      object_reg, iPluginManager);

  strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.shared.stringset", iStringSet);

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  if (!synldr)
  {
    synldr = CS_LOAD_PLUGIN (plugin_mgr,
      "crystalspace.syntax.loader.service.text", iSyntaxService);
    if (!synldr)
    {
      Report(CS_REPORTER_SEVERITY_ERROR, "Could not load the syntax service!");
      return false;
    }
    if (!object_reg->Register (synldr, "iSyntaxService"))
    {
      Report(CS_REPORTER_SEVERITY_ERROR, "Could not register the syntax service!");
      return false;
    }
  }

  csRef<iCommandLineParser> cmdline =
    CS_QUERY_REGISTRY (object_reg, iCommandLineParser);
  if (cmdline)
    do_verbose = (cmdline->GetOption ("verbose") != 0);
  else
    do_verbose = false;

  return true;
}

int csXMLShaderCompiler::CompareTechniqueKeeper (
  TechniqueKeeper const& t1, TechniqueKeeper const& t2)
{
  int v = t2.priority - t1.priority;
  if (v == 0) v = t2.tagPriority - t1.tagPriority;
  return v;
}

void csXMLShaderCompiler::ScanForTechniques (iDocumentNode* templ,
		csArray<TechniqueKeeper>& techniquesTmp,
		int forcepriority)
{
  csRef<iDocumentNodeIterator> it = templ->GetNodes();

  // Read in the techniques.
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () == CS_NODE_ELEMENT &&
      xmltokens.Request (child->GetValue ()) == XMLTOKEN_TECHNIQUE)
    {
      //save it
      unsigned int p = child->GetAttributeValueAsInt ("priority");
      if (forcepriority != -1 && int (p) != forcepriority) continue;
      TechniqueKeeper keeper (child, p);
      // Compute the tag's priorities.
      csRef<iDocumentNodeIterator> tagIt = child->GetNodes ("tag");
      while (tagIt->HasNext ())
      {
	csRef<iDocumentNode> tag = tagIt->Next ();
	csStringID tagID = strings->Request (tag->GetContentsValue ());

	csShaderTagPresence presence;
	int priority;
	shadermgr->GetTagOptions (tagID, presence, priority);
	if (presence == TagNeutral)
	{
	  keeper.tagPriority += priority;
	}
      }
      techniquesTmp.Push (keeper);
    }
  }

  techniquesTmp.Sort (&CompareTechniqueKeeper);
}

csPtr<iShader> csXMLShaderCompiler::CompileShader (iDocumentNode *templ,
		int forcepriority)
{
  if (!ValidateTemplate (templ))
    return 0;
  
  shadermgr = CS_QUERY_REGISTRY (objectreg, iShaderManager);
  CS_ASSERT (shadermgr); // Should be present - loads us, after all

  csArray<TechniqueKeeper> techniquesTmp;
  ScanForTechniques (templ, techniquesTmp, forcepriority);

  //now try to load them one in a time, until we are successful
  csRef<csXMLShader> shader;
  shader.AttachNew (new csXMLShader (this));
  shader->SetName (templ->GetAttributeValue ("name"));
  csArray<TechniqueKeeper>::Iterator techIt = techniquesTmp.GetIterator ();
  while (techIt.HasNext ())
  {
    TechniqueKeeper tk = techIt.Next();
    csXMLShaderTech* tech = new csXMLShaderTech (shader);
    if (tech->Load (tk.node, templ))
    {
      if (do_verbose)
	Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "Shader '%s': Technique with priority %d succeeds!",
	  shader->GetName(), tk.priority);
      shader->activeTech = tech;
      break;
    }
    else
    {
      if (do_verbose)
      {
	Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "Shader '%s': Technique with priority %d fails. Reason: %s.",
	  shader->GetName(), tk.priority, tech->GetFailReason());
      }
      delete tech;
    }
  }

  if (shader->activeTech == 0)
  {
    // @@@ Or a warning instead?
    Report (CS_REPORTER_SEVERITY_WARNING,
      "No technique validated for shader '%s'", shader->GetName());
    return 0;
  }

  csRef<iShader> ishader (shader);
  return csPtr<iShader> (ishader);
}

class csShaderPriorityList : public iShaderPriorityList
{
public:
  csArray<int> priorities;
  csShaderPriorityList ()
  {
    SCF_CONSTRUCT_IBASE (0);
  }
  virtual ~csShaderPriorityList ()
  {
    SCF_DESTRUCT_IBASE ();
  }

  SCF_DECLARE_IBASE;
  virtual int GetCount () const { return priorities.Length (); }
  virtual int GetPriority (int idx) const { return priorities[idx]; }
};

SCF_IMPLEMENT_IBASE (csShaderPriorityList)
  SCF_IMPLEMENTS_INTERFACE (iShaderPriorityList)
SCF_IMPLEMENT_IBASE_END

csPtr<iShaderPriorityList> csXMLShaderCompiler::GetPriorities (
	iDocumentNode* templ)
{
  csArray<TechniqueKeeper> techniquesTmp;
  ScanForTechniques (templ, techniquesTmp, -1);
  csShaderPriorityList* list = new csShaderPriorityList ();
  csArray<TechniqueKeeper>::Iterator techIt = techniquesTmp.GetIterator ();
  while (techIt.HasNext ())
  {
    TechniqueKeeper tk = techIt.Next();
    list->priorities.Push (tk.priority);
  }
  return csPtr<iShaderPriorityList> (list);
}

bool csXMLShaderCompiler::ValidateTemplate(iDocumentNode *templ)
{
  if (!IsTemplateToCompiler(templ))
    return false;

  /*@@TODO: Validation without accual compile. should check correct xml
  syntax, and that we have at least one techqniue which can load. Also check
  that we have valid texturemapping and buffermapping*/

  return true;
}

bool csXMLShaderCompiler::IsTemplateToCompiler(iDocumentNode *templ)
{
  //Check that we got an element
  if (templ->GetType() != CS_NODE_ELEMENT) return false;

  //With name shader  (<shader>....</shader>
  if (xmltokens.Request (templ->GetValue())!=XMLTOKEN_SHADER) return false;

  //Check the type-string in <shader>
  const char* shaderName = templ->GetAttributeValue ("name");
  const char* shaderType = templ->GetAttributeValue ("type");
  if ((shaderType == 0) || (xmltokens.Request (shaderType) != 
    XMLTOKEN_XMLSHADER))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, 
      "Type of shader '%s' is not 'xmlshader', but '%s'",
      shaderName, shaderType);
    return false;
  }

  //Check that we have children, no children == not a template to this one at
  //least.
  if (!templ->GetNodes()->HasNext()) return false;

  //Ok, passed check. We will try to validate it
  return true;
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_EXT(csXMLShader)
  SCF_IMPLEMENTS_INTERFACE(iShader)
SCF_IMPLEMENT_IBASE_EXT_END

csXMLShader::csXMLShader (csXMLShaderCompiler* compiler)
{
  activeTech = 0;
  csXMLShader::compiler = compiler;
  g3d = compiler->g3d;
}

csXMLShader::~csXMLShader ()
{
  delete activeTech;
}
