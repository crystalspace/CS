/*
    Copyright (C) 2005 Dan Hardfeldt and Seth Yastrov

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

#include "iutil/objreg.h"
#include "ivaria/script.h"
#include "ivideo/txtmgr.h"

#include "csgfx/renderbuffer.h"

#include "ceguirenderer.h"
#include "ceguitexture.h"
#include "CEGUIExceptions.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csCEGUIRenderer)

// TODO add description
csCEGUIRenderer::csCEGUIRenderer (iBase *parent) :
  scfImplementationType (this, parent),
  obj_reg(0),
  events(0),
  scriptModule(0),
  buffersDirty (true),
  queueing(true),
  queueStart(0),
  texture(0)
{
  d_identifierString = "Crystal Space Renderer";
  d_resourceProvider = 0;

  bufHolder.AttachNew (new csRenderBufferHolder);
}

// TODO add description
bool csCEGUIRenderer::Initialize (iScript* script)
{
  g3d = csQueryRegistry<iGraphics3D> (obj_reg);

  if (!g3d) {
    return false;
  }

  int w, h, a;

  // Initialize maximum texture size, CEGUI wants squares
  g3d->GetTextureManager ()->GetMaxTextureSize (w, h, a);

  if (w < h)
    m_maxTextureSize = w;
  else
    m_maxTextureSize = h;

  m_displayArea.d_left = 0;
  m_displayArea.d_top = 0;
  m_displayArea.d_right = g3d->GetWidth ();
  m_displayArea.d_bottom = g3d->GetHeight ();

  g2d = g3d->GetDriver2D ();

  if (!g2d)
    return false;

  if (script)
  {
    scriptModule = new csCEGUIScriptModule (script, obj_reg);
    new CEGUI::System (this, 0, 0, scriptModule);  
  }
  else
  {
    new CEGUI::System (this);
  }

  g2d->SetMouseCursor (csmcNone);
  events = new csCEGUIEventHandler (obj_reg, this);
  events->Initialize ();

  return true;
}

// TODO add description
csCEGUIRenderer::~csCEGUIRenderer ()
{
  destroyAllTextures();
  clearRenderList();
  delete CEGUI::System::getSingletonPtr();
  delete scriptModule;
  delete events;
}

// TODO add description
void csCEGUIRenderer::addQuad (const CEGUI::Rect& dest_rect, float z, 
  const CEGUI::Texture* tex, const CEGUI::Rect& texture_rect,
  const CEGUI::ColourRect& colours, CEGUI::QuadSplitMode quad_split_mode)
{
  if (!queueing)
  {
    RenderQuadDirect (dest_rect, z, tex, texture_rect, colours, quad_split_mode);
  }
  else
  {
    const csCEGUITexture* cstex = static_cast<const csCEGUITexture*> (tex);
    if (texture != cstex)
    {
      UpdateMeshList();
      texture = cstex;
    }

    CEGUI::Rect	position;
    position = dest_rect;
    position.d_bottom = m_displayArea.d_bottom - dest_rect.d_bottom;
    position.d_top = m_displayArea.d_bottom - dest_rect.d_top;

    uint firstVertex = uint (vertBuf.GetSize());
    if (quad_split_mode == CEGUI::TopLeftToBottomRight)
    {
      indexBuf.Push (firstVertex+0);
      indexBuf.Push (firstVertex+2);
      indexBuf.Push (firstVertex+1);
      indexBuf.Push (firstVertex+3);
      indexBuf.Push (firstVertex+2);
      indexBuf.Push (firstVertex+0);
    }
    else
    {
      indexBuf.Push (firstVertex+0);
      indexBuf.Push (firstVertex+3);
      indexBuf.Push (firstVertex+1);
      indexBuf.Push (firstVertex+1);
      indexBuf.Push (firstVertex+3);
      indexBuf.Push (firstVertex+2);
    }

    vertBuf.Push (csVector3(position.d_left, 
      g2d->GetHeight()-position.d_top, 0/*z*/));
    colBuf.Push (ColorToCS(colours.d_top_left));
    tcBuf.Push (csVector2(texture_rect.d_left, texture_rect.d_top));

    vertBuf.Push (csVector3(position.d_left, 
      g2d->GetHeight()-position.d_bottom, 0/*z*/));
    colBuf.Push (ColorToCS(colours.d_bottom_left));
    tcBuf.Push (csVector2(texture_rect.d_left, texture_rect.d_bottom));

    vertBuf.Push (csVector3(position.d_right, 
      g2d->GetHeight()-position.d_bottom, 0/*z*/));
    colBuf.Push (ColorToCS(colours.d_bottom_right));
    tcBuf.Push (csVector2(texture_rect.d_right, texture_rect.d_bottom));

    vertBuf.Push (csVector3(position.d_right, 
      g2d->GetHeight()-position.d_top, 0/*z*/));
    colBuf.Push (ColorToCS(colours.d_top_right));
    tcBuf.Push (csVector2(texture_rect.d_right, texture_rect.d_top));

    buffersDirty = true;
  }
}

// TODO add description
void csCEGUIRenderer::doRender ()
{
  if (queueStart < indexBuf.GetSize())
    UpdateMeshList();

  /* The vertex data buffers have been changed, recreate render buffer objects */
  if (buffersDirty)
  {
    csRef<iRenderBuffer> rbuf;
    rbuf = csRenderBuffer::CreateRenderBuffer (vertBuf.GetSize(),
      CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);
    rbuf->SetData (vertBuf.GetArray());
    bufHolder->SetRenderBuffer (CS_BUFFER_POSITION, rbuf);

    rbuf = csRenderBuffer::CreateRenderBuffer (colBuf.GetSize(),
      CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 4);
    rbuf->SetData (colBuf.GetArray());
    bufHolder->SetRenderBuffer (CS_BUFFER_COLOR, rbuf);

    rbuf = csRenderBuffer::CreateRenderBuffer (tcBuf.GetSize(),
      CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 2);
    rbuf->SetData (tcBuf.GetArray());
    bufHolder->SetRenderBuffer (CS_BUFFER_TEXCOORD0, rbuf);

    rbuf = csRenderBuffer::CreateIndexRenderBuffer (indexBuf.GetSize(),
      CS_BUF_STREAM, CS_BUFCOMP_UNSIGNED_INT, 0, vertBuf.GetSize()-1);
    rbuf->SetData (indexBuf.GetArray());
    bufHolder->SetRenderBuffer (CS_BUFFER_INDEX, rbuf);

    buffersDirty = false;
  }

  // Render all meshes
  g3d->DrawSimpleMeshes (meshList.GetArray(), meshList.GetSize(), csSimpleMeshScreenspace);
}

// TODO add description
void csCEGUIRenderer::clearRenderList(void)
{
  meshList.Empty ();
  tcBuf.Empty ();
  colBuf.Empty ();
  vertBuf.Empty ();
  indexBuf.Empty ();
  buffersDirty = true;
  queueStart = 0;
}

// TODO add description
CEGUI::Texture* csCEGUIRenderer::createTexture(void)
{
  csCEGUITexture* tex = new csCEGUITexture (this, obj_reg);
  textureList.Push (tex);
  return tex;
}

// TODO add description
CEGUI::Texture* csCEGUIRenderer::createTexture (
  const CEGUI::String& filename, const CEGUI::String& resourceGroup)
{
  csCEGUITexture* tex = (csCEGUITexture*) createTexture();
  tex->loadFromFile (filename, resourceGroup);

  return tex;
}

// TODO add description
CEGUI::Texture* csCEGUIRenderer::createTexture (float size)
{
  csCEGUITexture* tex = (csCEGUITexture*) createTexture();
  return tex;
}

// TODO add description
void csCEGUIRenderer::destroyTexture (CEGUI::Texture* texture)
{
  if (texture)
  {
    textureList.Delete ((csCEGUITexture*) texture);
  }
}

// TODO add description
void csCEGUIRenderer::destroyAllTextures ()
{
  textureList.DeleteAll ();
}

// Convert all quads into meshes, store mesh in meshlist
void csCEGUIRenderer::UpdateMeshList()
{
  if ((queueStart == indexBuf.GetSize()) || (texture == 0))
    // empty or invalid quads
    return;

  csSimpleRenderMesh mesh;
  mesh.renderBuffers = bufHolder;
  mesh.meshtype = CS_MESHTYPE_TRIANGLES;
  mesh.texture = texture->GetTexHandle();

  csAlphaMode mode;
  mode.autoAlphaMode = false;
  mode.alphaType = mesh.texture->GetAlphaType ();
  mesh.alphaType = mode;

  mesh.indexStart = queueStart;
  queueStart = indexBuf.GetSize();
  mesh.indexEnd = queueStart;

  meshList.Push(mesh);
}

// TODO add description
void csCEGUIRenderer::RenderQuadDirect(const CEGUI::Rect& dest_rect, 
  float z, const CEGUI::Texture* tex, const CEGUI::Rect& texture_rect,
  const CEGUI::ColourRect& colours, CEGUI::QuadSplitMode quad_split_mode)
{
  CEGUI::Rect position;
  position = dest_rect;
  position.d_bottom = m_displayArea.d_bottom - dest_rect.d_bottom;
  position.d_top = m_displayArea.d_bottom - dest_rect.d_top;
  const csCEGUITexture* texid = static_cast<const csCEGUITexture*> (tex);

  csVector3 verts[4];
  csVector2 texcoords[4];
  csVector4 col[4];
  uint ind[6];

  csSimpleRenderMesh mesh;
  mesh.vertices = verts;
  mesh.vertexCount = 4;
  mesh.indices = ind;
  mesh.indexCount = 6;
  mesh.colors = col;
  mesh.texcoords = texcoords;
  mesh.meshtype = CS_MESHTYPE_TRIANGLES;
  mesh.texture = ((csCEGUITexture*)tex)->GetTexHandle();
  
  csAlphaMode mode;
  mode.autoAlphaMode = false;
  mode.alphaType = mesh.texture->GetAlphaType ();
  mesh.alphaType = mode;

  verts[0].Set (position.d_left, 
    g2d->GetHeight()-position.d_top, 0/*z*/);
  verts[1].Set (position.d_left, 
    g2d->GetHeight()-position.d_bottom, 0/*z*/);
  verts[2].Set (position.d_right, 
    g2d->GetHeight()-position.d_bottom, 0/*z*/);
  verts[3].Set (position.d_right, 
    g2d->GetHeight()-position.d_top, 0/*z*/);

  col[0].Set (ColorToCS(colours.d_top_left));
  col[1].Set (ColorToCS(colours.d_bottom_left));
  col[2].Set (ColorToCS(colours.d_bottom_right));
  col[3].Set (ColorToCS(colours.d_top_right));

  texcoords[0].Set (texture_rect.d_left, texture_rect.d_top);
  texcoords[1].Set (texture_rect.d_left, texture_rect.d_bottom);
  texcoords[2].Set (texture_rect.d_right, texture_rect.d_bottom);
  texcoords[3].Set (texture_rect.d_right, texture_rect.d_top);

  if (quad_split_mode == CEGUI::TopLeftToBottomRight)
  {
    ind[0] = 0;
    ind[1] = 2;
    ind[2] = 1;
    ind[3] = 3;
    ind[4] = 2;
    ind[5] = 0;
  }
  else
  {
    ind[0] = 0;
    ind[1] = 3;
    ind[2] = 1;
    ind[3] = 1;
    ind[4] = 3;
    ind[5] = 2;
  }

  g3d->DrawSimpleMesh (mesh, csSimpleMeshScreenspace);
}

csVector4 csCEGUIRenderer::ColorToCS (const CEGUI::colour& col) const
{
  csVector4 color (col.getRed(), col.getGreen(), col.getBlue(), col.getAlpha());
  return color;
}

/// Allow CEGUI to capture mouse events.
void csCEGUIRenderer::EnableMouseCapture ()
{
  events->EnableMouseCapture ();
}

/// Keep CEGUI from capturing mouse events.
void csCEGUIRenderer::DisableMouseCapture ()
{
  events->DisableMouseCapture ();
}

/// Allow CEGUI to capture keyboard events.
void csCEGUIRenderer::EnableKeyboardCapture ()
{
  events->EnableKeyboardCapture ();
}

/// Keep CEGUI from capturing keyboard events.
void csCEGUIRenderer::DisableKeyboardCapture ()
{
  events->DisableKeyboardCapture ();
}

// TODO add description
void csCEGUIRenderer::setDisplaySize (const CEGUI::Size& sz)
{
  if (m_displayArea.getSize() != sz)
  {
    m_displayArea.setSize(sz);

    CEGUI::EventArgs args;
    fireEvent(EventDisplaySizeChanged, args, EventNamespace);
  }
}
  
CEGUI::ResourceProvider* csCEGUIRenderer::createResourceProvider ()
{
  if (!d_resourceProvider) {
    d_resourceProvider = new csCEGUIResourceProvider (obj_reg);
  }

  return d_resourceProvider;
}
