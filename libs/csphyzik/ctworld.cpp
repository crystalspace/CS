/*
    Dynamics/Kinematics modeling and simulation library.
    Copyright (C) 1999 by Michael Alexander Ewert

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
#include "csphyzik/articula.h"
#include "csphyzik/world.h"
#include "csphyzik/rigidbod.h"
#include "csphyzik/debug.h"
#include "csphyzik/ctcat.h"
#include "csphyzik/contact.h"

//#define __CT_NOREWINDENABLED__

static ctWorld *gcurrent_world = NULL;

static void __ctworld_dydt(real t, const real y[], real dy[]) 
{
  gcurrent_world->dydt_eval (t, y, dy);
}

void ctWorld::dydt_eval (real t, const real y[], real dy[] )
{
  //  assert_goto( gcurrent_world != NULL, "current world NULL in dydt\n",
  //               ASSERTFAIL );
  
  gcurrent_world->calc_delta_state ( t, y, dy ); 

  //ASSERTFAIL:
}

ctWorld::ctWorld ()
{
  ctArticulatedBody::set_joint_friction ( DEFAULT_JOINT_FRICTION );

  fsm_state = CTWS_NORMAL;
  // default
  ode_to_math = new OdeRungaKutta4 ();

  max_state_size = DEFAULT_INIT_MAX_STATE_SIZE;
  y0 = new real[max_state_size];
  y1 = new real[max_state_size];
  y_save = new real[max_state_size];
  y_save_size = 0;
  was_catastrophe_last_frame = false;
  max_time_subdivisions = 10;
}

//!me delete _lists and ode
ctWorld::~ctWorld()
{
  if ( ode_to_math ) 
    delete ode_to_math;
  //!me lists delete here
  delete [] y0;
  delete [] y1;
  delete [] y_save;
}

void ctWorld::calc_delta_state ( real t, const real y[], real dy[] ) 
{
  // move data from y array into all entities in this world
  reintegrate_state ( y );

  // zero out force accumulator and what-not
  init_state ();

  // solve forces and torques of bodies in this world
  solve ( t );

  // load all delta step data from bodies into ydot
  load_delta_state ( dy );
}

// zero out force accumulator and other transient variables
void ctWorld::init_state ()
{
  ctEntity *pe;

  pe = body_list.get_first ();
  while ( pe )
  {
    pe->init_state ();
    pe = body_list.get_next ();
  }
}

void ctWorld::resize_state_vector ( long new_size )
{
  delete [] y0;
  delete [] y1;
  delete [] y_save;
  max_state_size = new_size + STATE_RESIZE_EXTRA;
  y0 = new real[max_state_size];
  y1 = new real[max_state_size];
  y_save = new real[max_state_size];
}

void ctWorld::register_catastrophe_manager ( ctCatastropheManager *pcm )
{
  if ( pcm != NULL )
    catastrophe_list.add_link ( pcm );
}
 
// calculate new positions of world objects after time dt
errorcode ctWorld::do_time_step ( real t0, real t1 )
{
  long arr_size = 0;
  ctEntity *pe = body_list.get_first ();

  while ( pe )
  {
    arr_size += pe->get_state_size ();
    pe = body_list.get_next ();
  }

  gcurrent_world = this;
  
  if ( arr_size > max_state_size )
    resize_state_vector( arr_size );

  load_state ( y0 );
  
  // save this state for posible rewinding
  for ( int i = 0; i < arr_size; i++ )
    y_save[i] = y0[i];
 
  y_save_size = arr_size;

  if ( ode_to_math )
    ode_to_math->calc_step ( y0, y1, arr_size, t0, t1, __ctworld_dydt );
  else
  {
    if ( fsm_state == CTWS_REWOUND && t1 >= rewound_from )
    {
      fsm_state = CTWS_NORMAL;
      rewound_from = 0;
    }
    //!me boom!  no ode, so get out.
    return WORLD_ERR_NOODE;
  }

  reintegrate_state ( y1 );

  gcurrent_world = NULL;
  
  if ( fsm_state == CTWS_REWOUND && t1 >= rewound_from )
  {
    fsm_state = CTWS_NORMAL;
    rewound_from = 0;
  }
  
  return WORLD_NOERR;
}

void fcn_set_no_rewind ( ctEntity *ppe )
{
  ppe->set_rewind (false);
}

// using sloooooooow but accurate collision detection.  ( collision = catastrophe )
// this can be sped up in many ways.
// - use eulers method or mid-point for ODE just during CD
// - only evolve objects related to colliding objects when searching for c-time
// - don't forget to handle 'tunneling' problem as well.
errorcode ctWorld::evolve ( real t1, real t2 )
{
  real ta, tb;
  ctCatastropheManager *cat;
  ctLinkList<ctCatastropheManager> *this_slice_cat = new ctLinkList<ctCatastropheManager>();
  ctLinkList<ctCatastropheManager> *recent_cat = new ctLinkList<ctCatastropheManager>();
  ctLinkList<ctCatastropheManager> *swap_cat;

  //make sure we don't go into an infinite loop
  long loops = max_time_subdivisions; 

  ta = t1;
  tb = t2;
  real max_cat_dist, this_cat_dist;
  bool is_unhandled_catastrophe = false;
  bool simul_check_done = false;
  //!me no rewind option is probably a bad idea...
#ifdef __CT_NOREWINDENABLED__
  apply_function_to_body_list ( fcn_set_no_rewind );
#endif

  while ( ta < t2 && loops-- > 0)
  {
    do_time_step ( ta, tb );

    max_cat_dist = 0;
    this_slice_cat->remove_all ();
    cat = catastrophe_list.get_first ();
    bool no_great_cat_dist = true;
    while ( cat != NULL )
    {
      this_cat_dist = cat->check_catastrophe ();
      if ( this_cat_dist > 0 )
      {
        this_slice_cat->add_link ( cat );
        if ( this_cat_dist > max_cat_dist )
          max_cat_dist = this_cat_dist;

        if ( this_cat_dist > cat->get_epsilon () )
          no_great_cat_dist = false;
      }
      cat = catastrophe_list.get_next ();
    }
    
    // if there is a catastrophe
    if ( max_cat_dist > 0 )
    {
      is_unhandled_catastrophe = true;
      swap_cat = recent_cat;
      recent_cat = this_slice_cat;
      this_slice_cat = swap_cat;
      // bisect time backwards to search for time of impact
      rewind (ta, tb);   // rewind state back to ta

      // if there was a collision last frame it is likely followed by another
      // one almost immediately.
      if ( ta == t1 && was_catastrophe_last_frame && !simul_check_done )
      {
	tb = ta + 2.0*TIME_EPSILON;
	simul_check_done = true;
      }
      else
	tb -= (tb - ta)*0.5;
    // if we have not arrived at the end of our time interval 
    }
    else if ( is_unhandled_catastrophe )
    {
      // search forward in time for time of impact for collision(s)
      real last_ta = ta;

      // we have found a time close to when catastrpohe happened
      if ( ( tb - ta ) <= TIME_EPSILON )
      {  
        // resolve all catastrophes that occurred at this approx point in time.
        cat = recent_cat->get_first ();
        while ( cat )
	{
          cat->handle_catastrophe();
          cat = recent_cat->get_next();
        }
        recent_cat->remove_all();
        was_catastrophe_last_frame = is_unhandled_catastrophe;
        is_unhandled_catastrophe = false;

        tb = t2;
      }
      else
      {
        ta = tb;
        tb += (tb - last_ta)*0.5;
        if ( tb > t2 )
          tb = t2;
      }
    }
    else
    {  // we are finished, there were no catasprophes
      ta = tb;
      tb = t2;
    }
  }

  // we barfed out without handling catastrophe, 
  // so let's do the best we can in an ugly situation.
  if ( is_unhandled_catastrophe )
  {
    // evolve to the end of our time-slice
    tb = t2;
    do_time_step ( ta, tb );
    // should check for catastrophes
    this_slice_cat->remove_all ();
    cat = catastrophe_list.get_first ();
    while ( cat != NULL )
    {
      this_cat_dist = cat->check_catastrophe ();
      if ( this_cat_dist > 0 )
        this_slice_cat->add_link ( cat );

      cat = catastrophe_list.get_next();
    }

    // resolve all catastrophes that occured, 
    // not caring about getting the proper time-resolution.
    cat = this_slice_cat->get_first ();
    while ( cat )
    {
      cat->handle_catastrophe ();
      cat = this_slice_cat->get_next ();
    }
    this_slice_cat->remove_all ();
    was_catastrophe_last_frame = is_unhandled_catastrophe;
    is_unhandled_catastrophe = false;
  }

  delete this_slice_cat;
  delete recent_cat;

  return WORLD_NOERR;
}

errorcode ctWorld::rewind ( real /*t1*/, real t2 )
{
  fsm_state = CTWS_REWOUND;
  rewound_from = t2;
  reintegrate_state ( y_save );
  return WORLD_NOERR;
}

// find and resolve collisions
void ctWorld::collide ()
{

}


// resolve forces on all bodies
void ctWorld::solve ( real t )
{
  ctEntity *pe;
  ctForce *frc;

  // Solve for forces affecting contents of world.  The order matters.

  // First apply all environmental forces of this world to bodies
  frc = enviro_force_list.get_first ();
  while ( frc )
  {
    pe = body_list.get_first ();
    while ( pe )
    {
      if ( !(fsm_state == CTWS_REWOUND && (pe->flags & CTF_NOREWIND)) )
        pe->apply_given_F ( *frc );
      pe = body_list.get_next ();
    }

    frc = enviro_force_list.get_next ();
  }

  pe = body_list.get_first ();
  while ( pe )
  {
    if ( !(fsm_state == CTWS_REWOUND && (pe->flags & CTF_NOREWIND)) )
      pe->solve (t);
    pe = body_list.get_next ();
  }

}


// state to array
void ctWorld::load_state ( real *state_array )
{
  ctEntity *pe;
  long state_size;

  pe = body_list.get_first ();
  while ( pe )
  {
    if ( fsm_state == CTWS_REWOUND && (pe->flags & CTF_NOREWIND) )
      state_size = pe->get_state_size ();
    else
      state_size = pe->set_state ( state_array );
   
    state_array += state_size;
    pe = body_list.get_next ();
  }
}

// array to state
void ctWorld::reintegrate_state ( const real *state_array )
{
  ctEntity *pe;
  long state_size;

  pe = body_list.get_first ();
  while ( pe )
  {
    if ( fsm_state == CTWS_REWOUND && (pe->flags & CTF_NOREWIND) )
      state_size = pe->get_state_size ();
    else
    {
      state_size = pe->get_state ( state_array );
      state_array += state_size;
    }
    pe = body_list.get_next ();
  }
}


void ctWorld::load_delta_state ( real *state_array )
{
  ctEntity *pe;
  long state_size;

  pe = body_list.get_first ();
  while ( pe )
  {
    if ( fsm_state == CTWS_REWOUND && (pe->flags & CTF_NOREWIND) )
      state_size = pe->get_state_size ();
    else
    {
      state_size = pe->set_delta_state ( state_array );
      state_array += state_size;
    }
    pe = body_list.get_next ();
  }
}


// add an entity to this world
errorcode ctWorld::add_entity ( ctEntity *pe )
{
  if (pe->get_state_offset () < 0)
    pe->set_state_offset (state_alloc(pe->get_state_size()));

  if ( pe )
  {
    body_list.add_link( pe );
    return WORLD_NOERR;
  }
  else
    return WORLD_ERR_NULLPARAMETER;
}


errorcode ctWorld::add_enviro_force ( ctForce *f )
{
  if ( f )
  {
    enviro_force_list.add_link ( f );
    return WORLD_NOERR;
  }
  else
    return WORLD_ERR_NULLPARAMETER;
}

errorcode ctWorld::delete_entity ( ctEntity *pb )
{
  if (pb->get_state_offset() >= 0)
    state_free(pb->get_state_offset ());

  if ( pb )
  {
    body_list.delete_link ( pb );
    return WORLD_NOERR;
  }
  else
    return WORLD_ERR_NULLPARAMETER;
}


// apply the given function to all physical entities in the system.
void ctWorld::apply_function_to_body_list ( void(*fcn)( ctEntity *ppe ) )
{
  ctEntity *pe;

  pe = body_list.get_first ();
  while ( pe )
  {
    fcn ( pe );
    pe = body_list.get_next ();
  } 
}


// return the relative velocity between up to two bodies at a point in world space
ctVector3 ctWorld::get_relative_v 
  ( ctPhysicalEntity *body_a, ctPhysicalEntity *body_b, const ctVector3 &the_p )
{
  if ( (!body_a) && (!body_b) )
    return ctVector3 (0,0,0);	

  ctVector3 v_rel;
  ctVector3 body_x = body_a->get_pos ();
  ctVector3 ra = the_p - body_x;

  ctVector3 ra_v = body_a->get_angular_v ()%ra + body_a->get_v ();

  if ( body_b == NULL )
    v_rel = ra_v;
  else
  {
    ctVector3 rb = the_p - body_b->get_pos ();
    ctVector3 rb_v = body_b->get_angular_v () % rb + body_b->get_v ();
    v_rel = (ra_v - rb_v);
  }

  return v_rel;

}

// PONG collision model
// basic collision model for for objects with no mass.
/*void ctPhysicalEntity::resolve_collision( ctCollidingContact *cont )
{
ctVector3 j;

  if( cont == NULL )
    return;

  j = ( cont->n*((get_v())*cont->n) )*( -1.0 - cont->restitution );
  apply_impulse( cont->contact_p, j );

  if( cont->body_b != NULL ){
    j = ( cont->n*((cont->body_b->get_v())*cont->n) )
      *( -1.0 - cont->restitution );
    cont->body_b->apply_impulse( cont->contact_p, j );
  }
}
*/

// collision response
void ctWorld::resolve_collision ( ctCollidingContact *cont )
{
  ctVector3 j;
  real v_rel;       // relative velocity of collision points
  //ctVector3 ra_v, rb_v;
  real j_magnitude;
  real bottom;
  ctMatrix3 imp_I_inv;
  real ma_inv, mb_inv;   // 1/mass_body
  real rota, rotb;       // contribution from rotational inertia
  ctVector3 n;
  ctVector3 ra, rb;      // center of body to collision point in inertail ref frame 
  // keep track of previous object collided with.
  // in simultaneous collisions with the same object the restituion should
  // only be factored in once.  So all subsequent collisions are handled as
  // seperate collisions, but with a restitution of 1.0
  // if they are different objects then treat them as multiple collisions with
  // normal restitution.
  //!me this isn't actually a very good method.... maybe something better can be 
  //!me implemented once contact force solver is implemented
  ctPhysicalEntity *prev;
  ctPhysicalEntity *ba;
  ctPhysicalEntity *bb;
  ctCollidingContact *head_cont = cont;

  // since NULL is used for an immovable object we need a 
  // different "nothing" pointer
  prev = (ctPhysicalEntity *)this; 

  if ( (cont != NULL) && (cont->body_a != NULL) )
    ba = cont->body_a->get_collidable_entity();
  else
    return;
	
  if ( ba == NULL ) 
    return;

  while ( cont != NULL )
  {
    if ( cont->body_b != NULL )
      bb = cont->body_b->get_collidable_entity();
    else
      bb = NULL;

    n = cont->n;
    // get component of relative velocity along collision normal
    v_rel = n * get_relative_v ( ba, bb, cont->contact_p );

    // if the objects are traveling towards each other do collision response
    if (v_rel < 0) 
    {
      ra = cont->contact_p - ba->get_pos ();

      ba->get_impulse_m_and_I_inv ( &ma_inv, &imp_I_inv, ra, n );
      ma_inv = 1.0/ma_inv;
      rota = n * ((imp_I_inv*( ra%n ) )%ra);  

      if ( bb == NULL )
      {
        // hit some kind of immovable object
        mb_inv = 0;
        rotb = 0;
      }
      else
      {
        rb = cont->contact_p - bb->get_pos();
        bb->get_impulse_m_and_I_inv( &mb_inv, &imp_I_inv, rb, n*(-1.0) );
        mb_inv = 1.0/mb_inv;
        rotb = n * ((imp_I_inv*( rb%n ) )%rb);
      }

      // bottom part of equation
      bottom = ma_inv + mb_inv + rota + rotb;

      //!me hack to keep objects appart
      if ( v_rel > -0.1 )
        cont->restitution = 4.0;

      if ( prev != cont->body_b )
        j_magnitude = -(1.0 + cont->restitution ) * v_rel / bottom;
      else
        // if we are dealing with a simulatneous collision with 
        // same object.
        j_magnitude = -(1.0 + 1.0 ) * v_rel / bottom;

      j = n * j_magnitude;
      ba->apply_impulse ( ra, j );

      if ( bb != NULL )
        bb->apply_impulse( rb, j*(-1.0) );
  
      // treat next simultaneous collision as a seperate collision.
      prev = bb;

    }

    cont = cont->next;  
  }

  // now check if any of the contacts are in resting contact
  cont = head_cont;

/*  while( cont != NULL ){
    // get component of relative velocity along collision normal
    v_rel = cont->n*get_relative_v( cont->body_b, cont->contact_p );

    if( fabs(v_rel) < MIN_CONTACT ){
      ctContact *r_cont = new ctContact;

      r_cont->body_a = (ctRigidBody *)(cont->body_a);  //!me bad but works for now
      r_cont->body_b = (ctRigidBody *)(cont->body_b);
      r_cont->n = cont->n;
      r_cont->ea = cont->ea;
      r_cont->eb = cont->eb;
      r_cont->vf = cont->vf;
      r_cont->contact_p = cont->contact_p;
   
    }

    cont = cont->next;
  }
*/
}

// alloc stuff

// Remove block matching "offset" from the used_blocks list and return it
AllocNode *ctWorld::sa_make_unused(int offset) 
{
  AllocNode *node = (AllocNode*) used_blocks.GetFirstItem ();
  while (node) 
  {
    if (node->offset == offset) 
    {
      used_blocks.RemoveItem ();
      return node;
    }
    node = (AllocNode*) used_blocks.GetNextItem ();
    if (node == (AllocNode*)used_blocks.PeekFirstItem ()) node = NULL;
  }
  return NULL;  //  Couldn't find block
}

// Add block to the used_blocks list, inserted in its proper location
bool ctWorld::sa_make_used(AllocNode *block) 
{
  AllocNode *index = (AllocNode*) used_blocks.GetFirstItem ();
  if (!index || index->offset > block->offset) 
    return used_blocks.AddItem((void*)block);

  while (index) 
  {
    if (index->offset > block->offset) 
    {
      used_blocks.GetPrevItem ();
      return used_blocks.AddCurrentItem ( (void*)block );
    }
    index = (AllocNode*)used_blocks.GetNextItem ();
    if (index == (AllocNode*)used_blocks.PeekFirstItem ()) 
      index = NULL;
  }

  // Assert that we're back at the beginning -- csDLinkList is circular
  assert (!index);

  // Add elt to end of list
  used_blocks.GetPrevItem (); // Back to end of list
  return used_blocks.AddCurrentItem ((void*)block);
  return false;
}

int ctWorld::state_alloc (int size) 
{
  if ( size <= 0 ) 
    return -1;

  int maxsize = 0;
  int bestfit = 0;
  AllocNode *index = (AllocNode*) free_blocks.GetFirstItem ();
  while (index) 
  {
    if ( index->size > maxsize ) 
      maxsize = index->size;

    if ( (index->size >= size) && (index->size < bestfit) ) 
      bestfit = index->size;

    index = (AllocNode*) free_blocks.GetNextItem ();

    if ( index == (AllocNode*) free_blocks.PeekFirstItem () ) 
      index = NULL;
  }

  // Combination of best-fit and worst-fit algorithms that seem to me to
  // fill the bill nicely
  int blocksize;
  if ( bestfit == size ) 
    blocksize = bestfit;
  else 
    blocksize = maxsize;

  if ( blocksize < size ) 
  {
    // No already-allocated block fits
    AllocNode node;
    node.offset = state_size;
    node.size = size;
    state_size += size;
    assert (sa_make_used (&node));
    return node.offset;
  }

  // Grab first block of size blocksize and make it used, not free
  index = (AllocNode*)free_blocks.GetFirstItem ();
  while (index) 
  {
    if ( index->size == blocksize ) 
    {
      free_blocks.RemoveItem ();     // Block isn't free
      assert (sa_make_used (index));  // Block is now used
      return index->offset;
    }
    index = (AllocNode*) free_blocks.GetNextItem ();
    assert ( index != (AllocNode*)free_blocks.PeekFirstItem () );
  }
  // This should never happen -- our blocksize should be guaranteed valid
  assert (0 && "Invalid blocksize got through in state_alloc!  Dying!");
  return -1;
}

void ctWorld::state_free (int offset) 
{
  AllocNode *freenode;

  freenode = sa_make_unused (offset);

  if (!freenode)
    // Wasn't in used_blocks list, state_free() of invalid offset
    return;

  if (freenode->offset + freenode->size == state_size) 
  {
    // If this is the last block in the free & used lists, reduce state_size
    state_size -= freenode->size;
    delete freenode;
    return;
  }

  AllocNode *index = (AllocNode*) free_blocks.GetFirstItem ();
  bool handled = false;
  if ( !index || (index->offset > freenode->offset) ) 
  {
    // Tack it onto the beginning
    free_blocks.AddItem((void*)freenode);
    handled = true;
  }

  while ( !handled && index ) 
  {
    if ( index->offset > freenode->offset ) 
    {
      if ( freenode->offset + freenode->size >= index->offset ) 
      {
	assert (freenode->offset + freenode->size == index->offset);
	index->offset = freenode->offset;
	index->size += freenode->size;
	delete freenode;
	handled = true;
	break;
      } 
      else 
      {
	// Blocks are separate, just need to insert before current block
	// if() statement before while guarantees this isn't first list elt
	free_blocks.GetPrevItem ();
	free_blocks.AddCurrentItem ((void*)freenode);
      }
    }

    index = (AllocNode*) free_blocks.GetNextItem ();
    if ( index == (AllocNode*) free_blocks.PeekFirstItem () ) 
      index = NULL;
  }
  if (!handled) 
  {
    // Block wasn't inserted before the end of the list, so it presumably
    // goes at the end of the list, but before the end of the state vector.
    // It's all so complicated...  :)
    free_blocks.GetFirstItem ();  // Beginning of list
    free_blocks.GetPrevItem ();   // End of list -- it's circular
    free_blocks.AddCurrentItem ( (void*) freenode );
  }

  // Mark as freed
  state_offset = -1;
}
