#include "csutil/scf.h"

SCF_INTERFACE (iWorm, 0, 0, 1) : public iBase
{
  virtual void Crawl () = 0;
  virtual int Length () = 0;
  virtual iWorm *Split (int iLen1, int iLen2) = 0;
};
