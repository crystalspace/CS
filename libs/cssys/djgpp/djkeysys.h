/*
    DOS support for Crystal Space 3D library
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by David N. Arnold <derek_arnold@fuse.net>
    Written by Andrew Zabolotny <bit@eltech.ru>

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

/*
 *      Originally written by :
 *            Martynas Kunigelis, 1996/13/05
 *            e-mail:  Martynas.Kunigelis@vm.ktu.lt
 *      Translated to C++ class functions by:
 *              David N. Arnold <derek_arnold@fuse.net>
 */
#ifndef __DJKEYSYS_H__
#define __DJKEYSYS_H__

/// Simple keyboard handling helper class for SysKeyboardDriver
class KeyboardHandler
{
public:
  /// Install a handler in IRQ1 chain
  int install ();
  /// Deinstall our handler from keyboard handler chain
  void uninstall ();
  /// Allow (toggle == 1) or disallow passthrough to std keyboard handler
  void chain (int toggle);
};

#endif // __DJKEYSYS_H__
