#include "csutil/scf.h"

SCF_INTERFACE (iDog, 0, 0, 1) : public iBase
{
  virtual void Walk () = 0;
  virtual void Barf (char *iWhat) = 0;
};
