#ifndef __IAUTH_H__
#define __IAUTH_H__

#include "csutil/scf.h"
#include "isystem.h"
#include "iplugin.h"

#define IAUTH_SUCCESS     (0)
#define IAUTH_NOUSER      (1) 
#define IAUTH_INVALIDPASS (2)

#define IAUTH_ADMIN_USER (1)
#define IAUTH_NORMAL_USER (2)

SCF_VERSION (iAuth, 0, 0, 1);

struct iAuth: public iPlugIn
{
  virtual bool Initialize (iSystem *iSys) = 0;
  virtual bool Open()=0;
  virtual bool Close()=0;

  virtual bool Validate(char *UserName, char *PassWord) = 0;

  // Requires Admin password
  virtual bool Add(char *Username, char *PassWord,
	      char *AdminUser, char *AdminPass, int UserType) = 0;

  virtual bool Delete(char *Username,
		 char *AdminUser, char *AdminPass) = 0;

  virtual bool ChangePass(char *UserName, char *OldPass, char *NewPass) = 0;
  virtual bool AdminChangePass(char *UserName, char *NewPass, 
		  char *AdminUser, char *AdminPass) = 0;
  virtual int GetLastError() = 0;
};

#endif

