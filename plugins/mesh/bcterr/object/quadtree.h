#ifndef __CS_BCQUAD_H__
#define __CS_BCQUAD_H__

#include "csgeom/transfrm.h"
#include "csgeom/vector3.h"
#include "csutil/cscolor.h"
#include "csutil/refarr.h"
#include "iengine/mesh.h"
#include "imesh/object.h"
#include "imesh/terrfunc.h"
#include "imesh/bcterr.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivideo/vbufmgr.h"
#include "igeom/objmodel.h"
#include "qsqrt.h"

struct iEngine;
struct iMaterialWrapper;
struct iObjectRegistry;
class csBCTerrBlock;
class csBCCollisionQuad;

class csColQuad
{
private:
  bool CheckBox (csBox3 check, csBCTerrBlock *block);
  void SetupChildren (float shortest, iObjectRegistry* object_reg);
  void AddBlockToList (csBCTerrBlock *block);
public:
  csColQuad* children[4];
  csBox3 bbox;
  csBCTerrBlock **blocks;
  int num_blocks;
  // create root quad
  csColQuad (csVector3 *cntrl_pt, int x_blocks, int z_blocks,
    float shortest, iObjectRegistry* object_reg);
  // create a normal quad
  csColQuad (float shortest, csBox3 nbbox, iObjectRegistry* object_reg);
  ~csColQuad ();

  void AddBlock (csBCTerrBlock *block);
  void RebuildBoundingBoxes ();
  // basic
  void HeightTest (csVector3  *point, 
    int &hits);
  // camera test
  void HeightTestExt (csVector3  *point, 
    int &hits);
  // Exact point, may clip into polys
  void HeightTestExact (csVector3  *point, 
    int &hits);
  bool HitBeamOutline (csSegment3 seg,
    csVector3& isect, float* pr);
  bool HitBeamObject (csSegment3 seg,
    csVector3& isect, float* pr, float &distance);
};

class csBCCollisionQuad
{
  csColQuad *root_quad;
  iObjectRegistry* object_reg;
public:
  csBCCollisionQuad ();
  ~csBCCollisionQuad ();
  csBCCollisionQuad (csVector3 *cntrl_pt, int x_blocks, int z_blocks,
    float shortest, iObjectRegistry* nobject_reg);
  void AddBlock (csBCTerrBlock *block);
  void RebuildBoundingBoxes ();
  // uses control triangles
  int HeightTest (csVector3 *point);
  // uses control triangles, yet blocks don't have to be squares
  int HeightTestExt (csVector3 *point);
  // uses bezier compute to get point
  int HeightTestExact (csVector3 *point);
  bool HitBeamOutline (const csVector3& start, const csVector3& end,
    csVector3& isect, float* pr);
  bool HitBeamObject (const csVector3& start, const csVector3& end,
    csVector3& isect, float* pr);
};



#endif // __CS_BCQUAD_H__
