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

    This is built around the lib3ds open source library. It requires a version
    later than July 7th 2001.

    More information can be found at http://sourceforge.net/projects/lib3ds/
*/

#include "cssysdef.h"
#include "csutil/csstring.h"
#include "csutil/datastrm.h"
#include "cstool/mdldata.h"
#include "imesh/mdldata.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/databuff.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/parray.h"

#include "ie3ds.h"
#include <lib3ds/camera.h>
#include <lib3ds/file.h>
#include <lib3ds/io.h>
#include <lib3ds/light.h>
#include <lib3ds/material.h>
#include <lib3ds/matrix.h>
#include <lib3ds/mesh.h>
#include <lib3ds/node.h>
#include <lib3ds/vector.h>

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE( csModelConverter3ds )
  SCF_IMPLEMENTS_INTERFACE( iModelConverter )
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE( iComponent )
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE( csModelConverter3ds::eiComponent )
  SCF_IMPLEMENTS_INTERFACE( iComponent )
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY( csModelConverter3ds )


// these are wrappers for csDataStream to interface with Lib3dsIO
static Lib3dsBool DataErrorFunc( void * )
{
  //  csDataStream *pData = (csDataStream*)self;

  // does nothing for now
  return LIB3DS_FALSE;
}


static long DataSeekFunc( void *self, long offset, Lib3dsIoSeek origin )
{
  csDataStream *pData = (csDataStream*)self;

  switch (origin)
  {
    case LIB3DS_SEEK_SET:
      break;
    case LIB3DS_SEEK_CUR:
      offset += pData->GetPosition();
      break;
    case LIB3DS_SEEK_END:
      offset += pData->GetLength();
      break;
    default:
      return 1;
  }
  pData->SetPosition( offset );
  return 0;
}


static long DataTellFunc( void *self )
{
  csDataStream *pData = (csDataStream*)self;
  return (long)pData->GetPosition();
}


static int DataReadFunc( void *self, Lib3dsByte *buffer, int size )
{
  csDataStream *pData = (csDataStream*)self;
  return pData->Read( buffer, size );
}


static int DataWriteFunc( void* /*self*/, const Lib3dsByte* /*buffer*/,
	int /*size*/ )
{
  //  csDataStream *pData = (csDataStream*)self;

  // not yet implemented
  return 0;
}



csModelConverter3ds::csModelConverter3ds( iBase *pBase )
{
  SCF_CONSTRUCT_IBASE( pBase );
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);

  FormatInfo.Name = "3ds";
  FormatInfo.CanLoad = true;
  FormatInfo.CanSave = false;
}

csModelConverter3ds::~csModelConverter3ds ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE();
}

bool csModelConverter3ds::Initialize( iObjectRegistry * )
{
  return true;
}

size_t csModelConverter3ds::GetFormatCount()
{
  return 1;
}

const csModelConverterFormat *csModelConverter3ds::GetFormat( size_t idx )
{
  if( idx == 0 )
    return &FormatInfo;
  else
    return 0;
}



static void AddTexels(Lib3dsMesh *p3dsMesh,  
		      int numTexels, iModelDataVertices * Vertices)
{
  csVector2 texel;
  float tex1, tex2;
  int i;
  Lib3dsVector bbmin;
  Lib3dsVector bbmax;
  float xdist, ydist, zdist;

  if (!numTexels)
  {
    // This means that the model doesn't contains UV coordinates so
    // we will use the default ones.
    /*Vertices->AddTexel (csVector2 (0, 0));
    Vertices->AddTexel (csVector2 (1, 0));
    Vertices->AddTexel (csVector2 (1, 1));*/

    // do a planar mapping, onto the plane made up by the 
    // axis of the two largest bounding box dimensions.
    // looks interesting as well, and has nice side effect
    // that the same vertex can not have different texture
    // coordinates.
    lib3ds_mesh_bounding_box (p3dsMesh, bbmin, bbmax);
    xdist = bbmax[0] - bbmin[0];
    ydist = bbmax[1] - bbmin[1];
    zdist = bbmax[2] - bbmin[2];

    int numVertices = p3dsMesh->points;
    Lib3dsPoint *pCurPoint = p3dsMesh->pointL;

    if ((xdist > zdist) && (ydist > zdist))
    {
      for (i = 0; i < numVertices; i++)
      {
	Vertices->AddTexel (csVector2 ((pCurPoint->pos[0]  - bbmin[0])/ xdist,
	  (pCurPoint->pos[1]  - bbmin[1])/ ydist));
	pCurPoint++;
      }
    }
    else if ((ydist > xdist) && (zdist > xdist))
    {
      for (i = 0; i < numVertices; i++)
      {
	Vertices->AddTexel (csVector2 ((pCurPoint->pos[1] - bbmin[1]) / ydist,
	  (pCurPoint->pos[2] - bbmin[2]) / zdist));
	pCurPoint++;
      }
    } 
    else
    {
      for (i = 0; i < numVertices; i++)
      {
	Vertices->AddTexel (csVector2 ((pCurPoint->pos[0] - bbmin[0]) / xdist,
	  (pCurPoint->pos[2]  - bbmin[2])/ zdist));
	pCurPoint++;
      }
    }

    return;
  };
  Lib3dsTexel *pCurTexel = p3dsMesh->texelL;
  for(i = 0; i < numTexels; i++)
  {
     tex1 = pCurTexel[0][0];
     tex2 = pCurTexel[0][1];
     texel = csVector2(tex1, 1.-tex2);
     Vertices->AddTexel(texel);
     pCurTexel++;
  }
  return;
}

typedef struct {
  Lib3dsFace* face;
  int faceVertex;
  int normal;
} AdjacentFace;

typedef csDirtyAccessArray<AdjacentFace> Faces;
typedef csPDelArray<Faces> FacesVector;
typedef csPDelArray<csVector3> csVector3Vector;

static void LoadTriangles (iModelDataObject *pDataObject,
	iModelDataVertices* Vertices, Lib3dsFace * pCurFace,
    int numTriangles, bool hasTexels, Lib3dsMaterial* firstMaterial)
{
 
  int i,j,k,index;
  iModelDataPolygon * pCurPoly;
  int numVertices = (int)Vertices->GetVertexCount();

  FacesVector adj_faces;

  adj_faces.SetLength (numVertices);
  for (i=0; i < numVertices; i++)
  {
    adj_faces.Put (i, new Faces ());
  }

/* collect all adjacent triangles for every vertex
   (also at which corner of the triangle the vertex is) */

  for ( i = 0 ; i < numTriangles ; i++ ) {
    for( j = 0 ; j <3 ; j++ )
    {
      Faces* faces = adj_faces[pCurFace[i].points[j]];
      AdjacentFace adjface;
      adjface.face = &pCurFace[i];
      adjface.faceVertex = j;
      adjface.normal = -1;
      faces->Push (adjface);
    }
  }

  csVector3Vector normals;

  for ( i = 0 ; i < numTriangles ; i++ ) {
    // create a new poly for the data object
    pCurPoly = new csModelDataPolygon ();
    //  add to the mesh object
    pDataObject->QueryObject ()->ObjAdd (pCurPoly->QueryObject ());
    /***  process the information for each polygon vertex  ***/
    //  get the indices for each vector of the triangle

    Lib3dsMaterial* pCurMat;
    for(pCurMat = firstMaterial;
        pCurMat && strcmp(pCurFace->material, pCurMat->name);
        pCurMat = pCurMat->next);

    csVector3 backnormal[3];

    int normindex = 0;

    if (!pCurFace->smoothing)
    {
      normindex = (int)Vertices->AddNormal (csVector3(pCurFace->normal[0], 
	pCurFace->normal[1], pCurFace->normal[2]));
      for( j = 0 ; j <3 ; j++ )
      {
	index = pCurFace->points[j];
      // now add the vertex
	if(hasTexels) 
	  pCurPoly->AddVertex(index, normindex, 0, index);
	else 
	  pCurPoly->AddVertex(index, normindex, 0, j);
      }
      if (pCurMat && pCurMat->two_sided) {
	  // material indicates this triangle is two-sided, so
	  // add a new triangle facing the opposite direction
	pCurPoly = new csModelDataPolygon ();
	pDataObject->QueryObject ()->ObjAdd (pCurPoly->QueryObject ());
	normindex = (int)Vertices->AddNormal (csVector3(-pCurFace->normal[0], 
	  -pCurFace->normal[1], -pCurFace->normal[2]));
	for( j = 2 ; j >= 0 ; j-- )
	{
	  index = pCurFace->points[j];
	  // now add the vertex
//	  if(hasTexels) 
	    pCurPoly->AddVertex(index, normindex , 0, index);
//	  else 
//	    pCurPoly->AddVertex(index, normindex , 0, j);
	  }
      }
    }
    else
    {
      for( j = 0 ; j <3 ; j++ )
      {
        int firstadjface = -1;
        csVector3 normal;

	index = pCurFace->points[j];

	for (k = (int)adj_faces[index]->Length()-1; k >= 0; k--)
	{
	  AdjacentFace &adj_face = (*adj_faces[index])[k];
	  /* check if there is an adjacent triangle in the same
	    smoothing group */
	  if (adj_face.face->smoothing & pCurFace->smoothing)
	  {
	    /*  take the normal if already calculated */
	    if (firstadjface == -1) 
	    {
	      if ((normindex = adj_face.normal) != -1)
	      {
		break;
	      }
	      firstadjface = k;
	      normal = 0;
	    }

	    normal.x += adj_face.face->normal[0];
	    normal.y += adj_face.face->normal[1];
	    normal.z += adj_face.face->normal[2];
	  }
	}
	/* calc the normal of all adjacent tris in the same
	    smoothing group & save it */
	if (normindex == -1)
	{
	  normal.Normalize();
	  normindex = (int)Vertices->AddNormal (normal);
	  (*adj_faces[index])[firstadjface].normal = normindex;
	}
	// now add the vertex
//	if(hasTexels) 
	  pCurPoly->AddVertex(index, normindex, 0, index);
//	else 
//	  pCurPoly->AddVertex(index, normindex, 0, j);
        if (pCurMat && pCurMat->two_sided)
	  backnormal[j] = -normal;
      }
      if (pCurMat && pCurMat->two_sided) {
	  // material indicates this triangle is two-sided, so
	  // add a new triangle facing the opposite direction
	pCurPoly = new csModelDataPolygon ();
	pDataObject->QueryObject ()->ObjAdd (pCurPoly->QueryObject ());
	for( j = 2 ; j >= 0 ; j-- )
	{
	  index = pCurFace->points[j];
	  normindex = (int)Vertices->AddNormal (backnormal[j]);
	  // now add the vertex
//	  if(hasTexels) 
	    pCurPoly->AddVertex(index, normindex , 0, index);
//	  else 
//	    pCurPoly->AddVertex(index, normindex , 0, j);
	  }
      }
    }
    pCurFace++;
  }
}

csPtr<iModelData> csModelConverter3ds::Load( uint8* buffer, uint32 size )
{
  Lib3dsFile *p3dsFile;
  csModelData *pModelData;

  /***  send the buffer in to be translated  ***/
  p3dsFile = LoadFileData( buffer, size );
  if( !p3dsFile )
    return 0; // kick out if there is a problem

  // make a new instance of iModelData
  pModelData = new csModelData( );
  pModelData->QueryObject ()->SetName ("model");

//   JUST LOAD UP MESHES FOR NOW

  /***  go through all of the cameras and convert them  ***/
/*
iModelDataCamera *pDataCamera;
Lib3dsCamera *pCurCamera;

  // set the current camera to the first in the file
  pCurCamera = p3dsFile->cameras;

  // as long as we have a valid camera...
  while( pCurCamera )
  {
    // get a new iModelData data camera
    pDataCamera = pModelData->CreateCamera();

    // now process that camera
    if( !LoadCameraData( pDataCamera, pCurCamera ) )
      return 0;

    // get the next camera in the chain
    pCurCamera = pCurCamera->next;
  }
*/

  /***  go through all of the lights and convert them  ***/
/*
iModelDataLight *pDataLight;
Lib3dsLight *pCurLight;

  // set the current light to the first in the file
  pCurLight = p3dsFile->lights;

  // as long as we have a valid light...
  while( pCurLight )
  {
    // get a new iModelData data light
    pDataLight = pModelData->CreateLight();

    // now process that light
    if( !LoadLightData( pDataLight, pCurLight) )
      return 0;

    // get the next light in the chain
    pCurLight = pCurLight->next;
  }
*/

  /***  go through all of the materials and convert them  ***/
/*
iModelDataMaterial *pDataMaterial;
Lib3dsMaterial *pCurMaterial;

  // set the current material to the first in the file
  pCurMaterial = p3dsFile->materials;

  // as long as we have a valid material...
  while( pCurMaterial )
  {
    // get a new iModelData data material
    pDataMaterial = pModelData->CreateMaterial();

    // now process that material
    if( !LoadMeshObjectData( pDataMaterial, pCurMaterial) )
      return 0;

    // get the next material in the chain
    pCurMaterial = pCurMaterial->next;
  }
*/

  /***  go through all of the mesh objects and convert them  ***/
  iModelDataObject  *pDataObject;
  Lib3dsMesh *pCurMesh;

  // RDS NOTE: add support for frames

  // set the current mesh to the first in the file
  pCurMesh = p3dsFile->meshes;

  // as long as we have a valid mesh...
  while( pCurMesh )
  {
    // get a new iModelData data object and set it's name
    pDataObject = new csModelDataObject( );
    pDataObject->QueryObject ()->SetName (pCurMesh->name);

    // add it to the model data
    pModelData->QueryObject ()->ObjAdd (pDataObject->QueryObject ());

    // now process that mesh
    if( !LoadMeshObjectData( pDataObject, pCurMesh, p3dsFile->materials) )
      return 0;

    pDataObject->DecRef ();

    // get the next mesh in the chain
    pCurMesh = pCurMesh->next;
  }

  lib3ds_file_free( p3dsFile );

  return SCF_QUERY_INTERFACE( pModelData, iModelData );
}

csPtr<iDataBuffer> csModelConverter3ds::Save( iModelData* /*pMdl*/,
	const char* /*formatName*/ )
{
  // not yet supported

  return 0;
}

bool csModelConverter3ds::LoadMeshObjectData( iModelDataObject *pDataObject,
    Lib3dsMesh *p3dsMesh, Lib3dsMaterial* pCurMaterial )
{

  /***  Load up vertices and texels  ***/
  int numVertices, i;

  // create the default vertex set
  iModelDataVertices *Vertices = new csModelDataVertices ();
  pDataObject->SetDefaultVertices (Vertices);

  // get the number of vertices in the current mesh
  numVertices = p3dsMesh->points;

  //  load up the vertices
  Lib3dsPoint *pCurPoint;
  float *xyz;
  csVector3 vertex;

  int numTexels = p3dsMesh->texels;

  pCurPoint = p3dsMesh->pointL;

  // add a dummy normal, white as the default color and three default texels
  //Vertices->AddNormal (csVector3 (0, 0, 0));
  Vertices->AddColor (csColor (1, 1, 1));

  AddTexels(p3dsMesh,numTexels,Vertices);


  for ( i = 0 ; i < numVertices ; i++ )
  {
    // Index to the position on the list using index (do I have to
    // loop? should I build a list)
    xyz = pCurPoint->pos;
    vertex = csVector3( xyz[0], xyz[1], xyz[2] );

    /// Add a vertex
    Vertices->AddVertex( vertex );
    pCurPoint++;
  }

  // It should be possible to add the texels

  /***  Load up the triangles  ***/

  int numTriangles;//, index, j;
  Lib3dsFace *pCurFace;

  //  get the trianle count and go to the first triangle
  numTriangles = p3dsMesh->faces;
  pCurFace = p3dsMesh->faceL;

  // now copy each triangle over
  /*
   * These two following functions creates the faces and assigns texels
   * depending
   * if the models had UV coordinates or not
   */
  LoadTriangles(pDataObject,Vertices,pCurFace,numTriangles, numTexels > 0, pCurMaterial);

  return true;
}

Lib3dsFile *csModelConverter3ds::LoadFileData( uint8* pBuffer, uint32 size )
{
  // This code is pulled from lib3ds
  Lib3dsFile *pFile;
  Lib3dsIo *pLibIO;
  csDataStream *pData;

  pFile = lib3ds_file_new();
  if( !pFile )
    return 0;

  // create a data stream from the buffer and don't delete it
  pData = new csDataStream( pBuffer, size, false );

  pLibIO = lib3ds_io_new(
    pData,
    DataErrorFunc,
    DataSeekFunc,
    DataTellFunc,
    DataReadFunc,
    DataWriteFunc
  );

  if( !pLibIO )
  {
    lib3ds_file_free( pFile );
    return 0;
  }

  if ( !lib3ds_file_read( pFile, pLibIO ) )
  {
    lib3ds_file_free( pFile );
    return 0;
  }

  if( !pFile )
    return 0;

  lib3ds_io_free( pLibIO );
  return pFile;
}

