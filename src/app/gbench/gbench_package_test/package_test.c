/*  $Id: package_test.c 35106 2016-03-23 21:03:50Z katargir $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Josh Cherry
 *
 * File Description:  Test a shared object for missing dependencies
 *                    by attempting to load it
 *
 */

#include <ncbiconf.h>

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    void* hand;

    if (argc != 2) {
        printf("Usage: %s library_file\n", argv[0]);
        printf("exits 0 if successfully loaded, nonzero otherwise\n");
        exit(2);  /* Incorrect program usage */
    }

    hand = dlopen(argv[1], RTLD_NOW);
    if (!hand) {
        printf("%s\n", dlerror());
        exit(0);  /* Failure to load */
    }

#if defined(NCBI_COMPILER_ICC) && NCBI_COMPILER_VERSION >= 1000 \
    && NCBI_PLATFORM_BITS == 32
    /* Kludge: force (static) usage of certain ICC-specific libraries. */
    extern void* __intel_cpu_indicator;
    return hand == __intel_cpu_indicator;
#endif

    exit(0);      /* Success */
}
