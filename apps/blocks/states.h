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

#ifndef __BLOCKS_STATES_H__
#define __BLOCKS_STATES_H__

#include "blocdefs.h"
#include "csutil/bitset.h"

// The number of sections in the encodedData.
#define ST_NUM_ELEMS 4
#define ST_NUM_FLOATS 2
#define ST_NUM_INTS 6
#define ST_NUM_BITS (ZONE_HEIGHT * ZONE_DIM * ZONE_DIM)

#define ST_SIZE_INT sizeof(int)
#define ST_ENCODED_LENGTH \
  ((ST_NUM_BITS / 8) + 1 + (ST_NUM_INTS * ST_SIZE_INT) + 21)

// Have to include the '\0' character as well.
#define ST_CLIENT_EXTRA (19 + ST_SIZE_INT)
#define ST_SERVER_EXTRA (19 + ST_SIZE_INT)

// moving game_cube, set_cube, get_cube into states.

/**
 * Player state object.
 */
class States
{
public:
  /// Current dimensions of game area.
  int zone_dim;
  /// New dimensions of game area (set with menu).
  int new_zone_dim;

  /**
   * Tells us wheather a cell is occupied. It's padded at both ends along
   * each axis. It is not recomended to access it directly since it is easy
   * to forget about the safety zone.
   */
  bool game_cube[ZONE_SAFETY + ZONE_DIM + ZONE_SAFETY]
                [ZONE_SAFETY + ZONE_DIM + ZONE_SAFETY]
                [ZONE_SAFETY + ZONE_HEIGHT + ZONE_SAFETY];

  /// Shows if we have a complete plane at a certain height.
  bool filled_planes[ZONE_HEIGHT];

  /// Score.
  int score;

  /**
   * If true we are in the process moving cubes/things lower because at least
   * one plane was made.
   */
  bool transition;

  /// How much do we have to move down before we reach another cube-level?
  float move_down_todo;

  /// Current speed.
  float speed;
  /// Current speed depending on level.
  float cur_speed;

  /// Position of cube as it is falling down, relative to game_cube.
  int cube_x, cube_y, cube_z;

  /// Fog density.
  float fog_density;

  /// Z value of the plane whose dissapearance is being handled right now.
  int gone_z;

  /// The encoded data.
  unsigned char* encodedData;
  // unsigned char encodedData[ST_ENCODED_LENGTH];

public:
  /// Constructor.
  States ();
  /// Destructor.
  ~States ();

  /// Initialize the states.
  void InitStates();
  /// Initialise game cube.
  void Init_game_cube();

  /// Update score.
  void UpdateScore ();
  /// Add score moved from blocks to here.
  void AddScore (int dscore);

  /// Does cube occupy playing coordinate x, y, z?
  bool get_cube (int x, int y, int z)
  { return game_cube[x + ZONE_SAFETY][y + ZONE_SAFETY][z + ZONE_SAFETY]; }
  /// Set or clear occupancy of coordinate x, y, z.
  void set_cube (int x, int y, int z, bool v)
  { game_cube[x + ZONE_SAFETY][y + ZONE_SAFETY][z + ZONE_SAFETY] = v; }

  /**
   * Checks to see if a plane was formed.<p>
   * Fills filled_plane with the info about which planes are filled.
   */
  void checkForPlane ();

  /// Remove a plane at level Z from this state.
  void removePlane (int z);

  /// Check if the play area is empty.
  bool CheckEmptyPlayArea ();

  /// Encode the state into a comma delimited string in buffer 'encodedData'.
  void EncodeStates();

  /**
   * Decode 'encodedData' and fill this state with its contents.
   * Returns 'true' if the data could be decoded, else 'false'.
   * Does not change data if can not successfully decode a string.
   */
  bool DecodeStates();

  /// Print the encoded data to 'stdout'.
  bool PrintData(const char* fileName) const;

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

#if defined(BLOCKS_NETWORKING)

// -----------------------------------------------------------------------
//  This is a description of the protocol.
//
// client
// When receive: "SEND <State stateNumber>"
// Send: "STARTSTATE<int stateNumber><States currentState>ENDSTATE"
//
// Server
// When receive: "STARTSTATE<int stateNumber><States currentState>ENDSTATE"
// Send: "SEND <State stateNumber+1>"
//
// "BYE" other blocks disconnects.
//

class NetworkStates
{
 public:
  NetworkStates ();
  ~NetworkStates ();

  // Returns false if it can't encode the data for what ever reason.
  //  Increments the StateNumber everytime it is called.
  //  Sets the PreviousStateNumber to the current StateNumber.
  // EncodedData is a pointer to a state which has been encoded.
  bool EncodeForNetwork(unsigned char * EncodedData,
			unsigned char * NetworkData,
			int sizeOfEncoded, int sizeOfNetwork);

  // Returns false if it can't decode the data for what ever reason.
  //  Sets the StateNumber to the data it recieves from the net.
  //  Sets the PreviousStateNumber to the current StateNumber.
  bool DecodeFromNetwork(unsigned char * NetworkData, int sizeOfBuffer,
			 States * aState);

  // Returns the number of the state.
  int GetStateNumber();

  // Returns the state number what was around before StateNumber.
 private:
  int StateNumber;
  int PreviousStateNumber;
};

#endif // BLOCKS_NETWORKING

#endif // __BLOCKS_STATES_H__
