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

#include "csutil/scf.h"
#include "iplugin.h"

/**
 * These are the options you can set for one randomizer. You should not alter
 * any options between CreateWorld() and WriteWorld().
 */
enum {
  DN_DESECTORIZE=0,
  DN_AUTOCONNECTIONS,
  DN_ADDTEXTURES,
  DN_CLEARPRECALC,

  DN_NUM_OPTIONS
};

/**
 * This is a set of statistics variables. You can get a pointer to the
 * creator's statistics by calling GetStatistics();
 */
class dnStats
{
public:
  long NumAreas,NumAreaTemplates,NumAutoCons;
  char **AreaTemplateNames;
  long *AreaTemplates;
  long NumDirectCons,NumAngleCons,NumDAngleCons;
};

SCF_VERSION (iDungeon, 0, 0, 1);

/**
 * This is the randomizer plug-in itself. To use it, first set all options
 * to the values you want, or leave the default values. Then call CreateWorld
 * with the virtual path to the 'dungeon.gen' file (similar to lading a
 * world). After this, don't call SetOption() anymore (if you do, call
 * CreateWorld again. The old world will be invalid). Then call WriteWorld()
 * with the virtual output path as parameter. This path will then be usable
 * as a path to a CS world by your program. Optionally you can then get a
 * pointer to the statistics info.
 */
class iDungeon : public iPlugIn
{
public:
  /// plugin initialization
  virtual bool Initialize(iSystem *sys)=0;

  /// set a randomizer option.
  virtual void SetOption(int opt,int value)=0;

  /// query a randomizer option.
  virtual int GetOption(int opt)=0;

  /**
   * Create a random world. InputDirectory must be a valid VFS directory that
   * contains at least the main dungeon.gen file. Note : calling
   * CreateWorld() twice without WriteWorld in between will waste the first
   * generated world.
   */
  virtual void CreateWorld(const char *InputDirectory)=0;

  /**
   * Write the generated world to a file. OutputDirectory must be a valid VFS
   * directory with write access. It should be empty (no hard restriction),
   * because it will be filled with a text file and several textures to look
   * like a world directory. Note : due to the many small write operations,
   * writing to an archive directory may take some seconds.
   */
  virtual void WriteWorld(const char *OutputDirectory)=0;

  /// get a pointer to the statistics info
  virtual dnStats *GetStatistics()=0;
};
