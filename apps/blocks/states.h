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

#ifndef STATES_H
#define STATES_H

#include "apps/blocks/blocdefs.h"
#include "csutil/bitset.h"


// The number of sections in the encodedData.
#define ST_NUM_ELEMS 4
#define ST_NUM_FLOATS 2
#define ST_NUM_INTS 6
#define ST_NUM_BITS (ZONE_HEIGHT * ZONE_DIM * ZONE_DIM)
#define ST_SIZE_INT 4
#define ST_ENCODED_LENGTH ((ST_NUM_BITS / 8) + 1 + \
(ST_NUM_INTS * ST_SIZE_INT) +21 )


// moving game_cube, set_cube, get_cube into states.


class States
{
  
 public:
  
  States ();
  ~States ();
  
  
  // Current dimensions of game area.
  int zone_dim;
  // New dimensions of game area (set with menu).
  int new_zone_dim;
  
  
  
  // Tells us wheather a cell is occupied. It's padded at both ends along
  // each axis. It is not recomended to access it directly (eg I forgot
  // abotut ZONE_SAFETY, and ka-booom).
  bool game_cube[ZONE_SAFETY+ZONE_DIM+ZONE_SAFETY]
      [ZONE_SAFETY+ZONE_DIM+ZONE_SAFETY]
      [ZONE_SAFETY+ZONE_HEIGHT+ZONE_SAFETY];

  int score;
  
  // If true we are in the process moving cubes/things lower because
  // at least one plane was made.
  bool transition;


   /*
   * How much do we have to move down before we reach another
   * cube-level?
   */
  float move_down_todo;



  // Current speed.
  float speed;
  // Current speed depending on level.
  float cur_speed;

  // position of cube as it is falling down, relative to game_cube... I think.
  int cube_x, cube_y, cube_z;


  // Fog density.
  float fog_density;

  // This is the z of the plane which's dissapearance is handled right now.
  int gone_z;


  // InitStates
  void InitStates();
  
  // initialise game_cube
  void Init_game_cube();

  
  void UpdateScore ();

  
  // Addscore moved from blocks to here.
  //   Take out the reference to  if (screen == SCREEN_GAMEOVER) return; 
  //   It shouldn't be in here.

  void AddScore (int dscore);
  
  

  // Access the cubes in the playing field.
  bool get_cube (int x, int y, int z)
  { return game_cube[x+ZONE_SAFETY][y+ZONE_SAFETY][z+ZONE_SAFETY]; }
  void set_cube (int x, int y, int z, bool v)
  { game_cube[x+ZONE_SAFETY][y+ZONE_SAFETY][z+ZONE_SAFETY] = v; }  


  // Shows if we have a plane at a certain height.
  bool filled_planes[ZONE_HEIGHT];  

  // Checks to see if a plane was formed.
  //   Fills filled_plane with the info about which planes are filled.
  void checkForPlane ();

  // removePlane(z) will remove a plane at level z from this state.
  void removePlane (int z);


  // Check if the play area is empty.
  bool CheckEmptyPlayArea ();
  
  
  // Puts the state into a comma delimited string - encodedData.
  void EncodeStates();
  
  // Decodes encodedData and fills this state with values.
  //   Returns true if the data could be decoded, else false.
  // Does not change data if can not successfully decode a string.
  bool DecodeStates();

  // Prints the encoded data to stdout.
  bool PrintData(char* fileName);

  unsigned char* encodedData;
  
  int lenOfGameCube;

  // position in encodedData that these sections finish.
  int endOfFloats;
  int endOfInts;
  
  
 private:
  
  // Data for encoding decoding.
  int tints[ST_NUM_INTS];
  unsigned char* tbools;
  csBitSet* tempBitSet;
  
};


#endif // STATES_H
