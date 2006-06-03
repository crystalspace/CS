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

#ifndef __CS_STREAMLOADER_H__
#define __CS_STREAMLOADER_H__

#include "imap/streamsource.h"
#include "iutil/comp.h"
#include "csutil/scf.h"
#include "csutil/scf_implementation.h"

/**
 * This plugin will listen to a reporter plugin and uses the console,
 * and other output devices to show appropriate messages based on
 * what comes from the reporter plugin.
 */
class csForegroundStreamingLoader : 
	public scfImplementation2<csForegroundStreamingLoader, 
			iStreamSource,
			iComponent>
{
public:
	/// Constructor
	csForegroundStreamingLoader (iBase* iParent);

	/// Destructor
	~csForegroundStreamingLoader ();

	/// Initialize the plugin.
	virtual bool Initialize (iObjectRegistry* object_reg);

	/**
	 * Set the base VFS path from which to retrieve all buffers.
	 */
	void SetBasePath (const char* base);

	/**
	 * Load a buffer given an id. This will fire the callback as soon as
	 * the buffer is ready. Note that some implementations that don't support
	 * asynchronious loading may call the callback immediatelly from within
	 * this function.
	 * \return false if we can't find the buffer (early error). The error
	 * should be placed on the reporter.
	 */
	virtual bool QueryBuffer (const char* id, iStreamDataCallback* callback);

	/**
	 * Save a buffer with some id. Returns false if the buffer couldn't be
	 * saved for some reason. The error should be reported on the reporter
	 * by this function.
	 */
	virtual bool SaveBuffer (const char* id, iDataBuffer* buffer);

private:
	// The base VFS path from which to retrieve all buffers.
	csString basePath;
};

#endif // __CS_STREAMLOADER_H__

