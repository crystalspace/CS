#ifndef __FUR_MATERIAL_H__
#define __FUR_MATERIAL_H__

#include <csutil/scf.h>
#include <ivideo/material.h>

struct iFurMaterial;

class csVector3;
class csColor4;

struct iFurMaterialType : public virtual iBase
{
  SCF_INTERFACE (iFurMaterialType, 1, 0, 0);

  virtual void ClearFurMaterials () = 0;
  virtual void RemoveFurMaterial (const char *name) = 0;
  virtual iFurMaterial* CreateFurMaterial (const char *name) = 0;
  virtual iFurMaterial* FindFurMaterial (const char *name) const = 0;
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

  virtual void SetLength (float len) = 0;
  virtual float GetLength () const = 0;

  virtual void SetColor (const csColor4& color) = 0;
  virtual csColor4 GetColor () const = 0;
};
#endif // __FUR_MATERIAL_H__
