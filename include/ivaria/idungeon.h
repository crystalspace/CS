/*
    DUNGEON plugin interface definition
    Written by Martin Geisse (mgeisse@gmx.net)
 
    This program is free software; you can redistribute it and/or modify 
    it under the terms of the GNU General Public License as published by 
    the Free Software Foundation; either version 2 of the License, or 
    (at your option) any later version. 
 
    This program is distributed in the hope that it will be useful, 
    but WITHOUT ANY WARRANTY; without even the implied warranty of 
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
    GNU General Public License for more details. 
 
    You should have received a copy of the GNU General Public License 
    along with this program; if not, write to the Free Software 
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 
*/

#ifndef __IDUNGEON_H__
#define __IDUNGEON_H__

#include "csutil/scf.h"
#include "isys/iplugin.h"

/**
 * These are the options you can set for one randomizer. You should not alter
 * any options between CreateWorld() and WriteWorld().
 */
enum
{
  DN_DESECTORIZE = 0,
  DN_CROSSCONNECT,
  DN_ADDTEXTURES,
  DN_CLEARPRECALC,
  DN_PRINTLOG,
  DN_TABLENGTH,

  DN_NUM_OPTIONS
};

/**
 * This is a set of statistics variables. You can get a pointer to the
 * creator's statistics by calling GetStatistics();
 * NEVER alter the values you get from there!
 */
struct csDungeonStats
{
  long NumAreas,NumAreaTemplates,NumCrossCons;
  char **AreaTemplateNames;
  long *AreaTemplates;
  long NumDirectCons,NumAngleCons,NumDAngleCons;
};


SCF_VERSION (iDungeon, 2, 0, 0);

/**
 * This is the randomizer plug-in itself. This is how to use it:
 * <ul>
 * <li>1. Set all options via SetOption. These options SHOULD NOT be changed
 *    after this (unexpected results).
 * <li>2. Call PrepareInput with the path to the main dungeon.ini file as a
 *    parameter.  This will read the necessary data for the randomizer.  It
 *    will fill the NumAreaTemplates and AreaTemplateNames fields of the
 *    statistics.  Calling this function more than once will overwrite the old
 *    data.
 * <li>3. Call CreateWorld.  This will run the randomizer and create the world, as
 *    well as fill the remaining fields of the statistics.  You can run it
 *    again if you don't like the result.
 * <li>4. Call WriteWorld to save the generated world in CS map file format.  This
 *    will first clear the precalculated stuff (optionally), then write the
 *    map text file into the given virtual directory and finally copy the
 *    textures (optionally).
 * </ul>
 */
struct iDungeon : public iPlugIn
{
  /// Plugin initialization.
  virtual bool Initialize (iSystem *sys) = 0;

  /// Set a randomizer option.
  virtual void SetOption (int opt,int value) = 0;

  /// Query a randomizer option.
  virtual int GetOption (int opt) = 0;

  /**
   * Read the main dungeon.ini file from the given virtual directory and
   * prepare its contents for the dungeon randomizer.
   */
  virtual void PrepareInput (const char *InputDirectory) = 0;

  /**
   * Create a random world.
   */
  virtual void CreateWorld () = 0;

  /**
   * Write the generated world to a file. OutputDirectory must be a valid VFS
   * directory with write access. It should be empty (no hard restriction),
   * because it will be filled with a text file and several textures to look
   * like a map directory. Note : due to the many small write operations,
   * writing to an archive directory may take some seconds.
   */
  virtual void WriteWorld (const char *OutputDirectory) = 0;

  /// get a pointer to the statistics info
  virtual csDungeonStats *GetStatistics () = 0;
};

#endif // __IDUNGEON_H__
