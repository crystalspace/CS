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

#ifndef __CS_SPR3DLDR_H__
#define __CS_SPR3DLDR_H__

#include "imap/reader.h"
#include "imap/writer.h"
#include "imap/services.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/strhash.h"

struct iEngine;
struct iReporter;
struct iPluginManager;
struct iObjectRegistry;

namespace CS
{
namespace Plugins
{
namespace Spr3dLoader
{

/**
 * Sprite 3D factory loader.
 */
class csSprite3DFactoryLoader : 
  public scfImplementation2<csSprite3DFactoryLoader,
			    iLoaderPlugin,
			    iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csStringHash xmltokens;

public:
  /// Constructor.
  csSprite3DFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csSprite3DFactoryLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);

  virtual bool IsThreadSafe() { return true; }
};

/**
 * Sprite3D factory saver.
 */
class csSprite3DFactorySaver : 
  public scfImplementation2<csSprite3DFactorySaver,
			    iSaverPlugin,
			    iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

public:
  /// Constructor.
  csSprite3DFactorySaver (iBase*);

  /// Destructor.
  virtual ~csSprite3DFactorySaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
};

/**
 * Sprite 3D loader.
 */
class csSprite3DLoader : 
  public scfImplementation2<csSprite3DLoader,
			    iLoaderPlugin,
			    iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csStringHash xmltokens;

public:
  /// Constructor.
  csSprite3DLoader (iBase*);

  /// Destructor.
  virtual ~csSprite3DLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);

  virtual bool IsThreadSafe() { return true; }
};

/**
 * Sprite3D saver.
 */
class csSprite3DSaver : 
  public scfImplementation2<csSprite3DSaver,
			    iSaverPlugin,
			    iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

public:
  /// Constructor.
  csSprite3DSaver (iBase*);

  /// Destructor.
  virtual ~csSprite3DSaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
};

} // namespace Spr3dLoader
} // namespace Plugins
} // namespace CS

#endif // __CS_SPR3DLDR_H__
