#ifndef POLYGONBUFFERMATERIALEXT_H_INCLUDED
#define POLYGONBUFFERMATERIALEXT_H_INCLUDED


#include "gl_polybuflightmapext.h"

/**
* A class to store all triangles + lightmaps of a
*/

class csPolygonBufferMaterialEXT
{
public:
  csPolygonBufferMaterialEXT(iGraphics3D *g3d, csRealVertexBuffer &vbuf);
  ~csPolygonBufferMaterialEXT();

  void AddPolygon(csVector3 *origverts, int* verts, int num_verts,
	const csPlane3& poly_normal,
	const csMatrix3& m_obj2tex, const csVector3& v_obj2tex,
	iPolygonTexture* poly_texture);

  void Clear();
  void MarkLightmapsDirty();

  iMaterialHandle *m_mat_handle;
private:
  csRealVertexBuffer &m_realvbuf;


};


#endif