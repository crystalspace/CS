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

#include "ivaria/view.h"
#include "csutil/objreg.h"
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"
#include "csplugincommon/rendermanager/posteffects.h"


using namespace CS::RenderManager;

PostEffectManager::PostEffectManager ()
  : screenQuadVerts (0), screenQuadTex (0), currentWidth (0), currentHeight (0),
  textureCoordinateX (1), textureCoordinateY (1), 
  textureOffsetX (0), textureOffsetY (0), activeTexture (0)
{}

PostEffectManager::~PostEffectManager ()
{
  KillScreenQuad ();
}

void PostEffectManager::Initialize (iObjectRegistry* objectReg)
{
  graphics3D = csQueryRegistry<iGraphics3D> (objectReg);
}

void PostEffectManager::SetupView (iView* view)
{
  unsigned int width = view->GetContext ()->GetWidth ();
  unsigned int height = view->GetContext ()->GetHeight ();

  if (width != currentWidth || height != currentHeight)
  {
    currentWidth = width;
    currentHeight = height;

    AllocatePingpongTextures ();
    SetupScreenQuad (width, height);    
  }
}

iTextureHandle* PostEffectManager::GetScreenTarget ()
{
  if (postLayers.GetSize () > 0)
  {
    return textures[activeTexture];
  }

  return 0;
}

void PostEffectManager::DrawPostEffects ()
{ 
  graphics3D->FinishDraw ();

  for (size_t layer = 0; layer < postLayers.GetSize (); ++layer)
  {   
    // Draw and ping-pong   
    fullscreenQuad.texture = textures[activeTexture];
    fullscreenQuad.shader = postLayers[layer].effectShader;

    activeTexture ^= 1;
    graphics3D->SetRenderTarget (layer < postLayers.GetSize () - 1 ? textures[activeTexture] : 0);
    graphics3D->BeginDraw (CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER | CSDRAW_3DGRAPHICS);
    graphics3D->DrawSimpleMesh (fullscreenQuad, csSimpleMeshScreenspace);
    graphics3D->FinishDraw ();
  }
}

void PostEffectManager::SetupScreenQuad (unsigned int width, unsigned int height)
{
  // Deallocate any built-in data
  KillScreenQuad ();

  fullscreenQuad.meshtype = CS_MESHTYPE_QUADS;
  
  screenQuadVerts = static_cast<csVector3*> (cs_malloc(sizeof(csVector3)*4));
  fullscreenQuad.vertices = screenQuadVerts;
  fullscreenQuad.vertexCount = 4;

  screenQuadTex = static_cast<csVector2*> (cs_malloc(sizeof(csVector2)*4));
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

void PostEffectManager::KillScreenQuad ()
{
  cs_free (screenQuadVerts);
  cs_free (screenQuadTex);
}

void PostEffectManager::AllocatePingpongTextures ()
{
  textures[0] = graphics3D->GetTextureManager ()->CreateTexture (currentWidth, currentHeight, 
    csimg2D, "rgb8", CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_NPOTS | CS_TEXTURE_CLAMP | CS_TEXTURE_SCALE_UP);

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

  textures[1] = graphics3D->GetTextureManager ()->CreateTexture (currentWidth, currentHeight, 
    csimg2D, "rgb8", resultFlags);
}
