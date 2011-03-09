/*
  Copyright (C) 2011 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

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
#ifndef __CS_IMESH_ANIMNODE_TEMPLATE_H__
#define __CS_IMESH_ANIMNODE_TEMPLATE_H__

/**\file
 * Base implementation of CS::Animation::iSkeletonAnimNode objects.
 */

#include "csextern.h"
#include "csutil/csstring.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"
#include "csutil/weakref.h"
#include "imesh/animnode/skeleton2anim.h"
#include "iutil/comp.h"

namespace CS {
namespace Animation {

  /**
   * Template class for animation node plugin managers.
   * Usage:
   * - Your "node manager" class must descend from AnimNodeManagerCommon.
   * - \a ThisType must be the name of the "node manager" class.
   * - \a ManagerInterface must be the interface type for your node manager.
   * - \a FactoryType is the node factory to be used by this manager.
   */
  template<typename ThisType,
	   typename ManagerInterface,
	   typename FactoryType>
  class AnimNodeManagerCommon
    : public scfImplementation2<AnimNodeManagerCommon<ThisType, ManagerInterface, FactoryType>,
				ManagerInterface,
				iComponent>
  {
    typedef typename ManagerInterface::FactoryInterfaceType FactoryInterfaceType;
    typedef scfImplementation2<AnimNodeManagerCommon<ThisType, ManagerInterface, FactoryType>,
				ManagerInterface,
				iComponent> scfImplementationType;
  public:
    typedef AnimNodeManagerCommon<ThisType, ManagerInterface, FactoryType> AnimNodeManagerCommonType;
    
    AnimNodeManagerCommon (iBase* parent)
      : scfImplementationType (this, parent), object_reg (nullptr)
    {}      
    
    FactoryInterfaceType* CreateAnimNodeFactory (const char* name)
    {
      csRef<FactoryInterfaceType> newFact;
      newFact.AttachNew (new FactoryType (static_cast<ThisType*> (this), name));
      return nodeFactories.PutUnique (name, newFact);
    }
    FactoryInterfaceType* FindAnimNodeFactory (const char* name)
    {
      return nodeFactories.Get (name, 0);
    }
    void RemoveAnimNodeFactory (const char* name)
    {
      nodeFactories.DeleteAll (name);
    }
    void ClearAnimNodeFactories ()
    {
      nodeFactories.DeleteAll ();
    }
    
    /**\name iComponent implementation
     * @{ */
    bool Initialize (iObjectRegistry* object_reg)
    { this->object_reg = object_reg; return true; }
    /** @} */
    
    iObjectRegistry* GetObjectRegistry() const { return object_reg; }
  protected:
    iObjectRegistry* object_reg;
    csHash<csRef<FactoryInterfaceType>, csString> nodeFactories;
  };

/**
 * Base implementation of a CS::Animation::iSkeletonAnimNodeFactory
 */
class CS_CRYSTALSPACE_EXPORT SkeletonAnimNodeFactory
  : public virtual iSkeletonAnimNodeFactory
{
 public:
  /**
   * Constructor
   */
  SkeletonAnimNodeFactory (const char* name);

  /**
   * Destructor
   */
  virtual ~SkeletonAnimNodeFactory () {}

  /**
   * Get the name of this factory
   */
  virtual const char* GetNodeName () const;

  /// Get the name of this factory
  const csString& GetName() const { return name; }
 protected:
  csString name;
};

class SkeletonAnimNodeSingleBase;

/**
 * Base implementation of a CS::Animation::iSkeletonAnimNodeFactory with a single child
 */
class CS_CRYSTALSPACE_EXPORT SkeletonAnimNodeFactorySingle
  : public SkeletonAnimNodeFactory
{
 public:
  /**
   * Constructor
   */
  SkeletonAnimNodeFactorySingle (const char* name);

  /**
   * Destructor
   */
  virtual ~SkeletonAnimNodeFactorySingle () {}

  /**
   * Set the child animation node of this node. It is valid to provide a null pointer.
   */
  virtual void SetChildNode (iSkeletonAnimNodeFactory* factory);

  /**
   * Get the child animation node of this node, or nullptr if there are none.
   */
  virtual iSkeletonAnimNodeFactory* GetChildNode () const;

  csPtr<iSkeletonAnimNode> CreateInstance (iSkeletonAnimPacket* packet, iSkeleton* skeleton);
  iSkeletonAnimNodeFactory* FindNode (const char* name);
 protected:
  csRef<CS::Animation::iSkeletonAnimNodeFactory> childNodeFactory;
  
  /// To be overridden by derived classes: create actual instance of a node
  virtual csPtr<SkeletonAnimNodeSingleBase> ActualCreateInstance (iSkeletonAnimPacket* packet,
								  iSkeleton* skeleton) = 0;
  
};

/// Methods of SkeletonAnimNodeSingle not dependent on the factory type.
class CS_CRYSTALSPACE_EXPORT SkeletonAnimNodeSingleBase : public virtual iSkeletonAnimNode
{
 public:
  SkeletonAnimNodeSingleBase (CS::Animation::iSkeleton* skeleton);
  virtual ~SkeletonAnimNodeSingleBase () {}

  virtual iSkeletonAnimNode* GetChildNode () const;
  virtual void Play ();
  virtual void Stop ();
  virtual void SetPlaybackPosition (float time);
  virtual float GetPlaybackPosition () const;
  virtual float GetDuration () const;
  virtual void SetPlaybackSpeed (float speed);
  virtual float GetPlaybackSpeed () const;
  virtual void BlendState (csSkeletalState* state, float baseWeight = 1.0f);
  virtual void TickAnimation (float dt);
  virtual bool IsActive () const;
  virtual void AddAnimationCallback (iSkeletonAnimCallback* callback);
  virtual void RemoveAnimationCallback (iSkeletonAnimCallback* callback);

protected:
  friend class SkeletonAnimNodeFactorySingle;
  csWeakRef<CS::Animation::iSkeleton> skeleton;
  csRef<CS::Animation::iSkeletonAnimNode> childNode;
  bool isPlaying;
  float playbackSpeed;
};

/**
 * Base implementation of a CS::Animation::iSkeletonAnimNode with a single child.
 * \a FactoryType is the the of the node factory; it must be (publicly) derived
 * from SkeletonAnimNodeFactory.
 */
template<typename FactoryType>
class SkeletonAnimNodeSingle : public SkeletonAnimNodeSingleBase
{
public:
  SkeletonAnimNodeSingle (FactoryType* factory,
			  CS::Animation::iSkeleton* skeleton)
   : SkeletonAnimNodeSingleBase (skeleton), factory (factory) {}
  
  iSkeletonAnimNodeFactory* GetFactory () const
  {
    return factory;
  }

  iSkeletonAnimNode* FindNode (const char* name)
  {
    if (factory->GetName() == name)
	return this;

    if (childNode)
      return childNode->FindNode (name);
    return nullptr;
  }

protected:
  /// Reference to the factory that instanced a node.
  csRef<FactoryType> factory;
};

/**
 * Base implementation of a CS::Animation::iSkeletonAnimNodeFactory with more than one child
 */
class CS_CRYSTALSPACE_EXPORT SkeletonAnimNodeFactoryMulti
  : public SkeletonAnimNodeFactory
{
 public:
  /**
   * Constructor
   */
  SkeletonAnimNodeFactoryMulti (const char* name);

  /**
   * Destructor
   */
  virtual ~SkeletonAnimNodeFactoryMulti () {}

  /**
   * Add a child animation node to this node. It is NOT valid to provide a null pointer.
   */
  virtual void AddChildNode (iSkeletonAnimNodeFactory* factory);

  /**
   * Remove a child animation node from this node.
   */
  virtual void RemoveChildNode (iSkeletonAnimNodeFactory* factory);

  /**
   * Remove all child animation nodes from this node.
   */
  virtual void ClearChildNodes ();

  /**
   * Get the child animation node of this node with the given index.
   */
  virtual iSkeletonAnimNodeFactory* GetChildNode (size_t index) const;

 protected:
  csRefArray<CS::Animation::iSkeletonAnimNodeFactory> childNodeFactories;
};

} // namespace Animation
} // namespace CS

#endif // __CS_IMESH_ANIMNODE_TEMPLATE_H__
