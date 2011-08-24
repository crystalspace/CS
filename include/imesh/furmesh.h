/*
  Copyright (C) 2010 Alexandru - Teodor Voicu
      Faculty of Automatic Control and Computer Science of the "Politehnica"
      University of Bucharest
      http://csite.cs.pub.ro/index.php/en/

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

#ifndef __FUR_INTERF_H__
#define __FUR_INTERF_H__

#include "csutil/scf.h"
#include "ivideo/rendermesh.h"

class csVector3;
struct iRigidBody;
struct iSector;
struct iTextureWrapper;
struct iView;

namespace CS {
namespace Physics {
namespace Bullet {
struct iDynamicSystem;
} // namespace Bullet
} // namespace Physics
} // namespace CS


/**\file
 * Fur mesh interface files
 */

namespace CS
{
  namespace Mesh
  {
    struct iAnimatedMesh;
    struct iAnimatedMeshFactory;
    struct iAnimatedMeshSubMeshFactory;
  } // namespace Mesh
  
namespace Animation {

/**
 * Simple Animation Controller
 */
struct iFurAnimationControl : public virtual iBase
{
public:
  SCF_INTERFACE (CS::Animation::iFurAnimationControl, 1, 0, 0);

  /**
   * Initialize the fur strand with the given ID
   * \param strandID unique ID for the fur strand
   * \param coordinates C/C++ array of Vector3 representing the initial 
   * positions of the control points
   * \param coordinatesCount the number of control points 
   */
  virtual void InitializeStrand (size_t strandID, csVector3* coordinates,
    size_t coordinatesCount) = 0;

  /**
   * Animate the fur strand with the given ID
   * \param strandID unique ID for the fur strand
   * \param coordinates C/C++ array of Vector3 representing the initial 
   * positions of the control points
   * \param coordinatesCount the number of control points 
   */
  virtual void AnimateStrand (size_t strandID, csVector3* coordinates, size_t
    coordinatesCount) const = 0;

  /**
   * Remove the fur strand with the given ID
   * \param strandID unique ID for the fur strand
   */
  virtual void RemoveStrand (size_t strandID) = 0;

  /**
   * Remove all fur strands
   */
  virtual void RemoveAllStrands () = 0;
};

/**
 * Controller that updates the iFurMesh's geometry
 */
struct iFurPhysicsControl : public virtual iFurAnimationControl
{
public:
  SCF_INTERFACE (CS::Animation::iFurPhysicsControl, 1, 0, 0);

  /**
   * Set the animesh on which the iFurMesh is attached
   */
  virtual void SetAnimatedMesh (CS::Mesh::iAnimatedMesh* animesh) = 0;

  /**
   * Set the rigid body on which the iFurMesh is attached
   */
  virtual void SetRigidBody (iRigidBody* rigidBody) = 0;

  /**
   * Set the iDynamicSystem (optional)
   */
  virtual void SetBulletDynamicSystem (CS::Physics::Bullet::iDynamicSystem* 
    bulletDynamicSystem) = 0;
};

/**
 * Animation controller for animated mesh
 */
struct iFurAnimatedMeshControl : public virtual iFurAnimationControl
{
public:
  SCF_INTERFACE (CS::Animation::iFurAnimatedMeshControl, 1, 0, 0);

  /**
   * Set the animesh on which the iFurMesh is attached
   */
  virtual void SetAnimatedMesh (CS::Mesh::iAnimatedMesh* animesh) = 0;

  /**
   * Set displacement between fur and base mesh
   */
  virtual void SetDisplacement (float displacement) = 0;
};

} // namespace Animation
} // namespace CS

namespace CS {
namespace Mesh {

struct iFurMeshMaterialProperties;
struct iFurMeshFactory;
struct iFurMeshType;
struct iFurMesh;

/**
 * Access to the properties used for the iFurMesh.
 */
struct iFurMeshState : public virtual iBase
{
public:
  SCF_INTERFACE (CS::Mesh::iFurMeshState, 1, 0, 0);

  /**
   * Get the width of a strand
   */
  virtual float GetStrandWidth () const = 0;

  /**
   * Set the width of a strand
   */
  virtual void SetStrandWidth (float strandWidth) = 0;

  /**
   * Get the displacement between the fur mesh and the base mesh
   */
  virtual float GetDisplacement () const = 0;

  /**
   * Set the displacement between the fur mesh and the base mesh
   */
  virtual void SetDisplacement (float displacement) = 0;

  /**
   * Get the density map texture
   */
  virtual iTextureWrapper* GetDensityMap () const = 0;

  /**
   * Set the density map texture
   */
  virtual void SetDensityMap (iTextureWrapper* densityMap) = 0;

  /**
   * Get the density factor for guide furs
   */
  virtual float GetDensityFactorGuideFurs () const = 0;

  /**
   * Set the density factor for guide furs
   */
  virtual void SetDensityFactorGuideFurs (float densityFactorGuideFurs) = 0;

  /**
   * Get the density factor for fur strands
   */
  virtual float GetDensityFactorFurStrands () const = 0;

  /**
   * Set the density factor for fur strands
   */
  virtual void SetDensityFactorFurStrands (float densityFactorFurStrands) = 0;

  /**
   * Get the heightmap map texture
   */
  virtual iTextureWrapper* GetHeightMap () const = 0;

  /**
   * Set the heightmap map texture
   */
  virtual void SetHeightMap (iTextureWrapper* heightMap) = 0;

  /**
   * Get the height factor (for guide furs)
   */
  virtual float GetHeightFactor () const = 0;

  /**
   * Set the height factor (for guide furs)
   */
  virtual void SetHeightFactor (float heightFactor) = 0;

  /**
   * Get the average number of control points per fur strand
   */
  virtual uint GetAverageControlPointsCount () const = 0;

  /**
   * Set the average number of control points per fur strand
   */
  virtual void SetAverageControlPointsCount (uint averageControlPointsCount) = 0;

  /**
   * Get the distance between control points on a fur
   */
  virtual float GetControlPointsDistance () const = 0;

  /**
   * Set the distance between control points on a fur
   */
  virtual void SetControlPointsDistance (float controlPointsDistance) = 0;

  /**
   * Get the fur strand thickness variation
   */
  virtual float GetThicknessVariation () const = 0;

  /**
   * Set the fur strand thickness variation
   */
  virtual void SetThicknessVariation (float thicknessVariation) = 0;

  /**
   * Get the pointiness of a fur strand
   */
  virtual float GetPointiness () const = 0;

  /**
   * Set the pointiness of a fur strand
   */
  virtual void SetPointiness (float pointiness) = 0;

  /**
   * Get the fur strand position deviation
   */
  virtual float GetFurStrandDeviation () const = 0;

  /**
   * Set the fur strand position deviation
   */
  virtual void SetFurStrandDeviation (float furStrandDeviation) = 0;

  /**
   * Get the control points position deviation
   */
  virtual float GetControlPointsDeviation () const = 0;

  /**
   * Set the control points position deviation
   */
  virtual void SetControlPointsDeviation (float positionDeviation) = 0;

  /**
   * Check if fur grows based on tangent direction
   */
  virtual bool GetGrowTangent () const = 0;

  /**
   * Set if fur grows based on tangent direction
   */
  virtual void SetGrowTangent (bool growTangent) = 0;

  /**
   * Check if we grow small fur (tangents are reversed with normals)
   */
  virtual bool GetSmallFur () const = 0;

  /**
   * Set if fur grows based on tangent direction
   */
  virtual void SetSmallFur (bool smallFur) = 0;

  /**
   * Get mixmode
   */
  virtual uint GetMixmode () const = 0;

  /**
   * Set mixmode
   */
  virtual void SetMixmode (uint mode) = 0;

  /**
   * Get render priority
   */
  virtual CS::Graphics::RenderPriority GetRenderPriority () const = 0;

  /**
   * Set render priority
   */
  virtual void SetRenderPriority (CS::Graphics::RenderPriority priority) = 0;

  /**
   * Get Z-buffer
   */
  virtual csZBufMode GetZBufMode () const = 0;

  /**
   * Set Z-buffer
   */
  virtual void SetZBufMode (csZBufMode z_buf_mode) = 0;
};

/**
 * Store the material used for the iFurMesh.
 * Material variables can be updated each frame via the Update function.
 */
struct iFurMeshMaterialProperties : public virtual iBase
{
public:
  SCF_INTERFACE (CS::Mesh::iFurMeshMaterialProperties, 1, 0, 0);

  /**
   * Get the material used
   */
  virtual iMaterial* GetMaterial () const = 0;

  /**
   * Set the material used. Can be created externally from an XML
   */
  virtual void SetMaterial (iMaterial* material) = 0;

  /**
   * Update the material data after modifying the material variables
   */
  virtual void Invalidate () = 0;

  /**
   * Called each frame. New material variables values can be send to the shader
   */
  virtual void Update () = 0;
};

/**\addtogroup meshplugins
 * @{ */

/**\name Fur mesh
 * @{ */

/**
 * State of a fur mesh object factory
 */
struct iFurMeshFactory : public virtual iBase
{
public:
  SCF_INTERFACE (CS::Mesh::iFurMeshFactory, 1, 0, 0);
};

/**
 * This plugin describes a specific type of fur mesh objects.
 * All methods are inherited from iMeshObjectType
 */
struct iFurMeshType : public virtual iBase
{
public:
  SCF_INTERFACE (CS::Mesh::iFurMeshType, 1, 0, 0);

  /**
   * Create a FurMeshBasicProperties using a cons char * as unique ID.
   * This iFurMeshMaterialProperties only defines set and get material
   */
  virtual iFurMeshMaterialProperties* 
    CreateFurMeshBasicProperties (const char* name) = 0;

  /**
   * Create a HairMeshMarschnerProperties using a cons char * as unique ID
   */
  virtual iFurMeshMaterialProperties* 
    CreateHairMeshMarschnerProperties (const char* name) = 0;

  /**
   * Find iFurMeshMaterialProperties with ID name or return 0 otherwise.
   */
  virtual iFurMeshMaterialProperties* 
    FindFurMeshMaterialProperites (const char* name) const = 0;

  /**
   * Remove iFurMeshMaterialProperties with ID name if exists.
   */
  virtual void RemoveFurMeshMaterialProperites (const char* name) = 0;

  /**
   * Remove all iFurMeshMaterialProperties.
   */
  virtual void ClearFurMeshMaterialProperites () = 0;

  /**
   * Create a CS::Animation::FurPhysicsControl using a cons char * as unique ID
   */
  virtual CS::Animation::iFurAnimationControl* CreateFurPhysicsControl
    (const char* name) = 0;

  /**
   * Create a FurAnimatedMeshControl using a cons char * as unique ID
   */
  virtual CS::Animation::iFurAnimationControl* CreateFurAnimatedMeshControl
    (const char* name) = 0;

  /**
   * Find iFurAnimationControl with ID name or return 0 otherwise.
   */
  virtual CS::Animation::iFurAnimationControl* FindFurAnimationControl
    (const char* name) const = 0;

  /**
   * Remove iFurAnimationControl with ID name if exists.
   */
  virtual void RemoveFurAnimationControl (const char* name) = 0;

  /**
   * Remove all iFurAnimationControls.
   */
  virtual void ClearFurAnimationControls () = 0;
};

/**
 * State and setting for an instance of a fur mesh
 */
struct iFurMesh : public virtual iBase  
{
  SCF_INTERFACE (CS::Mesh::iFurMesh, 1, 0, 0);

  /**
   * Generates the geometry for the current instance of a fur mesh.
   * The associated iFurMeshFactory is created by this function
   */
  virtual void GenerateGeometry (iView* view, iSector* room) = 0;

  /**
   * Set the LOD for the guide fur. 
   * Pure guide fur is updated via the associated iFurPhysicsControl
   */
  virtual void SetGuideLOD (float guideLOD) = 0; 

  /**
   * Set the LOD for the fur strands. 
   * Fur strands are the rendered geometry.
   */
  virtual void SetStrandLOD (float strandLOD) = 0;

  /**
   * Set the LOD for the control points. 
   * It only has three level from 0 to 1.
   */
  virtual void SetControlPointsLOD(float controlPointsLOD) = 0;

  /**
   * Set the overall LOD. Equivalent to calling SetGuidLOD and SetStrandLOD 
   * with the same parameter.
   */
  virtual void SetLOD (float lod) = 0;

  /**
   * Set the animesh
   */
  virtual void SetAnimatedMesh (CS::Mesh::iAnimatedMesh* animesh) = 0;

  /**
   * Set the associated CS::Animation::iFurAnimationControl
   */
  virtual void SetAnimationControl (CS::Animation::iFurAnimationControl* physicsControl) = 0;
  
  /**
   * Start the associated CS::Animation::iFurAnimationControl. 
   * Pure guide furs will be synchronized with the
   * CS::Animation::iFurAnimationControl every frame
   */
  virtual void StartAnimationControl () = 0;

  /**
   * Stop the associated iFurAnimationControl. 
   * Pure guide furs will stop being synchronized with the
   * CS::Animation::iFurAnimationControl
   */
  virtual void StopAnimationControl () = 0;
 
  /**
   * Enable the fur mesh (by default the fur mesh is enabled)
   */
  virtual void EnableMesh () = 0;

  /**
   * Reset the position of the mesh on the base mesh.
   * Pure guide furs will stop and start being synchronized with
   * CS::Animation::iFurAnimationControl
   */
  virtual void ResetMesh () = 0;

  /**
   * Disable the fur mesh (used for small fur when camera is at a certain distance)
   */
  virtual void DisableMesh () = 0;

  /**
   * Set the associated iFurMeshMaterialProperties
   */
  virtual void SetFurMeshProperties (iFurMeshMaterialProperties* furMeshProperties) = 0;

  /**
   * Get the associated iFurMeshMaterialProperties.
   * Shader variables can be obtained via the material of the iFurMeshMaterialProperties
   */
  virtual iFurMeshMaterialProperties* GetFurMeshProperties () const = 0;

  /**
   * Set an iAnimatedMeshFactory corresponding to the iAnimatedMeshSubMeshFactory
   */
  virtual void SetMeshFactory (CS::Mesh::iAnimatedMeshFactory* meshFactory) = 0;

  /**
   * Set iAnimatedMeshSubMeshFactory on which fur will grow
   */
  virtual void SetMeshFactorySubMesh (CS::Mesh::iAnimatedMeshSubMeshFactory* 
    meshFactorySubMesh) = 0;
};


/** @} */

/** @} */

} // namespace Mesh
} // namespace CS

#endif // __FUR_INTERF_H__
