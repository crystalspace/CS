#include "sysdef.h"
#include "csengine/cdobj.h"
#include "csphyzik/phyziks.h"
#include "cstso.h"

static ctCollidingContact contact_heap[1024];  // no more than that
static int contact_heap_index = 0;

csRigidSpaceTimeObj *csRigidSpaceTimeObj::space_time_continuum[ MAX_SPACE_TIME_NUM ];
long csRigidSpaceTimeObj::continuum_end = 0;

// hideously inefficient collision detection/response algorithm
// just wanted to see some stuff bouncing around for now. 

csSpaceTimeObj::csSpaceTimeObj()
{
//  space_time_continuum[continuum_end++] = this;
  //what_type = ST_SPACETIME; 
}

csRigidSpaceTimeObj::csRigidSpaceTimeObj( csSprite3D *psprt, ctRigidBody *prb )
{
  space_time_continuum[continuum_end++] = this;
 // what_type = ST_SPACETIME; 
 // col = pcollide;
  sprt = psprt;
  rb = prb;
  CHK (col = new csCollider (sprt));
  csColliderPointerObject::SetCollider (*sprt, col, true);
  what_type = ST_RIGID;

}

void set_no_rewind( ctPhysicalEntity *ppe )
{
  ppe->flags |= CTF_NOREWIND;
}

// using sloooooooow but accurate collision detection.
// this can be sped up in many ways.
// - use eulers method or mid-point for ODE just during CD
// - use prune and sweep algorithm with axis aligned, sorted bounding boxes
// - only evolve objects related to colliding objects when searching for c-time
// - don't forget to handle 'tunneling' problem as well.
void csRigidSpaceTimeObj::evolve_system( real t1, real t2, ctWorld *time_world, csWorld *space_world )
{
static real collision_epsilon = 0.02;
static real time_epsilon = 0.001;
real ta, tb;
 
long loops = 300;  //make sure we don't go into an infinite loop

  ta = t1;
  tb = t2;
  real min_col_dist;

  time_world->apply_fuction_to_body_list( set_no_rewind );

  while( ta < t2 && loops-- > 0){
 
    time_world->evolve( ta, tb );

    min_col_dist = collision_check( space_world );
    // if there are objects intersecting
    if( min_col_dist > 0 ){
      // if interpenetrations distance is very small => time of impact found
      // or if the time step is really small
      if( min_col_dist <= collision_epsilon || ( tb - ta ) <= time_epsilon ){
        // respond to collision
        collision_response( space_world );
        ta = tb;
        tb = t2;
      }else{
        // bisect time backwards to search for time of impact
        time_world->rewind(ta, tb);   // rewind state back to ta
        tb -= (tb - ta)*0.5;
  //      ctVector3 px = space_time_continuum[0]->rb->get_pos();
    //    px = space_time_continuum[0]->rb->get_pos();
      }
    // if we have not arrived at the end of our time interval 
    }else if( tb < t2 ){
      // search forward in time for time of impact for collision(s)
      ta = tb;
      // if we are at our min time step, just finish this time step, collision
      // will get resolved next time.
      if( (t2 - ta) <= time_epsilon ){
        tb = t2;
        time_world->evolve( ta, tb );

        ta = tb;
      }else{
        tb += (t2 - tb)*0.5;
      }
    }else{
      ta = tb;
      tb = t2;
    }
  }

  update_space();

}

void csRigidSpaceTimeObj::update_space()
{
ctVector3 px;
csMatrix3 m; 
ctMatrix3 M;
csRigidSpaceTimeObj *sto;

  for( int i = 0; i < continuum_end; i++ ){
    sto = space_time_continuum[i];
    px = sto->rb->get_pos();

    csVector3 new_p( px[0], px[1], px[2] );   
    sto->sprt->SetMove ( new_p );

    M = sto->rb->get_R();   // get orientation for this link
    // ctMatrix3 and csMatrix3 not directly compatable yet
    m.Set( M[0][0], M[0][1], M[0][2],
         M[1][0], M[1][1], M[1][2],
         M[2][0], M[2][1], M[2][2]);    // set orientation of sprite
    sto->sprt->SetTransform(m);
  }
}


real csRigidSpaceTimeObj::collision_check( csWorld *space )
{
csCollider *coli;
//csSprite3D *sprt;
csSector* first_sector;
ctMatrix3 M;
csMatrix3 m;
csVector3 n;
csVector3 x;
ctVector3 X;
csVector3 trime;
CDTriangle *wall;
CDTriangle *htri;
real max_depth;
real current_depth;
csOrthoTransform tfm;
ctCollidingContact *this_contact;
//ctCollidingContact *prev_contact;
collision_pair *CD_contact = NULL;
//bool hit_found;

  max_depth = 0;

  for( int i = 0; i < continuum_end; i++ ){

    first_sector = (csSector*)(space_time_continuum[i]->sprt->sectors[0]);
    
    // Start collision detection.
    csCollider::CollideReset ();
    csCollider::firstHit = false;
    coli = space_time_continuum[i]->col;
   // for ( ; num_sectors-- ; )
    M = space_time_continuum[i]->rb->get_world_to_this();
    m.Set( M[0][0], M[0][1], M[0][2],
               M[1][0], M[1][1], M[1][2],
               M[2][0], M[2][1], M[2][2]);    // orientation of sprite
//    hits += CollisionDetect (col, first_sector, &m);
    X = space_time_continuum[i]->rb->get_pos();
    x.x = X[0]; x.y = X[1]; x.z = X[2];
    // this IS a transformaition from other to this space.
    tfm.SetO2T( m );
    // NOT a traslation from other space to this space.  Actually opposite.
    tfm.SetO2TTranslation( x );

    // Check collision with this sector.
    csCollider::numHits = 0;
    if( first_sector ){
      csCollider::CollidePair (coli, csColliderPointerObject::GetCollider(*first_sector), &tfm);
      CD_contact = csCollider::GetCollisions ();
    }

    space_time_continuum[i]->num_collisions = csCollider::numHits;
    space_time_continuum[i]->contact = NULL;
    contact_heap_index = 0;
    // determine type of collision and penetration depth
    if( space_time_continuum[i]->num_collisions == 0 ){
      // no collision
    }else{
      //IMPORTANT: turn NOREWIND off if there is a collision!
      space_time_continuum[i]->rb->flags &= (~CTF_NOREWIND);
      this_contact = &(contact_heap[contact_heap_index]);
      this_contact->next = NULL;
      
      for( int acol = 0; acol < csCollider::numHits; acol++ ){
        space_time_continuum[i]->cd_contact[acol] = CD_contact[acol];

        // here is where the body hit should be recorded as well
        // NULL means we hit an immovable object, like a wall
        this_contact->body_b = NULL;

        this_contact->restitution = 0.75;

        wall = CD_contact[acol].tr2;
        n = ((wall->p2-wall->p1)%(wall->p3-wall->p2)).Unit();
      //  CsPrintf( MSG_DEBUG_1, "n %f, %f, %f\n", n.x, n.y, n.z );
        this_contact->n =
          ctVector3(n.x, n.y, n.z );

      // just one collision at a time here.
//      this_contact->next = NULL;

 /*   // ignore ojbects that aren't really moving towards collision.  
    // they may "seep" though floor.
    if( fabs(space_time_continuum[i]->rb->get_v() * this_contact->n)  < MIN_REAL*1000.0)
      return 0;
*/

        htri = CD_contact[acol].tr1;

        // check each point of this triangle to see which penetrated the most
  
        for( int j = 0; j < 3 ; j++ ){
          if( j == 0 )
            trime = tfm.This2Other( htri->p1 );
          else if ( j == 1 )
            trime = tfm.This2Other( htri->p2 );
          else
            trime = tfm.This2Other( htri->p3 );

          current_depth = -(trime - wall->p1)*n;
          // this is the collision point
          if( current_depth > max_depth ){
            max_depth = current_depth;
          }
         
          if( current_depth > 0.0 ){
            this_contact->contact_p = 
              ctVector3( trime.x, trime.y, trime.z );
        
            ctCollidingContact *chk = space_time_continuum[i]->contact;
            bool duplicate = false;
            while( chk != NULL && !duplicate ){
              if( chk->contact_p[0] == this_contact->contact_p[0] &&
                chk->contact_p[1] == this_contact->contact_p[1] &&
                chk->contact_p[2] == this_contact->contact_p[2] ){
                duplicate = true;
              }
              chk = chk->next;
            }
            if( !duplicate ){
              this_contact->next = space_time_continuum[i]->contact;
              space_time_continuum[i]->contact = this_contact;
              this_contact = &(contact_heap[++contact_heap_index]);
              this_contact->body_b = NULL;
              this_contact->restitution = space_time_continuum[i]->contact->restitution;
              this_contact->n = space_time_continuum[i]->contact->n;

            }
            // add collision point to other object as well, here
          }
        } 
      }

    }

  }
  return sqrt(max_depth);

}


void csRigidSpaceTimeObj::collision_response( csWorld *space )
{
csRigidSpaceTimeObj *sto;

  for( int i = 0; i < continuum_end; i++ ){
    sto = space_time_continuum[i];
    if( sto->num_collisions > 0 && sto->contact != NULL ){
      sto->rb->resolve_collision( sto->contact );
    }
  }
}