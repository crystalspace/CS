#include "cssysdef.h"
#include "csphyzik/rigidbod.h"
#include "csphyzik/ctvector.h"
#include "csphyzik/contact.h"
#include "csphyzik/ctcontct.h"

// solve a = Af + b for f given that af >= 0,   a >= 0,   f >= 0  hold
// a are relative accelerations at a contact point.
// f are contact forces.  Unknown.
// A is a matrix to get accel from f's
// b are accel at a point resulting from all forces other than contact forces
// implementation of algorithm found in Baraff's 1994 paper:
// "Fast Contact Force Computation for Nonpenetrating Rigid Bodies"
void ctFastContactSolver::compute_contact_forces ( ctContact *c_list, int c_num )
{
  int i = 0;
  // vector of contact forces
  real *c_f = new real[c_num];
  // vector of relative accelerations at contact points
  real *c_a = new real[c_num];
  // delta vector for contact force vector
  real *c_df = new real[c_num];
  // delta vector for acceleration vector
  real *c_da = new real[c_num];
  // A matrix.
  ctMatrixN A(c_num);
  // the indexes of contacts with accleration = 0.  C in Baraff '94
  int *Czero_a = new int[c_num];
  // number of elements in C.  starts at 0
  int C_top = 0;
  // the indexes of contacts with force = 0.  NC in Baraff '94
  int *NCzero_f = new int[c_num];
  // number of elements in NC.  starts at 0
  int NC_top = 0;
  ctRigidBody *bod;

  compute_contact_force_matrix( c_list, c_num, A );

  // zero out contact forces
  while ( i < c_num )
  {
    c_f[ i ] = 0.0;
    c_df[ i ] = 0.0;
    c_da[ i ] = 0.0;
    i++;
  }

  // b vector.  external and interal forces at contact points
  compute_contact_force_b ( c_list, c_num, c_a );

  int row = 0;
  while ( row < c_num )
  {
    // if points are seperating ignore
    if ( c_a[row] > 0.0 )
    {
      // nuthin
      /*!me optimization	// if points are in resting contact add to clamped list
	}else	if( c_a[row] == 0.0 ){
	Czero_a[C_top++] = row;
	// points are accelerating such that they will interpenetrate
      */
    }
    else
    { // implied if( c_a[row] < 0.0
      rebalance_forces ( row, c_num, A, c_a, c_f, c_da, c_df, Czero_a, &C_top, NCzero_f, &NC_top );
    }
    row++;
  }

  // add contact forces to bodies
  for ( i = 0; i < c_num; i++ )
  {
    bod = c_list[i].body_a;
    if ( bod != NULL )
    {
      // don't add force if it is most likely round off error.
      if ( fabs(c_f[i]) > MIN_REAL )
      {
        bod->sum_force ( c_list[i].n*c_f[i] );
        bod->sum_torque ((c_list[i].contact_p - bod->get_pos ())
			 % ( c_list[i].n*c_f[i] ));
      }
    }
    bod = c_list[i].body_b;
    if ( bod != NULL )
    {
      // don't add force if it is most likely round off error.
      if ( fabs(c_f[i]) > MIN_REAL )
      {
        bod->sum_force( c_list[i].n*-c_f[i] );
        bod->sum_torque((c_list[i].contact_p - bod->get_pos())%( c_list[i].n*-c_f[i] ));
      }
    }
  }

  delete [] c_f;
  delete [] c_a;
  delete [] c_df;
  delete [] c_da;
  delete [] Czero_a;
  delete [] NCzero_f;
}

void ctFastContactSolver::rebalance_forces ( int row, int c_num, ctMatrixN &A, real *c_a, real *c_f, real *c_da, real *c_df, int *Czero_a, int *C_top, int *NCzero_f, int *NC_top )
{
  bool a_is_zero = false;
  real scale;
  int max_step_row;
  int i;

  while ( !a_is_zero )
  {
    // rebalances direction of all contact forces
    fdirection( c_df, row, c_num, A, Czero_a, *C_top, NCzero_f, *NC_top );

    A.mult_v( c_da, c_df );
    // determine max step we can adjust forces by without throwing
    // an already valid value off.
    scale = find_max_step( &max_step_row, row, c_a, c_f, c_da, c_df,
                           Czero_a, *C_top, NCzero_f, *NC_top);

    // max_step_row = -1 if under some circumstances..
    //!me try to figure out when this happens and what to do...
    if ( max_step_row >= 0 )
    {
      // adjust forces/accels
      // f = f + s*df
      // a = a + s*a
      for ( i = 0; i < c_num; i++ )
      {
        // try to minimize round-off error
        if ( fabs(c_df[i]*scale) < MIN_REAL )
          c_df[i] = 0.0;
        if ( fabs(c_da[i]*scale) < MIN_REAL )
          c_da[i] = 0.0;

	c_f[i] += scale*c_df[i];
	c_a[i] += scale*c_da[i];
      }

      bool step_row_in_C = false;
      int j;
      int swap_idx = 0;
      for ( j = 0; j < *C_top && !step_row_in_C; j++ )
      {
	if ( Czero_a[j] == max_step_row )
	{
	  step_row_in_C = true;
          swap_idx = j;
	}
      }

      // just decreased force of this contact to zero so move it into
      // zero force set.  also accel no longer neccesarily 0
      if ( step_row_in_C )
      {
        *C_top = (*C_top) - 1;
        Czero_a[swap_idx] = Czero_a[*C_top];
        NCzero_f[*NC_top] = max_step_row;
        *NC_top = (*NC_top) + 1;
      }
      else
      {
        bool step_row_in_NC = false;
	for ( j = 0; j < *NC_top && !step_row_in_NC; j++ )
	{
	  if ( NCzero_f[j] == max_step_row )
	  {
	    step_row_in_NC = true;
            swap_idx = j;
	  }
	}

        // increased force from zero to some positive value so move out
        // of zero force set
        if ( step_row_in_NC )
	{
          *NC_top = (*NC_top) - 1;
          NCzero_f[swap_idx] = NCzero_f[*NC_top];
          Czero_a[*C_top] = max_step_row;
          *C_top = (*C_top) + 1;

	  // ok this a has been driven to zero so flag exit condition and
	  // add to C set
        }
	else
	{
          Czero_a[*C_top] = max_step_row;
          *C_top = (*C_top) + 1;
          a_is_zero = true;
        }
      }
    }
    else
    { // because a must be +ve....  ignore... should never really happen..
        a_is_zero = true;
    }
  }
}

// rebalances direction of all contact forces.  determines delta vector for forces
// modifies c_df.
void ctFastContactSolver::fdirection( real *c_df, int row, int c_num, ctMatrixN &A, int *Czero_a, int C_top, int *NCzero_f, int NC_top )
{
  (void) NCzero_f;
  (void) NC_top;

  int i,j,k,z;
  int Acc_dimen = C_top;

  // df = 0
  for ( i = 0; i < c_num; c_df[i++] = 0.0 );
  c_df[row] = 1.0;

  if ( Acc_dimen == 0 )
    return;

  ctMatrixN Acc ( Acc_dimen );
  real *Acd = new real [ Acc_dimen ];

  bool *add_Acc_rows = new bool[c_num];
  for ( i = 0; i < c_num; add_Acc_rows[i++] = false );

  for ( i = 0; i < C_top; i++ )
    add_Acc_rows[Czero_a[i]] = true;


  // get submatrix Acc of A with all rows and column in Czero_a set
  // also get vector Acd at row d of A with rows of Czero_a
  j = 0;
  for ( i = 0; i < c_num; i++ )
  {
    if ( add_Acc_rows[i] )
    {
      z = 0;
      for ( k = 0; k < c_num; k++ )
      {
        if ( add_Acc_rows[k] )
	{
          Acc[j][z] = A[i][k];
          z++;
        }
      }
      Acd[j] = - A[i][row];
      j++;
    }
  }

  real *x = new real[Acc_dimen];
  Acc.solve ( x, Acd );

  j = 0;
  for ( i = 0; i < c_num; i++ )
  {
    if ( add_Acc_rows[i] )
    {
      c_df[i] = x[j];
      j++;
    }
  }

  delete [] x;
  delete [] add_Acc_rows;
  delete [] Acd;
}

real ctFastContactSolver::find_max_step ( int *max_step_row, int row,
				   real *c_a, real *c_f, real *c_da, real *c_df,
			       int *Czero_a, int C_top, int *NCzero_f, int NC_top )
{
  real scale = MAX_REAL;
  real temp_s;
  int max_s_row = -1;
  int i;

  if ( c_da[row] > 0.0 )
  {
    max_s_row = row;
    scale = -c_a[row]/c_da[row];
  }

  for ( i = 0; i < C_top; i++ )
  {
    int act_i = Czero_a[i];
    if ( c_df[act_i] < 0.0 )
    {
      temp_s = -c_f[act_i]/c_df[act_i];
      if ( temp_s < scale )
      {
        scale = temp_s;
        max_s_row = act_i;
      }
    }
  }

  for ( i = 0; i < NC_top; i++ )
  {
    int act_i = NCzero_f[i];
    if ( c_da[act_i] < 0.0 )
    {
      temp_s = -c_a[act_i]/c_da[act_i];
      if ( temp_s < scale )
      {
        scale = temp_s;
        max_s_row = act_i;
      }
    }
  }

  *max_step_row = max_s_row;
  return scale;
}



/********  ctContactSolver shared stuff  **************/

ctVector3 ctContactSolver::pt_velocity ( ctPhysicalEntity *body, ctVector3 p )
{
  if ( body != NULL )
    return body->get_v () + ( body->get_angular_v () % ( p - body->get_pos () ));
  else
    return ctVector3( 0,0,0 );
}

// compute the derivitive of the collision normal
ctVector3 ctContactSolver::compute_ndot ( ctContact *c )
{
  if ( c->vf )
  {
    ctVector3 w_norm;
    // body b is NULL if it an immovable object.
    // in such a case dn = 0
    if ( c->body_b != NULL )
      w_norm = (c->body_b->get_angular_v()) % c->n;
    else
      w_norm = ctVector3( 0.0, 0.0, 0.0 );
    return w_norm;
  }
  else /* vertex-edge */
  {
    ctRigidBody *bod_a = c->body_a;
    ctRigidBody *bod_b = c->body_b;
    ctVector3 ea = c->ea;
    ctVector3 eb = c->eb;

    ctVector3 eadot = bod_a->get_angular_v () % ea;
    ctVector3 ebdot;
    if ( bod_b != NULL )
      ebdot = bod_b->get_angular_v() % eb;
    else
      ebdot = ctVector3( 0,0,0 );

    // time derivative of contact normal
    //!me using % where it said * in witkin/baraff/kass
    ctVector3 n1 = ea % eb;
    ctVector3 z = ( eadot % eb ) + ( ea % ebdot );
    real len = n1.Norm();

    n1 *= 1.0/ len;

    return ( z - ( ( z % n1 ) % n1 ) )/ len;
  }
}

// b is a vector of scalars that are the magnitudes of the normal
// accelerations at a contact point between two bodies.  These accelerations
// do not include contact forces.
void ctContactSolver::compute_contact_force_b ( ctContact contacts[], int ncontacts, real b[] )
{
  ctVector3 a_ext_part, a_vel_part,
            b_ext_part, b_vel_part;

  int i;
  for (i = 0; i < ncontacts; i++)
  {
    ctContact *c = &contacts[i];
    ctRigidBody *A = c->body_a;
    ctRigidBody *B = c->body_b;

    ctVector3 n = c->n;
    ctVector3 ra = c->contact_p - A->get_pos ();
    ctVector3 f_ext_a = A->get_F ();
    ctVector3 t_ext_a = A->get_torque ();
    ctMatrix3 I_inv_world = A->get_I_inv_world ();

    a_ext_part = f_ext_a / A->get_m() +
      ((I_inv_world * t_ext_a) % ra );

    a_vel_part = (A->get_angular_v() % (A->get_angular_v() % ra )) +
      ((I_inv_world * (A->get_angular_P() % A->get_angular_v())) % ra );

    // B is NULL if it is an immovable object.
    if ( B != NULL )
    {
      ctVector3 rb = c->contact_p - B->get_pos ();
      ctVector3 f_ext_b = B->get_F ();
      ctVector3 t_ext_b = B->get_torque ();
      I_inv_world = B->get_I_inv_world ();

      b_ext_part = f_ext_b / B->get_m () +
        ((I_inv_world * t_ext_b) % rb );

      b_vel_part = (B->get_angular_v () % (B->get_angular_v () % rb )) +
        ((I_inv_world * (B->get_angular_P () % B->get_angular_v ())) % rb );
    }
    else
    {
      b_ext_part = ctVector3(0,0,0);
      b_vel_part = ctVector3(0,0,0);
    }

    double k1 = n * ((a_ext_part + a_vel_part) - ( b_ext_part + b_vel_part ));

    ctVector3 ndot = compute_ndot ( c );

    double k2 = ( ndot * 2.0 ) * ( pt_velocity( A, c->contact_p ) -
				   pt_velocity( B, c->contact_p ));

    b[i] = k1 + k2;
  }
}


void ctContactSolver::compute_contact_force_matrix
   ( ctContact contacts[], int ncontacts, ctMatrixN &a )
{
  int i,j;
  for ( i = 0; i < ncontacts; i++ )
    for( j = 0; j < ncontacts; j++ )
      a[i][j] = compute_aij( contacts[i], contacts[j] );

  //!me should check here that a is PSD matrix to make sure my
  //!me changes were ok
  for ( i = 0; i < ncontacts; i++ )
    for ( j = 0; j < ncontacts; j++ )
    {
      CS_ASSERT ( a[i][j] == a[j][i] );
    }
}

real ctContactSolver::compute_aij ( ctContact &ci, ctContact &cj )
{
  ctVector3 a_linear;
  ctVector3 a_angular;
  ctVector3 b_linear;
  ctVector3 b_angular;

  if (( ci.body_a != cj.body_a ) && ( ci.body_b != cj.body_b ) &&
      ( ci.body_a != cj.body_b ) && ( ci.body_b != cj.body_a ))
    return 0.0;

  ctRigidBody *A = ci.body_a;
  ctRigidBody *B = ci.body_b;
  ctVector3 ni = ci.n;
  ctVector3 nj = cj.n;
  ctVector3 pi = ci.contact_p;
  ctVector3 pj = cj.contact_p;
  ctVector3 ra = pi - A->get_pos ();

  ctVector3 force_on_a( 0.0, 0.0, 0.0 );
  ctVector3 torque_on_a( 0.0, 0.0, 0.0 );

  if ( cj.body_a == ci.body_a )
  {
    force_on_a = nj;
    torque_on_a = ( pj - A->get_pos() ) % nj;
  }
  else if ( cj.body_b == ci.body_a )
  {
    force_on_a = nj*(-1.0);
    //!me if nj is -ve why isn't torque?
    //!me I changed the sign of the torque as well.
    //!me this means there is a typo in Baraff's tutorial... I hope
    //!me Dr. Baraff said to me... "yeah that seems right" ..
    torque_on_a = ( pj - A->get_pos() ) % nj*(-1.0);
  }

  a_linear = force_on_a / A->get_m ();
  a_angular = ( A->get_I_inv_world () * torque_on_a ) % ra;

  // B is NULL if it is an immovable object
  if ( B != NULL )
  {
    ctVector3 rb = pi - B->get_pos();

    ctVector3 force_on_b( 0.0, 0.0, 0.0 );
    ctVector3 torque_on_b( 0.0, 0.0, 0.0 );

    if ( cj.body_a == ci.body_b )
    {
      force_on_b = nj;
      torque_on_b = ( pj - B->get_pos() ) % nj;
    }
    else if ( cj.body_b == ci.body_b )
    {
      force_on_b = nj*(-1.0);
      //!me if nj is -ve why isn't torque?
      //!me I changed the sign of the torque as well.
      //!me this means there is a typo in Baraff's tutorial... I hope
      //!me Dr. Baraff said to me... "yeah that seems right" ..
      torque_on_b = ( pj - B->get_pos() ) % nj *(-1.0);
    }

    b_linear = force_on_b / B->get_m ();
    b_angular = ( B->get_I_inv_world () * torque_on_b ) % rb;
  }
  else
  {
    b_linear = ctVector3( 0,0,0 );
    b_angular = ctVector3( 0,0,0 );
  }

  real r = ni *(( a_linear + a_angular ) - ( b_linear + b_angular ));
  return r;

}


