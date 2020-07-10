#ifndef CONNECT___NCBI_GNUTLS__H
#define CONNECT___NCBI_GNUTLS__H

/* $Id: ncbi_gnutls.h 531324 2017-03-23 16:34:47Z lavr $
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
 *   GNUTLS support for SSL (Secure Socket Layer) in connection library
 *
 */

#include <connect/ncbi_socket.h>


/** @addtogroup Sockets
 *
 * @{
 */


#ifdef __cplusplus
extern "C" {
#endif


/** Explicitly setup GNUTLS library to support SSL in ncbi_socket.h[pp].
 *
 * @note Do not use this call!  Instead, use NcbiSetupTls declared in
 *       <connect/ncbi_tls.h>.
 *
 * @sa
 *  NcbiSetupTls
 */
extern NCBI_XCONNECT_EXPORT
SOCKSSL NcbiSetupGnuTls(void);


/** Convert native GNUTLS certificate credentials' handle into an abstract
 *  toolkit handle.
 *
 * @note Does not create a copy of xcred, so xcred must remain valid for the
 * entire duration of a session (or sessions) that it is being used in.
 */
extern NCBI_XCONNECT_EXPORT
NCBI_CRED NcbiCredGnuTls(void* xcred);


#ifdef __cplusplus
} /* extern "C" */
#endif


/* @} */

#endif /* CONNECT___NCBI_GNUTLS_H */
