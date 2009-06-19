/*
    Copyright (C) 2006 by Jorrit Tyberghein

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

#ifndef __CS_SPR3MD2LDR_H__
#define __CS_SPR3MD2LDR_H__

#include "imap/reader.h"
#include "imap/writer.h"
#include "imap/services.h"
#include "imap/modelload.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

struct iEngine;
struct iReporter;
struct iPluginManager;
struct iObjectRegistry;

namespace CS
{
namespace Plugins
{
namespace Spr3dMd2
{

/**
 * Sprite 3D factory loader for Binary formatted sprites
 */
class csSprite3DMD2FactoryLoader : 
  public scfImplementation3<csSprite3DMD2FactoryLoader,
			    iBinaryLoaderPlugin,
			    iModelLoader,
			    iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

  bool Load (iSprite3DFactoryState* state, uint8 *Buffer, size_t Size);
  iMeshFactoryWrapper* Load (const char* factname, const char* filename,
  	iDataBuffer* buffer);
  bool TestMD2 (uint8 *Buffer, size_t Size);

public:
  /// Constructor.
  csSprite3DMD2FactoryLoader (iBase*);

  /// Destructor.
  virtual ~csSprite3DMD2FactoryLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse data  and return a new object for it.
  virtual csPtr<iBase> Parse (iDataBuffer* data, iStreamSource*,
    iLoaderContext* ldr_context, iBase* context, iStringArray*);

  virtual bool IsThreadSafe() { return true; }

  virtual iMeshFactoryWrapper* Load (const char* factname, const char* filename);
  virtual iMeshFactoryWrapper* Load (const char* factname, iDataBuffer* buffer);
  virtual bool IsRecognized (const char* filename);
  virtual bool IsRecognized (iDataBuffer* buffer);
};

} // namespace Spr3dMd2
} // namespace Plugins
} // namespace CS
#endif // __CS_SPR3MD2LDR_H__
