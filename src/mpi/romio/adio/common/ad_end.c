/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "adio.h"
#include "adio_extern.h"

void ADIO_End(int *error_code)
{
    ADIOI_Datarep *datarep, *datarep_next;

/*    FPRINTF(stderr, "reached end\n"); */

    /* if a default errhandler was set on MPI_FILE_NULL then we need to ensure
     * that our reference to that errhandler is released */
    MPI_File_set_errhandler(MPI_FILE_NULL, MPI_ERRORS_RETURN);

/* free file and info tables used for Fortran interface */
    if (ADIOI_Ftable)
        ADIOI_Free(ADIOI_Ftable);
#ifndef HAVE_MPI_INFO
    if (MPIR_Infotable)
        ADIOI_Free(MPIR_Infotable);
#endif


/* free the memory allocated for a new data representation, if any */
    datarep = ADIOI_Datarep_head;
    while (datarep) {
        datarep_next = datarep->next;
        ADIOI_Free(datarep->name);
        ADIOI_Free(datarep);
        datarep = datarep_next;
    }

    if (ADIOI_syshints != MPI_INFO_NULL)
        MPI_Info_free(&ADIOI_syshints);

    MPI_Op_free(&ADIO_same_amode);

    *error_code = MPI_SUCCESS;
}


/* This is the delete callback function associated with
   ADIO_Init_keyval when MPI_COMM_SELF is freed */

int ADIOI_End_call(MPI_Comm comm, int keyval, void *attribute_val, void
                   *extra_state)
{
    int error_code;

    MPL_UNREFERENCED_ARG(comm);
    MPL_UNREFERENCED_ARG(attribute_val);
    MPL_UNREFERENCED_ARG(extra_state);

    MPI_Comm_free_keyval(&keyval);

    /* The end call will be called after all possible uses of this keyval, even
     * if a file was opened with MPI_COMM_SELF.  Note, this assumes LIFO
     * MPI_COMM_SELF attribute destruction behavior mandated by MPI-2.2. */
    if (ADIOI_cb_config_list_keyval != MPI_KEYVAL_INVALID)
        MPI_Comm_free_keyval(&ADIOI_cb_config_list_keyval);

    if (ADIOI_Flattened_type_keyval != MPI_KEYVAL_INVALID)
        MPI_Type_free_keyval(&ADIOI_Flattened_type_keyval);

    ADIO_End(&error_code);
    return error_code;
}
