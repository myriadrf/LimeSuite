/* ************************************************************************ 
   FILE:	lms.h
   COMMENT:	
   CONTENT:
   AUTHOR:	Lime Microsystems
   DATE:	Feb 24, 2000
   REVISION:
   ************************************************************************ */
#include <stdlib.h>
#include <math.h>

/* This two parameters define filter symmetry */
#define POSITIVE	(1)
#define NEGATIVE	(-1)

/* Number of points on the frequency grid */
//#define LMS_POINTS 1000
#define LMS_POINTS 4000

/* Amplitude correction functions */
double InvSinc();
double InvSincS();
double One();

/* Amplitude correction functions we can support */
#define NONE		One		/* Constant */
#define ONE		One		/* as well */
#define INVSINC		InvSinc		/* Iverse sinc */
#define INVSINCS	InvSincS	/* Shifted Inverse Sinc */

/* Cosine filter type selection */
#define RAISEDCOSINE		1
#define ROOTRAISEDCOSINE	2
