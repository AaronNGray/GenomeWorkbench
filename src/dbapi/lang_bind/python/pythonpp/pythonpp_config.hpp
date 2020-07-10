/*  $Id: pythonpp_config.hpp 487444 2015-12-17 18:38:53Z ucko $
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
* Authors: Sergey Sikorskiy, Aaron Ucko
*
* File Description: Tiny Python API wrappers
*
* Status: *Initial*
*
* ===========================================================================
*/

#ifndef PYTHONPP_CONFIG_H
#define PYTHONPP_CONFIG_H

#ifdef _MSC_VER 
// disable warning C4005: macro redefinition.
#pragma warning(disable: 4005)
#endif

#ifdef HAVE_FSTAT
#  undef HAVE_FSTAT
#endif
#ifdef HAVE_GETHOSTBYNAME_R
#  undef HAVE_GETHOSTBYNAME_R
#endif

#ifdef __SunOS
#  define __MATHERR_RENAME_EXCEPTION 1
#endif

#include <Python.h>

#ifdef HAVE_FSTAT
#  undef HAVE_FSTAT
#endif
#ifdef HAVE_GETHOSTBYNAME_R
#  undef HAVE_GETHOSTBYNAME_R
#endif

#endif // PYTHONPP_CONFIG_H
