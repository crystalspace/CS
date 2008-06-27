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

#include "csgfx/renderbuffer.h"

#include "shader.h"
#include "shadertech.h"
#include "xmlshader.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

CS_LEAKGUARD_IMPLEMENT (csXMLShaderTech);

//---------------------------------------------------------------------------

csXMLShaderTech::csXMLShaderTech (csXMLShader* parent) : 
  passes(0), passesCount(0), currentPass((size_t)~0),
  xmltokens (parent->compiler->xmltokens)
{
  csXMLShaderTech::parent = parent;

  do_verbose = parent->compiler->do_verbose;
}

csXMLShaderTech::~csXMLShaderTech()
{
  delete[] passes;
}

static inline bool IsDestalphaMixmode (uint mode)
{
  return (mode == CS_FX_DESTALPHAADD);
}

bool csXMLShaderTech::LoadPass (iDocumentNode *node, ShaderPass* pass, 
                                size_t variant)
{
  iSyntaxService* synldr = parent->compiler->synldr;
  iStringSet* strings = parent->compiler->strings;
  iShaderVarStringSet* stringsSvName = parent->compiler->stringsSvName;

  //Load shadervar block
  csRef<iDocumentNode> varNode = node->GetNode(
    xmltokens.Request (csXMLShaderCompiler::XMLTOKEN_SHADERVARS));
 
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

    csRef<iDocumentNode> nodeZOffset = node->GetNode ("zoffset");
    if (nodeZOffset)
    {
      synldr->ParseBool (nodeZOffset, pass->zoffset, false);
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

  //if we got this far, load buffermappings
  csRef<iDocumentNodeIterator> it;
  it = node->GetNodes (xmltokens.Request (csXMLShaderCompiler::XMLTOKEN_BUFFER));
  while(it->HasNext ())
  {
    csRef<iDocumentNode> mapping = it->Next ();
    if (mapping->GetType () != CS_NODE_ELEMENT) continue;
    
    const char* dest = mapping->GetAttributeValue ("destination");
    csVertexAttrib attrib = CS_VATTRIB_INVALID;
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
	  attrib = resolveVP ? resolveVP->ResolveBufferDestination (dest) : 
	    CS_VATTRIB_INVALID;
          found = (attrib > CS_VATTRIB_INVALID);
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

        CS::ShaderVarStringID varID = stringsSvName->Request (cname);
        pass->custommapping_id.Push (varID);
        //pass->bufferGeneric[pass->bufferCount] = CS_VATTRIB_IS_GENERIC (attrib);

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
	  pass->custommapping_attrib.Push (attrib);
	  pass->custommapping_buffer.Push (sourceName);
          pass->custommapping_id.Push (CS::InvalidShaderVarStringID);
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
      if (attrib == CS_VATTRIB_INVALID)
      {
        parent->compiler->Report (CS_REPORTER_SEVERITY_WARNING,
	  "Shader '%s', pass %d: invalid buffer destination '%s'",
	  parent->GetName (), GetPassNumber (pass), dest);
      }
    }
  }


  //get texturemappings
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
      
      ShaderPass::TextureMapping texMap;
      
      const char* compareMode = mapping->GetAttributeValue ("comparemode");
      const char* compareFunc = mapping->GetAttributeValue ("comparefunc");
      if (compareMode && compareFunc)
      {
        if (strcmp (compareMode, "rToTexture") == 0)
          texMap.texCompare.mode = CS::Graphics::TextureComparisonMode::compareR;
        else if (strcmp (compareMode, "none") == 0)
          texMap.texCompare.mode = CS::Graphics::TextureComparisonMode::compareNone;
	else
	{
          SetFailReason ("invalid texture comparison mode '%s'",
	    compareMode);
          return false;
	}
      
        if (strcmp (compareFunc, "lequal") == 0)
          texMap.texCompare.function = CS::Graphics::TextureComparisonMode::funcLEqual;
        else if (strcmp (compareFunc, "gequal") == 0)
          texMap.texCompare.function = CS::Graphics::TextureComparisonMode::funcGEqual;
	else
	{
          SetFailReason ("invalid texture comparison function '%s'",
	    compareMode);
          return false;
	}
      }
      
      CS::Graphics::ShaderVarNameParser parser (
        mapping->GetAttributeValue("name"));
      texMap.id = stringsSvName->Request (parser.GetShaderVarName ());
      parser.FillArrayWithIndices (texMap.indices);
      texMap.textureUnit = texUnit;
      pass->textures.Push (texMap);
    }
  }

  return true;
}

bool csXMLShaderCompiler::LoadSVBlock (iLoaderContext* ldr_context,
    iDocumentNode *node, iShaderVariableContext *context)
{
  csRef<csShaderVariable> svVar;
  
  csRef<iDocumentNodeIterator> it = node->GetNodes ("shadervar");
  while (it->HasNext ())
  {
    csRef<iDocumentNode> var = it->Next ();
    svVar.AttachNew (new csShaderVariable);

    if (synldr->ParseShaderVar (ldr_context, var, *svVar))
      context->AddVariable(svVar);
  }

  return true;
}

csPtr<iShaderProgram> csXMLShaderTech::LoadProgram (
  iShaderDestinationResolver* resolve, iDocumentNode* node, ShaderPass* /*pass*/,
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

bool csXMLShaderTech::Load (iLoaderContext* ldr_context,
    iDocumentNode* node, iDocumentNode* parentSV, size_t variant)
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
      parent->compiler->LoadSVBlock (ldr_context, varNode, &svcontext);
  }

  csRef<iDocumentNode> varNode = node->GetNode(
    xmltokens.Request (csXMLShaderCompiler::XMLTOKEN_SHADERVARS));

  if (varNode)
    parent->compiler->LoadSVBlock (ldr_context, varNode, &svcontext);

  //alloc passes
  passes = new ShaderPass[passesCount];
  uint i;
  for (i = 0; i < passesCount; i++)
  {
    ShaderPass& pass = passes[i];
    pass.alphaMode.autoAlphaMode = true;
    pass.alphaMode.autoModeTexture = 
      parent->compiler->stringsSvName->Request (CS_MATERIAL_TEXTURE_DIFFUSE);
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

  ShaderPass* thispass = &passes[currentPass];
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
  ShaderPass* thispass = &passes[currentPass];
  currentPass = (size_t)~0;

  if(thispass->vproc) thispass->vproc->Deactivate ();
  if(thispass->vp) thispass->vp->Deactivate ();
  if(thispass->fp) thispass->fp->Deactivate ();

  iGraphics3D* g3d = parent->g3d;
  g3d->DeactivateBuffers (thispass->custommapping_attrib.GetArray (), 
    (int)thispass->custommapping_attrib.GetSize ());

  int texturesCount = (int)thispass->textures.GetSize();
  CS_ALLOC_STACK_ARRAY(int, textureUnits, texturesCount);
  for (int j = 0; j < texturesCount; j++)
    textureUnits[j] = thispass->textures[j].textureUnit;
  g3d->SetTextureState(textureUnits, 0, texturesCount);
  g3d->SetTextureComparisonModes (textureUnits, 0, texturesCount);
  
  if (thispass->overrideZmode)
    g3d->SetZMode (oldZmode);

  g3d->SetWriteMask (orig_wmRed, orig_wmGreen, orig_wmBlue, orig_wmAlpha);

  return true;
}

bool csXMLShaderTech::SetupPass (const csRenderMesh *mesh, 
			         csRenderMeshModes& modes,
			         const csShaderVariableStack& stack)
{
  if(currentPass>=passesCount)
    return false;

  iGraphics3D* g3d = parent->g3d;
  ShaderPass* thispass = &passes[currentPass];

  //first run the preprocessor
  if(thispass->vproc) thispass->vproc->SetupState (mesh, modes, stack);

  size_t buffersCount = thispass->custommapping_attrib.GetSize ();
  CS_ALLOC_STACK_ARRAY(iRenderBuffer*, customBuffers, buffersCount);
  //now map our buffers. all refs should be set
  size_t i;
  for (i = 0; i < thispass->custommapping_attrib.GetSize (); i++)
  {
    if (thispass->custommapping_buffer[i] != CS_BUFFER_NONE)
    {
      customBuffers[i] = modes.buffers->GetRenderBuffer (
	thispass->custommapping_buffer[i]);
    }
    else if (thispass->custommapping_id[i] < (csStringID)stack.GetSize ())
    {
      csShaderVariable* var = 0;
      var = csGetShaderVariableFromStack (stack, thispass->custommapping_id[i]);
      if (var)
        var->GetValue (customBuffers[i]);
      else
        customBuffers[i] = 0;
    }
    else
      customBuffers[i] = 0;
  }
  g3d->ActivateBuffers (modes.buffers, thispass->defaultMappings);
  g3d->ActivateBuffers (thispass->custommapping_attrib.GetArray (), 
    customBuffers, (uint)buffersCount);
  
  //and the textures
  size_t textureCount = thispass->textures.GetSize();
  CS_ALLOC_STACK_ARRAY(int, textureUnits, textureCount);
  CS_ALLOC_STACK_ARRAY(iTextureHandle*, textureHandles, textureCount);
  CS_ALLOC_STACK_ARRAY(CS::Graphics::TextureComparisonMode, texCompare,
    textureCount);
  for (size_t j = 0; j < textureCount; j++)
  {
    textureUnits[j] = thispass->textures[j].textureUnit;
    if (size_t (thispass->textures[j].id) < stack.GetSize ())
    {
      csShaderVariable* var = 0;
      var = csGetShaderVariableFromStack (stack, thispass->textures[j].id);
      if (var != 0)
        var = CS::Graphics::ShaderVarArrayHelper::GetArrayItem (var, 
          thispass->textures[j].indices.GetArray(),
          thispass->textures[j].indices.GetSize(),
          CS::Graphics::ShaderVarArrayHelper::maFail);
      if (var)
      {
        iTextureWrapper* wrap;
        var->GetValue (wrap);
        if (wrap) 
        {
          wrap->Visit ();
          textureHandles[j] = wrap->GetTextureHandle ();
        } else 
          var->GetValue (textureHandles[j]);
      } else
        textureHandles[j] = 0;
    }
    else
      textureHandles[j] = 0;
    texCompare[j] = thispass->textures[j].texCompare;
  }
  g3d->SetTextureState (textureUnits, textureHandles, textureCount);
  g3d->SetTextureComparisonModes (textureUnits, texCompare, textureCount);

  modes = *mesh;
  if (thispass->alphaMode.autoAlphaMode)
  {
    iTextureHandle* tex = 0;
    if (thispass->alphaMode.autoModeTexture != csInvalidStringID)
    {
      if (thispass->alphaMode.autoModeTexture < (csStringID)stack.GetSize ())
      {
        csShaderVariable* var = 0;
        var = csGetShaderVariableFromStack (stack, thispass->alphaMode.autoModeTexture);
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
  
  float alpha = 1.0f;
  if (modes.mixmode & CS_FX_MASK_ALPHA)
  {
    alpha = 1.0f - (float)(modes.mixmode & CS_FX_MASK_ALPHA) / 255.0f;
  }
  parent->shadermgr->GetVariableAdd (
    parent->compiler->string_mixmode_alpha)->SetValue (alpha);
  modes.zoffset = thispass->zoffset;

  if(thispass->vp) thispass->vp->SetupState (mesh, modes, stack);
  if(thispass->fp) thispass->fp->SetupState (mesh, modes, stack);

  return true;
}

bool csXMLShaderTech::TeardownPass ()
{
  ShaderPass* thispass = &passes[currentPass];

  if(thispass->vproc) thispass->vproc->ResetState ();
  if(thispass->vp) thispass->vp->ResetState ();
  if(thispass->fp) thispass->fp->ResetState ();

  return true;
}

void csXMLShaderTech::GetUsedShaderVars (csBitArray& bits) const
{
  csDirtyAccessArray<csStringID> allNames;

  for (size_t pass = 0; pass < passesCount; pass++)
  {
    ShaderPass* thispass = &passes[pass];

    if(thispass->vproc)
    {
      thispass->vproc->GetUsedShaderVars (bits);
    }

    for (size_t i = 0; i < thispass->custommapping_attrib.GetSize (); i++)
    {
      CS::ShaderVarStringID id = thispass->custommapping_id[i];
      if ((id != CS::InvalidShaderVarStringID) && (bits.GetSize() > id))
      {
        bits.SetBit (id);
      }
    }
    for (size_t j = 0; j < thispass->textures.GetSize(); j++)
    {
      CS::ShaderVarStringID id = thispass->textures[j].id;
      if ((id != CS::InvalidShaderVarStringID) && (bits.GetSize() > id))
      {
        bits.SetBit (id);
      }
    }

    if(thispass->vp)
    {
      thispass->vp->GetUsedShaderVars (bits);
    }

    if(thispass->fp)
    {
      thispass->fp->GetUsedShaderVars (bits);
    }
  }
}

int csXMLShaderTech::GetPassNumber (ShaderPass* pass)
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
