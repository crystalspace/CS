/*
    Crystal Space 3D engine
    Copyright (C) 2000 by Jorrit Tyberghein
  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
  
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

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

/**
 * @@@ Please fill in this Doc++ comment and
 * add Doc++ comments to the methods below.
 */
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

  virtual bool LockAccount(char *UserName, char *Reason, char *AdminUser, char *AdminPass) =0;
  virtual bool UnlockAccount(char *UserName, char *Reason, char *AdminUser, char *AdminPass) = 0;

  virtual int GetLastError() = 0;
};

#endif

