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
 * Optionally, the name of the function to init the token table can be set via
 * CS_INIT_TOKEN_TABLE_NAME, default is 'InitTokenTable'. Note that the 
 * user defines CS_TOKEN_ITEM_FILE and CS_INIT_TOKEN_TABLE_NAME won't be
 * undefined by this file; hence, if you want to build multiple token lists,
 * you have to redefine those macros and include tokenlist.h again.
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
 * #define CS_TOKEN_ITEM_FILE "plugins/proctex/standard/fire.tok"
 * #include "cstool/tokenlist.h"
 * // ...
 * };
 * \endcode
 *
 * fire.cpp:
 * \code
 * csFireLoader::csFireLoader(iBase *p)
 * {
 *   InitTokenTable (tokens);
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

#ifndef CS_INIT_TOKEN_TABLE_NAME
#define CS_INIT_TOKEN_TABLE_NAME_DEFAULT
#define CS_INIT_TOKEN_TABLE_NAME	InitTokenTable
#endif
  
static void CS_INIT_TOKEN_TABLE_NAME(csStringHash& t)
{
  csString s;
#include CS_TOKEN_ITEM_FILE
}
#undef CS_TOKEN_LIST_TOKEN

#ifdef CS_INIT_TOKEN_TABLE_NAME_DEFAULT
#undef CS_INIT_TOKEN_TABLE_NAME
#undef CS_INIT_TOKEN_TABLE_NAME_DEFAULT
#endif

/** @} */

/** @} */

