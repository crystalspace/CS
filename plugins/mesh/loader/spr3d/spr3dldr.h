/*
    Copyright (C) 2001 by Jorrit Tyberghein
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

#ifndef _SPR3DLDR_H_
#define _SPR3DLDR_H_

#include "imap/reader.h"
#include "imap/writer.h"

struct iEngine;
struct iSystem;
struct iSkeletonLimb;

/**
 * Sprite 3D factory loader.
 */
class csSprite3DFactoryLoader : public iLoaderPlugIn
{
private:
  iSystem* sys;

  // Load a skeleton.
  bool LoadSkeleton (iSkeletonLimb* limb, char* buf);

public:
  /// Constructor.
  csSprite3DFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csSprite3DFactoryLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  //------------------------ iLoaderPlugIn implementation --------------
  DECLARE_IBASE;

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine);
};

/**
 * Sprite3D factory saver.
 */
class csSprite3DFactorySaver : public iSaverPlugIn
{
private:
  iSystem* sys;

  // Save a skeleton.
  void SaveSkeleton (iSkeletonLimb* limb, iStrVector *str);

public:
  /// Constructor.
  csSprite3DFactorySaver (iBase*);

  /// Destructor.
  virtual ~csSprite3DFactorySaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  //------------------------ iSaverPlugIn implementation --------------
  DECLARE_IBASE;

  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);
};

/**
 * Sprite 3D loader.
 */
class csSprite3DLoader : public iLoaderPlugIn
{
private:
  iSystem* sys;

public:
  /// Constructor.
  csSprite3DLoader (iBase*);

  /// Destructor.
  virtual ~csSprite3DLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  //------------------------ iLoaderPlugIn implementation --------------
  DECLARE_IBASE;

  /// Parse a given string and return a new object for it.
  virtual iBase* Parse (const char* string, iEngine* engine);
};

/**
 * Sprite3D saver.
 */
class csSprite3DSaver : public iSaverPlugIn
{
private:
  iSystem* sys;

public:
  /// Constructor.
  csSprite3DSaver (iBase*);

  /// Destructor.
  virtual ~csSprite3DSaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

public:
  //------------------------ iSaverPlugIn implementation --------------
  DECLARE_IBASE;
  
  /// Write down given object and add to string vector.
  virtual void WriteDown (iBase *obj, iStrVector *str, iEngine* engine);
};

#endif // _SPR3DLDR_H_

