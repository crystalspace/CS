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
#include "iutil/verbositymanager.h"
#include "imap/services.h"
#include "ivaria/keyval.h"
#include "ivaria/reporter.h"
#include "ivideo/rendermesh.h"
#include "csutil/util.h"
#include "csutil/scfstr.h"
#include "csutil/scfstrset.h"
#include "csutil/xmltiny.h"
#include "csgfx/renderbuffer.h"

#include "xmlshader.h"

CS_LEAKGUARD_IMPLEMENT (csXMLShaderTech);
CS_LEAKGUARD_IMPLEMENT (csXMLShader);
CS_LEAKGUARD_IMPLEMENT (csXMLShaderCompiler);

CS_IMPLEMENT_PLUGIN

//---------------------------------------------------------------------------

iRenderBuffer* csXMLShaderTech::last_buffers[shaderPass::STREAMMAX*2];
iRenderBuffer* csXMLShaderTech::clear_buffers[shaderPass::STREAMMAX*2];
size_t csXMLShaderTech::lastBufferCount;

iTextureHandle* csXMLShaderTech::last_textures[shaderPass::TEXTUREMAX];
iTextureHandle* csXMLShaderTech::clear_textures[shaderPass::TEXTUREMAX];
int csXMLShaderTech::textureUnits[shaderPass::TEXTUREMAX];
size_t csXMLShaderTech::lastTexturesCount;

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
    parent->compiler->LoadSVBlock (varNode, &pass->svcontext);

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
      const char* cname = mapping->GetAttributeValue("customsource");
      const char *source = mapping->GetAttributeValue ("source");
      if ((source == 0) && (cname == 0))
      {
        SetFailReason ("invalid buffermapping, source missing.");
        return false;
      }
      csRenderBufferName sourceName = 
	csRenderBuffer::GetBufferNameFromDescr (source);
      if ((sourceName == CS_BUFFER_NONE) || (cname != 0)) 
      {
        //custom name
	if (cname == 0)
	  cname = source;

        csStringID varID = strings->Request (cname);
        pass->custommapping_id.Push (varID);
        //pass->bufferGeneric[pass->bufferCount] = CS_VATTRIB_IS_GENERIC (attrib);

        csShaderVariable *varRef=0;
        varRef = pass->svcontext.GetVariable(varID);

        if(!varRef)
          varRef = svcontext.GetVariable (varID);

        pass->custommapping_variables.Push (varRef);
        pass->custommaping_attrib.Push (attrib);
      }
      else
      {
        //default mapping
	if (sourceName < CS_BUFFER_POSITION)
        {
          SetFailReason ("invalid buffermapping, '%s' not allowed here.",
	    source);
          return false;
        }
        
        pass->defaultMappings[attrib] = sourceName;
      }
    }
    else
    {
      parent->compiler->Report (CS_REPORTER_SEVERITY_WARNING,
	"Shader '%s', pass %d: invalid buffer destination '%s'",
	parent->GetName (), GetPassNumber (pass), dest);
    }
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

bool csXMLShaderCompiler::LoadSVBlock (iDocumentNode *node,
  iShaderVariableContext *context)
{
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
    programNode = parent->LoadProgramFile (node->GetAttributeValue ("file"));
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
  iShaderManager* shadermgr = parent->shadermgr;

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
      parent->compiler->LoadSVBlock (varNode, &svcontext);
  }

  csRef<iDocumentNode> varNode = node->GetNode(
    xmltokens.Request (csXMLShaderCompiler::XMLTOKEN_SHADERVARS));

  if (varNode)
    parent->compiler->LoadSVBlock (varNode, &svcontext);

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

bool csXMLShaderTech::ActivatePass (size_t number)
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
/*  g3d->SetBufferState(thispass->vertexattributes, clear_buffers, 
    lastBufferCount);*/
  g3d->DeactivateBuffers (thispass->custommaping_attrib.GetArray (), (int)lastBufferCount);
  lastBufferCount=0;

  g3d->SetTextureState(textureUnits, clear_textures, 
    (int)lastTexturesCount);
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
  size_t i;
  for (i = 0; i < thispass->custommaping_attrib.Length (); i++)
  {
    if (thispass->custommapping_variables.Length () >= i && thispass->custommapping_variables[i] != 0)
      thispass->custommapping_variables[i]->GetValue(last_buffers[i]);
    else if (thispass->custommapping_id[i] < (csStringID)stacks.Length ())
    {
      csShaderVariable* var = 0;
      if (stacks[thispass->custommapping_id[i]].Length () > 0)
        var = stacks[thispass->custommapping_id[i]].Top ();
      if (var)
        var->GetValue(last_buffers[i]);
      else
        last_buffers[i] = 0;
    }
    else
      last_buffers[i] = 0;
  }
  g3d->ActivateBuffers (mesh->buffers, thispass->defaultMappings);
  g3d->ActivateBuffers (thispass->custommaping_attrib.GetArray (), last_buffers, 
    (uint)thispass->custommaping_attrib.Length ());
  lastBufferCount = thispass->custommaping_attrib.Length ();
  
  //and the textures
  int j;
  for (j = 0; j < thispass->textureCount; j++)
  {
    if (thispass->textureRef[j] != 0)
    {
      iTextureWrapper* wrap;
      thispass->textureRef[j]->GetValue(wrap);
      if (wrap)
      {
        wrap->Visit ();
        last_textures[j] = wrap->GetTextureHandle ();
      }
      else
        last_textures[j] = 0;
    }
    else if (thispass->textureID[j] < (csStringID)stacks.Length ())
    {
      csShaderVariable* var = 0;
      if (stacks[thispass->textureID[j]].Length () > 0)
        var = stacks[thispass->textureID[j]].Top ();
      if (var)
      {
        iTextureWrapper* wrap;
        var->GetValue(wrap);
        if (wrap) 
        {
          wrap->Visit ();
          last_textures[j] = wrap->GetTextureHandle ();
        } else 
          var->GetValue(last_textures[j]);
      } else
        last_textures[j] = 0;
    }
    else
      last_textures[j] = 0;
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

csShaderConditionResolver::csShaderConditionResolver (
  csXMLShaderCompiler* compiler) : evaluator (compiler->strings)
{
  rootNode = 0;
  nextVariant = 0;
  SetEvalParams (0, 0);
}

csShaderConditionResolver::~csShaderConditionResolver ()
{
}

const char* csShaderConditionResolver::SetLastError (const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  lastError.FormatV (msg, args);
  va_end (args);
  return lastError;
}

const char* csShaderConditionResolver::ParseCondition (const char* str, 
						       size_t len, 
						       csConditionID& result)
{
  csExpressionTokenList tokens;
  const char* err = tokenizer.Tokenize (str, len, tokens);
  if (err)
    return SetLastError ("Tokenization: %s", err);
  csExpression* newExpression;
  err = parser.Parse (tokens, newExpression);
  if (err)
  {
    delete newExpression;
    return SetLastError ("Parsing: %s", err);
  }

  err = evaluator.ProcessExpression (newExpression, result);
  delete newExpression;
  if (err)
    return SetLastError ("Processing: %s", err);

  return 0;
}

bool csShaderConditionResolver::Evaluate (csConditionID condition)
{
  const csRenderMeshModes* modes = csShaderConditionResolver::modes;
  const csShaderVarStack* stacks = csShaderConditionResolver::stacks;

  return evaluator.Evaluate (condition, modes ? *modes : csRenderMeshModes(),
    stacks ? *stacks : csShaderVarStack ());
}

csConditionNode* csShaderConditionResolver::NewNode ()
{
  csConditionNode* newNode = new csConditionNode;
  condNodes.Push (newNode);
  return newNode;
}

csConditionNode* csShaderConditionResolver::GetRoot ()
{
  if (rootNode == 0)
    rootNode = NewNode ();
  return rootNode;
}

void csShaderConditionResolver::AddToRealNode (csRealConditionNode* realNode, 
					       csConditionID condition, 
					       csConditionNode* trueNode, 
					       csConditionNode* falseNode)
{
  if (realNode->variant != csArrayItemNotFound)
  {
    /* There's a variant assigned, (!= csArrayItemNotFound)
       un-assign variant but assign condition */
    csRef<csRealConditionNode> realTrueNode;
    realTrueNode.AttachNew (new csRealConditionNode);
    realNode->trueNode = realTrueNode;
    trueNode = NewNode ();
    realTrueNode->variant = realNode->variant;
    trueNode->nodes.Push (realTrueNode);

    csRef<csRealConditionNode> realFalseNode;
    realFalseNode.AttachNew (new csRealConditionNode);
    realNode->falseNode = realFalseNode;
    falseNode = NewNode ();
    realFalseNode->variant = nextVariant++;
    falseNode->nodes.Push (realFalseNode);

    realNode->condition = condition;
    realNode->variant = csArrayItemNotFound;
  }
  else
  {
    /* There's no variant assigned, recursively add condition
      to T&F children */
    if (realNode->condition == condition)
    {
      trueNode->nodes.Push (realNode->trueNode);
      falseNode->nodes.Push (realNode->falseNode);
      return;
    }
    AddToRealNode (realNode->trueNode, condition, trueNode, falseNode);
    AddToRealNode (realNode->falseNode, condition, trueNode, falseNode);
  }
}

void csShaderConditionResolver::AddNode (csConditionNode* parent,
					 csConditionID condition, 
					 csConditionNode*& trueNode,
					 csConditionNode*& falseNode)
{
  if (rootNode == 0)
  {
    // This is the first condition, new node gets root
    CS_ASSERT_MSG ("No root but parent? Weird.", parent == 0);
    parent = GetRoot ();

    csRef<csRealConditionNode> realNode;
    realNode.AttachNew (new csRealConditionNode);
    realNode->condition = condition;
    parent->nodes.Push (realNode);

    csRef<csRealConditionNode> realTrueNode;
    realTrueNode.AttachNew (new csRealConditionNode);
    realNode->trueNode = realTrueNode;
    realTrueNode->variant = nextVariant++;
    trueNode = NewNode ();
    trueNode->nodes.Push (realTrueNode);

    csRef<csRealConditionNode> realFalseNode;
    realFalseNode.AttachNew (new csRealConditionNode);
    realNode->falseNode = realFalseNode;
    realFalseNode->variant = nextVariant++;
    falseNode = NewNode ();
    falseNode->nodes.Push (realFalseNode);
  }
  else
  {
    if (parent == 0)
      parent = GetRoot ();

    trueNode = NewNode ();
    falseNode = NewNode ();

    const size_t n = parent->nodes.Length ();
    for (size_t i = 0; i < n; i++)
    {
      AddToRealNode (parent->nodes[i], condition, 
	trueNode, falseNode);
    }
  }
}

void csShaderConditionResolver::SetEvalParams (const csRenderMeshModes* modes,
					       const csShaderVarStack* stacks)
{
  csShaderConditionResolver::modes = modes;
  csShaderConditionResolver::stacks = stacks;
}

size_t csShaderConditionResolver::GetVariant ()
{
  const csRenderMeshModes& modes = *csShaderConditionResolver::modes;
  const csShaderVarStack& stacks = *csShaderConditionResolver::stacks;

  if (rootNode == 0)
  {
    return 0;
  }
  else
  {
    CS_ASSERT (rootNode->nodes.Length () == 1);

    csRealConditionNode* currentRoot = 0;
    csRealConditionNode* nextRoot = rootNode->nodes[0];

    while (nextRoot != 0)
    {
      currentRoot = nextRoot;
      if (evaluator.Evaluate (currentRoot->condition, modes, stacks))
      {
	nextRoot = currentRoot->trueNode;
      }
      else
      {
	nextRoot = currentRoot->falseNode;
      }
    }
    CS_ASSERT (currentRoot != 0);
    return currentRoot->variant;
  }
}

void csShaderConditionResolver::DumpConditionTree ()
{
  if (rootNode == 0)
    return;

  CS_ASSERT (rootNode->nodes.Length () == 1);
  DumpConditionNode (rootNode->nodes[0], 0);
}

static void Indent (int n)
{
  for (int i = 0; i < n; i++)
    csPrintf (" ");
}

void csShaderConditionResolver::DumpConditionNode (csRealConditionNode* node, 
						   int level)
{
  if (node == 0)
  {
    Indent (level);
    csPrintf ("<none>\n");
  }
  else
  {
    Indent (level);
    if (node->variant != csArrayItemNotFound)
      csPrintf ("variant: %lu\n", (unsigned long)node->variant);
    else
    {
      csPrintf ("condition: %lu\n", (unsigned long)node->condition);
      Indent (level);
      csPrintf ("True node: ");
      DumpConditionNode (node->trueNode, level + 1);
      Indent (level);
      csPrintf ("False node: ");
      DumpConditionNode (node->falseNode, level + 1);
    }
  }
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
  wrapperFact = 0;
  InitTokenTable (xmltokens);
}

csXMLShaderCompiler::~csXMLShaderCompiler()
{
  delete wrapperFact;
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

  wrapperFact = new csWrappedDocumentNodeFactory (objectreg);

  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (
      object_reg, iPluginManager);

  strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.shared.stringset", iStringSet);

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
  
  CS_QUERY_REGISTRY_PLUGIN(synldr, object_reg,
    "crystalspace.syntax.loader.service.text", iSyntaxService);
  if (!synldr)
    return false;

  csRef<iVerbosityManager> verbosemgr (
    CS_QUERY_REGISTRY (object_reg, iVerbosityManager));
  if (verbosemgr) 
    do_verbose = verbosemgr->CheckFlag ("renderer", "shader");
  else
    do_verbose = false;
    
  return true;
}

csPtr<iShader> csXMLShaderCompiler::CompileShader (iDocumentNode *templ,
		int forcepriority)
{
  if (!ValidateTemplate (templ))
    return 0;
  
  // Create a shader. The actual loading happens later.
  csRef<csXMLShader> shader;
  shader.AttachNew (new csXMLShader (this, templ, forcepriority));
  shader->SetName (templ->GetAttributeValue ("name"));
  if (do_verbose)
  {
    csString str;
    shader->DumpStats (str);
    Report(CS_REPORTER_SEVERITY_NOTIFY, 
      "Shader %s: %s", shader->GetName (), str.GetData ());
  }

  csRef<iDocumentNodeIterator> tagIt = templ->GetNodes ("key");
  while (tagIt->HasNext ())
  {
    iKeyValuePair *keyvalue = 0;
    synldr->ParseKey (tagIt->Next (), keyvalue);
    if (keyvalue)
      shader->QueryObject ()->ObjAdd (keyvalue->QueryObject ());
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
  virtual size_t GetCount () const { return priorities.Length (); }
  virtual int GetPriority (size_t idx) const { return priorities[idx]; }
};

SCF_IMPLEMENT_IBASE (csShaderPriorityList)
  SCF_IMPLEMENTS_INTERFACE (iShaderPriorityList)
SCF_IMPLEMENT_IBASE_END

csPtr<iShaderPriorityList> csXMLShaderCompiler::GetPriorities (
	iDocumentNode* templ)
{
  csRef<iShaderManager> shadermgr = 
    CS_QUERY_REGISTRY (objectreg, iShaderManager);
  CS_ASSERT (shadermgr); // Should be present - loads us, after all

  csShaderPriorityList* list = new csShaderPriorityList ();

  /* @@@ A bit awkward, almost the same code as in 
     csXMLShader::ScanForTechniques */
  csRef<iDocumentNodeIterator> it = templ->GetNodes();

  // Read in the techniques.
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () == CS_NODE_ELEMENT &&
      xmltokens.Request (child->GetValue ()) == XMLTOKEN_TECHNIQUE)
    {
      //save it
      int p = child->GetAttributeValueAsInt ("priority");
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
	  p += priority;
	}
      }
      list->priorities.InsertSorted (p);
    }
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

//#define DUMP_SHADER_COND_TREE

csXMLShader::csXMLShader (csXMLShaderCompiler* compiler, 
			  iDocumentNode* source,
			  int forcepriority)
{
  InitTokenTable (xmltokens);

  activeTech = 0;
  filename = 0;
  csXMLShader::compiler = compiler;
  g3d = compiler->g3d;
  csXMLShader::forcepriority = forcepriority;

  shadermgr = CS_QUERY_REGISTRY (compiler->objectreg, iShaderManager);
  CS_ASSERT (shadermgr); // Should be present - loads us, after all

  resolver = new csShaderConditionResolver (compiler);
  csRef<iDocumentNode> wrappedNode;
  wrappedNode.AttachNew (compiler->wrapperFact->CreateWrapper (source, 
    resolver));
#ifdef DUMP_SHADER_COND_TREE
  csPrintf ("Condition tree for %s: ", source->GetAttributeValue ("name"));
  resolver->DumpConditionTree ();
#endif
  shaderSource = wrappedNode;
  vfsStartDir = csStrNew (compiler->vfs->GetCwd ());

  ParseGlobalSVs ();
}

csXMLShader::~csXMLShader ()
{
  for (size_t i = 0; i < variants.Length(); i++)
  {
    delete variants[i].tech;
  }

  delete[] filename;
  delete resolver;
  delete[] vfsStartDir;
}

int csXMLShader::CompareTechniqueKeeper (
  TechniqueKeeper const& t1, TechniqueKeeper const& t2)
{
  int v = t2.priority - t1.priority;
  if (v == 0) v = t2.tagPriority - t1.tagPriority;
  return v;
}

void csXMLShader::ScanForTechniques (iDocumentNode* templ,
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
      int p = child->GetAttributeValueAsInt ("priority");
      if ((forcepriority != -1) && (p != forcepriority)) continue;
      TechniqueKeeper keeper (child, p);
      // Compute the tag's priorities.
      csRef<iDocumentNodeIterator> tagIt = child->GetNodes ("tag");
      while (tagIt->HasNext ())
      {
	csRef<iDocumentNode> tag = tagIt->Next ();
	csStringID tagID = compiler->strings->Request (tag->GetContentsValue ());

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

/**
 * A wrapped class to have a csShaderVarStack synced to an
 * iShaderVariableContext*.
 */
class SVCWrapper : public iShaderVariableContext
{
  csShaderVariableContext& wrappedSVC;
public:
  csShaderVarStack svStack;

  SCF_DECLARE_IBASE;
  SVCWrapper (csShaderVariableContext& wrappedSVC) : wrappedSVC (wrappedSVC)
  {
    SCF_CONSTRUCT_IBASE(0);
    wrappedSVC.PushVariables (svStack);
  }
  virtual ~SVCWrapper ()
  {
    wrappedSVC.PopVariables (svStack);
    SCF_DESTRUCT_IBASE();
  }

  virtual void AddVariable (csShaderVariable *variable)
  {
    wrappedSVC.PopVariables (svStack);
    wrappedSVC.AddVariable (variable);
    wrappedSVC.PushVariables (svStack);
  }
  virtual csShaderVariable* GetVariable (csStringID name) const
  { return wrappedSVC.GetVariable (name); }
  virtual const csRefArray<csShaderVariable>& GetShaderVariables () const
  { return wrappedSVC.GetShaderVariables (); }
  virtual void PushVariables (csShaderVarStack &stacks) const
  { wrappedSVC.PushVariables (stacks); }
  virtual void PopVariables (csShaderVarStack &stacks) const
  { wrappedSVC.PopVariables (stacks); }
  virtual bool IsEmpty() const
  { return wrappedSVC.IsEmpty(); }
};

SCF_IMPLEMENT_IBASE(SVCWrapper)
  SCF_IMPLEMENTS_INTERFACE(iShaderVariableContext)
SCF_IMPLEMENT_IBASE_END

void csXMLShader::ParseGlobalSVs ()
{
  SVCWrapper wrapper (globalSVContext);
  resolver->SetEvalParams (0, &wrapper.svStack);
  compiler->LoadSVBlock (shaderSource, &wrapper);
  resolver->SetEvalParams (0, 0);
}

size_t csXMLShader::GetTicket (const csRenderMeshModes& modes, 
			       const csShaderVarStack& stacks)
{
  resolver->SetEvalParams (&modes, &stacks);
  size_t vi = resolver->GetVariant (/*modes, stacks*/);

  if (vi != csArrayItemNotFound)
  {
    ShaderVariant& var = variants.GetExtend (vi);

    if (!var.prepared)
    {
      // So external files are found correctly
      compiler->vfs->PushDir ();
      compiler->vfs->ChDir (vfsStartDir);

      csArray<TechniqueKeeper> techniquesTmp;
      ScanForTechniques (shaderSource, techniquesTmp, forcepriority);
		      
      csArray<TechniqueKeeper>::Iterator techIt = techniquesTmp.GetIterator ();
      while (techIt.HasNext ())
      {
	TechniqueKeeper tk = techIt.Next();
	csXMLShaderTech* tech = new csXMLShaderTech (this);
	if (tech->Load (tk.node, shaderSource))
	{
	  if (compiler->do_verbose)
	    compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
	      "Shader '%s': Technique with priority %d succeeds!",
	      GetName(), tk.priority);
	  var.tech = tech;
	  break;
	}
	else
	{
	  if (compiler->do_verbose)
	  {
	    compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
	      "Shader '%s': Technique with priority %d fails. Reason: %s.",
	      GetName(), tk.priority, tech->GetFailReason());
	  }
	  delete tech;
	}
      }
      compiler->vfs->PopDir ();

      if (var.tech == 0)
      {
	// @@@ Or a warning instead?
	compiler->Report (CS_REPORTER_SEVERITY_WARNING,
	  "No technique validated for shader '%s'", GetName());
      }

      var.prepared = true;
    }
  }
  resolver->SetEvalParams (0, 0);
  return vi;
}

bool csXMLShader::ActivatePass (size_t ticket, size_t number)
{ 
  CS_ASSERT_MSG ("ActivatePass() has already been called.",
    activeTech == 0);
  activeTech = (ticket != csArrayItemNotFound) ? variants[ticket].tech :
    0;
  return activeTech ? activeTech->ActivatePass (number) : false;
}

bool csXMLShader::DeactivatePass (size_t ticket)
{ 
  bool ret = activeTech ? activeTech->DeactivatePass() : false; 
  activeTech = 0;
  return ret;
}

void csXMLShader::DumpStats (csString& str)
{
  if (resolver->GetVariantCount () == 0)
    str.Replace ("unvarying");
  else
    str.Format ("%lu variations", (unsigned long)resolver->GetVariantCount ());
}

csRef<iDocumentNode> csXMLShader::LoadProgramFile (const char* filename)
{
  csRef<iVFS> VFS = CS_QUERY_REGISTRY (compiler->objectreg, 
    iVFS);
  csRef<iFile> programFile = VFS->Open (filename, VFS_FILE_READ);
  if (!programFile)
  {
    compiler->Report (CS_REPORTER_SEVERITY_ERROR,
      "Unable to open shader program file '%s'", filename);
    return 0;
  }
  csRef<iDocumentSystem> docsys (
    CS_QUERY_REGISTRY (compiler->objectreg, iDocumentSystem));
  if (docsys == 0)
    docsys.AttachNew (new csTinyDocumentSystem ());

  csRef<iDocument> programDoc = docsys->CreateDocument ();
  const char* err = programDoc->Parse (programFile, true);
  if (err != 0)
  {
    compiler->Report (CS_REPORTER_SEVERITY_ERROR,
      "Unable to parse shader program file '%s': %s", filename, err);
    return 0;
  }
  csRef<iDocumentNode> programNode;
  programNode.AttachNew (compiler->wrapperFact->CreateWrapper (
    programDoc->GetRoot (), resolver));
  return programNode;
}
