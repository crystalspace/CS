#ifndef __FUR_MATERIAL_H__
#define __FUR_MATERIAL_H__

#include <csutil/scf.h>

struct iFurMaterialType;

class csVector3;
class csColor4;

struct iFurMaterialFactory : public virtual iBase
{	
  SCF_INTERFACE (iFurMaterialFactory, 1, 0, 0);
  
  virtual void SetLength (float len) = 0;
  virtual void GetLength (float &len) = 0;

  virtual void SetColor (csColor4 color) = 0;
  virtual void GetColor (csColor4 &color) = 0;

  virtual iFurMaterialType* GetFurMaterialType () const = 0;
};

struct iFurMaterialType : public virtual iBase
{
  SCF_INTERFACE (iFurMaterialType, 1, 0, 0);

  virtual csPtr<iFurMaterialFactory> NewFactory() = 0;
};

/**
 * This is the API for our plugin. It is recommended
 * that you use better comments than this one in a
 * real situation.
 */
struct iFurMaterial : public virtual iBase
{
SCF_INTERFACE (iFurMaterial, 1, 0, 0);
  /// Do something.
  virtual void DoSomething (int param, const csVector3&) = 0;
  /// Get something.
  virtual int GetSomething () const = 0;
};
#endif // __FUR_MATERIAL_H__
