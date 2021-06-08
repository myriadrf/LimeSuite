/* --------------------------------------------------------------------------------------------
FILE:		nrc.h
DESCRIPTION:	Namespace which collects all Numerical Receipes in C functions.
CONTENT:
AUTHOR:		Lime Microsystems LTD
		LONGDENE HOUSE
		HEDGEHOG LANE
		HASLEMERE GU27 2PH
DATE:		Apr 28, 2005
REVISIONS:
   -------------------------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define TINY 1.0e-20

namespace nrc {
	double  *vector(int, int);
	void free_vector(double *, int , int );
	
	double **matrix(int, int, int, int);
	void free_matrix(double **, int , int , int , int );

	int  *ivector(int, int);
	void free_ivector(int *, int , int );

	void nrerror(char *);

	// LU decomposition and backsubstitution
	int ludcmp(double **, int, int *, double *);
	void lubksb(double **, int, int *, double *);
	
	// Gradient descent used to solve linear equations
	void lgrad(double **, double *, double *, int, double);
	
	// Gaus-Seidel to solve linear equations
	void gauss_seidel(double **, double *, double *, int);
}
