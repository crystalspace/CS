/*
    Crystal Space 8-bit software driver assembler-optimized routines
    Copyright (C) 1998 by Jorrit Tyberghein

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

#ifndef __SCANLN8_H__
#define __SCANLN8_H__

#if !defined(OS_LINUX)
extern void draw_sl_map_m68k_big(void) asm("_draw_sl_map_m68k_big");
#define DRAW_SL_MAP_M68K(XX,TW,TH,SHF_U,D,TMAP,INV_Z,U_DIV_Z,V_DIV_Z,M,J,K)	\
({										\
        {									\
                register int _xx1 __asm("d0") = XX;				\
                register int _tw1 __asm("d1") = TW;				\
                register int _th1 __asm("d2") = TH;				\
                register int _shf_u1 __asm("d3") = SHF_U;			\
                register unsigned char *_d1 __asm("a0") = D;			\
                register unsigned char *_tmap2 __asm("a1") = TMAP;		\
                register float _inv_z1 __asm("fp0") = INV_Z;			\
                register float _u_div_z1 __asm("fp1") = U_DIV_Z;		\
                register float _v_div_z1 __asm("fp2") = V_DIV_Z;		\
                register float _m1 __asm("fp3") = M;				\
                register float _j1 __asm("fp4") = J;				\
                register float _k1 __asm("fp5") = K;				\
                __asm volatile ("jsr _draw_sl_map_m68k_big"			\
                : /* no output */						\
                : "r"(_xx1), "r"(_tw1), "r"(_th1), "r"(_shf_u1), "r"(_d1), "r"(_tmap2), \
                  "r"(_inv_z1), "r"(_u_div_z1), "r"(_v_div_z1), "r"(_m1), "r"(_j1), \
                  "r"(_k1)							\
                : "d0", "d1", "a0", "a1", "fp0", "fp1","cc","memory");		\
   }										\
})

#else
// The GCC on my Linux/68k box didn't accept more than 10 parameters to
// __asm statement -- amlaukka@cc.helsinki.fi.

extern void DRAW_SL_MAP_M68K(   int, int, int, int, unsigned char*,
                                unsigned char*, float, float, float,
                                float, float, float)
                                asm("_draw_sl_map_m68k_big");

#endif

#define NO_draw_scanline_map_zfil
void Scan::draw_scanline_map_zfil (int xx, unsigned char* d,
                              unsigned long* z_buf,
                              float inv_z, float u_div_z, float v_div_z)
{
  (void)z_buf;
  DRAW_SL_MAP_M68K (xx, Scan::tw2, Scan::th2, shf_u, d,Scan::tmap2, inv_z,
		    u_div_z, v_div_z, M, J1, K1);
}

#endif // __SCANLN8_H__
