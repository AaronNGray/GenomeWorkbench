#ifndef DBAPI_DRIVER___DRIVERS__HPP
#define DBAPI_DRIVER___DRIVERS__HPP

/* $Id: drivers.hpp 583697 2019-04-01 15:00:36Z ucko $
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
 * Author:  Vladimir Soussov
 *
 * File Description:  Drivers' registration
 *
 */


/** @addtogroup DbDriverReg
 *
 * @{
 */


BEGIN_NCBI_SCOPE

// New and recommended.
extern void DBAPI_RegisterDriver_CTLIB      (void);
extern void DBAPI_RegisterDriver_FTDS       (void);
extern void DBAPI_RegisterDriver_FTDS95     (void);
extern void DBAPI_RegisterDriver_FTDS100    (void);
extern void DBAPI_RegisterDriver_ODBC       (void);
extern void DBAPI_RegisterDriver_MYSQL      (void);
// Development ...
// extern void DBAPI_RegisterDriver_GATEWAY (void);

END_NCBI_SCOPE


/* @} */


#endif  /* DBAPI_DRIVER___DRIVERS__HPP */
