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

#ifndef __CS_GMESH3DSLDR_H__
#define __CS_GMESH3DSLDR_H__

#include "csutil/dirtyaccessarray.h"
#include "imap/reader.h"
#include "imap/writer.h"
#include "imap/services.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

#include <lib3ds/types.h>

struct iEngine;
struct iReporter;
struct iPluginManager;
struct iObjectRegistry;

namespace cspluginGenmesh3DS
{

struct csMatAndTris
{
  iMaterialWrapper* material;
  csDirtyAccessArray<unsigned int> tris;
};

/**
 * Genmesh factory loader for 3DS models.
 */
class csGenmesh3DSFactoryLoader : 
  public scfImplementation2<csGenmesh3DSFactoryLoader,
			    iBinaryLoaderPlugin,
			    iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRef<iSyntaxService> synldr;

  Lib3dsFile* LoadFileData (uint8* pBuffer, size_t size);
  bool Load (iLoaderContext* ldr_context,
  	iGeneralFactoryState* gmstate, uint8* buffer, size_t size);
  bool LoadMeshObjectData (iLoaderContext* ldr_context,
  	iGeneralFactoryState* gmstate, Lib3dsMesh *p3dsMesh,
	Lib3dsMaterial* pCurMaterial);

  csArray<csMatAndTris> materials_and_tris;

public:
  /// Constructor.
  csGenmesh3DSFactoryLoader (iBase*);

  /// Destructor.
  virtual ~csGenmesh3DSFactoryLoader ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Parse data  and return a new object for it.
  virtual csPtr<iBase> Parse (void* data, size_t size,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);
};

} // namespace cspluginGenmesh3DS

#endif // __CS_GMESH3DSLDR_H__
