/*
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

#ifndef PRIMSCRI_H
#define PRIMSCRI_H

#include "csscript/csscript.h"
#include "csgeom/math3d.h"

class PolygonSet;
class PrimScriptRun;
class csObject;
class LanguageLayer;

#define CMD_QUIT 0		// Stop script
#define CMD_MOVE 1		// Move relative with given vector
#define CMD_TRANSFORM 2		// Transform with given matrix
#define CMD_LOOP 3		// Do a loop
#define CMD_FOREVER 4		// Loop forever
#define CMD_WAIT 5		// Wait
#define CMD_RETURN 6		// Return from current sequence (possibly stop script if toplevel)
#define CMD_SCRIPT 7		// Start a script for a local object (not implemented yet)
#define CMD_DIM 8		// Control the appearence of a dynamic light

#define CMD_INITLOOP 100	// Internal command to initialize the loop

typedef char csObjName[30];

/**
 * Command sequence for the primitive scripting support.
 */
class CmdSequence
{
  ///
  friend class PrimScript;
  ///
  friend class CmdSequenceRun;
  ///
  friend class PrimScriptRun;

private:
  /// All commands.
  UByte* cmds;

  /// Long arguments.
  long* long_args;
  /// Float arguments.
  float* flt_args;
  /// Matrix arguments.
  csMatrix3* mat_args;
  /// Vector arguments.
  csVector3* vec_args;
  /// CmdSequences.
  CmdSequence* seq_args;
  /// Names of csObjects that are used in this script..
  csObjName *cso_names;

  ///
  int num_cmds;
  ///
  int num_long_args;
  ///
  int num_flt_args;
  ///
  int num_mat_args;
  ///
  int num_vec_args;
  ///
  int num_seq_args;
  ///
  int num_long_vars;
  ///
  int num_cso_names;

  ///
  void compile_pass1 (char** buf);
  ///
  void compile_pass2 (char** buf);

  ///
  CmdSequence ();
  ///
  ~CmdSequence ();
};

/**
 * Primitive script.
 */
class PrimScript : public csScript
{
  ///
  friend class PrimScriptRun;

private:
  /// Main command sequence for this script.
  CmdSequence main;

public:
  ///
  PrimScript (LanguageLayer* layer);
  ///
  virtual ~PrimScript ();

  /// Load this script from the world file.
  void load (char** buf);

  ///
  virtual void prepare ();
  ///
  virtual csRunScript* run_script (csObject* attached);

  CSOBJTYPE;
};

/**
 * Runtime version of command sequence.
 */
class CmdSequenceRun
{
  ///
  friend class PrimScriptRun;

private:
  ///
  int cur_cmd;
  ///
  int cur_long_arg;
  ///
  int cur_flt_arg;
  ///
  int cur_mat_arg;
  ///
  int cur_vec_arg;
  ///
  int cur_seq_arg;
  /// Local variables.
  long* vars;

  ///
  CmdSequenceRun ();
  ///
  ~CmdSequenceRun ();

  ///
  void start (CmdSequence* seq);
};

/**
 * Every running primitive script will get an instance of this class.
 */
class PrimScriptRun : public csRunScript
{
private:
  /// Double linked list of all running scripts.
  PrimScriptRun* nextR;
  /// Double linked list of all running scripts.
  PrimScriptRun* prevR;

  ///
  CmdSequenceRun stackr[20];
  ///
  CmdSequence* stack[20];
  ///
  int stack_idx;
  ///
  CmdSequenceRun* seqr;
  ///
  CmdSequence* seq;

public:
  ///
  PrimScriptRun (PrimScript* script, csObject* attached);
  ///
  virtual ~PrimScriptRun ();

  ///
  virtual void init ();
  ///
  virtual bool step ();
  ///
  virtual void deliver_event (csScriptEvent event);
};

#endif /*PRIMSCRI_H*/
