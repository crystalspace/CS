/*
    Copyright (C) 2000-2002 by Jorrit Tyberghein

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

#ifndef __CS_IMAP_LOADER_H__
#define __CS_IMAP_LOADER_H__

#include "csutil/scf.h"

/**\file
 * Loader plugins
 */
/**\addtogroup loadsave
 * @{ */
struct iLoaderContext;
struct iDocumentNode;
struct iStreamSource;

/**
 * This is a plugin for the loader based on document tree.
 */
struct iLoaderPlugin : public virtual iBase
{
  SCF_INTERFACE(iLoaderPlugin, 2,0,0);
  /**
   * Parse a given document node and return a new object for it.
   * \param node is the node to parse.
   * \param ssource is an optional stream source where we can get buffers
   * from in a fast way.
   * \param ldr_context can be used to get the context for the loading.
   * You can use this to find meshes/materials/...
   * \param context is the context in which we are loading (can be the mesh
   * wrapper for meshes for example).
   */
  virtual csPtr<iBase> Parse (iDocumentNode* node,
  	iStreamSource* ssource, iLoaderContext* ldr_context,
  	iBase* context) = 0;
};

/**
 * This is a binary plugin for the loader.
 */
struct iBinaryLoaderPlugin : public virtual iBase
{
  SCF_INTERFACE(iBinaryLoaderPlugin, 2,0,0);
  /**
   * Parse given data and return a new object for it.
   * \param data is the data to parse.
   * \param ssource is an optional stream source where we can get buffers
   * from in a fast way.
   * \param ldr_context can be used to get the context for the loading.
   * You can use this to find meshes/materials/...
   * \param context is the context in which we are loading (can be the mesh
   * wrapper for meshes for example).
   */
  virtual csPtr<iBase> Parse (void* data,
  	iStreamSource* ssource, iLoaderContext* ldr_context,
  	iBase* context) = 0;
};

/** @} */

#endif // __CS_IMAP_LOADER_H__

