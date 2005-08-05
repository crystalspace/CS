/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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

#ifndef __CS_CSTOOL_IMPORTKIT_H__
#define __CS_CSTOOL_IMPORTKIT_H__

/**\file
 * Crystal Space Import Kit
 */

#include "cssysdef.h"
#include "csextern.h"
#include "csutil/array.h"

struct iObjectRegistry;

namespace CrystalSpace
{
  /**
   * \internal Namespace for the import kit implementation, to "shield"
   *  use of CSism from the import kit headers.
   */
  namespace ImportKitImpl 
  { 
    class Glue;
    struct GluedModel;
  }

  /**
   * Crystal Space Import Kit. A class wraps access to loader plugins
   * and mesh factories to allow simple access to mesh data (albeit the
   * returned data is limited compared to what the engine supports).
   */
  class CS_CRYSTALSPACE_EXPORT ImportKit
  {
    ImportKitImpl::Glue* glue;
  public:
    /**
     * Contains multiple models.
     */
    class CS_CRYSTALSPACE_EXPORT Container
    {
    public:
      /**
       * A model. A model contains of multiple meshes, each containing
       * vertices, triangles, and a single material.
       * \remarks All resources obtainable (models, mesh, materials...
       *  but also geometry and names) are generally valid as long as the
       *  container exists.
       */
      class CS_CRYSTALSPACE_EXPORT Model
      {
      public:
	/// Return the name of the model.
	const wchar_t* GetName () const { return name; }

	/// Model mesh, contains actual geometry.
	class CS_CRYSTALSPACE_EXPORT Mesh
	{
	protected:
	  friend class Model;
	  friend class ImportKitImpl::Glue;

	  unsigned int vertexCount;
	  float* verts;
	  float* texcoords;
	  float* normals;
	  size_t triCount;
	  unsigned int* tris;
	  size_t material;
	public:
	  /// Return numver of vertices.
	  unsigned int GetVertexCount () const { return vertexCount; }
	  /**
	   * Return vertices. The returned buffer contains 3 entries for
	   * x,y,z of each vertex.
	   */
	  const float* GetVertices () const { return verts; }
	  /**
	   * Return texture coordinates. The returned buffer contains 2 
	   * entries for u,v of each vertex.
	   */
	  const float* GetTexCoords () const { return texcoords; }
	  /**
	   * Return normals. The returned buffer contains 3 entries for
	   * nx,ny,nz of each vertex.
	   */
	  const float* GetNormals () const { return normals; }
	
	  /// Get the number of triangles.
	  size_t GetTriangleCount () const { return triCount; }
	  /**
	   * Get triangle. The returned buffer contains the indices into
	   * the vertex etc. arrays for each corner.
	   */
	  const unsigned int* GetTriangles () const { return tris; }

	  /** 
	   * Mesh material. Returns an index that can be used with
	   * Container::GetMaterial().
	   */
	  size_t GetMaterial () const { return material; }
	};
	/// Get number of meshes in this model.
	size_t GetMeshCount () const { return meshes.Length(); }
	/// Get a mesh.
	const Mesh& GetMesh (size_t index) const { return meshes[index]; }

	~Model();
	Model (const Model& other);
      protected:
	friend class Container;
	friend class ImportKitImpl::Glue;
	wchar_t* name;
	ImportKitImpl::GluedModel* glueModel;
	csArray<Mesh> meshes;

	Model () {}
      };
      /// Return number of models.
      size_t GetModelCount () const { return models.Length(); }
      /// Get a model.
      const Model& GetModel (size_t index) const { return models[index]; }
    
      /**
       * A material for a mesh.
       * \remarks While CrystalSpace distinguishes materials and textures,
       *  the import kit does not - a material corresponds to a texture.
       */
      class CS_CRYSTALSPACE_EXPORT Material
      {
      protected:
	friend class Container;
	friend class ImportKitImpl::Glue;
	wchar_t* name;
	char* texture;

	Material() {}
      public:
	/// Get name of the material.
	const wchar_t* GetName () const { return name; }
	/**
	 * Get texture filename of the material.
	 */
	const char* GetTextureFile () const { return texture; }

	~Material();
	Material (const Material& other);
      };
      /// Get number of materials.
      size_t GetMaterialCount () { return materials.Length(); }
      /// Get a material.
      const Material& GetMaterial (size_t index) { return materials[index]; }
    protected:
      friend class ImportKit;
      friend class ImportKitImpl::Glue;
      csArray<Model> models;
      csArray<Material> materials;
    };

    /// Initialize this kit.
    ImportKit (iObjectRegistry* objectReg);
    ~ImportKit ();
    /**
     * Open a CrystalSpace container from \a filename (which can
     * optionally be in the path relative to the current directory in
     * \a path. Note that the path can contain up to 1 zip file).
     * \a filename should point to a mesh library, meshfact file or
     * world file; it is detected whether a file contains sensible data,
     * so you can e.g. safely call this method for all files of a directory.
     */
    Container* OpenContainer (const char* filename, 
      const char* path = 0);
  };
  
} // namespace CrystalSpace

#endif // __CS_CSTOOL_IMPORTKIT_H__
