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

#include "sysdef.h"
#include "csengine/sysitf.h"
#include "csengine/cscoll.h"
#include "csengine/thing.h"
#include "csengine/world.h"
#include "csengine/light.h"
#include "csscript/primscri.h"
#include "csutil/token.h"

//---------------------------------------------------------------------------

CmdSequence::CmdSequence  ()
{
  cmds = NULL;
  long_args = NULL;
  flt_args = NULL;
  mat_args = NULL;
  vec_args = NULL;
  seq_args = NULL;
  cso_names = NULL;
}

CmdSequence::~CmdSequence ()
{
  if (cmds) CHKB (delete [] cmds);
  if (long_args) CHKB (delete [] long_args);
  if (flt_args) CHKB (delete [] flt_args);
  if (mat_args) CHKB (delete [] mat_args);
  if (vec_args) CHKB (delete [] vec_args);
  if (seq_args) CHKB (delete [] seq_args);
  if (cso_names) CHKB (delete [] cso_names);
}

void CmdSequence::compile_pass1 (char** buf)
{
  char* t;

  num_cmds = 0;
  num_long_args = 0;
  num_flt_args = 0;
  num_mat_args = 0;
  num_vec_args = 0;
  num_seq_args = 0;
  num_long_vars = 0;
  num_cso_names = 0;

  while (true)
  {
    t = get_token (buf);
    if (t == NULL || *t == ')' || *t == 0 || *t == '}') break;
    if (!strcmp (t, "move"))
    {
      skip_token (buf, "(", "Expected '%s' instead of '%s' after 'move' cmd!\n");
      get_token_float (buf);
      skip_token (buf, ",", "Expected '%s' instead of '%s' in 'move' cmd!\n");
      get_token_float (buf);
      skip_token (buf, ",", "Expected '%s' instead of '%s' in 'move' cmd!\n");
      get_token_float (buf);
      skip_token (buf, ")", "Expected '%s' instead of '%s' to end 'move' cmd!\n");

      num_cmds++;
      num_vec_args++;
      //num_long_vars++;		// For csObject
    }
    else if (!strcmp (t, "dim"))
    {
      get_token_float (buf);
      skip_token (buf, ",", "Expected '%s' instead of '%s' in 'move' cmd!\n");
      get_token_float (buf);
      skip_token (buf, ",", "Expected '%s' instead of '%s' in 'move' cmd!\n");
      get_token_float (buf);
      num_cmds++;
      num_flt_args++;
      num_flt_args++;
      num_flt_args++;
      //num_long_vars++;		// For csObject
    }
    else if (!strcmp (t, "transform"))
    {
      skip_token (buf, "(", "Expected '%s' instead of '%s' after 'transform' cmd!\n");
      t = get_token (buf);
      int bra = 1;
      while (*t)
      {
        if (*t == ')')
	  if (bra <= 1) break;
	  else bra--;
	if (*t == '(') bra++;
        t = get_token (buf);
      }
      num_cmds++;
      num_mat_args++;
      //num_long_vars++;		// For csObject
    }
    else if (!strcmp (t, "forever"))
    {
      skip_token (buf, "{", "Expected '%s' instead of '%s' after 'forever' cmd!\n");
      t = get_token (buf);
      int bra = 1;
      while (*t)
      {
        if (*t == '}')
	  if (bra <= 1) break;
	  else bra--;
	if (*t == '{') bra++;
        t = get_token (buf);
      }
      num_cmds++;
      num_seq_args++;
    }
    else if (!strcmp (t, "loop"))
    {
      get_token_int (buf);
      skip_token (buf, "{", "Expected '%s' instead of '%s' after 'loop' cmd!\n");
      t = get_token (buf);
      int bra = 1;
      while (*t)
      {
        if (*t == '}' && bra <= 1) break;
	if (*t == '{') bra++;
        t = get_token (buf);
      }
      num_cmds++;		// INITLOOP command
      num_long_args++;		// Number of times we should loop
      num_long_args++;		// Index to variable where we keep our loop counter
      num_cmds++;		// LOOP command
      num_long_args++;		// Index to variable where we keep our loop counter
      num_seq_args++;		// Sequence to loop
      num_long_vars++;		// We need one extra loop counter
    }
    else if (!strcmp (t, "wait"))
    {
      num_cmds++;
    }
    else
    {
      CsPrintf (MSG_FATAL_ERROR, "What is '%s' doing in a SCRIPT?\n", t);
      fatal_exit (0, false);
    }
  }

  num_cmds++; // Room for CMD_RETURN

  CHK (cmds = new UByte [num_cmds]);
  if (num_long_args) CHKB (long_args = new long [num_long_args]);
  if (num_flt_args) CHKB (flt_args = new float [num_flt_args]);
  if (num_mat_args) CHKB (mat_args = new csMatrix3 [num_mat_args]);
  if (num_vec_args) CHKB (vec_args = new csVector3 [num_vec_args]);
  if (num_seq_args) CHKB (seq_args = new CmdSequence [num_seq_args]);
  if (num_cso_names) CHKB (cso_names = new csObjName [num_cso_names]);
}

void CmdSequence::compile_pass2 (char** buf)
{
  char* t;

  int idx_cmds = 0;
  int idx_long_args = 0;
  int idx_flt_args = 0;
  int idx_mat_args = 0;
  int idx_vec_args = 0;
  int idx_seq_args = 0;
  int idx_long_vars = 0;

  while (true)
  {
    t = get_token (buf);
    if (t == NULL || *t == ')' || *t == 0 || *t == '}') break;
    if (!strcmp (t, "move"))
    {
      cmds[idx_cmds++] = CMD_MOVE;
      skip_token (buf, "(", "Expected '%s' instead of '%s' after 'move' cmd!\n");
      vec_args[idx_vec_args].x = get_token_float (buf);
      skip_token (buf, ",", "Expected '%s' instead of '%s' in 'move' cmd!\n");
      vec_args[idx_vec_args].y = get_token_float (buf);
      skip_token (buf, ",", "Expected '%s' instead of '%s' in 'move' cmd!\n");
      vec_args[idx_vec_args].z = get_token_float (buf);
      skip_token (buf, ")", "Expected '%s' instead of '%s' to end 'move' cmd!\n");
      idx_vec_args++;
      //long_args[idx_long_args++] = 0; // Reference to main object
    }
    else if (!strcmp (t, "dim"))
    {
      cmds[idx_cmds++] = CMD_DIM;
      flt_args[idx_flt_args++] = get_token_float (buf);
      skip_token (buf, ",", "Expected '%s' instead of '%s' in 'move' cmd!\n");
      flt_args[idx_flt_args++] = get_token_float (buf);
      skip_token (buf, ",", "Expected '%s' instead of '%s' in 'move' cmd!\n");
      flt_args[idx_flt_args++] = get_token_float (buf);
      //long_args[idx_long_args++] = 0; // Reference to main object
    }
    else if (!strcmp (t, "transform"))
    {
      cmds[idx_cmds++] = CMD_TRANSFORM;
      skip_token (buf, "(", "Expected '%s' instead of '%s' after 'transform' cmd!\n");
      csMatrix3 r;
      mat_args[idx_mat_args].Identity ();
      t = get_token (buf);
      while (*t && *t != ')')
      {
	if (!strcmp (t, "rot_x"))
	{
	  float a = get_token_float (buf);
	  a = a * 2*PI / 360.;
	  r.m11 = 1;       r.m12 = 0;       r.m13 = 0;
	  r.m21 = 0;       r.m22 = cos (a); r.m23 = -sin (a);
	  r.m31 = 0;       r.m32 = sin (a); r.m33 = cos (a);
	  r *= mat_args[idx_mat_args];
	  mat_args[idx_mat_args] = r;
	}
	else if (!strcmp (t, "rot_y"))
	{
	  float a = get_token_float (buf);
	  a = a * 2*PI / 360.;
	  r.m11 = cos (a); r.m12 = 0;       r.m13 = -sin (a);
	  r.m21 = 0;       r.m22 = 1;       r.m23 = 0;
	  r.m31 = sin (a); r.m32 = 0;       r.m33 = cos (a);
	  r *= mat_args[idx_mat_args];
	  mat_args[idx_mat_args] = r;
	}
	else if (!strcmp (t, "rot_z"))
	{
	  float a = get_token_float (buf);
	  a = a * 2*PI / 360.;
	  r.m11 = cos (a); r.m12 = -sin (a); r.m13 = 0;
	  r.m21 = sin (a); r.m22 = cos (a);  r.m23 = 0;
	  r.m31 = 0;       r.m32 = 0;        r.m33 = 1;
	  r *= mat_args[idx_mat_args];
	  mat_args[idx_mat_args] = r;
	}
	else
	{
	  CsPrintf (MSG_FATAL_ERROR, "What is '%s' doing in a 'transform' cmd?\n", t);
	  fatal_exit (0, false);
	}
	t = get_token (buf);
      }
      idx_mat_args++;
      //long_args[idx_long_args++] = 0; // Reference to main object
    }
    else if (!strcmp (t, "forever"))
    {
      cmds[idx_cmds++] = CMD_FOREVER;
      skip_token (buf, "{", "Expected '%s' instead of '%s' after 'forever' cmd!\n");
      char* old_buf;
      old_buf = *buf;
      seq_args[idx_seq_args].compile_pass1 (buf);
      *buf = old_buf;
      seq_args[idx_seq_args].compile_pass2 (buf);
      idx_seq_args++;
    }
    else if (!strcmp (t, "loop"))
    {
      cmds[idx_cmds++] = CMD_INITLOOP;
      cmds[idx_cmds++] = CMD_LOOP;
      long_args[idx_long_args++] = get_token_int (buf);
      long_args[idx_long_args++] = idx_long_vars;
      long_args[idx_long_args++] = idx_long_vars;
      skip_token (buf, "{", "Expected '%s' instead of '%s' after 'loop' cmd!\n");
      char* old_buf;
      old_buf = *buf;
      seq_args[idx_seq_args].compile_pass1 (buf);
      *buf = old_buf;
      seq_args[idx_seq_args].compile_pass2 (buf);
      idx_seq_args++;
      idx_long_vars++;
    }
    else if (!strcmp (t, "wait"))
    {
      cmds[idx_cmds++] = CMD_WAIT;
    }
    else
    {
      CsPrintf (MSG_FATAL_ERROR, "What is '%s' doing in a SCRIPT?\n", t);
      fatal_exit (0, false);
    }
  }

  cmds[idx_cmds++] = CMD_RETURN;
}

//---------------------------------------------------------------------------

CSOBJTYPE_IMPL(PrimScript,csScript);

PrimScript::PrimScript (LanguageLayer* layer) : csScript (layer)
{
}

PrimScript::~PrimScript ()
{
}

void PrimScript::load (char** buf)
{
  char* old_buf;

  old_buf = *buf;
  main.compile_pass1 (buf);
  *buf = old_buf;
  main.compile_pass2 (buf);
}

void PrimScript::prepare ()
{
}

csRunScript* PrimScript::run_script (csObject* attached)
{
  CHK (PrimScriptRun* r = new PrimScriptRun (this, attached));
  layer->link_run (r);
  return (csRunScript*)r;
}

//---------------------------------------------------------------------------

CmdSequenceRun::CmdSequenceRun ()
{
  vars = NULL;
}

CmdSequenceRun::~CmdSequenceRun ()
{
  if (vars) CHKB (delete [] vars);
}

void CmdSequenceRun::start (CmdSequence* seq)
{
  cur_cmd = 0;
  cur_long_arg = 0;
  cur_flt_arg = 0;
  cur_mat_arg = 0;
  cur_vec_arg = 0;
  cur_seq_arg = 0;
  if (vars) { CHK (delete [] vars); vars = NULL; }
  if (seq->num_long_vars)
    CHKB (vars = new long [seq->num_long_vars]);
}

//---------------------------------------------------------------------------

PrimScriptRun::PrimScriptRun (PrimScript* script, csObject* object) : csRunScript (script, object)
{
  init ();
}

PrimScriptRun::~PrimScriptRun ()
{
}

void PrimScriptRun::init ()
{
  stack_idx = 0;
  seqr = &stackr[stack_idx];
  seq = stack[stack_idx] = &(((PrimScript*)script)->main);
  seqr->start (seq);
}

bool PrimScriptRun::step ()
{
  for (;;)
  {
    // CsPrintf (MSG_DEBUG, "[%d] Exec cmd [%d]: %d\n", stack_idx, seqr->cur_cmd, seq->cmds[seqr->cur_cmd]);

    switch (seq->cmds[seqr->cur_cmd])
    {
      case CMD_QUIT:
	return true;

      case CMD_RETURN:
	if (stack_idx <= 0) return true;
	stack_idx--;
	seqr = &stackr[stack_idx];
	seq = stack[stack_idx];
	break;

      case CMD_FOREVER:
	{
	  CmdSequence* ns;
	  ns = &(seq->seq_args[seqr->cur_seq_arg]);
	  stack_idx++;
	  seqr = &stackr[stack_idx];
	  seq = stack[stack_idx] = ns;
	  seqr->start (seq);
	}
	break;

      case CMD_INITLOOP:
	{
	  seqr->cur_cmd++;
	  seqr->vars[seq->long_args[seqr->cur_long_arg+1]] = seq->long_args[seqr->cur_long_arg];
	  seqr->cur_long_arg += 2;
	}
	break;

      case CMD_LOOP:
	{
	  if (seqr->vars[seq->long_args[seqr->cur_long_arg]] > 0)
	  {
	    seqr->vars[seq->long_args[seqr->cur_long_arg]]--;

	    CmdSequence* ns;
	    ns = &(seq->seq_args[seqr->cur_seq_arg]);
	    stack_idx++;
	    seqr = &stackr[stack_idx];
	    seq = stack[stack_idx] = ns;
	    seqr->start (seq);
	  }
	  else
	  {
	    seqr->cur_long_arg++;
	    seqr->cur_seq_arg++;
	    seqr->cur_cmd++;
	  }
	}
	break;

      case CMD_DIM:
        seqr->cur_cmd++;
	{
	  csLight* l = NULL;
	  csIdType t = attached->GetType ();
	  if (t == csStatLight::Type() || t == csDynLight::Type())
	    l = (csLight*)attached;
	  else if (attached->GetType () == csCollection::Type())
	  {
	    csCollection* col = (csCollection*)attached;
	    int i;
	    for (i = 0 ; i < col->GetNumObjects () ; i++)
	    {
	      t = (*col)[i]->GetType ();
	      if (t == csStatLight::Type() || t == csDynLight::Type())
	      {
	        l = (csLight*)((*col)[i]);
		break;
	      }
	    }
	  }
	  if (l)
	    layer->set_light_intensity (l, seq->flt_args[seqr->cur_flt_arg],
	    		     seq->flt_args[seqr->cur_flt_arg+1],
	    		     seq->flt_args[seqr->cur_flt_arg+2]);
	  seqr->cur_flt_arg += 3;
	}
	break;

      case CMD_MOVE:
	seqr->cur_cmd++;
	layer->transform (attached, seq->vec_args[seqr->cur_seq_arg++]);
	break;

      case CMD_TRANSFORM:
	seqr->cur_cmd++;
	layer->transform (attached, seq->mat_args[seqr->cur_mat_arg++]);
	break;

      case CMD_WAIT:
	seqr->cur_cmd++;
	return false;
    }
  }
}

void PrimScriptRun::deliver_event (csScriptEvent event)
{
  (void)event;
}

//---------------------------------------------------------------------------
