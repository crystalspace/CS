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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "csphyzik/phyztype.h"
#include "csphyzik/mtrxutil.h"

static int *order;
static int order_size = 0;

// solve the matrix equation A*x = b using Gausse-Siedel elimination with back substitution
// and partial pivoting.
void linear_solve( real **A, int dim, double *x, double *b )
{
real max_pivot, factor;
int elimcol, elimrow, check_pivot, backcol, backrow, target_row, target_col;
int max_order_row_index;

	if( dim > order_size ){
		free( order );
		order = (int *)malloc( dim * sizeof(real) );
		order_size = dim;
	}

	for( target_row = 0; target_row < dim; target_row++ ){
		order[target_row] = target_row;
	}

	// eliminate all columns
	for( elimcol = 0; elimcol < dim; elimcol++ ){
		
		max_order_row_index = elimcol;

		// get row with largest value at pivot from set of rows not eliminated already
		for( max_pivot = 0, check_pivot = elimcol; check_pivot < dim; check_pivot++ ){
			if( max_pivot < fabs(A[order[check_pivot]][elimcol]) ){
				max_pivot = fabs(A[order[check_pivot]][elimcol]);	
				max_order_row_index = check_pivot;
			}
		}

		// swap rows in order
		elimrow = order[max_order_row_index];
		order[max_order_row_index] = order[elimcol];
		order[elimcol] = elimrow;
		
		// elimination:  only eliminate all rows that haven't been reduced already
		for( target_row = elimcol+1; target_row < dim; target_row++ ){
			if( A[elimrow][elimcol] == 0.0 ){ //!me should report errors
				factor = 0.0;  // the whole row was 0.0, so fail nicely
				printf( "shit singular matrix\n");
				exit(1);
			}else{
				if( fabs(A[elimrow][elimcol]) < MIN_REAL ){  
				exit(1);
					factor = MAX_REAL;
				}else{
					factor = A[order[target_row]][elimcol]/A[elimrow][elimcol];
				}
			}

			// do each column for this elimanation step that hasn't been eliminated already
			for( target_col = elimcol; target_col < dim; target_col++ ){
				A[order[target_row]][target_col] -= factor*A[elimrow][target_col];
			}

			// modify b as well for the elimination
			b[order[target_row]] -= factor*b[elimrow];
		}
	
	}

	// find solutions with back substitution
	for( backcol = dim-1; backcol >= 0; backcol-- ){
		
		x[backcol] = b[order[backcol]];
		for( backrow = dim-1; backrow > backcol; backrow-- ){
			x[backcol] -= A[order[backcol]][backrow]*x[backrow];
		}

		if( A[order[backcol]][backcol] == 0 ){
				exit(1);
			x[backcol] = 0;  //!me fail nicely
		}else{
			x[backcol] = x[backcol]/A[order[backcol]][backcol];
		}

	}

}


/*
#define NRANSI
#include "csphyzik/nrutil.h"
#define TINY 1.0e-20;
   
void ludcmp(float **a, int n, int *indx, float *d)
{
	int i,imax,j,k;
	float big,dum,sum,temp;
	float *vv;
   
	vv=vector(1,n);
	*d=1.0;
	for (i=1;i<=n;i++) {
		big=0.0;
		for (j=1;j<=n;j++)
			if ((temp=fabs(a[i-1][j-1])) > big) big=temp;
		if (big == 0.0) nrerror("Singular matrix in routine ludcmp");
		vv[i]=1.0/big;
	}
	for (j=1;j<=n;j++) {
		for (i=1;i<j;i++) {
			sum=a[i-1][j-1];
			for (k=1;k<i;k++) sum -= a[i-1][k-1]*a[k-1][j-1];
			a[i-1][j-1]=sum;
		}
		big=0.0;
		for (i=j;i<=n;i++) {
			sum=a[i-1][j-1];
			for (k=1;k<j;k++)
				sum -= a[i-1][k-1]*a[k-1][j-1];
				a[i-1][j-1]=sum;
			if ( (dum=vv[i]*fabs(sum)) >= big) {
				big=dum;
				imax=i;
			}
		}
		if (j != imax) {
			for (k=1;k<=n;k++) {
				dum=a[imax-1][k-1];
				a[imax-1][k-1]=a[j-1][k-1];
				a[j-1][k-1]=dum;
			}
			*d = -(*d);
			vv[imax]=vv[j];
		}
		indx[j-1]=imax;
		if (a[j-1][j-1] == 0.0) a[j-1][j-1]=TINY;
		if (j != n) {
			dum=1.0/(a[j-1][j-1]);
			for (i=j+1;i<=n;i++) a[i-1][j-1] *= dum;
		}
	}
	free_vector(vv,1,n);
}
#undef TINY
#undef NRANSI

void lubksb(float **a, int n, int *indx, float b[])
{
	int i,ii=0,ip,j;
	float sum;
   
	for (i=1;i<=n;i++) {
		ip=indx[i-1];
		sum=b[ip-1];
		b[ip-1]=b[i-1];
		if (ii)
			for (j=ii;j<=i-1;j++) sum -= a[i-1][j-1]*b[j-1];
		else if (sum) ii=i;
		b[i-1]=sum;
	}
	for (i=n;i>=1;i--) {
		sum=b[i-1];
		for (j=i+1;j<=n;j++) sum -= a[i-1][j-1]*b[j-1];
		b[i-1]=sum/a[i-1][i-1];
	}
}
*/
