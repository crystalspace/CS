#include "cssysdef.h"
#include "gl_polybuflightmapext.h"


csPolyBufMatLightmap::csPolyBufMatLightmap(csRealVertexBuffer &vbuf)
       : m_realvbuf(vbuf)
{
  CreateLightmap(256, 256);
}

csPolyBufMatLightmap::csPolyBufMatLightmap(iGraphics3D &g3d, csRealVertexBuffer &vbuf)
      : m_realvbuf(vbuf)
{
  CreateLightmap(256, 256);
}

csPolyBufMatLightmap::csPolyBufMatLightmap(int width, int height, csRealVertexBuffer &vbuf)
      : m_realvbuf(vbuf);
{
}

csPolyBufMatLightmap::~csPolyBufMatLightmap()
{
}

bool csPolyBufMatLightmap::AddPolygon(csVector3 *origverts,
      int* verts, int num_verts,
      const csPlane3& poly_normal,
      const csMatrix3& m_obj2tex, const csVector3& v_obj2tex,
      iPolygonTexture* poly_texture)
{
}

void csPolyBufMatLightmap::MarkLightmapsDirty()
{
  m_dirty = true;
}

bool csPolyBufMatLightmap::CreateLightmap(int width, int height)
{
  glGenTextures(1, &m_texhandle);

}