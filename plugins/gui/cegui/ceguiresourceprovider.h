/*
    Copyright (C) 2005 Seth Yastrov

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

#ifndef _CS_RESOURCEPROVIDER_H_
#define _CS_RESOURCEPROVIDER_H_

/**\file 
*/
/**
* \addtogroup CEGUI
* @{ */

#include "ceguiimports.h"
#include "csutil/ref.h"
#include "iutil/vfs.h"

struct iObjectRegistry;

CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
  /**
   * This is a VFS implementation of the CEGUI::ResourceProvider.
   */
  class ResourceProvider : public CEGUI::ResourceProvider
  {
  public:
    /// Constructor.
    ResourceProvider (iObjectRegistry*);

    /// Destructor.
    virtual ~ResourceProvider ();

    /**
     * Load raw data container using VFS.
     * @param filename VFS path.
     * @param output Raw data container where file will be loaded into.
     * @param resourceGroup Group the loaded data will belong to.
     */
    virtual void loadRawDataContainer (const CEGUI::String& filename,
      CEGUI::RawDataContainer& output, const CEGUI::String& resourceGroup);

    /// Delete the raw data container.
    virtual void unloadRawDataContainer (CEGUI::RawDataContainer& data);

    virtual size_t getResourceGroupFileNames(std::vector<CEGUI::String>& out_vec,
      const CEGUI::String& file_pattern,
      const CEGUI::String& resource_group);

  protected:
    iObjectRegistry* obj_reg;
    csRef<iVFS> vfs;
  };

} CS_PLUGIN_NAMESPACE_END(cegui)

#endif
