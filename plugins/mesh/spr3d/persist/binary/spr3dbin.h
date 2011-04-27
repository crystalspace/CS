/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_SPR3DBINLDR_H__
#define __CS_SPR3DBINLDR_H__

#include "imap/reader.h"
#include "imap/writer.h"
#include "imap/services.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

struct iEngine;
struct iReporter;
struct iPluginManager;
struct iObjectRegistry;
struct iSpriteAction;
struct iSprite3DFactoryState;
struct iSpriteFrame;

namespace CS
{
namespace Plugins
{
namespace Spr3dBin
{

/**
 * Sprite 3D factory loader for Binary formatted sprites
 */
class csSprite3DBinFactoryLoader : 
  public scfImplementation2<csSprite3DBinFactoryLoader,
			    iBinaryLoaderPlugin,
			    iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

  template<typename FloatGetter>
  bool ReadFrame (iSprite3DFactoryState* spr3dLook, iSpriteFrame* fr, const char*& p,
		  bool has_normals);
  template<typename FloatGetter>
  bool ReadAction (iSprite3DFactoryState* spr3dLook, iSpriteAction* act, const char*& p);
public:
  /// Constructor.
  csSprite3DBinFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csSprite3DBinFactoryLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse data  and return a new object for it.
  virtual csPtr<iBase> Parse (iDataBuffer* data,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context,
    iStringArray*);

  virtual bool IsThreadSafe() { return true; }
};

/**
 * Sprite3D factory saver.
 */
class csSprite3DBinFactorySaver : 
  public scfImplementation2<csSprite3DBinFactorySaver, 
			    iBinarySaverPlugin,
			    iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

public:
  /// Constructor.
  csSprite3DBinFactorySaver (iBase*);

  /// Destructor.
  virtual ~csSprite3DBinFactorySaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iFile* file,
  	iStreamSource*);
};


} // namespace Spr3dBin
} // namespace Plugins
} // namespace CS


#endif // __CS_SPR3DBINLDR_H__
