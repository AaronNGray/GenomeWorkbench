#ifndef GUI_WIDGETS_WX___FIXED_BASE__HPP
#define GUI_WIDGETS_WX___FIXED_BASE__HPP

/*  $Id: fixed_base.hpp 30858 2014-07-31 14:05:43Z ucko $
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
 * Author:  Aaron Ucko
 *
 */

/// @file fixed_base.hpp
/// Workaround for wxWidgets header errors in certain configurations;
/// MUST be included (at least indirectly) before any wxWidgets headers.
///
/// In certain configurations (including in particular those on FreeBSD 10,
/// which features a pure LLVM toolchain but lacks a fully C++98-compliant
/// <wchar.h>), inline calls to wide-string functions from wxcrt.h can
/// yield ambiguity errors due to the presence of formally distinct
/// versions in :: and std::, even though the latter are trivial
/// const-preserving wrappers for the former.
///
/// All such calls already go through wxCRT_Str*W macros, conditionally
/// redefined here to force the use of std::wcs*.

#include <cwchar>

#if defined(_LIBCPP_VERSION)  &&  !defined(_WCHAR_H_CPLUSPLUS_98_CONFORMANCE_)

#  include <wx/defs.h>
#  include <wx/wxcrtbase.h>

#  undef  wxCRT_StrchrW
#  undef  wxCRT_StrpbrkW
#  undef  wxCRT_StrrchrW
#  undef  wxCRT_StrstrW

#  define wxCRT_StrchrW    std::wcschr
#  define wxCRT_StrpbrkW   std::wcspbrk
#  define wxCRT_StrrchrW   std::wcsrchr
#  define wxCRT_StrstrW    std::wcsstr

#endif

#endif  /* GUI_WIDGETS_WX___FIXED_BASE__HPP */
