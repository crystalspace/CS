#ifndef __IVARIA_AWS_H__
#define __IVARIA_AWS_H__

#include "csutil/scf.h"

SCF_VERSION (iAws, 0, 0, 1);

struct iAws : public iBase
{
public:  
  virtual void Load(const char *def_file)=0;
};


SCF_VERSION (iAwsPrefs, 0, 0, 1);

struct iAwsPrefs : public iBase
{
public:  
  virtual void Load(const char *def_file)=0;
};

#endif // __IVARIA_AWS_H__
