#ifndef CT_CONTACT_SOLVER_H
#define CT_CONTACT_SOLVER_H

#include "csphyzik/phyztype.h"

class ctContact;
class ctMatrixN;

class ctContactSolver
{
public:
  virtual void compute_contact_forces ( ctContact *c_list, int c_num ) = 0;

protected:
  ctVector3 pt_velocity ( ctPhysicalEntity *body, ctVector3 p );
  ctVector3 compute_ndot ( ctContact *c );
  real compute_aij ( ctContact &ci, ctContact &cj );

  void compute_contact_force_matrix
    ( ctContact contacts[], int ncontacts, ctMatrixN &a );

  void compute_contact_force_b
    ( ctContact contacts[], int ncontacts, real b[] );
};

class ctFastContactSolver : public ctContactSolver
{
public:
  virtual void compute_contact_forces ( ctContact *c_list, int c_num );

protected:

  void rebalance_forces ( int row, int c_num, ctMatrixN
			  &A, real *c_a, real *c_f, real *c_da, real *c_df,
			  int *Czero_a, int *C_top, int *NCzero_f, int *NC_top );

  void fdirection ( real *c_df, int row, int c_num, ctMatrixN &A,
		    int *Czero_a, int C_top, int *NCzero_f, int NC_top );

  real find_max_step ( int *max_step_row, int row, real *c_a, real *c_f,
		       real *c_da, real *c_df, int *Czero_a, int C_top,
		       int *NCzero_f, int NC_top );

};

#endif // CT_CONTACT_SOLVER_H


