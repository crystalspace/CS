/*
    The Crystal Space geometry loader interface
    Copyright (C) 2000 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __IMAP_PARSER_H__
#define __IMAP_PARSER_H__

#include "isys/plugin.h"

SCF_VERSION (iLoader, 0, 0, 1);

/**
 * This is a common interface for geometry loaders.
 * It provides methods for loading entire maps as well as separate models.
 */
struct iLoader : public iPlugIn
{
  /**
   * Load the given file (from VFS) into engine.
   * This does not clear anything that could be already loaded there; call
   * ClearAll () if you need it.
   */
  virtual bool Load (const char *iName) = 0;

  /**
   * Parse a string and load the model/s.
   * It is not adviced to make heavy use of this feature; some loaders (e.g.
   * the standard one) can speed up loading by preparsing files and storing the
   * pre-tokenized data somewhere near the original file; if you use this
   * method you will always go through the tokenizer first.  The caller should
   * be prepared for iData to be modified.  This is not always the case (for
   * example the standard loader just temporarily modifies it and restores
   * back) but in any case for performance reasons this memory could be used.
   */
  virtual bool Parse (char *iData) = 0;
};

#endif // __IMAP_PARSER_H__
