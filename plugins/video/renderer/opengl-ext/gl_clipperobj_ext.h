#ifndef GL_CLIPPEROBJ_H_INCLUDED
#define GL_CLIPPEROBJ_H_INCLUDED

#include "clipperobj.h"
#include "gl_states_ext.h"

struct iGraphics3D;
struct iClipper2D;

class ClipperObjectStencil : public iClipperObject
{
public:
  ClipperObjectStencil(iGraphics3D *g3d, unsigned char layercount, gl_states_ext &states);
  ~ClipperObjectStencil();

  virtual void SetClipper(iClipper2D *clipper);
  virtual iClipper2D * GetCurClipper();

  virtual G3DPolygonDP  & ClipPolygon     ( G3DPolygonDP& poly );
  virtual G3DPolygonDPFX& ClipPolygonFX   ( G3DPolygonDPFX& poly );

  virtual G3DTriangleMesh& ClipTriangleMesh( G3DTriangleMesh& mesh );
  virtual G3DPolygonMesh & ClipPolygonMesh ( G3DPolygonMesh&  mesh );

private:

  void SetupClipper();
  unsigned char m_stencilmask;
  unsigned char m_layercount;
  unsigned char m_curbit;
  gl_states_ext &m_glstates;

  iClipper2D *  m_curclip;  
  iGraphics3D * m_g3d;
};


#endif