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

    class CS_CRYSTALSPACE_EXPORT csOccluvis : public AABBVisTree
    {
    private:
      iObjectRegistry *object_reg;
      csRef<iGraphics3D> g3d;
      csRef<iEngine> engine;

      // Structure of common f2b data.
      struct Front2BackData
      {
        csVector3 pos;
        iRenderView* rview;
        csPlane3* frustum;
        iVisibilityCullerListener* viscallback;
      };

      struct MeshList
      {
        MeshList (csSectorVisibleRenderMeshes* mL, const int nM) : numMeshes(nM)
        {
          meshList = new csSectorVisibleRenderMeshes[numMeshes];

          for (int m = 0; m < numMeshes; ++m)
          {
            meshList[m] = mL[m];
          }
        }

        ~MeshList ()
        {
          delete[] meshList;
        }

        int numMeshes;
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

    protected:
      /**
       * Renders the meshes in the given list.
       */
      template<bool bQueryVisibility>
      void RenderMeshes (AABBVisTreeNode* node, Front2BackData& f2bData, csArray<MeshList*> &meshList);

      /**
       * Transverses the tree from F2B.
       */
      template<bool bDoFrustumCulling>
      void TraverseNodeF2B(AABBVisTreeNode* node, bool parentVisible,
        uint32 frustum_mask, Front2BackData& f2bData, csArray<MeshList*>& meshList);

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

    public:
      csOccluvis (iObjectRegistry *object_reg);

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
      virtual void VisTest (iRenderView* rview, iVisibilityCullerListener* viscallback);

     /**
      * Mark that we're about to perform precache visibility culling.
      */
      virtual void PreparePrecacheCulling ()
      {
        bAllVisible = true;
      }
    };
  }
}

#endif // __CS_RENDERMANAGER_OCCLUVIS_H__
