#include "csutil/scf.h"

SCF_VERSION (iAws, 0, 0, 1);

struct iAws : public iBase
{
public:  
  virtual void Load(char *def_file)=0;
};

