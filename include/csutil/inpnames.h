/*
    Crystal Space input library
    Copyright (C) 2000 by Andrew Zabolotny <bit@eltech.ru>
    Copyright (C) 2002 by Mathew Sutcliffe <oktal@gmx.co.uk>

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

#ifndef __CS_UTIL_CSINPUTS_H__
#define __CS_UTIL_CSINPUTS_H__

/**\file 
 * Crystal Space input library
 */

#include "iutil/evdefs.h"
#include "iutil/event.h"
#include "csutil/csstring.h"

struct iEvent;

class csInputDefinition
{
protected:
  uint32 modifiersHonored;
  csKeyModifiers modifiers;
  int containedType;
  union
  {
    struct
    {
      utf32_char keyCode;
      bool codeIsCooked;
    } k;
    csEventMouseData m;
    csEventJoystickData j;
  };

  /* @@@ Not implemented, but prolly useful
  int CompareModifiers (const csKeyModifiers& m1, 
    const csKeyModifiers& m2, uint32 honorModifiers);*/
public:
  csInputDefinition ();
  csInputDefinition (iEvent* event);

  void SetHonoredModifiers (uint32 honorModifiers = CSMASK_ALLSHIFTS);
  uint32 GetHonoredModifiers ();

  bool Parse (const char* string, bool useCooked = true);
  csString GetDescription ();
  bool FromEvent (iEvent* event, bool useCookedKey = true);

  uint32 ComputeHash ();
  static uint32 ComputeEventHash (iEvent* event);
  int Compare (const csInputDefinition& def);
  int Compare (iEvent* event);
};

/**
 * Use in `int button' for csevXXXMove events
 * with the backward compatible funcs.
 */
#define CSAXIS_X -1
/**
 * Use in `int button' for csevXXXMove events
 * with the backward compatible funcs.
 */
#define CSAXIS_Y -2

/**
 * Convert a free-format string into a set of values that can be compared
 * against the data of a keyboard event. 
 * \name str Strings are in the form "Ctrl+a", "alt-shift+enter" etc. 
 * \param rawCode Pointer to where the raw code is written to.
 * \param cookedCode Pointer to where the cooked code is written to.
 * \param modifiers Pointer where the key modifiers are written to.
 * \returns Whether the string could be successfully parsed. Error can be
 *  unrecognized keys etc.
 * \remark If you don't want an information to be returned, pass in 0.
 * \remark The cooked code returned *may* be 0. This is the case if
 *  the non-modifier part is a single letter.
 */
extern bool csParseKeyDef (const char* str, utf32_char* rawCode,
  utf32_char* cookedCode, csKeyModifiers* modifiers);

/**
 * Convert a keycode and an optional set of modifiers into a free-form
 * key string.
 * \param code The key code. Is treated as a raw code, however raw vs
 *  cooked doesn't matter here, only when evaluating the data returned
 *  by e.g. csParseKeyString().
 * \param modifiers The modifiers to include in the string. Can be 0.
 * \param distinguishModifiers Whether to out put distinguished modifiers.
 *  (e.g. "LAlt" vs just "Alt".)
 * \returns The key string.
 */
extern csString csGetKeyDesc (utf32_char code, 
			      const csKeyModifiers* modifiers,
			      bool distinguishModifiers = true);

/*
  @@@ TODO:
    csParseMouseDef
    csParseJoystickDef
    csGetKeyDesc
    csGetMouseDesc
    csGetJoystickDesc
 */

#endif // __CS_UTIL_CSINPUTS_H__

