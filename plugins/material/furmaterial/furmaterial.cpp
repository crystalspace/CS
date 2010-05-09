#include <cssysdef.h>
#include <iutil/objreg.h>
#include <iutil/plugin.h>

#include "furmaterial.h"

SCF_IMPLEMENT_FACTORY (FurMaterial)

FurMaterial::FurMaterial (iBase* parent) :
scfImplementationType (this, parent),
object_reg(0)
{
}

FurMaterial::~FurMaterial ()
{
}

bool FurMaterial::Initialize (iObjectRegistry* r)
{
  object_reg = r;
  return true;
}

void FurMaterial::DoSomething (int param, const csVector3& v)
{
  // Just some behavior.
  if (param == 1)
    store_v = v;
  else
    store_v = -v;
}

int FurMaterial::GetSomething () const
{
  return (int)store_v.x + (int)store_v.y + (int)store_v.z;
}

void FurMaterial::SetLength (float len)
{

}

float FurMaterial::GetLength () const
{
  return 0;
}

void FurMaterial::SetColor (const csColor4& color)
{

}

csColor4 FurMaterial::GetColor () const
{
  return csColor4(0);
}
