#ifndef _CHCPP_H_
#define _CHCPP_H_

#include "csplugincommon/rendermanager/operations.h"
#include "csplugincommon/rendermanager/posteffects.h"
#include "csplugincommon/rendermanager/render.h"
#include "csplugincommon/rendermanager/renderlayers.h"
#include "csplugincommon/rendermanager/shadersetup.h"
#include "csplugincommon/rendermanager/standardsorter.h"
#include "csplugincommon/rendermanager/svsetup.h"
#include <csutil/list.h>

using namespace CS::RenderManager;

typedef CS::RenderManager::RenderTree<
CS::RenderManager::RenderTreeStandardTraits> RenderTreeType;

// empirically robust constant (might need tweaking)
#define PREV_INV_BATCH_SIZE 25
// visibility threshold parameter
#define VISIBILITY_THRESHOLD 0

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
  }
  typename csList<T>::Iterator PushFront(const T& elem)
  {
    ++n;
    return csList<T>::PushFront(elem);
  }

  typename csList<T>::Iterator PushBack(const T& elem)
  {
    ++n;
    return csList<T>::PushBack(elem);
  }

  bool PopFront()
  {
    if(csList<T>::IsEmpty()) return false;
    --n;
    return csList<T>::PopFront();
  }

  bool PopBack()
  {
    if(csList<T>::IsEmpty()) return false;
    --n;
    return csList<T>::PopBack();
  }
  
  bool IsEmpty()
  {
    return csList<T>::IsEmpty();
  }

  T & Front()
  {
    return csList<T>::Front();
  }

  T & Back()
  {
    return csList<T>::Back();
  }

  int Size() const
  {
    return n;
  }

  bool Delete(const T& item)
  {
    const bool rez=csList<T>::Delete(item);
    if(rez)
      n--;
    return rez;
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

  csVisibilityObjectHistory () : scfImplementationType (this), bVisible(false)//, qID(0)
  {
  }

  csVisibilityObjectHistory (const bool bV/*,const unsigned int ID*/) : scfImplementationType (this)
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
