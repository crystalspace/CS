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

/// This macro implements the CreateInstance and FindNode methods of an animation node factory
/// with a single child
#define CS_IMPLEMENT_ANIMNODE_FACTORY_SINGLE(nodename)			\
  csPtr<CS::Animation::iSkeletonAnimNode> nodename##Factory::CreateInstance \
    (CS::Animation::iSkeletonAnimPacket* packet, CS::Animation::iSkeleton* skeleton) \
    {									\
      csRef<nodename> newP;						\
      newP.AttachNew (new nodename (this, skeleton));			\
									\
      if (childNodeFactory)						\
	{								\
	  csRef<CS::Animation::iSkeletonAnimNode> node =		\
	    childNodeFactory->CreateInstance (packet, skeleton);	\
	  newP->childNode = node;					\
	}								\
									\
      return csPtr<CS::Animation::iSkeletonAnimNode> (newP);		\
    }									\
									\
  CS::Animation::iSkeletonAnimNodeFactory* nodename##Factory::FindNode (const char* name) \
    {									\
      if (this->name == name)						\
	return this;							\
									\
      if (childNodeFactory)						\
	return childNodeFactory->FindNode (name);			\
									\
      return nullptr;							\
    }

/// This macro implements the GetFactory and FindNode methods of an animation node with a single child
#define CS_IMPLEMENT_ANIMNODE_SINGLE(nodename)				\
  CS::Animation::iSkeletonAnimNodeFactory* nodename::GetFactory () const \
    {									\
      return factory;							\
    }									\
									\
  CS::Animation::iSkeletonAnimNode* nodename::FindNode (const char* name) \
    {									\
      if (factory->name == name)					\
	return this;							\
									\
      if (childNode)							\
	return childNode->FindNode (name);				\
									\
      return nullptr;							\
    }

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

 protected:
  csString name;
};

/**
 * Base implementation of a CS::Animation::iSkeletonAnimNodeFactory with a single child
 */
class CS_CRYSTALSPACE_EXPORT csSkeletonAnimNodeFactorySingle
  : public SkeletonAnimNodeFactory
{
 public:
  /**
   * Constructor
   */
  csSkeletonAnimNodeFactorySingle (const char* name);

  /**
   * Destructor
   */
  virtual ~csSkeletonAnimNodeFactorySingle () {}

  /**
   * Set the child animation node of this node. It is valid to provide a null pointer.
   */
  virtual void SetChildNode (iSkeletonAnimNodeFactory* factory);

  /**
   * Get the child animation node of this node, or nullptr if there are none.
   */
  virtual iSkeletonAnimNodeFactory* GetChildNode () const;

 protected:
  csRef<CS::Animation::iSkeletonAnimNodeFactory> childNodeFactory;
};

/**
 * Base implementation of a CS::Animation::iSkeletonAnimNode with a single child
 */
class CS_CRYSTALSPACE_EXPORT SkeletonAnimNodeSingle : public virtual iSkeletonAnimNode
{
 public:
  /**
   * Constructor
   */
  SkeletonAnimNodeSingle (CS::Animation::iSkeleton* skeleton);

  /**
   * Destructor
   */
  virtual ~SkeletonAnimNodeSingle () {}

  /**
   * Get the child node of this node, or nullptr if there are none.
   */
  virtual iSkeletonAnimNode* GetChildNode () const;

  /**
   * Start playing the node, it will therefore start modifying the state of the skeleton.
   */
  virtual void Play ();

  /**
   * Stop playing the node, it will no longer modify the state of the skeleton.
   */
  virtual void Stop ();

  /**
   * Set the current playback position, in seconds. If time is set beyond the end of the
   * animation then it will be capped.
   */
  virtual void SetPlaybackPosition (float time);

  /**
   * Get the current playback position, in seconds (ie a time value between 0 and GetDuration()).
   */
  virtual float GetPlaybackPosition () const;

  /**
   * Get the time length of this node, in seconds
   */
  virtual float GetDuration () const;

  /**
   * Set the playback speed.
   */
  virtual void SetPlaybackSpeed (float speed);

  /**
   * Get the playback speed. The default value is 1.0.
   */
  virtual float GetPlaybackSpeed () const;

  /**
   * Blend the state of this node into the global skeleton state.
   *
   * \param state The global blend state to blend into
   * \param baseWeight Global weight for the blending of this node
   */
  virtual void BlendState (csSkeletalState* state, float baseWeight = 1.0f);

  /**
   * Update the state of the animation generated by this node
   * \param dt The time since the last update, in seconds
   */
  virtual void TickAnimation (float dt);

  /**
   * Return whether or not this node is currently playing and needs any blending.
   */
  virtual bool IsActive () const;

  /**
   * Add a new animation callback to this node
   * \param callback The callback object
   */
  virtual void AddAnimationCallback (iSkeletonAnimCallback* callback);

  /**
   * Remove the given animation callback from this node
   * \param callback The callback object
   */
  virtual void RemoveAnimationCallback (iSkeletonAnimCallback* callback);

 protected:
  csWeakRef<CS::Animation::iSkeleton> skeleton;
  csRef<CS::Animation::iSkeletonAnimNode> childNode;
  bool isPlaying;
  float playbackSpeed;
};

/**
 * Base implementation of a CS::Animation::iSkeletonAnimNodeFactory with more than one child
 */
class CS_CRYSTALSPACE_EXPORT csSkeletonAnimNodeFactoryMulti
  : public SkeletonAnimNodeFactory
{
 public:
  /**
   * Constructor
   */
  csSkeletonAnimNodeFactoryMulti (const char* name);

  /**
   * Destructor
   */
  virtual ~csSkeletonAnimNodeFactoryMulti () {}

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
