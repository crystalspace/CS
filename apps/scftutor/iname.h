#include "csutil/scf.h"

SCF_INTERFACE (iName, 0, 0, 1) : public iBase
{
  virtual char *GetName () = 0;
  virtual void SetName (char *iName) = 0;
};
