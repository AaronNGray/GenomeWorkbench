#ifndef CONNECT___MBEDTLS__THREADING_ALT__H
#define CONNECT___MBEDTLS__THREADING_ALT__H

/* $Id: threading_alt.h 531176 2017-03-22 13:25:44Z lavr $
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
 * Author:  Anton Lavrentiev
 *
 * File Description:
 *   MBEDTLS support for SSL in connection library
 *
 */


struct MT_LOCK_tag;
typedef struct MT_LOCK_tag* MT_LOCK;
typedef MT_LOCK mbedtls_threading_mutex_t;


#endif /*CONNECT___MBEDTLS__THREADING_ALT__H*/
