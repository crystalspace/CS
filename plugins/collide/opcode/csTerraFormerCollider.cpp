#include "cssysdef.h"
#include "iutil/strset.h"
#include "iutil/objreg.h"
#include "csTerraFormerCollider.h"

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
}

float csTerraFormerCollider::SampleFloat (float x, float z)
{
  float y;
  former->SampleFloat (stringHeights, x, z, y);
  return y;
}