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

#include "furmesh.h"
#include "hairphysicscontrol.h"
#include "hairstrandgenerator.h"

CS_PLUGIN_NAMESPACE_BEGIN(FurMesh)
{
  /********************
  *  FurMesh
  ********************/

  CS_LEAKGUARD_IMPLEMENT(FurMesh);

  FurMesh::FurMesh (iEngine* engine, iObjectRegistry* object_reg, 
    iMeshObjectFactory* object_factory) : scfImplementationType (this, engine), 
    materialWrapper(0), object_reg(object_reg), object_factory(object_factory), 
    engine(engine), physicsControl(0), hairStrandGenerator(0), positionShift(0),
    rng(0), guideLOD(0),strandLOD(0), hairStrandsLODSize(0), 
    physicsControlEnabled(true), 
    densityFactorGuideHairs(10), densityFactorHairStrands(100),
    heightFactor(0.5f), displaceDistance(0.02f), strandWidth(0.0015f), 
    controlPointsDistance(0.05f), positionDeviation(0.01f), growTangents(0), 
    mixmode(0), priority(7), z_buf_mode(CS_ZBUF_USE), indexstart(0), indexend(0)
  {
    svStrings = csQueryRegistryTagInterface<iShaderVarStringSet> (
      object_reg, "crystalspace.shader.variablenameset");

    if (!svStrings) 
      printf ("No SV names string set!");

    if (!engine) printf ("Failed to locate 3D engine!");

    rng = new csRandomGen(csGetTicks());

    factory = scfQueryInterface<CS::Mesh::iFurMeshFactory>(object_factory);
  }

  FurMesh::~FurMesh ()
  {
    delete rng;
    
    if (positionShift)
      delete positionShift;

    // delete hairs
    for (size_t i = 0 ; i < guideHairs.GetSize() ; i ++)
      guideHairs.Get(i).Clear();

    guideHairs.DeleteAll();

    for (size_t i = 0 ; i < guideHairsLOD.GetSize() ; i ++)
      guideHairsLOD.Get(i).Clear();

    guideHairsLOD.DeleteAll();

    for (size_t i = 0 ; i < hairStrands.GetSize() ; i ++)
      hairStrands.Get(i).Clear();

    hairStrands.DeleteAll();

    guideHairsTriangles.DeleteAll();
  }

  iMeshObjectFactory* FurMesh::GetFactory () const
  {
    return object_factory;
  }

  bool FurMesh::HitBeamObject (const csVector3& start, const csVector3& end,
    csVector3& isect, float* pr, int* polygon_idx,
    iMaterialWrapper** material, iMaterialArray* materials)
  {
    return csIntersect3::BoxSegment (GetObjectBoundingBox(), csSegment3 (start, end),
      isect, pr) != 0;
  }

  void FurMesh::NextFrame (csTicks current_time, const csVector3& pos,
    uint currentFrame)
  {
    Update();
    UpdateObjectBoundingBox();
  }

  bool FurMesh::SetMaterialWrapper (iMaterialWrapper* mat)
  {
    materialWrapper = mat;
    return true;
  }

  iMaterialWrapper* FurMesh::GetMaterialWrapper () const
  {
    return materialWrapper;
  }

  void FurMesh::UpdateObjectBoundingBox ()
  {
    if (!factory->GetVertexCount())
      return;

    boundingbox.StartBoundingBox();
    
    csVector3* vertex_buffer = (csVector3*)factory->GetVertices()->Lock (CS_BUF_LOCK_READ);

    for (size_t i = 0 ; i < factory->GetVertexCount(); i ++)
      boundingbox.AddBoundingVertex(vertex_buffer[i]);

    factory->GetVertices()->Release();
  }

  CS::Graphics::RenderMesh** FurMesh::GetRenderMeshes (int& num, 
    iRenderView* rview, iMovable* movable, uint32 frustum_mask)
  {
    for (size_t i = 0 ; i < renderMeshes.GetSize() ; i ++)
      delete renderMeshes.Get(i);

    renderMeshes.Empty();

    // Boiler-plate stuff...
    iCamera* camera = rview->GetCamera ();

    int clip_portal, clip_plane, clip_z_plane;
    CS::RenderViewClipper::CalculateClipSettings (rview->GetRenderContext (),
      frustum_mask, clip_portal, clip_plane, clip_z_plane);

    const csReversibleTransform o2wt = movable->GetFullTransform ();

    if (indexstart == indexend)
    {
      num = 0;
      return 0;
    }

    if (!materialWrapper)
    {
      csPrintf ("INTERNAL ERROR: mesh used without material!\n");
      num = 0;
      return 0;
    }

    if (materialWrapper->IsVisitRequired ()) 
      materialWrapper->Visit ();

    csRenderMesh* meshPtr = new csRenderMesh;

    // Setup the render mesh
    meshPtr->clip_portal = clip_portal;
    meshPtr->clip_plane = clip_plane;
    meshPtr->clip_z_plane = clip_z_plane;
    meshPtr->do_mirror = camera->IsMirrored ();
    meshPtr->meshtype = CS_MESHTYPE_TRIANGLES;
    meshPtr->indexstart = indexstart;
    meshPtr->indexend = indexend;
    meshPtr->material = materialWrapper;

    meshPtr->mixmode = mixmode; //  mixmode

    meshPtr->buffers = bufferholder;
    meshPtr->renderPrio = priority; // renderpriority
    meshPtr->z_buf_mode = z_buf_mode; // zbufMode;

    meshPtr->object2world = o2wt;
    meshPtr->bbox = GetObjectBoundingBox();
    meshPtr->geometryInstance = factory;
    meshPtr->variablecontext = svContext;

    renderMeshes.Push (meshPtr);    

    num = (int)renderMeshes.GetSize ();
    return renderMeshes.GetArray ();
  }

  void FurMesh::SetMixMode (uint mode)
  {
    mixmode = mode;
  }

  uint FurMesh::GetMixMode () const
  {
    return mixmode;
  }

  void FurMesh::SetPriority (uint priority)
  {
    this->priority = priority;
  }

  uint FurMesh::GetPriority () const
  {
    return priority;
  }

  void FurMesh::SetZBufMode(csZBufMode z_buf_mode)
  {
    this->z_buf_mode = z_buf_mode;
  }

  csZBufMode FurMesh::GetZBufMode() const
  {
    return z_buf_mode;
  }

  void FurMesh::SetIndexRange (uint indexstart, uint indexend)
  {
    this->indexstart = indexstart;
    this->indexend = indexend;
  }

  void FurMesh::GenerateGeometry (iView* view, iSector *room)
  {
    GenerateGuideHairs();
    GenerateGuideHairsLOD();
    SynchronizeGuideHairs();
    GenerateHairStrands();

    SaveUVImage();

    this->view = view;
    size_t numberOfStrains = hairStrands.GetSize();
    hairStrandsLODSize = hairStrands.GetSize();
    
    if( !numberOfStrains ) 
      return;

    size_t controlPointsCount = 0;
    
    for ( size_t i = 0 ; i < numberOfStrains ; i ++ )
      controlPointsCount += hairStrands.Get(i).controlPointsCount;

    // First create the factory:
    factory->SetVertexCount ( 2 * controlPointsCount );
    factory->SetTriangleCount( 2 * ( controlPointsCount - numberOfStrains ) );

    csVector3* vbuf = (csVector3*)factory->GetVertices()->Lock (CS_BUF_LOCK_NORMAL);
    csVector2* uv = (csVector2*)factory->GetTexCoords()->Lock (CS_BUF_LOCK_NORMAL);
    csTriangle* ibuf = (csTriangle*)factory->GetIndices()->Lock (CS_BUF_LOCK_NORMAL);

    for ( size_t x = 0, controlPointSum = 0 ; x < numberOfStrains ; 
      controlPointSum += hairStrands.Get(x).controlPointsCount, x++ )
    {
      csHairStrand hairStrand = hairStrands.Get(x);
      csVector2 strandUV = hairStrand.uv;

      for ( size_t y = 0 ; y < hairStrand.controlPointsCount ; y ++ )
      {
        vbuf[ 2 * controlPointSum + 2 * y] = hairStrand.controlPoints[y];
        vbuf[ 2 * controlPointSum + 2 * y + 1] = 
          hairStrand.controlPoints[y] + csVector3(-0.01f,0,0);
        uv[ 2 * controlPointSum + 2 * y] = strandUV;
        uv[ 2 * controlPointSum + 2 * y + 1] = strandUV;
      }

      for ( size_t y = 0 ; y < 2 * (hairStrand.controlPointsCount - 1) ; y ++ )
      {
        if (y % 2 == 0)
          ibuf[ 2 * (controlPointSum - x) + y ] = csTriangle( 2 * controlPointSum + y , 
            2 * controlPointSum + y + 1 , 2 * controlPointSum + y + 2 );
        else
          ibuf[ 2 * (controlPointSum - x) + y ] = csTriangle( 2 * controlPointSum + y , 
            2 * controlPointSum + y + 2 , 2 * controlPointSum + y + 1 );
      }
    }

    factory->GetVertices()->Release();
    factory->GetTexCoords()->Release();
    factory->GetIndices()->Release();

    // generate color deviation and UV 
    csVector3* bin = (csVector3*) factory->GetBinormals()->Lock(CS_BUF_LOCK_NORMAL);

    for ( size_t x = 0, controlPointSum = 0 ; x < numberOfStrains ; 
      controlPointSum += hairStrands.Get(x).controlPointsCount, x++ )
    {
      csHairStrand hairStrand = hairStrands.Get(x);

      float len = csVector3::Norm(
        hairStrand.controlPoints[ hairStrand.controlPointsCount - 1 ] - 
        hairStrand.controlPoints[ 0 ]);

      for ( size_t y = 0 ; y < hairStrand.controlPointsCount ; y ++ )
      {
        float sum = csVector3::Norm( hairStrand.controlPoints[ y ] - 
          hairStrand.controlPoints[ 0 ]);

        bin[ 2 * controlPointSum + 2 * y] = csVector3( rng->Get(), sum/len, rng->Get() );
        bin[ 2 * controlPointSum + 2 * y + 1] = bin[ 2 * controlPointSum + 2 * y];
      }
    }

    factory->GetBinormals()->Release();

    // geneate position deviation
    positionShift = new csVector3 [ controlPointsCount ];

    for ( size_t i = 0 ; i < controlPointsCount ; i ++)
      positionShift[i] = csVector3(rng->Get() * 2 - 1, rng->Get() * 2 - 1, 
        rng->Get() * 2 - 1) * positionDeviation;

    // Default material
    csRef<iMaterialWrapper> materialWrapper = 
      CS::Material::MaterialBuilder::CreateColorMaterial
      (object_reg,"hairDummyMaterial",csColor(1,0,0));

    if (hairStrandGenerator && hairStrandGenerator->GetMaterial())
      materialWrapper->SetMaterial(hairStrandGenerator->GetMaterial());

    GetMeshWrapper()->SetFlagsRecursive(CS_ENTITY_NOSHADOWS, CS_ENTITY_NOSHADOWS);
    SetMaterialWrapper(materialWrapper);

    bufferholder.AttachNew (new csRenderBufferHolder);
    bufferholder->SetRenderBuffer (CS_BUFFER_INDEX, factory->GetIndices());
    bufferholder->SetRenderBuffer (CS_BUFFER_POSITION, factory->GetVertices());
    bufferholder->SetRenderBuffer (CS_BUFFER_TEXCOORD0, factory->GetTexCoords());
    bufferholder->SetRenderBuffer (CS_BUFFER_NORMAL, factory->GetNormals());
    bufferholder->SetRenderBuffer (CS_BUFFER_BINORMAL, factory->GetBinormals());
    bufferholder->SetRenderBuffer (CS_BUFFER_TANGENT, factory->GetTangents());

    svContext.AttachNew (new csShaderVariableContext);
    csShaderVariable* sv;

    // Get the SV names
    sv = svContext->GetVariableAdd (svStrings->Request ("position"));
    sv->SetValue (factory->GetVertices());

    sv = svContext->GetVariableAdd (svStrings->Request ("normal"));
    sv->SetValue (factory->GetNormals());

    sv = svContext->GetVariableAdd (svStrings->Request ("texture coordinate 0"));
    sv->SetValue (factory->GetTexCoords());

    sv = svContext->GetVariableAdd (svStrings->Request ("tangent"));
    sv->SetValue (factory->GetTangents());

    sv = svContext->GetVariableAdd (svStrings->Request ("binormal"));
    sv->SetValue (factory->GetBinormals());

    SetIndexRange(0, (uint)factory->GetIndices()->GetElementCount());
  }

  void FurMesh::GenerateGuideHairs()
  {
    size_t indexstart = meshFactorySubMesh->GetIndices(0)->GetRangeStart();
    size_t indexend = meshFactorySubMesh->GetIndices(0)->GetRangeEnd();

    size_t indexCount = 
      meshFactorySubMesh->GetIndices(0)->GetSize() / sizeof(csTriangle);

    csVector3* vertex_buffer = 
      (csVector3*)meshFactory->GetVertices()->Lock(CS_BUF_LOCK_READ);
    csVector3* normal_buffer = 
      (csVector3*)meshFactory->GetNormals()->Lock(CS_BUF_LOCK_READ);
    csTriangle* index_buffer = 
      (csTriangle*)meshFactorySubMesh->GetIndices(0)->Lock(CS_BUF_LOCK_READ);
    csVector2* texcoord_buffer = 
      (csVector2*)meshFactory->GetTexCoords()->Lock(CS_BUF_LOCK_READ);
    csVector3* tangent_buffer = 
      (csVector3*)meshFactory->GetTangents()->Lock(CS_BUF_LOCK_READ);

    // choose unique indices
    for ( size_t i = 0 ; i < indexCount ; i ++)
    {
      csTriangle tri = index_buffer[i];
      csTriangle triangleNew = csTriangle(tri.a - indexstart, 
        tri.b - indexstart, tri.c - indexstart);
      guideHairsTriangles.Push(triangleNew);
    }

    // generate the guide hairs
    for (size_t i = indexstart; i < indexend; i ++)
    {
      csVector3 pos = vertex_buffer[i] + 
        displaceDistance * normal_buffer[i];

      csGuideHair guideHair;
      guideHair.uv = texcoord_buffer[i];

      // based on heightmap
      // point heightmap - modify to use convolution matrix or such
      float height = heightmap.Get((int)(guideHair.uv.x * heightmap.width),  
        (int)(guideHair.uv.y * heightmap.height), 0) / 255.0f;

      size_t controlPointsCount = (int)( (height * heightFactor) / controlPointsDistance);
      
      if (controlPointsCount == 1)
        controlPointsCount++;

      float realDistance = (height * heightFactor) / controlPointsCount;

      if (growTangents)
        guideHair.Generate(controlPointsCount, realDistance, pos, tangent_buffer[i]);
      else
        guideHair.Generate(controlPointsCount, realDistance, pos, normal_buffer[i]);
        
      guideHairs.Push(guideHair);
    }

    meshFactory->GetVertices()->Release();
    meshFactory->GetNormals()->Release();
    meshFactorySubMesh->GetIndices(0)->Release();
    meshFactory->GetTexCoords()->Release();
    meshFactory->GetTangents()->Release();
  }

  void FurMesh::TriangleAreaDensity(const csTriangle& triangle, float &area, float &density,
    csGuideHair& A, csGuideHair& B, csGuideHair& C)
  {
    size_t indexA = triangle.a;
    size_t indexB = triangle.b;
    size_t indexC = triangle.c;

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

    // triangle area
    if ( ( A.controlPointsCount == 0 ) || ( B.controlPointsCount == 0 ) ||
      ( C.controlPointsCount == 0 ) )
    {
      area = density = 0;
      return;
    }

    float bA = csVector3::Norm(B.controlPoints[0] - C.controlPoints[0]);
    float bB = csVector3::Norm(A.controlPoints[0] - C.controlPoints[0]);
    float bC = csVector3::Norm(B.controlPoints[0] - A.controlPoints[0]);    

    float s = (bA + bB + bC) / 2.0f;
    area = sqrt(s * (s - bA) * (s - bB) * (s - bC));

    float area2d;

    csVector2 a = csVector2(A.uv.x * densitymap.width, A.uv.y * densitymap.height);
    csVector2 b = csVector2(B.uv.x * densitymap.width, B.uv.y * densitymap.height);
    csVector2 c = csVector2(C.uv.x * densitymap.width, C.uv.y * densitymap.height);

    float baseA = csVector2::Norm(b - c);
    float baseB = csVector2::Norm(a - c);
    float baseC = csVector2::Norm(b - a);

    s = (baseA + baseB + baseC) / 2.0f;
    area2d = sqrt(s * (s - baseA) * (s - baseB) * (s - baseC));

    if (area2d < EPSILON)
    {
      area = density = 0;
      return;
    }

    float hA = (2 * area2d) / baseA;
    float hB = (2 * area2d) / baseB;
    float hC = (2 * area2d) / baseC;

    density = 0;
    int count = 0;

    for( float bA = 0.0; bA <= 1.0; bA += 1 / hA )
      for (float bB = 0.0; bB <= 1.0 - bA; bB += 1 / baseA)
      {
        count++;
        float bC = 1 - bA - bB;
        csVector2 newPoint = a * bA + b * bB + c * bC;
        density += densitymap.Get((int)newPoint.x, (int)newPoint.y, 0);
      }

    for( float bB = 0.0; bB <= 1.0; bB += 1 / hB )
      for (float bA = 0.0; bA <= 1.0 - bB; bA += 1 / baseB)
      {
        count++;
        float bC = 1 - bA - bB;
        csVector2 newPoint = a * bA + b * bB + c * bC;
        density += densitymap.Get((int)newPoint.x, (int)newPoint.y, 0);
      }

    for( float bC = 0.0; bC <= 1.0; bC += 1 / hC )
      for (float bA = 0.0; bA <= 1.0 - bC; bA += 1 / baseC)
      {
        count++;
        float bB = 1 - bA - bC;
        csVector2 newPoint = a * bA + b * bB + c * bC;
        density += densitymap.Get((int)newPoint.x, (int)newPoint.y, 0);
      }

    if (count != 0)
      density /= count;
  }

  void FurMesh::GenerateGuideHairsLOD()
  {
    float density, area;
    csGuideHair A, B, C;

    // generate guide hairs for LOD
    for (size_t iter = 0 ; iter < guideHairsTriangles.GetSize(); iter ++)
    {
      csTriangle triangle = guideHairsTriangles.Get(iter);
      TriangleAreaDensity(triangle, area, density, A, B, C);

      // if a new guide hair is needed
      if ( (density * area * densityFactorGuideHairs) < 1)
        continue;

      // make new guide hair
      csGuideHairLOD guideHairLOD;
      guideHairLOD.SetGuideHairsRefs(triangle, rng);

      guideHairLOD.isActive = false;

      guideHairLOD.SetUV(guideHairs, guideHairsLOD);

      // generate control points
      size_t controlPointsCount = csMin( (csMin( A.controlPointsCount,
        B.controlPointsCount) ), C.controlPointsCount);

      guideHairLOD.Generate(controlPointsCount, guideHairs, guideHairsLOD);
      
      // add new triangles
      guideHairsLOD.Push(guideHairLOD);
      size_t indexD = guideHairsLOD.GetSize() - 1 + guideHairs.GetSize();
      csTriangle ADC = csTriangle(triangle.a, indexD, triangle.c);
      csTriangle ADB = csTriangle(triangle.a, indexD, triangle.b);
      csTriangle BDC = csTriangle(triangle.b, indexD, triangle.c);

      guideHairsTriangles.Push(ADC);
      guideHairsTriangles.Push(ADB);
      guideHairsTriangles.Push(BDC);
    }    
  }

  void FurMesh::SynchronizeGuideHairs ()
  {
    if (!physicsControlEnabled) // no physics support
      return;

    for (size_t i = 0 ; i < guideHairs.GetSize(); i ++)
      physicsControl->InitializeStrand(i,guideHairs.Get(i).controlPoints, 
        guideHairs.Get(i).controlPointsCount);
  }

  void FurMesh::GenerateHairStrands ()
  {
    float area, density;
    csGuideHair A, B, C;

    // density
    for (int den = 0 , change = 1 ; change && den < 100 ; den ++)
    {
      change = 0;
      // for every triangle
      for (size_t iter = 0 ; iter < guideHairsTriangles.GetSize(); iter ++)
      {
        csTriangle triangle = guideHairsTriangles.Get(iter);
        TriangleAreaDensity(triangle, area, density, A, B, C);

        // how many new guide hairs are needed
        if ( den < (density * area * densityFactorHairStrands))
        {
          change = 1;
          csHairStrand hairStrand;
          hairStrand.SetGuideHairsRefs(triangle, rng);

          size_t controlPointsCount = csMin( (csMin( A.controlPointsCount,
            B.controlPointsCount) ), C.controlPointsCount);

          hairStrand.Generate(controlPointsCount, guideHairs, guideHairsLOD);

          hairStrand.SetUV(guideHairs, guideHairsLOD);

          hairStrands.Push(hairStrand);		
        }
      }
    }
  }

  void FurMesh::SetGuideLOD(float guideLOD)
  {
    if ( fabs( this->guideLOD - guideLOD ) < EPSILON )
      return;

    this->guideLOD = guideLOD;

    if (!physicsControlEnabled) // no physics support
      return;

    // deactivate all
    for (size_t i = 0 ; i < guideHairsLOD.GetSize(); i ++)
    {
      guideHairsLOD.Get(i).isActive = false;
      physicsControl->RemoveStrand(i + guideHairs.GetSize());
    }
    // LOD ropes use ropes as well
    for (size_t i = 0 ; i < guideHairsLOD.GetSize(); i ++)
      if ( rng->Get() < guideLOD )
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
    csPrintf("Active LOD ropes: %d\n",count);
  }

  void FurMesh::SetStrandLOD(float strandLOD)
  {
    this->strandLOD = strandLOD;
    size_t totalGuideHairsCount = guideHairs.GetSize() + guideHairsLOD.GetSize();
    hairStrandsLODSize = totalGuideHairsCount + 
      (size_t)(strandLOD * (hairStrands.GetSize() - totalGuideHairsCount));

    strandWidthLOD = 1 / ( strandLOD * 0.75f + 0.25f ) * strandWidth;
  }

  void FurMesh::SetLOD(float lod)
  {
    SetGuideLOD(lod);
    SetStrandLOD(lod);
  }

  void FurMesh::SaveUVImage()
  {
    // from normal guide ropes
    for (size_t i = 0 ; i < guideHairs.GetSize() ; i ++)
    {
      csVector2 uv = guideHairs.Get(i).uv;

      densitymap.Set( (int)(uv.x * densitymap.width), 
        (int)(uv.y * densitymap.height), 1, 255);

      heightmap.Set( (int)(uv.x * heightmap.width),  
        (int)(uv.y * heightmap.height), 1, 255);

    }

    // from LOD guide ropes
    for (size_t i = 0 ; i < guideHairsLOD.GetSize() ; i ++)
    {
      csVector2 uv = guideHairsLOD.Get(i).uv;

      densitymap.Set( (int)(uv.x * densitymap.width),
        (int)(uv.y * densitymap.height), 0, 255);
    }

    // from hair strands
    for (size_t i = 0 ; i < hairStrands.GetSize() ; i ++)
    {
      csVector2 uv = hairStrands.Get(i).uv;

      densitymap.Set( (int)(uv.x * densitymap.width),
        (int)(uv.y * densitymap.height), 2, 255);
    }

    csPrintf("Pure guide ropes: %d\n", guideHairs.GetSize());
    csPrintf("Total guide ropes: %d\n", guideHairsLOD.GetSize() + guideHairs.GetSize());

    densitymap.SaveImage(object_reg, "/data/krystal/krystal_skull_densitymap_debug.png");
    heightmap.SaveImage(object_reg, "/data/krystal/krystal_skull_heightmap_debug.png");
  }

  void FurMesh::SetPhysicsControl (CS::Mesh::iFurPhysicsControl* physicsControl)
  {
    this->physicsControl = physicsControl;
  }

  void FurMesh::StartPhysicsControl()
  {
    if (!physicsControlEnabled)
    {
      physicsControlEnabled = true;
      SynchronizeGuideHairs();
    }
  }

  void FurMesh::StopPhysicsControl()
  {
    if (physicsControlEnabled)
    {
      physicsControlEnabled = false;
      physicsControl->RemoveAllStrands();
    }
  }

  void FurMesh::SetMeshFactory ( CS::Mesh::iAnimatedMeshFactory* meshFactory)
  {
    this->meshFactory = meshFactory;
  }

  void FurMesh::SetMeshFactorySubMesh ( CS::Mesh::iAnimatedMeshSubMeshFactory* 
    meshFactorySubMesh )
  { 
    this->meshFactorySubMesh = meshFactorySubMesh;
  }

  void FurMesh::SetFurStrandGenerator( CS::Mesh::iFurStrandGenerator* hairStrandGenerator)
  {
    this->hairStrandGenerator = hairStrandGenerator;
  }

  CS::Mesh::iFurStrandGenerator* FurMesh::GetFurStrandGenerator( ) const
  {
    return hairStrandGenerator;
  }

  void FurMesh::SetBaseMaterial ( iMaterial* baseMaterial )
  {
    this->baseMaterial = baseMaterial;

    SetBaseMaterialProperties();
  }

  void FurMesh::SetBaseMaterialProperties()
  {
    // set grow tangents
    CS::ShaderVarName growTangentsName (svStrings, "growTangents");	
    baseMaterial->GetVariableAdd(growTangentsName)->GetValue(growTangents);

    // set position deviation
    CS::ShaderVarName positionDeviationName (svStrings, "positionDeviation");	
    baseMaterial->GetVariableAdd(positionDeviationName)->GetValue(positionDeviation);

    // set strand width
    CS::ShaderVarName strandWidthName (svStrings, "strandWidth");	
    baseMaterial->GetVariableAdd(strandWidthName)->GetValue(strandWidth);
    strandWidthLOD = strandWidth;    

    // set denstiy map
    iTextureHandle* tex;
    csRef<csShaderVariable> shaderVariable;
    
    CS::ShaderVarName densitymapName (svStrings, "density map");	
    shaderVariable = baseMaterial->GetVariable(densitymapName);
      
    if (!shaderVariable)  
    {
      csPrintfErr ("Density map not specified!\n");
      return;
    }

    shaderVariable->GetValue(tex);
    densitymap.handle = tex;

    CS::ShaderVarName densityFactorGuideHairsName (svStrings, "densityFactorGuideHairs");	
    baseMaterial->GetVariableAdd(densityFactorGuideHairsName)->GetValue(densityFactorGuideHairs);

    CS::ShaderVarName densityFactorHairStrandsName (svStrings, "densityFactorHairStrands");	
    baseMaterial->GetVariableAdd(densityFactorHairStrandsName)->GetValue(densityFactorHairStrands);    

    // density map
    if ( !densitymap.Read() )
      csPrintfErr( "Error reading densitymap texture!\n" );

    // set height map
    CS::ShaderVarName heightmapName (svStrings, "height map");	
    shaderVariable = baseMaterial->GetVariable(heightmapName);
      
    if (!shaderVariable)  
    {
      csPrintfErr ("Height map not specified!\n");
      return;
    }      

    shaderVariable->GetValue(tex);
    heightmap.handle = tex;

    CS::ShaderVarName heightFactorName (svStrings, "heightFactor");	
    baseMaterial->GetVariableAdd(heightFactorName)->GetValue(heightFactor);

    // height map
    if ( !heightmap.Read() )
      csPrintfErr( "Error reading heightmap texture!\n" );

    // set displace distance
    CS::ShaderVarName displaceDistanceName (svStrings, "displaceDistance");	
    baseMaterial->GetVariableAdd(displaceDistanceName)->GetValue(displaceDistance);

    CS::ShaderVarName controlPointsDistanceName (svStrings, "controlPointsDistance");	
    baseMaterial->GetVariableAdd(controlPointsDistanceName)->GetValue(controlPointsDistance);
  }

  void FurMesh::UpdateGuideHairs()
  {
    // update guide ropes
    for (size_t i = 0 ; i < guideHairs.GetSize(); i ++)
      physicsControl->AnimateStrand(i, guideHairs.Get(i).controlPoints,
        guideHairs.Get(i).controlPointsCount);

    // update guide ropes LOD
    for (size_t i = 0 ; i < guideHairsLOD.GetSize(); i ++)
      if ( guideHairsLOD.Get(i).isActive )
        physicsControl->AnimateStrand(i + guideHairs.GetSize(),
          guideHairsLOD.Get(i).controlPoints, guideHairsLOD.Get(i).controlPointsCount);        
      else
        guideHairsLOD.Get(i).Update(guideHairs, guideHairsLOD);
  }

  void FurMesh::Update()
  {
    // update shader
    if (hairStrandGenerator)
      hairStrandGenerator->Update();

    // first update the control points
    if (physicsControlEnabled)
      UpdateGuideHairs();

    size_t numberOfStrains = hairStrandsLODSize;

    if (!numberOfStrains)
      return;

    // then update the hair strands
    if (physicsControlEnabled)
      for (size_t i = 0 ; i < numberOfStrains; i ++)
        hairStrands.Get(i).Update(guideHairs, guideHairsLOD);

    const csOrthoTransform& tc = view -> GetCamera() ->GetTransform ();

    csVector3* vbuf = (csVector3*)factory->GetVertices()->Lock(CS_BUF_LOCK_NORMAL);
    csVector3* normals = (csVector3*)factory->GetNormals()->Lock(CS_BUF_LOCK_NORMAL); 
    csVector3* tangents = (csVector3*)factory->GetTangents()->Lock(CS_BUF_LOCK_NORMAL);
    csVector3* binormals = (csVector3*)factory->GetBinormals()->Lock(CS_BUF_LOCK_READ);

    csVector3 normal, tangent, binormal, cameraOrigin;
    csVector3 strip, firstPoint, secondPoint;

    cameraOrigin = tc.GetOrigin();
    csVector3 *posShift = positionShift;

    size_t triangleCount = 0;

    for ( size_t x = 0 ; x < numberOfStrains ; x ++)
    {
      int y = 0;
      tangent = csVector3(0);
      strip = csVector3(0);

      csVector3 *controlPoints = hairStrands.Get(x).controlPoints;
      int controlPointsCount = hairStrands.Get(x).controlPointsCount;
      triangleCount += 2 * controlPointsCount - 2;

      for ( y = 0 ; y < controlPointsCount - 1; y ++, controlPoints ++, 
        vbuf += 2, tangents += 2, binormals += 2, posShift ++, normals += 2 )
      {
        firstPoint = *controlPoints + (*posShift);
        secondPoint = *(controlPoints + 1) + (*posShift);

        csMath3::CalcNormal(binormal, firstPoint, secondPoint, cameraOrigin);
        binormal.Normalize();
        strip = strandWidthLOD * binormal * ((*binormals).z + 1.0f) * 
          (0.5f * (1.0f - (*binormals).y) + 0.75f );

        (*vbuf) = firstPoint;
        (*(vbuf + 1)) = firstPoint + strip;

        tangent = firstPoint - secondPoint;
        tangent.Normalize();
        normal.Cross(tangent, binormal);

        (*normals) = normal;
        (*(normals + 1)) = normal;

        (*tangents) = tangent;
        (*(tangents + 1)) = tangent;
      }

      if (controlPointsCount)
      {
        (*vbuf) = *controlPoints + (*posShift);
        (*(vbuf + 1)) = *controlPoints + strip + (*posShift);

        (*tangents) = tangent;
        (*(tangents + 1)) = tangent;

        (*normals) = normal;
        (*(normals + 1)) = normal;

        vbuf += 2;
        tangents += 2;
        binormals += 2;
        normals += 2;
        posShift ++;
      }
    }

    factory->GetVertices()->Release();
    factory->GetNormals()->Release();
    factory->GetTangents()->Release();
    factory->GetBinormals()->Release();

    SetIndexRange(0, 3 * triangleCount);
  }
}
CS_PLUGIN_NAMESPACE_END(FurMesh)
