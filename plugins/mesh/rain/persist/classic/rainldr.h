/*  virtual iBase* Parse (const char* string, iEngine* engine);

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

#ifndef _RAINLDR_H_
#define _RAINLDR_H_

#include "imap/reader.h"
#include "imap/writer.h"

struct iEngine;
struct iSystem;

/**
 * Rain factory loader.
 */
class csRainFactoryLoader : public iLoaderPlugIn
{
private:
  iSystem* sys;

public:
  /// Constructor.
  csRainFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csRainFactoryLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  //------------------------ iLoaderPlugIn implementation --------------
  DECLARE_IBASE;

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);
};

/**
 * Rain factory saver.
 */
class csRainFactorySaver : public iSaverPlugIn
{
private:
  iSystem* sys;

public:
  /// Constructor.
  csRainFactorySaver (iBase*);

  /// Destructor.
  virtual ~csRainFactorySaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  //------------------------ iSaverPlugIn implementation --------------
  DECLARE_IBASE;

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);
};


/**
 * Rain loader.
 */
class csRainLoader : public iLoaderPlugIn
{
private:
  iSystem* sys;

public:
  /// Constructor.
  csRainLoader (iBase*);

  /// Destructor.
  virtual ~csRainLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  //------------------------ iLoaderPlugIn implementation --------------
  DECLARE_IBASE;

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);
};

/**
 * Rain saver.
 */
class csRainSaver : public iSaverPlugIn
{
private:
  iSystem* sys;

public:
  /// Constructor.
  csRainSaver (iBase*);

  /// Destructor.
  virtual ~csRainSaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  //------------------------ iSaverPlugIn implementation --------------
  DECLARE_IBASE;

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);
};



#endif // _RAINLDR_H_

