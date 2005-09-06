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

#include "ceguirenderer.h"
#include "ceguitexture.h"
#include "CEGUIExceptions.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csCEGUIRenderer)
  SCF_IMPLEMENTS_INTERFACE (iCEGUI)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csCEGUIRenderer)

// TODO add description
csCEGUIRenderer::csCEGUIRenderer (iBase *parent) :
  obj_reg(0),
  resourceProvider(0),
  queueing(true),
  m_bufferPos(0),
  texture(0)
{
  newQuadAdded = false;
  SCF_CONSTRUCT_IBASE (parent);
}

// TODO add description
bool csCEGUIRenderer::Initialize (iScript* script, int width, int height)
{
  g3d = CS_QUERY_REGISTRY (obj_reg, iGraphics3D);

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

  if (width < 1)
    width = g3d->GetWidth ();

  if (height < 1) 
    height = g3d->GetHeight ();

  m_displayArea.d_right = width;
  m_displayArea.d_bottom = height;

  g2d = g3d->GetDriver2D ();

  if (!g2d)
    return false;

  if (script)
  {
    scriptModule = new csCEGUIScriptModule (script, obj_reg);
    new CEGUI::System (this, scriptModule);
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
  delete resourceProvider;
  SCF_DESTRUCT_IBASE ();
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
    newQuadAdded = true;
    QuadInfo quad;
    quad.position = dest_rect;
    quad.position.d_bottom = m_displayArea.d_bottom - dest_rect.d_bottom;
    quad.position.d_top = m_displayArea.d_bottom - dest_rect.d_top;
    quad.z = z;
    quad.texid = (csCEGUITexture*) tex;
    quad.texPosition = texture_rect;
    quad.topLeftColor = ColorToCS(colours.d_top_left);
    quad.topRightColor = ColorToCS(colours.d_top_right);
    quad.bottomLeftColor = ColorToCS(colours.d_bottom_left);
    quad.bottomRightColor = ColorToCS(colours.d_bottom_right);

    quad.splitMode = quad_split_mode;

    meshIsValid = false;
    quadList.Push (quad);
  }
}

// TODO add description
void csCEGUIRenderer::doRender ()
{
  // If a new quad has been added since the last rendering
  // process the vertex buffer, create the meshes and cache it
  if(newQuadAdded)
  {
    // Add the new quads to meshes, empty the vertex buffer
    texture = 0;

    csArray<QuadInfo>::Iterator it = quadList.GetIterator();

    // iterate over each quad in the list
    while (it.HasNext())
    {
      const QuadInfo& quad = it.Next();

      if (texture != quad.texid)
      {
        meshIsValid = false;
        UpdateMeshList();
        texture = quad.texid;
      }

      PrepareQuad (quad, myBuff[m_bufferPos]);

      m_bufferPos++;

      if (m_bufferPos >= 2048)
      {
        UpdateMeshList();
      }
    }
    newQuadAdded = false;
    UpdateMeshList();
  }
  // Safe to clean up the quadList, all quads are now stored in meshes
  quadList.DeleteAll();
 
  // Render all meshes
  csPDelArray<csSimpleRenderMesh>::Iterator it = meshList.GetIterator();

  // iterate over each mesh in the list
  while (it.HasNext())
  {
    g3d->DrawSimpleMesh(*it.Next(), csSimpleMeshScreenspace);
  }
}

// TODO add description
void csCEGUIRenderer::clearRenderList(void)
{
  quadList.DeleteAll();
 
  csPDelArray<csSimpleRenderMesh>::Iterator it = meshList.GetIterator();

  // iterate over each mesh in the list
  while (it.HasNext())
  {
    const csSimpleRenderMesh *mesh = it.Next();

    delete[] mesh->texcoords;
    delete[] mesh->vertices;
    delete[] mesh->colors;
    delete[] mesh->indices;
  }
  meshList.DeleteAll();
   
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
  // if bufferPos is 0 there is no data in the buffer and nothing to render
  if (m_bufferPos == 0)
  {
    return;
  }

  csVector3 *verts = new csVector3[m_bufferPos*4];
  csVector2 *tex = new csVector2[m_bufferPos*4];
  csVector4 *col = new csVector4[m_bufferPos*4];
  uint *ind = new uint[m_bufferPos*6];

  csSimpleRenderMesh *mesh = new csSimpleRenderMesh();
  mesh->vertices = verts;
  mesh->vertexCount = m_bufferPos*4;
  mesh->indices = ind;
  mesh->indexCount = m_bufferPos*6;
  mesh->colors = col;
  mesh->meshtype = CS_MESHTYPE_TRIANGLES;
  mesh->texture = texture->GetTexHandle();
  mesh->texcoords = tex;

  csAlphaMode mode;
  mode.autoAlphaMode = false;
  mode.alphaType = mesh->texture->GetAlphaType ();
  mesh->alphaType = mode;

  int idx = 0, idx2 = 0;

  for (int i = 0; i < m_bufferPos; i++)
  {
    verts[idx] = myBuff[i].vertex[0];
    verts[idx+1] = myBuff[i].vertex[1];
    verts[idx+2] = myBuff[i].vertex[2];
    verts[idx+3] = myBuff[i].vertex[3];
    col[idx] = myBuff[i].color[0];
    col[idx+1] = myBuff[i].color[1];
    col[idx+2] = myBuff[i].color[2];
    col[idx+3] = myBuff[i].color[3];
    tex[idx] = myBuff[i].tex[0];
    tex[idx+1] = myBuff[i].tex[1];
    tex[idx+2] = myBuff[i].tex[2];
    tex[idx+3] = myBuff[i].tex[3];
    ind[idx2] = myBuff[i].indices[0]+idx;
    ind[idx2+1] = myBuff[i].indices[1]+idx;
    ind[idx2+2] = myBuff[i].indices[2]+idx;
    ind[idx2+3] = myBuff[i].indices[3]+idx;
    ind[idx2+4] = myBuff[i].indices[4]+idx;
    ind[idx2+5] = myBuff[i].indices[5]+idx;
    idx += 4;
    idx2 += 6;
  }

  meshIsValid = true;
  meshList.Push(mesh);

  // reset buffer position to 0...
  m_bufferPos = 0;
}

void csCEGUIRenderer::PrepareQuad (const QuadInfo quad, RenderQuad& rquad) const
{
  if (quad.splitMode == CEGUI::TopLeftToBottomRight)
  {
    rquad.indices[0] = 0;
    rquad.indices[1] = 2;
    rquad.indices[2] = 1;
    rquad.indices[3] = 3;
    rquad.indices[4] = 2;
    rquad.indices[5] = 0;
  }
  else
  {
    rquad.indices[0] = 0;
    rquad.indices[1] = 3;
    rquad.indices[2] = 1;
    rquad.indices[3] = 1;
    rquad.indices[4] = 3;
    rquad.indices[5] = 2;
  }

  rquad.vertex[0] = csVector3(quad.position.d_left, 
    g2d->GetHeight()-quad.position.d_top, quad.z);
  rquad.color[0] = quad.topLeftColor;
  rquad.tex[0] = csVector2(quad.texPosition.d_left, quad.texPosition.d_top);

  rquad.vertex[1] = csVector3(quad.position.d_left, 
    g2d->GetHeight()-quad.position.d_bottom, quad.z);
  rquad.color[1] = quad.bottomLeftColor;
  rquad.tex[1] = csVector2(quad.texPosition.d_left, quad.texPosition.d_bottom);

  rquad.vertex[2] = csVector3(quad.position.d_right, 
    g2d->GetHeight()-quad.position.d_bottom, quad.z);
  rquad.color[2] = quad.bottomRightColor;
  rquad.tex[2] = csVector2(quad.texPosition.d_right, quad.texPosition.d_bottom);

  rquad.vertex[3] = csVector3(quad.position.d_right, 
    g2d->GetHeight()-quad.position.d_top, quad.z);
  rquad.color[3] = quad.topRightColor;
  rquad.tex[3] = csVector2(quad.texPosition.d_right, quad.texPosition.d_top);
}

// TODO add description
void csCEGUIRenderer::RenderQuadDirect(const CEGUI::Rect& dest_rect, 
  float z, const CEGUI::Texture* tex, const CEGUI::Rect& texture_rect,
  const CEGUI::ColourRect& colours, CEGUI::QuadSplitMode quad_split_mode)
{
  QuadInfo quad;
  quad.position = dest_rect;
  quad.position.d_bottom = m_displayArea.d_bottom - dest_rect.d_bottom;
  quad.position.d_top = m_displayArea.d_bottom - dest_rect.d_top;
  quad.z = z;
  quad.texid = (csCEGUITexture*) tex;
  quad.texPosition = texture_rect;
  quad.topLeftColor = ColorToCS(colours.d_top_left);
  quad.topRightColor = ColorToCS(colours.d_top_right);
  quad.bottomLeftColor = ColorToCS(colours.d_bottom_left);
  quad.bottomRightColor = ColorToCS(colours.d_bottom_right);

  quad.splitMode = quad_split_mode;

  RenderQuad myquad;

  PrepareQuad (quad, myquad);

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

  verts[0] = myquad.vertex[0];
  verts[1] = myquad.vertex[1];
  verts[2] = myquad.vertex[2];
  verts[3] = myquad.vertex[3];
  col[0] = myquad.color[0];
  col[1] = myquad.color[1];
  col[2] = myquad.color[2];
  col[3] = myquad.color[3];
  texcoords[0] = myquad.tex[0];
  texcoords[1] = myquad.tex[1];
  texcoords[2] = myquad.tex[2];
  texcoords[3] = myquad.tex[3];
  ind[0] = myquad.indices[0];
  ind[1] = myquad.indices[1];
  ind[2] = myquad.indices[2];
  ind[3] = myquad.indices[3];
  ind[4] = myquad.indices[4];
  ind[5] = myquad.indices[5];

  g3d->DrawSimpleMesh (mesh, csSimpleMeshScreenspace);
}

csVector4 csCEGUIRenderer::ColorToCS (const CEGUI::colour& col) const
{
  csVector4 color (col.getRed(), col.getGreen(), col.getBlue(), col.getAlpha());
  return color;
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
  if (!resourceProvider) {
    resourceProvider = new csCEGUIResourceProvider (obj_reg);
  }

  return resourceProvider;
}
