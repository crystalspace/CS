#include "csutil/scf.h"

SCF_VERSION (iWorm, 0, 0, 1);

struct iWorm : public iBase
{
  virtual void Crawl () = 0;
  virtual int Length () = 0;
  virtual iWorm *Split (int iLen1, int iLen2) = 0;
};
