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

#include "csextern.h"
#include "iutil/evdefs.h"
#include "iutil/event.h"
#include "csstring.h"

struct iEvent;

// @@@ Document me
class CS_CSUTIL_EXPORT csInputDefinition
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

  /* @@@ Not implemented, but probably useful.
  int CompareModifiers (const csKeyModifiers& m1, 
    const csKeyModifiers& m2, uint32 honorModifiers) const;*/

public:
  csInputDefinition ();
  csInputDefinition (iEvent* event);

  void SetHonoredModifiers (uint32 honorModifiers = CSMASK_ALLSHIFTS);
  uint32 GetHonoredModifiers () const;

  bool Parse (const char* string, bool useCooked = true);
  csString GetDescription () const;
  bool FromEvent (iEvent* event, bool useCookedKey = true);

  uint32 ComputeHash () const;
  static uint32 ComputeEventHash (iEvent* event);
  int Compare (csInputDefinition const&) const;
  int Compare (iEvent*) const;
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
 * Returns the event type described by a free-format string.
 * \param str Strings are in the form "Ctrl+a", "mouse1", "joystickX" etc.
 * \return CSEVTYPE_Keyboard, CSEVTYPE_Mouse, CSEVTYPE_Joystick or 0 if
 *   the string could not be successfully parsed.
 */
CS_CSUTIL_EXPORT int csTypeOfInputDef (const char* str);

/**
 * Convert a free-format string into a set of values that can be compared
 * against the data of a keyboard event. 
 * \name str Strings are in the form "Ctrl+a", "alt-shift+enter" etc. 
 * \param rawCode Pointer to where the raw code is written to.
 * \param cookedCode Pointer to where the cooked code is written to.
 * \param modifiers Pointer where the key modifiers are written to.
 * \return Whether the string could be successfully parsed. Error can be
 *  unrecognized keys etc.
 * \remarks For any piece of information in which you are not interested, pass
 *   0 for the address.
 * \remarks The cooked code returned *may* be 0. This is the case if the
 *   non-modifier part is a single letter.
 */
CS_CSUTIL_EXPORT bool csParseKeyDef (const char* str, utf32_char* rawCode,
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
 * \return The key string.
 */
CS_CSUTIL_EXPORT csString csGetKeyDesc (utf32_char code, 
			      const csKeyModifiers* modifiers,
			      bool distinguishModifiers = true);

/**
 * Convert a free-format string into a set of values that can be compared 
 * against the data of a mouse event.
 * \param str Strings are in the form "mouseX", "mouse2" etc.
 * \param x pointer to where the X value is written;
 *        this will be 1 for mouseX events, 0 otherwise.
 * \param y pointer to where the Y value is written;
 *        this will be 1 for mouseY events, 0 otherwise.
 * \param button pointer to where the button number is written to.
 *        for example, the string "mouse1" results in *button = 1;
 *        this will be -1 if not a mouse button event.
 * \param modifiers pointer to where the modifiers are written to.
 * \return Whether the string could be successfully parsed.
 * \remarks For any piece of information in which you are not interested, pass
 *   0 for the address.
 */
CS_CSUTIL_EXPORT bool csParseMouseDef(const char* str, int* x, int* y, 
                            int* button, csKeyModifiers* modifiers);

/**
 * Convert mouse specifiers and an optional set of modifiers into a free-form
 * mouse event string.
 * \param x If x is not zero, it translates to a MouseX event.
 * \param y If y is not zero, it translates to a MouseY event.
 * \param button If button is not zero, it translates to a Mouse<button> button
 *        event.
 * \param modifiers The modifiers to include in the string. Can be 0.
 * \param distinguishModifiers Whether to out put distinguished modifiers.
 *  (e.g. "LAlt" vs just "Alt".)
 * \return The mouse event string or an empty string if translation failed.
 * \remarks The resulting event is either a MouseX, MouseY or Button event
 *         It will be tested in this order. So setting x and y to nonzero will
 *         result in a MouseX event.
 */
CS_CSUTIL_EXPORT csString csGetMouseDesc (int x, int y, int button,
			      const csKeyModifiers* modifiers,
			      bool distinguishModifiers = true);

/**
 * Convert a free-format string into a set of values that can be compared
 * against the data of a joystick event.
 * \param str Strings are in the form "joystickX", "joystick2" etc.
 * \param x pointer to where the X value is written
 *        this will be 1 for mouseX events, 0 otherwise
 * \param y pointer to where the Y value is written
 *        this will be 1 for mouseY events, 0 otherwise
 * \param button pointer to where the button number is written to.
 *        for example, the string "joystick1" results in *button = 1;
 *        this will be -1 if not a joystick button event.
 * \return Whether the string could be successfully parsed.
 * \remarks For any piece of information in which you are not interested, pass
 *   0 for the address.
 */
CS_CSUTIL_EXPORT bool csParseJoystickDef(const char* str, int* x, int* y,
  int* button, csKeyModifiers* modifiers);

/**
 * Convert joystick specifiers and an optional set of modifiers into a
 * free-form joystick event string.
 * \param x If x is not zero, it translates to a JoystickX event.
 * \param y If y is not zero, it translates to a JoystickY event.
 * \param button If button is not zero, it translates to a Joystick<button> 
 *  button event.
 * \param modifiers The modifiers to include in the string. Can be 0.
 * \param distinguishModifiers Whether to out put distinguished modifiers.
 *  (e.g. "LAlt" vs just "Alt".)
 * \return The joystick event string or an empty string if translation failed.
 * \remarks The resulting event is either a JoystickX, JoystickY or Button
 *   event It will be tested in this order. So setting x and y to nonzero will
 *   result in a JoystickX event.
 */
CS_CSUTIL_EXPORT csString csGetJoystickDesc (int x, int y, int button,
			      const csKeyModifiers* modifiers,
			      bool distinguishModifiers = true);

#endif // __CS_UTIL_CSINPUTS_H__
