#include <cssysdef.h>
#include <iutil/objreg.h>
#include <iutil/plugin.h>

#include "hairmaterial.h"

SCF_IMPLEMENT_FACTORY (HairMaterial)

HairMaterial::HairMaterial (iBase* parent) :
scfImplementationType (this, parent),
object_reg(0)
{
}

HairMaterial::~HairMaterial ()
{
}

bool HairMaterial::Initialize (iObjectRegistry* r)
{
  object_reg = r;
  return true;
}

void HairMaterial::DoSomething (int param, const csVector3& v)
{
  // Just some behavior.
  if (param == 1)
    store_v = v;
  else
    store_v = -v;
}

int HairMaterial::GetSomething () const
{
  return (int)store_v.x + (int)store_v.y + (int)store_v.z;
}

void HairMaterial::SetLength (float len)
{

}

float HairMaterial::GetLength () const
{
  return 0;
}

void HairMaterial::SetColor (const csColor4& color)
{

}

csColor4 HairMaterial::GetColor () const
{
  return csColor4(0);
}
