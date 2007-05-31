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


bool CheckSupportedInstruction(int iSet)
{
    switch(iSet)
    {
    case 0:
        {
            return (IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE) != 0);
        }
    case 1:
        {
            return (IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE) != 0);
        }
    case 2:
        {
            return (IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != 0);
        }
    case 3:
        {
#define PF_SSE3_INSTRUCTIONS_AVAILABLE 13
            return (IsProcessorFeaturePresent(PF_SSE3_INSTRUCTIONS_AVAILABLE) != 0);
        }
    default:
        {
            return false;
        }
    }
}            