/*
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

#ifndef _FIRELDR_H_
#define _FIRELDR_H_

#include "imap/reader.h"

struct iEngine;
struct iSystem;

/**
 * Fire factory loader.
 */
class csFireFactoryLoader : public iLoaderPlugIn
{
private:
  iSystem* sys;

public:
  /// Constructor.
  csFireFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csFireFactoryLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  //------------------------ iLoaderPlugIn implementation --------------
  DECLARE_IBASE;

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine);
};

/**
 * Fire loader.
 */
class csFireLoader : public iLoaderPlugIn
{
private:
  iSystem* sys;

public:
  /// Constructor.
  csFireLoader (iBase*);

  /// Destructor.
  virtual ~csFireLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  //------------------------ iLoaderPlugIn implementation --------------
  DECLARE_IBASE;

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine);
};

#endif // _FIRELDR_H_

