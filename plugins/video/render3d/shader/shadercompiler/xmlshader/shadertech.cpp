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
#include "iutil/stringarray.h"
#include "ivaria/reporter.h"
#include "ivideo/rendermesh.h"
#include "ivideo/material.h"

#include "csgfx/renderbuffer.h"
#include "csutil/csendian.h"
#include "csutil/documenthelper.h"
#include "csutil/stringarray.h"

#include "shader.h"
#include "shadertech.h"
#include "xmlshader.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

CS_LEAKGUARD_IMPLEMENT (csXMLShaderTech);

/* Magic value for tech + pass cache files.
 * The most significant byte serves as a "version", increase when the
 * cache file format changes. */
static const uint32 cacheFileMagic = 0x04747863;

//---------------------------------------------------------------------------

struct CachedPlugin
{
  bool available;
  csString pluginID;
  csString progType;
  
  csRef<iShaderProgramPlugin> programPlugin;
  csRef<iDocumentNode> programNode;
  
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

struct csXMLShaderTech::LoadHelpers
{
  iSyntaxService* synldr;
  iStringSet* strings;
  iShaderVarStringSet* stringsSvName;
};

bool csXMLShaderTech::LoadPass (iDocumentNode *node, ShaderPass* pass, 
                                size_t variant, iFile* cacheFile,
                                iHierarchicalCache* cacheTo)
{
  LoadHelpers hlp;
  hlp.synldr = parent->compiler->synldr;
  hlp.strings = parent->compiler->strings;
  hlp.stringsSvName = parent->compiler->stringsSvName;

  CachedPlugins cachedPlugins;

  GetProgramPlugins (node, cachedPlugins, variant);

  csRef<iShaderProgram> program;
  // load fp
  /* This is done before VP loading b/c the VP could query for TU bindings
  * which are currently handled by the FP. */
  bool result = true;
  bool setFailReason = true;
  
  csString tagFP, tagVP, tagVPr;
  
  if (cachedPlugins.fp.programNode)
  {
    csRef<iHierarchicalCache> fpCache;
    if (cacheTo) fpCache = cacheTo->GetRootedCache (csString().Format (
      "/pass%dfp", GetPassNumber (pass)));
    program = LoadProgram (0, cachedPlugins.fp.programNode, pass, 
      variant, fpCache, cachedPlugins.fp, tagFP);
    if (program)
      pass->fp = program;
    else
    {
      if (do_verbose && setFailReason)
      {
	SetFailReason ("pass %d fragment program failed to load",
	  GetPassNumber (pass));
	setFailReason = false;
      }
      result = false;
    }
  }

  csRef<iShaderDestinationResolver> resolveFP;
  if (pass->fp) 
    resolveFP = scfQueryInterface<iShaderDestinationResolver> (pass->fp);

  //load vp
  if (cachedPlugins.vp.programNode)
  {
    csRef<iHierarchicalCache> vpCache;
    if (cacheTo) vpCache = cacheTo->GetRootedCache (csString().Format (
      "/pass%dvp", GetPassNumber (pass)));
    program = LoadProgram (resolveFP, cachedPlugins.vp.programNode, 
      pass, variant, vpCache, cachedPlugins.vp, tagVP);
    if (program)
    {
      pass->vp = program;
    }
    else
    {
      if (do_verbose && setFailReason)
      {
	SetFailReason ("pass %d vertex program failed to load",
	  GetPassNumber (pass));
	setFailReason = false;
      }
      result = false;
    }
  }

  csRef<iShaderDestinationResolver> resolveVP;
  if (pass->vp) 
    resolveVP = scfQueryInterface<iShaderDestinationResolver> (pass->vp);

  //load vproc
  if (cachedPlugins.vproc.programNode)
  {
    csRef<iHierarchicalCache> vprCache;
    if (cacheTo) vprCache = cacheTo->GetRootedCache (csString().Format (
      "/pass%dvpr", GetPassNumber (pass)));
    program = LoadProgram (resolveFP, cachedPlugins.vproc.programNode,
      pass, variant, vprCache, cachedPlugins.vproc, tagVPr);
    if (program)
    {
      pass->vproc = program;
    }
    else
    {
      if (do_verbose && setFailReason)
      {
	SetFailReason ("pass %d vertex preprocessor failed to load",
	  GetPassNumber (pass));
	setFailReason = false;
      }
      result = false;
    }
  }

  if (result)
  {
    if (!ParseModes (pass, node, hlp)) return false;

    const csGraphics3DCaps* caps = parent->g3d->GetCaps();
    if (IsDestalphaMixmode (pass->mixMode) && !caps->DestinationAlpha)
    {
      if (do_verbose)
	SetFailReason ("destination alpha not supported by renderer");
      return false;
    }
  }

  WritePass (pass, cachedPlugins, cacheFile);

  //if we got this far, load buffermappings
  if (result && !ParseBuffers (*pass, GetPassNumber (pass),
    node, hlp, resolveFP, resolveVP)) result = false;

  //get texturemappings
  if (result && !ParseTextures (*pass, node, hlp, resolveFP)) result = false;
  
  if (cacheTo)
  {
    csMemFile perTagFile;
    WritePassPerTag (*pass, &perTagFile);
    cacheTo->CacheData (perTagFile.GetData(), perTagFile.GetSize(),
      csString().Format ("/pass%ddata/%s_%s_%s", GetPassNumber (pass),
      tagFP.GetDataSafe(), tagVP.GetDataSafe(), tagVPr.GetDataSafe()));
  }
  
  return result;
}

struct PassActionPrecache
{
  PassActionPrecache (int passIndex, csXMLShaderTech* tech, size_t variant,
    iHierarchicalCache* cacheTo, iDocumentNode* node,
    csXMLShaderTech::LoadHelpers hlp) : passIndex (passIndex),
    tech (tech), variant (variant), cacheTo (cacheTo), node (node),
    hlp (hlp)
  {
    if (cacheTo)
    {
      fpCache = cacheTo->GetRootedCache (csString().Format (
        "/pass%dfp", passIndex));
      vpCache = cacheTo->GetRootedCache (csString().Format (
        "/pass%dvp", passIndex));
      vprCache = cacheTo->GetRootedCache (csString().Format (
        "/pass%dvpr", passIndex));
    }
  }
  
  bool Perform (const csStringArray& tags,
    CachedPlugins& cachedPlugins)
  {
    // load fp
    /* This is done before VP loading b/c the VP could query for TU bindings
    * which are currently handled by the FP. */
    bool result = true;
    bool setFailReason = true;
    
    csRef<iBase> fp;
    if (cachedPlugins.fp.programNode)
    {
      csRef<iBase>* cacheP = fpTagCache.GetElementPointer (tags[0]);
      if (!cacheP)
      {
	if (!tech->PrecacheProgram (0, cachedPlugins.fp.programNode, variant,
	    fpCache, cachedPlugins.fp, fp, tags[0]))
	{
	  if (tech->do_verbose && setFailReason)
	  {
	    tech->SetFailReason ("pass %d fragment program failed to precache",
	      passIndex);
	    setFailReason = false;
	  }
	  result = false;
	}
	fpTagCache.PutUnique (tags[0], fp);
      }
      else
	fp = *cacheP;
    }
  
    csRef<iBase> vp;
    //load vp
    if (cachedPlugins.vp.programNode)
    {
      csString pcachekey;
      pcachekey.Format ("%s_%s", tags[0], tags[1]);
      csRef<iBase>* cacheP = vpTagCache.GetElementPointer (pcachekey);
      if (!cacheP)
      {
	if (!tech->PrecacheProgram (fp, cachedPlugins.vp.programNode, variant,
	    vpCache, cachedPlugins.vp, vp, tags[1]))
	{
	  if (tech->do_verbose && setFailReason)
	  {
	    tech->SetFailReason ("pass %d vertex program failed to precache",
	      passIndex);
	    setFailReason = false;
	  }
	  result = false;
	}
	vpTagCache.PutUnique (pcachekey, vp);
      }
      else
	vp = *cacheP;
    }
  
    csRef<iBase> vpr;
    //load vproc
    if (cachedPlugins.vproc.programNode)
    {
      csString pcachekey;
      pcachekey.Format ("%s_%s_%s", tags[0], tags[1], tags[2]);
      csRef<iBase>* cacheP = vprTagCache.GetElementPointer (pcachekey);
      if (!cacheP)
      {
	if (!tech->PrecacheProgram (vp, cachedPlugins.vproc.programNode, variant,
	    vprCache, cachedPlugins.vproc, vpr, tags[2]))
	{
	  if (tech->do_verbose && setFailReason)
	  {
	    tech->SetFailReason ("pass %d vertex preprocessor failed to precache",
	      passIndex);
	    setFailReason = false;
	  }
	  result = false;
	}
	vprTagCache.PutUnique (pcachekey, vpr);
      }
      else
	vpr = *cacheP;
    }

    if (result)
    {
      csRef<iShaderDestinationResolver> resolveFP;
      if (fp) resolveFP = scfQueryInterface<iShaderDestinationResolver> (fp);
      csRef<iShaderDestinationResolver> resolveVP;
      if (vp) resolveVP = scfQueryInterface<iShaderDestinationResolver> (vp);
      
      csXMLShaderTech::ShaderPassPerTag passPerTag;
      
      //if we got this far, load buffermappings
      if (result && !tech->ParseBuffers (passPerTag, passIndex,
	node, hlp, resolveFP, resolveVP)) result = false;
    
      //get texturemappings
      if (result && !tech->ParseTextures (passPerTag, node, hlp, resolveFP))
	result = false;
      
      if (result)
      {
	csMemFile perTagFile;
	tech->WritePassPerTag (passPerTag, &perTagFile);
	cacheTo->CacheData (perTagFile.GetData(), perTagFile.GetSize(),
	  csString().Format ("/pass%ddata/%s_%s_%s", passIndex,
	  tags[0] ? tags[0] : "",
	  tags[1] ? tags[1] : "",
	  tags[2] ? tags[2] : ""));
      }
      
      oneComboWorked |= result;
    }
  
    return false;
  }

  bool oneComboWorked;
private:
  int passIndex;
  csXMLShaderTech* tech;
  size_t variant;
  iHierarchicalCache* cacheTo;
  iDocumentNode* node;

  csRef<iHierarchicalCache> fpCache;
  csRef<iHierarchicalCache> vpCache;
  csRef<iHierarchicalCache> vprCache;

  csHash<csRef<iBase>, csString> fpTagCache;
  csHash<csRef<iBase>, csString> vpTagCache;
  csHash<csRef<iBase>, csString> vprTagCache;
  
  csXMLShaderTech::LoadHelpers hlp;
};

bool csXMLShaderTech::PrecachePass (iDocumentNode *node, ShaderPass* pass, 
                                    size_t variant, iFile* cacheFile,
                                    iHierarchicalCache* cacheTo)
{
  LoadHelpers hlp;
  hlp.synldr = parent->compiler->synldr;
  hlp.strings = parent->compiler->strings;
  hlp.stringsSvName = parent->compiler->stringsSvName;

  CachedPlugins cachedPlugins;

  GetProgramPlugins (node, cachedPlugins, variant);

  if (!ParseModes (pass, node, hlp)) return false;

  WritePass (pass, cachedPlugins, cacheFile);
  
  PassActionPrecache passAction (GetPassNumber (pass), this, variant,
    cacheTo, node, hlp);
  LoadPassPrograms (node, passAction, variant, cachedPlugins);
  
  return passAction.oneComboWorked;
}
  
template<typename PassAction>
bool csXMLShaderTech::LoadPassPrograms (iDocumentNode* passNode, 
                                        PassAction& action,
                                        size_t variant,
                                        CachedPlugins& cachedPlugins)
{
  csArray<csStringArray> tuples;
  tuples.Push (csStringArray ());
  
  for (size_t i = 0; i < 3; i++)
  {
    CachedPlugin plugin;
    switch (i)
    {
      case 0: plugin = cachedPlugins.fp; break;
      case 1: plugin = cachedPlugins.vp; break;
      case 2: plugin = cachedPlugins.vproc; break;
    }
    if (plugin.programNode == 0)
    {
      for (size_t j = 0; j < tuples.GetSize(); j++)
      {
        tuples[j].Push ("");
      }
      continue;
    }
  
    csArray<csStringArray> newTuples;
    
    csRef<iStringArray> tags = plugin.programPlugin->QueryPrecacheTags (
      plugin.progType);
    for (size_t j = 0; j < tuples.GetSize(); j++)
    {
      csStringArray tupleStrs (tuples[j]);
      for (size_t t = 0; t < tags->GetSize(); t++)
      {
        csStringArray newTupleStrs (tupleStrs);
        newTupleStrs.Push (tags->Get (t));
        newTuples.Push (newTupleStrs);
      }
    }
    tuples = newTuples;
  }
  
  for (size_t j = 0; j < tuples.GetSize(); j++)
  {
    const csStringArray& tupleStrs (tuples[j]);
      
    if (action.Perform (tupleStrs, cachedPlugins)) return true;
  }
  return false;
}

bool csXMLShaderTech::ParseModes (ShaderPass* pass,
                                  iDocumentNode* node, 
                                  LoadHelpers& h)
{
  csRef<iDocumentNode> nodeMixMode = node->GetNode ("mixmode");
  if (nodeMixMode != 0)
  {
    uint mm;
    if (h.synldr->ParseMixmode (nodeMixMode, mm, true))
      pass->mixMode = mm;
  }

  csRef<iDocumentNode> nodeAlphaMode = node->GetNode ("alphamode");
  if (nodeAlphaMode != 0)
  {
    csAlphaMode am;
    if (h.synldr->ParseAlphaMode (nodeAlphaMode, h.strings, am))
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

    if (h.synldr->ParseZMode (node, pass->zMode, true))
    {
      pass->overrideZmode = true;
    }
    else
    {
      h.synldr->ReportBadToken (node);
    }
  }

  csRef<iDocumentNode> nodeFlipCulling = node->GetNode ("flipculling");
  if (nodeFlipCulling)
  {
    h.synldr->ParseBool(nodeFlipCulling, pass->flipCulling, false);
  }

  csRef<iDocumentNode> nodeZOffset = node->GetNode ("zoffset");
  if (nodeZOffset)
  {
    h.synldr->ParseBool (nodeZOffset, pass->zoffset, false);
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
  
  pass->minLights = node->GetAttributeValueAsInt ("minlights");
  
  return true;
}

bool csXMLShaderTech::ParseBuffers (ShaderPassPerTag& pass, int passNum,
                                    iDocumentNode* node, 
                                    LoadHelpers& h,
                                    iShaderDestinationResolver* resolveFP,
                                    iShaderDestinationResolver* resolveVP)
{
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

        CS::ShaderVarStringID varID = h.stringsSvName->Request (cname);
        pass.custommapping_id.Push (varID);
        //pass->bufferGeneric[pass->bufferCount] = CS_VATTRIB_IS_GENERIC (attrib);

        pass.custommapping_attrib.Push (attrib);
	pass.custommapping_buffer.Push (CS_BUFFER_NONE);
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
	  pass.defaultMappings[attrib] = sourceName;
	else
	{
	  pass.custommapping_attrib.Push (attrib);
	  pass.custommapping_buffer.Push (sourceName);
          pass.custommapping_id.Push (CS::InvalidShaderVarStringID);
	  /* Those buffers are mapped by default to some specific vattribs; 
	   * since they are now to be mapped to some generic vattrib,
	   * turn off the default map. */
	  if (sourceName == CS_BUFFER_POSITION)
	    pass.defaultMappings[CS_VATTRIB_POSITION] = CS_BUFFER_NONE;
	}
      }
    }
    else
    {
      if ((attrib == CS_VATTRIB_INVALID)
        && parent->compiler->do_verbose)
      {
        parent->compiler->Report (CS_REPORTER_SEVERITY_WARNING,
	  "Shader '%s', pass %d: invalid buffer destination '%s'",
	  parent->GetName (), passNum, dest);
      }
    }
  }
  return true;
}

bool csXMLShaderTech::ParseTextures (ShaderPassPerTag& pass,
                                     iDocumentNode* node, 
                                     LoadHelpers& h,
                                     iShaderDestinationResolver* resolveFP)
{
  csRef<iDocumentNodeIterator> it;
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
      texMap.id = h.stringsSvName->Request (parser.GetShaderVarName ());
      parser.FillArrayWithIndices (texMap.indices);
      texMap.textureUnit = texUnit;
      pass.textures.Push (texMap);
    }
  }
  return true;
}

// Used to generate data written on disk!
enum
{
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
  {
    int32 diskMinLights = csLittleEndian::Int32 (pass->minLights);
    if (cacheFile->Write ((char*)&diskMinLights, sizeof (diskMinLights))
	!= sizeof (diskMinLights)) return false;
  }
  return true;
}
  
bool csXMLShaderTech::WritePassPerTag (const ShaderPassPerTag& pass,
                                       iFile* cacheFile)
{
  if (!cacheFile) return false;

  uint32 diskMagic = csLittleEndian::UInt32 (cacheFileMagic);
  if (cacheFile->Write ((char*)&diskMagic, sizeof (diskMagic))
      != sizeof (diskMagic)) return false;
      
  for (int i = 0; i < CS_VATTRIB_SPECIFIC_NUM; i++)
  {
    int32 diskMapping = csLittleEndian::Int32 (pass.defaultMappings[i]);
    if (cacheFile->Write ((char*)&diskMapping, sizeof (diskMapping))
	!= sizeof (diskMapping)) return false;
  }
  {
    uint32 diskNumBuffers = csLittleEndian::UInt32 (
      (uint)pass.custommapping_buffer.GetSize());
    if (cacheFile->Write ((char*)&diskNumBuffers, sizeof (diskNumBuffers))
	!= sizeof (diskNumBuffers)) return false;
  }
  for (size_t i = 0; i < pass.custommapping_buffer.GetSize(); i++)
  {
    int32 diskMapping = csLittleEndian::Int32 (pass.custommapping_buffer[i]);
    if (cacheFile->Write ((char*)&diskMapping, sizeof (diskMapping))
	!= sizeof (diskMapping)) return false;
	
    int32 diskAttr = csLittleEndian::Int32 (pass.custommapping_attrib[i]);
    if (cacheFile->Write ((char*)&diskAttr, sizeof (diskAttr))
	!= sizeof (diskAttr)) return false;
	
    const char* svStr = parent->compiler->stringsSvName->Request (
      pass.custommapping_id[i]);
    if (!CS::PluginCommon::ShaderCacheHelper::WriteString (cacheFile, svStr))
      return false;
  }

  {
    uint32 diskNumTextures = csLittleEndian::UInt32 (
      (uint)pass.textures.GetSize());
    if (cacheFile->Write ((char*)&diskNumTextures, sizeof (diskNumTextures))
	!= sizeof (diskNumTextures)) return false;
  }
  for (size_t i = 0; i < pass.textures.GetSize(); i++)
  {
    const char* svStr = parent->compiler->stringsSvName->Request (
      pass.textures[i].id);
    if (!CS::PluginCommon::ShaderCacheHelper::WriteString (cacheFile, svStr))
      return false;
      
    uint32 diskNumIndices = csLittleEndian::UInt32 (
      (uint)pass.textures[i].indices.GetSize());
    if (cacheFile->Write ((char*)&diskNumIndices, sizeof (diskNumIndices))
	!= sizeof (diskNumIndices)) return false;
    for (size_t n = 0; n < pass.textures[i].indices.GetSize(); n++)
    {
      uint32 diskIndex = csLittleEndian::UInt32 (
	(uint)pass.textures[i].indices[n]);
      if (cacheFile->Write ((char*)&diskIndex, sizeof (diskIndex))
	  != sizeof (diskIndex)) return false;
    }
    
    int32 diskTU = csLittleEndian::Int32 (pass.textures[i].textureUnit);
    if (cacheFile->Write ((char*)&diskTU, sizeof (diskTU))
	!= sizeof (diskTU)) return false;
    
    int16 diskCompareFunc = csLittleEndian::Int16 (
      pass.textures[i].texCompare.function);
    if (cacheFile->Write ((char*)&diskCompareFunc, sizeof (diskCompareFunc))
	!= sizeof (diskCompareFunc)) return false;
    
    int16 diskCompareMode = csLittleEndian::Int16 (
      pass.textures[i].texCompare.mode);
    if (cacheFile->Write ((char*)&diskCompareMode, sizeof (diskCompareMode))
	!= sizeof (diskCompareMode)) return false;
  }
  
  return true;
}
  
iShaderProgram::CacheLoadResult csXMLShaderTech::LoadPassFromCache (
  ShaderPass* pass, iDocumentNode* node, size_t variant, iFile* cacheFile,
  iHierarchicalCache* cache)
{
  if (!cacheFile) return iShaderProgram::loadFail;

  CachedPlugins plugins;
  GetProgramPlugins (node, plugins, variant);
  
  if (!ReadPass (pass, cacheFile, plugins)) return iShaderProgram::loadFail;
  
  csString tagFP, tagVP, tagVPr;
  
  iShaderProgram::CacheLoadResult loadRes;
  if (plugins.fp.available)
  {
    csRef<iHierarchicalCache> fpCache;
    if (cache) fpCache = cache->GetRootedCache (csString().Format (
      "/pass%dfp", GetPassNumber (pass)));
    loadRes = LoadProgramFromCache (0, variant, fpCache, plugins.fp,
      pass->fp, tagFP, GetPassNumber (pass));
    if (loadRes != iShaderProgram::loadSuccessShaderValid) return loadRes;
  }
  
  if (plugins.vp.available)
  {
    csRef<iHierarchicalCache> vpCache;
    if (cache) vpCache = cache->GetRootedCache (csString().Format (
      "/pass%dvp", GetPassNumber (pass)));
    loadRes = LoadProgramFromCache (pass->fp, variant, vpCache, plugins.vp, pass->vp,
      tagVP, GetPassNumber (pass));
    if (loadRes != iShaderProgram::loadSuccessShaderValid) return loadRes;
  }
  
  if (plugins.vproc.available)
  {
    csRef<iHierarchicalCache> vprCache;
    if (cache) vprCache = cache->GetRootedCache (csString().Format (
      "/pass%dvpr", GetPassNumber (pass)));
    loadRes = LoadProgramFromCache (pass->vp, variant, vprCache, plugins.vproc,
      pass->vproc, tagVPr, GetPassNumber (pass));
    if (loadRes != iShaderProgram::loadSuccessShaderValid) return loadRes;
  }
  
  csRef<iDataBuffer> perTagData = cache->ReadCache (
    csString().Format ("/pass%ddata/%s_%s_%s", GetPassNumber (pass),
      tagFP.GetDataSafe(), tagVP.GetDataSafe(), tagVPr.GetDataSafe()));
  if (!perTagData.IsValid()) return iShaderProgram::loadFail;
  csMemFile perTagCacheFile (perTagData, true);
  if (!ReadPassPerTag (*pass, &perTagCacheFile))
    return iShaderProgram::loadFail;
  
  return iShaderProgram::loadSuccessShaderValid;
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
  
    pass->wmRed = cacheFlags & (1 << cacheFlagWMR);
    pass->wmGreen = cacheFlags & (1 << cacheFlagWMG);
    pass->wmBlue = cacheFlags & (1 << cacheFlagWMB);
    pass->wmAlpha = cacheFlags & (1 << cacheFlagWMA);
    
    pass->overrideZmode = cacheFlags & (1 << cacheFlagOverrideZ);
    pass->flipCulling = cacheFlags & (1 << cacheFlagFlipCulling);
    pass->zoffset = cacheFlags & (1 << cacheFlagZoffset);
    
    pass->alphaMode.autoAlphaMode = cacheFlags & (1 << cacheFlagAlphaAuto);
  }
  
  {
    uint32 diskMM;
    if (cacheFile->Read ((char*)&diskMM, sizeof (diskMM))
	!= sizeof (diskMM)) return false;
    pass->mixMode = csLittleEndian::UInt32 (diskMM);
  }
  
  if (pass->alphaMode.autoAlphaMode)
  {
    const char* svName =
      CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile);
    if (!svName) return false;
    pass->alphaMode.autoModeTexture = parent->compiler->stringsSvName->Request (
      svName);
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
  {
    int32 diskMinLights;
    if (cacheFile->Read ((char*)&diskMinLights, sizeof (diskMinLights))
	!= sizeof (diskMinLights)) return false;
    pass->minLights = csLittleEndian::Int32 (diskMinLights);
  }
  return true;
}
  
bool csXMLShaderTech::ReadPassPerTag (ShaderPassPerTag& pass, 
                                      iFile* cacheFile)
{
  if (!cacheFile) return false;

  uint32 diskMagic;
  if (cacheFile->Read ((char*)&diskMagic, sizeof (diskMagic))
      != sizeof (diskMagic)) return false;
  if (csLittleEndian::UInt32 (diskMagic) != cacheFileMagic)
    return false;
    
  for (int i = 0; i < CS_VATTRIB_SPECIFIC_NUM; i++)
  {
    int32 diskMapping;
    if (cacheFile->Read ((char*)&diskMapping, sizeof (diskMapping))
	!= sizeof (diskMapping)) return false;
    pass.defaultMappings[i] =
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
      pass.custommapping_buffer.Push (
        (csRenderBufferName)csLittleEndian::Int32 (diskMapping));
	  
      int32 diskAttr;
      if (cacheFile->Read ((char*)&diskAttr, sizeof (diskAttr))
	  != sizeof (diskAttr)) return false;
      pass.custommapping_attrib.Push (
        (csVertexAttrib)csLittleEndian::Int32 (diskAttr));

      const char* mappingStr = 
          CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile);
      if (mappingStr != 0)
	pass.custommapping_id.Push (
	  parent->compiler->stringsSvName->Request (mappingStr));
      else
	pass.custommapping_id.Push (CS::InvalidShaderVarStringID);
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
      
      pass.textures.Push (mapping);
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
  size_t variant, iHierarchicalCache* cacheTo, CachedPlugin& cacheInfo,
  csString& tag)
{
  csRef<iShaderProgram> program;

  program = cacheInfo.programPlugin->CreateProgram (cacheInfo.progType);
  if (program == 0)
    return 0;
  if (!program->Load (resolve, cacheInfo.programNode))
    return 0;

  /* Even if compilation fails, for since we need to handle the case where a
      program is valid but not supported on the current HW in caching, 
      flag program as available */
  cacheInfo.available = true;
  
  csRef<iString> progTag;
  if (!program->Compile (cacheTo, &progTag))
    return 0;
  if (!progTag.IsValid()) return 0;
  tag = progTag->GetData();
    
  return csPtr<iShaderProgram> (program);
}
  
iShaderProgram::CacheLoadResult csXMLShaderTech::LoadProgramFromCache (
  iBase* previous, size_t variant, iHierarchicalCache* cache,
  const CachedPlugin& cacheInfo, csRef<iShaderProgram>& prog, csString& tag,
  int passNumber)
{
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
    return iShaderProgram::loadFail;
  }

  prog = plg->CreateProgram (cacheInfo.progType);
  csRef<iString> progTag;
  csRef<iString> failReason;
  iShaderProgram::CacheLoadResult loadRes = prog->LoadFromCache (cache, 
    previous, cacheInfo.programNode, &failReason, &progTag);
  if (loadRes == iShaderProgram::loadFail)
  {
    if (parent->compiler->do_verbose)
      SetFailReason(
	"Failed to read '%s' for pass %d from cache: %s",
	cacheInfo.progType.GetData(), passNumber,
	failReason.IsValid() ? failReason->GetData() : "no reason given");
    return iShaderProgram::loadFail;
  }
  
  if (progTag.IsValid()) tag = progTag->GetData();

  return loadRes;
}

bool csXMLShaderTech::PrecacheProgram (iBase* previous, iDocumentNode* node, 
  size_t variant, iHierarchicalCache* cacheTo, 
  CachedPlugin& cacheInfo, csRef<iBase>& object, const char* tag)
{
  return cacheInfo.programPlugin->Precache (cacheInfo.progType, tag, previous,
    cacheInfo.programNode, cacheTo, &object);
}

void csXMLShaderTech::GetProgramPlugins (iDocumentNode *node,
                                         CachedPlugins& cacheInfo,
                                         size_t variant)
{
  {
    csRef<iDocumentNode> programNodeFP;
    programNodeFP = node->GetNode (xmltokens.Request (
      csXMLShaderCompiler::XMLTOKEN_FP));
      
    if (programNodeFP)
      GetProgramPlugin (programNodeFP, cacheInfo.fp, variant);
  }
  
  {
    csRef<iDocumentNode> programNodeVP;
    programNodeVP = node->GetNode(xmltokens.Request (
      csXMLShaderCompiler::XMLTOKEN_VP));
      
    if (programNodeVP)
      GetProgramPlugin (programNodeVP, cacheInfo.vp, variant);
  }

  {
    csRef<iDocumentNode> programNodeVPr;
    programNodeVPr = node->GetNode(xmltokens.Request (
      csXMLShaderCompiler::XMLTOKEN_VPROC));
      
    if (programNodeVPr)
      GetProgramPlugin (programNodeVPr, cacheInfo.vproc, variant);
  }
  
}

void csXMLShaderTech::GetProgramPlugin (iDocumentNode *node,
                                        CachedPlugin& cacheInfo,
                                        size_t variant)
{
  if (node->GetAttributeValue("plugin") == 0)
  {
    parent->compiler->Report (CS_REPORTER_SEVERITY_ERROR,
      "No shader program plugin specified for <%s> in shader '%s'",
      node->GetValue (), parent->GetName ());
    return;
  }

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
    return;
  }

  const char* programType = node->GetAttributeValue("type");
  if (programType == 0)
    programType = node->GetValue ();
  csRef<iDocumentNode> programNode;
  if (node->GetAttributeValue ("file") != 0)
    programNode = parent->LoadProgramFile (node->GetAttributeValue ("file"), 
      variant);
  else
    programNode = node;

  /* Even if compilation fails, for since we need to handle the case where a
      program is valid but not supported on the current HW in caching, 
      flag program as available */
  cacheInfo.available = true;
  cacheInfo.pluginID = plugin;
  cacheInfo.progType = programType;
  cacheInfo.programPlugin = plg;
  cacheInfo.programNode = programNode;
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
	CS::DocSystem::CloneNode (child, newNode);
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
  bool result = true;
  while (it->HasNext ())
  {
    csRef<iDocumentNode> passNode = it->Next ();
    result &= LoadPass (passNode, &passes[currentPassNr++], variant, cacheFile, cacheTo);
  }
  
  if (cacheFile.IsValid())
  {
    cacheTo->CacheData (cacheFile->GetData(), cacheFile->GetSize(),
      "/passes");
  }

  return result;
}
  
bool csXMLShaderTech::Precache (iDocumentNode* node, size_t variant, 
                                iHierarchicalCache* cacheTo)
{
  csRef<csMemFile> cacheFile;
  cacheFile.AttachNew (new csMemFile);
  uint32 diskMagic = csLittleEndian::UInt32 (cacheFileMagic);
  if (cacheFile->Write ((char*)&diskMagic, sizeof (diskMagic))
      != sizeof (diskMagic))
    return false;
  
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
      CS::DocSystem::CloneNode (child, newNode);
    }
    
    csRef<iDataBuffer> boilerplateBuf (parent->compiler->WriteNodeToBuf (
      boilerplateDoc));
    if (!CS::PluginCommon::ShaderCacheHelper::WriteDataBuffer (
	cacheFile, boilerplateBuf))
      return false;
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
  
  int32 diskPassNum = csLittleEndian::Int32 (passesCount);
  if (cacheFile->Write ((char*)&diskPassNum, sizeof (diskPassNum))
      != sizeof (diskPassNum))
    return false;

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
    if (!PrecachePass (passNode, &passes[currentPassNr++], variant, cacheFile, cacheTo))
    {
      return false;
    }
  }
  
  return cacheTo->CacheData (cacheFile->GetData(), cacheFile->GetSize(),
    "/passes");
}
  
iShaderProgram::CacheLoadResult csXMLShaderTech::LoadFromCache (
  iLoaderContext* ldr_context, iDocumentNode* node, 
  iHierarchicalCache* cache, iDocumentNode* parentSV, size_t variant)
{
  csRef<iDataBuffer> cacheData (cache->ReadCache ("/passes"));
  if (!cacheData.IsValid()) return iShaderProgram::loadFail;

  csMemFile cacheFile (cacheData, true);
  
  uint32 diskMagic;
  size_t read = cacheFile.Read ((char*)&diskMagic, sizeof (diskMagic));
  if (read != sizeof (diskMagic)) return iShaderProgram::loadFail;
  if (csLittleEndian::UInt32 (diskMagic) != cacheFileMagic)
    return iShaderProgram::loadFail;
  
  csRef<iDataBuffer> boilerPlateDocBuf =
    CS::PluginCommon::ShaderCacheHelper::ReadDataBuffer (&cacheFile);
  if (!boilerPlateDocBuf.IsValid()) return iShaderProgram::loadFail;
  
  csRef<iDocumentNode> boilerplateNode (parent->compiler->ReadNodeFromBuf (
    boilerPlateDocBuf));
  if (!boilerplateNode.IsValid()) return iShaderProgram::loadFail;
  
  if (!LoadBoilerplate (ldr_context, boilerplateNode, parentSV))
    return iShaderProgram::loadFail;
  
  int32 diskPassNum;
  read = cacheFile.Read ((char*)&diskPassNum, sizeof (diskPassNum));
  if (read != sizeof (diskPassNum))
    return iShaderProgram::loadFail;
  
  size_t nodePassesCount = 0;
  csRef<iDocumentNodeIterator> it = node->GetNodes (xmltokens.Request (
    csXMLShaderCompiler::XMLTOKEN_PASS));
  while(it->HasNext ())
  {
    it->Next ();
    nodePassesCount++;
  }
  
  passesCount = csLittleEndian::Int32 (diskPassNum);
  if (passesCount != nodePassesCount) return iShaderProgram::loadFail;
  passes = new ShaderPass[passesCount];
  it = node->GetNodes (xmltokens.Request (csXMLShaderCompiler::XMLTOKEN_PASS));
  for (uint p = 0; p < passesCount; p++)
  {
    csRef<iDocumentNode> child = it->Next ();
    iShaderProgram::CacheLoadResult loadRes =
      LoadPassFromCache (&passes[p], child, variant, &cacheFile, cache);
    if (loadRes != iShaderProgram::loadSuccessShaderValid)
      return loadRes;
  }
  
  return iShaderProgram::loadSuccessShaderValid;
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

  int lightCount = 0;
  if (stack.GetSize() > parent->compiler->stringLightCount)
  {
    csShaderVariable* svLightCount = stack[parent->compiler->stringLightCount];
    if (svLightCount != 0)
      svLightCount->GetValue (lightCount);
  }
  if (lightCount < thispass->minLights) return false;
  
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
