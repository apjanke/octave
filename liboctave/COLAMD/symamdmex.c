/* ========================================================================== */
/* === symamd mexFunction =================================================== */
/* ========================================================================== */

/* COLAMD Version 2.4.

    Usage:

	P = symamd (A) ;
	[ P, stats ] = symamd (A, knobs) ;

    See symamd.m for a description.

    Authors:

	The authors of the code itself are Stefan I. Larimore and Timothy A.
	Davis (davis at cise.ufl.edu), University of Florida.  The algorithm was
	developed in collaboration with John Gilbert, Xerox PARC, and Esmond
	Ng, Oak Ridge National Laboratory.

    Acknowledgements:

	This work was supported by the National Science Foundation, under
	grants DMS-9504974 and DMS-9803599.

    Notice:

	Copyright (c) 1998-2005, Timothy A. Davis.  All Rights Reserved.

	See http://www.cise.ufl.edu/research/sparse/colamd (the colamd.c
	file) for the License.

    Availability:

	The colamd/symamd library is available at

	    http://www.cise.ufl.edu/research/sparse/colamd/

	This is the http://www.cise.ufl.edu/research/sparse/colamd/symamdmex.c
	file.  It requires the colamd.c and colamd.h files.

*/

/* ========================================================================== */
/* === Include files ======================================================== */
/* ========================================================================== */

#include "colamd.h"
#include "mex.h"
#include "matrix.h"
#include <stdlib.h>

/* ========================================================================== */
/* === symamd mexFunction =================================================== */
/* ========================================================================== */

void mexFunction
(
    /* === Parameters ======================================================= */

    int nlhs,			/* number of left-hand sides */
    mxArray *plhs [],		/* left-hand side matrices */
    int nrhs,			/* number of right--hand sides */
    const mxArray *prhs []	/* right-hand side matrices */
)
{
    /* === Local variables ================================================== */

    int *perm ;			/* column ordering of M and ordering of A */
    int *A ;			/* row indices of input matrix A */
    int *p ;			/* column pointers of input matrix A */
    int n_col ;			/* number of columns of A */
    int n_row ;			/* number of rows of A */
    int full ;			/* TRUE if input matrix full, FALSE if sparse */
    double knobs [COLAMD_KNOBS] ; /* colamd user-controllable parameters */
    double *out_perm ;		/* output permutation vector */
    double *out_stats ;		/* output stats vector */
    double *in_knobs ;		/* input knobs vector */
    int i ;			/* loop counter */
    mxArray *Ainput ;		/* input matrix handle */
    int spumoni ;		/* verbosity variable */
    int stats [COLAMD_STATS] ;	/* stats for symamd */

    colamd_printf = mexPrintf ;	/* COLAMD printf routine */

    /* === Check inputs ===================================================== */

    if (nrhs < 1 || nrhs > 2 || nlhs < 0 || nlhs > 2)
    {
	mexErrMsgTxt (
	"symamd: incorrect number of input and/or output arguments.") ;
    }

    /* === Get knobs ======================================================== */

    colamd_set_defaults (knobs) ;
    spumoni = 0 ;

    /* check for user-passed knobs */
    if (nrhs == 2)
    {
	in_knobs = mxGetPr (prhs [1]) ;
	i = mxGetNumberOfElements (prhs [1]) ;
	if (i > 0) knobs [COLAMD_DENSE_ROW] = in_knobs [0] ;
	if (i > 1) spumoni = (int) (in_knobs [1] != 0) ;
    }

    /* print knob settings if spumoni is set */
    if (spumoni)
    {
	mexPrintf ("\nsymamd version %d.%d, %s:\n",
	    COLAMD_MAIN_VERSION, COLAMD_SUB_VERSION, COLAMD_DATE) ;
	if (knobs [COLAMD_DENSE_ROW] >= 0)
	{
	    mexPrintf ("knobs(1): %g, rows/cols with > "
		"max(16,%g*sqrt(size(A,2))) entries removed\n",
		in_knobs [0], knobs [COLAMD_DENSE_ROW]) ;
	}
	else
	{
	    mexPrintf ("knobs(1): %g, no dense rows removed\n", in_knobs [0]) ;
	}
	mexPrintf ("knobs(2): %g, statistics and knobs printed\n",
	    in_knobs [1]) ;
    }

    /* === If A is full, convert to a sparse matrix ========================= */

    Ainput = (mxArray *) prhs [0] ;
    if (mxGetNumberOfDimensions (Ainput) != 2)
    {
	mexErrMsgTxt ("symamd: input matrix must be 2-dimensional.") ;
    }
    full = !mxIsSparse (Ainput) ;
    if (full)
    {
	mexCallMATLAB (1, &Ainput, 1, (mxArray **) prhs, "sparse") ;
    }

    /* === Allocate workspace for symamd ==================================== */

    /* get size of matrix */
    n_row = mxGetM (Ainput) ;
    n_col = mxGetN (Ainput) ;
    if (n_col != n_row)
    {
	mexErrMsgTxt ("symamd: matrix must be square.") ;
    }

    A = mxGetIr (Ainput) ;
    p = mxGetJc (Ainput) ;
    perm = (int *) mxCalloc (n_col+1, sizeof (int)) ;

    /* === Order the rows and columns of A (does not destroy A) ============= */

    if (!symamd (n_col, A, p, perm, knobs, stats, &mxCalloc, &mxFree))
    {
	symamd_report (stats) ;
	mexErrMsgTxt ("symamd error!") ;
    }

    if (full)
    {
	mxDestroyArray (Ainput) ;
    }

    /* === Return the permutation vector ==================================== */

    plhs [0] = mxCreateDoubleMatrix (1, n_col, mxREAL) ;
    out_perm = mxGetPr (plhs [0]) ;
    for (i = 0 ; i < n_col ; i++)
    {
	/* symamd is 0-based, but MATLAB expects this to be 1-based */
	out_perm [i] = perm [i] + 1 ;
    }
    mxFree (perm) ;

    /* === Return the stats vector ========================================== */

    /* print stats if spumoni is set */
    if (spumoni)
    {
	symamd_report (stats) ;
    }

    if (nlhs == 2)
    {
	plhs [1] = mxCreateDoubleMatrix (1, COLAMD_STATS, mxREAL) ;
	out_stats = mxGetPr (plhs [1]) ;
	for (i = 0 ; i < COLAMD_STATS ; i++)
	{
	    out_stats [i] = stats [i] ;
	}

	/* fix stats (5) and (6), for 1-based information on jumbled matrix. */
	/* note that this correction doesn't occur if symamd returns FALSE */
	out_stats [COLAMD_INFO1] ++ ; 
	out_stats [COLAMD_INFO2] ++ ; 
    }
}
