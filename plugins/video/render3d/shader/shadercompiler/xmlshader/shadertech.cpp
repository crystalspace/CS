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
#include "iutil/hiercache.h"
#include "iutil/plugin.h"
#include "iutil/string.h"
#include "ivaria/reporter.h"
#include "ivideo/rendermesh.h"
#include "ivideo/material.h"

#include "csgfx/renderbuffer.h"
#include "csutil/csendian.h"
#include "csutil/documenthelper.h"

#include "shader.h"
#include "shadertech.h"
#include "xmlshader.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

CS_LEAKGUARD_IMPLEMENT (csXMLShaderTech);

/* Magic value for tech + pass cache files.
 * The most significant byte serves as a "version", increase when the
 * cache file format changes. */
static const uint32 cacheFileMagic = 0x01747863;

//---------------------------------------------------------------------------

struct CachedPlugin
{
  bool available;
  csString pluginID;
  csString progType;
  
  CachedPlugin() : available (false) {}
};

struct CachedPlugins
{
  CachedPlugin fp, vp, vproc;
};

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
                                size_t variant, iFile* cacheFile,
                                iHierarchicalCache* cacheTo)
{
  iSyntaxService* synldr = parent->compiler->synldr;
  iStringSet* strings = parent->compiler->strings;
  iShaderVarStringSet* stringsSvName = parent->compiler->stringsSvName;

  CachedPlugins cachedPlugins;

  csRef<iDocumentNode> programNode;
  csRef<iShaderProgram> program;
  // load fp
  /* This is done before VP loading b/c the VP could query for TU bindings
   * which are currently handled by the FP. */
  programNode = node->GetNode (xmltokens.Request (
    csXMLShaderCompiler::XMLTOKEN_FP));

  if (programNode)
  {
    csRef<iHierarchicalCache> fpCache;
    if (cacheTo) fpCache = cacheTo->GetRootedCache (csString().Format (
      "/pass%dfp", GetPassNumber (pass)));
    program = LoadProgram (0, programNode, pass, variant, fpCache,
      cachedPlugins.fp);
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
    csRef<iHierarchicalCache> vpCache;
    if (cacheTo) vpCache = cacheTo->GetRootedCache (csString().Format (
      "/pass%dvp", GetPassNumber (pass)));
    program = LoadProgram (resolveFP, programNode, pass, variant, vpCache,
      cachedPlugins.vp);
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
    csRef<iHierarchicalCache> vprCache;
    if (cacheTo) vprCache = cacheTo->GetRootedCache (csString().Format (
      "/pass%dvpr", GetPassNumber (pass)));
    program = LoadProgram (resolveFP, programNode, pass, variant, vprCache,
      cachedPlugins.vproc);
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
  
  WritePass (pass, cachedPlugins, cacheFile);

  return true;
}

// Used to generate data written on disk!
enum
{
  cacheFlagHasFP,
  cacheFlagHasVP,
  cacheFlagHasVProc,
  
  cacheFlagWMR = 4,
  cacheFlagWMG,
  cacheFlagWMB,
  cacheFlagWMA,
  
  cacheFlagOverrideZ,
  cacheFlagFlipCulling,
  cacheFlagZoffset,
  
  cacheFlagAlphaAuto
};

bool csXMLShaderTech::WritePass (ShaderPass* pass, 
                                 const CachedPlugins& plugins, 
                                 iFile* cacheFile)
{
  if (!cacheFile) return false;

  {
    uint32 cacheFlags = 0;
    if (plugins.fp.available) cacheFlags |= 1 << cacheFlagHasFP;
    if (plugins.vp.available) cacheFlags |= 1 << cacheFlagHasVP;
    if (plugins.vproc.available) cacheFlags |= 1 << cacheFlagHasVProc;
    
    if (pass->wmRed) cacheFlags |= 1 << cacheFlagWMR;
    if (pass->wmGreen) cacheFlags |= 1 << cacheFlagWMG;
    if (pass->wmBlue) cacheFlags |= 1 << cacheFlagWMB;
    if (pass->wmAlpha) cacheFlags |= 1 << cacheFlagWMA;
    
    if (pass->overrideZmode) cacheFlags |= 1 << cacheFlagOverrideZ;
    if (pass->flipCulling) cacheFlags |= 1 << cacheFlagFlipCulling;
    if (pass->zoffset) cacheFlags |= 1 << cacheFlagZoffset;
    
    if (pass->alphaMode.autoAlphaMode) cacheFlags |= 1 << cacheFlagAlphaAuto;
    
    uint32 diskFlags = csLittleEndian::UInt32 (cacheFlags);
    if (cacheFile->Write ((char*)&diskFlags, sizeof (diskFlags))
	!= sizeof (diskFlags)) return false;
  }
  
  if (!CS::PluginCommon::ShaderCacheHelper::WriteString (cacheFile, 
      plugins.fp.progType))
    return false;
  if (!CS::PluginCommon::ShaderCacheHelper::WriteString (cacheFile, 
      plugins.fp.pluginID))
    return false;
  
  if (!CS::PluginCommon::ShaderCacheHelper::WriteString (cacheFile, 
      plugins.vp.progType))
    return false;
  if (!CS::PluginCommon::ShaderCacheHelper::WriteString (cacheFile, 
      plugins.vp.pluginID))
    return false;
  
  if (!CS::PluginCommon::ShaderCacheHelper::WriteString (cacheFile, 
      plugins.vproc.progType))
    return false;
  if (!CS::PluginCommon::ShaderCacheHelper::WriteString (cacheFile, 
      plugins.vproc.pluginID))
    return false;
  
  {
    uint32 diskMM = csLittleEndian::UInt32 (pass->mixMode);
    if (cacheFile->Write ((char*)&diskMM, sizeof (diskMM))
	!= sizeof (diskMM)) return false;
  }
  
  if (pass->alphaMode.autoAlphaMode)
  {
    const char* autoTexStr = parent->compiler->stringsSvName->Request (
      pass->alphaMode.autoModeTexture);
    if (!CS::PluginCommon::ShaderCacheHelper::WriteString (cacheFile, autoTexStr))
      return false;
  }
  else
  {
    uint32 diskAlpha = csLittleEndian::UInt32 (pass->alphaMode.alphaType);
    if (cacheFile->Write ((char*)&diskAlpha, sizeof (diskAlpha))
	!= sizeof (diskAlpha)) return false;
  }
  {
    uint32 diskZ = csLittleEndian::UInt32 (pass->zMode);
    if (cacheFile->Write ((char*)&diskZ, sizeof (diskZ))
	!= sizeof (diskZ)) return false;
  }
  for (int i = 0; i < CS_VATTRIB_SPECIFIC_NUM; i++)
  {
    int32 diskMapping = csLittleEndian::Int32 (pass->defaultMappings[i]);
    if (cacheFile->Write ((char*)&diskMapping, sizeof (diskMapping))
	!= sizeof (diskMapping)) return false;
  }
  
  {
    uint32 diskNumBuffers = csLittleEndian::UInt32 (
      (uint)pass->custommapping_buffer.GetSize());
    if (cacheFile->Write ((char*)&diskNumBuffers, sizeof (diskNumBuffers))
	!= sizeof (diskNumBuffers)) return false;
  }
  for (size_t i = 0; i < pass->custommapping_buffer.GetSize(); i++)
  {
    int32 diskMapping = csLittleEndian::Int32 (pass->custommapping_buffer[i]);
    if (cacheFile->Write ((char*)&diskMapping, sizeof (diskMapping))
	!= sizeof (diskMapping)) return false;
	
    int32 diskAttr = csLittleEndian::Int32 (pass->custommapping_attrib[i]);
    if (cacheFile->Write ((char*)&diskAttr, sizeof (diskAttr))
	!= sizeof (diskAttr)) return false;
	
    const char* svStr = parent->compiler->stringsSvName->Request (
      pass->custommapping_id[i]);
    if (!CS::PluginCommon::ShaderCacheHelper::WriteString (cacheFile, svStr))
      return false;
  }

  {
    uint32 diskNumTextures = csLittleEndian::UInt32 (
      (uint)pass->textures.GetSize());
    if (cacheFile->Write ((char*)&diskNumTextures, sizeof (diskNumTextures))
	!= sizeof (diskNumTextures)) return false;
  }
  for (size_t i = 0; i < pass->textures.GetSize(); i++)
  {
    const char* svStr = parent->compiler->stringsSvName->Request (
      pass->textures[i].id);
    if (!CS::PluginCommon::ShaderCacheHelper::WriteString (cacheFile, svStr))
      return false;
      
    uint32 diskNumIndices = csLittleEndian::UInt32 (
      (uint)pass->textures[i].indices.GetSize());
    if (cacheFile->Write ((char*)&diskNumIndices, sizeof (diskNumIndices))
	!= sizeof (diskNumIndices)) return false;
    for (size_t n = 0; n < pass->textures[i].indices.GetSize(); n++)
    {
      uint32 diskIndex = csLittleEndian::UInt32 (
	(uint)pass->textures[i].indices[n]);
      if (cacheFile->Write ((char*)&diskIndex, sizeof (diskIndex))
	  != sizeof (diskIndex)) return false;
    }
    
    int32 diskTU = csLittleEndian::Int32 (pass->textures[i].textureUnit);
    if (cacheFile->Write ((char*)&diskTU, sizeof (diskTU))
	!= sizeof (diskTU)) return false;
    
    int16 diskCompareFunc = csLittleEndian::Int16 (
      pass->textures[i].texCompare.function);
    if (cacheFile->Write ((char*)&diskCompareFunc, sizeof (diskCompareFunc))
	!= sizeof (diskCompareFunc)) return false;
    
    int16 diskCompareMode = csLittleEndian::Int16 (
      pass->textures[i].texCompare.mode);
    if (cacheFile->Write ((char*)&diskCompareMode, sizeof (diskCompareMode))
	!= sizeof (diskCompareMode)) return false;
  }
  
  return true;
}
  
bool csXMLShaderTech::LoadPassFromCache (ShaderPass* pass, size_t variant,
                                         iFile* cacheFile,
                                         iHierarchicalCache* cache)
{
  if (!cacheFile) return false;

  CachedPlugins plugins;
  if (!ReadPass (pass, cacheFile, plugins)) return false;
  
  if (plugins.fp.available)
  {
    csRef<iHierarchicalCache> fpCache;
    if (cache) fpCache = cache->GetRootedCache (csString().Format (
      "/pass%dfp", GetPassNumber (pass)));
    pass->fp = LoadProgramFromCache (pass, variant, fpCache, plugins.fp);
    if (!pass->fp.IsValid()) return false;
  }
  
  if (plugins.vp.available)
  {
    csRef<iHierarchicalCache> vpCache;
    if (cache) vpCache = cache->GetRootedCache (csString().Format (
      "/pass%dvp", GetPassNumber (pass)));
    pass->vp = LoadProgramFromCache (pass, variant, vpCache, plugins.vp);
    if (!pass->vp.IsValid()) return false;
  }
  
  if (plugins.vproc.available)
  {
    csRef<iHierarchicalCache> vprCache;
    if (cache) vprCache = cache->GetRootedCache (csString().Format (
      "/pass%dvpr", GetPassNumber (pass)));
    pass->vproc = LoadProgramFromCache (pass, variant, vprCache, plugins.vproc);
    if (!pass->vproc.IsValid()) return false;
  }
  
  return true;
}

bool csXMLShaderTech::ReadPass (ShaderPass* pass, 
                                 iFile* cacheFile, 
                                 CachedPlugins& plugins)
{
  if (!cacheFile) return false;

  {
    uint32 diskFlags;
    if (cacheFile->Read ((char*)&diskFlags, sizeof (diskFlags))
	!= sizeof (diskFlags)) return false;
    uint32 cacheFlags = csLittleEndian::UInt32 (diskFlags);
  
    plugins.fp.available = cacheFlags & (1 << cacheFlagHasFP);
    plugins.vp.available = cacheFlags & (1 << cacheFlagHasVP);
    plugins.vproc.available = cacheFlags & (1 << cacheFlagHasVProc);
    
    pass->wmRed = cacheFlags & (1 << cacheFlagWMR);
    pass->wmGreen = cacheFlags & (1 << cacheFlagWMG);
    pass->wmBlue = cacheFlags & (1 << cacheFlagWMB);
    pass->wmAlpha = cacheFlags & (1 << cacheFlagWMA);
    
    pass->overrideZmode = cacheFlags & (1 << cacheFlagOverrideZ);
    pass->flipCulling = cacheFlags & (1 << cacheFlagFlipCulling);
    pass->zoffset = cacheFlags & (1 << cacheFlagZoffset);
    
    pass->alphaMode.autoAlphaMode = cacheFlags & (1 << cacheFlagAlphaAuto);
  }
  
  plugins.fp.progType = CS::PluginCommon::ShaderCacheHelper::ReadString (
    cacheFile);
  plugins.fp.pluginID = CS::PluginCommon::ShaderCacheHelper::ReadString (
    cacheFile);
  
  plugins.vp.progType = CS::PluginCommon::ShaderCacheHelper::ReadString (
    cacheFile);
  plugins.vp.pluginID = CS::PluginCommon::ShaderCacheHelper::ReadString (
    cacheFile);
  
  plugins.vproc.progType = CS::PluginCommon::ShaderCacheHelper::ReadString (
    cacheFile);
  plugins.vproc.pluginID = CS::PluginCommon::ShaderCacheHelper::ReadString (
    cacheFile);
  
  {
    uint32 diskMM;
    if (cacheFile->Read ((char*)&diskMM, sizeof (diskMM))
	!= sizeof (diskMM)) return false;
    pass->mixMode = csLittleEndian::UInt32 (diskMM);
  }
  
  if (pass->alphaMode.autoAlphaMode)
  {
    pass->alphaMode.autoModeTexture = parent->compiler->stringsSvName->Request (
      CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile));
  }
  else
  {
    uint32 diskAlpha;
    if (cacheFile->Read ((char*)&diskAlpha, sizeof (diskAlpha))
	!= sizeof (diskAlpha)) return false;
    pass->alphaMode.alphaType =
      (csAlphaMode::AlphaType)csLittleEndian::UInt32 (diskAlpha);
  }
  {
    uint32 diskZ;
    if (cacheFile->Read ((char*)&diskZ, sizeof (diskZ))
	!= sizeof (diskZ)) return false;
    pass->zMode = (csZBufMode)csLittleEndian::UInt32 (diskZ);
  }
  for (int i = 0; i < CS_VATTRIB_SPECIFIC_NUM; i++)
  {
    int32 diskMapping;
    if (cacheFile->Read ((char*)&diskMapping, sizeof (diskMapping))
	!= sizeof (diskMapping)) return false;
    pass->defaultMappings[i] =
      (csRenderBufferName)csLittleEndian::Int32 (diskMapping);
  }
  
  {
    size_t numBuffers;
    uint32 diskNumBuffers;
    if (cacheFile->Read ((char*)&diskNumBuffers, sizeof (diskNumBuffers))
	!= sizeof (diskNumBuffers)) return false;
    numBuffers = csLittleEndian::UInt32 (diskNumBuffers);
    for (size_t i = 0; i < numBuffers; i++)
    {
      int32 diskMapping;
      if (cacheFile->Read ((char*)&diskMapping, sizeof (diskMapping))
	  != sizeof (diskMapping)) return false;
      pass->custommapping_buffer.Push (
        (csRenderBufferName)csLittleEndian::Int32 (diskMapping));
	  
      int32 diskAttr;
      if (cacheFile->Read ((char*)&diskAttr, sizeof (diskAttr))
	  != sizeof (diskAttr)) return false;
      pass->custommapping_attrib.Push (
        (csVertexAttrib)csLittleEndian::Int32 (diskAttr));

      const char* mappingStr = 
          CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile);
      if (mappingStr != 0)
	pass->custommapping_id.Push (
	  parent->compiler->stringsSvName->Request (mappingStr));
      else
	pass->custommapping_id.Push (CS::InvalidShaderVarStringID);
    }
  }

  {
    size_t numTextures;
    uint32 diskNumTextures;
    if (cacheFile->Read ((char*)&diskNumTextures, sizeof (diskNumTextures))
	!= sizeof (diskNumTextures)) return false;
    numTextures = csLittleEndian::UInt32 (diskNumTextures);
    for (size_t i = 0; i < numTextures; i++)
    {
      ShaderPass::TextureMapping mapping;
      mapping.id = parent->compiler->stringsSvName->Request (
        CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile));
	
      size_t numIndices;
      uint32 diskNumIndices;
      if (cacheFile->Read ((char*)&diskNumIndices, sizeof (diskNumIndices))
	  != sizeof (diskNumIndices)) return false;
      numIndices = csLittleEndian::UInt32 (diskNumIndices);
      for (size_t n = 0; n < numIndices; n++)
      {
	uint32 diskIndex;
	if (cacheFile->Read ((char*)&diskIndex, sizeof (diskIndex))
	    != sizeof (diskIndex)) return false;
	mapping.indices.Push (csLittleEndian::UInt32 (diskIndex));
      }
      
      int32 diskTU;
      if (cacheFile->Read ((char*)&diskTU, sizeof (diskTU))
	  != sizeof (diskTU)) return false;
      mapping.textureUnit = csLittleEndian::Int32 (diskTU);
      
      int16 diskCompareFunc;
      if (cacheFile->Read ((char*)&diskCompareFunc, sizeof (diskCompareFunc))
	  != sizeof (diskCompareFunc)) return false;
      mapping.texCompare.function = (CS::Graphics::TextureComparisonMode::Function)
        csLittleEndian::Int16 (diskCompareFunc);
      
      int16 diskCompareMode;
      if (cacheFile->Read ((char*)&diskCompareMode, sizeof (diskCompareMode))
	  != sizeof (diskCompareMode)) return false;
      mapping.texCompare.mode = (CS::Graphics::TextureComparisonMode::Mode)
        csLittleEndian::Int16 (diskCompareMode);
      
      pass->textures.Push (mapping);
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
  size_t variant, iHierarchicalCache* cacheTo, CachedPlugin& cacheInfo)
{
  if (node->GetAttributeValue("plugin") == 0)
  {
    parent->compiler->Report (CS_REPORTER_SEVERITY_ERROR,
      "No shader program plugin specified for <%s> in shader '%s'",
      node->GetValue (), parent->GetName ());
    return 0;
  }

  csRef<iShaderProgram> program;

  csStringFast<256> plugin ("crystalspace.graphics3d.shader.");
  plugin.Append (node->GetAttributeValue("plugin"));
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
	  plugin.GetData(), node->GetValue (), parent->GetName ());
    return 0;
  }

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

  if (!program->Compile (cacheTo))
    return 0;
    
  cacheInfo.available = true;
  cacheInfo.pluginID = plugin;
  cacheInfo.progType = programType;

  return csPtr<iShaderProgram> (program);
}
  
csPtr<iShaderProgram> csXMLShaderTech::LoadProgramFromCache (
  ShaderPass* /*pass*/,
  size_t variant, iHierarchicalCache* cache, const CachedPlugin& cacheInfo)
{
  csRef<iShaderProgram> program;

  //load the plugin
  csRef<iShaderProgramPlugin> plg;
  plg = csLoadPluginCheck<iShaderProgramPlugin> (parent->compiler->objectreg,
  	cacheInfo.pluginID, false);
  if(!plg)
  {
    if (parent->compiler->do_verbose)
      SetFailReason(
	  "Couldn't retrieve shader plugin '%s' for '%s' in shader '%s'",
	  cacheInfo.pluginID.GetData(), cacheInfo.progType.GetData(),
	  parent->GetName ());
    return 0;
  }

  program = plg->CreateProgram (cacheInfo.progType);
  csRef<iString> failReason;
  if (!program->LoadFromCache (cache, &failReason))
  {
    if (parent->compiler->do_verbose)
      SetFailReason(
	"Failed to read '%s' from cache: %s",
	cacheInfo.progType.GetData(),
	failReason.IsValid() ? failReason->GetData() : "no reason given");
    return 0;
  }

  return csPtr<iShaderProgram> (program);
}
  
bool csXMLShaderTech::LoadBoilerplate (iLoaderContext* ldr_context, 
                                       iDocumentNode* node,
                                       iDocumentNode* parentSV)
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
      return false;
    }
  }

  if ((requiredCount != 0) && (requiredPresent == 0))
  {
    if (do_verbose) SetFailReason ("No required shader tag is present");
    return false;
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

  return true;
}

bool csXMLShaderTech::Load (iLoaderContext* ldr_context,
    iDocumentNode* node, iDocumentNode* parentSV, size_t variant,
    iHierarchicalCache* cacheTo)
{
  if (!LoadBoilerplate (ldr_context, node, parentSV))
    return 0;

  csRef<csMemFile> cacheFile;
  if (cacheTo)
  {
    cacheFile.AttachNew (new csMemFile);
    uint32 diskMagic = csLittleEndian::UInt32 (cacheFileMagic);
    if (cacheFile->Write ((char*)&diskMagic, sizeof (diskMagic))
	!= sizeof (diskMagic))
      cacheFile.Invalidate();
    
    if (cacheFile.IsValid())
    {
      csRef<iDocument> boilerplateDoc (parent->compiler->CreateCachingDoc());
      csRef<iDocumentNode> root = boilerplateDoc->CreateRoot();
      csRef<iDocumentNode> techNode = root->CreateNodeBefore (node->GetType());
      techNode->SetValue (node->GetValue ());
      CS::DocSystem::CloneAttributes (node, techNode);
      
      csRef<iDocumentNodeIterator> it = node->GetNodes ();
      while(it->HasNext ())
      {
	csRef<iDocumentNode> child = it->Next ();
	if (child->GetType() == CS_NODE_COMMENT) continue;
	if (xmltokens.Request (child->GetValue())
	  == csXMLShaderCompiler::XMLTOKEN_PASS) continue;
	  
	csRef<iDocumentNode> newNode = techNode->CreateNodeBefore (child->GetType());
	CS::DocSystem::CloneAttributes (child, newNode);
      }
      
      csRef<iDataBuffer> boilerplateBuf (parent->compiler->WriteNodeToBuf (
        boilerplateDoc));
      if (!CS::PluginCommon::ShaderCacheHelper::WriteDataBuffer (
	  cacheFile, boilerplateBuf))
	cacheFile.Invalidate();
    }
  }

  //count passes
  passesCount = 0;
  csRef<iDocumentNodeIterator> it = node->GetNodes (xmltokens.Request (
    csXMLShaderCompiler::XMLTOKEN_PASS));
  while(it->HasNext ())
  {
    it->Next ();
    passesCount++;
  }
  
  if (cacheFile.IsValid())
  {
    int32 diskPassNum = csLittleEndian::Int32 (passesCount);
    if (cacheFile->Write ((char*)&diskPassNum, sizeof (diskPassNum))
	!= sizeof (diskPassNum))
      cacheFile.Invalidate();
  }

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
    if (!LoadPass (passNode, &passes[currentPassNr++], variant, cacheFile, cacheTo))
    {
      return false;
    }
  }
  
  if (cacheFile.IsValid())
  {
    cacheTo->CacheData (cacheFile->GetData(), cacheFile->GetSize(),
      "/passes");
  }

  return true;
}
  
bool csXMLShaderTech::LoadFromCache (iLoaderContext* ldr_context, 
                                     iHierarchicalCache* cache,
                                     iDocumentNode* parentSV, 
                                     size_t variant)
{
  csRef<iDataBuffer> cacheData (cache->ReadCache ("/passes"));
  if (!cacheData.IsValid()) return false;

  csMemFile cacheFile (cacheData, true);
  
  uint32 diskMagic;
  size_t read = cacheFile.Read ((char*)&diskMagic, sizeof (diskMagic));
  if (read != sizeof (diskMagic)) return false;
  if (csLittleEndian::UInt32 (diskMagic) != cacheFileMagic) return false;
  
  csRef<iDataBuffer> boilerPlateDocBuf =
    CS::PluginCommon::ShaderCacheHelper::ReadDataBuffer (&cacheFile);
  if (!boilerPlateDocBuf.IsValid()) return false;
  
  csRef<iDocumentNode> boilerplateNode (parent->compiler->ReadNodeFromBuf (
    boilerPlateDocBuf));
  if (!boilerplateNode.IsValid()) return false;
  
  if (!LoadBoilerplate (ldr_context, boilerplateNode, parentSV)) return false;
  
  int32 diskPassNum;
  read = cacheFile.Read ((char*)&diskPassNum, sizeof (diskPassNum));
  if (read != sizeof (diskPassNum)) return false;
  
  passesCount = csLittleEndian::Int32 (diskPassNum);
  passes = new ShaderPass[passesCount];
  for (uint p = 0; p < passesCount; p++)
  {
    if (!LoadPassFromCache (&passes[p], variant, &cacheFile, cache))
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
