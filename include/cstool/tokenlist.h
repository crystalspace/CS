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

#include "csutil/strhash.h"
#include "csutil/csstring.h"
#include "csutil/scopedmutexlock.h"

/**
 * \name Token list helper macros
 * This macros provide a convenient way to automatically create a list of
 * string, every having a unique ID connected to it. The ID for (a lowercase
 * version of) a string is declared as a constant and can thus be used in
 * switch { } statements. A facility to request an ID for a string is also
 * provided. This features make the created token list particular useful for
 * document parsers.
 * <p>
 * To create a token list, put somewhere outside the class where the tokens 
 * are to be used a #CS_TOKEN_LIST_PREPARE(&lt;classname&gt;) statement, and 
 * inside a number of #CS_TOKEN_LIST_TOKEN(&lt;token&gt;)s, surrounded by 
 * #CS_TOKEN_LIST_BEGIN(&lt;classname&gt;) and 
 * #CS_TOKEN_LIST_END(&lt;membername&gt;). The token IDs can be accessed 
 * via &lt;membername&gt;.&lt;token&gt;, an ID for a string can be requested 
 * with &lt;membername&gt;.Lookup().
 * <p>
 * Example:
 * \code
 * CS_TOKEN_LIST_PREPARE(MyParser)
 * class MyParser
 * {
 *     CS_TOKEN_LIST_BEGIN(MyParser)
 *	 CS_TOKEN_LIST_TOKEN(SOURCE)
 *	 CS_TOKEN_LIST_TOKEN(DEST)
 *     CS_TOKEN_LIST_END(tokens)
 *   public:
 *     void Parse ();
 * };
 *
 * void MyParser::Parse()
 * {
 *   ...
 *   const char* tokenFromFile;
 *   csStringID id = tokens.Lookup (tokenFromFile);
 *   switch (id)
 *   {
 *     case tokens.SOURCE:
 *       ...
 *       break;
 *     case tokens.DEST:
 *       ...
 *       break;
 *     default:
 *       ...
 *       break;
 *   }
 *   ...
 * };
 * \endcode
 *
 * @{ */

#define CS_TOKEN_LIST_PREPARE_EXT(Parent, Classname) \
  CS_IMPLEMENT_STATIC_VAR (_tlGet_ ## Parent ## _ ## Classname ## _Hash, \
    csStringHash, ()) \
    static void _tl_ ## Parent ## _ ## Classname ## _KillMutex(); \
    static csMutex* _tl_ ## Parent ## _ ## Classname ## _GetMutex() { \
      static csMutex* m = NULL; \
      if (!m) \
      { \
        csRef<csMutex> t = csMutex::Create (); \
        m = t; m->IncRef(); t = NULL; \
	CS_REGISTER_STATIC_FOR_DESTRUCTION \
	  (_tl_ ## Parent ## _ ## Classname ## _KillMutex); \
      } \
      return m; \
    } \
    static void _tl_ ## Parent ## _ ## Classname ## _KillMutex() { \
      _tl_ ## Parent ## _ ## Classname ## _GetMutex()->DecRef(); \
    }

#define CS_TOKEN_LIST_PREPARE(Parent) \
  CS_TOKEN_LIST_PREPARE_EXT (Parent, _LocalTokenList)

/**
 * Begin a token list.
 * \p Classname is the name of the class containing the token list.
 */
#define CS_TOKEN_LIST_BEGIN_EXT(Parent, Classname) \
  class Classname \
  { \
  public: \
    \
    static inline csStringHash* GetHash() \
    { \
      return _tlGet_ ## Parent ## _ ## Classname ## _Hash(); \
    } \
    static inline csMutex* GetMutex() \
    { \
      return _tl_ ## Parent ## _ ## Classname ## _GetMutex(); \
    } 

/**
 * Begin a token list with a predefined class name.
 * Shortcut in the most occuring case only one token list is required.
 */
#define CS_TOKEN_LIST_BEGIN(Parent) \
  CS_TOKEN_LIST_BEGIN_EXT(Parent, _LocalTokenList)

/**
 * Define a token.
 * Note that the case of the token doesn't matter, it's stored in lowercase
 * internally. However, it's suggested that uppercase is used.
 */
#define CS_TOKEN_LIST_TOKEN(token) \
    static const csStringID token = __LINE__; /* line = unique ID */ \
    struct _tokenClass##token { \
      _tokenClass##token () \
      { \
        csScopedMutexLock lock (GetMutex ()); \
        csString tempStr (#token); tempStr.strlwr();      \
        GetHash()->Register (tempStr, token); \
      }; \
    } _token##token;

/**
 * End a token list.
 * \p Classname must be the same as in the corresponding 
 * #CS_TOKEN_LIST_BEGIN_EXT. \p Membername is the name of the list, usually
 * something like 'tokens'.
 */
#define CS_TOKEN_LIST_END_EXT(Membername, Classname) \
    Classname() \
    { } \
    \
    csStringID Lookup (const char* token) \
    { \
      csScopedMutexLock lock (GetMutex ()); \
      return GetHash()->Request (token); \
    } \
  } Membername;

/**
 * End a token list with a predefined class name.
 * Counterpart to #CS_TOKEN_LIST_BEGIN.
 */
#define CS_TOKEN_LIST_END(Membername) \
  CS_TOKEN_LIST_END_EXT(Membername, _LocalTokenList)
  
/** @} */

/** @} */

#endif
