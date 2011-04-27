/*
    Copyright (C) 2005 Dan Hardfeldt, Seth Yastrov and Jelle Hellemans

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

#include "ceguiimports.h"
#include "ceguigeometrybuffer.h"
#include "ceguitexture.h"

CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{

  //----------------------------------------------------------------------------//
  GeometryBuffer::GeometryBuffer(iObjectRegistry* reg) :
    d_activeTexture(0),
    d_translation(0, 0, 0),
    d_rotation(0, 0, 0),
    d_pivot(0, 0, 0),
    d_effect(0),
    d_texelOffset(0, 0),
    d_matrixValid(false),
    d_sync(false)
  {
    obj_reg = reg;
    bufHolder.AttachNew (new csRenderBufferHolder);
    g3d = csQueryRegistry<iGraphics3D> (obj_reg);
    g2d = g3d->GetDriver2D();
  }

  //----------------------------------------------------------------------------//
  GeometryBuffer::~GeometryBuffer()
  {
  }

  //----------------------------------------------------------------------------//
  void GeometryBuffer::draw() const
  {
    // Set up clipping for this buffer
    // TODO: need to round to the closer 'int'?
    g2d->SetClipRect((int) d_clipRect.d_left, (int) d_clipRect.d_top,
                     (int) d_clipRect.d_right, (int) d_clipRect.d_bottom);

    if (!d_sync)
      syncHardwareBuffer();

    if (!d_matrixValid)
      updateMatrix();

    const int pass_count = d_effect ? d_effect->getPassCount() : 1;
    for (int pass = 0; pass < pass_count; ++pass)
    {
      // set up RenderEffect
      if (d_effect)
        d_effect->performPreRenderFunctions(pass);

      // Render all meshes 
      g3d->DrawSimpleMeshes(renderMeshes.GetArray(), renderMeshes.GetSize(), csSimpleMeshScreenspace);
    }

    // clean up RenderEffect
    if (d_effect)
      d_effect->performPostRenderFunctions();
  }

  //----------------------------------------------------------------------------//
  void GeometryBuffer::setTranslation(const CEGUI::Vector3& v)
  {
    d_translation = v;
    d_matrixValid = false;
  }

  //----------------------------------------------------------------------------//
  void GeometryBuffer::setRotation(const CEGUI::Vector3& r)
  {
    d_rotation = r;
    d_matrixValid = false;
  }

  //----------------------------------------------------------------------------//
  void GeometryBuffer::setPivot(const CEGUI::Vector3& p)
  {
    d_pivot = p;
    d_matrixValid = false;
  }

  //----------------------------------------------------------------------------//
  void GeometryBuffer::setClippingRegion(const CEGUI::Rect& region)
  {
    d_clipRect = region;
  }

  //----------------------------------------------------------------------------//
  void GeometryBuffer::appendVertex(const CEGUI::Vertex& vertex)
  {
    appendGeometry(&vertex, 1);
  }

  //----------------------------------------------------------------------------//
  void GeometryBuffer::appendGeometry(const CEGUI::Vertex* const vbuff,
                                        uint vertex_count)
  {
    iTextureHandle* t = 0;
    if (d_activeTexture)
      t = d_activeTexture->GetTexHandle();

    if (renderMeshes.IsEmpty() || renderMeshes.Top().texture != t)
    {
      csSimpleRenderMesh simple;
      simple.renderBuffers = bufHolder;
      simple.meshtype = CS_MESHTYPE_TRIANGLES;
      simple.texture = t;

      csAlphaMode mode;
      mode.autoAlphaMode = false;
      mode.alphaType = t->GetAlphaType ();
      simple.alphaType = mode;

      simple.object2world = d_matrix.GetTransform();

      simple.indexStart = (int)indexBuf.GetSize();
      simple.indexEnd = simple.indexStart;

      renderMeshes.Push(simple);
    }

    // Update index range
    renderMeshes.Top().indexEnd += vertex_count;

    // buffer these vertices
    size_t currentIndex = indexBuf.GetSize();
    for (uint i = 0; i < vertex_count; ++i)
    {
      const CEGUI::Vertex& vs = vbuff[i];
      // convert from CEGUI::Vertex to something directly usable by CS.
      indexBuf.Push((int)vertBuf.GetSize());
      csVector3 v(vs.position.d_x + d_texelOffset.d_x, 
                  vs.position.d_y + d_texelOffset.d_y, 
                  vs.position.d_z);
      vertBuf.Push(v);
      csVector4 c(ColorToCS(vs.colour_val));
      colBuf.Push(c);
      csVector2 tc(vs.tex_coords.d_x, vs.tex_coords.d_y);
      tcBuf.Push(tc);

      // Reverse winding order.
      if ((currentIndex+i)%3 == 2) 
        std::swap(indexBuf[currentIndex+i], indexBuf[currentIndex+i-1]);
    }

    d_sync = false;
  }

  //----------------------------------------------------------------------------//
  void GeometryBuffer::setActiveTexture(CEGUI::Texture* texture)
  {
    d_activeTexture = static_cast<Texture*>(texture);
  }

  //----------------------------------------------------------------------------//
  void GeometryBuffer::reset()
  {
    renderMeshes.Empty();
    tcBuf.Empty();
    colBuf.Empty();
    vertBuf.Empty();
    indexBuf.Empty();
    d_activeTexture = 0;
    d_sync = false;
  }

  //----------------------------------------------------------------------------//
  CEGUI::Texture* GeometryBuffer::getActiveTexture() const
  {
    return d_activeTexture;
  }

  //----------------------------------------------------------------------------//
  uint GeometryBuffer::getVertexCount() const
  {
    return (int)vertBuf.GetSize();
  }

  //----------------------------------------------------------------------------//
  uint GeometryBuffer::getBatchCount() const
  {
    return (int)renderMeshes.GetSize();
  }

  //----------------------------------------------------------------------------//
  void GeometryBuffer::setRenderEffect(CEGUI::RenderEffect* effect)
  {
    d_effect = effect;
  }

  //----------------------------------------------------------------------------//
  CEGUI::RenderEffect* GeometryBuffer::getRenderEffect()
  {
    return d_effect;
  }

  //----------------------------------------------------------------------------//
  void GeometryBuffer::updateMatrix() const
  {
    // translation to position geometry and offset to pivot point
    csReversibleTransform trans;
    trans.Translate(csVector3(d_translation.d_x + d_pivot.d_x,
      d_translation.d_y + d_pivot.d_y,
      d_translation.d_z + d_pivot.d_z));

    // rotation
    csQuaternion qx, qy, qz;
    qx.SetAxisAngle(csVector3(1, 0, 0), d_rotation.d_x);
    qy.SetAxisAngle(csVector3(0, 1, 0), d_rotation.d_y);
    qz.SetAxisAngle(csVector3(0, 0, 1), d_rotation.d_z);
    csQuaternion r = qz * qy * qx;
    CS::Math::Matrix4 rot(r.GetMatrix());

    // translation to remove rotation pivot offset
    csReversibleTransform inv_pivot_trans;
    inv_pivot_trans.Translate(csVector3(-d_pivot.d_x,
      -d_pivot.d_y,
      -d_pivot.d_z));

    // calculate final matrix
    d_matrix = trans * rot * inv_pivot_trans;

    // Update mesh transforms
    for (size_t i = 0; i != renderMeshes.GetSize(); i++)
      renderMeshes.Get(i).object2world = d_matrix.GetTransform();

    d_matrixValid = true;
  }

  //----------------------------------------------------------------------------//
  void GeometryBuffer::syncHardwareBuffer() const
  {
    // Reallocate h/w buffer as requied
    csRef<iRenderBuffer> rbuf;
    rbuf = csRenderBuffer::CreateRenderBuffer (vertBuf.GetSize(),
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
    rbuf->SetData (vertBuf.GetArray());
    bufHolder->SetRenderBuffer (CS_BUFFER_POSITION, rbuf);

    rbuf = csRenderBuffer::CreateRenderBuffer (colBuf.GetSize(),
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 4);
    rbuf->SetData (colBuf.GetArray());
    bufHolder->SetRenderBuffer (CS_BUFFER_COLOR, rbuf);

    rbuf = csRenderBuffer::CreateRenderBuffer (tcBuf.GetSize(),
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2);
    rbuf->SetData (tcBuf.GetArray());
    bufHolder->SetRenderBuffer (CS_BUFFER_TEXCOORD0, rbuf);

    rbuf = csRenderBuffer::CreateIndexRenderBuffer (indexBuf.GetSize(),
      CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_INT, 0, indexBuf.GetSize()-1);
    rbuf->SetData (indexBuf.GetArray());
    bufHolder->SetRenderBuffer (CS_BUFFER_INDEX, rbuf);

    d_sync = true;
  }

  //----------------------------------------------------------------------------//
  csVector4 GeometryBuffer::ColorToCS (const CEGUI::colour& col) const
  {
    csVector4 color (col.getRed(), col.getGreen(), col.getBlue(), col.getAlpha());
    return color;
  }

  //----------------------------------------------------------------------------//
  const CS::Math::Matrix4& GeometryBuffer::getMatrix() const
  {
    if (!d_matrixValid)
      updateMatrix();

    return d_matrix;
  }

  //----------------------------------------------------------------------------//

} CS_PLUGIN_NAMESPACE_END(cegui)
