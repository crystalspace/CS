#include "csutil/scf.h"

SCF_VERSION (iDog, 0, 0, 1);

struct iDog : public iBase
{
  virtual void Walk () = 0;
  virtual void Barf (char *iWhat) = 0;
};
