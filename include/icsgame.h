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
#include "csgeom/transfrm.h"
#include "iplugin.h"
#include "ivfs.h"

struct iEntity;
struct iEntityClass;
struct iEntityClassRepository;
struct iEntityIterator;
struct iAttribute;
struct iAttributeArray;
struct iAttributeList;
struct iAttributeIterator;
struct iCollider;
struct iDataLoader;
struct iDataSaver; 
struct iSector;

#define geMAPITERATOR int

//---------------------------------------------------------------------------

SCF_VERSION (iPosition, 0, 1, 0);

/**
 * A position in the Crystal Space world, coupled together with an orientation
 * You can create a position by using the CREATE_INSTANCE function or if you
 * want to create a linked position (where one object follows an other like
 * they would be mechanically linked) by calling CreateLinkedPosition in an
 * existing objects.
 */
struct iPosition : public iBase
{
  /**
   * Sets the position using traditional access.
   * pSector is a pointer to the sector,
   * Pos is the worldspace position in that sector,
   * Orientation is the orientation of the entity<br>
   * In orientation:
   * <ul>
   * <li>x is the rotation around the upward axis,
   * <li>y is the elevation against the floor plane
   * <li>z is the rotation of the object around the forward axis
   * </ul>
   * All angles are in Rad, which means 360° = 2Pi
   * So the orientation will allow you, to have the object being turned
   * in every possible orentation, but you can not stretch or bend it.
   * (Of course that could be possible in classes that extend the 
   * current iPosition interface.)
   */
  virtual void SetPosition (iSector*         pSector, 
                       const csVector3& Pos, 
                       const csVector3& Orientation) = 0;
  
  /**
   * Set the position, using an existing position object, The position data
   * will only be copied by this call. This call will not create a link
   * between these objects
   */
  virtual void SetPosition (iPosition* pPos) = 0;

  /// Gets the current sector.
  virtual iSector* GetSector () = 0;

  /// Gets the current position in the world.
  virtual csVector3 GetVector () = 0;

  /// Gets the orientation, as being set by SetOrientation
  virtual csVector3 GetOrientation() = 0;

  /// Gets Orientation and Vector in one structure
  virtual csTransform GetTransform() = 0;

  /**
   * Move the Position by a relative Vector. 
   * returns false, if that move is not possible.
   */
  virtual bool Move (const csVector3& Offset) = 0;

  /**
   * Tries to Move the position to the given Vector. That can only be done, 
   * if the direct path is notblocked by any walls. (Sprites dont count 
   * here...)
   * returns false, if that move is not possible.
   */
  virtual bool MoveTo (const csVector3& Pos) = 0;
  
  /**
   * Change the orientation by the given angle differences
   */
  virtual void Rotate(const csVector3& Offset) = 0;

  /**
   * Sets a new orientation for the position
   */
  virtual void SetOrientation(const csVector3& Orientation) = 0;

  /**
   * Create a new Position object, that is linked to this position object.
   * How the link behaves exactly can be controlled by a combination
   * of CS_POSLINKFLAG_xxx Flags.
   * It is important to note, that the given transformation needs to be
   * a transformation in worldcoordinates. The iPosition objects will
   * convert this position to a relative position and update the linked
   * position whenever its own coordinates change.
   * You can use any calls to set te position of the linked object,
   * but when the controlling object moves it will be moved according to 
   * the linked objects new relative coordinates.
   */
  virtual iPosition* CreateLinkedPosition(const csTransform& Transform, 
                                          ULong LinkFlags) = 0;
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
   * the according entity will be saved. 
   */
  virtual bool IsPersistant() = 0;

  /**
   * returns true, if the referenced data is considered to belong to
   * the attribute, instead of just pointing to the data.
   * by default all simple types like float or string are aggreagted.
   * This is also true for Arrays and Position. For Entities you can 
   * choose, if you want reference or aggregation.
   */
  virtual bool IsAggregated() = 0;

  /// Get the value as various types.
  virtual double           GetFloat()        = 0;
  virtual int              GetInteger()      = 0;
  virtual const char*      GetString()       = 0;
  virtual iEntity*         GetEntity()       = 0;
  virtual iPosition*       GetPosition()     = 0;
  virtual csVector3        GetVector()       = 0;
  virtual iAttributeArray* GetArray()        = 0;

  /// Set the value as various types.
  virtual void SetFloat   (double Val)                   = 0;
  virtual void SetInteger (int Val)                      = 0;
  virtual void SetString  (const char* Val)              = 0;
  virtual void SetEntity  (iEntity* Val, bool Aggregate) = 0;
  virtual void SetPosition(iPosition* Val)               = 0;
  virtual void SetVector  (const csVector3& Val)         = 0;
  virtual void SetArray   (iAttributeArray* Val)         = 0;
};

//---------------------------------------------------------------------------

SCF_VERSION (iAttributeIterator, 0, 1, 0);

/**
 * Allows to iterate across several Attributes
 */
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
  virtual int GetNum () = 0;

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

SCF_VERSION (iEntityComponent, 0, 1, 0);

/**
 * A component of a game entity that handles events
 */
struct iEntityEventhandler : public iBase
{
  /**
   * Let the entity component handle a given event. If there are no return
   * parameters, OutPar can be NULL. If there are no Input parameters,
   * InPar can also be NULL.
   * The method needs to return true, if the event has been handled, 
   * otherwise it must return false. In that case the entity will keep on
   * searching for another entity that can handle this event.
   */
  virtual bool HandleEvent(const char*     EventName, 
                           iAttributeList* InPar,
                           iAttributeList* OutPar) = 0;
};

//---------------------------------------------------------------------------

SCF_VERSION (iEntityCollider, 0, 1, 0);

/**
 * A collider component of a game entity.
 */
struct iEntityCollider : public iBase
{
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
   * Returns true, if this entity wants the other entity to return
   * its detailed collider fot the following collision test.
   */
  virtual bool WantDetailedCollider(iEntity* pOtherEntity) = 0;

  /**
   * Get the collider for a collision test against the given Entity
   * The pointer to the collider is being returned. Also the relevant
   * Transformation for the collision test, and the information, whether
   * the collider is solid or not.
   */
  virtual void GetCollider(iEntity*     pOtherEntity, 
                           bool         WantDetailed,
                           iCollider*&  pCollider,
                           csTransform& pTransform,
                           bool&        Solid);
};

//---------------------------------------------------------------------------

SCF_VERSION (iEntityAttributes, 0, 1, 0);

/**
 * An entity component to manage attributes
 */
struct iEntityAttributes : public iBase
{
  /// Get the given Attribute.
  virtual iAttribute* GetAttribute (const char* Name) = 0;

  /// Get the given Tag.
  virtual iAttribute* GetTag (const char* TagKey, const char* Name) = 0;

  /**
   * Get an iterator, that will allow the caller to iterate across all
   * attributes in this entity and all baseclasses. (Used mainly for
   * persistance.)
   */
  virtual iAttributeIterator* GetAllAttributes() = 0;

  /**
   * returns true, if this entity is to be stored to disk, when
   * the game state will be saved (persistance is only useful for 
   * objects that can be modifed in a way while the game is running,
   * an whose state the player will remember all other entites
   * can happily remain unsaved.
   */
  virtual bool IsPersistant() = 0;
};

//---------------------------------------------------------------------------

SCF_VERSION (iEntityClassInformation, 0, 1, 0);

/**
 * An entity component to manage attributes
 */
struct iEntityClassInformation : public iBase
{
  /// Get the classname of this entity
  virtual const char* GetClassname() = 0;

  /**
   * Check if this entity supports a specific classname, either
   * by inheritance or by manually implementing its interface.
   * useful for example to query an entity of class "Sabertooth"
   * if it supports the "cs_weapon" interface
   */
  virtual bool SupportsInterface(const char* EntityClassname) = 0;
};


//---------------------------------------------------------------------------

SCF_VERSION (iEntityComponentIterator, 0, 1, 0);

/**
 * Allows to iterate across several components of an Entity
 */
struct iEntityComponentIterator : public iBase
{
  /// Get a pointer to the first matching interface.
  virtual void* GetFirst () = 0;

  /// Get a pointer to the next matching interface.
  virtual void* GetNext () = 0;
};

// This Macro can make it more conventient to iterate all interfaces of 
// the given type. Useage example:
//
//  iEntity* pEntity = ...;
//  FOREACH_ENTITYCOMPONENT(iEntityEventhandler, EvtHdler, pEntity)
//  {
//    if (EvtHdler->HandleEvent(...)) break;
//  }

#define FOREACH_ENTITYCOMPONENT(classname, var, entity) \
  classname* var = NULL; \
  iEntityComponentIterator* iter## __LINE__ = \
    entity->GetComponents(#classname); \
  for (var = (classname*) iter## __LINE__ ->GetFirst(); var; \
       var = (classname*) iter## __LINE__ ->GetNext()) 

//---------------------------------------------------------------------------

SCF_VERSION (iEntity, 0, 1, 0);

/**
 * A game entity. Mainly a collection of aggegated objects and baseclasses.
 * all the real work is being done by these objects.
 * The entity is only the container to manage all the contained components.
 */
struct iEntity : public iBase
{
  /// Get the name of the Entity.
  virtual const char* GetName () = 0;

  /// Set the name of the Entity.
  virtual void SetName (const char* Name) = 0;

  /**
   * Get a pointer to the first embedded interface with the given
   * interface name, or NULL, if there is no embedded interface
   * of that type. It is safe to directly cast up the returned pointer
   * to (ScfClassname*).
   *
   * Typical Classes that are to be used inside an entity are:
   * - iEntityCollider
   * - iEntityAttributes
   * - iPositition
   * - iEntityClassInformation
   * - iEntityEventhandler (But you should use GetComponents instead of this!)
   */
  virtual void* GetFirstComponent(const char* ScfClassname, int Version) = 0;

  /**
   * Get an iterator, that will return all contained Components that
   * support the given interface. Please keep in mind, that using 
   * GetFirstComponent to get a specific interface is not just more
   * convenient, but also much faster, so you should only use this
   * call, if you are expecting several instaces of the given interface.
   * By using "iBase" as ScfClassname, you can get all contained classes
   * and Query for interface yourself.
   */
  virtual iEntityComponentIterator* GetComponents
            (const char* ScfClassname, int Version) = 0;
};

//---------------------------------------------------------------------------

SCF_VERSION (iEntityClass, 0, 1, 0);

/**
 * An Entity class. Defines the behaviour and default data for a class of
 * entites
 */
struct iEntityClass : public iBase
{
  /// Get the name of the Entity-Class (eg: "cs_door").
  virtual const char* GetName () = 0;

  /// Create an Entity of this class.
  virtual iEntity* CreateEntity () = 0;

  /**
   * Read the definition for this class from the given structured data
   * file. Normally all information needed to define this class should 
   * be contained there. For all missing data we will use defaults.
   */
  virtual void ReadDefinition(iDataLoader* pDefinition) = 0;
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

SCF_VERSION (iGameSprite3D, 0, 1, 0);

/**
 * The interface for an iterator across Game entites
 */
struct iGameSprite3D : public iBase
{
  /**
   * Get the name of the model, that defines that sprite
   */
  virtual const char* GetModelName() = 0;
  
  /**
   * Set the position to be used by the Sprite. You can either use an 
   * existing position object (for example the entity position), or
   * create a new position object for the Sprite to use.
   * Using the entites position object has the advantage, that the 
   * Sprite will be able to update the position of the engine while
   * it plays the animation. This is very useful for example, when
   * you have a model of a walking object to keep movement in sync
   * with position.
   */
  virtual void SetPosition (iPosition* pPos) = 0;

  /// Get the current position of the sprite
  virtual iPosition* GetPosition() = 0;

  /**
   * Get a pointer to the current collider. While animation advances,
   * the collider frame will also advance
   */
  virtual iCollider* GetCollider() = 0;

  /**
   * Set a pointer to the entity, that will receive the events that
   * occur while the animation is running. (For example there could
   * be an event "fire_gun" that was triggered, whenever the animation
   * frame was passed that has the model react to the firing)
   */
  virtual void SetEntity(iEntity* pEntity) = 0;

  /**
   * switches to one of the predefined animation modes available for
   * this sprite.
   */
  virtual void SetAnmiation(const char* Animation) = 0;

  /**
   * Sets the current Animation Frame. Automatic animation will contine
   * at that point 
   */
  virtual void SetAnimationFrame(const char* Animation, int Frame) = 0;

  /// Get the current frame of animation
  virtual int GetAnimationFrame() = 0;

  /// Get the name of the current animation sequence.
  virtual const char* GetAnimation() = 0;

  /**
   * Used by the game engine, to keep the sprite animated. Do not call this
   * method from your game code.
   */
  virtual void AdvanceAnimation() = 0;
};

//---------------------------------------------------------------------------

SCF_VERSION (iColliderSelector, 0, 1, 0);

/**
 * A Collider Selector allows to pick a collider without creating an event.
 * This is a big advantage, because Scripting is not very fast, and the
 * engine needs to do lots of collision tests, so using a native class seems
 * to be adviseable. If you really want to handle this in scripts, feel free
 * to do so. The iColliderSelector is an internal class for an entity, and
 * can easily be omitted.
 */
struct iColliderSelector : public iBase
{
  /**
   * Returns true, if this entity wants the other entity to return
   * its detailed collider fot the following collision test.
   */
  virtual bool WantDetailedCollider(iEntity* pOtherEntity) = 0;

  /**
   * Get the number of the collider for a collision test against the given 
   * Entity. Also returns, the information, whether the collider is solid 
   * or not. It is the task of the containing entity, to pick the remaining
   * information itself.
   */
  virtual void GetCollider(iEntity*     pOtherEntity, 
                           bool         WantDetailed,
                           int&         NumCollider,
                           bool&        Solid);
  /**
   * Loads the definition from stored data. This seams to be easier to use,
   * than putting all the rules togehter at runtime. The format of the rules
   * will be described later. (And will of course be specific for any class
   * that implements this selector.)
   */
  virtual void LoadDefinition(iDataLoader* pLoader) = 0;
};


//---------------------------------------------------------------------------

SCF_VERSION (iDataLoader, 0, 1, 0);

/**
 * A Data Loader allows entites to structured data. Where the data
 * comes from, is not important for this interface.
 */
struct iDataLoader : public iBase
{
  /**
   * return the name of the current context
   */
  virtual const char* GetName() = 0;

  /**
   * return the data of the current context
   */
  virtual const char* GetData() = 0;

  /**
   * Gets a specific contained Data Context. This function returns 
   * NULL, if the given context does not exist.
   */
  virtual iDataLoader* GetContext(const char* Context) = 0;
  
  /**
   * Gets the value of the Element with the given name
   */
  virtual const char* GetAttributeData(const char* Key) = 0;

  /**
   * Gets the value of the Context with the given name
   */
  virtual const char* GetContextData(const char* Context) = 0;

  /**
   * Gets the name of the first context in the current context, or NULL 
   * if there is no Context at all
   */
  virtual iDataLoader* GetFirstContext(geMAPITERATOR& iter) = 0;

  /**
   * Gets the name of the next context in the current context, or NULL 
   * if there is no further Context.
   */
  virtual iDataLoader* GetNextContext(geMAPITERATOR& iter) = 0;

  /**
   * Gets the name and value of the first attribute in the current context, 
   * or NULL if there are no attributes at all
   */
  virtual bool GetFirstAttribute(geMAPITERATOR& iter,
                                 const char*& Key, 
                                 const char*& Value) = 0;

  /**
   * Gets the name and value of the next attribute in the current context, 
   * or NULL if there are no further attributes.
   */
  virtual bool GetNextAttribute(geMAPITERATOR& iter,
                                const char*& Key, 
                                const char*& Value) = 0;

};

//---------------------------------------------------------------------------

SCF_VERSION (iDataLoaderFile, 0, 1, 0);

/**
 * A Data Loader File ins an interface, that allows you to read structured 
 * Data from a file. 
 * Normally this is an additional interface of a class that supports the
 * iDataFile interface.
 */
struct iDataLoaderFile : public iBase
{
  /**
   * Opens a Data File. The file is read completely and is being closed again.
   * After a call to Read(), you can read the contained data by using the
   * iDataLoader interface. The function will return false, if something
   * went wrong loading the file.
   */
  virtual bool Read(iVFS* pVFS, const char* filename) = 0;

  /// Get the root context of the file.
  virtual iDataLoader* GetContext() = 0;
};

//---------------------------------------------------------------------------

SCF_VERSION (iDataSaver, 0, 1, 0);

/**
 * A Data Saver allows entites to write a structured datafile
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

SCF_VERSION (iGameNetworkConnection, 0, 1, 0);

/**
 * A network connection takes care of sending events to entities on a remote
 * computer for network gameplay.
 */
struct iGameNetworkConnection : public iBase
{
  /**
   * Sends an event across the network connection to an entity.
   * Relevance time is in seconds and gives the time, after that,
   * the event can be safely dropped. If you set Relevance time
   * to 0, the event will get sent, even if it takes forever.
   */
  virtual void SendEvent(const char*     Sender,
                         const char*     Target, 
                         const char*     EventName,
                         iAttributeList* InPar,
                         float           RelevanceTime) = 0;

  /**
   * Revokes an event, that has not yet been deleivered. 
   * This method is only there to optimise transmission. It
   * can't be guaranteed, that the event will not arrive after
   * you have deleted it!
   */
  virtual void RevokeEvent(const char*     Sender,
                           const char*     Target,
                           const char*     EventName) = 0;

  /**
   * Set the connection parameters
   */
  virtual void SetConnectionParameters(int MaxBytesPerSecond,
                                       int MaxMessagesPerSecond) = 0;

  /**
   * Checks, if the connection is established and working.
   */
  virtual bool IsConnectionReady();

  /// Disconnect a network connection
  virtual void Disconnect();
};

//---------------------------------------------------------------------------

SCF_VERSION (iGameServerConnection, 0, 1, 0);

/**
 * A network connection from the multiplayer client to the server.
 */
struct iGameServerConnection : public iBase
{
  /// Get the according network connection object
  virtual iGameNetworkConnection* GetConnection() = 0;

  /// Return a list of server names
  virtual void FindServers(iAttributeArray& pServers) = 0;

  /// Connect to the server with the given name
  virtual void ConnectToServer(const char* Server) = 0;
};

//---------------------------------------------------------------------------

SCF_VERSION (iGameClientConnection, 0, 1, 0);

/**
 * A network connection from the multiplayer client to the server.
 */
struct iGameClientConnection : public iBase
{
  /// Get the according network connection object
  virtual iGameNetworkConnection* GetConnection() = 0;

  /// Gets the name of the Client. (Must be unique for a server)
  virtual const char* GetName() = 0;
};

//---------------------------------------------------------------------------

SCF_VERSION (iGameCore, 0, 1, 0);

/**
 * The csgame core plugin itself
 */
struct iGameCore : public iPlugIn
{

//Temporary hack, to avoid the need to implement any of these interfaces.
//on the long term they are all needed, so the #if should go away
#if 0

  /// Get a pointer to the class repository.
  virtual iEntityClassRepository* GetClassRepository () = 0;

  /// Add an entity to be managed by the game core.
  virtual void AddEntity (iEntity* pEntity) = 0;

  /// Get the entity by the given name, or NULL, if not found.
  virtual iEntity* GetEntity(const char* Name) = 0;

  /// Remove an entity from the game core.
  virtual void RemoveEntity (iEntity* pEntity) = 0;

  /// Creates a 3D sprite from an existing definition
  virtual iGameSprite3D* CreateSprite(const char* Modelname);

  /**
   * Register a 3D sprite model from its textual representation
   * It is assumed that the caller has already set the proper 
   * context for loading, by using pLoader->OpenContext()
   */
  virtual void RegisterSprite3DModel(iDataLoader* pLoader);

  /**
   * Starts moving an entity to a new position and automatically stops the
 * Entity * at that position.  Of course, if there occurs a collision on the
 * way, the * object is stopped also, and the collision is being handled by the
 * involved * entities
   */
  virtual void MoveEntityTo(iEntity*, csVector3 Destination, float speed);

  /**
   * Starts rotating an entity.  The Rotation vector gives the amount of
 * rotation * per second, so by picing smaller numbers you get slower roation
 * and by * picking larger numbers you get faster rotation.
   */
  virtual void RotateEntity(iEntity* pEntity, csVector3 Rotation);

  /// Stops the current movement and rotation of an entity
  virtual void StopEntity(iEntity* pEntiy);

  /**
   * Send an event directly to an entity. This is mostly a convenience 
   * function to avoid searching for an entity and calling HandleEvent 
   * directly. Anyway, using this method is the recommended way of sending 
   * events.
   * You can give a NULL pointer as sender, but this is not recommended.
   */
  virtual void SendEvent(iEntity*        pSender,
                         iEntity*        pTarget, 
                         const char*     EventName,
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
   * You can give a NULL pointer as sender, but this is not recommended.
   */
  virtual void SendDelayedEvent(iEntity*        pSender,
                                iEntity*        pTarget, 
                                const char*     EventName,
                                iAttributeList* InPar,
                                double          delay, 
                                bool            Persistant) = 0;

  /**
   * Send an event directly to all entitie within a radius around a 
   * position.
   */
  virtual void SendEventToGroup(iEntity*        pSender,
                                iPosition*      pCenter, 
                                csVector3       Radius,
                                const char*     EventName,
                                iAttributeList* InPar) = 0;


  /**
   * Revoke an event that has been sent delayed to an entity and that
   * has not yet arrived. If there are multiple Events with the same
   * name, that wait to be delivered, all of them are removed.
   * if pSender is NULL, all events of that type will be revoked, not 
   * just the ones sent by giving a NULL sender!
   */
  virtual void RevokeDelayedEvent(iEntity*        pSender,
                                  iEntity*        pTarget, 
                                  const char*     EventName) = 0;

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

#endif 
};

#endif // __ICSGAME_H__
