#ifndef POLYBUFLIGHTMAPEXT_H_INCLUDED
#define POLYBUFLIGHTMAPEXT_H_INCLUDED
// Look for the CS OPENGL PATH
#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif

#include "csgeom/vector2.h"
#include "csgeom/vector3.h"
#include "csgeom/matrix3.h"
#include "csutil/garray.h"
#include "plugins/video/renderer/common/vbufmgr.h"
#include "ivideo/graph3d.h"
#include "csgeom/subrec.h"

struct csPolyBufMatExtPolygon
{
  iPolygonTexture *m_lmhandle;
  int             *m_indices;
  int              m_index_count;
  csRect           m_lightmaprect;
};

struct csRealVertexBuffer
{
  CS_DECLARE_GROWING_ARRAY(m_vertices, csVector3);
  CS_DECLARE_GROWING_ARRAY(m_texcoords, csVector2);
  CS_DECLARE_GROWING_ARRAY(m_lmcoords, csVector2);
};

class csPolyBufMatLightmap
{
public:
  csPolyBufMatLightmap(csRealVertexBuffer &vbuf);
  csPolyBufMatLightmap(iGraphics3D &g3d, csRealVertexBuffer &vbuf);
  csPolyBufMatLightmap(int width, int height, csRealVertexBuffer &vbuf);
  ~csPolyBufMatLightmap();

  bool AddPolygon(csVector3 *origverts,
      int* verts, int num_verts,
      const csPlane3& poly_normal,
      const csMatrix3& m_obj2tex, const csVector3& v_obj2tex,
      iPolygonTexture* poly_texture);

  void MarkLightmapsDirty();
private:
  bool CreateLightmap(int width, int height);


  CS_DECLARE_GROWING_ARRAY(m_polygons, csPolyBufMatExtPolygon);
  csRealVertexBuffer &m_realvbuf;
  bool                m_dirty;
  unsigned int        m_texhandle;
};



#endif
