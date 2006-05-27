/*
  Copyright (C) 2003-2006 by Marten Svanfeldt
		2004-2006 by Frank Richter

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
#include <ctype.h>

#include "imap/services.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "ivideo/rendermesh.h"
#include "ivideo/material.h"
#include "ivideo/graph3d.h"

#include "csgfx/renderbuffer.h"

#include "shader.h"
#include "shadertech.h"
#include "xmlshader.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

CS_LEAKGUARD_IMPLEMENT (csXMLShaderTech);

//---------------------------------------------------------------------------

iRenderBuffer* csXMLShaderTech::last_buffers[shaderPass::STREAMMAX*2];
iRenderBuffer* csXMLShaderTech::clear_buffers[shaderPass::STREAMMAX*2];
size_t csXMLShaderTech::lastBufferCount;

iTextureHandle* csXMLShaderTech::last_textures[shaderPass::TEXTUREMAX];
iTextureHandle* csXMLShaderTech::clear_textures[shaderPass::TEXTUREMAX];
int csXMLShaderTech::textureUnits[shaderPass::TEXTUREMAX];
size_t csXMLShaderTech::lastTexturesCount;

csXMLShaderTech::csXMLShaderTech (csXMLShader* parent) : 
  passes(0), passesCount(0), currentPass((size_t)~0),
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

bool csXMLShaderTech::ParseInstanceBinds (iDocumentNode *node, shaderPass *pass)
{
  if (!node) return true;

  csRef<iDocumentNodeIterator> it = node->GetNodes (
    xmltokens.Request (csXMLShaderCompiler::XMLTOKEN_INSTANCEPARAMETER));

  iStringSet* strings = parent->compiler->strings;
  csRef<csShaderVariable> instance_template = parent->GetVariable (
    strings->Request ("instance_template"));
  if (!instance_template) 
  {
    if (do_verbose)
        SetFailReason ("can't find instances template");
    return false;
  }

  pass->pseudo_instancing = true;

  while (it->HasNext ())
  {
    csRef<iDocumentNode> ch_node = it->Next ();
    csRef<iDocumentNode> src_node = ch_node->GetNode (xmltokens.Request (
      csXMLShaderCompiler::XMLTOKEN_SOURCE));
    
    csStringID source_id = strings->Request (src_node->GetContentsValue ());
    csRef<csShaderVariable> source_variable;
    //check if we have such source
    for (size_t i = 0; i < instance_template->GetArraySize (); i++)
    {
      if (instance_template->GetArrayElement (i)->GetName () == source_id)
      {
        source_variable = instance_template->GetArrayElement (i);
        break;
      }
    }
    if (!source_variable) continue;

    size_t binds_cnt = 0;
    switch (source_variable->GetType ())
    {
    case csShaderVariable::COLOR:
    case csShaderVariable::VECTOR2:
    case csShaderVariable::VECTOR3:
    case csShaderVariable::VECTOR4:
      binds_cnt = 1;
      break;
    case csShaderVariable::MATRIX:
      binds_cnt = 3;
      break;
    case csShaderVariable::TRANSFORM:
      binds_cnt = 4;
      break;
    default:
      break;
    }

    csRef<iDocumentNodeIterator> dest_it = ch_node->GetNodes (
      xmltokens.Request (csXMLShaderCompiler::XMLTOKEN_DESTINATION));

    size_t first_id = pass->instances_binds.GetSize ();
    for (size_t i = 0; i < binds_cnt; i++)
      pass->instances_binds.Push (CS_VATTRIB_TEXCOORD0);

    size_t bind = 0;
    while (dest_it->HasNext ())
    {
      if (bind >= binds_cnt) break;
      size_t index = first_id + bind%binds_cnt;
      bind++;
      csRef<iDocumentNode> dest_nodes = dest_it->Next ();
      switch (xmltokens.Request (dest_nodes->GetContentsValue ()))
      {
      case csXMLShaderCompiler::XMLTOKEN_TEXCOORD0:
        pass->instances_binds[index] = CS_VATTRIB_TEXCOORD0;
        break;
      case csXMLShaderCompiler::XMLTOKEN_TEXCOORD1:
        pass->instances_binds[index] = CS_VATTRIB_TEXCOORD1;
        break;
      case csXMLShaderCompiler::XMLTOKEN_TEXCOORD2:
        pass->instances_binds[index] = CS_VATTRIB_TEXCOORD2;
        break;
      case csXMLShaderCompiler::XMLTOKEN_TEXCOORD3:
        pass->instances_binds[index] = CS_VATTRIB_TEXCOORD3;
        break;
      case csXMLShaderCompiler::XMLTOKEN_TEXCOORD4:
        pass->instances_binds[index] = CS_VATTRIB_TEXCOORD4;
        break;
      default:
        break;
      }
    }
  }
  return true;
}

bool csXMLShaderTech::LoadPass (iDocumentNode *node, shaderPass *pass, 
                                size_t variant)
{
  iSyntaxService* synldr = parent->compiler->synldr;
  iStringSet* strings = parent->compiler->strings;

  pass->pseudo_instancing = false;
  //Load pseudo-instancing binds
  csRef<iDocumentNode> pi_node = node->GetNode (
    xmltokens.Request (csXMLShaderCompiler::XMLTOKEN_PSEUDOINSTANCING));
  if (!ParseInstanceBinds (pi_node, pass))
    return false;

  //Load shadervar block
  csRef<iDocumentNode> varNode = node->GetNode(
    xmltokens.Request (csXMLShaderCompiler::XMLTOKEN_SHADERVARS));
 
  if (varNode)
    parent->compiler->LoadSVBlock (varNode, &pass->svcontext);

  csRef<iDocumentNode> programNode;
  csRef<iShaderProgram> program;
  // load fp
  /* This is done before VP loading b/c the VP could query for TU bindings
   * which are currently handled by the FP. */
  programNode = node->GetNode (xmltokens.Request (
    csXMLShaderCompiler::XMLTOKEN_FP));

  if (programNode)
  {
    program = LoadProgram (0, programNode, pass, variant);
    if (program)
      pass->fp = program;
    else
    {
      if (do_verbose)
        SetFailReason ("fragment program failed to load");
      return false;
    }
  }

  csRef<iShaderDestinationResolver> resolveFP;
  if (pass->fp) 
    resolveFP = scfQueryInterface<iShaderDestinationResolver> (pass->fp);

  //load vp
  programNode = node->GetNode(xmltokens.Request (
    csXMLShaderCompiler::XMLTOKEN_VP));

  if (programNode)
  {
    program = LoadProgram (resolveFP, programNode, pass, variant);
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

  csRef<iShaderDestinationResolver> resolveVP;
  if (pass->vp) 
    resolveVP = scfQueryInterface<iShaderDestinationResolver> (pass->vp);

  //load vproc
  programNode = node->GetNode(xmltokens.Request (
    csXMLShaderCompiler::XMLTOKEN_VPROC));

  if (programNode)
  {
    program = LoadProgram (resolveFP, programNode, pass, variant);
    if (program)
    {
      pass->vproc = program;
    }
    else
    {
      if (do_verbose)
        SetFailReason ("vertex preprocessor failed to load");
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

    csRef<iDocumentNode> nodeFlipCulling = node->GetNode ("flipculling");
    if (nodeFlipCulling)
    {
      synldr->ParseBool(nodeFlipCulling, pass->flipCulling, false);
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
      csString str;
      str.Format ("attribute %d", i);
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

	  int texUnit = 
	    resolveFP ? resolveFP->ResolveTU (target) : -1;
	  if (texUnit >= 0)
	  {
	    attrib = (csVertexAttrib)((int)CS_VATTRIB_TEXCOORD0 + texUnit);
	    found = true;
	  }
	  else
	  {
	    char dummy;
	    if (sscanf (target, "%d%c", &texUnit, &dummy) == 1)
	    {
	      attrib = (csVertexAttrib)((int)CS_VATTRIB_TEXCOORD0 + texUnit);
	      found = true;
	    }
	  }
	}
	else
	{
	  csVertexAttrib attr = 
	    resolveVP ? resolveVP->ResolveBufferDestination (dest) : 
	    CS_VATTRIB_INVALID;
	  if (attr != CS_VATTRIB_INVALID)
	  {
	    attrib = attr;
	    found = true;
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
      
      // The user has explicitly asked for a "none" mapping
      const bool explicitlyUnmapped = (strcasecmp (source, "none") == 0);
      if (((sourceName == CS_BUFFER_NONE) || (cname != 0)) 
	&& !explicitlyUnmapped)
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
        pass->custommapping_attrib.Push (attrib);
	pass->custommapping_buffer.Push (CS_BUFFER_NONE);
      }
      else
      {
        //default mapping
	if ((sourceName < CS_BUFFER_POSITION) && !explicitlyUnmapped)
        {
          SetFailReason ("invalid buffermapping, '%s' not allowed here.",
	    source);
          return false;
        }
        
	if ((attrib >= CS_VATTRIB_SPECIFIC_FIRST)
	  && (attrib <= CS_VATTRIB_SPECIFIC_LAST))
	  pass->defaultMappings[attrib] = sourceName;
	else
	{
	  pass->custommapping_variables.Push (0);
	  pass->custommapping_attrib.Push (attrib);
	  pass->custommapping_buffer.Push (sourceName);
	  /* Those buffers are mapped by default to some specific vattribs; 
	   * since they are now to be mapped to some generic vattrib,
	   * turn off the default map. */
	  if (sourceName == CS_BUFFER_POSITION)
	    pass->defaultMappings[CS_VATTRIB_POSITION] = CS_BUFFER_NONE;
	}
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
      int texUnit = resolveFP ? resolveFP->ResolveTU (dest) : -1;
      if (texUnit < 0)
      {
	if (csStrNCaseCmp (dest, "unit ", 5) == 0)
	{
	  dest += 5;
	  char dummy;
	  if (sscanf (dest, "%d%c", &texUnit, &dummy) != 1)
	    texUnit = -1;
	}
      }

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

csPtr<iShaderProgram> csXMLShaderTech::LoadProgram (
  iShaderDestinationResolver* resolve, iDocumentNode* node, shaderPass* /*pass*/,
  size_t variant)
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
  csRef<iShaderProgramPlugin> plg;
  plg = csLoadPluginCheck<iShaderProgramPlugin> (parent->compiler->objectreg,
  	plugin, false);
  if(!plg)
  {
    if (parent->compiler->do_verbose)
      parent->compiler->Report (CS_REPORTER_SEVERITY_WARNING,
	  "Couldn't retrieve shader plugin '%s' for <%s> in shader '%s'",
	  plugin, node->GetValue (), parent->GetName ());
    delete[] plugin;
    return 0;
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
    programNode = parent->LoadProgramFile (node->GetAttributeValue ("file"), 
      variant);
  else
    programNode = node;
  if (!program->Load (resolve, programNode))
    return 0;

  if (!program->Compile ())
    return 0;

  return csPtr<iShaderProgram> (program);
}

bool csXMLShaderTech::Load (iDocumentNode* node, iDocumentNode* parentSV, 
                            size_t variant)
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

  // copy over metadata from parent
  metadata.description = csStrNew (parent->allShaderMeta.description);
  metadata.numberOfLights = node->GetAttributeValueAsInt ("lights");

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
    if (!LoadPass (passNode, &passes[currentPassNr++], variant))
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
  if(thispass->vproc) thispass->vproc->Activate ();
  if(thispass->vp) thispass->vp->Activate ();
  if(thispass->fp) thispass->fp->Activate ();
  
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
  currentPass = (size_t)~0;

  if(thispass->vproc) thispass->vproc->Deactivate ();
  if(thispass->vp) thispass->vp->Deactivate ();
  if(thispass->fp) thispass->fp->Deactivate ();

  iGraphics3D* g3d = parent->g3d;
/*  g3d->SetBufferState(thispass->vertexattributes, clear_buffers, 
    lastBufferCount);*/
  g3d->DeactivateBuffers (thispass->custommapping_attrib.GetArray (), 
    (int)lastBufferCount);
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

  //first run the preprocessor
  if(thispass->vproc) thispass->vproc->SetupState (mesh, modes, stacks);

  //now map our buffers. all refs should be set
  size_t i;
  for (i = 0; i < thispass->custommapping_attrib.Length (); i++)
  {
    if (thispass->custommapping_buffer[i] != CS_BUFFER_NONE)
    {
      last_buffers[i] = modes.buffers->GetRenderBuffer (
	thispass->custommapping_buffer[i]);
    }
    else if ((thispass->custommapping_variables.Length () >= i) 
      && (thispass->custommapping_variables[i] != 0))
      thispass->custommapping_variables[i]->GetValue(last_buffers[i]);
    else if (thispass->custommapping_id[i] < (csStringID)stacks.Length ())
    {
      csShaderVariable* var = 0;
      var = csGetShaderVariableFromStack (stacks, thispass->custommapping_id[i]);
      if (var)
        var->GetValue(last_buffers[i]);
      else
        last_buffers[i] = 0;
    }
    else
      last_buffers[i] = 0;
  }
  g3d->ActivateBuffers (modes.buffers, thispass->defaultMappings);
  g3d->ActivateBuffers (thispass->custommapping_attrib.GetArray (), 
    last_buffers, (uint)thispass->custommapping_attrib.Length ());
  lastBufferCount = thispass->custommapping_attrib.Length ();
  
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
      var = csGetShaderVariableFromStack (stacks, thispass->textureID[j]);
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
        var = csGetShaderVariableFromStack (stacks, thispass->alphaMode.autoModeTexture);
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
  if ((thispass->mixMode & CS_MIXMODE_TYPE_MASK) != CS_FX_MESH)
    modes.mixmode = thispass->mixMode;

  modes.flipCulling = thispass->flipCulling;

  if(thispass->vp) thispass->vp->SetupState (mesh, modes, stacks);
  if(thispass->fp) thispass->fp->SetupState (mesh, modes, stacks);

  //pseudo instancing setup
  SetupInstances (modes, thispass);

  return true;
}

void csXMLShaderTech::SetupInstances (csRenderMeshModes& modes, shaderPass *thispass)
{
  modes.supports_pseudoinstancing = thispass->pseudo_instancing;
  modes.instances_binds = thispass->instances_binds;
}

bool csXMLShaderTech::TeardownPass ()
{
  shaderPass *thispass = &passes[currentPass];

  if(thispass->vproc) thispass->vproc->ResetState ();
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

}
CS_PLUGIN_NAMESPACE_END(XMLShader)
