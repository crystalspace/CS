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

#ifndef _CEGUIGEOMETRYBUFFER_H_
#define _CEGUIGEOMETRYBUFFER_H_

#include "crystalspace.h"
#include "ceguiimports.h"
#include "ceguirenderer.h"

#include <utility>
#include <vector>
#include <list>

CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
  /// Implementation of CEGUI::GeometryBuffer for the CS engine
  class GeometryBuffer : public CEGUI::GeometryBuffer
  {
  public:
    /// Constructor
    GeometryBuffer(iObjectRegistry* reg);
    /// Destructor
    virtual ~GeometryBuffer();

    /// return the transformation matrix used for this buffer.
    const CS::Math::Matrix4& getMatrix() const;

    // implement CEGUI::GeometryBuffer interface.
    void draw() const;
    void setTranslation(const CEGUI::Vector3& v);
    void setRotation(const CEGUI::Vector3& r);
    void setPivot(const CEGUI::Vector3& p);
    void setClippingRegion(const CEGUI::Rect& region);
    void appendVertex(const CEGUI::Vertex& vertex);
    void appendGeometry(const CEGUI::Vertex* const vbuff, uint vertex_count);
    void setActiveTexture(CEGUI::Texture* texture);
    void reset();
    CEGUI::Texture* getActiveTexture() const;
    uint getVertexCount() const;
    uint getBatchCount() const;
    void setRenderEffect(CEGUI::RenderEffect* effect);
    CEGUI::RenderEffect* getRenderEffect();

  protected:
    /// update cached matrix
    void updateMatrix() const;
    /// Synchronise data in the hardware buffer with what's been added
    void syncHardwareBuffer() const;
    /// Convert a CEGUI::colour to a CS csVector4 color
    csVector4 ColorToCS (const CEGUI::colour &color) const;

    /// CS stuff
    iObjectRegistry* obj_reg;
    csRef<iGraphics3D> g3d;
    csRef<iGraphics2D> g2d;
    

    /* Cached data */

    /// Cached list of rendermeshes.
    mutable csDirtyAccessArray<csSimpleRenderMesh> renderMeshes;

    /// H/W buffer where the vertices are rendered from.
    csRef<csRenderBufferHolder> bufHolder;

#define BUFFER_ARRAY(T, ElementStep)		\
  csDirtyAccessArray<T,				\
  csArrayElementHandler<T>,			\
  CS::Container::ArrayAllocDefault,		\
  csArrayCapacityFixedGrow<ElementStep*2048> >
    BUFFER_ARRAY(csVector2, 6) tcBuf;
    BUFFER_ARRAY(csVector4, 6) colBuf;
    BUFFER_ARRAY(csVector3, 6) vertBuf;
    mutable BUFFER_ARRAY(uint, 6) indexBuf;
#undef BUFFER_ARRAY


    /// Texture that is set as active
    Texture* d_activeTexture;
    /// rectangular clip region
    CEGUI::Rect d_clipRect;
    /// translation vector
    CEGUI::Vector3 d_translation;
    /// rotation vector
    CEGUI::Vector3 d_rotation;
    /// pivot point for rotation
    CEGUI::Vector3 d_pivot;
    /// RenderEffect that will be used by the GeometryBuffer
    CEGUI::RenderEffect* d_effect;
    /// offset to be applied to all geometry
    CEGUI::Vector2 d_texelOffset;
    /// model matrix cache
    mutable CS::Math::Matrix4 d_matrix;
    /// true when d_matrix is valid and up to date
    mutable bool d_matrixValid;
    /// whether the h/w buffer is in sync with the added geometry
    mutable bool d_sync;
  };


} CS_PLUGIN_NAMESPACE_END(cegui)

#endif  // end of guard _CEGUIGEOMETRYBUFFER_H_
