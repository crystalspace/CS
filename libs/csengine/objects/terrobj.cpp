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

IMPLEMENT_IBASE_EXT_QUERY (csTerrainWrapper)
  IMPLEMENTS_EMBEDDED_INTERFACE (iTerrainWrapper)
IMPLEMENT_IBASE_EXT_QUERY_END
IMPLEMENT_IBASE_EXT_INCREF(csTerrainWrapper)

// We implement a custom DecRef() in order to work around a shortcoming of the
// NextStep compiler.  The UnlinkTerrain(this) invocation which appears here
// used to appear in the destructor of this class.  During the processing of
// UnlinkTerrain(), QueryInterface(iMeshWrapper) is invoked on this object.
// Unfortunately, the NextStep compiler modifies the `vptr' of this object to
// point at its superclass' `vtbl' as soon as the destructor is entered, rather
// than modifying it after the destructor has completed, which is how all other
// compilers behave.  This early vptr modification, thus transmogrifies this
// object into its superclass (csObject) too early; before
// QueryInterface(iMeshWrapper) is invoked.  As a result, by the time
// UnlinkTerrain(this) was being called, the object already appeared to be a
// csObject and failed to respond positively to QueryInterface(iMeshWrapper).
// To work around this problem, the UnlinkTerrain() invocation was moved out of
// the destructor and into DecRef(), thus it is now called prior to the
// undesirable transmogrification.  Note that the csTerrainWrapper destructor
// is now private, thus it is ensured that terrain wrappers can only be
// destroyed via DecRef(), which is public.

void csTerrainWrapper::DecRef()
{
  if (scfRefCount <= 1) // About to be deleted...
  {
    // Since RemoveTerrain does DecRef() we first increase
    // ref count here.
    scfRefCount++;
    csEngine::current_engine->RemoveTerrain (this);
  }
  __scf_superclass::DecRef();
}

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
  if (pTerrObj)
    pTerrObj->DecRef();
}

void csTerrainWrapper::AddSector( csSector *pSector )
{
  if (!pSector)
    return;

  sectors.Push(pSector);
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

void csTerrainWrapper::Draw (iRenderView* rview, bool bUseZBuf)
{
  if (rview->GetCallback ())
  {
    rview->CallCallback (CALLBACK_VISMESH, (void*)&scfiTerrainWrapper);
  }
  else
  {
    pTerrObj->Draw (rview, bUseZBuf);
  }
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

IMPLEMENT_IBASE_EXT (csTerrainFactoryWrapper)
  IMPLEMENTS_EMBEDDED_INTERFACE (iTerrainFactoryWrapper)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csTerrainFactoryWrapper::TerrainFactoryWrapper)
  IMPLEMENTS_INTERFACE (iTerrainFactoryWrapper)
IMPLEMENT_EMBEDDED_IBASE_END

csTerrainFactoryWrapper::csTerrainFactoryWrapper (
  iTerrainObjectFactory *pFactory)
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

void csTerrainFactoryWrapper::SetTerrainObjectFactory (
  iTerrainObjectFactory *pFactory)
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
