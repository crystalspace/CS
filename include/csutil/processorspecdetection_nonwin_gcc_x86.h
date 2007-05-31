/*
Copyright (C) 2007 by Michael Gist

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

#define cpuid(func, ax, bx, cx, dx)\
        __asm__ __volatile__ ("cpuid":\
        "=a" (ax), "=b" (bx), "=c" (cx), "=d" (dx) : "a" (func));

bool CheckSupportedInstruction(int iSet)
{
    int a, b, c, d;
    cpuid(0x1, a, b, c, d);

    switch(iSet)
    {
    case 0:
        {
            return ((d & (1<<23)) != 0);
        }
    case 1:
        {
            return ((d & (1<<25)) != 0);
        }
    case 2:
        {
            return ((d & (1<<26)) != 0);
        }
    case 3:
        {
            return ((c & 1) != 0);
        }
    default:
        {
            return false;
        }
    }
}
