/*
    Copyright (C) 2004 by Eric Sunshine <sunshine@sunshineco.com>

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
 * Clients should #include this file in order to restore some sanity to the
 * global preprocessor namespace after #including MS Windows headers. This file
 * #undefines various MS macros which interfere with method names in our own
 * classes.
 */
#undef DeleteFile
#undef GetClassName
#undef LoadImage
#undef max
#undef min
#undef RegisterClass
#undef SendMessage
#undef UnregisterClass
#undef Yield
