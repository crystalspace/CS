#include "csutil/scf.h"

SCF_VERSION (iAwsPrefs, 0, 0, 1);

struct iAwsPrefs : public iBase
{
public:  
  virtual void Load(char *def_file)=0;
};

