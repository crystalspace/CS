/*
    Copyright (C) 2004 by Jorrit Tyberghein

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

#ifndef __LITPARSECFG_H__
#define __LITPARSECFG_H__

#include <stdarg.h>
#include "csutil/strhash.h"
#include "csutil/csstring.h"
#include "iutil/vfs.h"
#include "iutil/objreg.h"
#include "litobjsel.h"
#include "litconfig.h"

struct iObjectRegistry;
class litObjectSelect;
class Lighter;

/**
 * Configuration parser.
 */
class litConfigParser
{
private:
  Lighter* lighter;
  iObjectRegistry* object_reg;

  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE "apps/tools/lighter/lighter.tok"
#include "cstool/tokenlist.h"

  // Find the first element child.
  csRef<iDocumentNode> FindChildNode (iDocumentNode* node);

  bool ParseMulti (iDocumentNode* multi_node,
	litObjectSelectChildren* objsel);
  bool ParseObjectSelect (iDocumentNode* objsel_node,
  	csRef<litObjectSelect>& objsel);
  bool ParseLighter (iDocumentNode* lighter_node,
  	litConfig& litconfig);

public:
  litConfigParser (Lighter* lighter, iObjectRegistry* object_reg);

  /**
   * Parse the given file and set up all the fields.
   * \param vfsfile is the VFS path for the file to parse.
   * \return false on failure. Error will already be reported then.
   */
  bool ParseConfigFile (const char* vfsfile,
  	litConfig& litconfig);
};

#endif // __LITPARSECFG_H__

