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
	    object_reg(object_reg)
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

  // Obsoleted
  void FurMaterial::GenerateGeometry(iView *view,iSector *room, 
	int controlPoints, int numberOfStrains, float length)
  {
    CS_ASSERT(controlPoints > 1);
	CS_ASSERT(numberOfStrains > 1);

	this->view = view;

    csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
	if (!engine) csApplicationFramework::ReportError("Failed to locate iEngine plugin!");

	// First create the factory:
	csRef<iMeshFactoryWrapper> factory = engine->CreateMeshFactory (
		"crystalspace.mesh.object.genmesh", "hairFactory");

	factoryState = scfQueryInterface<iGeneralFactoryState> (
		factory->GetMeshObjectFactory ());

	factoryState -> SetVertexCount ( numberOfStrains * numberOfStrains * 2 * 
	  controlPoints );
	factoryState -> SetTriangleCount ( numberOfStrains * numberOfStrains * 2 * 
	  (controlPoints - 1));

	csVector3 *vbuf = factoryState->GetVertices (); 
    csTriangle *ibuf = factoryState->GetTriangles ();

	for ( int x = 0 ; x < numberOfStrains ; x ++ )
	  for ( int z = 0 ; z < numberOfStrains ; z ++ )
	{
	  for ( int y = 0 ; y < controlPoints ; y ++ )
	  {
		vbuf[ ( x * numberOfStrains + z ) * 2 * controlPoints + 2 * y].Set
		    ( csVector3( (float) x / (numberOfStrains - 1), 
		    y * length / (controlPoints - 1), (float) z / (numberOfStrains - 1)));
		vbuf[( x * numberOfStrains + z ) * 2 * controlPoints + 2 * y + 1].Set
		    ( csVector3( (float) x / (numberOfStrains - 1) + 0.01f, 
		    y * length / (controlPoints - 1), (float) z / (numberOfStrains - 1)));
	  }

	  for ( int y = 0 ; y < 2 * (controlPoints - 1) ; y ++ )
	  {
		if (y % 2 == 0)
		{
		  ibuf[( x * numberOfStrains + z ) * 2 * (controlPoints - 1) + y ].Set
			  ( 2 * ( x * numberOfStrains + z ) * controlPoints + y , 
			    2 * ( x * numberOfStrains + z ) * controlPoints + y + 3 , 
			    2 * ( x * numberOfStrains + z ) * controlPoints + y + 1 );
		  //printf("%d %d %d\n", 2 * x + y , 2 * x + y + 3 , 2 * x + y + 1);
		}
		else
		{
		  ibuf[( x * numberOfStrains + z ) * 2 * (controlPoints - 1) + y ].Set
			  ( 2 * ( x * numberOfStrains + z ) * controlPoints + y + 1 , 
			    2 * ( x * numberOfStrains + z ) * controlPoints + y + 2 , 
			    2 * ( x * numberOfStrains + z ) * controlPoints + y - 1 );
		  //printf("%d %d %d\n", 2 * x + y + 1 , 2 * x + y + 2 , 2 * x + y - 1);
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

	csRef<iGeneralMeshState> meshState =
		scfQueryInterface<iGeneralMeshState> (meshWrapper->GetMeshObject ());

	csRef<FurMaterialControl> animationControl;

	animationControl.AttachNew(new FurMaterialControl(meshWrapper->GetMeshObject ()));

	animationControl->SetGeneralFactoryState(factoryState);
	animationControl->SetView(view);

	meshState -> SetAnimationControl(animationControl);
  }

  void FurMaterial::GenerateGeometry (iView* view, iSector *room, 
    csRefArray<iBulletSoftBody> hairStrands)
  {
	this->view = view;
	int controlPoints = hairStrands.Get(0)->GetVertexCount();
	int numberOfStrains = hairStrands.GetSize();

    csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
	if (!engine) csApplicationFramework::ReportError("Failed to locate iEngine plugin!");

	// First create the factory:
	csRef<iMeshFactoryWrapper> factory = engine->CreateMeshFactory (
		"crystalspace.mesh.object.genmesh", "hairFactory");

	factoryState = scfQueryInterface<iGeneralFactoryState> (
		factory->GetMeshObjectFactory ());

	factoryState -> SetVertexCount ( numberOfStrains * 2 * 
	  controlPoints );
	factoryState -> SetTriangleCount ( numberOfStrains * 2 * 
	  (controlPoints - 1));

	csVector3 *vbuf = factoryState->GetVertices (); 
    csTriangle *ibuf = factoryState->GetTriangles ();

	for ( int x = 0 ; x < numberOfStrains ; x ++ )
	{
	  for ( int y = 0 ; y < controlPoints ; y ++ )
	  {
		vbuf[ x * 2 * controlPoints + 2 * y].Set
		    ( hairStrands.Get(x)->GetVertexPosition(y) );
		vbuf[ x * 2 * controlPoints + 2 * y + 1].Set
		    ( hairStrands.Get(x)->GetVertexPosition(y) + csVector3(0.01f,0,0) );
	  }

	  for ( int y = 0 ; y < 2 * (controlPoints - 1) ; y ++ )
	  {
		if (y % 2 == 0)
		{
		  ibuf[ x * 2 * (controlPoints - 1) + y ].Set
			  ( 2 * x * controlPoints + y , 
			    2 * x * controlPoints + y + 3 , 
			    2 * x * controlPoints + y + 1 );
		  //printf("%d %d %d\n", 2 * x + y , 2 * x + y + 3 , 2 * x + y + 1);
		}
		else
		{
		  ibuf[ x * 2 * (controlPoints - 1) + y ].Set
			  ( 2 * x * controlPoints + y + 1 , 
			    2 * x * controlPoints + y + 2 , 
			    2 * x * controlPoints + y - 1 );
		  //printf("%d %d %d\n", 2 * x + y + 1 , 2 * x + y + 2 , 2 * x + y - 1);
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

	csRef<iGeneralMeshState> meshState =
		scfQueryInterface<iGeneralMeshState> (meshWrapper->GetMeshObject ());

	csRef<FurMaterialControl> animationControl;

	animationControl.AttachNew(new FurMaterialControl(meshWrapper->GetMeshObject ()));

	animationControl->SetGeneralFactoryState(factoryState);
	animationControl->SetView(view);
	animationControl->SetHairStrands(hairStrands);

	meshState -> SetAnimationControl(animationControl);
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

  /********************
   *  FurMaterialControl
   ********************/

  CS_LEAKGUARD_IMPLEMENT(FurMaterialControl);

  FurMaterialControl::FurMaterialControl (iMeshObject* mesh)
    : scfImplementationType (this), mesh (mesh), lastTicks (0)
  {
  }

  FurMaterialControl::~FurMaterialControl ()
  {
  }  

  void FurMaterialControl::SetGeneralFactoryState(iGeneralFactoryState *factory)
  {
	factoryState = factory;
  }

  void FurMaterialControl::SetHairStrands(csRefArray<iBulletSoftBody> hairStrands)
  {
    this->hairStrands = hairStrands;
  }

  void FurMaterialControl::SetView (iView* view)
  {
    this->view = view;
  }

  bool FurMaterialControl::AnimatesColors () const
  {
    return false;
  }

  bool FurMaterialControl::AnimatesNormals () const
  {
	return false;
  }

  bool FurMaterialControl::AnimatesTexels () const
  {
	return false;
  }

  bool FurMaterialControl::AnimatesVertices () const
  {
	return true;
  }

  void FurMaterialControl::Update (csTicks current, int num_verts, uint32 version_id)
  {
    const csOrthoTransform& tc = view -> GetCamera() ->GetTransform ();
	
	int controlPoints = hairStrands.Get(0)->GetVertexCount();
	int numberOfStrains = hairStrands.GetSize();

	csVector3 *vbuf = factoryState->GetVertices (); 
	csTriangle *ibuf = factoryState->GetTriangles ();

	for ( int x = 0 ; x < numberOfStrains ; x ++ )
	{
	  for ( int y = 0 ; y < controlPoints ; y ++ )
	  {
		vbuf[ x * 2 * controlPoints + 2 * y].Set
		  ( hairStrands.Get(x)->GetVertexPosition(y) );
		vbuf[ x * 2 * controlPoints + 2 * y + 1].Set
		  ( ( hairStrands.Get(x)->GetVertexPosition(y) + 
		    csVector3(0.01f,0,0) ) );
	  }

	  int a, b, c;
	  for ( int y = 0 ; y < 2 * (controlPoints - 1) ; y ++ )
	  {
		if (y % 2 == 0)
		{
		  a = 2 * x * controlPoints + y;
		  b = 2 * x * controlPoints + y + 3;
		  c = 2 * x * controlPoints + y + 1;
			
		  if (csMath3::Visible(tc.GetOrigin(), vbuf[a], vbuf[b], vbuf[c]) )
		    ibuf[ x * 2 * (controlPoints - 1) + y ].Set( a, b, c );
		  else
			ibuf[ x * 2 * (controlPoints - 1) + y ].Set( a, c, b );
		}
		else
		{
		  a = 2 * x * controlPoints + y + 1;
		  b = 2 * x * controlPoints + y + 2;
		  c = 2 * x * controlPoints + y - 1;

		  if (csMath3::Visible(tc.GetOrigin(), vbuf[a], vbuf[b], vbuf[c]) )
		    ibuf[ x * 2 * (controlPoints - 1) + y ].Set( a, b, c );
		  else
			ibuf[ x * 2 * (controlPoints - 1) + y ].Set( a, c, b );
		}
	  }

	}
	factoryState -> CalculateNormals();
  }

  const csColor4* FurMaterialControl::UpdateColors (csTicks current, 
	const csColor4* colors, int num_colors, uint32 version_id)
  {
	return colors;
  }

  const csVector3* FurMaterialControl::UpdateNormals (csTicks current, 
	const csVector3* normals, int num_normals, uint32 version_id)
  {
	return normals;
  }

  const csVector2* FurMaterialControl::UpdateTexels (csTicks current, 
	const csVector2* texels, int num_texels, uint32 version_id)
  {
	return texels;
  }

  const csVector3* FurMaterialControl::UpdateVertices (csTicks current, 
	const csVector3* verts, int num_verts, uint32 version_id)
  {
	return 0;
  }
}

CS_PLUGIN_NAMESPACE_END(FurMaterial)
