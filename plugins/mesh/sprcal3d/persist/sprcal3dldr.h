/*
    Copyright (C) 2003 by Keith Fulton

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

#ifndef __CS_SPRCAL3DLDR_H__
#define __CS_SPRCAL3DLDR_H__

#include "imap/reader.h"
#include "imap/writer.h"
#include "imap/services.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/strhash.h"

namespace CS
{
namespace Plugins
{
namespace SprCal3dLoader
{


/**
 * Sprite Cal3D factory loader.
 */
class csSpriteCal3DFactoryLoader : 
  public scfImplementation2<csSpriteCal3DFactoryLoader,
			    iLoaderPlugin,
			    iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csStringHash xmltokens;
  csRef<iVFS> vfs;

  iMaterialWrapper * LoadMaterialTag(iSpriteCal3DFactoryState *newspr,
		iDocumentNode* node,
		iLoaderContext* ldr_context,
		const char *file,
                const char* matName = 0);

public:
  /// Constructor.
  csSpriteCal3DFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csSpriteCal3DFactoryLoader ();

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
class csSpriteCal3DFactorySaver :
  public scfImplementation2<csSpriteCal3DFactorySaver,
			    iSaverPlugin,
			    iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

public:
  /// Constructor.
  csSpriteCal3DFactorySaver (iBase*);

  /// Destructor.
  virtual ~csSpriteCal3DFactorySaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
};

/**
 * Sprite Cal3D loader.
 */
class csSpriteCal3DLoader :
  public scfImplementation2<csSpriteCal3DLoader,
			    iLoaderPlugin,
			    iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;
  csStringHash xmltokens;

public:
  /// Constructor.
  csSpriteCal3DLoader (iBase*);

  /// Destructor.
  virtual ~csSpriteCal3DLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse a given node and return a new object for it.
  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);

  virtual bool IsThreadSafe() { return true; }
};

/**
 * SpriteCal3D saver.
 */
class csSpriteCal3DSaver :
  public scfImplementation2<csSpriteCal3DSaver,
			    iSaverPlugin,
			    iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

public:
  /// Constructor.
  csSpriteCal3DSaver (iBase*);

  /// Destructor.
  virtual ~csSpriteCal3DSaver ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Write down given object and add to iDocumentNode.
  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
};

} // namespace SprCal3dLoader
} // namespace Plugins
} // namespace CS

#endif // __CS_SPRCAL3DLDR_H__
