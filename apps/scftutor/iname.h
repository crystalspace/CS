#include "csutil/scf.h"

SCF_VERSION (iName, 0, 0, 1);

struct iName : public iBase
{
  virtual char *GetName () = 0;
  virtual void SetName (char *iName) = 0;
};
