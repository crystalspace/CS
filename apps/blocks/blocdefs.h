/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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

#ifndef __BLOC_DEFS_H__
#define __BLOC_DEFS_H__

#undef BLOCKS_NETWORKING

//-----------------------------------------------------------------------------
// All of the defines for blocks.  Some of these will need to be changed so
// we can use them for network stuff.
//-----------------------------------------------------------------------------

#define NUM_EASY_SHAPE   (SHAPE_T1 + 1)
#define NUM_MEDIUM_SHAPE (SHAPE_U + 1)
#define NUM_HARD_SHAPE   (SHAPE_FLATXX + 1)

#define CUBE_DIM 0.4
#define RAST_DIM 0.02

// By zone we mean the space where the shapes move/fall.
#define ZONE_DIM 6
#define ZONE_HEIGHT 15

// When blocks freeze at this height the game is over.
#define GAMEOVER_HEIGHT (ZONE_HEIGHT - 3)

// This is a kind of padding around the world (zone).
#define ZONE_SAFETY 2

// Max cubes in a shape.
#define MAX_CUBES 30

// Maximum speed (fall down speed).
#define MAX_FALL_SPEED 16.0
// Maximum speed in game.
#define MAX_SPEED 8.0
// Slowest speed.
#define MIN_SPEED 0.2

// Menus.
#define MENU_NOVICE     0
#define MENU_AVERAGE    1
#define MENU_EXPERT     2
#define MENU_HIGHSCORES 3
#define MENU_SETUP      4
#define MENU_QUIT       5
#define MENU_3X3        6
#define MENU_4X4        7
#define MENU_5X5        8
#define MENU_6X6        9
#define MENU_KEYCONFIG 10
#define MENU_STARTGAME 11
#define MENU_TOTAL     12 // Total number of menu entries in system.

#define MAX_MENUS      20 // Maximum number of menus visible at same time.

// Screens.
#define SCREEN_STARTUP    1
#define SCREEN_GAME       2
#define SCREEN_KEYCONFIG  3
#define SCREEN_GAMEOVER   4
#define SCREEN_HIGHSCORES 5

// Frequency to check network during network games.
#define NUM_FRAMES_CHK_NET 5

#endif // __BLOC_DEFS_H__
