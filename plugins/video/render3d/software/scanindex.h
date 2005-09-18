/*
    Copyright (C) 2005 by Jorrit Tyberghein
              (C) 2005 by Frank Richter

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

#ifndef __CS_SOFT3D_SCANINDEX_H__
#define __CS_SOFT3D_SCANINDEX_H__

//-------------------------- The indices into arrays of scanline routines ------

/*
 *  The rules for scanproc index name building:
 *  Curly brackets means optional name components
 *  Square brackets denote enforced name components
 *  Everything outside brackets is a must
 *
 *  SCANPROC_{Persp_}{Source_}{Effects_}{Zmode}
 *
 *  Persp       = PI for perspective-incorrect routines
 *  Source      = TEX for non-lightmapped textures
 *                MAP for lightmapped textures
 *                FLAT for flat-shaded
 *                FOG for drawing fog
 *  Effects     = GOU for Gouraud-shading applied to the texture
 *                KEY for "key-color" source pixel removal
 *                FX if routine supports table-driven effects
 *                FXKEY for both FX and KEY effects
 *                ALPHA for alpha-mapped textures
 *  Zmode       = ZUSE for polys that are tested against Z-buffer (and fills)
 *                ZFIL for polys that just fills Z-buffer without testing
 *
 *  Example:
 *      SCANPROC_TEX_ZFIL
 *              scanline procedure for drawing a non-lightmapped
 *              texture with Z-fill
 *  Note:
 *      For easier runtime decisions odd indices use Z buffer
 *      while even indices fills Z-buffer (if appropiate)
 */
#define SCANPROC_FLAT_ZNONE             0x00
#define SCANPROC_FLAT_ZFIL              0x01
#define SCANPROC_FLAT_ZUSE              0x02
#define SCANPROC_FLAT_ZTEST             0x03
#define SCANPROC_MAP_ZNONE              0x04
#define SCANPROC_MAP_ZFIL               0x05
#define SCANPROC_MAP_ZUSE               0x06
#define SCANPROC_MAP_ZTEST              0x07
#define SCANPROC_MAP_KEY_ZNONE          0x08
#define SCANPROC_MAP_KEY_ZFIL           0x09
#define SCANPROC_MAP_KEY_ZUSE           0x0a
#define SCANPROC_MAP_KEY_ZTEST          0x0b
#define SCANPROC_TEX_ZNONE              0x0c
#define SCANPROC_TEX_ZFIL               0x0d
#define SCANPROC_TEX_ZUSE               0x0e
#define SCANPROC_TEX_ZTEST              0x0f
#define SCANPROC_TEX_KEY_ZNONE          0x10
#define SCANPROC_TEX_KEY_ZFIL           0x11
#define SCANPROC_TEX_KEY_ZUSE           0x12
#define SCANPROC_TEX_KEY_ZTEST          0x13
#define SCANPROC_TEX_ALPHA_ZNONE        0x14
#define SCANPROC_TEX_ALPHA_ZFIL         0x15
#define SCANPROC_TEX_ALPHA_ZUSE         0x16
#define SCANPROC_TEX_ALPHA_ZTEST        0x17
#define SCANPROC_MAP_ALPHA_ZNONE        0x18
#define SCANPROC_MAP_ALPHA_ZFIL         0x19
#define SCANPROC_MAP_ALPHA_ZUSE         0x1a
#define SCANPROC_MAP_ALPHA_ZTEST        0x1b
#define SCANPROC_TEX_FX_ZNONE           0x1c
#define SCANPROC_TEX_FX_ZFIL            0x1d
#define SCANPROC_TEX_FX_ZUSE            0x1e
#define SCANPROC_TEX_FX_ZTEST           0x1f
#define SCANPROC_MAP_FX_ZNONE           0x20
#define SCANPROC_MAP_FX_ZFIL            0x21
#define SCANPROC_MAP_FX_ZUSE            0x22
#define SCANPROC_MAP_FX_ZTEST           0x23
// these do not have "zuse" counterparts
#define SCANPROC_ZFIL                   0x24
#define SCANPROC_FOG                    0x25
#define SCANPROC_FOG_VIEW               0x26

// The following routines have a different prototype

// Flat-shaded perspective-incorrect routines
#define SCANPROC_PI_FLAT_ZNONE          0x00
#define SCANPROC_PI_FLAT_ZFIL           0x01
#define SCANPROC_PI_FLAT_ZUSE           0x02
#define SCANPROC_PI_FLAT_ZTEST          0x03
// Textured flat-shaded polygons
#define SCANPROC_PI_TEX_ZNONE           0x04
#define SCANPROC_PI_TEX_ZFIL            0x05
#define SCANPROC_PI_TEX_ZUSE            0x06
#define SCANPROC_PI_TEX_ZTEST           0x07
#define SCANPROC_PI_TEX_KEY_ZNONE       0x08
#define SCANPROC_PI_TEX_KEY_ZFIL        0x09
#define SCANPROC_PI_TEX_KEY_ZUSE        0x0a
#define SCANPROC_PI_TEX_KEY_ZTEST       0x0b
// Textured flat-shaded polygons with tiling
#define SCANPROC_PI_TILE_TEX_ZNONE      0x0c
#define SCANPROC_PI_TILE_TEX_ZFIL       0x0d
#define SCANPROC_PI_TILE_TEX_ZUSE       0x0e
#define SCANPROC_PI_TILE_TEX_ZTEST      0x0f
#define SCANPROC_PI_TILE_TEX_KEY_ZNONE  0x10
#define SCANPROC_PI_TILE_TEX_KEY_ZFIL   0x11
#define SCANPROC_PI_TILE_TEX_KEY_ZUSE   0x12
#define SCANPROC_PI_TILE_TEX_KEY_ZTEST  0x13
// Scanline drawing routines with flat shading + effects.
#define SCANPROC_PI_FLAT_FX_ZNONE       0x14
#define SCANPROC_PI_FLAT_FX_ZFIL        0x15
#define SCANPROC_PI_FLAT_FX_ZUSE        0x16
#define SCANPROC_PI_FLAT_FX_ZTEST       0x17
#define SCANPROC_PI_TEX_FX_ZNONE        0x18
#define SCANPROC_PI_TEX_FX_ZFIL         0x19
#define SCANPROC_PI_TEX_FX_ZUSE         0x1a
#define SCANPROC_PI_TEX_FX_ZTEST        0x1b
#define SCANPROC_PI_TEX_FXKEY_ZNONE     0x1c
#define SCANPROC_PI_TEX_FXKEY_ZFIL      0x1d
#define SCANPROC_PI_TEX_FXKEY_ZUSE      0x1e
#define SCANPROC_PI_TEX_FXKEY_ZTEST     0x1f
#define SCANPROC_PI_TILE_TEX_FX_ZNONE   0x20
#define SCANPROC_PI_TILE_TEX_FX_ZFIL    0x21
#define SCANPROC_PI_TILE_TEX_FX_ZUSE    0x22
#define SCANPROC_PI_TILE_TEX_FX_ZTEST   0x23
#define SCANPROC_PI_TILE_TEX_FXKEY_ZNONE 0x24
#define SCANPROC_PI_TILE_TEX_FXKEY_ZFIL 0x25
#define SCANPROC_PI_TILE_TEX_FXKEY_ZUSE 0x26
#define SCANPROC_PI_TILE_TEX_FXKEY_ZTEST 0x27
// Perspective-incorrect flat-shaded alpha-mapped texture
#define SCANPROC_PI_TEX_ALPHA_ZNONE     0x28
#define SCANPROC_PI_TEX_ALPHA_ZFIL      0x29
#define SCANPROC_PI_TEX_ALPHA_ZUSE      0x2a
#define SCANPROC_PI_TEX_ALPHA_ZTEST     0x2b

// Gouraud-shaded PI routines should have same indices
// as their non-Gouraud counterparts. Every routine except
// flat-shaded ones have two versions: without table-driven
// effects (FX) and one with them.
#define SCANPROC_PI_FLAT_GOU_ZNONE           0x00
#define SCANPROC_PI_FLAT_GOU_ZFIL            0x01
#define SCANPROC_PI_FLAT_GOU_ZUSE            0x02
#define SCANPROC_PI_FLAT_GOU_ZTEST           0x03
// Textured Gouraud-shaded polygons
#define SCANPROC_PI_TEX_GOU_ZNONE            0x04
#define SCANPROC_PI_TEX_GOU_ZFIL             0x05
#define SCANPROC_PI_TEX_GOU_ZUSE             0x06
#define SCANPROC_PI_TEX_GOU_ZTEST            0x07
#define SCANPROC_PI_TEX_GOUKEY_ZNONE         0x08
#define SCANPROC_PI_TEX_GOUKEY_ZFIL          0x09
#define SCANPROC_PI_TEX_GOUKEY_ZUSE          0x0a
#define SCANPROC_PI_TEX_GOUKEY_ZTEST         0x0b
// Textured Gouraud-shaded polygons with tiling
#define SCANPROC_PI_TILE_TEX_GOU_ZNONE       0x0c
#define SCANPROC_PI_TILE_TEX_GOU_ZFIL        0x0d
#define SCANPROC_PI_TILE_TEX_GOU_ZUSE        0x0e
#define SCANPROC_PI_TILE_TEX_GOU_ZTEST       0x0f
#define SCANPROC_PI_TILE_TEX_GOUKEY_ZNONE    0x10
#define SCANPROC_PI_TILE_TEX_GOUKEY_ZFIL     0x11
#define SCANPROC_PI_TILE_TEX_GOUKEY_ZUSE     0x12
#define SCANPROC_PI_TILE_TEX_GOUKEY_ZTEST    0x13
// Scanline drawing routines with Gouraud shading + effects.
#define SCANPROC_PI_FLAT_GOUFX_ZNONE         0x14
#define SCANPROC_PI_FLAT_GOUFX_ZFIL          0x15
#define SCANPROC_PI_FLAT_GOUFX_ZUSE          0x16
#define SCANPROC_PI_FLAT_GOUFX_ZTEST         0x17
#define SCANPROC_PI_TEX_GOUFX_ZNONE          0x18
#define SCANPROC_PI_TEX_GOUFX_ZFIL           0x19
#define SCANPROC_PI_TEX_GOUFX_ZUSE           0x1a
#define SCANPROC_PI_TEX_GOUFX_ZTEST          0x1b
#define SCANPROC_PI_TEX_GOUFXKEY_ZNONE       0x1c
#define SCANPROC_PI_TEX_GOUFXKEY_ZFIL        0x1d
#define SCANPROC_PI_TEX_GOUFXKEY_ZUSE        0x1e
#define SCANPROC_PI_TEX_GOUFXKEY_ZTEST       0x1f
#define SCANPROC_PI_TILE_TEX_GOUFX_ZNONE     0x20
#define SCANPROC_PI_TILE_TEX_GOUFX_ZFIL      0x21
#define SCANPROC_PI_TILE_TEX_GOUFX_ZUSE      0x22
#define SCANPROC_PI_TILE_TEX_GOUFX_ZTEST     0x23
#define SCANPROC_PI_TILE_TEX_GOUFXKEY_ZNONE  0x24
#define SCANPROC_PI_TILE_TEX_GOUFXKEY_ZFIL   0x25
#define SCANPROC_PI_TILE_TEX_GOUFXKEY_ZUSE   0x26
#define SCANPROC_PI_TILE_TEX_GOUFXKEY_ZTEST  0x27

#endif // __CS_SOFT3D_SCANINDEX_H__
