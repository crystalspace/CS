
/*
    (C) 2002 Mat Sutcliffe <oktal@gmx.co.uk>

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

#ifndef __IMAP_META_H__
#define __IMAP_META_H__

struct iObject;

/**
 * The meta-data manager allows arbitrary text to be associated with a
 * map object, for instance for level-editor-specific information which
 * won't be needed in-game.
 * To activate meta manager usage use the functions
 * iLoader::UseMetaManager (iMetaManager *) and
 * iSaver::UseMetaManager (iMetaManager *).
 * The application-writer must write the implementation of the interface.
 * The metadata is saved inside &lt;meta&gt;...&lt;/meta&gt; tags within
 * the object, ie. &lt;meshobj&gt;...&lt;meta&gt;...&lt;/meta&gt;&lt;/meshobj&gt;
 * If UseMetaManager is not called or is called with NULL, all meta
 * tags will be ignored.
 */
struct iMetaManager : public iBase
{
  /// Associate a string with an object.
  /// NULL string means disassociate.
  virtual void Load (iObject *, const char *) = 0;

  /// Get the associate string of the given object.
  /// NULL return value means no associate exists.
  virtual const char* Save (iObject *) = 0;
};

#endif

