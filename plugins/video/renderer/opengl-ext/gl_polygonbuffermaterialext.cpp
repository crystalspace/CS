#include "cssysdef.h"
#include "gl_polygonbuffermaterialext.h"


csPolygonBufferMaterialEXT::
csPolygonBufferMaterialEXT(iGraphics3D*, csRealVertexBuffer &vbuf):
  m_realvbuf(vbuf)
{
}

csPolygonBufferMaterialEXT::~csPolygonBufferMaterialEXT()
{
}


void csPolygonBufferMaterialEXT::AddPolygon(csVector3 *origverts,
      int* verts, int num_verts,
      const csPlane3& poly_normal,
      const csMatrix3& m_obj2tex, const csVector3& v_obj2tex,
      iPolygonTexture* poly_texture)
{
  (void)origverts;
  (void)verts;
  (void)num_verts;
  (void)poly_normal;
  (void)m_obj2tex;
  (void)v_obj2tex;
  (void)poly_texture;
}

void csPolygonBufferMaterialEXT::Clear()
{
}

void csPolygonBufferMaterialEXT::MarkLightmapsDirty()
{
}
