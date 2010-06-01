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
	    object_reg(object_reg), physicsControl(0)
  {
    svStrings = csQueryRegistryTagInterface<iShaderVarStringSet> (
	  object_reg, "crystalspace.shader.variablenameset");
    if (!svStrings) 
	  printf ("No SV names string set!");
  }

  FurMaterial::~FurMaterial ()
  {
  }

  void FurMaterial::GenerateGeometry (iView* view, iSector *room)
  {
	iRenderBuffer* vertexes = meshFactory->GetVertices();
	//iRenderBuffer* texCoord = meshFactory->GetTexCoords();
	iRenderBuffer* indices = meshFactorySubMesh->GetIndices(0);

	GenerateGuidHairs(indices, vertexes);
	SynchronizeGuideHairs();
	GenerateHairStrands(indices, vertexes);

	this->view = view;
	int controlPoints = hairStrands.Get(0).controlPointsCount;
	int numberOfStrains = hairStrands.GetSize();

    csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
	if (!engine) csApplicationFramework::ReportError("Failed to locate iEngine plugin!");

	// First create the factory:
	csRef<iMeshFactoryWrapper> factory = engine->CreateMeshFactory (
		"crystalspace.mesh.object.genmesh", "hairFactory");

	factoryState = scfQueryInterface<iGeneralFactoryState> (
		factory->GetMeshObjectFactory ());

	factoryState -> SetVertexCount ( numberOfStrains * 2 * controlPoints );
	factoryState -> SetTriangleCount ( numberOfStrains * 2 * (controlPoints - 1));

	csVector3 *vbuf = factoryState->GetVertices (); 
    csTriangle *ibuf = factoryState->GetTriangles ();

	for ( int x = 0 ; x < numberOfStrains ; x ++ )
	{
	  for ( int y = 0 ; y < controlPoints ; y ++ )
	  {
		vbuf[ x * 2 * controlPoints + 2 * y].Set
		  ( hairStrands.Get(x).controlPoints[y] );
		vbuf[ x * 2 * controlPoints + 2 * y + 1].Set
		  ( hairStrands.Get(x).controlPoints[y] - csVector3(0.01f,0,0) );
	  }

	  for ( int y = 0 ; y < 2 * (controlPoints - 1) ; y ++ )
	  {
		if (y % 2 == 0)
		{
		  ibuf[ x * 2 * (controlPoints - 1) + y ].Set
			  ( 2 * x * controlPoints + y , 
			    2 * x * controlPoints + y + 1 , 
			    2 * x * controlPoints + y + 3 );
		  //printf("%d %d %d\n", 2 * x + y , 2 * x + y + 3 , 2 * x + y + 1);
		}
		else
		{
		  ibuf[ x * 2 * (controlPoints - 1) + y ].Set
			  ( 2 * x * controlPoints + y + 1 , 
			    2 * x * controlPoints + y - 1 , 
			    2 * x * controlPoints + y + 2 );
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

	csRef<FurAnimationControl> animationControl;
	animationControl.AttachNew(new FurAnimationControl(this));
	meshState -> SetAnimationControl(animationControl);
  }

  void FurMaterial::GenerateGuidHairs(iRenderBuffer* indices, iRenderBuffer* vertexes)
  {
	csRenderBufferLock<csVector3> positions (vertexes, CS_BUF_LOCK_READ);
	CS::TriangleIndicesStream<size_t> tris (indices, CS_MESHTYPE_TRIANGLES);    
	csArray<int> uniqueIndices;

	// chose unique indices
    while (tris.HasNext())
    {
      CS::TriangleT<size_t> tri (tris.Next ());

	  if(uniqueIndices.Contains(tri.a) == csArrayItemNotFound)
	    uniqueIndices.Push(tri.a);
	  if(uniqueIndices.Contains(tri.b) == csArrayItemNotFound)
	    uniqueIndices.Push(tri.b);
	  if(uniqueIndices.Contains(tri.c) == csArrayItemNotFound)
	    uniqueIndices.Push(tri.c);
    }

	// generate the guide hairs - this should be done based on heightmap
	for (size_t i = 0; i < uniqueIndices.GetSize(); i ++)
	{
  	  csVector3 pos = positions.Get(uniqueIndices.Get(i));
	  
	  csGuideHair guideHair;
	  guideHair.controlPointsCount = 5;
	  guideHair.controlPoints = new csVector3[ guideHair.controlPointsCount ];
	  
	  for ( size_t i = 0 ; i < guideHair.controlPointsCount ; i ++ )
		guideHair.controlPoints[i] = csVector3(pos.x,pos.y + i * 0.05f, pos.z);

	  guideHairs.Push(guideHair);
	}
  }

  void FurMaterial::SynchronizeGuideHairs ()
  {
	if (!physicsControl) // no physics support
	  return;

	for (size_t i = 0 ; i < guideHairs.GetSize(); i ++)
	  physicsControl->InitializeStrand(i,guideHairs.Get(i).controlPoints, 
	    guideHairs.Get(i).controlPointsCount);
  }

  void FurMaterial::GenerateHairStrands (iRenderBuffer* indices, iRenderBuffer* 
	vertexes)
  {
	csRenderBufferLock<csVector3> positions (vertexes, CS_BUF_LOCK_READ);
	CS::TriangleIndicesStream<size_t> tris (indices, CS_MESHTYPE_TRIANGLES); 
	csArray<int> uniqueIndices;

	csRandomGen rng (csGetTicks ());
	float bA, bB, bC; // barycentric coefficients

	int density = 10;

	// for every triangle
    while (tris.HasNext())
    {
      CS::TriangleT<size_t> tri (tris.Next ());

	  if(uniqueIndices.Contains(tri.a) == csArrayItemNotFound)
		  uniqueIndices.Push(tri.a);
	  if(uniqueIndices.Contains(tri.b) == csArrayItemNotFound)
		  uniqueIndices.Push(tri.b);
	  if(uniqueIndices.Contains(tri.c) == csArrayItemNotFound)
		  uniqueIndices.Push(tri.c);

	  for ( int i = 0 ; i < density ; i ++ )
	  {
		csHairStrand hairStrand;

		bA = rng.Get();
		bB = rng.Get() * (1 - bA);
		bC = 1 - bA - bB;

		hairStrand.guideHairsCount = 3;
		hairStrand.guideHairs = new csGuideHairReference[hairStrand.guideHairsCount];

		hairStrand.guideHairs[0].distance = bA;
		hairStrand.guideHairs[0].index = uniqueIndices.Contains(tri.a);
		hairStrand.guideHairs[1].distance = bB;
		hairStrand.guideHairs[1].index = uniqueIndices.Contains(tri.b);
		hairStrand.guideHairs[2].distance = bC;
		hairStrand.guideHairs[2].index = uniqueIndices.Contains(tri.c);

		csVector3 pos = bA * positions.Get(tri.a) + bB * positions.Get(tri.b) +
		  bC * positions.Get(tri.c);

		hairStrand.controlPointsCount = 5;
		hairStrand.controlPoints = new csVector3[ hairStrand.controlPointsCount ];

		for ( size_t i = 0 ; i < hairStrand.controlPointsCount ; i ++ )
		  hairStrand.controlPoints[i] = csVector3(pos.x,pos.y + i * 0.05f, pos.z);

		hairStrands.Push(hairStrand);		
	  }
    }
  }

  void FurMaterial::SetPhysicsControl (iFurPhysicsControl* physicsControl)
  {
	this->physicsControl = physicsControl;
  }

  void FurMaterial::SetMeshFactory ( iAnimatedMeshFactory* meshFactory)
  {
	this->meshFactory = meshFactory;
  }

  void FurMaterial::SetMeshFactorySubMesh ( iAnimatedMeshFactorySubMesh* 
	meshFactorySubMesh )
  { 
	this->meshFactorySubMesh = meshFactorySubMesh;
  }

  void FurMaterial::SetMaterial ( iMaterial* material )
  {
	this->material = material;
	SetDensitymap();
	SetHeightmap();
  }

  void FurMaterial::SetDensitymap ()
  {
	CS::ShaderVarName densitymapName (svStrings, "density map");	
	csRef<csShaderVariable> shaderVariable = 
		material->GetVariable(densitymapName);

	shaderVariable->GetValue(densitymap);
	//printf("%s\n", densitymap->GetImageName());
  }

  void FurMaterial::SetHeightmap ()
  {
	CS::ShaderVarName heightmapName (svStrings, "height map");	
	csRef<csShaderVariable> shaderVariable = 
		material->GetVariable(heightmapName);

	shaderVariable->GetValue(heightmap);
	//printf("%s\n", heightmap->GetImageName());
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
   *  FurAnimationControl
   ********************/

  CS_LEAKGUARD_IMPLEMENT(FurAnimationControl);

  FurAnimationControl::FurAnimationControl (FurMaterial* furMaterial)
    : scfImplementationType (this), lastTicks (0), furMaterial(furMaterial)
  {
  }

  FurAnimationControl::~FurAnimationControl ()
  {
  }  

  bool FurAnimationControl::AnimatesColors () const
  {
    return false;
  }

  bool FurAnimationControl::AnimatesNormals () const
  {
	return false;
  }

  bool FurAnimationControl::AnimatesTexels () const
  {
	return false;
  }

  bool FurAnimationControl::AnimatesVertices () const
  {
	return true;
  }

  void FurAnimationControl::UpdateHairStrand (csHairStrand* hairStrand)
  {
	for ( size_t i = 0 ; i < hairStrand->controlPointsCount; i++ )
	{
	  hairStrand->controlPoints[i] = csVector3(0);
	  for ( size_t j = 0 ; j < hairStrand->guideHairsCount ; j ++ )
	    hairStrand->controlPoints[i] += hairStrand->guideHairs[j].distance * 
		  furMaterial->guideHairs.Get(hairStrand->guideHairs[j].index).controlPoints[i];
	}
  }

  void FurAnimationControl::Update (csTicks current, int num_verts, uint32 version_id)
  {
	  
	// first update the control points
	if (furMaterial->physicsControl)
	  for (size_t i = 0 ; i < furMaterial->guideHairs.GetSize(); i ++)
		furMaterial->physicsControl->AnimateStrand(i,
		  furMaterial->guideHairs.Get(i).controlPoints,
		  furMaterial->guideHairs.Get(i).controlPointsCount);

	// then update the hair strands
	if (furMaterial->physicsControl)
	  for (size_t i = 0 ; i < furMaterial->hairStrands.GetSize(); i ++)
		UpdateHairStrand(&furMaterial->hairStrands.Get(i));

    const csOrthoTransform& tc = furMaterial->view -> GetCamera() ->GetTransform ();
	
	int controlPoints = furMaterial->hairStrands.Get(0).controlPointsCount;
	int numberOfStrains = furMaterial->hairStrands.GetSize();

	csVector3 *vbuf = furMaterial->factoryState->GetVertices (); 

	for ( int x = 0 ; x < numberOfStrains ; x ++ )
	{
	  int y = 0;
	  csVector3 strip = csVector3(0);

	  for ( y = 0 ; y < controlPoints - 1 ; y ++ )
	  {
		csVector2 firstPoint = csVector2(furMaterial->hairStrands.Get(x).controlPoints[y].x,
		  furMaterial->hairStrands.Get(x).controlPoints[y].y);
		csVector2 secondPoint = csVector2(furMaterial->hairStrands.Get(x).controlPoints[y + 1].x,
		  furMaterial->hairStrands.Get(x).controlPoints[y + 1].y);

		csVector2 diff = firstPoint - secondPoint;
		if (diff.Norm() > 0.0001f)
		  diff = diff / diff.Norm();
		else
		  diff = csVector2(0);

		strip = 0.005f * csVector3(diff.y,diff.x,0);

		vbuf[ x * 2 * controlPoints + 2 * y].Set
		  ( furMaterial->hairStrands.Get(x).controlPoints[y] );
		vbuf[ x * 2 * controlPoints + 2 * y + 1].Set
		  ( furMaterial->hairStrands.Get(x).controlPoints[y] + 
		    tc.GetT2O() * strip );
	  }

	  vbuf[ x * 2 * controlPoints + 2 * y].Set
		( furMaterial->hairStrands.Get(x).controlPoints[y] );
	  vbuf[ x * 2 * controlPoints + 2 * y + 1].Set
		( furMaterial->hairStrands.Get(x).controlPoints[y] + 
		  tc.GetT2O() * strip );	  
	}
	furMaterial->factoryState -> CalculateNormals();
	
  }

  const csColor4* FurAnimationControl::UpdateColors (csTicks current, 
	const csColor4* colors, int num_colors, uint32 version_id)
  {
	return colors;
  }

  const csVector3* FurAnimationControl::UpdateNormals (csTicks current, 
	const csVector3* normals, int num_normals, uint32 version_id)
  {
	return normals;
  }

  const csVector2* FurAnimationControl::UpdateTexels (csTicks current, 
	const csVector2* texels, int num_texels, uint32 version_id)
  {
	return texels;
  }

  const csVector3* FurAnimationControl::UpdateVertices (csTicks current, 
	const csVector3* verts, int num_verts, uint32 version_id)
  {
	return 0;
  }


  /********************
   *  FurPhysicsControl
   ********************/
  SCF_IMPLEMENT_FACTORY (FurPhysicsControl)

  CS_LEAKGUARD_IMPLEMENT(FurPhysicsControl);	

  FurPhysicsControl::FurPhysicsControl (iBase* parent)
    : scfImplementationType (this, parent), object_reg(0)
  {
  }

  FurPhysicsControl::~FurPhysicsControl ()
  {
	guideRopes.DeleteAll();
  }

  // From iComponent
  bool FurPhysicsControl::Initialize (iObjectRegistry* r)
  {
    object_reg = r;
    return true;
  }

  //-- iFurPhysicsControl
  
  void FurPhysicsControl::SetRigidBody (iRigidBody* rigidBody)
  {
	this->rigidBody = rigidBody;
  }

  void FurPhysicsControl::SetBulletDynamicSystem (iBulletDynamicSystem* 
	bulletDynamicSystem)
  {
	this->bulletDynamicSystem = bulletDynamicSystem;
  }

  // Initialize the strand with the given ID
  void FurPhysicsControl::InitializeStrand (size_t strandID, const csVector3* 
	coordinates, size_t coordinatesCount)
  {
  	csVector3 first = coordinates[0];
	csVector3 last = coordinates[coordinatesCount - 1];
	
	iBulletSoftBody* bulletBody = bulletDynamicSystem->
	  CreateRope(first, last, coordinatesCount);
	bulletBody->SetMass (0.1f);
	bulletBody->SetRigidity (0.99f);
	bulletBody->AnchorVertex (0, rigidBody);
	
	guideRopes.PutUnique(strandID, bulletBody);
  }

  // Animate the strand with the given ID
  void FurPhysicsControl::AnimateStrand (size_t strandID, csVector3* 
	coordinates, size_t coordinatesCount)
  {
    csRef<iBulletSoftBody> bulletBody = guideRopes.Get (strandID, 0);

	if(!bulletBody)
	  return;

	CS_ASSERT(coordinatesCount != bulletBody->GetVertexCount());

	for ( size_t i = 0 ; i < coordinatesCount ; i ++ )
	  coordinates[i] = bulletBody->GetVertexPosition(i);
  }

  void FurPhysicsControl::RemoveStrand (size_t strandID)
  {
	csRef<iBulletSoftBody> bulletBody = guideRopes.Get (strandID, 0);
	if(!bulletBody)
		return;

    guideRopes.Delete(strandID, bulletBody);
  }

  void FurPhysicsControl::RemoveAllStrands ()
  {
	guideRopes.DeleteAll();
  }

}
CS_PLUGIN_NAMESPACE_END(FurMaterial)

