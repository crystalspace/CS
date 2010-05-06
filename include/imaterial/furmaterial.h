#ifndef __FUR_MATERIAL_H__
#define __FUR_MATERIAL_H__

#include <csutil/scf.h>
#include <csutil/scf_implementation.h>

class csVector3;

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
