/*
    Copyright (C) 2003 by Keith Fulton

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
#include "cssys/sysfunc.h"
#include "sprcal3d.h"
#include "csgeom/polyclip.h"
#include "csgeom/sphere.h"
#include "csutil/garray.h"
#include "csutil/randomgen.h"
#include "ivideo/graph3d.h"
#ifndef CS_USE_NEW_RENDERER
#include "ivideo/vbufmgr.h"
#endif // CS_USE_NEW_RENDERER
#include "iengine/camera.h"
#include "iengine/rview.h"
#include "iengine/movable.h"
#include "iengine/light.h"
#include "iengine/engine.h"
#include "iengine/sector.h"
#include "ivaria/reporter.h"
#include "iutil/objreg.h"
#include "iutil/cache.h"
#include "iutil/object.h"
#include "iutil/vfs.h"
#include "csutil/memfile.h"
#include "csutil/csmd5.h"
#include "iengine/mesh.h"
#include "cssys/csendian.h"
#include "qsqrt.h"

// STL include required by cal3d
#include <string>


CS_IMPLEMENT_PLUGIN



//--------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csSpriteCal3DMeshObjectFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iSpriteCal3DFactoryState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iLODControl)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
  {
    static scfInterfaceID iPolygonMesh_scfID = (scfInterfaceID)-1;		
    if (iPolygonMesh_scfID == (scfInterfaceID)-1)				
      iPolygonMesh_scfID = iSCF::SCF->GetInterfaceID ("iPolygonMesh");		
    if (iInterfaceID == iPolygonMesh_scfID &&				
      scfCompatibleVersion (iVersion, iPolygonMesh_VERSION))		
    {
      printf ("Deprecated feature use: iPolygonMesh queried from Sprite3d "
	"factory; use iObjectModel->GetPolygonMeshColldet() instead.\n");
      iPolygonMesh* Object = scfiObjectModel.GetPolygonMeshColldet();
      (Object)->IncRef ();						
      return STATIC_CAST(iPolygonMesh*, Object);				
    }
  }
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpriteCal3DMeshObjectFactory::SpriteCal3DFactoryState)
  SCF_IMPLEMENTS_INTERFACE (iSpriteCal3DFactoryState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpriteCal3DMeshObjectFactory::LODControl)
  SCF_IMPLEMENTS_INTERFACE (iLODControl)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpriteCal3DMeshObjectFactory::ObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

void csSpriteCal3DMeshObjectFactory::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.mesh.sprite.3d", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

csSpriteCal3DMeshObjectFactory::csSpriteCal3DMeshObjectFactory (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSpriteCal3DFactoryState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiLODControl);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPolygonMesh);

  scfiPolygonMesh.SetFactory (this);
  scfiObjectModel.SetPolygonMeshBase (&scfiPolygonMesh);
  scfiObjectModel.SetPolygonMeshColldet (&scfiPolygonMesh);
  scfiObjectModel.SetPolygonMeshViscull (0);
  scfiObjectModel.SetPolygonMeshShadows (0);
}

csSpriteCal3DMeshObjectFactory::~csSpriteCal3DMeshObjectFactory ()
{
    // Now remove ugly CS material hack from model so cal dtor will work
//    for (int i=0; i<calCoreModel.getCoreMaterialCount(); i++)
//    {
//	CalCoreMaterial *mat = calCoreModel.getCoreMaterial(i);
//	std::vector< CalCoreMaterial::Map > maps = mat->getVectorMap();
//	maps.clear();
//    }

    calCoreModel.destroy();
}

bool csSpriteCal3DMeshObjectFactory::Create(const char *name)
{
    return calCoreModel.create(name);
}

void csSpriteCal3DMeshObjectFactory::ReportLastError ()
{
    CalError::printLastError();
}

void csSpriteCal3DMeshObjectFactory::SetBasePath(const char *path)
{
    basePath = path;
}

void csSpriteCal3DMeshObjectFactory::SetRenderScale(float scale)
{
    renderScale = scale;
}

bool csSpriteCal3DMeshObjectFactory::LoadCoreSkeleton(const char *filename)
{
    csString path(basePath);
    path.Append(filename);
    return calCoreModel.loadCoreSkeleton((const char *)path);
}

int  csSpriteCal3DMeshObjectFactory::LoadCoreAnimation(const char *filename,
						       const char *name,
						       int type,
						       float base_vel, 
						       float min_vel, 
						       float max_vel)
{
    csString path(basePath);
    path.Append(filename);
    int id = calCoreModel.loadCoreAnimation((const char *)path);
    if (id != -1)
    {
	csCal3DAnimation *an = new csCal3DAnimation;
	an->name          = name;
	an->type          = type;
	an->base_velocity = base_vel;
	an->min_velocity  = min_vel;
	an->max_velocity  = max_vel;

	an->index = anims.Push(an);

	std::string str(name);
	calCoreModel.addAnimHelper(str,id);
    }
    return id;
}

bool csSpriteCal3DMeshObjectFactory::LoadCoreMesh(const char *filename,
						  const char *name,
						  bool attach,
						  iMaterialWrapper *defmat)
{
    csString path(basePath);
    path.Append(filename);

    csCal3DMesh *mesh = new csCal3DMesh;
    mesh->index = calCoreModel.loadCoreMesh((const char *)path);
    if (mesh->index == -1)
    {
	delete mesh;
	return false;
    }
    mesh->name              = name;
    mesh->attach_by_default = attach;
    mesh->default_material  = defmat;

    submeshes.Push(mesh);

    return true;
}

const char *csSpriteCal3DMeshObjectFactory::GetMeshName(int idx)
{
    if (idx >= submeshes.Length())
	return 0;

    return submeshes[idx]->name;
}

bool csSpriteCal3DMeshObjectFactory::IsMeshDefault(int idx)
{
    if (idx >= submeshes.Length())
	return 0;

    return submeshes[idx]->attach_by_default;
}

int  csSpriteCal3DMeshObjectFactory::FindMeshName(const char *meshName)
{
    for (int i=0; i<submeshes.Length(); i++)
    {
	if (submeshes[i]->name == meshName)
	    return i;
    }
    return -1;
}

bool csSpriteCal3DMeshObjectFactory::AddCoreMaterial(iMaterialWrapper *mat)
{
    CalCoreMaterial *newmat = new CalCoreMaterial;
    CalCoreMaterial::Map newmap;
    newmap.userData = mat;

    newmat->create();
    newmat->reserve(1);
    newmat->setMap(0,newmap);  // sticking iMaterialWrapper into 2 places
    newmat->setUserData(mat);  // jam CS iMaterialWrapper into cal3d material holder
   
    calCoreModel.addCoreMaterial(newmat);
    return true;
}

void csSpriteCal3DMeshObjectFactory::BindMaterials()
{
  int materialId;

  // make one material thread for each material
  // NOTE: this is not the right way to do it, but this viewer can't do the right
  // mapping without further information on the model etc.
  for(materialId = 0; materialId < calCoreModel.getCoreMaterialCount(); materialId++)
  {
    // create the a material thread
    calCoreModel.createCoreMaterialThread(materialId);

    // initialize the material thread
    calCoreModel.setCoreMaterialId(materialId, 0, materialId);
  }
}


csPtr<iMeshObject> csSpriteCal3DMeshObjectFactory::NewInstance ()
{
  csSpriteCal3DMeshObject* spr = new csSpriteCal3DMeshObject (0, calCoreModel);
  spr->SetFactory (this);

  csRef<iMeshObject> im (SCF_QUERY_INTERFACE (spr, iMeshObject));
  spr->DecRef ();
  return csPtr<iMeshObject> (im);
}


//=============================================================================

SCF_IMPLEMENT_IBASE (csSpriteCal3DMeshObjectFactory::PolyMesh)
  SCF_IMPLEMENTS_INTERFACE (iPolygonMesh)
SCF_IMPLEMENT_IBASE_END

//=============================================================================

SCF_IMPLEMENT_IBASE (csSpriteCal3DMeshObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iSpriteCal3DState)
#ifndef CS_USE_NEW_RENDERER
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iVertexBufferManagerClient)
#else
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iRenderBufferSource)
#endif // CS_USE_NEW_RENDERER
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iLODControl)
  {
    static scfInterfaceID iPolygonMesh_scfID = (scfInterfaceID)-1;		
    if (iPolygonMesh_scfID == (scfInterfaceID)-1)				
      iPolygonMesh_scfID = iSCF::SCF->GetInterfaceID ("iPolygonMesh");		
    if (iInterfaceID == iPolygonMesh_scfID &&				
      scfCompatibleVersion (iVersion, iPolygonMesh_VERSION))		
    {
#ifdef CS_DEBUG
      printf ("Deprecated feature use: iPolygonMesh queried from Sprite3d "
	"object; use iMeshObject->GetObjectModel()->"
	"GetPolygonMeshColldet() instead.\n");
#endif
      (&scfiPolygonMesh)->IncRef ();						
      return STATIC_CAST(iPolygonMesh*, &scfiPolygonMesh);				
    }
  }
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
  {
    static scfInterfaceID iPolygonMesh_scfID = (scfInterfaceID)-1;		
    if (iPolygonMesh_scfID == (scfInterfaceID)-1)				
      iPolygonMesh_scfID = iSCF::SCF->GetInterfaceID ("iPolygonMesh");		
    if (iInterfaceID == iPolygonMesh_scfID &&				
      scfCompatibleVersion (iVersion, iPolygonMesh_VERSION))		
    {
      printf ("Deprecated feature use: iPolygonMesh queried from Sprite3d "
	"factory; use iObjectModel->GetPolygonMeshColldet() instead.\n");
      iPolygonMesh* Object = scfiObjectModel.GetPolygonMeshColldet();
      (Object)->IncRef ();						
      return STATIC_CAST(iPolygonMesh*, Object);				
    }
  }
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpriteCal3DMeshObject::SpriteCal3DState)
  SCF_IMPLEMENTS_INTERFACE (iSpriteCal3DState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpriteCal3DMeshObject::PolyMesh)
  SCF_IMPLEMENTS_INTERFACE (iPolygonMesh)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

#ifndef CS_USE_NEW_RENDERER
SCF_IMPLEMENT_EMBEDDED_IBASE (csSpriteCal3DMeshObject::eiVertexBufferManagerClient)
  SCF_IMPLEMENTS_INTERFACE (iVertexBufferManagerClient)
SCF_IMPLEMENT_EMBEDDED_IBASE_END
#else
SCF_IMPLEMENT_EMBEDDED_IBASE (csSpriteCal3DMeshObject::RenderBufferSource)
  SCF_IMPLEMENTS_INTERFACE (iRenderBufferSource)
SCF_IMPLEMENT_EMBEDDED_IBASE_END
#endif // CS_USE_NEW_RENDERER

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpriteCal3DMeshObject::LODControl)
  SCF_IMPLEMENTS_INTERFACE (iLODControl)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpriteCal3DMeshObject::ObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END



csSpriteCal3DMeshObject::csSpriteCal3DMeshObject (iBase *pParent,CalCoreModel& calCoreModel)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSpriteCal3DState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiLODControl);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiVertexBufferManagerClient);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPolygonMesh);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);

//  scfiPolygonMesh.SetFactory (this);
//  scfiObjectModel.SetPolygonMeshBase (&scfiPolygonMesh);
//  scfiObjectModel.SetPolygonMeshColldet (&scfiPolygonMesh);
//  scfiObjectModel.SetPolygonMeshViscull (0);
//  scfiObjectModel.SetPolygonMeshShadows (0);

  // create the model instance from the loaded core model
  if(!calModel.create(&calCoreModel))
  {
    CalError::printLastError();
    return;
  }

  // set the material set of the whole model
  vis_cb = 0;
  arrays_initialized = false;
}

csSpriteCal3DMeshObject::~csSpriteCal3DMeshObject ()
{
  calModel.destroy();
  delete [] meshes;
  delete [] is_initialized;
  delete [] meshes_colors;
}


void csSpriteCal3DMeshObject::SetFactory (csSpriteCal3DMeshObjectFactory* tmpl)
{
  factory = tmpl;

  // attach all default meshes to the model
  int meshId;
  for(meshId = 0; meshId < factory->GetMeshCount(); meshId++)
  {
    if (factory->submeshes[meshId]->attach_by_default)
    {
	AttachCoreMesh(factory->submeshes[meshId]->index,(int)factory->submeshes[meshId]->default_material);
    }
  }
//  calModel.setMaterialSet(0);
  calModel.update(0);
}

void csSpriteCal3DMeshObject::SetupVertexBuffer (int mesh,int submesh,int num_vertices,int num_triangles,csTriangle *triangles)
{
  if (!meshes[mesh][submesh].buffers[0])
  {
    // @@@ priority should be a parameter.
    if (!vbuf)
    {
        iObjectRegistry* object_reg = ((csSpriteCal3DMeshObjectFactory*)factory)
						  ->object_reg;
	csRef<iGraphics3D> g3d (CS_QUERY_REGISTRY (object_reg, iGraphics3D));
	vbufmgr = g3d->GetVertexBufferManager ();
	vbuf = vbufmgr->CreateBuffer (0);
	vbufmgr->AddClient (&scfiVertexBufferManagerClient);
    }
    meshes[mesh][submesh].buffers[0] = vbuf;

    meshes[mesh][submesh].vertex_fog = new G3DFogInfo[num_vertices];
    meshes[mesh][submesh].num_triangles = num_triangles;
    meshes[mesh][submesh].triangles = new csTriangle[num_triangles];
    memcpy (meshes[mesh][submesh].triangles, triangles, sizeof(csTriangle)*num_triangles);
  }
}

void csSpriteCal3DMeshObject::GetRadius (csVector3& rad, csVector3& cent)
{
    cent.Set(object_bbox.GetCenter());
    csVector3 maxbox,minbox;
    
    maxbox = object_bbox.Max();
    minbox = object_bbox.Min();
    float r1 = (maxbox.x-minbox.x)/2;
    float r2 = (maxbox.y-minbox.y)/2;
    float r3 = (maxbox.z-minbox.z)/2;
    rad.Set(r1,r2,r3);
}

void csSpriteCal3DMeshObject::GetObjectBoundingBox (csBox3& bbox, int type)
{
    bbox = object_bbox;
}

void csSpriteCal3DMeshObjectFactory::GetRadius (csVector3& rad, csVector3& cent)
{
    cent.Set(0,0,0);
    rad.Set(1,1,1);
}

void csSpriteCal3DMeshObjectFactory::GetObjectBoundingBox (csBox3& bbox, int type)
{
    bbox.AddBoundingVertexSmart(-1,-1,-1);
    bbox.AddBoundingVertexSmart(1,1,1);
}

void csSpriteCal3DMeshObject::GetObjectBoundingBox (csBox3& bbox, int type, csVector3 *verts,int vertCount)
{
  int vertex;

//  bbox.StartBoundingBox (verts[0]);
  for ( vertex = 1 ; vertex < vertCount; vertex++ )
  {
      bbox.AddBoundingVertexSmart (verts[vertex]);
  }
}


void csSpriteCal3DMeshObject::UpdateLighting(iLight** lights, int num_lights,iMovable* movable)
{
  CalRenderer *pCalRenderer;
  pCalRenderer = calModel.getRenderer();

  // begin the rendering loop
  if(!pCalRenderer->beginRendering())
  {
    return;
  }

  int meshCount;
  meshCount = pCalRenderer->getMeshCount();

  // loop through all meshes of the model
  int meshId;
  for(meshId = 0; meshId < meshCount; meshId++)
  {
    // get the number of submeshes
    int submeshCount;
    submeshCount = pCalRenderer->getSubmeshCount(meshId);

    // loop through all submeshes of the mesh
    int submeshId;
    for(submeshId = 0; submeshId < submeshCount; submeshId++)
    {
      // select mesh and submesh for further data access
      if(pCalRenderer->selectMeshSubmesh(meshId, submeshId))
      {
	  UpdateLightingSubmesh(lights,num_lights,movable,pCalRenderer,meshId, submeshId);
      }
    }
  }

  pCalRenderer->endRendering();

  return;
}


void csSpriteCal3DMeshObject::UpdateLightingSubmesh (iLight** lights, int num_lights,iMovable* movable,CalRenderer *pCalRenderer,int mesh, int submesh)
{
  SetupObject ();

  int vertCount;
  vertCount = pCalRenderer->getVertexCount();

  int i, l;
  csColor* colors = meshes_colors[mesh][submesh];

  if (!colors)
  {
      meshes_colors[mesh][submesh] = new csColor[vertCount];
      colors = meshes_colors[mesh][submesh];
  }

  // get the transformed normals of the submesh
  static float meshNormals[30000][3];
  pCalRenderer->getNormals(&meshNormals[0][0]);

  // Set all colors to ambient light.
  csColor col;
  if (((csSpriteCal3DMeshObjectFactory*)factory)->engine)
  {
    ((csSpriteCal3DMeshObjectFactory*)factory)->engine->GetAmbientLight (col);
//    col += color;  // no inherent color in cal3d sprites
    iSector* sect = movable->GetSectors ()->Get (0);
    if (sect)
      col += sect->GetDynamicAmbientLight ();
  }
  else
  {
//    col = color;
  }
  for (i = 0 ; i < vertCount ; i++)
    colors[i] = col;

//  if (!do_lighting)
//      return;

  // Do the lighting.
  csReversibleTransform trans = movable->GetFullTransform ();
  // the object center in world coordinates. "0" because the object
  // center in object space is obviously at (0,0,0).
  csColor color;
  for (l = 0 ; l < num_lights ; l++)
  {
    iLight* li = lights[l];
    // Compute light position in object coordinates
    csVector3 wor_light_pos = li->GetCenter ();
    csVector3 obj_light_pos = trans.Other2This (wor_light_pos);
    float obj_sq_dist = csSquaredDist::PointPoint (obj_light_pos, 0);
#ifdef CS_USE_NEW_RENDERER
    if (obj_sq_dist >= li->GetInfluenceRadiusSq ()) continue;
#else
    if (obj_sq_dist >= li->GetSquaredRadius ()) continue;
#endif
    float in_obj_dist = (obj_sq_dist >= SMALL_EPSILON)?qisqrt (obj_sq_dist):1.0f;

    csColor light_color = li->GetColor () * (256.0f / CS_NORMAL_LIGHT_LEVEL)
      * li->GetBrightnessAtDistance (qsqrt (obj_sq_dist));

    for (i = 0; i < vertCount; i++)
    {
      csVector3 normal(meshNormals[i][0],meshNormals[i][1],meshNormals[i][2]);
      float cosinus;
      if (obj_sq_dist < SMALL_EPSILON) cosinus = 1;
      else cosinus = obj_light_pos * normal; 
      // because the vector from the object center to the light center
      // in object space is equal to the position of the light

      if (cosinus > 0)
      {
        color = light_color;
        if (obj_sq_dist >= SMALL_EPSILON) cosinus *= in_obj_dist;
        if (cosinus < 1.0f) color *= cosinus;
	colors[i] += color;
      }
    }
  }

  // Clamp all vertex colors to 2.
  for (i = 0 ; i < vertCount; i++)
    colors[i].Clamp (2.0f, 2.0f, 2.0f);
}

void csSpriteCal3DMeshObject::SetupObjectSubmesh(int index)
{
    if (!arrays_initialized)
	return;  // First draw call to SetupObject will take care of most of this.

    if (!calModel.getMesh(index))
	return;

    int submeshes = calModel.getMesh(index)->getSubmeshCount();
    for (int j=0; j<submeshes; j++)
    {
	bool flag = false;
	is_initialized[index].Push(flag);
	G3DTriangleMesh sample;
	sample.num_vertices_pool = 1;  // no blending
	sample.buffers[0]  = 0; // NULL
	meshes[index].Push(sample);
	meshes_colors[index].Push(0);
    }
}

void csSpriteCal3DMeshObject::SetupObject()
{
  int meshCount;
  meshCount = calModel.getRenderer()->getMeshCount();

  if (!arrays_initialized)
  {
    arrays_initialized = true;
    meshes = new csArray<G3DTriangleMesh> [ meshCount ];
    is_initialized = new csArray<bool> [ meshCount ];
    meshes_colors  = new csArray<csColor*> [ meshCount ];

    for (int index=0; index<meshCount; index++)
    {
      SetupObjectSubmesh(index);
    }
  }
  for (int index=0; index<meshCount; index++)
  {
    CalMesh *mesh = calModel.getMesh(index);
    if (!mesh)
	continue;
    int submeshes = mesh->getSubmeshCount();
    for (int j=0; j<submeshes; j++)
    {
      if (!is_initialized[index][j])
      {
	  is_initialized[index][j] = true;

//	  delete[] meshes[index][j].triangles;
//	  delete[] meshes[index][j].vertex_fog;
	  meshes[index][j].triangles = 0;
	  meshes[index][j].vertex_fog = 0;

	  factory->GetObjectBoundingBox(object_bbox);  // initialize object_bbox here

	  meshes[index][j].morph_factor = 0.0f;
	  meshes[index][j].num_vertices_pool = 1;
	  meshes[index][j].do_morph_texels = false;
	  meshes[index][j].do_morph_colors = false;
	  meshes[index][j].vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
      }
    }
  }
}

bool csSpriteCal3DMeshObject::DrawTest (iRenderView* rview, iMovable* movable)
{
  SetupObject ();
  iGraphics3D* g3d = rview->GetGraphics3D ();
  iCamera* camera = rview->GetCamera ();

  // First create the transformation from object to camera space directly:
  //   W = Mow * O - Vow;
  //   C = Mwc * (W - Vwc)
  // ->
  //   C = Mwc * (Mow * O - Vow - Vwc)
  //   C = Mwc * Mow * O - Mwc * (Vow + Vwc)
  csReversibleTransform tr_o2c;
  tr_o2c = camera->GetTransform ();
  if (!movable->IsFullTransformIdentity ())
    tr_o2c /= movable->GetFullTransform ();

//  float scale = factory->GetRenderScale();
//  csMatrix3 scale_mat(scale,0,0,0,scale,0,0,0,scale);

//  tr_o2c *= scale_mat;

  csVector3 radius;
  csSphere sphere;
  GetRadius (radius, sphere.GetCenter ());
  float max_radius = radius.x;
  if (max_radius < radius.y) max_radius = radius.y;
  if (max_radius < radius.z) max_radius = radius.z;
  sphere.SetRadius (max_radius);
  int clip_portal, clip_plane, clip_z_plane;
  if (rview->ClipBSphere (tr_o2c, sphere, clip_portal, clip_plane,
  	clip_z_plane) == false)
    return false;

  g3d->SetObjectToCamera (&tr_o2c);
  int meshCount;
  meshCount = calModel.getRenderer()->getMeshCount();
  for (int i=0; i<meshCount; i++)
  {
    if (!calModel.getMesh(i))
	continue;
    for (int j=0; j<calModel.getMesh(i)->getSubmeshCount(); j++)
    {
      meshes[i][j].clip_portal = clip_portal;
      meshes[i][j].clip_plane = clip_plane;
      meshes[i][j].clip_z_plane = clip_z_plane;
      meshes[i][j].do_mirror = camera->IsMirrored ();
    }
  }

  return true;
}


bool csSpriteCal3DMeshObject::DrawSubmesh (iGraphics3D* g3d,
					   iRenderView* rview,
					   CalRenderer *pCalRenderer,
					   int mesh, 
					   int submesh,
					   iMaterialWrapper *material)
{
    // get the transformed vertices of the submesh
    static float meshVertices[30000][3];
    int vertexCount;
    vertexCount = pCalRenderer->getVertices(&meshVertices[0][0]);

    GetObjectBoundingBox (object_bbox, 0, (csVector3 *)meshVertices,vertexCount);

    // get the texture coordinates of the submesh
    // (only for the first map as example, others can be accessed in the same way though)
    static float meshTextureCoordinates[30000][2];
    int textureCoordinateCount;
    textureCoordinateCount = pCalRenderer->getTextureCoordinates(0, &meshTextureCoordinates[0][0]);

    // get the stored texture identifier
    // (only for the first map as example, others can be accessed in the same way though)
//    iMaterialWrapper *material;
//    material = (iMaterialWrapper *)pCalRenderer->getMapUserData(0);

    // get the faces of the submesh
    static int meshFaces[50000][3];
    int faceCount;
    faceCount = pCalRenderer->getFaces(&meshFaces[0][0]);

    SetupVertexBuffer (mesh, submesh, vertexCount, faceCount, (csTriangle *)meshFaces);
    if (material)
	material->Visit ();

    meshes[mesh][submesh].mat_handle = (material)?material->GetMaterialHandle():0;
    meshes[mesh][submesh].use_vertex_color = true;
    meshes[mesh][submesh].mixmode = /* MixMode  | */ CS_FX_GOURAUD;
    meshes[mesh][submesh].vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
    
    CS_ASSERT (!vbuf->IsLocked ());

    vbufmgr->LockBuffer (vbuf,
			 (csVector3 *)meshVertices,
			 (csVector2 *)meshTextureCoordinates,
			 meshes_colors[mesh][submesh],
			 vertexCount,
			 0, object_bbox);

    rview->CalculateFogMesh (g3d->GetObjectToCamera (), meshes[mesh][submesh]);
    g3d->DrawTriangleMesh (meshes[mesh][submesh]);

    vbufmgr->UnlockBuffer (vbuf);
    return true;
}

bool csSpriteCal3DMeshObject::Draw (iRenderView* rview, iMovable* /*movable*/,
	csZBufMode mode)
{
  if (vis_cb) if (!vis_cb->BeforeDrawing (this, rview)) return false;

  iGraphics3D* g3d = rview->GetGraphics3D ();

  CalRenderer *pCalRenderer;
  pCalRenderer = calModel.getRenderer();

  // begin the rendering loop
  if(!pCalRenderer->beginRendering())
  {
    return false;
  }

  // Prepare for rendering.
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, mode);


  int meshCount;
  meshCount = pCalRenderer->getMeshCount();

  // loop through all meshes of the model
  int meshId;
  for(meshId = 0; meshId < meshCount ; meshId++)
  {
    // get the number of submeshes
    int submeshCount;
    submeshCount = pCalRenderer->getSubmeshCount(meshId);

    // loop through all submeshes of the mesh
    int submeshId;
    for(submeshId = 0; submeshId < submeshCount; submeshId++)
    {
      // select mesh and submesh for further data access
      if(pCalRenderer->selectMeshSubmesh(meshId, submeshId))
      {
	  iMaterialWrapper *mat = (iMaterialWrapper *)calModel.getVectorMesh()[meshId]->getSubmesh(submeshId)->getCoreMaterialId();
	  DrawSubmesh(g3d,rview,pCalRenderer,meshId,submeshId,mat);
      }
    }
  }

  pCalRenderer->endRendering();

  return true;
}

bool csSpriteCal3DMeshObject::Advance (csTicks current_time)
{
  float delta = ((float)current_time - last_update_time)/1000.0F;
  calModel.update(delta);
  last_update_time = current_time;
  return true;
}

//--------------------------------------------------------------------------

csMeshedPolygon* csSpriteCal3DMeshObject::PolyMesh::GetPolygons ()
{
    return 0;
}

void csSpriteCal3DMeshObject::eiVertexBufferManagerClient::ManagerClosing ()
{
  if (scfParent->vbuf)
  {
    scfParent->vbuf = 0;
    scfParent->vbufmgr = 0;
  }
}

int csSpriteCal3DMeshObject::GetAnimCount()
{
    return calModel.getCoreModel()->getCoreAnimationCount();
}

const char *csSpriteCal3DMeshObject::GetAnimName(int idx)
{
    if (idx >= GetAnimCount())
	return 0;

    return factory->anims[idx]->name;
}

int csSpriteCal3DMeshObject::GetAnimType(int idx)
{
    if (idx >= GetAnimCount())
	return 0;

    return factory->anims[idx]->type;
}

int csSpriteCal3DMeshObject::FindAnim(const char *name)
{
    int count = GetAnimCount();

    for (int i=0; i<count; i++)
    {
	if (factory->anims[i]->name == name)
	    return i;
    }
    return -1;
}

void csSpriteCal3DMeshObject::ClearAllAnims()
{
    while (active_anims.Length())
    {
	csCal3DAnimation *pop = active_anims.Pop();
	ClearAnimCycle(pop->index,0);  // do not delete pop because ptr is shared with factory
    }
}

bool csSpriteCal3DMeshObject::SetAnimCycle(const char *name, float weight)
{
    ClearAllAnims();
    return AddAnimCycle(name, weight, 0);
}

bool csSpriteCal3DMeshObject::AddAnimCycle(const char *name, float weight, float delay)
{
    int idx = FindAnim(name);
    if (idx == -1)
	return false;

    calModel.getMixer()->blendCycle(idx,weight,delay);
    
    active_anims.Push(factory->anims[idx]);

    return true;
}

void csSpriteCal3DMeshObject::ClearAnimCycle(int idx, float delay)
{
    calModel.getMixer()->clearCycle(idx,delay);
}

bool csSpriteCal3DMeshObject::ClearAnimCycle(const char *name, float delay)
{
    int idx = FindAnim(name);
    if (idx == -1)
	return false;
    
    ClearAnimCycle(idx,delay);

    return true;
}

bool csSpriteCal3DMeshObject::SetAnimAction(const char *name, float delayIn, float delayOut)
{
    int idx = FindAnim(name);
    if (idx == -1)
	return false;

    calModel.getMixer()->executeAction(idx,delayIn,delayOut);

    return true;
}

bool csSpriteCal3DMeshObject::SetVelocity(float vel)
{
    int count = GetAnimCount();

    for (int i=0; i<count; i++)
    {
	if (factory->anims[i]->type == iSpriteCal3DState::C3D_ANIM_TYPE_TRAVEL)
	{
	    if (vel < factory->anims[i]->min_velocity ||
		vel > factory->anims[i]->max_velocity)
		continue;

	    float pct,vel_diff;
	    if (vel == factory->anims[i]->base_velocity)
		pct = 1;
	    else if (vel < factory->anims[i]->base_velocity)
	    {
		vel_diff = factory->anims[i]->base_velocity - factory->anims[i]->min_velocity;
		pct      = (vel - factory->anims[i]->min_velocity) / vel_diff;
	    }
	    else
	    {
		vel_diff = factory->anims[i]->max_velocity - factory->anims[i]->base_velocity;
		pct      = 1 - (factory->anims[i]->min_velocity - vel) / vel_diff;
	    }
	    calModel.getMixer()->blendCycle(i,pct,0);
	    if (pct == 1)
		break;
	}
    }

    return true;    
}

void csSpriteCal3DMeshObject::SetLOD(float lod)
{
    calModel.setLodLevel(lod);
}

bool csSpriteCal3DMeshObject::AttachCoreMesh(const char *meshname)
{
    int idx = factory->FindMeshName(meshname);
    if (idx == -1)
	return false;

    return AttachCoreMesh(factory->submeshes[idx]->index,(int)factory->submeshes[idx]->default_material);
}

bool csSpriteCal3DMeshObject::AttachCoreMesh(int mesh_id,int iMatWrapID)
{
    if (!calModel.attachMesh(mesh_id))
	return false;

    // Since all our vertex buffers and G3DTriangles are in arrays
    // which line up with the attached submesh list on the calModel,
    // we need to track these id's in the identical way so that
    // we can remove the right G3DTriangles when the submesh is
    // removed.

    attached_ids.Push(mesh_id);
    SetupObjectSubmesh(attached_ids.Length()-1);
    CalMesh *mesh = calModel.getMesh(mesh_id);
    for (int i=0; i<mesh->getSubmeshCount(); i++)
    {
	mesh->getSubmesh(i)->setCoreMaterialId(iMatWrapID);
    }
//    mesh->setMaterialSet(0);
    return true;
}

bool csSpriteCal3DMeshObject::DetachCoreMesh(const char *meshname)
{
    int idx = factory->FindMeshName(meshname);
    if (idx == -1)
	return false;

    return DetachCoreMesh(factory->submeshes[idx]->index);
}

bool csSpriteCal3DMeshObject::DetachCoreMesh (int mesh_id)
{
  if (!calModel.detachMesh(mesh_id))
    return false;

  // Now that the submesh is removed from the model, we must
  // remove all the CS rendering structures as well.
  int i;
  for (i=0; i<attached_ids.Length(); i++)
  {
    if (attached_ids[i] == mesh_id)
    {
      meshes[i].DeleteAll();
      is_initialized[i].DeleteAll();
      meshes_colors[i].DeleteAll();

      int j;
      for (j=i+1; j<attached_ids.Length(); j++)
      {
        meshes[j-1] = meshes[j];
	is_initialized[j-1] = is_initialized[j];
	meshes_colors[j-1]  = meshes_colors[j];
      }
      meshes[j-1].DeleteAll();
      is_initialized[j-1].DeleteAll();
      meshes_colors[j-1].DeleteAll();

      attached_ids.DeleteIndex(i);
      break;
    }
  }
  return true;
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csSpriteCal3DMeshObjectType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iConfig)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iLODControl)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpriteCal3DMeshObjectType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpriteCal3DMeshObjectType::csSpriteCal3DConfig)
  SCF_IMPLEMENTS_INTERFACE (iConfig)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpriteCal3DMeshObjectType::LODControl)
  SCF_IMPLEMENTS_INTERFACE (iLODControl)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csSpriteCal3DMeshObjectType)

csSpriteCal3DMeshObjectType::csSpriteCal3DMeshObjectType (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiConfig);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiLODControl);
}

csSpriteCal3DMeshObjectType::~csSpriteCal3DMeshObjectType ()
{
}

bool csSpriteCal3DMeshObjectType::Initialize (iObjectRegistry* object_reg)
{
  csSpriteCal3DMeshObjectType::object_reg = object_reg;
  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);
  csRef<iEngine> eng = CS_QUERY_REGISTRY (object_reg, iEngine);
  // We don't want to keep a reference to the engine (circular ref otherwise).
  engine = eng;
  return true;
}

csPtr<iMeshObjectFactory> csSpriteCal3DMeshObjectType::NewFactory ()
{
  csSpriteCal3DMeshObjectFactory* cm = new csSpriteCal3DMeshObjectFactory (this);
  cm->object_reg = object_reg;
  cm->vc = vc;
  cm->engine = engine;
#ifdef CS_USE_NEW_RENDERER
  cm->g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  cm->anon_buffers = new csAnonRenderBufferManager (object_reg);
#endif
  csRef<iMeshObjectFactory> ifact (
  	SCF_QUERY_INTERFACE (cm, iMeshObjectFactory));
  cm->DecRef ();
  return csPtr<iMeshObjectFactory> (ifact);
}
