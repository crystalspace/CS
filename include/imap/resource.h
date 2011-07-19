/*
  Copyright (C) 2011 by Michael Gist

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

#ifndef __CS_IMAP_RESOURCE_H__
#define __CS_IMAP_RESOURCE_H__

#include "csutil/csstring.h"
#include "csutil/refarr.h"
#include "iutil/document.h"
#include "iutil/strset.h"

namespace CS
{
  namespace Resource
  {
    typedef int TypeID;
  }
}

struct ResourceReference
{
  // Resource name (unique within a type).
  csString id;

  // Resource type ID.
  CS::Resource::TypeID typeID;

  // The property that refers to this resource.
  csStringID property;
};

struct iResource : public virtual iBase
{
  SCF_INTERFACE(iResource, 1, 0, 0);

  virtual const CS::Resource::TypeID GetTypeID () const = 0;

  virtual const csArray<ResourceReference>& GetDependencies () const = 0;

  virtual void SetProperty (csStringID propery, iResource* resource) = 0;
};

struct iResourceLoader : public virtual iBase
{
  SCF_INTERFACE(iResourceLoader, 1, 0, 0);

  /**
   * Loads a resource from a document node.
   * Returns an invalid object on failure.
   */
  virtual csPtr<iResource> Load (iDocumentNode* node) = 0;

  /**
   * Loads a resource from a data buffer.
   * Returns an invalid object on failure.
   */
  virtual csPtr<iResource> Load (CS::Resource::TypeID typeID, iDataBuffer* buf) = 0;
};

struct iResourceSaver : public virtual iBase
{
  SCF_INTERFACE(iResourceSaver, 1, 0, 0);

  /**
   * Saves a resource to a document node.
   * Returns success.
   */
  virtual bool Save (iResource* resource, iDocumentNode* node) = 0;

  /**
   * Saves a resource to a data buffer.
   * Returns an invalid object on failure.
   */
  virtual csPtr<iDataBuffer> Save (iResource* resource) = 0;
};

struct iLoadingResource;
/**
 * The iResourceListener interface.
 */
struct iResourceListener : public virtual iBase
{
  SCF_INTERFACE (iResourceListener, 1,0,0);
  virtual void OnLoaded (iLoadingResource* resource) = 0;
};

/**
 * The iLoading interface.
 */
struct iLoadingResource : public virtual iBase
{
  SCF_INTERFACE (iLoadingResource, 1,0,0);
  virtual const char* GetName() = 0;
  virtual bool Ready () const = 0;
  virtual csRef<iResource> Get() = 0;
  /**
   * The OnLoaded will always be trigger in the mainloop
   * so you don't have to worry about locking the engine 
   * or the resource itself.
   */
  virtual void AddListener(iResourceListener* listener) = 0;
  virtual void RemoveListener(iResourceListener* listener) = 0;

protected:
  friend void iResourceTrigger(iLoadingResource*);
  virtual void TriggerCallback() = 0;
};


/**
 * The iResourceCache interface.
 */
struct iResourceCache : public virtual iBase
{
  SCF_INTERFACE (iResourceCache, 1,0,0);
  
  /**
   * Add a given resource to the cache.
   * @param typeID The resource type ID.
   * @param name The resource name.
   */
  virtual void Add (CS::Resource::TypeID typeID, const char* name, iLoadingResource* resource) = 0;

  /**
   * Get a Resource by name.
   * @param typeID The resource type ID.
   * @param name The resource name.
   */
  virtual csRef<iLoadingResource> Get (CS::Resource::TypeID typeID, const char* name) = 0;

  /**
   * Mark a resource as a candidate for removal.
   * @param typeID The resource type ID.
   * @param name The resource name.
   */
  virtual void Release (CS::Resource::TypeID typeID, const char* name) = 0;
};

/**
 * The iResourceManager interface.
 */
struct iResourceManager : public virtual iBase
{
  SCF_INTERFACE (iResourceManager, 1,0,0);

  /**
   * Get a Resource by name.
   * Begins loading the resource if not already loading or loaded.
   * @param typeID The resource type ID.
   * @param name The resource name.
   */
  virtual csRef<iLoadingResource> Get (CS::Resource::TypeID typeID, const char* name) = 0;
  
  /**
   * Add a given resource to the manager.
   * @param resource.
   */
  virtual void Add (iResource* resource) = 0;

  /**
   * Mark a resource as a candidate for removal.
   * Calling this function makes no guarantees as to the removal of the resource.
   * @param typeID The resource type ID.
   * @param name The resource name.
   */
  virtual void Remove (iResource* resource) = 0;
};

#endif // __CS_IMAP_RESOURCE_H__
