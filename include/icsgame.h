/*
    Crystal Space Gaming Library
    Copyright (C) 2000 by Thomas Hieber
  
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

#ifndef __ICSGAME_H__
#define __ICSGAME_H__

#include "csutil/scf.h"
#include "csgeom/vector3.h"
#include "iplugin.h"

struct iEntity;
struct iEntityClass;
struct iEntityClassRepository;
struct iEntityIterator;
struct iAttribute;
struct iAttributeArray;
struct iAttributeList;
struct iAttributeIterator;
struct iCollider;

//---------------------------------------------------------------------------

SCF_VERSION (iPosition, 0, 1, 0);

/**
 * A position in the Crystal Space world
 */
struct iPosition : public iBase
{
  /// Sets the position using traditional access.
  virtual SetPosition (csSector* pSector, const csVector3& Pos) = 0;
  
  /// Set the position, using an existing position object.
  virtual SetPosition (iPosition* pPos) = 0;

  /**
   * Move the Position by a relative Vector. 
   * returns false, if that move is not possible.
   */
  virtual bool Move (const csVector3& Offset) = 0;

  /// Sets a new Position.
  virtual void MoveTo (const gePosition& Pos) = 0;

  /**
   * Tries to Move the position to the given Vector. That can only be done, 
   * if the direct path is notblocked by any walls. (Sprites dont count 
   * here...)
   * returns false, if that move is not possible.
   */
  virtual bool MoveTo (const csVector3& Pos) = 0;

  /// Sets the current position.
  virtual void MoveTo (csSector* pSector, const csVector3& Pos) = 0;

  /// Gets the current sector.
  virtual csSector* GetSector () = 0;

  /// Gets the current position in the world.
  virtual csVector3 GetPosition () = 0;
};

//---------------------------------------------------------------------------

SCF_VERSION (iAttribute, 0, 1, 0);

// Definition of the possible types for attributes.
const int AttrType_Float    = 1;
const int AttrType_Integer  = 2;
const int AttrType_String   = 3;
const int AttrType_Entity   = 4;
const int AttrType_Position = 5;
const int AttrType_Vector   = 6;
const int AttrType_Array    = 7;

/**
 * An attribute a name and a value bound together
 */
struct iAttribute : public iBase
{
  /// Get the name of the Attribute.
  virtual const char* GetName() = 0;

  /**
   * Get the type of the Attribute, can be one of the AttrType_xxx
   * constants
   */
  virtual int GetType() = 0;

  /**
   * returns true, if this attribute is to be stored to disk, when
   * the according entity will be saved
   */
  virtual bool IsPersistant() = 0;

  /// Get the value as various types.
  virtual double           GetFloat()        = 0;
  virtual int              GetInteger()      = 0;
  virtual const char*      GetString()       = 0;
  virtual iEntity*         GetEntity()       = 0;
  virtual iPosition*       GetPosition()     = 0;
  virtual csVector3        GetVector()       = 0;
  virtual iAttributeArray* GetArray()        = 0;

  /// Set the value as various types.
  virtual void SetFloat   (double Val)           = 0;
  virtual void SetInteger (int Val)              = 0;
  virtual void SetString  (const char* Val)      = 0;
  virtual void SetEntity  (iEntity* Val)         = 0;
  virtual void SetPosition(iPosition* Val)       = 0;
  virtual void SetVector  (csVector3 Val)        = 0;
  virtual void SetArray   (iAttributeArray* Val) = 0;
};

//---------------------------------------------------------------------------

SCF_VERSION (iAttributeIterator, 0, 1, 0);

struct iAttributeIterator : public iBase
{
  /// Get a pointer to the first matching attribute.
  virtual iAttribute* GetFirst () = 0;

  /// Get a pointer to the next matching attribute.
  virtual iAttribute* GetNext () = 0;
};

//---------------------------------------------------------------------------

SCF_VERSION (iAttributeList, 0, 1, 0);

/**
 * A list of named attributes and according values
 */
struct iAttributeList : public iBase
{
  /// Get the entry with the given name.
  virtual iAttribute* Get (const char* Name) = 0;

  /**
   * Add the entry with the given name, use NULL to 
   * remove the entry again.
   */
  virtual void Add (const char* Name, iAttribute* Entry) = 0;

  /**
   * Get an iterator for all contained Attributes
   */
  virtual iAttributeIterator* GetAllAttributes() = 0;
};

//---------------------------------------------------------------------------

SCF_VERSION (iAttributeArray, 0, 1, 0);

/**
 * An Array of attributes
 */
struct iAttributeArray : public iBase
{
  /// Get the number of entries.
  virtual int* GetNum () = 0;

  /**
   * Get the entry at the given position, 
   * can return NULL for invalid index.
   */
  virtual iAttribute* GetAt (int Index) = 0;

  /**
   * Set the entry at the given position
   * Set to NULL, if you want the array to shrink
   * Setting to an Index that is larger than GetNum()-1 
   * will automatically grow the Array.
   */
  virtual void SetAt (int Index, iAttribute* Entry) = 0;
};

//---------------------------------------------------------------------------

SCF_VERSION (iEntity, 0, 1, 0);

/**
 * A game entity.
 */
struct iEntity : public iBase
{
  /// Get the name of the Entity.
  virtual const char* GetName () = 0;

  /// Set the name of the Entity.
  virtual const char* SetName () = 0;

  /// Get the classname of this entity
  virtual const char* GetClassname() = 0;

  /**
   * Check if this entity supports a specific classname, either
   * by inheritance or by manually implementing its interface.
   * useful for example to query an entity of class "Sabertooth"
   * if it supports the "cs_weapon" interface
   */
  virtual bool SupportsInterface(const char* classname) = 0;

  /**
   * Get the position of the entity, or NULL if the entity has
   * no position in the 3D world.
   */
  virtual iPosition* GetPosition () = 0;

  /**
   * Get the size of the bounding box for this entity. There will
   * be no collision detection happening outside of this bounding
   * box, so the entity sould rather return a too big bounding
   * box than a too small one. The radius is expected to be constant
   * all the time. If an entity returns a radius of (0,0,0) it is
   * seen as a point based entity, which means some special handling
   * for collision detection.
   */
  virtual csVector3 GetRadius() = 0;

  /**
   * Get the collider for a collision test against the given Entity
   * The pointer to the collider is being returned. Also the relevant
   * Transformation for the collision test, and the information, whether
   * the collider is solid or not.
   */
  virtual void GetCollider(iEntity*     pOtherEntity, 
                           iCollider*&  pCollider,
                           csTransform& pTransform,
                           bool&        Solid);

  /**
   * Let the entity handle a given event. If you don't need any
   * return parameters, you can set OutPar to NULL. If you don't
   * want to hand over Input parameters, InPar can also be NULL
   * the method will return true, if the event has been handled,
   * otherwise it will return false.
   */
  virtual bool HandleEvent(const char*     EventName, 
                           iAttributeList* InPar,
                           iAttributeList* OutPar) = 0;

  /// Get the given Attribute.
  virtual iAttribute* GetAttribute (const char* Name) = 0;

  /// Get the given Tag.
  virtual iAttribute* GetTag (const char* TagKey, const char* Name) = 0;

  /**
   * returns true, if this entity is to be stored to disk, when
   * the game state will be saved (persistance is only useful for 
   * objects that can be modifed in a way while the game is running,
   * an whose state the player will remember all other entites
   * can happily remain unsaved.
   */
  virtual bool IsPersistant() = 0;

  /**
   * Get an iterator, that will allow the caller to iterate across all
   * attributes in this entity and all baseclasses. (Used mainly for
   * persistance.)
   */
  virtual iAttributeIterator* GetAllAttributes() = 0;
};

//---------------------------------------------------------------------------

SCF_VERSION (iEntityClass, 0, 1, 0);

/**
 * An Entity class.
 */
struct iEntityClass : public iBase
{
  /// Get the name of the Entity-Class (eg: "cs_door").
  virtual const char* GetName () = 0;

  /// Create an Entity of this class.
  virtual iEntity* CreateEntity () = 0;
};

//---------------------------------------------------------------------------

SCF_VERSION (iEntityClassRepository, 0, 1, 0);

/**
 * The entity class.repository
 */
struct iEntityClassRepository : public iBase
{
  /// Create an Entity of the given classname.
  virtual iEntity* CreateEntity (const char* classname) = 0;

  /// Get an Registered Entity Class, or NULL if not registered.
  virtual iEntityClass* GetEntityClass (const char* classname) = 0;

  /// Register an EntityClass. Call with NULL to remove registration.
  virtual void RegisterClass (iEntityClass* pEntityClass) = 0;
};

//---------------------------------------------------------------------------

SCF_VERSION (iEntityIterator, 0, 1, 0);

/**
 * The interface for an iterator across Game entites
 */
struct iEntityIterator : public iBase
{
  /// Get a pointer to the first matching entity.
  virtual iEntity* GetFirst () = 0;

  /// Get a pointer to the next matching entity.
  virtual iEntity* GetNext () = 0;
};

//---------------------------------------------------------------------------

SCF_VERSION (iDataLoader, 0, 1, 0);

/**
 * A Data Loader allows entites to read a structured datafile
 */
struct iDataLoader : public iBase
{
  /**
   * Opens a Data Context. This operation can be compared to switching
   * to a subdirectory in a filesystem. All following operations will
   * happen relative to that Context. A top level context could be for 
   * example the name of an entity. This function returns false, if the
   * given context does not exist.
   */
  virtual bool OpenContext(const char* Context) = 0;
  
  /**
   * Closes a Context that has been opened by OpenContext
   */
  virtual void CloseContext() = 0;

  /**
   * Gets the name of the first context in the current context, or NULL 
   * if there is no Context at all
   */
  virtual const char* GetFirstContext() = 0;

  /**
   * Gets the name of the next context in the current context, or NULL 
   * if there is no further Context.
   */
  virtual const char* GetNextContext() = 0;

  /**
   * Gets the name of the first element in the current context, or NULL 
   * if there is no element at all
   */
  virtual const char* GetFirstElement() = 0;

  /**
   * Gets the name of the next element in the current context, or NULL 
   * if there is no further element.
   */
  virtual const char* GetNextElement() = 0;

  /**
   * Gets the value of the Element
   */
  virtual const char* GetElement(const char* Key) = 0;
};

//---------------------------------------------------------------------------

SCF_VERSION (iDataSaver, 0, 1, 0);

/**
 * A Data Loader allows entites to read a structured datafile
 */
struct iDataSaver : public iBase
{
  /**
   * Creates a new Data Context. This operation can be compared to creating
   * a subdirectory in a filesystem and switching to it. All following 
   * operations will happen relative to that new Context. A top level 
   * context could be for example the name of an entity. 
   */
  virtual void CreateContext(const char* Context) = 0;
  
  /**
   * Closes a Context that has been created by CreateContext and returns to
   * the previous Context level
   */
  virtual void CloseContext() = 0;

  /**
   * Creates a new Element at the current Context level.
   */
  virtual void CreateElement(const char* Key, const char* Value) = 0;
};


//---------------------------------------------------------------------------

SCF_VERSION (iGameCore, 0, 1, 0);

/**
 * The csgame core
 */
struct iGameCore : public iBase
{
  /// Get a pointer to the class repository.
  virtual iEntityClassRepository* GetClassRepository () = 0;

  /// Add an entity to be managed by the game core.
  virtual void AddEntity (iEntity* pEntity) = 0;

  /// Remove an entity from the game core.
  virtual void RemoveEntity (const char* Name) = 0;

  /**
   * Send an event directly to an entity. This is mostly a convenience 
   * function to avoid searching for an entity and calling HandleEvent 
   * directly. Anyway, using this method is the recommended way of sending 
   * events.
   */
  virtual void SendEvent(const char* EntityName, 
                         const char* EventName,
                         iAttributeList* InPar,
                         iAttributeList* OutPar) = 0;

  /**
   * Send an event delayed to an entity. This is mostly a convenience 
   * function to avoid searching for an entity and calling HandleEvent 
   * directly. Anyway, using this method is the recommended way of sending 
   * events.
   * The delay will be given in seconds. If the event is marked as 
   * persistant, then the event will be stored with the target object, 
   * when the object will be stored to disk.
   */
  virtual void SendDelayedEvent(const char* EntityName, 
                                const char* EventName,
                                iAttributeList* InPar,
                                double delay, bool Persistant) = 0;

  /**
   * Send an event directly to all entitie within a radius around a 
   * position.
   */
  virtual void SendEventToGroup(iPosition* pCenter, 
                                csVector3  Radius,
                                const char* EventName,
                                iAttributeList* InPar) = 0;


  /**
   * Revoke an event that has been sent delayed to an entity and that
   * has not yet arrived. If there are multiple Events with the same
   * name, that wait to be delivered, all of them are removed.
   */
  virtual void RevokeDelayedEvent(const char* EntityName, 
                                  const char* EventName) = 0;

  /**
   * Get an iterator, that will allow you to iterate across all entities 
   * within the specified box.
   */
  virtual iEntityIterator* GetAllObjectsWithinBox (iPosition* Center,
  	const csVector3& Radius) = 0;

  /**
   * Redirect the all user input to go as events to the specified entity
   */
  virtual void SetInputFocus(iEntity* pEntity) = 0;

  /// Try to store all relevant data to the DataSaver
  virtual void StoreState(iDataSaver* pSaver) = 0;

  /// Try to load all relevant data from the DataLoader
  virtual void LoadState(iDataSaver* pSaver) = 0;

  /**
   * Saves all persistant data of the given entity. It is assumed, that the
   * proper context for saving in the DataSaver has already been set by the
   * caller, for example by calling pSaver->CreateContext(pEntity->GetName());
   * The data written will contain the entities class and name.
   */
  virtual void StoreEntityData(iDataSaver* pSaver, iEntity* pEntity) = 0;

  /**
   * Create an Entity from the information within a DataLoader Object.
   * The classname and name of the entity will be used to create a new
   * object with the proper class and name. It is assumed that the caller
   * has already set the proper context for loading, by using 
   * pLoader->OpenContext()
   */
  virtual iEntity* LoadEntity(iDataLoader* pLoader) = 0;
};

#endif // __ICSGAME_H__

