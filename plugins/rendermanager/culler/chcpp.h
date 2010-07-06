#ifndef _CHCPP_H_
#define _CHCPP_H_

// empirically robust constant (might need tweaking)
#define PREV_INV_BATCH_SIZE 25

template <class T, class MemoryAllocator = CS::Memory::AllocatorMalloc>
class CHCList
{
public:
  CHCList() : n(0)
  {
  }
  int n;
  csList<T> list;
  typename csList<T>::Iterator PushFront(T elem)
  {
    ++n;
    return list.PushFront(elem);
  }

  typename csList<T>::Iterator PushBack(T elem)
  {
    ++n;
    return list.PushBack(elem);
  }

  bool PopFront()
  {
    if(list.IsEmpty()) return false;
    --n;
    return list.PopFront();
  }

  bool PopBack()
  {
    if(list.IsEmpty()) return false;
    --n;
    return list.PopBack();
  }
  
  bool IsEmpty()
  {
    return list.IsEmpty();
  }

  T & Front()
  {
    return list.Front();
  }

  T & Back()
  {
    return list.Back();
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
