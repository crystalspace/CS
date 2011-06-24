/*
  Copyright (C) 2011 Christian Van Brussel, Institute of Information
      and Communication Technologies, Electronics and Applied Mathematics
      at Universite catholique de Louvain, Belgium
      http://www.uclouvain.be/en-icteam.html

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

#ifndef __CS_CSTOOL_ANIMESHTOOLS_H__
#define __CS_CSTOOL_ANIMESHTOOLS_H__

/**\file
 * Tools for the generation of CS::Mesh::iAnimatedMesh's
 */

#include "ivaria/reporter.h"

#include "csutil/dirtyaccessarray.h"

struct iGeneralFactoryState;
struct iLoader;
struct iObjectRegistry;

namespace CS {
namespace Mesh {

struct iAnimatedMeshFactory;

/**
 * Helper class to generate new CS::Mesh::iAnimatedMesh's.
 */
class CS_CRYSTALSPACE_EXPORT AnimatedMeshTools
{
  static void ReportError (const char* msg, ...)
  {
#ifdef CS_DEBUG
    va_list arg;
    va_start (arg, msg);
    csReportV (iSCF::SCF->object_reg, CS_REPORTER_SEVERITY_ERROR,
	       "crystalspace.mesh.animeshtools", msg, arg);
    va_end (arg);
#endif
  }

  static void ReportWarning (const char* msg, ...)
  {
#ifdef CS_DEBUG
    va_list arg;
    va_start (arg, msg);
    csReportV (iSCF::SCF->object_reg, CS_REPORTER_SEVERITY_WARNING,
	       "crystalspace.mesh.animeshtools", msg, arg);
    va_end (arg);
#endif
  }

  static csPtr<iAnimatedMeshFactory> LoadAnimesh (iObjectRegistry* object_reg,
						  iLoader* loader,
						  const char* factoryName,
						  const char* filename);

 public:

  /**
   * Import a mesh with its morph targets split in several different
   * model files. This is useful eg to load directly models from the
   * FaceGen software.
   *
   * The system will scan the provided directory, and load all animesh
   * model files matching the given mask. All these animeshes will be
   * imported in the base animesh as a new morph target instead of a
   * new animesh.
   *
   * \param object_reg The object registry
   * \param path A VFS path to the directory containing all the data files
   * \param baseMesh The name of the file defining the base animesh where
   * all morph targets will be imported.
   * \param factoryName Name to be given to the newly created animesh
   * factory.
   * \param meshMask A regular expression to determine which files should
   * be imported as a new morph target. Use for example ".*[.]3[dD][sS]"
   * if you want all files terminating by ".3ds" or ".3DS". If you provide
   * an empty mask, then a new one will be built based on \p baseMesh.
   * \note The animesh models with a different number of vertices than the
   * base animesh won't be imported. These models are also supposed to have
   * the same vertices in the same order.
   */
  static csPtr<iAnimatedMeshFactory> ImportSplitMorphMesh
    (iObjectRegistry* object_reg, const char* path, const char* baseMesh,
     const char* factoryName, const char* meshMask = "");

  /**
   * Create a new morph target in an animesh by importing the vertices from
   * another animesh.
   * You must call Invalidate() on baseMesh after creation of the
   * morph targets.
   * \param object_reg The object registry
   * \param baseMesh The base animesh where the morph target will be imported.
   * \param morphMesh The animesh to import into the other as a new morph target.
   * \param morphName The name of the new morph target
   * \param deleteMesh Whether or not the animesh \a morphMesh should be
   * removed from the engine.
   *
   * \warning The two animeshes must have the same vertices in the same order.
   */
  static bool ImportMorphMesh
    (iObjectRegistry* object_reg, iAnimatedMeshFactory* baseMesh,
     iAnimatedMeshFactory* morphMesh, const char* morphName,
     bool deleteMesh = true);

  /**
   * Create a new animesh from a genmesh. It imports only the mesh part of the
   * genmesh (ie vertices, indices, etc), not the animations.
   *
   * \param object_reg The object registry
   * \param genmesh The genmesh to be imported
   * \param deleteMesh Whether or not the genmesh should be removed from the engine.
   */
  static csPtr<iAnimatedMeshFactory> ImportGeneralMesh
    (iObjectRegistry* object_reg, iGeneralFactoryState* genmesh,
     bool deleteMesh = true);

};

} //namespace Mesh
} //namespace CS

#endif // __CS_CSTOOL_ANIMESHTOOLS_H__
