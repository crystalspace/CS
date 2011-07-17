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

struct ResourceReference
{
  // Resource type name.
  csString type;

  // Resource unique ID.
  csString id;

  // The property that refers to this resource.
  csStringID property;
};

struct iResource : public virtual iBase
{
  SCF_INTERFACE(iResource, 1, 0, 0);

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
  virtual csPtr<iResource> Load (iDataBuffer* buf) = 0;
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

struct iLoading;

/**
 * The iResourceCache interface.
 */
struct iResourceCache : public virtual iBase
{
  SCF_INTERFACE (iResourceCache, 1,0,0);
  
  /**
   * Add a given resource to the cache.
   * @param name The resource name.
   */
  virtual void Add (const char* name, iLoading* resource) = 0;

  /**
   * Get a Resource by name.
   * @param name The resource name.
   */
  virtual csRef<iLoading> Get (const char* name) = 0;

  /**
   * Mark a resource as a candidate for removal.
   * @param name The resource name.
   */
  virtual void Release (const char* name) = 0;
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
   * @param name The resource name.
   */
  virtual csRef<iLoading> Get (const char* name) = 0;
};

#endif // __CS_IMAP_RESOURCE_H__
