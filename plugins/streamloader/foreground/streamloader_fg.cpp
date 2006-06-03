/*
	Copyright (C) 2006 by David H. Bronke

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

#include "cssysdef.h"

#include "streamloader_fg.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csForegroundStreamingLoader)


/// Constructor
csForegroundStreamingLoader::csForegroundStreamingLoader (iBase* iParent) : scfImplementationType (this, iParent)
{
	basePath = "";
}

/// Destructor
csForegroundStreamingLoader::~csForegroundStreamingLoader ()
{
}

/// Initialize the plugin.
bool csForegroundStreamingLoader::Initialize (iObjectRegistry* object_reg)
{
	return true;
}

/**
 * Set the base VFS path from which to retrieve all buffers.
 */
void csForegroundStreamingLoader::SetBasePath (const char* base)
{
}

/**
 * Load a buffer given an id. This will fire the callback as soon as
 * the buffer is ready. Note that some implementations that don't support
 * asynchronious loading may call the callback immediatelly from within
 * this function.
 * \return false if we can't find the buffer (early error). The error
 * should be placed on the reporter.
 */
bool csForegroundStreamingLoader::QueryBuffer (const char* id, iStreamDataCallback* callback)
{
	return false;
}

/**
 * Save a buffer with some id. Returns false if the buffer couldn't be
 * saved for some reason. The error should be reported on the reporter
 * by this function.
 */
bool csForegroundStreamingLoader::SaveBuffer (const char* id, iDataBuffer* buffer)
{
	return false;
}

