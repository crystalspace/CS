#include "csutil/scf.h"

SCF_VERSION (iFrog, 0, 0, 1);

struct iFrog : public iBase
{
  virtual void Jump () = 0;
  virtual void Croak (char *iWhat) = 0;
};
