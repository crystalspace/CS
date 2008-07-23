/*
    Copyright (C) 2008 by Eric Sunshine <sunshine@sunshineco.com>

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
 * global preprocessor namespace after #including OpenSolaris system headers.
 * This file #undefines various OpenSolaris macros which interfere with our own
 * symbols.
 */

// Pollution from <sys/regset.h> which gets included (very) indirectly by
// #include <stdlib.h>.
#undef CS
#undef DS
#undef EAX
#undef EBP
#undef EBX
#undef ECX
#undef EDI
#undef EDX
#undef EFL
#undef EIP
#undef ERR
#undef ES
#undef ESI
#undef ESP
#undef FS
#undef GS
#undef SS
#undef TRAPNO
#undef UESP
