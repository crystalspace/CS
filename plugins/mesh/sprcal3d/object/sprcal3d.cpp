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
#include "csgfx/renderbuffer.h"
#include "csutil/sysfunc.h"
#include "sprcal3d.h"
#include "csgeom/math.h"
#include "csgeom/polyclip.h"
#include "csgeom/quaterni.h"
#include "csgeom/sphere.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/randomgen.h"
#include "csutil/cfgacc.h"
#include "ivideo/graph3d.h"
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
#include "csutil/csendian.h"
#include "csqsqrt.h"

// STL include required by cal3d
#include <string>

CS_LEAKGUARD_IMPLEMENT (csCal3DMesh);
CS_LEAKGUARD_IMPLEMENT (csSpriteCal3DMeshObject);
CS_LEAKGUARD_IMPLEMENT (csSpriteCal3DMeshObjectFactory);

CS_IMPLEMENT_PLUGIN

static void ReportCalError (iObjectRegistry* objreg, const char* msgId, 
			    const char* msg)
{
  csString text;

  if (msg && (*msg != 0))
    text << msg << " [";
  text << "Cal3d: " << CalError::getLastErrorDescription().data();

  if (CalError::getLastErrorText ().size () > 0)
  {
    text << " '" << CalError::getLastErrorText().data() << "'";
  }

  text << " in " << CalError::getLastErrorFile().data() << "(" << 
    CalError::getLastErrorLine ();
  if (msg && (*msg != 0))
    text << "]";

  csReport (objreg, CS_REPORTER_SEVERITY_ERROR, msgId,
    text);
}

//--------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csSpriteCal3DSocket)
  SCF_IMPLEMENTS_INTERFACE (iSpriteCal3DSocket)
SCF_IMPLEMENT_IBASE_END

csSpriteCal3DSocket::csSpriteCal3DSocket()
{
  SCF_CONSTRUCT_IBASE (0);
  triangle_index = 0;
  submesh_index = 0;
  mesh_index = 0;
  name = 0;
  attached_mesh = 0;
}

csSpriteCal3DSocket::~csSpriteCal3DSocket ()
{
  delete [] name;
  SCF_DESTRUCT_IBASE ();
}

void csSpriteCal3DSocket::SetName (char const* n)
{
  delete [] name;
  if (n)
  {
    name = new char [strlen (n)+1];
    strcpy (name, n);
  }
  else
    name = 0;
}

void csSpriteCal3DSocket::SetMeshWrapper (iMeshWrapper* mesh)
{
  attached_mesh = mesh;
  csMatrix3 mat;
  mat.Identity();
  attached_mesh_trans = csReversibleTransform(mat, csVector3(0,0,0));
}

size_t csSpriteCal3DSocket::AttachSecondary (iMeshWrapper * mesh, csReversibleTransform trans)
{
  secondary_meshes.Push(csSpriteCal3DSocketMesh(mesh, trans));
  return secondary_meshes.Length()-1;
}

void csSpriteCal3DSocket::DetachSecondary (const csString & mesh_name)
{
  size_t a=FindSecondary(mesh_name);
  if (a < secondary_meshes.Length())
    secondary_meshes.DeleteIndex(a);
}

void csSpriteCal3DSocket::DetachSecondary (size_t index)
{
  secondary_meshes.DeleteIndex(index);
}

size_t csSpriteCal3DSocket::FindSecondary (const csString & mesh_name)
{
  for (size_t a=0; a<secondary_meshes.Length(); ++a)
  {
    if (secondary_meshes[a].mesh->QueryObject()->GetName() == mesh_name)
      return a;
  }
  return secondary_meshes.Length();
}

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
      scfCompatibleVersion(iVersion, scfInterface<iPolygonMesh>::GetVersion()))
    {
      printf ("Deprecated feature use: iPolygonMesh queried from Sprite3d "
        "factory; use iObjectModel->GetPolygonMeshColldet() instead.\n");
      iPolygonMesh* Object = scfiObjectModel.GetPolygonMeshColldet();
      (Object)->IncRef ();						
      return CS_STATIC_CAST(iPolygonMesh*, Object);				
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

csStringID csSpriteCal3DMeshObjectFactory::vertex_name = csInvalidStringID;
csStringID csSpriteCal3DMeshObjectFactory::texel_name = csInvalidStringID;
csStringID csSpriteCal3DMeshObjectFactory::normal_name = csInvalidStringID;
csStringID csSpriteCal3DMeshObjectFactory::color_name = csInvalidStringID;
csStringID csSpriteCal3DMeshObjectFactory::index_name = csInvalidStringID;

void csSpriteCal3DMeshObjectFactory::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.mesh.sprite.cal3d", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

csSpriteCal3DMeshObjectFactory::csSpriteCal3DMeshObjectFactory (
  iMeshObjectType* pParent, csSpriteCal3DMeshObjectType* type,
  iObjectRegistry* object_reg)
  : sprcal3d_type(pParent), sprcal3d_type2 (type), calCoreModel("no name")
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

  csSpriteCal3DMeshObjectFactory::object_reg = object_reg;

  if ((vertex_name == csInvalidStringID) ||
    (texel_name == csInvalidStringID) ||
    (normal_name == csInvalidStringID) ||
    (color_name == csInvalidStringID) ||
    (index_name == csInvalidStringID))
  {
    csRef<iStringSet> strings = 
      CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg, 
      "crystalspace.shared.stringset", iStringSet);
    vertex_name = strings->Request ("vertices");
    texel_name = strings->Request ("texture coordinates");
    normal_name = strings->Request ("normals");
    color_name = strings->Request ("colors");
    index_name = strings->Request ("indices");
  }

  light_mgr = CS_QUERY_REGISTRY (object_reg, iLightManager);
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

//  calCoreModel.destroy();

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiPolygonMesh);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiLODControl);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiSpriteCal3DFactoryState);
  SCF_DESTRUCT_IBASE ();
}

bool csSpriteCal3DMeshObjectFactory::Create(const char *name)
{  
  // return calCoreModel.create(name);
  return true;
}

void csSpriteCal3DMeshObjectFactory::ReportLastError ()
{
  ReportCalError (object_reg, "crystalspace.mesh.sprite.cal3d", 0);
}

void csSpriteCal3DMeshObjectFactory::SetLoadFlags(int flags)
{
  CalLoader::setLoadingMode(flags);
}

void csSpriteCal3DMeshObjectFactory::SetBasePath(const char *path)
{
  basePath = path;
}

void csSpriteCal3DMeshObjectFactory::RescaleFactory(float factor)
{
  calCoreModel.scale(factor);
  calCoreModel.getCoreSkeleton()->calculateBoundingBoxes(&calCoreModel);
}

bool csSpriteCal3DMeshObjectFactory::LoadCoreSkeleton (iVFS *vfs,
	const char *filename)
{
  csString path(basePath);
  path.Append(filename);
  csRef<iDataBuffer> file = vfs->ReadFile (path);
  if (file)
  {
    CalCoreSkeletonPtr skel = CalLoader::loadCoreSkeleton (
    	(void *)file->GetData() );
    if (skel)
    {
      calCoreModel.setCoreSkeleton (skel.get());
      return true;
    }
    else
      return false;
  }
  else
    return false;
}

int csSpriteCal3DMeshObjectFactory::LoadCoreAnimation (
	iVFS *vfs,const char *filename,
	const char *name,
	int type,
	float base_vel, float min_vel, float max_vel,
	int min_interval, int max_interval,
	int idle_pct, bool lock)
{
  csString path(basePath);
  path.Append(filename);
  csRef<iDataBuffer> file = vfs->ReadFile (path);
  if (file)
  {
    CalCoreAnimationPtr anim = CalLoader::loadCoreAnimation (
    	(void*)file->GetData(), calCoreModel.getCoreSkeleton() );
    if (anim)
    {
      int id = calCoreModel.addCoreAnimation(anim.get());
      if (id != -1)
      {
        csCal3DAnimation *an = new csCal3DAnimation;
        an->name          = name;
        an->type          = type;
        an->base_velocity = base_vel;
        an->min_velocity  = min_vel;
        an->max_velocity  = max_vel;
        an->min_interval  = min_interval;
        an->max_interval  = max_interval;
        an->idle_pct      = idle_pct;
        an->lock          = lock;

        an->index = (int)anims.Push(an);

        std::string str(name);
        calCoreModel.addAnimationName (str,id);
      }
      return id;
    }
    return -1;
  }
  return -1;
}

int csSpriteCal3DMeshObjectFactory::LoadCoreMesh (
	iVFS *vfs,const char *filename,
	const char *name,
	bool attach,
	iMaterialWrapper *defmat)
{
  csString path(basePath);
  path.Append(filename);
  csRef<iDataBuffer> file = vfs->ReadFile (path);
  if (file)
  {
    csCal3DMesh *mesh = new csCal3DMesh;
    CalCoreMeshPtr coremesh = CalLoader::loadCoreMesh((void*)file->GetData() );
    if (coremesh)
    {
      mesh->index = calCoreModel.addCoreMesh(coremesh.get());
      if (mesh->index == -1)
      {
        delete mesh;
        return false;
      }
      mesh->name              = name;
      mesh->attach_by_default = attach;
      mesh->default_material  = defmat;

      submeshes.Push(mesh);

      return mesh->index;
    }
    else
      return -1;
  }
  else
    return -1;
}

int csSpriteCal3DMeshObjectFactory::LoadCoreMorphTarget (
	iVFS *vfs,int mesh_index,
	const char *filename,
	const char *name)
{
  if (mesh_index < 0 || submeshes.Length() <= (size_t)mesh_index)
  {
    return -1;
  }

  csString path(basePath);
  path.Append(filename);
  csRef<iDataBuffer> file = vfs->ReadFile (path);
  if (file)
  {
    CalCoreMeshPtr core_mesh = CalLoader::loadCoreMesh((void *)file->GetData() );
    if(core_mesh.get() == 0)
      return -1;
    
    int morph_index = calCoreModel.getCoreMesh(mesh_index)->
    	addAsMorphTarget(core_mesh.get());
    if(morph_index == -1)
    {
      return -1;
    }
    submeshes[mesh_index]->morph_target_name.Push(name);
    return morph_index;
  }
  return -1;
}

void csSpriteCal3DMeshObjectFactory::CalculateAllBoneBoundingBoxes()
{
  // This function is SLOOOW.  Should only be called once after model is
  // fully loaded.
  calCoreModel.getCoreSkeleton()->calculateBoundingBoxes(&calCoreModel);
}

int csSpriteCal3DMeshObjectFactory::AddMorphAnimation(const char *name)
{
  int id = calCoreModel.addCoreMorphAnimation(new CalCoreMorphAnimation());
  morph_animation_names.Push(name);
  return id;
}

bool csSpriteCal3DMeshObjectFactory::AddMorphTarget(
	int morphanimation_index,
	const char *mesh_name, const char *morphtarget_name)
{
  int mesh_index = FindMeshName(mesh_name);
  if(mesh_index == -1)
  {
    return false;
  }
  csArray<csString>& morph_target = submeshes[mesh_index]->morph_target_name;
  size_t i;
  for (i=0; i<morph_target.Length(); i++)
  {
    if (morph_target[i] == morphtarget_name)
      break;
  }
  if(i==morph_target.Length())
  {
    return false;
  }
  CalCoreMorphAnimation* morph_animation = calCoreModel.getCoreMorphAnimation (
  	morphanimation_index);
  return morph_animation->addMorphTarget (mesh_index, (int)i);
}

int csSpriteCal3DMeshObjectFactory::GetMorphTargetCount(int mesh_id)
{
  if (mesh_id < 0|| submeshes.Length() <= (size_t)mesh_id)
  {
    return -1;
  }
  return (int)submeshes[mesh_id]->morph_target_name.Length();
}

const char *csSpriteCal3DMeshObjectFactory::GetMeshName(int idx)
{
  if ((size_t)idx >= submeshes.Length())
    return 0;

  return submeshes[idx]->name;
}

bool csSpriteCal3DMeshObjectFactory::IsMeshDefault(int idx)
{
  if ((size_t)idx >= submeshes.Length())
    return false;

  return submeshes[idx]->attach_by_default;
}

csSpriteCal3DSocket* csSpriteCal3DMeshObjectFactory::AddSocket ()
{
  csSpriteCal3DSocket* socket = new csSpriteCal3DSocket();
  sockets.Push (socket);
  return socket;
}

csSpriteCal3DSocket* csSpriteCal3DMeshObjectFactory::FindSocket (
	const char *n) const
{
  int i;
  for (i = GetSocketCount () - 1; i >= 0; i--)
    if (strcmp (GetSocket (i)->GetName (), n) == 0)
      return GetSocket (i);

  return 0;
}

csSpriteCal3DSocket* csSpriteCal3DMeshObjectFactory::FindSocket (
	iMeshWrapper *mesh) const
{
  int i;
  for (i = GetSocketCount () - 1; i >= 0; i--)
    if (GetSocket (i)->GetMeshWrapper() == mesh)
      return GetSocket (i);

  return 0;
}

int csSpriteCal3DMeshObjectFactory::FindMeshName (const char *meshName)
{
  for (size_t i=0; i<submeshes.Length(); i++)
  {
    if (submeshes[i]->name == meshName)
      return (int)i;
  }
  return -1;
}

const char* csSpriteCal3DMeshObjectFactory::GetDefaultMaterial (
	const char* meshName)
{
  int meshIndex = FindMeshName (meshName);
  if ( meshIndex != -1 )
  {
    if ( submeshes[meshIndex]->default_material )
    {
      return submeshes[meshIndex]->default_material->QueryObject()->GetName();
    }
  }
    
  return 0;                        
}

const char *csSpriteCal3DMeshObjectFactory::GetMorphAnimationName(int idx)
{
  if ((size_t)idx >= morph_animation_names.Length())
    return 0;

  return morph_animation_names[idx];
}

int csSpriteCal3DMeshObjectFactory::FindMorphAnimationName (
	const char *meshName)
{
  for (size_t i=0; i<morph_animation_names.Length(); i++)
  {
    if (morph_animation_names[i] == meshName)
      return (int)i;
  }
  return -1;
}


bool csSpriteCal3DMeshObjectFactory::AddCoreMaterial(iMaterialWrapper *mat)
{
  CalCoreMaterial *newmat = new CalCoreMaterial;
  CalCoreMaterial::Map newmap;
  newmap.userData = mat;

//  newmat->create();
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
  // NOTE: this is not the right way to do it, but this viewer can't do the
  // right mapping without further information on the model etc.
  for (materialId = 0 ; materialId < calCoreModel.getCoreMaterialCount()
  	; materialId++)
  {
    // create the a material thread
    calCoreModel.createCoreMaterialThread (materialId);

    // initialize the material thread
    calCoreModel.setCoreMaterialId (materialId, 0, materialId);
  }
}


csPtr<iMeshObject> csSpriteCal3DMeshObjectFactory::NewInstance ()
{
  csSpriteCal3DMeshObject* spr = new csSpriteCal3DMeshObject (0, 
    object_reg, calCoreModel);
  spr->SetFactory (this);
  spr->updateanim_sqdistance1 = sprcal3d_type2->updateanim_sqdistance1;
  spr->updateanim_skip1 = sprcal3d_type2->updateanim_skip1;
  spr->updateanim_sqdistance2 = sprcal3d_type2->updateanim_sqdistance2;
  spr->updateanim_skip2 = sprcal3d_type2->updateanim_skip2;
  spr->updateanim_sqdistance3 = sprcal3d_type2->updateanim_sqdistance3;
  spr->updateanim_skip3 = sprcal3d_type2->updateanim_skip3;

  csRef<iMeshObject> im (SCF_QUERY_INTERFACE (spr, iMeshObject));
  spr->DecRef ();
  return csPtr<iMeshObject> (im);
}

bool csSpriteCal3DMeshObjectFactory::RegisterAnimCallback(
    const char *anim, CalAnimationCallback *callback,float min_interval)
{
  for (size_t i=0; i<anims.Length(); i++)
  {
    if (anims[i]->name == anim)
    {
      CalCoreAnimation *cal_anim = calCoreModel.getCoreAnimation(anims[i]->index);
      cal_anim->registerCallback(callback,min_interval);
      return true;
    }
  }
  return false;
}

bool csSpriteCal3DMeshObjectFactory::RemoveAnimCallback(
    const char *anim, CalAnimationCallback *callback)
{
  for (size_t i=0; i<anims.Length(); i++)
  {
    if (anims[i]->name == anim)
    {
      CalCoreAnimation *cal_anim = calCoreModel.getCoreAnimation(anims[i]->index);
      cal_anim->removeCallback(callback);
      return true;
    }
  }
  return false;
}

void csSpriteCal3DMeshObjectFactory::HardTransform (
	const csReversibleTransform& t)
{
  csQuaternion quat (t.GetO2T ());
  CalQuaternion quatrot(quat.x,quat.y,quat.z,quat.r);
  csVector3 trans (t.GetOrigin () );
  CalVector translation (trans.x,trans.y,trans.z);

  // First we transform the skeleton, then we do the same to each animation.

  // get core skeleton
  CalCoreSkeleton *pCoreSkeleton;
  pCoreSkeleton = calCoreModel.getCoreSkeleton();
	
  // get core bone vector
  std::vector<CalCoreBone *>& vectorCoreBone = pCoreSkeleton
  	->getVectorCoreBone();

  // loop through all root core bones
  std::list<int>::iterator iteratorRootCoreBoneId;
  for (iteratorRootCoreBoneId = pCoreSkeleton->getListRootCoreBoneId().begin()
  	; iteratorRootCoreBoneId != pCoreSkeleton->getListRootCoreBoneId().end()
	; ++iteratorRootCoreBoneId)
  {
    CalCoreBone *bone = vectorCoreBone[*iteratorRootCoreBoneId];
    CalQuaternion bonerot = bone->getRotation();
    CalVector bonevec = bone->getTranslation();
    bonerot *= quatrot;
    bonevec *= quatrot;
    bonevec += translation;
    bone->setRotation(bonerot);
    bone->setTranslation(bonevec);
  }

  int i,count = calCoreModel.getCoreAnimationCount();
  for (i = 0; i < count; i++)
  {
    CalCoreAnimation *anim = calCoreModel.getCoreAnimation(i);
    if (!anim) continue;
    // loop through all root core bones
    std::list<int>::iterator iteratorRootCoreBoneId;
    for (iteratorRootCoreBoneId = pCoreSkeleton->getListRootCoreBoneId().begin()
    	; iteratorRootCoreBoneId != pCoreSkeleton->getListRootCoreBoneId().end()
	; ++iteratorRootCoreBoneId)
    {
      CalCoreTrack *track = anim->getCoreTrack(*iteratorRootCoreBoneId);
      if (!track) continue;
      for (int j=0; j<track->getCoreKeyframeCount(); j++)
      {
	CalCoreKeyframe *frame = track->getCoreKeyframe(j);
	CalQuaternion bonerot = frame->getRotation();
	CalVector bonevec = frame->getTranslation();
	bonerot *= quatrot;
	bonevec *= quatrot;
	bonevec += translation;
	frame->setRotation(bonerot);
	frame->setTranslation(bonevec);
      }
    }
  }
//  calCoreModel.getCoreSkeleton()->calculateBoundingBoxes(&calCoreModel);
}

//=============================================================================

SCF_IMPLEMENT_IBASE (csSpriteCal3DMeshObjectFactory::PolyMesh)
  SCF_IMPLEMENTS_INTERFACE (iPolygonMesh)
SCF_IMPLEMENT_IBASE_END

//=============================================================================

SCF_IMPLEMENT_IBASE (csSpriteCal3DMeshObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iLightingInfo)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iSpriteCal3DState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iLODControl)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpriteCal3DMeshObject::LightingInfo)
  SCF_IMPLEMENTS_INTERFACE (iLightingInfo)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpriteCal3DMeshObject::SpriteCal3DState)
  SCF_IMPLEMENTS_INTERFACE (iSpriteCal3DState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpriteCal3DMeshObject::PolyMesh)
  SCF_IMPLEMENTS_INTERFACE (iPolygonMesh)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpriteCal3DMeshObject::LODControl)
  SCF_IMPLEMENTS_INTERFACE (iLODControl)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpriteCal3DMeshObject::ObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END



csSpriteCal3DMeshObject::csSpriteCal3DMeshObject (iBase *pParent,
						  iObjectRegistry* object_reg,
						  CalCoreModel& calCoreModel)
                          : calModel(&calCoreModel)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiLightingInfo);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSpriteCal3DState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiLODControl);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPolygonMesh);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);

  csSpriteCal3DMeshObject::object_reg = object_reg;

  //  scfiPolygonMesh.SetFactory (this);
  //  scfiObjectModel.SetPolygonMeshBase (&scfiPolygonMesh);
  //  scfiObjectModel.SetPolygonMeshColldet (&scfiPolygonMesh);
  //  scfiObjectModel.SetPolygonMeshViscull (0);
  //  scfiObjectModel.SetPolygonMeshShadows (0);

  // create the model instance from the loaded core model
//  if(!calModel.create (&calCoreModel))
//  {
//    ReportCalError (object_reg, "crystalspace.mesh.sprite.cal3d", 
//      "Error creating model instance");
//    return;
//  }
  vertices_allocated = false;
  
  strings =  CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg, 
    "crystalspace.shared.stringset", iStringSet);
  G3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);

  // set the material set of the whole model
  vis_cb = 0;
  arrays_initialized = false;
  is_idling = false;
  meshes_colors = 0;
  is_initialized = 0;

  rmeshesSetup = false;

  meshVersion = 0;
  bboxVersion = (uint)-1;
  default_idle_anim = -1;
  last_locked_anim = -1;

  do_update = -1;
  updateanim_sqdistance1 = 10*10;
  updateanim_skip1 = 5;		// Skip every 5 frames.
  updateanim_sqdistance2 = 20*20;
  updateanim_skip2 = 20;	// Skip every 20 frames.
  updateanim_sqdistance3 = 50*50;
  updateanim_skip3 = 1000;	// Animate very rarely.
}

csSpriteCal3DMeshObject::~csSpriteCal3DMeshObject ()
{
  if (vertices_allocated)
  {
    size_t m;
    for (m=0;m<vertices.Length();m++)
    {
      size_t s; 
      for (s=0;s<vertices[m].Length();s++)
	delete[] vertices[m][s];
    }
  }
//  calModel.destroy();

  if (arrays_initialized)
  {
    delete [] is_initialized;
    delete [] meshes_colors;
  }

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiPolygonMesh);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiLODControl);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiSpriteCal3DState);
  SCF_DESTRUCT_IBASE ();
}


void csSpriteCal3DMeshObject::SetFactory (csSpriteCal3DMeshObjectFactory* tmpl)
{
  factory = tmpl;

  CalSkeleton *skeleton;
  CalBone *bone;
  skeleton = calModel.getSkeleton();
  std::vector < CalBone *> &bones = skeleton->getVectorBone();
  int i;
  for (i=0; i < (int)bones.size(); i++)
  {
    bone = bones[i];
    bone->calculateState ();
  }
  skeleton->calculateState ();

  // attach all default meshes to the model
  int meshId;
  for(meshId = 0; meshId < factory->GetMeshCount(); meshId++)
  {
    if (factory->submeshes[meshId]->attach_by_default)
    {
      AttachCoreMesh (factory->submeshes[meshId]->index,
	materialMapper.GetIdForMaterial (
	factory->submeshes[meshId]->default_material));
    }
  }
  // To get an accurate bbox below, you must have the model standing in
  // the default anim first
  calModel.getMixer()->blendCycle(0,1,0);
  calModel.update(0);
  last_update_time = csGetTicks();

  RecalcBoundingBox(object_bbox);
  calModel.getMixer()->clearCycle(0,0);

//  printf("Object bbox is (%1.2f, %1.2f, %1.2f) to (%1.2f, %1.2f, %1.2f)\n",
//         object_bbox.MinX(),object_bbox.MinY(),object_bbox.MinZ(),object_bbox.MaxX(),object_bbox.MaxY(),object_bbox.MaxZ());

  // Copy the sockets list down to the mesh
  iSpriteCal3DSocket *factory_socket,*new_socket;
  for (i=0; i<tmpl->GetSocketCount(); i++)
  {
    factory_socket = tmpl->GetSocket(i);
    new_socket = AddSocket();  // mesh now
    new_socket->SetName (factory_socket->GetName() );
    new_socket->SetTriangleIndex (factory_socket->GetTriangleIndex() );
    new_socket->SetSubmeshIndex (factory_socket->GetSubmeshIndex() );
    new_socket->SetMeshIndex (factory_socket->GetMeshIndex() );
    new_socket->SetMeshWrapper (0);
  }
}


void csSpriteCal3DMeshObject::GetRadius (csVector3& rad, csVector3& cent)
{
  cent.Set (object_bbox.GetCenter());
  csVector3 maxbox, minbox;

  RecalcBoundingBox (object_bbox);
  maxbox = object_bbox.Max();
  minbox = object_bbox.Min();
  float r1 = (maxbox.x-minbox.x)/2;
  float r2 = (maxbox.y-minbox.y)/2;
  float r3 = (maxbox.z-minbox.z)/2;
  rad.Set(r1,r2,r3);
}

#define CAL3D_EXACT_BOXES true 

void csSpriteCal3DMeshObject::RecalcBoundingBox (csBox3& bbox)
{
  if (bboxVersion == meshVersion)
    return;

  CalBoundingBox &calBoundingBox  = calModel.getBoundingBox(CAL3D_EXACT_BOXES);
  CalVector p[8];
  calBoundingBox.computePoints(p);

  bbox.Set(p[0].x, p[0].y, p[0].z, p[0].x, p[0].y, p[0].z);
  for (int i=1; i<8; i++)
  {
    bbox.AddBoundingVertexSmart(p[i].x,p[i].y,p[i].z);
  }

  bboxVersion = meshVersion;
//  printf("Bbox Width:%1.2f Height:%1.2f Depth:%1.2f\n",bbox.Max().x - bbox.Min().x,bbox.Max().y - bbox.Min().y,bbox.Max().z - bbox.Min().z);
}


void csSpriteCal3DMeshObject::GetObjectBoundingBox (csBox3& bbox)
{
  RecalcBoundingBox (object_bbox);
  bbox = object_bbox;
}

void csSpriteCal3DMeshObject::SetObjectBoundingBox (const csBox3& bbox)
{
  object_bbox = bbox;
  scfiObjectModel.ShapeChanged ();
}

void csSpriteCal3DMeshObjectFactory::GetRadius (csVector3& rad, csVector3& cent)
{
  cent.Set(0,0,0);
  rad.Set(1,1,1);
}

void csSpriteCal3DMeshObjectFactory::GetObjectBoundingBox (csBox3& bbox)
{
  CalCoreSkeleton *skel = calCoreModel.getCoreSkeleton ();
  skel->calculateBoundingBoxes(&calCoreModel);
  std::vector<CalCoreBone*> &vectorCoreBone = skel->getVectorCoreBone();
  CalBoundingBox &calBoundingBox  = vectorCoreBone[0]->getBoundingBox();
  CalVector p[8];
  calBoundingBox.computePoints(p);

  bbox.Set (p[0].x, p[0].y, p[0].z, p[0].x, p[0].y, p[0].z);
  for (int i=1; i<8; i++)
  {
    bbox.AddBoundingVertexSmart(p[i].x, p[i].y, p[i].z);
  }
}

void csSpriteCal3DMeshObjectFactory::SetObjectBoundingBox (const csBox3&)
{
  // @@@ TODO
}

void csSpriteCal3DMeshObject::GetObjectBoundingBox (csBox3& bbox,
	csVector3 *verts,int vertCount)
{
  if (object_bbox.Empty())
    RecalcBoundingBox (object_bbox);
  bbox = object_bbox;
}

void csSpriteCal3DMeshObject::LightChanged (iLight*)
{
  lighting_dirty = true;
}

void csSpriteCal3DMeshObject::LightDisconnect (iLight* light)
{
  lighting_dirty = true;
}

void csSpriteCal3DMeshObject::SetUserData(void *data)
{
  calModel.setUserData(data);
}

void csSpriteCal3DMeshObject::UpdateLighting (const csArray<iLight*>& lights,
					      iMovable* movable)
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
        UpdateLightingSubmesh (lights, movable, pCalRenderer,
            meshId, submeshId);
      }
    }
  }
  
  pCalRenderer->endRendering();
}


void csSpriteCal3DMeshObject::UpdateLightingSubmesh (
	const csArray<iLight*>& lights, 
	iMovable* movable,
	CalRenderer *pCalRenderer,
	int mesh, int submesh,float *have_normals)
{
  int vertCount;
  vertCount = pCalRenderer->getVertexCount();

  int i;

  // get the transformed normals of the submesh
  static float localNormals[60000];
  float *meshNormals;
  if (!have_normals)
  {
    pCalRenderer->getNormals(localNormals);
    meshNormals = localNormals;
  }
  else
  {
    meshNormals = have_normals;
  }

  // Do the lighting.
  csReversibleTransform trans = movable->GetFullTransform ();
  // the object center in world coordinates. "0" because the object
  // center in object space is obviously at (0,0,0).
  csColor color;

  size_t num_lights = lights.Length ();

  // Make sure colors array exists and set all to ambient
  InitSubmeshLighting (mesh, submesh, pCalRenderer, movable);
  csColor* colors = meshes_colors[mesh][submesh];

  // Update Lighting for all relevant lights
  for (size_t l = 0; l < num_lights; l++)
  {
    iLight* li = lights[l];
    // Compute light position in object coordinates
    // @@@ Can be optimized a bit. E.g. store obj_light_pos so it can be
    //  reused by submesh lighting.
    csVector3 wor_light_pos = li->GetCenter ();
    csVector3 obj_light_pos = trans.Other2This (wor_light_pos);
    float obj_sq_dist = csSquaredDist::PointPoint (obj_light_pos, 0);
    if (obj_sq_dist >= csSquare (li->GetCutoffDistance ())) return;
    float in_obj_dist = (obj_sq_dist >= SMALL_EPSILON)?
    	csQisqrt (obj_sq_dist):1.0f;

    csColor light_color = li->GetColor () * (256.0f / CS_NORMAL_LIGHT_LEVEL)
      * li->GetBrightnessAtDistance (csQsqrt (obj_sq_dist));

    int normal_index=0;
    for (i = 0; i < vertCount; i++)
    {
      csVector3 normal(meshNormals[normal_index],
      	meshNormals[normal_index+1],
	meshNormals[normal_index+2]);
      normal_index+=3;
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

void csSpriteCal3DMeshObject::InitSubmeshLighting (int mesh, int submesh,
						   CalRenderer *pCalRenderer,
						   iMovable* movable)
{
  csColor* colors = meshes_colors[mesh][submesh];

  int vertCount = pCalRenderer->getVertexCount();
  if (!colors)
  {
    meshes_colors[mesh][submesh] = new csColor[vertCount];
    colors = meshes_colors[mesh][submesh];
  }

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
  for (int i = 0 ; i < vertCount ; i++)
    colors[i] = col;
}

void csSpriteCal3DMeshObject::UpdateLighting (iMovable* movable, 
					      CalRenderer *pCalRenderer)
{
  if (!lighting_dirty)
    return;
  
  int meshCount;
  meshCount = pCalRenderer->getMeshCount();
  
  if (factory->light_mgr)
  {
    const csArray<iLight*>& relevant_lights = factory->light_mgr
      ->GetRelevantLights (logparent, -1, false);
    
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
            UpdateLightingSubmesh (relevant_lights,
              movable, pCalRenderer, meshId, submeshId);
        }
      }
    }
  }
  
  lighting_dirty = false;
}

void csSpriteCal3DMeshObject::SetupObjectSubmesh(int index)
{
  if (!arrays_initialized)
    return;  // First draw call to SetupObject will take care of most of this.

  if (!calModel.getMesh(index))
    return;

  int submeshes = calModel.getMesh(index)->getSubmeshCount();
  renderMeshes[index].SetLength (submeshes);

  for (int j=0; j<submeshes; j++)
  {
    bool flag = false;
    is_initialized[index].Push(flag);
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
    is_initialized = new csArray<bool> [ meshCount ];
    meshes_colors  = new csArray<csColor*> [ meshCount ];
    renderMeshes.SetLength (meshCount);
    
    for (int index=0; index<meshCount; index++)
    {
      SetupObjectSubmesh (index);
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

//	factory->GetObjectBoundingBox(object_bbox);  // initialize object_bbox here
      }
    }
  }
  RecalcBoundingBox(object_bbox);
}

void csSpriteCal3DMeshObject::SetupVertices()
{
  if(!vertices_allocated)
  {
    vertices.SetLength (attached_ids.Length());
    size_t m;
    for (m = 0; m < vertices.Length(); m++)
    {
      CalMesh* mesh = calModel.getMesh ((int)m);
      csArray<csVector3*>& vM = vertices[m];
      vM.SetLength (mesh->getSubmeshCount());
      size_t s;
      for (s = 0; s < vertices[m].Length(); s++)
      {
	CalSubmesh* submesh = mesh->getSubmesh ((int)s);
	csVector3*& vSM = vertices[m][s];
	vSM = new csVector3[submesh->getVertexCount ()];
	int v;
	for(v = 0; v < submesh->getVertexCount (); v++)
	{
	  vSM[v].Set (0, 0, 0);
	}
      }
    }
    vertices_allocated = true;
    vertices_dirty = false;
  }
  if(vertices_dirty)
  {
    size_t m;
    for (m = 0; m < vertices.Length(); m++)
    {
      csArray<csVector3*>& vM = vertices[m];
      size_t s;
      for (s = 0; s < vertices[m].Length(); s++)
      {
	csVector3*& vSM = vM[s];
	int v;
	for(v = 0; 
	  v < calModel.getMesh((int)m)->getSubmesh((int)s)->getVertexCount ();
	  v++)
	{
	  vSM[v].Set (0, 0, 0);
	}
      }
    }
    vertices_dirty = false;
  }
}

bool csSpriteCal3DMeshObject::HitBeamOutline (const csVector3& start,
	const csVector3& end, csVector3& isect, float* pr)
{
  SetupVertices();
 
  //Checks all of the cal3d bounding boxes of each bone to see if they hit

  bool hit = false;
  std::vector<CalBone *> vectorBone = calModel.getSkeleton()->getVectorBone();
  csArray<bool> bboxhits;
  bboxhits.SetLength(vectorBone.size());
  int b = 0;
  std::vector<CalBone *>::iterator iteratorBone = vectorBone.begin();
  while (iteratorBone != vectorBone.end())
  {
    CalBoundingBox& cbb = (*iteratorBone)->getBoundingBox();
    int i;
    csPlane3 planes[6];
    for(i=0;i<6;i++)
    {
      planes[i].Set(cbb.plane[i].a,
	  cbb.plane[i].b,
	  cbb.plane[i].c,
	  cbb.plane[i].d);
    }
    csVector3
      tempisect;
    float
      tempdist;
    if(csIntersect3::Planes(start,end,planes,6,tempisect,tempdist))
    {
      hit = true;
      bboxhits[b] = true;
    }
    else
    {
      bboxhits[b] = false;
    }
    ++iteratorBone;
    b++;
  }

  if(hit)
  {
    csSegment3 seg (start, end);
    float dist, temp, max;
    temp = dist = max = csSquaredDist::PointPoint (start, end);
    csVector3 tsect;
    size_t m;
    for (m = 0; m < vertices.Length(); m++)
    {
      size_t s;
      for (s = 0; s < vertices[m].Length(); s++)
      {
	std::vector<CalCoreSubmesh::Face>& vectorFace
		= calModel.getMesh((int)m)->getSubmesh((int)s)
		->getCoreSubmesh()->getVectorFace();
	std::vector<CalCoreSubmesh::Face>::iterator iteratorFace
	  = vectorFace.begin();
	while(iteratorFace != vectorFace.end())
	{
	  bool bboxhit = false;
	  int f;
	  for(f=0;!bboxhit&&f<3;f++)
	  {
	    std::vector<CalCoreSubmesh::Influence> influ
	    	= calModel.getMesh((int)m)->getSubmesh((int)s)
		->getCoreSubmesh()->getVectorVertex ()
	      [(*iteratorFace).vertexId[f]].vectorInfluence;
	    std::vector<CalCoreSubmesh::Influence>::iterator iteratorInflu =
	      influ.begin();
	    while(iteratorInflu != influ.end())
	    {
	      if(bboxhits[(*iteratorInflu).boneId])
	      {
		bboxhit = true;
		break;
	      }
	      ++iteratorInflu;
	    }
	  }
	  if (bboxhit)
	  {
	    for(f=0;f<3;f++)
	    {
	      //Did we allready calculate this vertex?
	      if(vertices[m][s][(*iteratorFace).vertexId[f]].x == 0 &&
	         vertices[m][s][(*iteratorFace).vertexId[f]].y == 0 &&
		 vertices[m][s][(*iteratorFace).vertexId[f]].z == 0)
	      {
		CalVector calVector = calModel.getPhysique()->calculateVertex(
		    calModel.getMesh((int)m)->getSubmesh((int)s),
		    (*iteratorFace).vertexId[f]);
		vertices[m][s][(*iteratorFace).vertexId[f]].x = calVector.x;
		vertices[m][s][(*iteratorFace).vertexId[f]].y = calVector.y;
		vertices[m][s][(*iteratorFace).vertexId[f]].z = calVector.z;
	      }
	    }
	    if (csIntersect3::IntersectTriangle (
		  vertices[m][s][(*iteratorFace).vertexId[0]],
		  vertices[m][s][(*iteratorFace).vertexId[1]],
		  vertices[m][s][(*iteratorFace).vertexId[2]], seg, tsect))
	    {
	      isect = tsect;
	      if (pr) *pr = csQsqrt (csSquaredDist::PointPoint (start, isect) /
		  csSquaredDist::PointPoint (start, end));
	      return true;
	    }
	  }
	  ++iteratorFace;
	}
      }
    }
  }
  return false;
}


bool csSpriteCal3DMeshObject::HitBeamObject (const csVector3& start,
    const csVector3& end, csVector3& isect, float* pr, int* polygon_idx)
{
  SetupVertices();

  //Checks all of the cal3d bounding boxes of each bone to see if they hit

  bool hit = false;
  std::vector<CalBone *> vectorBone = calModel.getSkeleton()->getVectorBone();
  csArray<bool> bboxhits;
  bboxhits.SetLength(vectorBone.size());
  int b = 0;
  std::vector<CalBone *>::iterator iteratorBone = vectorBone.begin();
  while (iteratorBone != vectorBone.end())
  {
    CalBoundingBox& cbb = (*iteratorBone)->getBoundingBox();
    int i;
    csPlane3 planes[6];
    for(i=0;i<6;i++)
    {
      planes[i].Set(cbb.plane[i].a,
	  cbb.plane[i].b,
	  cbb.plane[i].c,
	  cbb.plane[i].d);
    }
    csVector3
      tempisect;
    float
      tempdist;
    if(csIntersect3::Planes(start,end,planes,6,tempisect,tempdist))
    { 
      hit = true;
      bboxhits[b] = true;
    }
    else
    {
      bboxhits[b] = false;
    }
    ++iteratorBone;
    b++;
  }

  if(hit)
  {
    // This routine is slow, but it is intended to be accurate.
    if (polygon_idx) *polygon_idx = -1;

    csSegment3 seg (start, end);
    float dist, temp, max;
    temp = dist = max = csSquaredDist::PointPoint (start, end);
    csVector3 tsect;

    size_t m;
    for (m = 0; m < vertices.Length(); m++)
    {
      size_t s;
      for (s = 0; s < vertices[m].Length(); s++)
      {
	std::vector<CalCoreSubmesh::Face>& vectorFace
		= calModel.getMesh((int)m)->getSubmesh((int)s)
		->getCoreSubmesh()->getVectorFace();
	std::vector<CalCoreSubmesh::Face>::iterator iteratorFace
	  = vectorFace.begin();
	while(iteratorFace != vectorFace.end())
	{
	  bool bboxhit = false;
	  int f;
	  for(f=0;!bboxhit&&f<3;f++)
	  {
	    std::vector<CalCoreSubmesh::Influence> influ
	    	= calModel.getMesh((int)m)->getSubmesh((int)s)
		->getCoreSubmesh()->getVectorVertex ()
	      [(*iteratorFace).vertexId[f]].vectorInfluence;
	    std::vector<CalCoreSubmesh::Influence>::iterator iteratorInflu =
	      influ.begin();
	    while(iteratorInflu != influ.end())
	    {
	      if(bboxhits[(*iteratorInflu).boneId])
	      {
		bboxhit = true;
		break;
	      }
	      ++iteratorInflu;
	    }
	  }
	  if (bboxhit)
	  {
	    for(f=0;f<3;f++)
	    {
	      //Is the vertex calculated?
	      if(vertices[m][s][(*iteratorFace).vertexId[f]].x == 0 &&
		  vertices[m][s][(*iteratorFace).vertexId[f]].y == 0 &&
		  vertices[m][s][(*iteratorFace).vertexId[f]].z == 0)
	      {
		CalVector calVector = calModel.getPhysique()->calculateVertex(
		    calModel.getMesh((int)m)->getSubmesh((int)s),
		    (*iteratorFace).vertexId[f]);
		vertices[m][s][(*iteratorFace).vertexId[f]].x = calVector.x;
		vertices[m][s][(*iteratorFace).vertexId[f]].y = calVector.y;
		vertices[m][s][(*iteratorFace).vertexId[f]].z = calVector.z;
	      }
	    }

	    if (csIntersect3::IntersectTriangle (
		  vertices[m][s][(*iteratorFace).vertexId[0]], 
		  vertices[m][s][(*iteratorFace).vertexId[1]],
		  vertices[m][s][(*iteratorFace).vertexId[2]], seg, tsect))
	    {
	      temp = csSquaredDist::PointPoint (start, tsect);
	      if (temp < dist)
	      {
		dist = temp;
		isect = tsect;
	      }
	    }
	  }
	  ++iteratorFace;
	}
      }
    }
    if (pr) *pr = csQsqrt (dist / max);
    if (dist >= max) return false;
    return true;
  }
  else
  {
    return false;
  }
}

void csSpriteCal3DMeshObject::PositionChild (iMeshObject* child,
	csTicks current_time)
{
  iSpriteCal3DSocket* socket = 0;
  size_t i;
  for ( i = 0; i < sockets.Length(); i++)
  {
    if(sockets[i]->GetMeshWrapper())
    {
      if (sockets[i]->GetMeshWrapper()->GetMeshObject() == child)
      {
	socket = sockets[i];
	break;
      }
    }
    for (size_t a=0; a<sockets[i]->GetSecondaryCount(); ++a)
    {
      if (sockets[i]->GetSecondaryMesh(a)->GetMeshObject() == child)
      {
        socket = sockets[i];
        break;
      }
    }
  }
  
  if (socket)
  {
    Advance(current_time);
    int m = socket->GetMeshIndex();
    int s = socket->GetSubmeshIndex();
    int f = socket->GetTriangleIndex();
    
    // Get the submesh
    CalSubmesh* submesh = calModel.getMesh(m)->getSubmesh(s);
    // Get the core submesh
    CalCoreSubmesh* coresubmesh = calModel.getCoreModel()->
      getCoreMesh(m)->getCoreSubmesh(s);
    // get the triangle
    CalCoreSubmesh::Face face = coresubmesh->getVectorFace()[f];
    // get the vertices
    CalVector vector[3];
    vector[0] = calModel.getPhysique()->calculateVertex(
	submesh,
	face.vertexId[0]);
    vector[1] = calModel.getPhysique()->calculateVertex(
	submesh,
	face.vertexId[1]);
    vector[2] = calModel.getPhysique()->calculateVertex(
	submesh,
	face.vertexId[2]);
    csVector3 vert1(vector[0].x,vector[0].y,vector[0].z);
    csVector3 vert2(vector[1].x,vector[1].y,vector[1].z);
    csVector3 vert3(vector[2].x,vector[2].y,vector[2].z);

    csVector3 center= (vert1+vert2+vert3)/3;

    csVector3 bc = vert3 - vert2;
        
    csVector3 up = vert1-center;
    up.Normalize();
    
    csVector3 normal = bc % up;
    normal.Normalize();

    csReversibleTransform trans; //= movable->GetFullTransform();
    trans.SetOrigin(center);
    trans.LookAt(normal, up);

    if (socket->GetMeshWrapper())
    {
      iMovable* movable = socket->GetMeshWrapper()->GetMovable();
      movable->SetTransform(socket->GetTransform()*trans);
      movable->UpdateMove();
    }
    
    // update secondary meshes
    for (size_t a=0; a<socket->GetSecondaryCount(); ++a)
    {
      iMeshWrapper * sec_mesh = socket->GetSecondaryMesh(a);
      if (sec_mesh)
      {
        iMovable * sec_movable = sec_mesh->GetMovable();
        if (sec_movable)
        {
            sec_movable->SetTransform(socket->GetSecondaryTransform(a)*trans);
            sec_movable->UpdateMove();
        }
      }
    }
  }
}

csRenderMesh** csSpriteCal3DMeshObject::GetRenderMeshes (int &n, 
	iRenderView* rview,
	iMovable* movable, uint32 frustum_mask)
{
  SetupObject ();

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

  int clip_portal, clip_plane, clip_z_plane;
  rview->CalculateClipSettings (frustum_mask, clip_portal, clip_plane,
  	clip_z_plane);
  csVector3 camera_origin = tr_o2c.GetT2OTranslation ();

  // Distance between camera and object. Use this for LOD.
  float sqdist = camera_origin.x * camera_origin.x
  	+ camera_origin.y * camera_origin.y
  	+ camera_origin.z * camera_origin.z;
  if (sqdist < updateanim_sqdistance1) do_update = -1;
  else if (sqdist < updateanim_sqdistance2)
  {
    if (do_update == -1 || do_update > updateanim_skip1)
      do_update = updateanim_skip1;
  }
  else if (sqdist < updateanim_sqdistance3)
  {
    if (do_update == -1 || do_update > updateanim_skip2)
      do_update = updateanim_skip2;
  }
  else
  {
    if (do_update == -1 || do_update > updateanim_skip3)
      do_update = updateanim_skip3;
  }

  SetupRenderMeshes ();
  csDirtyAccessArray<csRenderMesh*>& meshes = 
    rmHolder.GetUnusedMeshes (rview->GetCurrentFrameNumber ());

  for (size_t m = 0; m < allRenderMeshes.Length(); m++)
  {
    csRenderMesh* rm;
    if (m >= meshes.Length())
    {
      rm = new csRenderMesh (*allRenderMeshes[m]);
      meshes.Push (rm);
    }
    else
    {
      rm = meshes[m];
      // Not yet initialized.
      if (rm->indexend == 0)
        *rm = *allRenderMeshes[m];
    }

    rm->clip_portal = clip_portal;
    rm->clip_plane = clip_plane;
    rm->clip_z_plane = clip_z_plane;
    rm->do_mirror = camera->IsMirrored ();
    rm->object2camera = tr_o2c;
    rm->camera_origin = camera_origin;
    rm->camera_transform = &camera->GetTransform();
    rm->geometryInstance = this;
  }
  currentMovable = movable;
  // @@@ One movable for all meshes... not good.

  n = (int)meshes.Length();
  return meshes.GetArray();

  /*n = allRenderMeshes.Length();
  return allRenderMeshes.GetArray();*/
}

void csSpriteCal3DMeshObject::SetupRenderMeshes ()
{
  if (rmeshesSetup)
    return;

  for (size_t m = 0; m < renderMeshes.Length(); m++)
  {
    for (size_t s = 0; s < renderMeshes[m].Length(); s++)
    {
      CalSubmesh* submesh = calModel.getMesh ((int)m)->getSubmesh ((int)s);
      csRenderMesh& rm = renderMeshes[m][s];
      rm.indexstart = 0;
      rm.indexend = submesh->getFaceCount () * 3;
      rm.meshtype = CS_MESHTYPE_TRIANGLES;
      rm.material = materialMapper.GetMaterialForId (submesh->getCoreMaterialId ());
      rm.buffers.AttachNew (new csRenderBufferHolder );

      csRef<iRenderBufferAccessor> accessor (
	csPtr<iRenderBufferAccessor> (new BaseAccessor (this, (int)m, (int)s)));
      rm.buffers->SetAccessor (accessor, CS_BUFFER_ALL_MASK);

      allRenderMeshes.Push (&rm);
    }
  }

  rmeshesSetup = true;
}

bool csSpriteCal3DMeshObject::Advance (csTicks current_time)
{
  if (do_update != -1)
  {
    if (do_update >= 0)
      do_update--;
    if (do_update >= 0)
      return true;
  }

  // update anim frames, etc. here
  float delta = ((float)current_time - last_update_time)/1000.0F;
  if (!current_time)
    delta = 0;
  calModel.update(delta);
  if (current_time)
    last_update_time = current_time;

  if (is_idling) // check for override and play if time
  {
    idle_override_interval -= delta;
    if (idle_override_interval <= 0)
    {
      SetAnimAction(factory->anims[idle_action]->name,.25,.25);
      idle_override_interval = 20;
    }
  }
  meshVersion++;
  lighting_dirty = true;
  vertices_dirty = true;
  return true;
}

//--------------------------------------------------------------------------

csMeshedPolygon* csSpriteCal3DMeshObject::PolyMesh::GetPolygons ()
{
  return 0;
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
    ClearAnimCyclePos ((int)(active_anims.Length() - 1), 0);

  if (last_locked_anim != -1)
  {
     calModel.getMixer()->removeAction(last_locked_anim);
     last_locked_anim = -1;
     is_idling = false;
  }
}

bool csSpriteCal3DMeshObject::SetAnimCycle(const char *name, float weight)
{
  ClearAllAnims();
  return AddAnimCycle(name, weight, 0);
}

bool csSpriteCal3DMeshObject::SetAnimCycle(int idx, float weight)
{
  ClearAllAnims();
  return AddAnimCycle(idx, weight, 0);
}

bool csSpriteCal3DMeshObject::AddAnimCycle(const char *name, float weight,
	float delay)
{
  int idx = FindAnim(name);
  if (idx == -1)
    return false;

  AddAnimCycle(idx,weight,delay);

  Advance(0);
  return true;
}

bool csSpriteCal3DMeshObject::AddAnimCycle(int idx, float weight, float delay)
{
  calModel.getMixer()->blendCycle(idx,weight,delay);

  ActiveAnim const a = { factory->anims[idx], weight };
  active_anims.Push(a);
  return true;
}

int csSpriteCal3DMeshObject::FindAnimCyclePos(int idx) const
{
  for (size_t i = active_anims.Length(); i-- > 0; )
    if (active_anims[i].anim->index == idx)
      return (int)i;
  return -1;
}

int csSpriteCal3DMeshObject::FindAnimCycleNamePos(char const* name) const
{
  for (size_t i = active_anims.Length(); i-- > 0; )
    if (active_anims[i].anim->name == name)
      return (int)i;
  return -1;
}

void csSpriteCal3DMeshObject::ClearAnimCyclePos(int pos, float delay)
{
  calModel.getMixer()->clearCycle(active_anims[pos].anim->index,delay);
  // We do not 'delete' active_anims[pos].anim because it is owned by factory.
  active_anims.DeleteIndex(pos);
}

bool csSpriteCal3DMeshObject::ClearAnimCycle (int idx, float delay)
{
  int const pos = FindAnimCyclePos (idx);
  bool const ok = (pos != -1);
  if (ok)
    ClearAnimCyclePos (pos, delay);
  return ok;
}

bool csSpriteCal3DMeshObject::ClearAnimCycle (const char *name, float delay)
{
  int const pos = FindAnimCycleNamePos (name);
  bool const ok = (pos != -1);
  if (ok)
    ClearAnimCyclePos (pos, delay);
  return ok;
}

int csSpriteCal3DMeshObject::GetActiveAnimCount()
{
  return (int)active_anims.Length();
}

int csSpriteCal3DMeshObject::GetActiveAnims(char *buffer,int max_length)
{
  int count=0;
  size_t i, n;

  for (i=0, n=active_anims.Length(); i<n && count<max_length-1; i++)
  {
    ActiveAnim const& a = active_anims[i];
    buffer[count++] = a.anim->index;
    // will not work for % weights, but only for weights >= 1.
    buffer[count++] = (char)a.weight;
  }
  return i == n;  // true if successful
}

void csSpriteCal3DMeshObject::SetActiveAnims(const char *buffer,int anim_count)
{
  ClearAllAnims();

  for (int i=0; i<anim_count; i++)
  {
    AddAnimCycle(buffer[i*2],buffer[i*2+1],0);
  }
}

bool csSpriteCal3DMeshObject::SetAnimAction(int idx, float delayIn,
	                                        float delayOut)
{
  if (idx < 0 || (size_t)idx >=factory->anims.Length() )
    return false;

  calModel.getMixer()->executeAction(idx,delayIn,delayOut,
                                     1,
                                     factory->anims[idx]->lock  );

  if (factory->anims[idx]->lock)
  {
     last_locked_anim = idx;
     is_idling = false;
  }

  return true;
}

bool csSpriteCal3DMeshObject::SetAnimAction(const char *name, float delayIn,
	float delayOut)
{ 
  int idx = FindAnim(name);
  if (idx == -1)
    return false;

  return SetAnimAction(idx,delayIn, delayOut);
}

void csSpriteCal3DMeshObject::SetIdleOverrides(csRandomGen *rng,int which)
{
  csCal3DAnimation *anim = factory->anims[which];

  is_idling = true;

  // Determine interval till next override.
  idle_override_interval = rng->Get(anim->max_interval - anim->min_interval)
  	+ anim->min_interval;

  // Determine which idle override will be played.
  int odds = rng->Get(100);
  idle_action = 0;
  for (int i=0; i<GetAnimCount(); i++)
  {
    if (factory->anims[i]->idle_pct > odds)
    {
      idle_action = i;
      return;
    }
    else
    {
      odds -= factory->anims[i]->idle_pct;
    }
  }
}

void csSpriteCal3DMeshObject::SetDefaultIdleAnim(const char *name)
{
    default_idle_anim = FindAnim(name);
}

bool csSpriteCal3DMeshObject::SetVelocity(float vel,csRandomGen *rng)
{
  int count = GetAnimCount();
  int i;
  ClearAllAnims();
  if (!vel)
  {
    if (default_idle_anim != -1)
    {
      AddAnimCycle(default_idle_anim,1,0);
      if (rng)
        SetIdleOverrides (rng,default_idle_anim);
      return true;
    }
    for (i=0; i<count; i++)
    {
      if (factory->anims[i]->type == iSpriteCal3DState::C3D_ANIM_TYPE_IDLE)
      {
        AddAnimCycle(i,1,0);
	if (rng)
	  SetIdleOverrides(rng,i);
        return true;
      }
    }
  }

  bool reversed = (vel<0);
  if (reversed)
  {
    vel = -vel;
    SetTimeFactor(-1);
  }
  else
  {
    SetTimeFactor(1);
  }

  is_idling = false;
  // first look for animations with a base velocity that exactly matches
  bool found_match = false;
  for (i=0; i<count; i++)
  {
    if (factory->anims[i]->type == iSpriteCal3DState::C3D_ANIM_TYPE_TRAVEL)
    {
      if (vel < factory->anims[i]->min_velocity ||
          vel > factory->anims[i]->max_velocity)
    	continue;
      if (vel == factory->anims[i]->base_velocity)
      {
        AddAnimCycle(i,1,0);
        found_match = true;
      }
    }
  }
  if (found_match)
    return true;
   
  /* since no exact matches were found, look for animations with a min to max velocity range that
      this velocity falls in and blend those animations based on how close thier base velocity matches */
  for (i=0; i<count; i++)
  {
    float pct,vel_diff;
    if (factory->anims[i]->type == iSpriteCal3DState::C3D_ANIM_TYPE_TRAVEL)
    {
      if (vel < factory->anims[i]->min_velocity ||
          vel > factory->anims[i]->max_velocity)
    	continue;
      if (vel < factory->anims[i]->base_velocity)
      {
        vel_diff = 
            factory->anims[i]->base_velocity - factory->anims[i]->min_velocity;
        pct = (vel - factory->anims[i]->min_velocity) / vel_diff;
      }
      else
      {
        vel_diff = 
            factory->anims[i]->max_velocity - factory->anims[i]->base_velocity;
        pct = (factory->anims[i]->max_velocity - vel) / vel_diff;
      }
      AddAnimCycle(i,pct,0);
//     printf("  Adding %s weight=%1.2f\n",factory->anims[i]->name.GetData(),pct);
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

  return AttachCoreMesh (factory->submeshes[idx]->index,
    materialMapper.GetIdForMaterial (
    factory->submeshes[idx]->default_material));
}

bool csSpriteCal3DMeshObject::AttachCoreMesh(int mesh_id,int iMatWrapID)
{
  if ( attached_ids.Find( mesh_id ) != csArrayItemNotFound )
    return true;
 
  if (!calModel.attachMesh(mesh_id))
    return false;

  // Since all our vertex buffers and G3DTriangles are in arrays
  // which line up with the attached submesh list on the calModel,
  // we need to track these id's in the identical way so that
  // we can remove the right G3DTriangles when the submesh is
  // removed.

  attached_ids.Push(mesh_id);
  SetupObjectSubmesh ((int)(attached_ids.Length()-1));
  CalMesh *mesh = calModel.getMesh(mesh_id);
  for (int i=0; i<mesh->getSubmeshCount(); i++)
  {
    mesh->getSubmesh(i)->setCoreMaterialId(iMatWrapID);
  }

  // Make sure space is allocated for new vertices in SetupVertices()
  vertices_allocated = false;

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
  return true;
}

bool csSpriteCal3DMeshObject::BlendMorphTarget(int morph_animation_id,
	float weight, float delay)
{
  if(morph_animation_id < 0||
  	factory->morph_animation_names.Length() <= (size_t)morph_animation_id)
  {
    return false;
  }
  return calModel.getMorphTargetMixer()->blend(morph_animation_id,weight,delay);
}

bool csSpriteCal3DMeshObject::ClearMorphTarget(int morph_animation_id,
	float delay)
{
  if(morph_animation_id < 0||
  	factory->morph_animation_names.Length() <= (size_t)morph_animation_id)
  {
    return false;
  }
  return calModel.getMorphTargetMixer()->clear(morph_animation_id,delay);
}

csSpriteCal3DSocket* csSpriteCal3DMeshObject::AddSocket ()
{
  csSpriteCal3DSocket* socket = new csSpriteCal3DSocket();
  sockets.Push (socket);
  return socket;
}

csSpriteCal3DSocket* csSpriteCal3DMeshObject::FindSocket (const char *n) const
{
  int i;
  for (i = GetSocketCount () - 1; i >= 0; i--)
    if (strcmp (GetSocket (i)->GetName (), n) == 0)
      return GetSocket (i);

  return 0;
}

csSpriteCal3DSocket* csSpriteCal3DMeshObject::FindSocket (
	iMeshWrapper *mesh) const
{
  int i;
  for (i = GetSocketCount () - 1; i >= 0; i--)
  {
    if (GetSocket (i)->GetMeshWrapper() == mesh)
      return GetSocket (i);
    for (size_t a=0; a<GetSocket (i)->GetSecondaryCount(); ++a)
    {
      if (GetSocket (i)->GetSecondaryMesh(a) == mesh)
        return GetSocket (i);  
    }
  }

  return 0;
}

bool csSpriteCal3DMeshObject::SetMaterial(const char *mesh_name,
	iMaterialWrapper *mat)
{
  int idx = factory->FindMeshName(mesh_name);
  if (idx == -1)
    return false;
          
  size_t i;
  int j;
  for (i=0; i<attached_ids.Length(); i++)
  {
    if (attached_ids[i] == idx)
    {
      CalMesh *mesh = calModel.getMesh(attached_ids[i]);
      for (j=0; j<mesh->getSubmeshCount(); j++)
      {
	mesh->getSubmesh(j)->setCoreMaterialId (
	  materialMapper.GetIdForMaterial (mat));
        if (arrays_initialized)
        {
          csRenderMesh *rm = &renderMeshes[attached_ids[i]][j];
          rm->material = mat;
	}
      }
      rmHolder.Clear();
      return true;
    }
  }
  return false;
}

void csSpriteCal3DMeshObject::SetTimeFactor(float timeFactor)
{
  calModel.getMixer()->setTimeFactor(timeFactor);
}

float csSpriteCal3DMeshObject::GetTimeFactor()
{
  return calModel.getMixer()->getTimeFactor();
}


//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csSpriteCal3DMeshObject::BaseAccessor)
SCF_IMPLEMENT_IBASE_END

void csSpriteCal3DMeshObject::BaseAccessor::PreGetBuffer 
  (csRenderBufferHolder* holder, csRenderBufferName buffer)
{
  if (!holder) return;
  if ((buffer == CS_BUFFER_INDEX) || 
      (buffer == CS_BUFFER_POSITION) ||
      (buffer == CS_BUFFER_TEXCOORD0) ||
      (buffer == CS_BUFFER_NORMAL) ||
      (buffer == CS_BUFFER_COLOR))
  {
    if (meshobj->meshVersion != meshVersion)
    {
      CalRenderer* render = meshobj->calModel.getRenderer();
      render->beginRendering();
      render->selectMeshSubmesh (mesh, submesh);

      int vertexCount = render->getVertexCount ();
      int indexCount = render->getFaceCount() * 3;
      if ((index_buffer == 0) || (index_size < indexCount))
      {
	// @@@ How often do those change?
	index_buffer = csRenderBuffer::CreateIndexRenderBuffer (
	  indexCount, CS_BUF_DYNAMIC,
	  CS_BUFCOMP_UNSIGNED_INT, 0, vertexCount - 1);
	index_size = indexCount;
      }
      uint* indices = (uint*)index_buffer->Lock (CS_BUF_LOCK_NORMAL);
      render->getFaces ((CalIndex*)indices);
      index_buffer->Release ();

      if ((vertex_buffer == 0) || (vertex_size < vertexCount))
      {
	vertex_buffer = csRenderBuffer::CreateRenderBuffer (
	  vertexCount, CS_BUF_DYNAMIC,
	  CS_BUFCOMP_FLOAT, 3);
	normal_buffer = csRenderBuffer::CreateRenderBuffer (
	  vertexCount, CS_BUF_DYNAMIC,
	  CS_BUFCOMP_FLOAT, 3);
	texel_buffer = csRenderBuffer::CreateRenderBuffer (
	  vertexCount, CS_BUF_DYNAMIC,
	  CS_BUFCOMP_FLOAT, 2);
	color_buffer = csRenderBuffer::CreateRenderBuffer (
	  vertexCount, CS_BUF_DYNAMIC,
	  CS_BUFCOMP_FLOAT, 3);
	vertex_size = vertexCount;
      }
      float* vertices = (float*)vertex_buffer->Lock (CS_BUF_LOCK_NORMAL);
      render->getVertices (vertices);
      vertex_buffer->Release ();

      CS_ALLOC_STACK_ARRAY(float, normals, vertexCount * 3);
      render->getNormals (normals);

      const csArray<iLight*>& relevant_lights = meshobj->factory->light_mgr
              ->GetRelevantLights (meshobj->logparent, -1, false);

      meshobj->UpdateLightingSubmesh (relevant_lights, 
                                      meshobj->currentMovable,
                                      render,
                                      mesh,
                                      submesh,
                                      normals);

      normal_buffer->CopyInto (normals, vertexCount);

      float* texels = (float*)texel_buffer->Lock (CS_BUF_LOCK_NORMAL);
      render->getTextureCoordinates (0, texels);
      texel_buffer->Release ();

      color_buffer->CopyInto (meshobj->meshes_colors[mesh][submesh],
	                                vertexCount);

      render->endRendering();

      meshVersion = meshobj->meshVersion;
    }

    if (buffer == CS_BUFFER_INDEX)
    {
      holder->SetRenderBuffer (CS_BUFFER_INDEX, index_buffer);   
    }
    else if (buffer == CS_BUFFER_POSITION)
    {
      holder->SetRenderBuffer (CS_BUFFER_POSITION, vertex_buffer);
    }
    else if (buffer == CS_BUFFER_NORMAL)
    {
      holder->SetRenderBuffer (CS_BUFFER_NORMAL, normal_buffer);
    }
    else if (buffer == CS_BUFFER_TEXCOORD0)
    {
      holder->SetRenderBuffer (CS_BUFFER_TEXCOORD0, texel_buffer);
    }
    else if (buffer == CS_BUFFER_COLOR)
    {
      holder->SetRenderBuffer (CS_BUFFER_COLOR, color_buffer);
    }
  }
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
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiLODControl);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiConfig);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSpriteCal3DMeshObjectType::Initialize (iObjectRegistry* object_reg)
{
  csSpriteCal3DMeshObjectType::object_reg = object_reg;
  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);

  csConfigAccess cfg (object_reg, "/config/sprcal3d.cfg");

  updateanim_sqdistance1 = cfg->GetFloat (
  	"Mesh.SpriteCal3D.DistanceThresshold1", 10.0f);
  updateanim_sqdistance1 *= updateanim_sqdistance1;
  updateanim_skip1 = cfg->GetInt ("Mesh.SpriteCal3D.SkipFrames1", 4);

  updateanim_sqdistance2 = cfg->GetFloat (
  	"Mesh.SpriteCal3D.DistanceThresshold2", 20.0f);
  updateanim_sqdistance2 *= updateanim_sqdistance2;
  updateanim_skip2 = cfg->GetInt ("Mesh.SpriteCal3D.SkipFrames2", 20);

  updateanim_sqdistance3 = cfg->GetFloat (
  	"Mesh.SpriteCal3D.DistanceThresshold3", 50.0f);
  updateanim_sqdistance3 *= updateanim_sqdistance3;
  updateanim_skip3 = cfg->GetInt ("Mesh.SpriteCal3D.SkipFrames3", 1000);

  return true;
}

csPtr<iMeshObjectFactory> csSpriteCal3DMeshObjectType::NewFactory ()
{
  csSpriteCal3DMeshObjectFactory* cm = new csSpriteCal3DMeshObjectFactory (
    this, this, object_reg);
  cm->vc = vc;
  cm->engine = engine;
  cm->g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  //cm->anon_buffers = new csAnonRenderBufferManager (object_reg);
  csRef<iMeshObjectFactory> ifact (
    SCF_QUERY_INTERFACE (cm, iMeshObjectFactory));
  cm->DecRef ();
  return csPtr<iMeshObjectFactory> (ifact);
}
