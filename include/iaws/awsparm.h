#ifndef __AWS_FLEXIBLE_PARAMETER_LIST_INTERFACE__
#define __AWS_FLEXIBLE_PARAMETER_LIST_INTERFACE__
/**************************************************************************
    Copyright (C) 2001 by Christopher Nelson 
    
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
*****************************************************************************/

#include "csgeom/csrect.h"
#include "csgeom/cspoint.h"
#include "csutil/scfstr.h"
#include "csutil/csvector.h"
#include "csutil/csstrvec.h"

SCF_VERSION (iAwsParmList, 0, 0, 1);

/***********************************************************************************
 * Provides support for safely passing named parameters through to different functions
 * in a portable manner.  Note that awsParmList does not utilize copy semantics.  In
 * the interests of space and speed, it simply takes a reference to the pointers passed
 * in.  This means that you should NOT use an awsParmList if any parm it references
 * has gone out of scope!
 ***********************************************************************************/
struct iAwsParmList : public iBase
{
  /// Adds an integer to the parmeter list
  virtual void AddInt(char *name, int value)=0;
  /// Adds a float to the parmeter list
  virtual void AddFloat(char *name, float value)=0;
  /// Adds a bool to the parmeter list
  virtual void AddBool(char *name, bool value)=0;
  /// Adds a string to the parmeter list
  virtual void AddString(char *name, iString* value)=0;
  /// Adds a vector to the parmeter list
  virtual void AddBasicVector(char *name, csBasicVector* value)=0;
  /// Adds a string vector to the parmeter list
  virtual void AddStringVector(char *name, csStrVector* value)=0;
  /// Adds a rect to the parmeter list
  virtual void AddRect(char *name, csRect *value)=0;
  /// Adds a point to the parmeter list
  virtual void AddPoint(char *name, csPoint *value)=0;

  /// Returns the int named "name" in value.  True if it was found, otherwise false.
  virtual bool GetInt(char *name, int *value)=0;
  /// Returns the float named "name" in value.  True if it was found, otherwise false.
  virtual bool GetFloat(char *name, float *value)=0;
  /// Returns the bool named "name" in value.  True if it was found, otherwise false.
  virtual bool GetBool(char *name, bool *value)=0;
  /// Returns the string named "name" in value.  True if it was found, otherwise false.
  virtual bool GetString(char *name, iString **value)=0;
  /// Returns the basic vector named "name" in value.  True if it was found, otherwise false.
  virtual bool GetBasicVector(char *name, csBasicVector **value)=0;
  /// Returns the string vector named "name" in value.  True if it was found, otherwise false.
  virtual bool GetStringVector(char *name, csStrVector **value)=0;
  /// Returns the rect named "name" in value.  True if it was found, otherwise false.
  virtual bool GetRect(char *name, csRect **value)=0;
  /// Returns the point named "name" in value.  True if it was found, otherwise false.
  virtual bool GetPoint(char *name, csPoint **value)=0;

  /// Clears the parameter list
  virtual void Clear()=0;
};

#endif
