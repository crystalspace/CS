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

#ifndef _SNOWLDR_H_
#define _SNOWLDR_H_

#include "imap/reader.h"
#include "imap/writer.h"

struct iEngine;
struct iSystem;

/**
 * Snow factory loader.
 */
class csSnowFactoryLoader : public iLoaderPlugIn
{
private:
  iSystem* sys;

public:
  /// Constructor.
  csSnowFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csSnowFactoryLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  //------------------------ iLoaderPlugIn implementation --------------
  DECLARE_IBASE;

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine);
};

/**
 * Snow factory saver.
 */
class csSnowFactorySaver : public iSaverPlugIn
{
private:
  iSystem* sys;

public:
  /// Constructor.
  csSnowFactorySaver (iBase*);

  /// Destructor.
  virtual ~csSnowFactorySaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  //------------------------ iSaverPlugIn implementation --------------
  DECLARE_IBASE;

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);
};

/**
 * Snow loader.
 */
class csSnowLoader : public iLoaderPlugIn
{
private:
  iSystem* sys;

public:
  /// Constructor.
  csSnowLoader (iBase*);

  /// Destructor.
  virtual ~csSnowLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  //------------------------ iLoaderPlugIn implementation --------------
  DECLARE_IBASE;

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine);
};

/**
 * Snow saver.
 */
class csSnowSaver : public iSaverPlugIn
{
private:
  iSystem* sys;

public:
  /// Constructor.
  csSnowSaver (iBase*);

  /// Destructor.
  virtual ~csSnowSaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  //------------------------ iSaverPlugIn implementation --------------
  DECLARE_IBASE;

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);
};


#endif // _SNOWLDR_H_

