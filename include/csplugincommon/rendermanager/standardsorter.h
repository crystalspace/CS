/*
    Copyright (C) 2007 by Marten Svanfeldt

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation; 

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_STANDARDSOTER_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_STANDARDSOTER_H__

namespace CS
{
namespace RenderManager
{

  template<typename Tree>
  class StandardMeshSorter
  {
  public:
    StandardMeshSorter (iEngine* engine, iCamera* camera)
    {
      // Build table of sorting options
      renderPriorityCount = engine->GetRenderPriorityCount ();
      sortingTypeTable = new csRenderPrioritySorting[renderPriorityCount];
      
      for (int i = 0; i < renderPriorityCount; ++i)
      {
        //TODO: Consider lazy evaluation of this
        sortingTypeTable[i] = engine->GetRenderPrioritySorting (i);
      }

      sort_cameraOrigin = camera->GetTransform ().GetOrigin ();
    }

    ~StandardMeshSorter ()
    {
      delete[] sortingTypeTable;
    }

    void operator()(const typename Tree::TreeTraitsType::MeshNodeKeyType& key, 
      typename Tree::MeshNode* meshNode, typename Tree::ContextNode& contextNode, Tree& tree)
    {
      // Get the render priority for node
      int renderPrio = meshNode->customData.priority;
      CS_ASSERT(renderPrio >= 0 && renderPrio < renderPriorityCount);
      
      switch (sortingTypeTable[renderPrio])
      {
      case CS_RENDPRI_SORT_NONE:
        {
          // Do normal sorting
          meshNode->meshes.Sort (SortMeshNormal);
        }
        break;
      case CS_RENDPRI_SORT_BACK2FRONT:
        {
          // 
        }
        break;
      case CS_RENDPRI_SORT_FRONT2BACK:
        {

        }
        break;
      default:
        CS_ASSERT_MSG("Invalid sorting mode!", 0);
      }
    }

    static int SortMeshNormal (typename Tree::MeshNode::SingleMesh const& m1,
      typename Tree::MeshNode::SingleMesh const& m2)
    {
      const csRenderMesh* rm1 = m1.renderMesh;
      const csRenderMesh* rm2 = m2.renderMesh;

    if (rm1->material > rm2->material)
      return 1;
    else if (rm1->material < rm2->material)
      return -1;
    
    if (rm1->geometryInstance > rm2->geometryInstance)
      return 1;
    else if (rm1->geometryInstance < rm2->geometryInstance)
      return -1;

    return 0;
    }

  private:
    int renderPriorityCount;
    csRenderPrioritySorting* sortingTypeTable;

    static csVector3 sort_cameraOrigin;
  };

  template<typename Tree>
  csVector3 StandardMeshSorter<Tree>::sort_cameraOrigin; 
}
}

#endif
