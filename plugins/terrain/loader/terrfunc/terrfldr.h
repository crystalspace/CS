/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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

#ifndef _TERRFLDR_H_
#define _TERRFLDR_H_

#include "imap/reader.h"

struct iEngine;
struct iSystem;

/**
 * TerrFunc factory loader.
 */
class csTerrFuncFactoryLoader : public iLoaderPlugIn
{
private:
  iSystem* pSystem;

public:
  /// Constructor.
  csTerrFuncFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csTerrFuncFactoryLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSys);

public:
  //------------------------ iLoaderPlugIn implementation --------------
  DECLARE_IBASE;

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);
};

/**
 * TerrFunc loader.
 */
class csTerrFuncLoader : public iLoaderPlugIn
{
private:
  iSystem* pSystem;

public:
  /// Constructor.
  csTerrFuncLoader (iBase*);

  /// Destructor.
  virtual ~csTerrFuncLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSys);

public:
  //------------------------ iLoaderPlugIn implementation --------------
  DECLARE_IBASE;

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);
};

#endif // _TERRFLDR_H_

