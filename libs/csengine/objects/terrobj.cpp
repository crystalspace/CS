/*
    Copyright (C) 2001 by Jorrit Tyberghein
    Portions written by Richard D Shank

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

#include "cssysdef.h"
#include "csengine/sector.h"
#include "csengine/terrobj.h"
#include "csengine/engine.h"
#include "csengine/light.h"

IMPLEMENT_CSOBJTYPE (csTerrainWrapper, csObject)

IMPLEMENT_OBJECT_INTERFACE (csTerrainWrapper)
  IMPLEMENTS_EMBEDDED_OBJECT_TYPE (iTerrainWrapper)
IMPLEMENT_OBJECT_INTERFACE_END

IMPLEMENT_IBASE_EXT (csTerrainWrapper)
  IMPLEMENTS_EMBEDDED_INTERFACE (iTerrainWrapper)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csTerrainWrapper::TerrainWrapper)
  IMPLEMENTS_INTERFACE (iTerrainWrapper)
IMPLEMENT_EMBEDDED_IBASE_END


csTerrainWrapper::csTerrainWrapper( iEngine *pEng, iTerrainObject *pTerr )
	: csObject ()
{
  CONSTRUCT_EMBEDDED_IBASE (scfiTerrainWrapper);
  csTerrainWrapper::pEngine = pEng;
  csTerrainWrapper::pTerrObj = pTerr;
  pTerr->IncRef ();
  pFactory = NULL;
  csEngine::current_engine->AddToCurrentRegion (this);
}

csTerrainWrapper::csTerrainWrapper( iEngine *pEng )
	: csObject ()
{
  CONSTRUCT_EMBEDDED_IBASE (scfiTerrainWrapper);
  csTerrainWrapper::pEngine = pEng;
  csTerrainWrapper::pTerrObj = NULL;
  pFactory = NULL;
  csEngine::current_engine->AddToCurrentRegion (this);
}

void csTerrainWrapper::SetTerrainObject( iTerrainObject *pObj )
{
  if (pObj)
    pObj->IncRef ();
  if (csTerrainWrapper::pTerrObj)
    csTerrainWrapper::pTerrObj->DecRef ();
  csTerrainWrapper::pTerrObj = pObj;
}

csTerrainWrapper::~csTerrainWrapper ()
{
  if( pTerrObj )
    pTerrObj->DecRef();
  csEngine::current_engine->UnlinkTerrain (this);
}

void csTerrainWrapper::AddSector( csSector *pSector )
{
  if( !pSector )
    return;

  sectors.Push( pSector );
}

void csTerrainWrapper::ClearSectors()
{
  for( int i = 0 ; i < sectors.Length() ; i++ )
  {
    csSector *pSector = (csSector *)sectors[i];
    if( pSector )
      pSector->UnlinkTerrain( this );
  }

  sectors.SetLength (0);
}

void csTerrainWrapper::Draw( iRenderView *iRView, bool bUseZBuf )
{
// if (mesh->DrawTest (irv, imov))
// {
    pTerrObj->Draw( iRView, bUseZBuf );
// }

}

int csTerrainWrapper::CollisionDetect( csTransform *pTrans )
{
  return pTerrObj->CollisionDetect( pTrans );
}

void csTerrainWrapper::UpdateLighting( iLight** lights, int iNumLights )
{
  lights = lights;
  (void)iNumLights;
  // nothing doing for now
}

//--------------------------------------------------------------------------

IMPLEMENT_IBASE (csTerrainFactoryWrapper)
  IMPLEMENTS_EMBEDDED_INTERFACE (iTerrainFactoryWrapper)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csTerrainFactoryWrapper::TerrainFactoryWrapper)
  IMPLEMENTS_INTERFACE (iTerrainFactoryWrapper)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_CSOBJTYPE (csTerrainFactoryWrapper, csObject)

IMPLEMENT_OBJECT_INTERFACE (csTerrainFactoryWrapper)
  IMPLEMENTS_EMBEDDED_OBJECT_TYPE (iTerrainFactoryWrapper)
IMPLEMENT_OBJECT_INTERFACE_END

csTerrainFactoryWrapper::csTerrainFactoryWrapper (iTerrainObjectFactory *pFactory)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiTerrainFactoryWrapper);
  csTerrainFactoryWrapper::pTerrFact = pFactory;
  pTerrFact->IncRef ();
}

csTerrainFactoryWrapper::csTerrainFactoryWrapper ()
{
  CONSTRUCT_EMBEDDED_IBASE (scfiTerrainFactoryWrapper);
  csTerrainFactoryWrapper::pTerrFact = NULL;
}

csTerrainFactoryWrapper::~csTerrainFactoryWrapper ()
{
  if( pTerrFact )
    pTerrFact->DecRef();
}

void csTerrainFactoryWrapper::SetTerrainObjectFactory (iTerrainObjectFactory *pFactory)
{
  if( pFactory )
    pFactory->DecRef ();
  csTerrainFactoryWrapper::pTerrFact = pFactory;
  if( pFactory )
    pFactory->IncRef ();
}

csTerrainWrapper* csTerrainFactoryWrapper::NewTerrainObject( iEngine *pEngine )
{
  iTerrainObject *pTerrObj = pTerrFact->NewInstance ();
  csTerrainWrapper* pTerrain = new csTerrainWrapper( pEngine, pTerrObj );
  pTerrObj->DecRef ();
  pTerrain->SetFactory( this );
  return pTerrain;
}

