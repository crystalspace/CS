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
struct iAttribute;

SCF_VERSION (iPosition, 0, 1, 0);

/**
 * A position in the Crystal Space world
 */
struct iPosition : public iBase
{
  //Sets the position using traditional access
  virtual SetPosition(csSector*  pSector, csVector3 Pos) = 0;
  
  //Set the position, using an existing position object
  virtual SetPosition(iPosition* pPos) = 0;

  //Move the Position by a relative Vector. 
  //returns false, if that move is not possible
  virtual bool Move  (csVector3 Offset) = 0;

  //Sets a new Position
  virtual void MoveTo(const gePosition& Pos) = 0;

  //Trys to Move the position to the given Vector. That can only be done, 
  //if the direct path is notblocked by any walls. (Sprites dont count 
  //here...)
  //returns false, if that move is not possible
  virtual bool MoveTo(csVector3           Pos) = 0;

  //Sets the current position:
  virtual void MoveTo(csSector* pSector, csVector3 Pos) = 0;

  //Gets the current sector.
  virtual csSector* GetSector() = 0;

  //Gets the current position in the world.
  virtual csVector3 GetPosition() = 0;
};

SCF_VERSION (iAttributeArray, 0, 1, 0);

/**
 * An Array of attributes
 */
struct iAttributeArray : public iBase
{
  //Get the number of entries
  virtual int* GetNum() = 0;

  //Get the entry at the given position, 
  //can return NULL for invalid index.
  virtual iAttribute* GetAt(int Index) = 0;

  //Set the entry at the given position
  //Set to NULL, if you want the array to shrink
  //Setting to an Index that is larger than GetNum()-1 
  //will automatically grow the Array
  virtual void SetAt(int Index, iAttribute* Entry) = 0;
};

SCF_VERSION (iAttribute, 0, 1, 0);

//Definition of the possible types for attributes
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
  //Get the name of the Attribute
  virtual const char* GetName() = 0;

  virtual int GetType() = 0;

  //Get the value as various types
  virtual double           GetFloat()        = 0;
  virtual int              GetInteger()      = 0;
  virtual const char*      GetString()       = 0;
  virtual iEntity*         GetEntity()       = 0;
  virtual iPosition*       GetPosition()     = 0;
  virtual csVector3        GetVector()       = 0;
  virtual iAttributeArray* GetArray()        = 0;

  //Set the value as various types
  virtual void SetFloat   (double Val)           = 0;
  virtual void SetInteger (int Val)              = 0;
  virtual void SetString  (const char* Val)      = 0;
  virtual void SetEntity  (iEntity* Val)         = 0;
  virtual void SetPosition(iPosition* Val)       = 0;
  virtual void SetVector  (csVector3 Val)        = 0;
  virtual void SetArray   (iAttributeArray* Val) = 0;
};

SCF_VERSION (iAttributeList, 0, 1, 0);

/**
 * A list of named attributes and according values
 */
struct iAttributeList : public iBase
{
  //Get the number of entries
  virtual int* GetNum() = 0;

  //Get the entry at the given position, 
  //can return NULL for invalid index.
  virtual iAttribute* GetAt(int Index) = 0;

  //Get the entry with the given name 
  virtual iAttribute* Get(const char* Name) = 0;

  //Add the entry with the given name, use NULL to 
  //remove the entry again.
  virtual void Add(const char* Name, iAttribute* Entry) = 0;
};

SCF_VERSION (iEntity, 0, 1, 0);

/**
 * A game entity.
 */
struct iEntity : public iBase
{
  //Get the name of the Entity
  virtual const char* GetName() = 0;

  //Set the name of the Entity
  virtual const char* SetName() = 0;

  //Get the position of the entity, or NULL if the entity has
  //no position in the 3D world
  virtual iPosition*  GetPosition() = 0;

  /// Return a reference to the polygon mesh for this collider.
  virtual iPolygonMesh* HandleEvent(const char*     EventName, 
                                    iAttributeList* InPar,
                                    iAttributeList* OutPar) = 0;

  ///Get the given Attribute
  virtual iAttribute*  GetAttribute(const char* Name) = 0;

  ///Get the given Tag
  virtual iAttribute*  GetTag(const char* TagKey, const char* Name) = 0;
};

SCF_VERSION (iEntityClass, 0, 1, 0);

/**
 * An Entity class.
 */
struct iEntityClass : public iBase
{
  //Get the name of the Entity-Class (eg: "cs_door"}
  virtual const char* GetName() = 0;

  //Create an Entity of this class
  virtual iEntity* CreateEntity() = 0;
};

SCF_VERSION (iEntityClassRepository, 0, 1, 0);

/**
 * The entity class.repository
 */
struct iEntityClassRepository : public iBase
{
  //Create an Entity of the given classname
  virtual iEntity* CreateEntity(const char* classname) = 0;

  //Get an Registered Entity Class, or NULL if not registered
  virtual iEntityClass* GetEntityClass(const char* classname) = 0;

  //Register an EntityClass. Call with NULL to remove registration
  virtual void RegisterClass(iEntityClass* pEntityClass) = 0;
};

SCF_VERSION (iEntityIterator, 0, 1, 0);

/**
 * The interface for an iterator across Game entites
 */
struct iEntityIterator : public iBase
{
  //Get a pointer to the first matching entity
  virtual iEntity* GetFirst() = 0;

  //Get a pointer to the next matching entity
  virtual iEntity* GetNext() = 0;
};

SCF_VERSION (iGameCore, 0, 1, 0);

/**
 * The csgame core
 */
struct iGameCore : public iBase
{
  //Get a pointer to the class repository
  virtual iEntityClassRepository* GetClassRepository() = 0;

  //Add an entity to be managed by the game core 
  virtual void AddEntity(iEntity* pEntity) = 0;

  //Remove an entity from the game core
  virtual void RemoveEntity(const char* Name) = 0;

  //Get an iterator, that will allow you to iterate across all entities within
  //the specified box
  virtual iEntityIterator* GetAllObjectsWithinBox(iPosition* Center, csVector3 Radius) = 0;
};

#endif // __ICSGAME_H__

