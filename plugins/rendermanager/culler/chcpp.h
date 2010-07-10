#ifndef _CHCPP_H_
#define _CHCPP_H_

#include "csplugincommon/rendermanager/operations.h"
#include "csplugincommon/rendermanager/posteffects.h"
#include "csplugincommon/rendermanager/render.h"
#include "csplugincommon/rendermanager/renderlayers.h"
#include "csplugincommon/rendermanager/shadersetup.h"
#include "csplugincommon/rendermanager/standardsorter.h"
#include "csplugincommon/rendermanager/svsetup.h"

using namespace CS::RenderManager;

typedef CS::RenderManager::RenderTree<
CS::RenderManager::RenderTreeStandardTraits> RenderTreeType;

// empirically robust constant (might need tweaking)
#define PREV_INV_BATCH_SIZE 25

/*  A small implementation of a list based on the csList class.
 * the main difference between CHCList and csList is that CHCList
 * keeps track of how many elements the list has. This is strictly
 * for convenience is the CHCList is mostly limited in use to this
 * particular implementation of the CHC++ algorithm
 */
template <class T>
class CHCList : public csList<T>
{
  int n;
public:
  CHCList() : n(0)
  {
    csList<T>::csList();
  }
  typename csList<T>::Iterator PushFront(const T& elem)
  {
    ++n;
    return csList::PushFront(elem);
  }

  typename csList<T>::Iterator PushBack(const T& elem)
  {
    ++n;
    return csList::PushBack(elem);
  }

  bool PopFront()
  {
    if(csList::IsEmpty()) return false;
    --n;
    return csList::PopFront();
  }

  bool PopBack()
  {
    if(csList::IsEmpty()) return false;
    --n;
    return csList::PopBack();
  }
  
  bool IsEmpty()
  {
    return csList::IsEmpty();
  }

  T & Front()
  {
    return csList::Front();
  }

  T & Back()
  {
    return csList::Back();
  }

  int Size() const
  {
    return n;
  }
};

/*  Class to hold the visibility information of a kdtree node.
 * The implementation is one that facilitates the use of the  
 * iKDTreeUserData mechanism for storing this information.
 */
class csVisibilityObjectHistory :
    public scfImplementation1<csVisibilityObjectHistory, iKDTreeUserData>
{
public:
  bool bVisible;

  csVisibilityObjectHistory () : scfImplementationType (this), bVisible(false)
  {
  }

  csVisibilityObjectHistory (bool bV) : scfImplementationType (this)
  {
    bVisible=bV;
  }

  virtual ~csVisibilityObjectHistory()
  {
  }

  bool GetVisibility() const
  {
    return bVisible;
  }

  void SetVisibility(const bool bV)
  {
    bVisible=bV;
  }
};

#endif
