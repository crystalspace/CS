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
#include "hairphysicscontrol.h"
#include "hairstrandgenerator.h"

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
    object_reg(object_reg), physicsControl(0), hairStrandGenerator(0)
  {
    svStrings = csQueryRegistryTagInterface<iShaderVarStringSet> (
      object_reg, "crystalspace.shader.variablenameset");

    if (!svStrings) 
      printf ("No SV names string set!");

    engine = csQueryRegistry<iEngine> (object_reg);
    if (!engine) printf ("Failed to locate 3D engine!");

    loader = csQueryRegistry<iLoader> (object_reg);
    if (!loader) printf ("Failed to locate Loader!");

    rng = new csRandomGen(csGetTicks());
  }

  FurMaterial::~FurMaterial ()
  {
    delete rng;
  }

  void FurMaterial::GenerateGeometry (iView* view, iSector *room)
  {
    iRenderBuffer* vertexes = meshFactory->GetVertices();
    iRenderBuffer* normals = meshFactory->GetNormals();
    iRenderBuffer* indices = meshFactorySubMesh->GetIndices(0);
    iRenderBuffer* texCoords = meshFactory->GetTexCoords();

    GenerateGuidHairs(indices, vertexes, normals, texCoords);
    GenerateGuideHairsLOD();
    SynchronizeGuideHairs();
    GenerateHairStrands();

    SetLOD(0.0f);

    SaveUVImage();

    this->view = view;
    int numberOfStrains = hairStrands.GetSize();

    if( !numberOfStrains ) 
      return;

    int controlPoints = hairStrands.Get(0).controlPointsCount;

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
          ( hairStrands.Get(x).controlPoints[y] + csVector3(-0.01f,0,0) );
      }

      for ( int y = 0 ; y < 2 * (controlPoints - 1) ; y ++ )
      {
        if (y % 2 == 0)
        {
          ibuf[ x * 2 * (controlPoints - 1) + y ].Set
            ( 2 * x * controlPoints + y , 
            2 * x * controlPoints + y + 1 , 
            2 * x * controlPoints + y + 2 );
          //printf("%d %d %d\n", 2 * x + y , 2 * x + y + 3 , 2 * x + y + 1);
        }
        else
        {
          ibuf[ x * 2 * (controlPoints - 1) + y ].Set
            ( 2 * x * controlPoints + y , 
            2 * x * controlPoints + y + 2 , 
            2 * x * controlPoints + y + 1 );
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
      (object_reg,"hairDummyMaterial",csColor(1,0,0));

    if (hairStrandGenerator && hairStrandGenerator->GetMaterial())
      materialWrapper->SetMaterial(hairStrandGenerator->GetMaterial());

    meshWrapper -> GetMeshObject() -> SetMaterialWrapper(materialWrapper);

    csRef<iGeneralMeshState> meshState =
      scfQueryInterface<iGeneralMeshState> (meshWrapper->GetMeshObject ());

    csRef<FurAnimationControl> animationControl;
    animationControl.AttachNew(new FurAnimationControl(this));
    meshState -> SetAnimationControl(animationControl);
  }

  void FurMaterial::GenerateGuidHairs(iRenderBuffer* indices, 
    iRenderBuffer* vertexes, iRenderBuffer* normals, iRenderBuffer* texCoords)
  {
    csRenderBufferLock<csVector3> positions (vertexes, CS_BUF_LOCK_READ);
    csRenderBufferLock<csVector2> UV (texCoords, CS_BUF_LOCK_READ);
    csRenderBufferLock<csVector3> norms (normals, CS_BUF_LOCK_READ);
    CS::TriangleIndicesStream<size_t> tris (indices, CS_MESHTYPE_TRIANGLES);    
    csArray<int> uniqueIndices;

    // choose unique indices
    while (tris.HasNext())
    {
      CS::TriangleT<size_t> tri (tris.Next ());

      if(uniqueIndices.Contains(tri.a) == csArrayItemNotFound)
        uniqueIndices.Push(tri.a);
      if(uniqueIndices.Contains(tri.b) == csArrayItemNotFound)
        uniqueIndices.Push(tri.b);
      if(uniqueIndices.Contains(tri.c) == csArrayItemNotFound)
        uniqueIndices.Push(tri.c);

      csTriangle triangleNew = csTriangle(uniqueIndices.Contains(tri.a),
        uniqueIndices.Contains(tri.b), uniqueIndices.Contains(tri.c));
      guideHairsTriangles.Push(triangleNew);
    }

    // generate the guide hairs - this should be done based on heightmap
    for (size_t i = 0; i < uniqueIndices.GetSize(); i ++)
    {
      csVector3 pos = positions.Get(uniqueIndices.Get(i)) + 
        displaceEps * norms.Get(uniqueIndices.Get(i));

      csGuideHair guideHair;
      guideHair.uv = UV.Get(uniqueIndices.Get(i));;
      guideHair.controlPointsCount = 5;
      guideHair.controlPoints = new csVector3[ guideHair.controlPointsCount ];

      for ( size_t j = 0 ; j < guideHair.controlPointsCount ; j ++ )
        guideHair.controlPoints[j] = pos + j * 0.05f * norms.Get(uniqueIndices.Get(i));

      guideHairs.Push(guideHair);
    }
  }

  void FurMaterial::GenerateGuideHairsLOD()
  {
    // density map
    CS::StructuredTextureFormat readbackFmt 
      (CS::TextureFormatStrings::ConvertStructured ("abgr8"));

    csRef<iDataBuffer> densitymapDB = densitymap->Readback(readbackFmt);
    int densitymapW, densitymapH;
    densitymap->GetOriginalDimensions(densitymapW, densitymapH);
    uint8* densitymapData = densitymapDB->GetUint8();

    // generate guide hairs for LOD
    for (size_t iter = 0 ; iter < guideHairsTriangles.GetSize(); iter ++)
    {
      csTriangle currentTriangle = guideHairsTriangles.Get(iter);

      size_t indexA = currentTriangle.a;
      size_t indexB = currentTriangle.b;
      size_t indexC = currentTriangle.c;

      csGuideHair A, B, C;
      
      if ( indexA < guideHairs.GetSize())
        A = guideHairs.Get(indexA);
      else
        A = guideHairsLOD.Get(indexA - guideHairs.GetSize());

      if ( indexB < guideHairs.GetSize())
        B = guideHairs.Get(indexB);
      else
        B = guideHairsLOD.Get(indexB - guideHairs.GetSize());

      if ( indexC < guideHairs.GetSize())
        C = guideHairs.Get(indexC);
      else
        C = guideHairsLOD.Get(indexC - guideHairs.GetSize());

      // average density - modify to use convolution matrix or such
      float density = (densitymapData[ 4 * ((int)(A.uv.x * densitymapW) + 
        (int)(A.uv.y * densitymapH) * densitymapW )] + 
        densitymapData[ 4 * ((int)(B.uv.x * densitymapW) + 
        (int)(B.uv.y * densitymapH) * densitymapW )] + 
        densitymapData[ 4 * ((int)(C.uv.x * densitymapW) + 
        (int)(C.uv.y * densitymapH) * densitymapW )] ) / (3.0f);
 
      // triangle area
      if ( ( A.controlPointsCount == 0 ) || ( B.controlPointsCount == 0 ) ||
        ( C.controlPointsCount == 0 ) )
        continue;

      float a, b, c;
      a = csVector3::Norm(B.controlPoints[0] - C.controlPoints[0]);
      b = csVector3::Norm(A.controlPoints[0] - C.controlPoints[0]);
      c = csVector3::Norm(B.controlPoints[0] - A.controlPoints[0]);

      // just for debug
//       a = csVector2::Norm(B.uv - C.uv);
//       b = csVector2::Norm(A.uv - C.uv);
//       c = csVector2::Norm(B.uv - A.uv);

      float s = (a + b + c) / 2.0f;
      float area = sqrt(s * (s - a) * (s - b) * (s - c));

      //csPrintf("%f\t%f\t%f\t%f\n", density, area, density * area, densityFactor);

      // if a new guide hair is needed
      if ( (density * area) < densityFactor)
        continue;

      // make new guide hair
      csGuideHairLOD guideHairLOD;
      float bA, bB, bC; // barycentric coefficients

      bA = rng->Get();
      bB = rng->Get() * (1 - bA);
      bC = 1 - bA - bB;

      guideHairLOD.guideHairs[0].distance = bA;
      guideHairLOD.guideHairs[0].index = indexA;
      guideHairLOD.guideHairs[1].distance = bB;
      guideHairLOD.guideHairs[1].index = indexB;
      guideHairLOD.guideHairs[2].distance = bC;
      guideHairLOD.guideHairs[2].index = indexC;

      guideHairLOD.isActive = false;

      guideHairLOD.uv = A.uv * bA + B.uv * bB + C.uv * bC;

      // generate control points
      guideHairLOD.controlPointsCount = csMin( (csMin( A.controlPointsCount,
        B.controlPointsCount) ), C.controlPointsCount);

      guideHairLOD.controlPoints = new csVector3[ guideHairLOD.controlPointsCount ];

      for ( size_t i = 0 ; i < guideHairLOD.controlPointsCount ; i ++ )
      {
        guideHairLOD.controlPoints[i] = csVector3(0);
        for ( size_t j = 0 ; j < GUIDE_HAIRS_COUNT ; j ++ )
          if ( guideHairLOD.guideHairs[j].index < guideHairs.GetSize() )
            guideHairLOD.controlPoints[i] += guideHairLOD.guideHairs[j].distance *
              guideHairs.Get(guideHairLOD.guideHairs[j].index).controlPoints[i];
          else
            guideHairLOD.controlPoints[i] += guideHairLOD.guideHairs[j].distance *
              guideHairsLOD.Get(guideHairLOD.guideHairs[j].index - 
              guideHairs.GetSize()).controlPoints[i];
      }

      // add new triangles
      guideHairsLOD.Push(guideHairLOD);
      size_t indexD = guideHairsLOD.GetSize() - 1 + guideHairs.GetSize();
      csTriangle ADC = csTriangle(indexA, indexD, indexC);
      csTriangle ADB = csTriangle(indexA, indexD, indexB);
      csTriangle BDC = csTriangle(indexB, indexD, indexC);

      guideHairsTriangles.Push(ADC);
      guideHairsTriangles.Push(ADB);
      guideHairsTriangles.Push(BDC);
    }    

    //csPrintf("end generate guide hairs LOD\n");
  }

  void FurMaterial::SynchronizeGuideHairs ()
  {
    if (!physicsControl) // no physics support
      return;

    for (size_t i = 0 ; i < guideHairs.GetSize(); i ++)
      physicsControl->InitializeStrand(i,guideHairs.Get(i).controlPoints, 
        guideHairs.Get(i).controlPointsCount);
  }

  void FurMaterial::GenerateHairStrands ()
  {
    float bA, bB, bC; // barycentric coefficients

    float density = 5;

    CS::ShaderVarName densityName (svStrings, "density");	
    csRef<csShaderVariable> shaderVariable = material->GetVariable(densityName);

    if (shaderVariable)
      shaderVariable->GetValue(density);

    //csPrintf("start\n");

    // for every triangle
    for (size_t iter = 0 ; iter < guideHairsTriangles.GetSize(); iter ++)
    {
      for ( int den = 0 ; den < density ; den ++ )
      {
        csHairStrand hairStrand;

        bA = rng->Get();
        bB = rng->Get() * (1 - bA);
        bC = 1 - bA - bB;

        size_t indexA = guideHairsTriangles.Get(iter).a;
        size_t indexB = guideHairsTriangles.Get(iter).b;
        size_t indexC = guideHairsTriangles.Get(iter).c;

        csGuideHair A, B, C;

        if ( indexA < guideHairs.GetSize())
          A = guideHairs.Get(indexA);
        else
          A = guideHairsLOD.Get(indexA - guideHairs.GetSize());

        if ( indexB < guideHairs.GetSize())
          B = guideHairs.Get(indexB);
        else
          B = guideHairsLOD.Get(indexB - guideHairs.GetSize());

        if ( indexC < guideHairs.GetSize())
          C = guideHairs.Get(indexC);
        else
          C = guideHairsLOD.Get(indexC - guideHairs.GetSize());

        //csPrintf("%d\t%d\t%d\n", indexA, indexB, indexC);

        hairStrand.guideHairs[0].distance = bA;
        hairStrand.guideHairs[0].index = indexA;
        hairStrand.guideHairs[1].distance = bB;
        hairStrand.guideHairs[1].index = indexB;
        hairStrand.guideHairs[2].distance = bC;
        hairStrand.guideHairs[2].index = indexC;

        hairStrand.controlPointsCount = csMin( (csMin( A.controlPointsCount,
          B.controlPointsCount) ), C.controlPointsCount);

        hairStrand.controlPoints = new csVector3[ hairStrand.controlPointsCount ];

        for ( size_t i = 0 ; i < hairStrand.controlPointsCount ; i ++ )
        {
          hairStrand.controlPoints[i] = csVector3(0);
          for ( size_t j = 0 ; j < GUIDE_HAIRS_COUNT ; j ++ )
            if ( hairStrand.guideHairs[j].index < guideHairs.GetSize() )
              hairStrand.controlPoints[i] += hairStrand.guideHairs[j].distance *
                guideHairs.Get(hairStrand.guideHairs[j].index).controlPoints[i];
            else
              hairStrand.controlPoints[i] += hairStrand.guideHairs[j].distance *
                guideHairsLOD.Get(hairStrand.guideHairs[j].index - 
                guideHairs.GetSize()).controlPoints[i];
        }

        hairStrands.Push(hairStrand);		
      }
    }
    //csPrintf("end\n");
  }

  void FurMaterial::SetLOD(float LOD)
  {
    // deactivate all
    if (!physicsControl) // no physics support
      return;

    // LOD ropes use ropes as well
    for (size_t i = 1 ; i < guideHairsLOD.GetSize(); i ++)
      if ( rng->Get() < LOD )
      {
        guideHairsLOD.Get(i).isActive = true;
        physicsControl->InitializeStrand(i + guideHairs.GetSize(), 
          guideHairsLOD.Get(i).controlPoints, guideHairsLOD.Get(i).controlPointsCount);
      }

    // print the number of control hairs
    int count = 0;
    for (size_t i = 0 ; i < guideHairsLOD.GetSize(); i ++)
      if (guideHairsLOD.Get(i).isActive)
        count++;
    csPrintf("%d\n",count);
  }

  void FurMaterial::SaveUVImage()
  {
    // density map
    CS::StructuredTextureFormat readbackFmt 
      (CS::TextureFormatStrings::ConvertStructured ("abgr8"));

    csRef<iDataBuffer> densitymapDB = densitymap->Readback(readbackFmt);
    int densitymapW, densitymapH;
    densitymap->GetOriginalDimensions(densitymapW, densitymapH);
    uint8* densitymapData = densitymapDB->GetUint8();

    //csPrintf("%s\n", densitymap->GetImageName());

    // from normal guide ropes
    for (size_t i = 0 ; i < guideHairs.GetSize() ; i ++)
    {
      csVector2 uv = guideHairs.Get(i).uv;

      densitymapData[ 4 * ((int)(uv.x * densitymapW) + 
        (int)(uv.y * densitymapH) * densitymapW ) + 1 ] = 255;
    }

    // from LOD guide ropes
    for (size_t i = 0 ; i < guideHairsLOD.GetSize() ; i ++)
    {
      csVector2 uv = guideHairsLOD.Get(i).uv;

      densitymapData[ 4 * ((int)(uv.x * densitymapW) + 
        (int)(uv.y * densitymapH) * densitymapW ) ] = 255;
    }

    csPrintf("%d\n", guideHairsLOD.GetSize() + guideHairs.GetSize());

    SaveImage(densitymapData, "/data/krystal/krystal_debug.png",
      densitymapW, densitymapH);
  }

  void FurMaterial::SaveImage(uint8* buf, const char* texname, 
    int width, int height)
  {
    csRef<iImageIO> imageio = csQueryRegistry<iImageIO> (object_reg);
    csRef<iVFS> VFS = csQueryRegistry<iVFS> (object_reg);

    if(!buf)
    {
      printf("Bad data buffer!\n");
      return;
    }

    csRef<iImage> image;
    image.AttachNew(new csImageMemory (width, height, buf,false,
      CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA));

    if(!image.IsValid())
    {
      printf("Error loading image\n");
      return;
    }

    csPrintf ("Saving %zu KB of data.\n", 
      csImageTools::ComputeDataSize (image)/1024);

    csRef<iDataBuffer> db = imageio->Save (image, "image/png", "progressive");
    if (db)
    {
      if (!VFS->WriteFile (texname, (const char*)db->GetData (), db->GetSize ()))
      {
        printf("Failed to write file '%s'!", texname);
        return;
      }
    }
    else
    {
      printf("Failed to save png image for basemap!");
      return;
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

  void FurMaterial::SetFurStrandGenerator( iFurStrandGenerator* hairStrandGenerator)
  {
    this->hairStrandGenerator = hairStrandGenerator;
  }

  iFurStrandGenerator* FurMaterial::GetFurStrandGenerator( )
  {
    return hairStrandGenerator;
  }

  void FurMaterial::SetMaterial ( iMaterial* material )
  {
    this->material = material;

    SetColor(csColor(1,1,0));
    SetDensitymap();
    SetHeightmap();
    SetStrandWidth();
    SetDisplaceEps();
  }

  void FurMaterial::SetColor(csColor color)
  {
    CS::ShaderVarName furColorName (svStrings, "mat furcolor");	
    csRef<csShaderVariable> shaderVariable = 
      material->GetVariable(furColorName);
    if(!shaderVariable)
    {
      shaderVariable = material->GetVariableAdd(furColorName);
      shaderVariable->SetValue(color);	
    }
  }

  void FurMaterial::SetStrandWidth()
  {
    CS::ShaderVarName strandWidthName (svStrings, "width");	
    csRef<csShaderVariable> shaderVariable = material->GetVariable(strandWidthName);

    shaderVariable->GetValue(strandWidth);
  }

  void FurMaterial::SetDensitymap ()
  {
    CS::ShaderVarName densitymapName (svStrings, "density map");	
    csRef<csShaderVariable> shaderVariable = material->GetVariable(densitymapName);

    shaderVariable->GetValue(densitymap);

    CS::ShaderVarName densityFactorName (svStrings, "densityFactor");	
    material->GetVariable(densityFactorName)->GetValue(densityFactor);
  }

  void FurMaterial::SetHeightmap ()
  {
    CS::ShaderVarName heightmapName (svStrings, "height map");	
    csRef<csShaderVariable> shaderVariable = material->GetVariable(heightmapName);

    shaderVariable->GetValue(heightmap);
    //printf("%s\n", heightmap->GetImageName());
  }

  void FurMaterial::SetDisplaceEps()
  {
    displaceEps = 0.02f;

    CS::ShaderVarName displaceEpsName (svStrings, "displaceEps");	
    csRef<csShaderVariable> shaderVariable = 
      material->GetVariable(displaceEpsName);

    if (shaderVariable)
      shaderVariable->GetValue(displaceEps);
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

  void FurAnimationControl::UpdateGuideHairs()
  {
    // update guide ropes
    for (size_t i = 0 ; i < furMaterial->guideHairs.GetSize(); i ++)
      furMaterial->physicsControl->AnimateStrand(i,
        furMaterial->guideHairs.Get(i).controlPoints,
        furMaterial->guideHairs.Get(i).controlPointsCount);

    // update guide ropes LOD
    for (size_t i = 0 ; i < furMaterial->guideHairsLOD.GetSize(); i ++)
      if ( furMaterial->guideHairsLOD.Get(i).isActive )
        furMaterial->physicsControl->AnimateStrand(i + furMaterial->guideHairs.GetSize(),
          furMaterial->guideHairsLOD.Get(i).controlPoints,
          furMaterial->guideHairsLOD.Get(i).controlPointsCount);        
      else
        UpdateControlPoints(furMaterial->guideHairsLOD.Get(i).controlPoints,
          furMaterial->guideHairsLOD.Get(i).controlPointsCount,
          furMaterial->guideHairsLOD.Get(i).guideHairs);      
  }

  void FurAnimationControl::UpdateControlPoints(csVector3 *controlPoints,
    size_t controlPointsCount, csGuideHairReference guideHairs[GUIDE_HAIRS_COUNT])
  {
    for ( size_t i = 0 ; i < controlPointsCount; i++ )
    {
      controlPoints[i] = csVector3(0);
      for ( size_t j = 0 ; j < GUIDE_HAIRS_COUNT ; j ++ )
        if ( guideHairs[j].index < furMaterial->guideHairs.GetSize() )
          controlPoints[i] += guideHairs[j].distance * 
            (furMaterial->guideHairs.Get(guideHairs[j].index).controlPoints[i]);
        else
          controlPoints[i] += guideHairs[j].distance * 
            (furMaterial->guideHairsLOD.Get(guideHairs[j].index - 
            furMaterial->guideHairs.GetSize()).controlPoints[i]);
    }
  }

  void FurAnimationControl::Update (csTicks current, int num_verts, uint32 version_id)
  {
    // update shader
    if (furMaterial->hairStrandGenerator)
      furMaterial->hairStrandGenerator->Update();

    // first update the control points
    if (furMaterial->physicsControl)
      UpdateGuideHairs();

    // then update the hair strands
    if (furMaterial->physicsControl)
      for (size_t i = 0 ; i < furMaterial->hairStrands.GetSize(); i ++)
        UpdateControlPoints(furMaterial->hairStrands.Get(i).controlPoints,
          furMaterial->hairStrands.Get(i).controlPointsCount,
          furMaterial->hairStrands.Get(i).guideHairs);

    const csOrthoTransform& tc = furMaterial->view -> GetCamera() ->GetTransform ();

    int numberOfStrains = furMaterial->hairStrands.GetSize();

    if (!numberOfStrains)
      return;

    int controlPoints = furMaterial->hairStrands.Get(0).controlPointsCount;

    csVector3 *vbuf = furMaterial->factoryState->GetVertices (); 
    iRenderBuffer *tangents = furMaterial->factoryState->GetRenderBuffer(CS_BUFFER_TANGENT);
    csRenderBufferLock<csVector3> tan (tangents, CS_BUF_LOCK_NORMAL);

    for ( int x = 0 ; x < numberOfStrains ; x ++ )
    {
      int y = 0;
      csVector3 tangent = csVector3(0);
      csVector3 strip = csVector3(0);

      for ( y = 0 ; y < controlPoints - 1; y ++ )
      {
        csVector3 firstPoint = furMaterial->hairStrands.Get(x).controlPoints[y];
        csVector3 secondPoint = furMaterial->hairStrands.Get(x).controlPoints[y + 1];
        
        csVector3 binormal;
        csMath3::CalcNormal(binormal, firstPoint, secondPoint, tc.GetOrigin());
        binormal.Normalize();
        strip = furMaterial->strandWidth * binormal;

        vbuf[ x * 2 * controlPoints + 2 * y].Set( firstPoint );
        vbuf[ x * 2 * controlPoints + 2 * y + 1].Set( firstPoint + strip );

        csVector3 normal;
        csMath3::CalcNormal(normal, firstPoint, firstPoint + strip, secondPoint);
        tangent.Cross(-binormal, normal);

        tan.Get(x * 2 * controlPoints + 2 * y).Set( tangent );
        tan.Get(x * 2 * controlPoints + 2 * y + 1).Set( tangent );
      }

      vbuf[ x * 2 * controlPoints + 2 * y].Set
        ( furMaterial->hairStrands.Get(x).controlPoints[y] );
      vbuf[ x * 2 * controlPoints + 2 * y + 1].Set
        ( furMaterial->hairStrands.Get(x).controlPoints[y] + strip );

      tan.Get(x * 2 * controlPoints + 2 * y).Set( tangent );
      tan.Get(x * 2 * controlPoints + 2 * y + 1).Set( tangent );
    }

//     furMaterial->factoryState->CalculateNormals();
//     furMaterial->factoryState->Invalidate();
// 
//     furMaterial->factoryState->AddRenderBuffer(CS_BUFFER_TANGENT, 
//       furMaterial->factoryState->GetRenderBuffer(CS_BUFFER_TANGENT));
//     furMaterial->factoryState->RemoveRenderBuffer(CS_BUFFER_TANGENT);

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

}
CS_PLUGIN_NAMESPACE_END(FurMaterial)

