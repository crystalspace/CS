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

#include "furmesh.h"
#include "hairphysicscontrol.h"
#include "hairstrandgenerator.h"


CS_PLUGIN_NAMESPACE_BEGIN(FurMesh)
{
  /********************
  *  FurMeshFactory
  ********************/

  CS_LEAKGUARD_IMPLEMENT(FurMeshFactory);

  FurMeshFactory::FurMeshFactory (iEngine *e, iObjectRegistry* reg, iMeshObjectType* type)
    : scfImplementationType(this, e, reg, type), indexCount(0), vertexCount(0), 
    indexBuffer(0), vertexBuffer(0), texcoordBuffer(0), tangentBuffer(0), binormalBuffer(0)
  {
  }

  FurMeshFactory::~FurMeshFactory ()
  {
  }

  csPtr<iMeshObject> FurMeshFactory::NewInstance ()
  {
    csRef<iMeshObject> ref;
    ref.AttachNew (new FurMesh(Engine, object_reg, this));
    return csPtr<iMeshObject> (ref);
  }

  /// geometry access
  void FurMeshFactory::SetVertexCount (uint n)
  {
    vertexCount = n;

    vertexBuffer = csRenderBuffer::CreateRenderBuffer (n, 
      CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);

    texcoordBuffer = csRenderBuffer::CreateRenderBuffer (n, 
      CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 2);

    normalBuffer = csRenderBuffer::CreateRenderBuffer (n, 
      CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);

    binormalBuffer = csRenderBuffer::CreateRenderBuffer (n, 
      CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);

    tangentBuffer = csRenderBuffer::CreateRenderBuffer (n, 
      CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);
  }

  void FurMeshFactory::SetTriangleCount (uint n)
  {
    indexCount = n;

    indexBuffer = csRenderBuffer::CreateIndexRenderBuffer (3 * n, 
      CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_INT, 0, vertexCount - 1);
  }

  uint FurMeshFactory::GetVertexCount()
  {
    return vertexCount;
  }

  uint FurMeshFactory::GetIndexCount()
  {
    return indexCount;
  }

  iRenderBuffer* FurMeshFactory::GetIndices ()
  {
    return indexBuffer;
  }

  bool FurMeshFactory::SetIndices (iRenderBuffer* renderBuffer)
  {
    indexBuffer = renderBuffer;
    indexCount = (uint)indexBuffer->GetElementCount ();
    return true;
  }

  iRenderBuffer* FurMeshFactory::GetVertices ()
  {
    return vertexBuffer;
  }

  bool FurMeshFactory::SetVertices (iRenderBuffer* renderBuffer)
  {
    if (renderBuffer->GetComponentCount () < 3)
      return false;

    vertexBuffer = renderBuffer;
    vertexCount = (uint)vertexBuffer->GetElementCount ();

    return true;
  }

  iRenderBuffer* FurMeshFactory::GetTexCoords ()
  {
    return texcoordBuffer;
  }

  bool FurMeshFactory::SetTexCoords (iRenderBuffer* renderBuffer)
  {
    if (renderBuffer->GetElementCount () < vertexCount)
      return false;

    texcoordBuffer = renderBuffer;    
    return true;
  }

  iRenderBuffer* FurMeshFactory::GetNormals ()
  {
    return normalBuffer;
  }

  bool FurMeshFactory::SetNormals (iRenderBuffer* renderBuffer)
  {
    if (renderBuffer->GetElementCount () < vertexCount)
      return false;

    normalBuffer = renderBuffer;    
    return true;
  }

  iRenderBuffer* FurMeshFactory::GetTangents ()
  {
    return tangentBuffer;
  }

  bool FurMeshFactory::SetTangents (iRenderBuffer* renderBuffer)
  {
    if (renderBuffer->GetElementCount () < vertexCount)
      return false;

    tangentBuffer = renderBuffer;    
    return true;
  }

  iRenderBuffer* FurMeshFactory::GetBinormals ()
  {
    return binormalBuffer;
  }

  bool FurMeshFactory::SetBinormals (iRenderBuffer* renderBuffer)
  { 
    if (renderBuffer->GetElementCount () < vertexCount)
      return false;

    binormalBuffer = renderBuffer;    
    return true;
  }

  /********************
  *  FurMeshType
  ********************/

  SCF_IMPLEMENT_FACTORY (FurMeshType)

    CS_LEAKGUARD_IMPLEMENT(FurMeshType);

  FurMeshType::FurMeshType (iBase* parent) :
  scfImplementationType (this, parent),
    object_reg(0)
  {
  }

  FurMeshType::~FurMeshType ()
  {
  }

  // From iComponent
  bool FurMeshType::Initialize (iObjectRegistry* r)
  {
    csRef<iEngine> e = csQueryRegistry<iEngine> (r);
    Engine = e;
    object_reg = r;
    return true;
  }

  csPtr<iMeshObjectFactory> FurMeshType::NewFactory ()
  {
    csRef<iMeshObjectFactory> ref;
    ref.AttachNew (new FurMeshFactory(Engine, object_reg, this));
    return csPtr<iMeshObjectFactory> (ref);
  }

  /********************
  *  FurMesh
  ********************/

  CS_LEAKGUARD_IMPLEMENT(FurMesh);

  FurMesh::FurMesh (iEngine* engine, iObjectRegistry* object_reg, 
    iMeshObjectFactory* object_factory) : scfImplementationType (this, engine), 
    materialWrapper(0), object_reg(object_reg), object_factory(object_factory), 
    engine(engine), physicsControl(0), hairStrandGenerator(0), positionShift(0),
    rng(0), guideLOD(0),strandLOD(0), hairStrandsLODSize(0), 
    physicsControlEnabled(true), growTangents(0), mixmode(0), priority(7),
    z_buf_mode(CS_ZBUF_USE), indexstart(0), indexend(0)
  {
    svStrings = csQueryRegistryTagInterface<iShaderVarStringSet> (
      object_reg, "crystalspace.shader.variablenameset");

    if (!svStrings) 
      printf ("No SV names string set!");

    if (!engine) printf ("Failed to locate 3D engine!");

    loader = csQueryRegistry<iLoader> (object_reg);
    if (!loader) printf ("Failed to locate Loader!");

    rng = new csRandomGen(csGetTicks());

    factory = scfQueryInterface<iFurMeshFactory>(object_factory);
  }

  FurMesh::~FurMesh ()
  {
    delete rng;
    if (positionShift)
      delete positionShift;
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
    boundingbox.StartBoundingBox();
    
    csVector3* vertex_buffer = (csVector3*)factory->GetVertices()->Lock (CS_BUF_LOCK_READ);

    for (size_t i = 0 ; i < factory->GetVertexCount(); i ++)
      boundingbox.AddBoundingVertex(vertex_buffer[i]);

    factory->GetVertices()->Release();
  }

  CS::Graphics::RenderMesh** FurMesh::GetRenderMeshes (int& num, 
    iRenderView* rview, iMovable* movable, uint32 frustum_mask)
  {
    renderMeshes.Empty();

    // Boiler-plate stuff...
    iCamera* camera = rview->GetCamera ();

    int clip_portal, clip_plane, clip_z_plane;
    CS::RenderViewClipper::CalculateClipSettings (rview->GetRenderContext (),
      frustum_mask, clip_portal, clip_plane, clip_z_plane);

    const csReversibleTransform o2wt = movable->GetFullTransform ();

    if (!materialWrapper)
    {
      csPrintf ("INTERNAL ERROR: mesh used without material!\n");
      num = 0;
      return 0;
    }

    if (materialWrapper->IsVisitRequired ()) 
      materialWrapper->Visit ();

    csRenderMesh *meshPtr = new csRenderMesh;

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
    iRenderBuffer* vertexes = meshFactory->GetVertices();
    iRenderBuffer* normals = meshFactory->GetNormals();
    iRenderBuffer* indices = meshFactorySubMesh->GetIndices(0);
    iRenderBuffer* texCoords = meshFactory->GetTexCoords();

    GenerateGuideHairs(indices, vertexes, normals, texCoords);
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
      csVector2 strandUV(0);
      csHairStrand hairStrand = hairStrands.Get(x);

      for ( size_t j = 0 ; j < GUIDE_HAIRS_COUNT ; j ++ )
        if (hairStrand.guideHairs[j].index < guideHairs.GetSize() )
          strandUV += hairStrand.guideHairs[j].distance * 
            guideHairs.Get(hairStrand.guideHairs[j].index).uv;
        else
          strandUV += hairStrand.guideHairs[j].distance * 
            guideHairsLOD.Get(hairStrand.guideHairs[j].index - 
              guideHairs.GetSize()).uv;

      for ( size_t y = 0 ; y < hairStrands.Get(x).controlPointsCount ; y ++ )
      {
        vbuf[ 2 * controlPointSum + 2 * y].Set
          ( hairStrands.Get(x).controlPoints[y] );
        vbuf[ 2 * controlPointSum + 2 * y + 1].Set
          ( hairStrands.Get(x).controlPoints[y] + csVector3(-0.01f,0,0) );
        uv[ 2 * controlPointSum + 2 * y].Set( strandUV );
        uv[ 2 * controlPointSum + 2 * y + 1].Set( strandUV );
      }

      for ( size_t y = 0 ; y < 2 * (hairStrands.Get(x).controlPointsCount - 1) ; y ++ )
      {
        if (y % 2 == 0)
        {
          ibuf[ 2 * (controlPointSum - x) + y ].Set
            ( 2 * controlPointSum + y , 
            2 * controlPointSum + y + 1 , 
            2 * controlPointSum + y + 2 );
          //printf("%d %d %d\n", 2 * x + y , 2 * x + y + 3 , 2 * x + y + 1);
        }
        else
        {
          ibuf[ 2 * (controlPointSum - x) + y ].Set
            ( 2 * controlPointSum + y , 
            2 * controlPointSum + y + 2 , 
            2 * controlPointSum + y + 1 );
          //printf("%d %d %d\n", 2 * x + y + 1 , 2 * x + y + 2 , 2 * x + y - 1);
        }
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
      float len = csVector3::Norm(
        hairStrands.Get(x).controlPoints[ hairStrands.Get(x).controlPointsCount - 1 ] - 
        hairStrands.Get(x).controlPoints[ 0 ]);

      for ( size_t y = 0 ; y < hairStrands.Get(x).controlPointsCount ; y ++ )
      {
        float sum = csVector3::Norm(
          hairStrands.Get(x).controlPoints[ y ] - 
          hairStrands.Get(x).controlPoints[ 0 ]);

        bin[ 2 * controlPointSum + 2 * y].Set( csVector3( rng->Get(), sum/len, rng->Get() ) );
        bin[ 2 * controlPointSum + 2 * y + 1].Set( bin[ 2 * controlPointSum + 2 * y] );
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

  void FurMesh::GenerateGuideHairs(iRenderBuffer* indices, 
    iRenderBuffer* vertexes, iRenderBuffer* nrmls, iRenderBuffer* texCoords)
  {
    csRenderBufferLock<csVector3> positions (vertexes, CS_BUF_LOCK_READ);
    csRenderBufferLock<csVector2> UV (texCoords, CS_BUF_LOCK_READ);
    csRenderBufferLock<csVector3> norms (nrmls, CS_BUF_LOCK_READ);
    CS::TriangleIndicesStream<size_t> tris (indices, CS_MESHTYPE_TRIANGLES);    
    csArray<int> uniqueIndices;

    csVector3 *tangentsArray = new csVector3[positions.GetSize()];
    csVector3 *binormalArray = new csVector3[positions.GetSize()];
    
    csTriangle *triangles = new csTriangle[indices->GetSize() / sizeof(csTriangle)];
    csVector3 *vertices = new csVector3[positions.GetSize()];
    csVector3 *normals = new csVector3[positions.GetSize()];
    csVector2 *texcoords = new csVector2[positions.GetSize()];

    for ( size_t i = 0 ; i < positions.GetSize() ; i ++ ) 
    {
      tangentsArray[i] = csVector3(0);
      binormalArray[i] = csVector3(0);
      vertices[i] = csVector3( positions.Get(i) ); 
      normals[i] = csVector3( norms.Get(i) );
      texcoords[i] = csVector2( UV.Get(i) ); 
    }

    // choose unique indices
    for ( size_t i = 0 ; tris.HasNext() ; i ++)
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
  
      triangles[i] = csTriangle(tri.a, tri.b, tri.c);
    }

    csNormalMappingTools::CalculateTangents(indices->GetSize() / sizeof(csTriangle), triangles, 
      positions.GetSize(), vertices, normals, texcoords, tangentsArray, binormalArray);

    // generate the guide hairs
    for (size_t i = 0; i < uniqueIndices.GetSize(); i ++)
    {
      csVector3 pos = positions.Get(uniqueIndices.Get(i)) + 
        displaceDistance * norms.Get(uniqueIndices.Get(i));

      csGuideHair guideHair;
      guideHair.uv = UV.Get(uniqueIndices.Get(i));

      // based on heightmap
      // point heightmap - modify to use convolution matrix or such
      float height = heightmap.data[ 4 * ((int)(guideHair.uv.x * heightmap.width) + 
        (int)(guideHair.uv.y * heightmap.height) * heightmap.width )] / 255.0f;

      guideHair.controlPointsCount = (int)( (height * heightFactor) / controlPointsDistance);
      
      if (guideHair.controlPointsCount == 1)
        guideHair.controlPointsCount = 2;

      guideHair.controlPoints = new csVector3[ guideHair.controlPointsCount ];

      float realDistance = (height * heightFactor) / guideHair.controlPointsCount;

      if (growTangents)
        for ( size_t j = 0 ; j < guideHair.controlPointsCount ; j ++ )
          guideHair.controlPoints[j] = pos + j * realDistance * 
            tangentsArray[uniqueIndices.Get(i)];
      else
        for ( size_t j = 0 ; j < guideHair.controlPointsCount ; j ++ )
          guideHair.controlPoints[j] = pos + j * realDistance * 
            norms.Get(uniqueIndices.Get(i));
        
      guideHairs.Push(guideHair);
    }
  }

  float FurMesh::TriangleDensity(csGuideHair A, csGuideHair B, csGuideHair C)
  {
    csVector2 a = csVector2(A.uv.x * densitymap.width, A.uv.y * densitymap.height);
    csVector2 b = csVector2(B.uv.x * densitymap.width, B.uv.y * densitymap.height);
    csVector2 c = csVector2(C.uv.x * densitymap.width, C.uv.y * densitymap.height);

    float baseA = csVector2::Norm(b - c);
    float baseB = csVector2::Norm(a - c);
    float baseC = csVector2::Norm(b - a);

    float s = (baseA + baseB + baseC) / 2.0f;
    float area = sqrt(s * (s - baseA) * (s - baseB) * (s - baseC));

    if (area < EPSILON)
      return 0;

    float hA = (2 * area) / baseA;
    float hB = (2 * area) / baseB;
    float hC = (2 * area) / baseC;

    float density = 0;
    int count = 0;

    for( float bA = 0.0; bA <= 1.0; bA += 1 / hA )
      for (float bB = 0.0; bB <= 1.0 - bA; bB += 1 / baseA)
      {
        count++;
        float bC = 1 - bA - bB;
        csVector2 newPoint = a * bA + b * bB + c * bC;
        density += densitymap.data[4 * ((int)newPoint.x + (int)newPoint.y * densitymap.width )];
      }

    for( float bB = 0.0; bB <= 1.0; bB += 1 / hB )
      for (float bA = 0.0; bA <= 1.0 - bB; bA += 1 / baseB)
      {
        count++;
        float bC = 1 - bA - bB;
        csVector2 newPoint = a * bA + b * bB + c * bC;
        density += densitymap.data[4 * ((int)newPoint.x + (int)newPoint.y * densitymap.width )];
      }

    for( float bC = 0.0; bC <= 1.0; bC += 1 / hC )
      for (float bA = 0.0; bA <= 1.0 - bC; bA += 1 / baseC)
      {
        count++;
        float bB = 1 - bA - bC;
        csVector2 newPoint = a * bA + b * bB + c * bC;
        density += densitymap.data[4 * ((int)newPoint.x + (int)newPoint.y * densitymap.width )];
      }

    if (count != 0)
      density /= count;

    // the old method based on average mean
//     density = (densitymap.data[ 4 * ((int)a.x + (int)a.y * densitymap.width )] + 
//       densitymap.data[ 4 * ((int)b.x + (int)b.y * densitymap.width )] + 
//       densitymap.data[ 4 * ((int)c.x + (int)c.y * densitymap.width )] ) / (3.0f);

    return density;
  }

  void FurMesh::GenerateGuideHairsLOD()
  {
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

      // average density - modify to use convolution matrix or such
      float density = TriangleDensity(A, B, C);

      //csPrintf("%f\t%f\t%f\t%f\n", density, area, density * area, densityFactorGuideHairs);

      // if a new guide hair is needed
      if ( (density * area * densityFactorGuideHairs) < 1)
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
    float bA, bB, bC; // barycentric coefficients

    // density
    for (int den = 0 , change = 1 ; change && den < 100 ; den ++)
    {
      change = 0;
      // for every triangle
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

        // triangle area
        if ( ( A.controlPointsCount == 0 ) || ( B.controlPointsCount == 0 ) ||
          ( C.controlPointsCount == 0 ) )
          continue;

        float a, b, c;
        a = csVector3::Norm(B.controlPoints[0] - C.controlPoints[0]);
        b = csVector3::Norm(A.controlPoints[0] - C.controlPoints[0]);
        c = csVector3::Norm(B.controlPoints[0] - A.controlPoints[0]);

        float s = (a + b + c) / 2.0f;
        float area = sqrt(s * (s - a) * (s - b) * (s - c));

        // average density - modify to use convolution matrix or such
        float density = TriangleDensity(A, B, C);

  //       csPrintf("%f\t%f\t%f\t%f\n", density, area, density * area, densityFactorHairStrands);

        // how many new guide hairs are needed
        if ( den < (density * area * densityFactorHairStrands))
        {
          change = 1;
          csHairStrand hairStrand;

          bA = rng->Get();
          bB = rng->Get() * (1 - bA);
          bC = 1 - bA - bB;

          //csPrintf("%d\t%d\t%d\n", indexA, indexB, indexC);

          hairStrand.guideHairs[0].distance = bA;
          hairStrand.guideHairs[0].index = indexA;
          hairStrand.guideHairs[1].distance = bB;
          hairStrand.guideHairs[1].index = indexB;
          hairStrand.guideHairs[2].distance = bC;
          hairStrand.guideHairs[2].index = indexC;

          hairStrand.controlPointsCount = csMin( (csMin( A.controlPointsCount,
            B.controlPointsCount) ), C.controlPointsCount);

  //         csPrintf("%d\n", hairStrand.controlPointsCount);

          csVector2 uv = A.uv * bA + B.uv * bB + C.uv * bC;

          // based on heightmap
          // point heightmap - modify to use convolution matrix or such
          float height = heightmap.data[ 4 * ((int)(uv.x * heightmap.width) + 
            (int)(uv.y * heightmap.height) * heightmap.width )] / 255.0f;

          float realDistance = height * heightFactor;
          float realTipDistance = realDistance - ((hairStrand.controlPointsCount - 2) 
            * controlPointsDistance ) ;

  //         csPrintf("%f\t%f\t%f\n", height, realDistance, realTipDistance);

          hairStrand.tipRatio = realTipDistance / controlPointsDistance;

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
    }
    //csPrintf("end\n");
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

  void FurMesh::GaussianBlur(TextureData texture)
  {
    CS::StructuredTextureFormat readbackFmt 
      (CS::TextureFormatStrings::ConvertStructured ("abgr8"));

    csRef<iDataBuffer> bufDB = texture.handle->Readback(readbackFmt);
    int width, height;
    texture.handle->GetOriginalDimensions(width, height);
    uint8* buf = bufDB->GetUint8();

    int gaussianMask[][5] = { {1, 4, 6, 4, 1}, {4, 16, 24, 16, 4}, {6, 24, 36, 24, 6},
      {4, 16, 24, 16, 4}, {1, 4, 6, 4, 1}};

    int *dest = new int [ 4 * width * height ];

    for (int x = 0; x < width; x ++)
      for (int y = 0; y < height; y ++)
        for (int channel = 0; channel < 4; channel ++)
        {
          int baseCoord = 4 * (x + y * width) + channel;
          dest[ baseCoord ] = 0;

          for (int x_offset = -2 ; x_offset <= 2; x_offset ++)
            for (int y_offset = -2; y_offset <= 2; y_offset ++)
            {
              int coord = 4 * (x + x_offset + (y + y_offset) * width) + channel;
              if (x + x_offset >= 0 && x + x_offset < width &&
                y + y_offset >= 0 && y + y_offset < height)
                dest[ baseCoord ] += buf[ coord ] * 
                  gaussianMask[2 + x_offset][2 + y_offset];
            }
        }

      for (int x = 0; x < width; x ++)
        for (int y = 0; y < height; y ++)
          for (int channel = 0; channel < 4; channel ++)
            buf[4 * (x + y * width) + channel] = 
              (uint8)(dest[4 * (x + y * width) + channel] / 256);

      // send buffer to texture
      densitymap.handle->Blit(0, 0, width, height / 2, buf);
      densitymap.handle->Blit(0, height / 2, width, height / 2, 
        buf + (width * height * 2));

      delete dest;
  }

  void FurMesh::SaveUVImage()
  {
    //csPrintf("%s\n", densitymap->GetImageName());

    // from normal guide ropes
    for (size_t i = 0 ; i < guideHairs.GetSize() ; i ++)
    {
      csVector2 uv = guideHairs.Get(i).uv;

      densitymap.data[ 4 * ((int)(uv.x * densitymap.width) + 
        (int)(uv.y * densitymap.height) * densitymap.width ) + 1 ] = 255;

      heightmap.data[ 4 * ((int)(uv.x * heightmap.width) + 
        (int)(uv.y * heightmap.height) * heightmap.width ) + 1 ] = 255;

    }

    // from LOD guide ropes
    for (size_t i = 0 ; i < guideHairsLOD.GetSize() ; i ++)
    {
      csVector2 uv = guideHairsLOD.Get(i).uv;

      densitymap.data[ 4 * ((int)(uv.x * densitymap.width) + 
        (int)(uv.y * densitymap.height) * densitymap.width ) ] = 255;
    }

    // from hair strands
    for (size_t i = 0 ; i < hairStrands.GetSize() ; i ++)
    {
      csVector2 uv = csVector2(0);
      csGuideHairReference *guideHairsReference = hairStrands.Get(i).guideHairs;
      
      for ( size_t j = 0 ; j < GUIDE_HAIRS_COUNT ; j ++ )
        if ( guideHairsReference[j].index < guideHairs.GetSize() )
          uv += guideHairsReference[j].distance * 
            (guideHairs.Get(guideHairsReference[j].index).uv);
        else
          uv += guideHairsReference[j].distance * (guideHairsLOD.Get(
            guideHairsReference[j].index - guideHairs.GetSize()).uv);

      densitymap.data[ 4 * ((int)(uv.x * densitymap.width) + 
        (int)(uv.y * densitymap.height) * densitymap.width ) + 2 ] = 255;
    }

    csPrintf("Pure guide ropes: %d\n", guideHairs.GetSize());
    csPrintf("Total guide ropes: %d\n", guideHairsLOD.GetSize() + guideHairs.GetSize());

    SaveImage(densitymap.data, "/data/krystal/krystal_skull_densitymap_debug.png",
      densitymap.width, densitymap.height);

    SaveImage(heightmap.data, "/data/krystal/krystal_skull_heightmap_debug.png",
      heightmap.width, heightmap.height);

  }

  void FurMesh::SaveImage(uint8* buf, const char* texname, 
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

  void FurMesh::SetPhysicsControl (iFurPhysicsControl* physicsControl)
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

  void FurMesh::SetMeshFactory ( iAnimatedMeshFactory* meshFactory)
  {
    this->meshFactory = meshFactory;
  }

  void FurMesh::SetMeshFactorySubMesh ( iAnimatedMeshFactorySubMesh* 
    meshFactorySubMesh )
  { 
    this->meshFactorySubMesh = meshFactorySubMesh;
  }

  void FurMesh::SetFurStrandGenerator( iFurStrandGenerator* hairStrandGenerator)
  {
    this->hairStrandGenerator = hairStrandGenerator;
  }

  iFurStrandGenerator* FurMesh::GetFurStrandGenerator( )
  {
    return hairStrandGenerator;
  }

  void FurMesh::SetMaterial ( iMaterial* material )
  {
    this->material = material;

    SetColor(csColor(1,1,0));
    SetGrowTangents();
    SetDensitymap();
    SetHeightmap();
    SetStrandWidth();
    SetDisplaceDistance();
  }

  void FurMesh::SetGrowTangents()
  {
    CS::ShaderVarName growTangentsName (svStrings, "growTangents");	
    csRef<csShaderVariable> shaderVariable = material->GetVariableAdd(growTangentsName);

    shaderVariable->GetValue(growTangents);

    CS::ShaderVarName positionDeviationName (svStrings, "positionDeviation");	
    shaderVariable = material->GetVariableAdd(positionDeviationName);

    shaderVariable->GetValue(positionDeviation);
  }

  void FurMesh::SetColor(csColor color)
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

  void FurMesh::SetStrandWidth()
  {
    CS::ShaderVarName strandWidthName (svStrings, "width");	
    csRef<csShaderVariable> shaderVariable = material->GetVariable(strandWidthName);

    shaderVariable->GetValue(strandWidth);
    strandWidthLOD = strandWidth;
  }

  void FurMesh::SetDensitymap ()
  {
    CS::ShaderVarName densitymapName (svStrings, "density map");	
    csRef<csShaderVariable> shaderVariable = material->GetVariable(densitymapName);

    shaderVariable->GetValue(densitymap.handle);

    CS::ShaderVarName densityFactorGuideHairsName (svStrings, "densityFactorGuideHairs");	
    material->GetVariable(densityFactorGuideHairsName)->GetValue(densityFactorGuideHairs);

    CS::ShaderVarName densityFactorHairStrandsName (svStrings, "densityFactorHairStrands");	
    material->GetVariable(densityFactorHairStrandsName)->GetValue(densityFactorHairStrands);    

    // density map
    CS::StructuredTextureFormat readbackFmt 
      (CS::TextureFormatStrings::ConvertStructured ("abgr8"));

    csRef<iDataBuffer> densitymapDB = densitymap.handle->Readback(readbackFmt);
    densitymap.handle->GetOriginalDimensions(densitymap.width, densitymap.height);
    densitymap.data = densitymapDB->GetUint8();

    // apply a Gaussian blur
    GaussianBlur(densitymap);
  }

  void FurMesh::SetHeightmap ()
  {
    CS::ShaderVarName heightmapName (svStrings, "height map");	
    csRef<csShaderVariable> shaderVariable = material->GetVariable(heightmapName);

    shaderVariable->GetValue(heightmap.handle);

    CS::ShaderVarName heightFactorName (svStrings, "heightFactor");	
    material->GetVariable(heightFactorName)->GetValue(heightFactor);

    // height map
    CS::StructuredTextureFormat readbackFmt 
      (CS::TextureFormatStrings::ConvertStructured ("abgr8"));

    csRef<iDataBuffer> heightmapDB = heightmap.handle->Readback(readbackFmt);
    heightmap.handle->GetOriginalDimensions(heightmap.width, heightmap.height);
    heightmap.data = heightmapDB->GetUint8();
  }

  void FurMesh::SetDisplaceDistance()
  {
    displaceDistance = 0.02f;

    CS::ShaderVarName displaceDistanceName (svStrings, "displaceDistance");	
    material->GetVariableAdd(displaceDistanceName)->GetValue(displaceDistance);

    controlPointsDistance = 0.05f;

    CS::ShaderVarName controlPointsDistanceName (svStrings, "controlPointsDistance");	
    material->GetVariableAdd(controlPointsDistanceName)->GetValue(controlPointsDistance);
  }

  void FurMesh::UpdateGuideHairs()
  {
    // update guide ropes
    for (size_t i = 0 ; i < guideHairs.GetSize(); i ++)
      physicsControl->AnimateStrand(i,  guideHairs.Get(i).controlPoints,
        guideHairs.Get(i).controlPointsCount);

    // update guide ropes LOD
    for (size_t i = 0 ; i < guideHairsLOD.GetSize(); i ++)
      if ( guideHairsLOD.Get(i).isActive )
        physicsControl->AnimateStrand(i + guideHairs.GetSize(),
          guideHairsLOD.Get(i).controlPoints, guideHairsLOD.Get(i).controlPointsCount);        
      else
        UpdateControlPoints(guideHairsLOD.Get(i).controlPoints,
          guideHairsLOD.Get(i).controlPointsCount, guideHairsLOD.Get(i).guideHairs);      
  }

  void FurMesh::UpdateControlPoints(csVector3 *controlPoints,
    size_t controlPointsCount, csGuideHairReference guideHairReference[GUIDE_HAIRS_COUNT])
  {
    for ( size_t i = 0 ; i < controlPointsCount; i++ )
    {
      controlPoints[i] = csVector3(0);
      for ( size_t j = 0 ; j < GUIDE_HAIRS_COUNT ; j ++ )
        if ( guideHairReference[j].index < guideHairs.GetSize() )
          controlPoints[i] += guideHairReference[j].distance * 
          (guideHairs.Get(guideHairReference[j].index).controlPoints[i]);
        else
          controlPoints[i] += guideHairReference[j].distance * (guideHairsLOD.Get
          (guideHairReference[j].index - guideHairs.GetSize()).controlPoints[i]);
    }
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
      {
        csHairStrand hairStrand = hairStrands.Get(i);

        UpdateControlPoints(hairStrand.controlPoints,
          hairStrand.controlPointsCount, hairStrand.guideHairs);
      }

    const csOrthoTransform& tc = view -> GetCamera() ->GetTransform ();

    csVector3* vbuf = (csVector3*)factory->GetVertices()->Lock(CS_BUF_LOCK_NORMAL);
    csVector3* normals = (csVector3*)factory->GetNormals()->Lock(CS_BUF_LOCK_NORMAL); 
    csVector3* tan = (csVector3*)factory->GetTangents()->Lock(CS_BUF_LOCK_NORMAL);
    csVector3* bin = (csVector3*)factory->GetBinormals()->Lock(CS_BUF_LOCK_READ);

    csVector3 normal, tangent, binormal, cameraOrigin;
    csVector3 strip, firstPoint, secondPoint;

    cameraOrigin = tc.GetOrigin();
    csVector3 *tangentBuffer = tan;
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
        vbuf += 2, tangentBuffer += 2, bin += 2, posShift ++, normals += 2 )
      {
        firstPoint = *controlPoints + (*posShift);
        secondPoint = *(controlPoints + 1) + (*posShift);

        csMath3::CalcNormal(binormal, firstPoint, secondPoint, cameraOrigin);
        binormal.Normalize();
        strip = strandWidthLOD * binormal * ((*bin).z + 1.0f) * (1.5f - (*bin).y);

        (*vbuf) = firstPoint;
        (*(vbuf + 1)) = firstPoint + strip;

        tangent = firstPoint - secondPoint;
        tangent.Normalize();
        normal.Cross(tangent, binormal);

        (*normals) = normal;
        (*(normals + 1)) = normal;

        (*tangentBuffer) = tangent;
        (*(tangentBuffer + 1)) = tangent;
      }

      if (controlPointsCount)
      {
        (*vbuf) = *controlPoints + (*posShift);
        (*(vbuf + 1)) = *controlPoints + strip + (*posShift);

        (*tangentBuffer) = tangent;
        (*(tangentBuffer + 1)) = tangent;

        (*normals) = normal;
        (*(normals + 1)) = normal;

        vbuf += 2;
        tangentBuffer += 2;
        bin += 2;
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

