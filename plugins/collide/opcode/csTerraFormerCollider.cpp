#include "cssysdef.h"
#include "iutil/strset.h"
#include "iutil/objreg.h"
#include "csgeom/box.h"
#include "csTerraFormerCollider.h"
#include "OPC_TreeBuilders.h"

using namespace CS::Plugins::Opcode;
using namespace CS::Plugins::Opcode::Opcode;

#define RESOLUTION 300
#define BOX_SIZE 30

csTerraFormerCollider::csTerraFormerCollider (iTerraFormer* terraformer, 
                                              iObjectRegistry* object_reg)
: scfImplementationType(this, object_reg)
{
  csTerraFormerCollider::object_reg = object_reg;
  former = terraformer;
  // Get the shared string repository
  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.shared.stringset", iStringSet);
  stringHeights = strings->Request ("heights");
  stringVertices = strings->Request ("vertices");

  opcMeshInt.SetCallback (&MeshCallback, this); 

  transform.m[0][0] = 1;
  transform.m[1][0] = 0;
  transform.m[2][0] = 0;
  transform.m[3][0] = 0;  

  transform.m[0][1] = 0;
  transform.m[1][1] = 1;
  transform.m[2][1] = 0;
  transform.m[3][1] = 0;  

  transform.m[0][2] = 0;
  transform.m[1][2] = 0;
  transform.m[2][2] = 1;
  transform.m[3][2] = 0;  

  transform.m[0][3] = 0;
  transform.m[1][3] = 0;
  transform.m[2][3] = 0;
  transform.m[3][3] = 1; 

  InitOPCODEModel ();
}

void csTerraFormerCollider::UpdateOPCODEModel (const csVector3 &other_pos)
{
  csRef<iTerraSampler> sampler = former->GetSampler (
    csBox2 (other_pos.x - BOX_SIZE, other_pos.y - BOX_SIZE,
    other_pos.x + BOX_SIZE, other_pos.y + BOX_SIZE), RESOLUTION , RESOLUTION);

  const csVector3 *v = sampler->SampleVector3 (stringVertices);

  for (int y = 0 ; y < RESOLUTION ; y++)
  {
    for (int x = 0 ; x < RESOLUTION ; x++)
    {
      int index = y*RESOLUTION + x;
      vertices[index].Set (v[index].x,v[index].y,v[index].z);
    }
  }
  int i = 0;
  for (int y = 0 ; y < RESOLUTION-1 ; y++)
  {
    int yr = y * RESOLUTION;
    for (int x = 0 ; x < RESOLUTION-1 ; x++)
    {
      triangles[i++].Set (yr + x, yr+RESOLUTION + x, yr + x+1);
      triangles[i++].Set (yr + x+1, yr+RESOLUTION + x, yr+RESOLUTION + x+1);
    }
  }

  opcode_model->Build (OPCC);
}

void csTerraFormerCollider::InitOPCODEModel ()
{
  triangles.SetLength (2 * (RESOLUTION-1) * (RESOLUTION-1));
  vertices.SetLength (RESOLUTION*RESOLUTION);

  opcode_model = new CS::Plugins::Opcode::Opcode::Model;

  opcMeshInt.SetNbTriangles (triangles.GetSize ());
  opcMeshInt.SetNbVertices (vertices.GetSize ());

  // Mesh data
  OPCC.mIMesh = &opcMeshInt;
  OPCC.mSettings.mRules = SPLIT_SPLATTER_POINTS | SPLIT_GEOM_CENTER;
  OPCC.mNoLeaf = true;
  OPCC.mQuantized = true;
  OPCC.mKeepOriginal = false;
  OPCC.mCanRemap = false;

  UpdateOPCODEModel (csVector3 (0));
}
csTerraFormerCollider::~csTerraFormerCollider ()
{
  if (opcode_model)
  {
    delete opcode_model;
    opcode_model = 0;
  }
}
void csTerraFormerCollider::MeshCallback (CS::Plugins::Opcode::udword triangle_index, 
                          CS::Plugins::Opcode::Opcode::VertexPointers& triangle, void* user_data)
{
  csTerraFormerCollider* collider = (csTerraFormerCollider*)user_data;
  csTriangle tri = collider->triangles[triangle_index];
  triangle.Vertex[0] = &collider->vertices [tri.a];
  triangle.Vertex[1] = &collider->vertices [tri.b];
  triangle.Vertex[2] = &collider->vertices [tri.c];
}

float csTerraFormerCollider::SampleFloat (float x, float z)
{
  float y;
  former->SampleFloat (stringHeights, x, z, y);
  return y;
}