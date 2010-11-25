/*
  Copyright (C) 2010 by Mike Gist and Claudiu Mihail

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_RENDERMANAGER_OCCLUVIS_H__
#define __CS_RENDERMANAGER_OCCLUVIS_H__

#include "csgeom/aabbtree.h"
#include "csutil/hash.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"
#include "iengine/movable.h"
#include "iengine/sector.h"
#include "iengine/viscull.h"
#include "imesh/objmodel.h"

struct iEngine;
struct iGraphics3D;
struct iMeshWrapper;
struct iObjectRegistry;

namespace CS
{
  namespace RenderManager
  {
    // Frustum culling visibility results.
    enum NodeVisibility
    {
      NODE_INVISIBLE,
      NODE_VISIBLE,
      NODE_INSIDE
    };

    // Occlusion visibility results.
    enum OcclusionVisibility
    {
      VISIBLE,
      UNKNOWN,
      INVISIBLE,
      INVALID
    };

    typedef csArray<iVisibilityObject*, csArrayElementHandler<iVisibilityObject*>,
    CS::Container::ArrayAllocDefault, csArrayCapacityFixedGrow<256> >
    VistestObjectsArray;

    struct IntersectSegmentFront2BackData
    {
      csSegment3 seg;
      csVector3 isect;
      float sqdist;		// Squared distance between seg.start and isect.
      float r;
      iMeshWrapper* mesh;
      int polygon_idx;
      VistestObjectsArray* vector;	// If not-null we need all objects.
      bool accurate;
    };

    struct QueryData : public csRefCount
    {
      unsigned int uOQuery;
      uint32 uQueryFrame;
      uint32 uNextCheck;
      OcclusionVisibility eResult;

      QueryData ()
        : uOQuery (0), uQueryFrame (0),
          uNextCheck (0), eResult (INVALID)
      {}
    };

    struct AABBTreeNodeVisibilityData
    {
      AABBTreeNodeVisibilityData ()
      {
        g3d = 0;
      }

      ~AABBTreeNodeVisibilityData ()
      {
        if (g3d)
        {
          FreeQueryData ();
        }
      }

      void LeafAddObject (iVisibilityObject*)
      {
        RViewQueryHash::GlobalIterator itr = RViewQueries.GetIterator ();
        while (itr.HasNext ())
        {
          itr.Next ()->eResult = INVALID;
        }
      }

      void LeafUpdateObjects (iVisibilityObject**, uint)
      {
        RViewQueryHash::GlobalIterator itr = RViewQueries.GetIterator ();
        while (itr.HasNext ())
        {
          itr.Next ()->eResult = INVALID;
        }
      }

      void NodeUpdate (const AABBTreeNodeVisibilityData& child1,
        const AABBTreeNodeVisibilityData& child2)
      {
        RViewQueryHash::GlobalIterator itr = RViewQueries.GetIterator ();
        while (itr.HasNext ())
        {
          itr.Next ()->eResult = INVALID;
        }
      }

      QueryData* GetQueryData (iGraphics3D* ig3d, iRenderView* rview)
      {
        csRef<QueryData> queryData = RViewQueries.Get (csPtrKey<iRenderView> (rview), csRef<QueryData> ());

        if (!queryData.IsValid ())
        {
          g3d = ig3d;
          queryData.AttachNew (new QueryData);
          g3d->OQInitQueries (&queryData->uOQuery, 1);
          RViewQueries.Put (csPtrKey<iRenderView> (rview), queryData);
        }

        return queryData;
      }

      void FreeQueryData ()
      {
        RViewQueryHash::GlobalIterator itr = RViewQueries.GetIterator ();
        while (itr.HasNext ())
        {
          g3d->OQDelQueries (&itr.Next ()->uOQuery, 1);
        }

        RViewQueries.DeleteAll ();
      }

    private:
      iGraphics3D* g3d;

      typedef csHash<csRef<QueryData>, csPtrKey<iRenderView> > RViewQueryHash;
      RViewQueryHash RViewQueries;
    };

    typedef CS::Geometry::AABBTree<iVisibilityObject, 1, AABBTreeNodeVisibilityData> AABBVisTree;
    typedef AABBVisTree::Node AABBVisTreeNode;

    class CS_CRYSTALSPACE_EXPORT csOccluvis : public AABBVisTree,
      public scfImplementation1<csOccluvis, iVisibilityCuller>
    {
    private:
      iObjectRegistry *object_reg;
      csRef<iGraphics3D> g3d;
      csRef<iEngine> engine;
      csRef<iShaderManager> shaderMgr;
      csRef<iStringSet> stringSet;
      csRef<iShaderVarStringSet> svStrings;

      // Structure of common f2b data.
      struct Front2BackData
      {
        csVector3 pos;
        iRenderView* rview;
        csPlane3* frustum;
        iVisibilityCullerListener* viscallback;
      };

      struct NodeMeshList : public csRefCount
      {
        NodeMeshList () : meshList (nullptr)
        {
        }

        ~NodeMeshList ()
        {
          delete[] meshList;
        }

        int numMeshes;
        uint framePassed;
        csBitArray onlyTestZ;
        bool alwaysVisible;
        AABBVisTreeNode* node;
        csSectorVisibleRenderMeshes* meshList;
      };

      class csVisibilityObjectWrapper
        : public scfImplementation2<csVisibilityObjectWrapper, iMovableListener, iObjectModelListener>
      {
      public:
        csVisibilityObjectWrapper (csOccluvis* culler, iVisibilityObject* vis_obj);

        /// The object model has changed.
        virtual void ObjectModelChanged (iObjectModel* model);

        /// The movable has changed.
        virtual void MovableChanged (iMovable* movable);

        /// The movable is about to be destroyed.
        virtual void MovableDestroyed (iMovable*);

        inline iVisibilityObject* GetVisObject () const
        {
          return vis_obj;
        }

      private:
        csOccluvis* culler;
        iVisibilityObject* vis_obj;
        csBox3 oldBBox;
      };

      // Array of visibility objects.
      csRefArray<csVisibilityObjectWrapper> visObjects;

      // Set to true to pass all objects regardless of visibility next VisCull.
      bool bAllVisible;

      // Frame skip parameter
      static const unsigned int visibilityFrameSkip = 10;

      // Hash of mesh nodes for a render view.
      csHash<csRefArray<NodeMeshList>*, csPtrKey<iRenderView> > nodeMeshHash;

      // Hash of MeshList objects for visibility objects.
      csHash<NodeMeshList*, csPtrKey<iVisibilityObject> > visobjMeshHash;

      // Vector of vistest objects (used in the box/sphere/etc. tests).
      VistestObjectsArray vistest_objects;
      bool vistest_objects_inuse;

      // Depth test shader type ID.
      csStringID depthTestID;

      // Depth write shader type ID.
      csStringID depthWriteID;

      // Fallback depth write shader type ID.
      csStringID fbDepthWriteID;

      // Shader variable stack for depth rendering.
      csShaderVariableStack shaderVarStack;

      friend class F2BSorter;

    protected:
      /**
       * Renders the meshes in the given list.
       */
      template<bool bQueryVisibility>
      void RenderMeshes (AABBVisTreeNode* node,
                         iRenderView* rview,
                         NodeMeshList*& nodeMeshList);

      /**
       * Transverses the tree from F2B.
       */
      template<bool bDoFrustumCulling>
      void TraverseTreeF2B(AABBVisTreeNode* node, uint32 frustum_mask,
        Front2BackData& f2bData, csRefArray<NodeMeshList>& meshList);

      /**
       * Returns the visibility data of a node.
       */
      inline AABBTreeNodeVisibilityData& GetNodeVisData (AABBVisTreeNode* node)
      {
        return static_cast<AABBTreeNodeVisibilityData&> (*node);
      }

      /**
       * Returns the visibility of a view-node pair.
       */
      OcclusionVisibility GetNodeVisibility (AABBVisTreeNode* node, iRenderView* rview);

      /**
       * Returns whether to check the visibility of a view-node pair.
       */
      bool CheckNodeVisibility (AABBVisTreeNode* node, iRenderView* rview);

      /**
       * Begin a occlusion query of a view-node pair.
       */
      void BeginNodeQuery (AABBVisTreeNode* node, iRenderView* rview);

      /**
       * Get the front and back children of a node.
       */
      void GetF2BChildren (AABBVisTreeNode* node, Front2BackData& data,
        AABBVisTreeNode*& fChild, AABBVisTreeNode*& bChild);

      /**
       * Perform frustum culling on a node.
       */
      NodeVisibility TestNodeVisibility (AABBVisTreeNode* node, Front2BackData& data, uint32& frustum_mask);

      /**
       * Mark all meshes as visible.
       */
      void MarkAllVisible (AABBVisTreeNode* node, Front2BackData& f2bData);

      /**
       * Mesh list comparison function.
       */
      static int NodeMeshListCompare (NodeMeshList* const& object, AABBVisTreeNode* const& key);

      /**
       * Transverses the tree checking for objects that intersect with the given box.
       */
      void TraverseTreeBox(AABBVisTreeNode* node,
        VistestObjectsArray* voArray, const csBox3& box);

      /**
       * Transverses the tree checking for objects that intersect with the given sphere.
       */
      void TraverseTreeSphere(AABBVisTreeNode* node,
        VistestObjectsArray* voArray, const csVector3& centre,
        const float sqradius);

      /**
       * Transverses the tree checking for objects that intersect with the given sphere.
       */
      void TraverseTreeSphere(AABBVisTreeNode* node,
        iVisibilityCullerListener* viscallback,
        const csVector3& centre, const float sqradius);

      /**
       * Transverses the tree checking for objects that are in the volume 
       * formed by the set of planes.
       */
      void TraverseTreePlanes (AABBVisTreeNode* node,
        VistestObjectsArray* voArray, csPlane3* planes,
        uint32 frustum_mask);

      /**
      * Transverses the tree checking for objects that are in the volume 
      * formed by the set of planes.
       */
      void TraverseTreePlanes (AABBVisTreeNode* node,
        iVisibilityCullerListener* viscallback,
        csPlane3* planes, uint32 frustum_mask);

    public:
      csOccluvis (iObjectRegistry *object_reg);
      virtual ~csOccluvis ();

      virtual void Setup (const char*) {}

     /**
      * Register a visibility object with this culler.
      */
      virtual void RegisterVisObject (iVisibilityObject* visobj);

     /**
      * Unregister a visibility object with this culler.
      */
      virtual void UnregisterVisObject (iVisibilityObject* visobj);

     /**
      * Do a visibility test from a given viewpoint.
      */
      virtual bool VisTest (iRenderView* rview, iVisibilityCullerListener* viscallback, int, int);

      virtual csPtr<iVisibilityObjectIterator> VisTest (const csBox3& box);

      virtual csPtr<iVisibilityObjectIterator> VisTest (const csSphere& sphere);

      virtual void VisTest (const csSphere& sphere, 
        iVisibilityCullerListener* viscallback);

      virtual csPtr<iVisibilityObjectIterator> VisTest (csPlane3* planes, int num_planes);

      virtual void VisTest (csPlane3* planes,
        int num_planes, iVisibilityCullerListener* viscallback);

      virtual csPtr<iVisibilityObjectIterator> IntersectSegmentSloppy (
        const csVector3& start, const csVector3& end);

      virtual csPtr<iVisibilityObjectIterator> IntersectSegment (
        const csVector3& start, const csVector3& end, bool accurate = false);

      virtual bool IntersectSegment (const csVector3& start,
        const csVector3& end, csVector3& isect, float* pr = 0,
        iMeshWrapper** p_mesh = 0, int* poly_idx = 0,
        bool accurate = true);

      /**
       * Prepare culling for the next frame.
       */
      virtual void RenderViscull (iRenderView* rview);

     /**
      * Mark that we're about to perform precache visibility culling.
      */
      virtual void PrecacheCulling ()
      {
        bAllVisible = true;
      }

      virtual const char* ParseCullerParameters (iDocumentNode* node) { return 0; }
    };

    class F2BSorter
    {
    public:
      F2BSorter (const csVector3& cameraOrigin)
        : cameraOrigin (cameraOrigin)
      {}

      bool operator() (csOccluvis::NodeMeshList* const& m1,
                       csOccluvis::NodeMeshList* const& m2);

    private:
      const csVector3& cameraOrigin;
    };

    class csOccluvisObjIt :
      public scfImplementation1<csOccluvisObjIt, iVisibilityObjectIterator>
    {
    private:
      VistestObjectsArray* vector;
      size_t position;
      bool* vistest_objects_inuse;

    public:
      csOccluvisObjIt (VistestObjectsArray* vector,
        bool* vistest_objects_inuse) :
        scfImplementationType(this)
      {
        csOccluvisObjIt::vector = vector;
        csOccluvisObjIt::vistest_objects_inuse = vistest_objects_inuse;
        if (vistest_objects_inuse) *vistest_objects_inuse = true;
        Reset ();
      }
      virtual ~csOccluvisObjIt ()
      {
        // If the vistest_objects_inuse pointer is not 0 we set the
        // bool to false to indicate we're no longer using the base
        // vector. Otherwise we delete the vector.
        if (vistest_objects_inuse)
          *vistest_objects_inuse = false;
        else
          delete vector;
      }

      virtual iVisibilityObject* Next()
      {
        if (position == (size_t)-1) return 0;
        iVisibilityObject* vo = vector->Get (position);
        position++;
        if (position == vector->GetSize ())
          position = (size_t)-1;
        return vo;
      }

      virtual void Reset()
      {
        if (vector == 0 || vector->GetSize () < 1)
          position = (size_t)-1;
        else
          position = 0;
      }

      virtual bool HasNext () const
      {
        return ((position != (size_t)-1) && position <= vector->GetSize ());
      }
    };
  }
}

#endif // __CS_RENDERMANAGER_OCCLUVIS_H__
