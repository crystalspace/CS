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
 * external file, with each token the argument to a '#CS_TOKEN_LIST_TOKEN()'
 * invocation. The name of the file (full path!) has to be put in a macro named
 * CS_TOKEN_ITEM_FILE.  Optionally, the name of the function to initialize the
 * token table can be set via CS_INIT_TOKEN_TABLE_NAME; the default is
 * 'InitTokenTable'.  In addition to invoking the initialization function to
 * populate the string hash, an enumeration is also created. Elements of the
 * enumeration are named XMLTOKEN_FOO (where 'FOO' represents the argument to
 * #CS_TOKEN_LIST_TOKEN()). If you prefer a prefix other than 'XMLTOKEN_',
 * define CS_TOKEN_LIST_TOKEN_PREFIX with the prefix of your choice.  As a
 * convenience, in addition to entries for each #CS_TOKEN_LIST_TOKEN
 * invocation, a final item is added to the enumeration with the name provided
 * by CS_TOKEN_LIST_TOKEN_LAST. If you do not \#define this macro, then the
 * name XMLTOKEN_TOKEN_COUNT is given to the last item in the enumeration.
 * This value will equate to the count of items in the enumeration (not
 * including this automatically added item).  Note that the client defines
 * CS_TOKEN_ITEM_FILE, CS_INIT_TOKEN_TABLE_NAME, CS_TOKEN_LIST_TOKEN_PREFIX,
 * and CS_TOKEN_LIST_TOKEN_LAST, and they will not be undefined by this file;
 * hence, if you want to build multiple token lists, you may redefine those
 * macros and include <cstool/tokenlist.h> again.
 *
 * Example (from a real-world use):
 * fire.tok:
 * \code
 * CS_TOKEN_LIST_TOKEN(PALETTE)
 * ... 
 * \endcode
 *
 * fire.h:
 * \code
 * class csFireLoader
 * {
 *   csStringHash tokens;
 * #define CS_TOKEN_ITEM_FILE "plugins/proctex/standard/fire.tok"
 * #include "cstool/tokenlist.h"
 *   ...
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

/**
 * A token list entry.
 */
#ifndef CS_TOKEN_LIST_TOKEN_PREFIX
#define CS_TOKEN_LIST_TOKEN_PREFIX_DEFAULT
#define CS_TOKEN_LIST_TOKEN_PREFIX XMLTOKEN_
#endif

#ifndef CS_TOKEN_LIST_TOKEN_LAST
#define CS_TOKEN_LIST_TOKEN_LAST_DEFAULT
#define CS_TOKEN_LIST_TOKEN_LAST TOKEN_COUNT
#endif

#undef CS_TOKEN_LIST_PASTE
#undef CS_TOKEN_LIST_PASTE1
#undef CS_TOKEN_LIST_TOKEN
#undef CS_TOKEN_LIST_TOKEN_FINAL
#define CS_TOKEN_LIST_PASTE(X,Y) CS_TOKEN_LIST_PASTE1(X,Y)
#define CS_TOKEN_LIST_PASTE1(X,Y) X ## Y
#define CS_TOKEN_LIST_TOKEN(X) \
  CS_TOKEN_LIST_PASTE(CS_TOKEN_LIST_TOKEN_PREFIX,X),
#define CS_TOKEN_LIST_TOKEN_FINAL(X) \
  CS_TOKEN_LIST_PASTE(CS_TOKEN_LIST_TOKEN_PREFIX,X)

enum {
#include CS_TOKEN_ITEM_FILE
  CS_TOKEN_LIST_TOKEN_FINAL(CS_TOKEN_LIST_TOKEN_LAST)
};

#ifdef CS_TOKEN_LIST_TOKEN_LAST_DEFAULT
#undef CS_TOKEN_LIST_TOKEN_LAST_DEFAULT
#undef CS_TOKEN_LIST_TOKEN_LAST
#endif

#undef CS_TOKEN_LIST_TOKEN
#define CS_TOKEN_LIST_TOKEN(X) s = #X; s.Downcase(); \
  t.Register(s, CS_TOKEN_LIST_PASTE(CS_TOKEN_LIST_TOKEN_PREFIX,X));

#ifndef CS_INIT_TOKEN_TABLE_NAME
#define CS_INIT_TOKEN_TABLE_NAME_DEFAULT
#define CS_INIT_TOKEN_TABLE_NAME InitTokenTable
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

#ifdef CS_TOKEN_LIST_TOKEN_PREFIX_DEFAULT
#undef CS_TOKEN_LIST_TOKEN_PREFIX_DEFAULT
#undef CS_TOKEN_LIST_TOKEN_PREFIX
#endif

/** @} */

/** @} */

