#ifndef CLIPPEROBJ_H_INCLUDED
#define CLIPPEROBJ_H_INCLUDED


struct iGraphics3d;
struct iClipper2D;
struct G3DPolygonDP;
struct G3DPolygonDPFX;
struct G3DTriangleMesh;
struct G3DPolygonMesh;

class iClipperObject
{
public:

  virtual void SetClipper(iClipper2D *clipper) = 0;
  virtual iClipper2D * GetCurClipper() = 0;

  virtual G3DPolygonDP  & ClipPolygon     ( G3DPolygonDP& poly ) = 0;
  virtual G3DPolygonDPFX& ClipPolygonFX   ( G3DPolygonDPFX& poly ) = 0;

  virtual G3DTriangleMesh& ClipTriangleMesh( G3DTriangleMesh& mesh ) = 0;
  virtual G3DPolygonMesh & ClipPolygonMesh ( G3DPolygonMesh&  mesh ) = 0;

};

#endif