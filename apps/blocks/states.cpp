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

#include "cssysdef.h"
#include "states.h"

States::States ()
{
  zone_dim = ZONE_DIM;
  new_zone_dim = ZONE_DIM;
  InitStates();
}

States::~States ()
{
}

void States::InitStates ()
{
  Init_game_cube();
  score = 0;
  cur_speed = MIN_SPEED;
  speed = 0;
  transition = false;
}

void States::Init_game_cube()
{
  int k, j, i;
  for (k = 0; k < ZONE_SAFETY + ZONE_HEIGHT + ZONE_SAFETY; k++)
    for (j = 0; j < ZONE_SAFETY + zone_dim + ZONE_SAFETY; j++)
      for (i = 0; i < ZONE_SAFETY + zone_dim + ZONE_SAFETY; i++)
        game_cube[i][j][k] =
	  i < ZONE_SAFETY || j < ZONE_SAFETY || k < ZONE_SAFETY ||
	  i >= ZONE_SAFETY + zone_dim || j >= ZONE_SAFETY + zone_dim ||
	  k >= ZONE_SAFETY + ZONE_HEIGHT;
}

void States::AddScore (int dscore)
{
  float bonus_fact = (cur_speed - MIN_SPEED) / (MAX_SPEED - MIN_SPEED);
  bonus_fact = bonus_fact * 2 + 1;
  score += (int)((float)dscore * bonus_fact);
}

void States::UpdateScore ()
{
  int i, increase = 0;
  for (i = 0; i < ZONE_HEIGHT; i++)
    if (filled_planes[i]) increase++;
  AddScore (zone_dim * zone_dim * 10 * increase * increase);
}

void States::checkForPlane ()
{
  int z, x, y;
  for (z = 0; z < ZONE_HEIGHT; z++)
  {
    bool plane_hit = true;
    for (x = 0; plane_hit && x < zone_dim; x++)
      for (y = 0; y < zone_dim; y++)
	if (!get_cube (x, y, z))
	  { plane_hit = false; break; }
    filled_planes[z] = plane_hit;
    if (plane_hit)
    {
      // We've got at least one plane, switch to transition mode.
      transition = true;
      removePlane (z);
      move_down_todo = CUBE_DIM; // Distance cubes above will drop.
      UpdateScore ();
    }
  }
}

void States::removePlane (int z)
{
  int x, y;
  for (x = 0; x < zone_dim; x++)
    for (y = 0; y < zone_dim; y++)
      set_cube (x, y, z, false);
}

bool States::CheckEmptyPlayArea ()
{
  int z, x, y;
  for (z = 0; z < ZONE_HEIGHT; z++)
    for (x = 0; x < zone_dim; x++)
      for (y = 0; y < zone_dim; y++)
        if (get_cube (x, y, z))
	  return false;
  return true;
}

// @@@ FIXME: Fails to take endian into account when transporting integers.
// FIXME: This code needs a major redesign and rewrite.
void States::EncodeStates ()
{
  sprintf ((char*)encodedData,"%f9,%f9,", speed, cur_speed);
  int i = strlen((char*)encodedData);
  // i is at the start of the ints.

  tints[0] = score;
  tints[1] = cube_x;
  tints[2] = cube_y;
  tints[3] = cube_z;
  tints[4] = zone_dim;
  tints[5] = new_zone_dim;

  memcpy(encodedData + i, tints, ST_NUM_INTS * ST_SIZE_INT);

  i += ST_NUM_INTS * ST_SIZE_INT + 1;
  encodedData[i] = ',';
  i++;
  // i here is at the bitset.

//  csBitSet tempBitSet(ST_NUM_BITS);
  tempBitSet->Reset();

  int pos = 0, l, k, j;
  for (l = 0; l < ZONE_HEIGHT; l++)
    for (k = 0; k < ZONE_DIM; k++)
      for (j = 0; j < ZONE_DIM; j++, pos++)
	if (get_cube(j, k, l))
          tempBitSet->Set(pos);
  memcpy(encodedData + i, tempBitSet->GetBits(), tempBitSet->GetByteCount());
}

// @@@ FIXME: Fails to take endian into account when transporting integers.
// FIXME: This code needs a major redesign and rewrite.
bool States::DecodeStates ()
{
  int i, numSections;
  // Find the floats.
  for(i = 0, numSections = 0; i < (int)ST_ENCODED_LENGTH; i++)
  {
    if (encodedData[i] == ',')
    {
      numSections++;
      // If there are 2 floats.
      if(numSections == ST_NUM_FLOATS)
        break;
    }
  }

  if (numSections != ST_NUM_FLOATS)
    return false;

  // copy the floats into temps.
  encodedData[i] = '\0';
  float tspeed, tcur_speed;
  if(sscanf((char*)encodedData,"%f,%f", &tspeed, &tcur_speed) != ST_NUM_FLOATS)
    return false;

  speed = tspeed;
  cur_speed = tcur_speed;

  encodedData[i] = ',';
  i++;

  int tints[ST_NUM_INTS];
  memcpy(tints, encodedData + i, ST_NUM_INTS * ST_SIZE_INT);
  score = tints[0];
  cube_x = tints[1];
  cube_y = tints[2];
  cube_z = tints[3];
  zone_dim = tints[4];
  new_zone_dim = tints[5];

  i += ST_NUM_INTS * ST_SIZE_INT + 2;
  // here i is at the start of the bitset.

  csBitSet tempBitSet(ST_NUM_BITS);
  memcpy(tempBitSet.GetBits(), encodedData + i, tempBitSet.GetByteCount());
  // i here is just after the ",".

  int pos = 0, l, k, j;
  for (l = 0; l < ZONE_HEIGHT; l++)
    for (k = 0; k < ZONE_DIM; k++)
      for (j = 0; j < ZONE_DIM; j++, pos++)
        set_cube(j, k, l, tempBitSet.Get(pos));
  return true;
}

bool States::PrintData (const char* fileName) const
{
  printf("\n--------------------------------------------------\n");
  printf("%s\n",encodedData);
  printf("\n--------------------------------------------------\n");
  printf("%f,%f,%d,%d,%d,%d,%d,%d\n",
    speed, cur_speed, score, cube_x, cube_y, cube_z, zone_dim, new_zone_dim);
  int k, j, i;
  for (k = 0; k < ZONE_SAFETY + ZONE_HEIGHT + ZONE_SAFETY ; k++)
    for (j = 0; j < ZONE_SAFETY + ZONE_DIM + ZONE_SAFETY ; j++)
      for (i = 0; i < ZONE_SAFETY + ZONE_DIM + ZONE_SAFETY ; i++)
        putchar(game_cube[i][j][k] ? '1' : '0');
  putchar('\n');

  FILE* fd = fopen(fileName, "wb");
  if (!fd)
    return false;
  bool ok = (fwrite(encodedData, ST_ENCODED_LENGTH, 1, fd) == 1);
  fclose(fd);
  return ok;
}

#if defined(BLOCKS_NETWORKING)

NetworkStates::NetworkStates ()
{
  StateNumber = 0;
  PreviousStateNumber = -1;
}

NetworkStates::~NetworkStates ()
{
}

//bool NetworkStates::EncodeForNetwork (char * EncodedData, int sizeOfBuffer)
bool NetworkStates::EncodeForNetwork(unsigned char * EncodedData,
				     unsigned char * NetworkData,
			             int sizeOfEncoded, int sizeOfNetwork)
{
// Move the buffer of encoded data forwards so that STARTSTATE can be added.

  // A shorthand.
  unsigned char * tb;
  tb = NetworkData;

//  tb = (char *) malloc(sizeOfBuffer * sizeof(char));

  memcpy(tb+10+ST_SIZE_INT, EncodedData, sizeOfEncoded);

  tb[0] = 'S'; tb[1] = 'T'; tb[2] = 'A'; tb[3] = 'R'; tb[4] = 'T';
  tb[5] = 'S'; tb[6] = 'T'; tb[7] = 'A'; tb[8] = 'T'; tb[9] = 'E';

  tb[sizeOfNetwork-9] = 'E';
  tb[sizeOfNetwork-8] = 'N';
  tb[sizeOfNetwork-7] = 'D';
  tb[sizeOfNetwork-6] = 'S';
  tb[sizeOfNetwork-5] = 'T';
  tb[sizeOfNetwork-4] = 'A';
  tb[sizeOfNetwork-3] = 'T';
  tb[sizeOfNetwork-2] = 'E';

  StateNumber ++;
  PreviousStateNumber++;

  int tempint[1];
  tempint[0] = StateNumber;

  memcpy(tb+10,tempint, ST_SIZE_INT);

  tb[sizeOfNetwork-1] = '\0';

//  memcpy(EncodedData, tb, sizeOfBuffer);
//  free(tb);
  return true;
}

bool NetworkStates::DecodeFromNetwork(unsigned char * NetworkData,
				      int sizeOfBuffer,
				      States * aState)
{
  // First we have to see if there is a full set of network data.
  // Check if the STARTSTATE and ENDSTATE are there.
  if
  (
    !(
    NetworkData[0] == 'S' &&
    NetworkData[1] == 'T' &&
    NetworkData[2] == 'A' &&
    NetworkData[3] == 'R' &&
    NetworkData[4] == 'T' &&
    NetworkData[5] == 'S' &&
    NetworkData[6] == 'T' &&
    NetworkData[7] == 'A' &&
    NetworkData[8] == 'T' &&
    NetworkData[9] == 'E' &&

    NetworkData[sizeOfBuffer-9] == 'E' &&
    NetworkData[sizeOfBuffer-8] == 'N' &&
    NetworkData[sizeOfBuffer-7] == 'D' &&
    NetworkData[sizeOfBuffer-6] == 'S' &&
    NetworkData[sizeOfBuffer-5] == 'T' &&
    NetworkData[sizeOfBuffer-4] == 'A' &&
    NetworkData[sizeOfBuffer-3] == 'T' &&
    NetworkData[sizeOfBuffer-2] == 'E'
    )
  )
  {
    return false;
  }

  // Try to decode the State data in the middle.
  // Get the state number.
  int tempint[1];
  memcpy(tempint, NetworkData+10, ST_SIZE_INT);

//  char * tempStateData;
//  tempStateData = aState->encodedData;

  memcpy(aState->encodedData, NetworkData+10+ST_SIZE_INT,
	 sizeOfBuffer - 8 - (10 + ST_SIZE_INT));

  if(aState->DecodeStates())
  {
    PreviousStateNumber = StateNumber;
    StateNumber = tempint[0];
    return true;
  }
  else
  {
    return false;
  }
}

#endif // BLOCKS_NETWORKING
