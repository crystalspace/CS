/*
  Copyright (C) 2010 Alexandru - Teodor Voicu

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include <cssysdef.h>
#include <iutil/objreg.h>
#include <iutil/plugin.h>

#include "furmaterial.h"

CS_PLUGIN_NAMESPACE_BEGIN(FurMaterial)
{
  /********************
   *  FurMaterialType
   ********************/

  SCF_IMPLEMENT_FACTORY (FurMaterialType)

  CS_LEAKGUARD_IMPLEMENT(FurMaterialType);

  FurMaterialType::FurMaterialType (iBase* parent) :
  scfImplementationType (this, parent),
  object_reg(0)
  {
  }

  FurMaterialType::~FurMaterialType ()
  {
    furMaterialHash.DeleteAll();
  }

  // From iComponent
  bool FurMaterialType::Initialize (iObjectRegistry* r)
  {
    object_reg = r;
    return true;
  }

  // From iFurMaterialType
  void FurMaterialType::ClearFurMaterials ()
  {
    furMaterialHash.DeleteAll ();
  }

  void FurMaterialType::RemoveFurMaterial (const char *name, iFurMaterial* furMaterial)
  {
    furMaterialHash.Delete (name, furMaterial);
  }

  iFurMaterial* FurMaterialType::CreateFurMaterial (const char *name)
  {
    csRef<iFurMaterial> newFur;
    newFur.AttachNew(new FurMaterial (this, name, object_reg));
    return furMaterialHash.PutUnique (name, newFur);
  }

  iFurMaterial* FurMaterialType::FindFurMaterial (const char *name) const
  {
    return furMaterialHash.Get (name, 0);
  }

  /********************
   *  FurMaterial
   ********************/

  CS_LEAKGUARD_IMPLEMENT(FurMaterial);

  FurMaterial::FurMaterial (FurMaterialType* manager, const char *name, 
	iObjectRegistry* object_reg) :
      scfImplementationType (this), manager (manager), name (name), 
	    object_reg(object_reg), length(0.5f)
  {
  }

  FurMaterial::~FurMaterial ()
  {
  }

  void FurMaterial::DoSomething (int param, const csVector3& v)
  {
    // Just some behavior.
    if (param == 1)
      store_v = v;
    else
      store_v = -v;
  }

  int FurMaterial::GetSomething () const
  {
    return (int)store_v.x + (int)store_v.y + (int)store_v.z;
  }

  void FurMaterial::SetLength (float len)
  {
	length = len;
  }

  float FurMaterial::GetLength () const
  {
    return length;
  }

  void FurMaterial::GenerateGeometry(iSector *room, int controlPoints, int numberOfStrains)
  {
    assert(controlPoints > 1);

	this->controlPoints = controlPoints;
	this->numberOfStrains = numberOfStrains;

    csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
	if (!engine) csApplicationFramework::ReportError("Failed to locate iEngine plugin!");

	// First create the factory:
	csRef<iMeshFactoryWrapper> factory = engine->CreateMeshFactory (
		"crystalspace.mesh.object.genmesh", "hairFactory");

	factoryState = scfQueryInterface<iGeneralFactoryState> (
		factory->GetMeshObjectFactory ());

	factoryState -> SetVertexCount (numberOfStrains * 2 * controlPoints);
	factoryState -> SetTriangleCount ( numberOfStrains * 2 * (controlPoints - 1));

	for ( int i = 0 ; i < numberOfStrains ; i ++ )
	{
	  for ( int j = 0 ; j < controlPoints ; j ++ )
	  {
		factoryState -> GetVertices()[i * 2 * controlPoints + 2 * j].Set
		  ( csVector3(i/10.0f, j * length / (controlPoints - 1), -1));
		factoryState -> GetVertices()[i * 2 * controlPoints + 2 * j + 1].Set
		  ( csVector3(i/10.0f + 0.01f, j * length / (controlPoints - 1), -1));
	  }

	  for ( int j = 0 ; j < 2 * (controlPoints - 1) ; j ++ )
	  {
		if (j % 2 == 0)
		{
	      factoryState -> GetTriangles( )[i * 2 * (controlPoints - 1) + j ].Set
		    ( 2 * i + i * 2 * (controlPoints - 1) + j , 
			  2 * i + i * 2 * (controlPoints - 1) + j + 3 , 
			  2 * i + i * 2 * (controlPoints - 1) + j + 1 );
		  //printf("%d %d %d\n", 2 * i + j , 2 * i + j + 3 , 2 * i + j + 1);
		}
		else
		{
		  factoryState -> GetTriangles( )[i * 2 * (controlPoints - 1) + j ].Set
			( 2 * i + i * 2 * (controlPoints - 1) + j + 1 , 
			  2 * i + i * 2 * (controlPoints - 1) + j + 2 , 
			  2 * i + i * 2 * (controlPoints - 1) + j - 1 );
		  //printf("%d %d %d\n", 2 * i + j + 1 , 2 * i + j + 2 , 2 * i + j - 1);
		}
	  }
	}

	factoryState -> CalculateNormals();
	
	// Make a ball using the genmesh plug-in.
	csRef<iMeshWrapper> meshWrapper =
		engine->CreateMeshWrapper (factory, "hair", room, csVector3 (0, 0, 0));

	csRef<iMaterialWrapper> materialWrapper = 
		CS::Material::MaterialBuilder::CreateColorMaterial
		(object_reg,"hairDummyMaterial",csColor(0,1,0));

	meshWrapper -> GetMeshObject() -> SetMaterialWrapper(materialWrapper);
  }

  void FurMaterial::UpdateGeometry (iView *view)
  {
    const csOrthoTransform& tc = view -> GetCamera() ->GetTransform ();

	factoryState -> SetVertexCount (numberOfStrains * 2 * controlPoints);
	factoryState -> SetTriangleCount ( numberOfStrains * 2 * (controlPoints - 1));

	for ( int i = 0 ; i < numberOfStrains ; i ++ )
	{
	  for ( int j = 0 ; j < controlPoints ; j ++ )
	  {
		factoryState -> GetVertices()[i * 2 * controlPoints + 2 * j].Set
		  ( tc.GetT2O() * csVector3(i/10.0f, j * length / (controlPoints - 1), -1));
		factoryState -> GetVertices()[i * 2 * controlPoints + 2 * j + 1].Set
		  ( tc.GetT2O() * csVector3(i/10.0f + 0.01f, j * length / (controlPoints - 1), -1));
	  }

	  for ( int j = 0 ; j < 2 * (controlPoints - 1) ; j ++ )
	  {
		if (j % 2 == 0)
		{
	      factoryState -> GetTriangles( )[i * 2 * (controlPoints - 1) + j ].Set
		    ( 2 * i + i * 2 * (controlPoints - 1) + j , 
			  2 * i + i * 2 * (controlPoints - 1) + j + 3 , 
			  2 * i + i * 2 * (controlPoints - 1) + j + 1 );
		  //printf("%d %d %d\n", 2 * i + j , 2 * i + j + 3 , 2 * i + j + 1);
		}
		else
		{
		  factoryState -> GetTriangles( )[i * 2 * (controlPoints - 1) + j ].Set
			( 2 * i + i * 2 * (controlPoints - 1) + j + 1 , 
			  2 * i + i * 2 * (controlPoints - 1) + j + 2 , 
			  2 * i + i * 2 * (controlPoints - 1) + j - 1 );
		  //printf("%d %d %d\n", 2 * i + j + 1 , 2 * i + j + 2 , 2 * i + j - 1);
		}
	  }
	}

	factoryState -> CalculateNormals();
  }

  void FurMaterial::SetShader (csStringID type, iShader* shd)
  {
	  shaders.PutUnique (type, shd);
  }

  iShader* FurMaterial::GetShader(csStringID type)
  {
	  return shaders.Get (type, (iShader*)0);
  }

  iShader* FurMaterial::GetFirstShader (const csStringID* types,
	  size_t numTypes)
  {
	  iShader* s = 0;
	  for (size_t i = 0; i < numTypes; i++)
	  {
		  s = shaders.Get (types[i], (iShader*)0);
		  if (s != 0) break;
	  }
	  return s;
  }

  iTextureHandle *FurMaterial::GetTexture ()
  {
	  return 0;
  }

  iTextureHandle* FurMaterial::GetTexture (CS::ShaderVarStringID name)
  {
	  return 0;
  }  
}

CS_PLUGIN_NAMESPACE_END(FurMaterial)
