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


#include "sysdef.h"
#include "cssys/system.h"
#include "apps/blocks/states.h"


States::States ()
{
  InitStates();
  zone_dim = ZONE_DIM;
  new_zone_dim = ZONE_DIM;
  encodedData = new unsigned char[ST_ENCODED_LENGTH];
  lenOfGameCube = (ZONE_SAFETY+ZONE_HEIGHT+ZONE_SAFETY) * (ZONE_SAFETY+ZONE_DIM+ZONE_SAFETY) * (ZONE_SAFETY+ZONE_DIM+ZONE_SAFETY);
  tempBitSet = new csBitSet(ST_NUM_BITS);
}


States::~States ()
{
  delete encodedData;
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
  int i, j, k;
  for (k = 0 ; k < ZONE_SAFETY+ZONE_HEIGHT+ZONE_SAFETY ; k++)
    for (j = 0 ; j < ZONE_SAFETY+zone_dim+ZONE_SAFETY ; j++)
      for (i = 0 ; i < ZONE_SAFETY+zone_dim+ZONE_SAFETY ; i++)
        game_cube[i][j][k] =
	  i < ZONE_SAFETY || j < ZONE_SAFETY || k < ZONE_SAFETY ||
	  i >= ZONE_SAFETY+zone_dim || j >= ZONE_SAFETY+zone_dim ||
	  k >= ZONE_SAFETY+ZONE_HEIGHT;
}




void States::AddScore (int dscore)
{
//  if (screen == SCREEN_GAMEOVER) return;
  float bonus_fact = (cur_speed - MIN_SPEED) / (MAX_SPEED-MIN_SPEED);
  bonus_fact = bonus_fact*2+1;
  score += (int)((float)dscore*bonus_fact);
}




void States::UpdateScore ()
{
 // if (screen == SCREEN_GAMEOVER) return;
  int increase = 0;
  int i;
	
  for (i=0 ; i<ZONE_HEIGHT ; i++)
  {
    if (filled_planes[i]) increase++;
  }

  AddScore (zone_dim * zone_dim * 10 * increase * increase);
}





void States::checkForPlane ()
{
  bool plane_hit;
  int x,y,z,i;

  // We know nothing yet.
  for (i=0 ; i<ZONE_HEIGHT ; i++) 
    filled_planes[i] = false;

  for (z=0 ; z<ZONE_HEIGHT ; z++)
  {
    plane_hit = true;
    for (x=0 ; x < zone_dim ; x++)
      for (y=0 ; y < zone_dim ; y++)
      {
	// If one cube is missing we don't have a plane.
	if (!get_cube (x, y, z)) { plane_hit = false; break; }
      }
    if (plane_hit)
    {
      // We've got at least one plane, switch to transition mode.
      transition = true;
      filled_planes[z] = true;
      removePlane (z);
      // That's how much all the cubes above the plane will lower.
      move_down_todo = CUBE_DIM;
      
//      if (screen != SCREEN_GAMEOVER)
      UpdateScore ();
      
    }
  }
}





void States::removePlane (int z)
{
  int x,y;
  for (x=0 ; x < zone_dim ; x++)
    for (y=0 ; y < zone_dim ; y++)
    {
      // Remove it from game_cube[][][].
      set_cube (x, y, z, false);
    }
}





bool States::CheckEmptyPlayArea ()
{
  int x, y, z;
  for (z = 0 ; z < ZONE_HEIGHT ; z++)
    for (x = 0 ; x < zone_dim ; x++)
      for (y = 0 ; y < zone_dim ; y++)
        if (get_cube (x, y, z)) return false;
  return true;
}


void States::EncodeStates ()
{
  int i, j, k, l;
  
  sprintf ((char*)encodedData,"%f9,%f9,",
           speed,      // float
	   cur_speed);  // float
  
  i = (int) strlen((char*)encodedData);
  
  // i is at the start of the ints.
    
  
  tints[0] = score;
  tints[1] = cube_x;
  tints[2] = cube_y;
  tints[3] = cube_z;
  tints[4] = zone_dim;
  tints[5] = new_zone_dim;
  
  memcpy(encodedData + i, tints, ST_NUM_INTS * ST_SIZE_INT);  
  
  i = i + (ST_NUM_INTS * ST_SIZE_INT);
  i++;

  encodedData[i] = ','; i++;

  // i here is at the bitset.
  
  
//  csBitSet tempBitSet(ST_NUM_BITS);
  tempBitSet->Reset();

  int pos = 0;

  for (l = ZONE_SAFETY ; l < ZONE_SAFETY+ZONE_HEIGHT; l++)
    for (k = ZONE_SAFETY ; k < ZONE_SAFETY+ZONE_DIM; k++)
      for (j = ZONE_SAFETY ; j < ZONE_SAFETY+ZONE_DIM; j++)
      {
	if(game_cube[j][k][l])
	{
          tempBitSet->Set(pos);
	}
        pos++;
      }

//  unsigned char* temp2;
  tbools = tempBitSet->GetBits();
  
  memcpy(encodedData+i,tbools,tempBitSet->GetByteCount());  
  
}





bool States::DecodeStates ()
{
  // Temporary variables.
  float tspeed;
  float tcur_speed;

  // indexes.
  int i,j,k,l;

  int numSections;

  i = 0;


  // Find the floats.
  for(i = 0, numSections = 0; i < ST_ENCODED_LENGTH; i++)
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
  else
  {
    // copy the floats into temps.
    encodedData[i] = '\0';
    
    if(sscanf((char*)encodedData,"%f,%f",&tspeed,&tcur_speed)
       != ST_NUM_FLOATS)
      return false;
    
    speed = tspeed;
    cur_speed = tcur_speed;
    
    encodedData[i] = ',';
  }

  i++;

  memcpy(tints,encodedData + i,ST_NUM_INTS *4);
  
  score = tints[0];
  cube_x = tints[1];
  cube_y = tints[2];
  cube_z = tints[3];
  zone_dim = tints[4];
  new_zone_dim = tints[5];

  i = i + (ST_NUM_INTS * 4) + 2;

  // here i is at the start of the bitset.

  tbools = tempBitSet->GetBits();
  // i here is just after the ",".
  memcpy(tbools,encodedData + i,(ST_NUM_BITS/8) +1);

  int pos = 0;
  for (l = ZONE_SAFETY ; l < ZONE_SAFETY+ZONE_HEIGHT; l++)
    for (k = ZONE_SAFETY ; k < ZONE_SAFETY+ZONE_DIM; k++)
      for (j = ZONE_SAFETY ; j < ZONE_SAFETY+ZONE_DIM; j++)
      {
        if(tempBitSet->Get(pos))
          game_cube[j][k][l] = true;
        else
          game_cube[j][k][l] = false;
        pos++;
      }

  return true;
}



bool States::PrintData (char* fileName)
{
  printf("\n--------------------------------------------------\n"); 
  printf("%s",encodedData);
  printf("\n--------------------------------------------------\n");
  printf("%f,%f,%d,%d,%d,%d,%d,%d\n", speed, cur_speed, score, cube_x, cube_y, 
	 cube_z, zone_dim, new_zone_dim);
  int i,j,k;

  for (k = 0 ; k < ZONE_SAFETY+ZONE_HEIGHT+ZONE_SAFETY ; k++)
    for (j = 0 ; j < ZONE_SAFETY+ZONE_DIM+ZONE_SAFETY ; j++)
      for (i = 0 ; i < ZONE_SAFETY+ZONE_DIM+ZONE_SAFETY ; i++)
      {
        if(game_cube[i][j][k] == true)
	  printf("1");
	else
	  printf("0");
      }

  //Open Outfile
  FILE* fd = fopen(fileName, "wb");
  if (!fd) return false;

  bool ok = (fwrite(encodedData, ST_ENCODED_LENGTH, 1, fd) != 1);

  fclose(fd);
  return ok;  
}
