#ifndef  __INETCMD_H__
#define __INETCMD_H__

#include "csutil/scf.h"

SCF_VERSION (iNetCmd, 0, 0, 1);

struct iNetCmd : public iBase
{
  virtual const char *GetName() const =0;
  virtual void SetName(const char *iName) = 0;
  virtual int GetNodeType() = 0;
  virtual void SetLeft(struct iNetCmd *) =0;
  virtual void SetRight(struct iNetCmd *) =0;
  virtual void SetNext(struct iNetCmd *) = 0;
  virtual char *Stringify() = 0;

  virtual iNetCmd *GetLeft() =0;
  virtual iNetCmd *GetRight() =0;  
  virtual iNetCmd *GetNext() =0;  
  virtual void Push(iNetCmd *cmd);
  virtual int GetValue() = 0;
};



#endif






