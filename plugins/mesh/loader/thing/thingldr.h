/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef _THINGLDR_H
#define _THINGLDR_H

#include "imap/reader.h"
#include "imap/writer.h"

struct iEngine;
struct iSystem;

/**
 * Thing loader.
 */
class csThingLoader : public iLoaderPlugIn
{
private:
  iSystem* sys;

public:
  /// Constructor.
  csThingLoader (iBase*);

  /// Destructor.
  virtual ~csThingLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  //------------------------ iLoaderPlugIn implementation --------------
  DECLARE_IBASE;

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);
};

/**
 * Thing saver.
 */
class csThingSaver : public iSaverPlugIn
{
private:
  iSystem* sys;

public:
  /// Constructor.
  csThingSaver (iBase*);

  /// Destructor.
  virtual ~csThingSaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  //------------------------ iSaverPlugIn implementation --------------
  DECLARE_IBASE;

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);
};

/**
 * Plane loader.
 */
class csPlaneLoader : public iLoaderPlugIn
{
private:
  iSystem* sys;

public:
  /// Constructor.
  csPlaneLoader (iBase*);

  /// Destructor.
  virtual ~csPlaneLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  //------------------------ iLoaderPlugIn implementation --------------
  DECLARE_IBASE;

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);
};

/**
 * Plane saver.
 */
class csPlaneSaver : public iSaverPlugIn
{
private:
  iSystem* sys;

public:
  /// Constructor.
  csPlaneSaver (iBase*);

  /// Destructor.
  virtual ~csPlaneSaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  //------------------------ iSaverPlugIn implementation --------------
  DECLARE_IBASE;

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);
};

#endif // _THINGLDR_H

