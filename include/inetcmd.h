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

#ifndef  __INETCMD_H__
#define __INETCMD_H__

#include "csutil/scf.h"

#define NODE_UNKNOWN   (100)
#define NODE_OPTVAL    (102)     // no children
#define NODE_CMDOPTION (103)     // has a right child 
#define NODE_TAG       (104)     // Single tag
#define NODE_HEAD      (105)
#define NODE_BODY      (106)
#define NODE_CMD       (107)
#define NODE_LIST      (108)

#define CS_NETCMD_LONG   (1)
#define CS_NETCMD_INT    (2)
#define CS_NETCMD_BOOL   (3)
#define CS_NETCMD_FLOAT  (4)
#define CS_NETCMD_STRING (5)

SCF_VERSION (iNetCmd, 0, 0, 2);

/**
 * @@@ Please document me using Doc++!
 */
struct iNetCmd : public iBase
{
  virtual const char *GetName() const = 0;
  virtual void SetName(const char*) = 0;
  virtual int GetNodeType() const = 0;
  virtual void SetLeft(iNetCmd*) = 0;
  virtual void SetRight(iNetCmd*) = 0;
  virtual void SetNext(iNetCmd*) = 0;
  virtual char *Stringify() const = 0; // Caller must delete[] returned string.
  virtual iNetCmd *GetLeft() const = 0;
  virtual iNetCmd *GetRight() const = 0;  
  virtual iNetCmd *GetNext() const = 0;  
  virtual void Push(iNetCmd*) = 0;
  virtual int GetValue() const = 0;
};

#endif
