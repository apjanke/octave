/* ========================================================================== */
/* === umfpack_save_symbolic================================================= */
/* ========================================================================== */

/* -------------------------------------------------------------------------- */
/* UMFPACK Version 4.4, Copyright (c) 2005 by Timothy A. Davis.  CISE Dept,   */
/* Univ. of Florida.  All Rights Reserved.  See ../Doc/License for License.   */
/* web: http://www.cise.ufl.edu/research/sparse/umfpack                       */
/* -------------------------------------------------------------------------- */

int umfpack_di_save_symbolic
(
    void *Symbolic,
    char *filename
) ;

long umfpack_dl_save_symbolic
(
    void *Symbolic,
    char *filename
) ;

int umfpack_zi_save_symbolic
(
    void *Symbolic,
    char *filename
) ;

long umfpack_zl_save_symbolic
(
    void *Symbolic,
    char *filename
) ;

/*
double int Syntax:

    #include "umfpack.h"
    int status ;
    char *filename ;
    void *Symbolic ;
    status = umfpack_di_save_symbolic (Symbolic, filename) ;

double long Syntax:

    #include "umfpack.h"
    long status ;
    char *filename ;
    void *Symbolic ;
    status = umfpack_dl_save_symbolic (Symbolic, filename) ;

complex int Syntax:

    #include "umfpack.h"
    int status ;
    char *filename ;
    void *Symbolic ;
    status = umfpack_zi_save_symbolic (Symbolic, filename) ;

complex long Syntax:

    #include "umfpack.h"
    long status ;
    char *filename ;
    void *Symbolic ;
    status = umfpack_zl_save_symbolic (Symbolic, filename) ;

Purpose:

    Saves a Symbolic object to a file, which can later be read by
    umfpack_*_load_symbolic.  The Symbolic object is not modified.

Returns:

    UMFPACK_OK if successful.
    UMFPACK_ERROR_invalid_Symbolic_object if Symbolic is not valid.
    UMFPACK_ERROR_file_IO if an I/O error occurred.

Arguments:

    void *Symbolic ;	    Input argument, not modified.

	Symbolic must point to a valid Symbolic object, computed by
	umfpack_*_symbolic or loaded by umfpack_*_load_symbolic.

    char *filename ;	    Input argument, not modified.

	A string that contains the filename to which the Symbolic
	object is written.
*/
