/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#ifndef __CS_CSTOOL_TOKENLIST_H__
#define __CS_CSTOOL_TOKENLIST_H__

/**\file
 * Token list helper macros.
 */

/**
 * \addtogroup util
 * @{ */

/**
 * \name Token list helper macros
 * The macros here provide an easy way to automatically build a token list
 * useful for e.g. parsers. The list of tokens have to be declared in an
 * external file, surrounded by '#CS_TOKEN_LIST_TOKEN()'. The name of the
 * file (full path!) has to be put in a macro named CS_TOKEN_ITEM_FILE.
 *
 * Example (from a real-world use):
 * fire.tok:
 * \code
 * CS_TOKEN_LIST_TOKEN(PALETTE)
 * // ... 
 * \endcode
 *
 * fire.h:
 * \code
 * class csFireLoader
 * {
 *  csStringHash tokens;
 * #define CS_TOKEN_ITEM_FILE "proctex/standard/fire.tok"
 * #include "cstool/tokenlist.h"
 * // ...
 * };
 * \endcode
 *
 * fire.cpp:
 * \code
 * csFireLoader::csFireLoader(iBase *p)
 * {
 *   init_token_table (tokens);
 * // ...
 * }
 *
 * csPtr<iBase> csFireLoader::Parse (iDocumentNode* node, 
 * 				     iLoaderContext* ldr_context,
 *   				     iBase* context)
 * {
 * // ...
 *   csStringID id = tokens.Request (child->GetValue ());
 *   switch (id)
 *   {
 *     case XMLTOKEN_PALETTE:
 *       // ...
 *       break;
 *   }
 * // ...
 * }
 * \endcode
 * 
 * @{ */

#undef CS_TOKEN_LIST_TOKEN
/**
 * A token list entry.
 */
#define CS_TOKEN_LIST_TOKEN(X) XMLTOKEN_ ## X,

enum {
#include CS_TOKEN_ITEM_FILE
};

#undef CS_TOKEN_LIST_TOKEN
#define CS_TOKEN_LIST_TOKEN(X) s = #X; s.Downcase(); \
  t.Register(s, XMLTOKEN_ ## X);

static void init_token_table(csStringHash& t)
{
  csString s;
#include CS_TOKEN_ITEM_FILE
}
#undef CS_TOKEN_LIST_TOKEN

/** @} */

/** @} */

#endif
