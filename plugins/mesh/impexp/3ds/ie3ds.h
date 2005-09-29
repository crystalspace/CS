/*
    Written by Richard D Shank
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

#ifndef __CS_IE3DS_H__
#define __CS_IE3DS_H__

#include "imesh/mdlconv.h"
#include <lib3ds/types.h>

class csModelConverter3ds : public iModelConverter
{
private:
  iObjectRegistry *object_reg;
  iModelData *pModelData;
  csModelConverterFormat FormatInfo;

//bool LoadCameraData( iCamera *pCSCamera, Lib3dsCamera *p3dsCamera );
//bool LoadLightData( iLight *pCSight, Lib3dsLight *p3dsLight );
//bool LoadMaterialData( iMaterial *pCSMaterial, Lib3dsMaterial *p3dsMaterial );
  bool LoadMeshObjectData( iModelDataObject *pData, Lib3dsMesh *p3dsMesh, Lib3dsMaterial* pCurMaterial );

  Lib3dsFile * LoadFileData( uint8* Buffer, size_t size );

public:
  SCF_DECLARE_IBASE;

  /// constructor
  csModelConverter3ds( iBase *pBase );

  /// destructor
  virtual ~csModelConverter3ds();

  bool Initialize( iObjectRegistry *object_reg );
  virtual size_t GetFormatCount();
  virtual const csModelConverterFormat *GetFormat( size_t idx );
  virtual csPtr<iModelData> Load( uint8* Buffer, size_t size );
  virtual csPtr<iDataBuffer> Save( iModelData*, const char *format );

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE( csModelConverter3ds );
    virtual bool Initialize (iObjectRegistry *object_reg)
    {
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;
};

#endif // __CS_IE3DS_H__
