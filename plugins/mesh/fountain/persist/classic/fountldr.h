/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Copyright (C) 2001 by W.C.A. Wijngaards

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

#ifndef _FOUNTLDR_H_
#define _FOUNTLDR_H_

#include "imap/reader.h"
#include "imap/writer.h"

struct iEngine;
struct iSystem;

/**
 * Fountain factory loader.
 */
class csFountainFactoryLoader : public iLoaderPlugIn
{
private:
  iSystem* sys;

public:
  /// Constructor.
  csFountainFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csFountainFactoryLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  //------------------------ iLoaderPlugIn implementation --------------
  DECLARE_IBASE;

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);
};

/**
 * Fountain factory saver.
 */
class csFountainFactorySaver : public iSaverPlugIn
{
private:
  iSystem* sys;

public:
  /// Constructor.
  csFountainFactorySaver (iBase*);

  /// Destructor.
  virtual ~csFountainFactorySaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  //------------------------ iSaverPlugIn implementation --------------
  DECLARE_IBASE;

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);
};


/**
 * Fountain loader.
 */
class csFountainLoader : public iLoaderPlugIn
{
private:
  iSystem* sys;

public:
  /// Constructor.
  csFountainLoader (iBase*);

  /// Destructor.
  virtual ~csFountainLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  //------------------------ iLoaderPlugIn implementation --------------
  DECLARE_IBASE;

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);
};

/**
 * Fountain saver.
 */
class csFountainSaver : public iSaverPlugIn
{
private:
  iSystem* sys;

public:
  /// Constructor.
  csFountainSaver (iBase*);

  /// Destructor.
  virtual ~csFountainSaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  //------------------------ iSaverPlugIn implementation --------------
  DECLARE_IBASE;

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);
};


#endif // _FOUNTLDR_H_

