/*
  OS/2 support for Crystal Space 3D library
  Copyright (C) 1998 by Jorrit Tyberghein
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

#ifndef __SCANCODE_H__
#define __SCANCODE_H__

// Keyboard scan codes
#define SCANCODE_ESC        0x01
#define SCANCODE_1          0x02
#define SCANCODE_2          0x03
#define SCANCODE_3          0x04
#define SCANCODE_4          0x05
#define SCANCODE_5          0x06
#define SCANCODE_6          0x07
#define SCANCODE_7          0x08
#define SCANCODE_8          0x09
#define SCANCODE_9          0x0A
#define SCANCODE_0          0x0B
#define SCANCODE_MINUS      0x0C
#define SCANCODE_PLUS       0x0D
#define SCANCODE_BACKSPACE  0x0E
#define SCANCODE_TAB        0x0F
#define SCANCODE_Q          0x10
#define SCANCODE_W          0x11
#define SCANCODE_E          0x12
#define SCANCODE_R          0x13
#define SCANCODE_T          0x14
#define SCANCODE_Y          0x15
#define SCANCODE_U          0x16
#define SCANCODE_I          0x17
#define SCANCODE_O          0x18
#define SCANCODE_P          0x19
#define SCANCODE_OSBRACKET  0x1A
#define SCANCODE_CSBRACKET  0x1B
#define SCANCODE_ENTER      0x1C
#define SCANCODE_CTRL       0x1D
#define SCANCODE_A          0x1E
#define SCANCODE_S          0x1F
#define SCANCODE_D          0x20
#define SCANCODE_F          0x21
#define SCANCODE_G          0x22
#define SCANCODE_H          0x23
#define SCANCODE_J          0x24
#define SCANCODE_K          0x25
#define SCANCODE_L          0x26
#define SCANCODE_SEMICOLON  0x27
#define SCANCODE_QUOTE      0x28
#define SCANCODE_BQUOTE     0x29
#define SCANCODE_LSHIFT     0x2A
#define SCANCODE_BACKSLASH  0x2B
#define SCANCODE_Z          0x2C
#define SCANCODE_X          0x2D
#define SCANCODE_C          0x2E
#define SCANCODE_V          0x2F
#define SCANCODE_B          0x30
#define SCANCODE_N          0x31
#define SCANCODE_M          0x32
#define SCANCODE_LESS       0x33
#define SCANCODE_GREAT      0x34
#define SCANCODE_SLASH      0x35
#define SCANCODE_RSHIFT     0x36
#define SCANCODE_GRAYAST    0x37
#define SCANCODE_ALT        0x38
#define SCANCODE_SPACE      0x39
#define SCANCODE_CAPSLOCK   0x3A
#define SCANCODE_F1         0x3B
#define SCANCODE_F2         0x3C
#define SCANCODE_F3         0x3D
#define SCANCODE_F4         0x3E
#define SCANCODE_F5         0x3F
#define SCANCODE_F6         0x40
#define SCANCODE_F7         0x41
#define SCANCODE_F8         0x42
#define SCANCODE_F9         0x43
#define SCANCODE_F10        0x44
#define SCANCODE_NUMLOCK    0x45
#define SCANCODE_SCRLOCK    0x46
#define SCANCODE_HOME       0x47
#define SCANCODE_UP         0x48
#define SCANCODE_PGUP       0x49
#define SCANCODE_GRAYMINUS  0x4A
#define SCANCODE_LEFT       0x4B
#define SCANCODE_CENTER     0x4C
#define SCANCODE_RIGHT      0x4D
#define SCANCODE_GRAYPLUS   0x4E
#define SCANCODE_END        0x4F
#define SCANCODE_DOWN       0x50
#define SCANCODE_PGDN       0x51
#define SCANCODE_INS        0x52
#define SCANCODE_DEL        0x53
#define SCANCODE_ALTPRTSCR  0x54
#define SCANCODE_F11        0x57
#define SCANCODE_F12        0x58
#define SCANCODE_EXT        0xE0
// These scancodes are valid only in OS/2
#define SCANCODE_GRAYENTER  0x5A
#define SCANCODE_RCTRL      0x5B
#define SCANCODE_PRINTSCR   0x5D
#define SCANCODE_RALT       0x5E
#define SCANCODE_GRAYHOME   0x60
#define SCANCODE_GRAYUP     0x61
#define SCANCODE_GRAYPGUP   0x62
#define SCANCODE_GRAYLEFT   0x63
#define SCANCODE_GRAYRIGHT  0x64
#define SCANCODE_GRAYEND    0x65
#define SCANCODE_GRAYDOWN   0x66
#define SCANCODE_GRAYPGDN   0x67
#define SCANCODE_GRAYINS    0x68
#define SCANCODE_GRAYDEL    0x69

extern int ScancodeToChar[128];

#endif // __SCANCODE_H__
