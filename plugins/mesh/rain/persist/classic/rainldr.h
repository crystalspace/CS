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
#include "isys/plugin.h"

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
  SCF_DECLARE_IBASE;

  /// Constructor.
  csRainFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csRainFactoryLoader ();

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugIn : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csRainFactoryLoader);
    virtual bool Initialize (iSystem* p) { scfParent->sys = p; return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugIn;
};

/**
 * Rain factory saver.
 */
class csRainFactorySaver : public iSaverPlugIn
{
private:
  iSystem* sys;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csRainFactorySaver (iBase*);

  /// Destructor.
  virtual ~csRainFactorySaver ();

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugIn : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csRainFactorySaver);
    virtual bool Initialize (iSystem* p) { scfParent->sys = p; return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugIn;
};


/**
 * Rain loader.
 */
class csRainLoader : public iLoaderPlugIn
{
private:
  iSystem* sys;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csRainLoader (iBase*);

  /// Destructor.
  virtual ~csRainLoader ();

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine, iBase* context);

  struct eiPlugIn : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csRainLoader);
    virtual bool Initialize (iSystem* p) { scfParent->sys = p; return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugIn;
};

/**
 * Rain saver.
 */
class csRainSaver : public iSaverPlugIn
{
private:
  iSystem* sys;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csRainSaver (iBase*);

  /// Destructor.
  virtual ~csRainSaver ();

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);

  struct eiPlugIn : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csRainSaver);
    virtual bool Initialize (iSystem* p) { scfParent->sys = p; return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugIn;
};

#endif // _RAINLDR_H_
