#ifndef ASN_CACHE___ASN_CACHE_EXPORT__HPP
#define ASN_CACHE___ASN_CACHE_EXPORT__HPP

/*  $Id: asn_cache_export.h 501731 2016-05-18 15:58:33Z gouriano $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

BEGIN_NCBI_SCOPE


#if defined(NCBI_OS_MSWIN)  &&  defined(NCBI_DLL_BUILD)

#  ifndef _MSC_VER
#    error "This toolkit is not buildable with a compiler other than MSVC."
#  endif

/* Export specifier for library netcache (IBlobStorage)
 */
#ifdef NCBI_ASN_CACHE_EXPORTS
#  define NCBI_ASN_CACHE_EXPORT NCBI_DLL_EXPORT
#else
#  define NCBI_ASN_CACHE_EXPORT NCBI_DLL_IMPORT
#endif

#else

#  define NCBI_ASN_CACHE_EXPORT

#endif

END_NCBI_SCOPE


#endif  // ASN_CACHE___ASN_CACHE_EXPORT__HPP
