/*
    Copyright (C)2007 by Marten Svanfeldt

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

#include "csgfx/renderbuffer.h"
#include "csgfx/shadervarcontext.h"
#include "csutil/cfgacc.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/objreg.h"
#include "csutil/xmltiny.h"
#include "cstool/rbuflock.h"
#include "iengine/rview.h"
#include "imap/loader.h"
#include "imap/services.h"
#include "iutil/document.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivaria/view.h"
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"

#include "csplugincommon/rendermanager/posteffects.h"

#include <stdarg.h>

using namespace CS::RenderManager;

PostEffectManager::PostEffectManager ()
  : currentWidth (0), currentHeight (0),
    textureFmt ("abgr8"), lastLayer (0)
{
  AddLayer (0, 0, 0);
  GetBucketIndex (lastLayer->options);
}

PostEffectManager::~PostEffectManager ()
{
}

void PostEffectManager::Initialize (iObjectRegistry* objectReg)
{
  graphics3D = csQueryRegistry<iGraphics3D> (objectReg);
  svStrings = csQueryRegistryTagInterface<iShaderVarStringSet> (objectReg,
    "crystalspace.shader.variablenameset");
    
  csConfigAccess cfg (objectReg);
  keepAllIntermediates =
    cfg->GetBool ("PostEffectManager.KeepAllIntermediates", false);
}

void PostEffectManager::SetIntermediateTargetFormat (const char* textureFmt)
{
  this->textureFmt = textureFmt;
}

void PostEffectManager::SetupView (iView* view)
{
  unsigned int width = view->GetContext ()->GetWidth ();
  unsigned int height = view->GetContext ()->GetHeight ();

  if (width != currentWidth || height != currentHeight)
  {
    currentWidth = width;
    currentHeight = height;

    UpdateTextureDistribution();
    AllocatePingpongTextures ();
    SetupScreenQuad (width, height);
    UpdateSVContexts ();
  }
}

void PostEffectManager::SetupView (uint width, uint height)
{
  if (width != currentWidth || height != currentHeight)
  {
    currentWidth = width;
    currentHeight = height;

    UpdateTextureDistribution();
    AllocatePingpongTextures ();
    SetupScreenQuad (width, height);
    UpdateSVContexts ();
  }
}

iTextureHandle* PostEffectManager::GetScreenTarget ()
{
  if (postLayers.GetSize () > 1)
  {
    return GetBucket (postLayers[0]->options).textures[postLayers[0]->outTextureNum];
  }

  return 0;
}

void PostEffectManager::DrawPostEffects ()
{ 
  graphics3D->FinishDraw ();

  for (size_t layer = 1; layer < postLayers.GetSize (); ++layer)
  {   
    // Draw and ping-pong   
    fullscreenQuad.dynDomain = postLayers[layer]->svContext;
    fullscreenQuad.shader = postLayers[layer]->effectShader;
    fullscreenQuad.renderBuffers = postLayers[layer]->buffers;

    size_t bucket = GetBucketIndex (postLayers[layer]->options);
    graphics3D->SetRenderTarget (layer < postLayers.GetSize () - 1 
      ? buckets[bucket].textures[postLayers[layer]->outTextureNum] 
      : (iTextureHandle*)target);
    
    /* @@@ FIXME: Without this, screen space doesn't always work when
       a custom proj is set */
    graphics3D->SetPerspectiveCenter (currentWidth/2, currentHeight/2);
    graphics3D->SetPerspectiveAspect (currentHeight);
    
    graphics3D->BeginDraw (CSDRAW_CLEARZBUFFER | CSDRAW_3DGRAPHICS);
    graphics3D->DrawSimpleMesh (fullscreenQuad, csSimpleMeshScreenspace);
    graphics3D->FinishDraw ();
  }
}
    
PostEffectManager::Layer* PostEffectManager::AddLayer (iShader* shader)
{
  LayerOptions opt;
  return AddLayer (shader, opt);
}

PostEffectManager::Layer* PostEffectManager::AddLayer (iShader* shader, const LayerOptions& opt)
{
  LayerInputMap map;
  map.inputLayer = lastLayer;
  return AddLayer (shader, opt, 1, &map);
}

PostEffectManager::Layer* PostEffectManager::AddLayer (iShader* shader, size_t numMaps, 
                                                       const LayerInputMap* maps)
{
  LayerOptions opt;
  return AddLayer (shader, opt, numMaps, maps);
}

PostEffectManager::Layer* PostEffectManager::AddLayer (iShader* shader, const LayerOptions& opt,
                                                       size_t numMaps, const LayerInputMap* maps)
{
  Layer* newLayer = new Layer;
  newLayer->effectShader = shader;
  for (size_t n = 0; n < numMaps; n++)
    newLayer->inputs.Push (maps[n]);
  newLayer->options = opt;
  postLayers.Push (newLayer);
  textureDistributionDirty = true;
  lastLayer = newLayer;
  return newLayer;
}

void PostEffectManager::ClearLayers()
{
  buckets.DeleteAll();
  postLayers.DeleteAll();
  currentWidth = 0; currentHeight = 0;
  lastLayer = 0;
  
  AddLayer (0, 0, 0);
  GetBucketIndex (lastLayer->options);
}

void PostEffectManager::SetupScreenQuad (unsigned int width, unsigned int height)
{
  indices = csRenderBuffer::CreateIndexRenderBuffer (4,
    CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_SHORT, 0, 3);
  {
    csRenderBufferLock<unsigned short> indexLock (indices);
    for (uint i = 0; i < 4; i++)
      indexLock[(size_t)i] = i;
  }

  fullscreenQuad.meshtype = CS_MESHTYPE_QUADS;
  fullscreenQuad.vertexCount = 4;
  fullscreenQuad.mixmode = CS_MIXMODE_BLEND(ONE, ZERO);
  
  for (size_t b = 0; b < buckets.GetSize(); b++)
  {
    iRenderBuffer* vertBuf = buckets[b].vertBuf =
      csRenderBuffer::CreateRenderBuffer (4, CS_BUF_STATIC, 
        CS_BUFCOMP_FLOAT, 3);
    iRenderBuffer* texcoordBuf = buckets[b].texcoordBuf =
      csRenderBuffer::CreateRenderBuffer (4, CS_BUF_STATIC,
        CS_BUFCOMP_FLOAT, 2);
        
    csRenderBufferLock<csVector3> screenQuadVerts (vertBuf);
    csRenderBufferLock<csVector2> screenQuadTex (texcoordBuf);
  
    int texW = width >> buckets[b].options.downsample;
    int texH = height >> buckets[b].options.downsample;
    
    // Setup the vertices & texcoords
    screenQuadVerts[(size_t)0].Set (0, 0, 0);
    screenQuadTex[(size_t)0].Set (buckets[b].textureOffsetX,
      buckets[b].textureOffsetY);
  
    screenQuadVerts[(size_t)1].Set (texW, 0, 0);
    screenQuadTex[(size_t)1].Set (
      buckets[b].textureCoordinateX + buckets[b].textureOffsetX, 
      buckets[b].textureOffsetY);
  
    screenQuadVerts[(size_t)2].Set (texW, texH, 0);
    screenQuadTex[(size_t)2].Set (
      buckets[b].textureCoordinateX + buckets[b].textureOffsetX, 
      buckets[b].textureCoordinateY + buckets[b].textureOffsetY);
  
    screenQuadVerts[(size_t)3].Set (0, texH, 0);
    screenQuadTex[(size_t)3].Set (buckets[b].textureOffsetX, 
      buckets[b].textureCoordinateY + buckets[b].textureOffsetY);
  }
}

void PostEffectManager::AllocatePingpongTextures ()
{
  for (size_t b = 0; b < buckets.GetSize(); b++)
  {
    uint texFlags =
      CS_TEXTURE_3D | CS_TEXTURE_NPOTS | CS_TEXTURE_CLAMP | CS_TEXTURE_SCALE_UP;
    if (!buckets[b].options.mipmap)
      texFlags |= CS_TEXTURE_NOMIPMAPS;
    csRef<iTextureHandle> t;
    int texW = currentWidth >> buckets[b].options.downsample;
    int texH = currentHeight >> buckets[b].options.downsample;
    t = graphics3D->GetTextureManager ()->CreateTexture (texW, texH, 
      csimg2D, textureFmt, texFlags);
    if (buckets[b].options.maxMipmap >= 0)
      t->SetMipmapLimits (buckets[b].options.maxMipmap);
    buckets[b].textures.Put (0, t);
  
    buckets[b].textureOffsetX = buckets[b].textureOffsetY = 0;
  
    // Check if we actually got NPOTs textures
    int resultFlags = buckets[b].textures[0]->GetFlags ();
    if (!(resultFlags & CS_TEXTURE_NPOTS))
    {
      // Handle that we didn't get a power of 2 texture
      // Means the texture is bigger and we need to use <1 for texture coordinate
      int tw, th, td;
      buckets[b].textures[0]->GetRendererDimensions (tw, th, td);
  
      buckets[b].textureCoordinateX = texW / (float)tw;
      buckets[b].textureCoordinateY = texH / (float)th;
  
      buckets[b].textureOffsetY = 1-buckets[b].textureCoordinateY;
    }
    else if (buckets[b].textures[0]->GetTextureType () == iTextureHandle::texTypeRect)
    {
      // Handle a rect texture
      // Rect texture use non-normalized texture coordinates
      buckets[b].textureCoordinateX = texW;
      buckets[b].textureCoordinateY = texH;
    }
    else
    {
      // NPOT texture, coordinates are 0-1,0-1
      buckets[b].textureCoordinateX = 1;
      buckets[b].textureCoordinateY = 1;
    }  
    buckets[b].svPixelSize.AttachNew (new csShaderVariable (svStrings->Request ("pixel size")));
    buckets[b].svPixelSize->SetValue (
      csVector2 (buckets[b].textureCoordinateX/texW, 
      buckets[b].textureCoordinateY/texH));
  
    for (size_t i = 1; i < buckets[b].textures.GetSize(); i++)
    {
      t = graphics3D->GetTextureManager ()->CreateTexture (texW, texH, 
	csimg2D, textureFmt, resultFlags);
      buckets[b].textures.Put (i, t);
    }
  }
}

void PostEffectManager::UpdateTextureDistribution()
{
  for (size_t l = 0; l < postLayers.GetSize (); l++)
  {
    GetBucket (postLayers[l]->options);
  }

  csArray<csArray<csBitArray> > allUsedTextureBits;
  allUsedTextureBits.SetSize (buckets.GetSize());
  for (size_t b = 0; b < buckets.GetSize(); b++)
  {
    allUsedTextureBits[b].SetSize (postLayers.GetSize ());
  }
  
  for (size_t l = 0; l < postLayers.GetSize ()-1; l++)
  {
    size_t bucket = GetBucketIndex (postLayers[l]->options);
    csArray<csBitArray>& usedTextureBits = allUsedTextureBits[bucket];
    
    // Look for an unused texture
    size_t freeTexture;
    if (keepAllIntermediates || buckets[bucket].options.noTextureReuse)
      freeTexture = csArrayItemNotFound;
    else
      freeTexture = usedTextureBits[l].GetFirstBitUnset ();
    if (freeTexture == csArrayItemNotFound)
    {
      // Add a new texture
      freeTexture = usedTextureBits[l].GetSize();
      for (size_t b = l; b < postLayers.GetSize (); b++)
        usedTextureBits[b].SetSize (freeTexture+1);
    }
    
    postLayers[l]->outTextureNum = freeTexture;
    
    size_t lastLayer = l;
    // Look for last layer which has current layer as an input
    for (size_t l2 = postLayers.GetSize (); l2-- > l; )
    {
      if (postLayers[l2]->IsInput (postLayers[l]))
      {
        lastLayer = l2;
        break;
      }
    }
    // Mark texture as used
    for (size_t l2 = l; l2 <= lastLayer; l2++)
    {
      usedTextureBits[l2].SetBit (freeTexture);
    }
  }
  
  for (size_t b = 0; b < buckets.GetSize(); b++)
  {
    csArray<csBitArray>& usedTextureBits = allUsedTextureBits[b];
    buckets[b].textures.SetSize (usedTextureBits[postLayers.GetSize()-1].GetSize());
  }
}

void PostEffectManager::UpdateSVContexts ()
{
  for (size_t l = 0; l < postLayers.GetSize (); l++)
  {
    postLayers[l]->buffers.AttachNew (new csRenderBufferHolder);
    for (size_t i = 0; i < postLayers[l]->inputs.GetSize(); i++)
    {
      const LayerInputMap& input = postLayers[l]->inputs[i];
      
      size_t inBucket = GetBucketIndex (input.inputLayer->options);
      csRef<csShaderVariable> sv;
      sv.AttachNew (new csShaderVariable (svStrings->Request (
        input.textureName)));
      sv->SetValue (buckets[inBucket].textures[input.inputLayer->outTextureNum]);
      postLayers[l]->svContext->AddVariable (sv);
      
      csRenderBufferName bufferName =
        csRenderBuffer::GetBufferNameFromDescr (input.texcoordName);
      if (bufferName != CS_BUFFER_NONE)
        postLayers[l]->buffers->SetRenderBuffer (bufferName,
          buckets[inBucket].texcoordBuf);
      else
      {
        sv.AttachNew (new csShaderVariable (svStrings->Request (
          input.texcoordName)));
        postLayers[l]->svContext->AddVariable (sv);
      }
    }
    size_t thisBucket = GetBucketIndex (postLayers[l]->options);
    postLayers[l]->svContext->AddVariable (buckets[thisBucket].svPixelSize);
    postLayers[l]->buffers->SetRenderBuffer (CS_BUFFER_INDEX, indices);
    postLayers[l]->buffers->SetRenderBuffer (CS_BUFFER_POSITION,
      buckets[thisBucket].vertBuf);
  }
}

size_t PostEffectManager::GetBucketIndex (const LayerOptions& options)
{
  for (size_t i = 0; i < buckets.GetSize(); i++)
  {
    if (buckets[i].options == options) return i;
  }
  size_t index = buckets.GetSize();
  buckets.SetSize (index+1);
  buckets[index].options = options;
  return index;
}

//---------------------------------------------------------------------------

bool PostEffectManager::Layer::IsInput (const Layer* layer) const
{
  for (size_t i = 0; i < inputs.GetSize(); i++)
  {
    if (inputs[i].inputLayer == layer) return true;
  }
  return false;
}

//---------------------------------------------------------------------------

namespace
{
  static const char messageID[] = "crystalspace.posteffects.parser";
        
#define CS_TOKEN_ITEM_FILE "libs/csplugincommon/rendermanager/posteffects.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE
}

PostEffectLayersParser::PostEffectLayersParser (iObjectRegistry* objReg)
 : objReg (objReg)
{
  InitTokenTable (xmltokens);
  synldr = csQueryRegistry<iSyntaxService> (objReg);
  loader = csQueryRegistry<iLoader> (objReg);
}

bool PostEffectLayersParser::ParseInputs (iDocumentNode* node, 
                                          PostEffectManager& effects,
		                          ParsedLayers& layers,
                                          ShadersLayers& shaders, 
		                          InputsArray& inputs)
{
  csRef<iDocumentNodeIterator> inputsIt = node->GetNodes ("input");
  while (inputsIt->HasNext())
  {
    csRef<iDocumentNode> child = inputsIt->Next();
    if (child->GetType() != CS_NODE_ELEMENT) continue;
    
    const char* layerInpID = child->GetAttributeValue ("layer");
    if (!layerInpID || !*layerInpID)
    {
      synldr->ReportError (messageID, child, "Expected 'layer' attribute");
      return false;
    }
    PostEffectManager::Layer* inpLayer = 0;
    if (strcmp (layerInpID, "*screen") == 0)
      inpLayer = effects.GetScreenLayer();
    else
      inpLayer = layers.Get (layerInpID, 0);
    if (inpLayer == 0)
    {
      synldr->ReportError (messageID, child, "Invalid input layer");
      return false;
    }
    
    PostEffectManager::LayerInputMap inp;
    inp.inputLayer = inpLayer;
    if (child->GetAttribute ("texname").IsValid())
      inp.textureName = child->GetAttributeValue ("texname");
    if (child->GetAttribute ("texcoord").IsValid())
      inp.texcoordName = child->GetAttributeValue ("texcoord");
    inputs.Push (inp);
  }
  return true;
}

bool PostEffectLayersParser::ParseLayer (iDocumentNode* node, 
                                         PostEffectManager& effects,
		                         ParsedLayers& layers,
                                         ShadersLayers& shaders)
{
  const char* layerID = node->GetAttributeValue ("name");
  PostEffectManager::LayerOptions layerOpts;
  layerOpts.mipmap = node->GetAttributeValueAsBool ("mipmap", false);
  layerOpts.downsample = node->GetAttributeValueAsInt ("downsample");
  if (node->GetAttribute ("maxmipmap").IsValid())
    layerOpts.maxMipmap = node->GetAttributeValueAsInt ("maxmipmap");
    
  csRefArray<csShaderVariable> shaderVars;
  bool hasInputs = false;
  csDirtyAccessArray<PostEffectManager::LayerInputMap> inputs;
  csRef<iDocumentNodeIterator> it = node->GetNodes();
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next();
    if (child->GetType() != CS_NODE_ELEMENT) continue;
    
    csStringID id = xmltokens.Request (child->GetValue());
    switch (id)
    {
      case XMLTOKEN_INPUTS:
        {
          hasInputs = true;
          if (!ParseInputs (child, effects, layers, shaders, inputs))
            return false;
        }
	break;
      case XMLTOKEN_SHADERVAR:
        {
          csRef<csShaderVariable> sv;
          sv.AttachNew (new csShaderVariable);
          if (!synldr->ParseShaderVar (0, child, *sv))
            return false;
          shaderVars.Push (sv);
        }
        break;
      default:
	synldr->ReportBadToken (child);
	return false;
    }
  }
    
  const char* shader = node->GetAttributeValue ("shader");
  if (!shader || (*shader == 0))
  {
    synldr->ReportError (messageID, node, "Expected 'shader' attribute");
    return false;
  }
  csRef<iShader> shaderObj = shaders.Get (shader, 0);
  if (!shaderObj.IsValid())
  {
    shaderObj = loader->LoadShader (shader);
    if (!shaderObj.IsValid()) return false;
    shaders.Put (shader, shaderObj);
  }
  
  PostEffectManager::Layer* layer;
  if (hasInputs)
  {
    layer = effects.AddLayer (shaderObj, layerOpts, inputs.GetSize(),
      inputs.GetArray());
  }
  else
  {
    layer = effects.AddLayer (shaderObj, layerOpts);
  }
  if (layerID && *layerID)
    layers.Put (layerID, layer);
    
  for (size_t i = 0; i < shaderVars.GetSize(); i++)
    layer->GetSVContext()->AddVariable (shaderVars[i]);
    
  return true;
}

bool PostEffectLayersParser::AddLayersFromDocument (iDocumentNode* node, 
                                                    PostEffectManager& effects)
{
  ParsedLayers layers;
  ShadersLayers shaders;

  csRef<iDocumentNodeIterator> it = node->GetNodes();
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next();
    if (child->GetType() != CS_NODE_ELEMENT) continue;
    
    csStringID id = xmltokens.Request (child->GetValue());
    switch (id)
    {
      case XMLTOKEN_LAYER:
        if (!ParseLayer (child, effects, layers, shaders))
          return false;
	break;
      default:
	synldr->ReportBadToken (child);
	return false;
    }
  }
  
  return true;
}

bool PostEffectLayersParser::AddLayersFromFile (const char* filename, 
  PostEffectManager& effects)
{
  csRef<iDocumentSystem> docsys = csQueryRegistry<iDocumentSystem> (
    objReg);
  if (!docsys.IsValid())
    docsys.AttachNew (new csTinyDocumentSystem ());
  
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (objReg);
  CS_ASSERT(vfs);
  csRef<iFile> file = vfs->Open (filename, VFS_FILE_READ);
  if (!file)
  {
    csReport (objReg, CS_REPORTER_SEVERITY_WARNING, messageID,
      "Error opening '%s'", filename);
    return false;
  }
  
  csRef<iDocument> doc = docsys->CreateDocument();
  const char* error = doc->Parse (file);
  if (error != 0)
  {
    csReport (objReg, CS_REPORTER_SEVERITY_WARNING, messageID,
      "Error parsing '%s': %s", filename, error);
    return false;
  }
  
  csRef<iDocumentNode> docRoot = doc->GetRoot();
  if (!docRoot) return false;
  csRef<iDocumentNode> postEffectNode = docRoot->GetNode ("posteffect");
  if (!postEffectNode)
  {
    csReport (objReg, CS_REPORTER_SEVERITY_WARNING, messageID,
      "No <posteffect> in '%s'", filename);
    return false;
  }
  return AddLayersFromDocument (postEffectNode, effects);
}
