/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein
    Copyright (C) 1998-2000 by Ivan Avramovic <ivan@avramovic.com>

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

#ifndef __CS_THREADED_LOADER_H__
#define __CS_THREADED_LOADER_H__

#include "csutil/scf_implementation.h"
#include "iutil/comp.h"
#include "imap/loader.h"

struct iCollection;
struct iDocumentNode;
struct iObjectRegistry;

CS_PLUGIN_NAMESPACE_BEGIN(csparser)
{
  class csThreadedLoader : public ThreadedCallable<csThreadedLoader>,
                           public scfImplementation2<csThreadedLoader,
                                                     iThreadedLoader,
                                                     iComponent>
  {
  public:
    csThreadedLoader(iBase *p);
    virtual ~csThreadedLoader();

    virtual bool Initialize(iObjectRegistry *object_reg);
  };

}
CS_PLUGIN_NAMESPACE_END(csparser)

#endif // __CS_THREADED_LOADER_H__
