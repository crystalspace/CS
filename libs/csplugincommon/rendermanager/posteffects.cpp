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

#include "csgfx/shadervarcontext.h"
#include "csutil/objreg.h"
#include "ivaria/view.h"
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"

#include "csplugincommon/rendermanager/posteffects.h"

#include <stdarg.h>

using namespace CS::RenderManager;

PostEffectManager::PostEffectManager ()
  : currentWidth (0), currentHeight (0),
  textureCoordinateX (1), textureCoordinateY (1), 
  textureOffsetX (0), textureOffsetY (0), textureFmt ("rgba8"),
  lastLayer (0)
{
  AddLayer (0, 0);
}

PostEffectManager::~PostEffectManager ()
{
}

void PostEffectManager::Initialize (iObjectRegistry* objectReg)
{
  graphics3D = csQueryRegistry<iGraphics3D> (objectReg);
  svStrings = csQueryRegistryTagInterface<iShaderVarStringSet> (objectReg,
    "crystalspace.shader.variablenameset");
  svPixelSize.AttachNew (new csShaderVariable (svStrings->Request ("pixel size")));
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
    UpdateSVContexts ();
    SetupScreenQuad (width, height);
  }
}

iTextureHandle* PostEffectManager::GetScreenTarget ()
{
  if (postLayers.GetSize () > 1)
  {
    return textures[postLayers[0]->outTextureNum];
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

    graphics3D->SetRenderTarget (layer < postLayers.GetSize () - 1 
      ? textures[postLayers[layer]->outTextureNum] : 0);
    graphics3D->BeginDraw (CSDRAW_CLEARZBUFFER | CSDRAW_3DGRAPHICS);
    graphics3D->DrawSimpleMesh (fullscreenQuad, csSimpleMeshScreenspace);
    graphics3D->FinishDraw ();
  }
}
    
PostEffectManager::Layer* PostEffectManager::AddLayer (iShader* shader)
{
  return AddLayer (shader, 1, lastLayer, "tex diffuse");
}

PostEffectManager::Layer* PostEffectManager::AddLayer (iShader* shader, const LayerInputMap& inputs)
{
  Layer* newLayer = new Layer;
  newLayer->effectShader = shader;
  newLayer->inputs = inputs;
  postLayers.Push (newLayer);
  textureDistributionDirty = true;
  lastLayer = newLayer;
  return newLayer;
}

PostEffectManager::Layer* PostEffectManager::AddLayer (iShader* shader, size_t numMaps, ...)
{
  LayerInputMap inputs;

  va_list va;
  va_start (va, numMaps);
  for (size_t i = 0; i < numMaps; i++)
  {
    Layer* layer = va_arg (va, Layer*);
    const char* dst = va_arg (va, const char*);
    inputs.Put (dst, layer);
  }
  va_end (va);
  return AddLayer (shader, inputs);
}

void PostEffectManager::SetupScreenQuad (unsigned int width, unsigned int height)
{
  fullscreenQuad.meshtype = CS_MESHTYPE_QUADS;
  
  fullscreenQuad.vertices = screenQuadVerts;
  fullscreenQuad.vertexCount = 4;
  fullscreenQuad.texcoords = screenQuadTex;

  // Setup the vertices & texcoords
  screenQuadVerts[0].Set (0, 0, 0);
  screenQuadTex[0].Set (textureOffsetX, textureOffsetY);

  screenQuadVerts[1].Set (width, 0, 0);
  screenQuadTex[1].Set (textureCoordinateX + textureOffsetX, textureOffsetY);

  screenQuadVerts[2].Set (width, height, 0);
  screenQuadTex[2].Set (textureCoordinateX + textureOffsetX, 
    textureCoordinateY + textureOffsetY);

  screenQuadVerts[3].Set (0, height, 0);
  screenQuadTex[3].Set (textureOffsetX, textureCoordinateY + textureOffsetY);
}

void PostEffectManager::AllocatePingpongTextures ()
{
  csRef<iTextureHandle> t;
  t = graphics3D->GetTextureManager ()->CreateTexture (currentWidth, currentHeight, 
    csimg2D, textureFmt, 
    CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_NPOTS | CS_TEXTURE_CLAMP | CS_TEXTURE_SCALE_UP);
  textures.Put (0, t);

  textureOffsetX = textureOffsetY = 0;

  // Check if we actually got NPOTs textures
  int resultFlags = textures[0]->GetFlags ();
  if (!(resultFlags & CS_TEXTURE_NPOTS))
  {
    // Handle that we didn't get a power of 2 texture
    // Means the texture is bigger and we need to use <1 for texture coordinate
    int tw, th, td;
    textures[0]->GetRendererDimensions (tw, th, td);

    textureCoordinateX = currentWidth / (float)tw;
    textureCoordinateY = currentHeight / (float)th;

    textureOffsetY = 1-textureCoordinateY;
  }
  else if (textures[0]->GetTextureType () == iTextureHandle::texTypeRect)
  {
    // Handle a rect texture
    // Rect texture use non-normalized texture coordinates
    textureCoordinateX = currentWidth;
    textureCoordinateY = currentHeight;
  }
  else
  {
    // NPOT texture, coordinates are 0-1,0-1
    textureCoordinateX = 1;
    textureCoordinateY = 1;
  }  
  svPixelSize->SetValue (csVector2 (textureCoordinateX/currentWidth, 
    textureCoordinateY/currentHeight));

  for (size_t i = 1; i < textures.GetSize(); i++)
  {
    t = graphics3D->GetTextureManager ()->CreateTexture (currentWidth, currentHeight, 
      csimg2D, textureFmt, resultFlags);
    textures.Put (i, t);
  }
}

void PostEffectManager::UpdateTextureDistribution()
{
  csArray<csBitArray> usedTextureBits;
  usedTextureBits.SetSize (postLayers.GetSize ());
  
  for (size_t l = 0; l < postLayers.GetSize ()-1; l++)
  {
    // Look for an unused texture
    size_t freeTexture = usedTextureBits[l].GetFirstBitUnset ();
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
  textures.SetSize (usedTextureBits[postLayers.GetSize()-1].GetSize());
}

void PostEffectManager::UpdateSVContexts ()
{
  for (size_t l = 0; l < postLayers.GetSize (); l++)
  {
    postLayers[l]->svContext.AttachNew (new csShaderVariableContext);
    LayerInputMap::GlobalIterator iter (postLayers[l]->inputs.GetIterator());
    while (iter.HasNext())
    {
      csString texName;
      Layer* input = iter.Next (texName);
      
      csRef<csShaderVariable> sv;
      sv.AttachNew (new csShaderVariable (svStrings->Request (texName)));
      sv->SetValue (textures[input->outTextureNum]);
      postLayers[l]->svContext->AddVariable (sv);
      postLayers[l]->svContext->AddVariable (svPixelSize);
    }
  }
}

//---------------------------------------------------------------------------

bool PostEffectManager::Layer::IsInput (const Layer* layer) const
{
  LayerInputMap::ConstGlobalIterator iter (inputs.GetIterator());
  while (iter.HasNext())
  {
    if (iter.Next() == layer) return true;
  }
  return false;
}
