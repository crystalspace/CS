#include "csutil/scf.h"

SCF_INTERFACE (iFrog, 0, 0, 1) : public iBase
{
  virtual void Jump () = 0;
  virtual void Croak (char *iWhat) = 0;
};
