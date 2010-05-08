#ifndef __HAIR_MATERIAL_H__
#define __HAIR_MATERIAL_H__

#include <iutil/comp.h>
#include <csgeom/vector3.h>
#include <imaterial/furmaterial.h>

#include "csutil/scf_implementation.h"

struct iObjectRegistry;

/**
* This is the implementation for our API and
* also the implementation of the plugin.
*/
class HairMaterial : public scfImplementation2<HairMaterial,iFurMaterial,iComponent>
{
private:
  iObjectRegistry* object_reg;
  csVector3 store_v;

public:
  HairMaterial (iBase* parent);
  virtual ~HairMaterial ();

  // From iComponent.
  virtual bool Initialize (iObjectRegistry*);

  // From iMyApi.
  virtual void DoSomething (int param, const csVector3&);
  virtual int GetSomething () const;
};

#endif // __HAIR_MATERIAL_H__
