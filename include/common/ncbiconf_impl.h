#ifndef COMMON___NCBICONF_IMPL__H
#define COMMON___NCBICONF_IMPL__H

/* $Id: ncbiconf_impl.h 606329 2020-04-20 16:28:09Z ivanov $
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
 *  Author:  Anton Lavrentiev
 *
 *
 */

/**
 * @file ncbiconf_impl.h
 *
 * Configuration macros.
 */

#ifndef FORWARDING_NCBICONF_H
#  error "The header can be used from <ncbiconf.h> only."
#endif /*!FORWARDING_NCBICONF_H*/

#include <common/ncbi_build_info.h>


/** @addtogroup Portability
 *
 * @{
 */

/* Convenience macro for use when the precise vendor and version
   number don't matter. */
#if defined(NCBI_COMPILER_APPLE_CLANG)  ||  defined(NCBI_COMPILER_LLVM_CLANG)
#  define NCBI_COMPILER_ANY_CLANG 1
#endif

/* Threads configuration
 */

#undef NCBI_NO_THREADS
#undef NCBI_THREADS
#undef NCBI_POSIX_THREADS
#undef NCBI_WIN32_THREADS

#if defined(_MT)  &&  !defined(NCBI_WITHOUT_MT)
#  if defined(NCBI_OS_MSWIN)
#    define NCBI_WIN32_THREADS
#  elif defined(NCBI_OS_UNIX)
#    define NCBI_POSIX_THREADS
#  else
#    define NCBI_NO_THREADS
#  endif
#else
#  define NCBI_NO_THREADS
#endif

#if !defined(NCBI_NO_THREADS)
#  define NCBI_THREADS
#endif

/* Sync Windows/Cygwin preprocessor conditionals governing wide
 * character usage. */

#if defined(UNICODE)  &&  !defined(_UNICODE)
#  define _UNICODE 1
#elif defined(_UNICODE)  &&  !defined(UNICODE)
#  define UNICODE 1
#endif

/* New/nonstandard keywords and attributes
 */

#if defined(__cplusplus)  &&  defined(__has_cpp_attribute) \
    &&  ( !defined(NCBI_COMPILER_GCC)  ||  NCBI_COMPILER_VERSION >= 600)
// Spurn modern standard [[...]] syntax under GCC 5.x, which overstates
// its support for it.  (No great loss, since our wrapper macros can
// readily substitute equivalent legacy __attribute__ syntax.)
#  define NCBI_HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#  define NCBI_HAS_CPP_ATTRIBUTE(x) 0
#endif
#ifndef __has_attribute
#  define __has_attribute(x) 0
#endif

#if defined(__cplusplus)  &&  defined(NCBI_RESTRICT_CXX)
#  define NCBI_RESTRICT NCBI_RESTRICT_CXX
#elif !defined(__cplusplus)  &&  defined(NCBI_RESTRICT_C)
#  define NCBI_RESTRICT NCBI_RESTRICT_C
#elif __STDC_VERSION__ >= 199901 /* C99 specifies restrict */
#  define NCBI_RESTRICT restrict
#else
#  define NCBI_RESTRICT
#endif

#ifdef NCBI_DEPRECATED
#  undef NCBI_DEPRECATED
#endif
/* C++11 [[deprecated]] and legacy synonyms aren't fully interchangeable;
 * depending on the compiler and context, using one form rather than the
 * other may yield a warning or even an outright error.  */
#if __has_attribute(deprecated)
#  define NCBI_LEGACY_DEPRECATED_0      __attribute__((deprecated))
#  define NCBI_LEGACY_DEPRECATED_1(msg) __attribute__((deprecated(msg)))
#elif defined(NCBI_COMPILER_MSVC)
#  define NCBI_LEGACY_DEPRECATED_0      __declspec(deprecated)
#  define NCBI_LEGACY_DEPRECATED_1(msg) __declspec(deprecated(msg))
#else
#  define NCBI_LEGACY_DEPRECATED_0
#  define NCBI_LEGACY_DEPRECATED_1(msg)
#endif
#if NCBI_HAS_CPP_ATTRIBUTE(deprecated)  || \
  (defined(__cplusplus)  &&  defined(NCBI_COMPILER_MSVC))
#  define NCBI_STD_DEPRECATED_0          [[deprecated]]
#  define NCBI_STD_DEPRECATED_1(message) [[deprecated(message)]]
#else
#  define NCBI_STD_DEPRECATED_0          NCBI_LEGACY_DEPRECATED_0
#  define NCBI_STD_DEPRECATED_1(message) NCBI_LEGACY_DEPRECATED_1(message)
#endif
#if !defined(NCBI_COMPILER_GCC)  ||  NCBI_COMPILER_VERSION >= 600
#  define NCBI_STD_DEPRECATED(message) NCBI_STD_DEPRECATED_1(message)
#else
#  define NCBI_STD_DEPRECATED(message)
#endif
#if 0
#  define NCBI_DEPRECATED NCBI_STD_DEPRECATED_0
#else
#  define NCBI_DEPRECATED NCBI_LEGACY_DEPRECATED_0
#endif

#ifndef NCBI_FORCEINLINE
#  ifdef __cplusplus
#    define NCBI_FORCEINLINE inline
#  else
#    define NCBI_FORCEINLINE
#  endif
#endif

#ifdef HAVE_ATTRIBUTE_DESTRUCTOR
#  undef HAVE_ATTRIBUTE_DESTRUCTOR
#endif
#if __has_attribute(destructor)
#  define HAVE_ATTRIBUTE_DESTRUCTOR 1
#endif

#ifndef NCBI_NORETURN
#  if NCBI_HAS_CPP_ATTRIBUTE(noreturn)
#    define NCBI_NORETURN [[noreturn]]
#  elif __has_attribute(noreturn)
#    define NCBI_NORETURN __attribute__((__noreturn__))
#  elif defined(NCBI_COMPILER_MSVC)
#    define NCBI_NORETURN __declspec(noreturn)
#  else
#    define NCBI_NORETURN
#  endif
#endif

#ifdef NCBI_WARN_UNUSED_RESULT
#  undef NCBI_WARN_UNUSED_RESULT
#endif
#if NCBI_HAS_CPP_ATTRIBUTE(nodiscard)
#  define NCBI_WARN_UNUSED_RESULT [[nodiscard]]
#elif __has_attribute(warn_unused_result)
#  define NCBI_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#elif defined(NCBI_COMPILER_MSVC)
#  define NCBI_WARN_UNUSED_RESULT _Check_return_
#else
#  define NCBI_WARN_UNUSED_RESULT
#endif
    
#if NCBI_HAS_CPP_ATTRIBUTE(fallthrough)  &&  \
    (!defined(__clang__)  ||  __clang_major__ > 7  ||  __cplusplus >= 201703L)
#  define NCBI_FALLTHROUGH [[fallthrough]]
#elif NCBI_HAS_CPP_ATTRIBUTE(gcc::fallthrough)
#  define NCBI_FALLTHROUGH [[gcc::fallthrough]]
#elif NCBI_HAS_CPP_ATTRIBUTE(clang::fallthrough)
#  define NCBI_FALLTHROUGH [[clang::fallthrough]]
#elif __has_attribute(fallthrough)
#  define NCBI_FALLTHROUGH __attribute__ ((fallthrough))
#else
#  define NCBI_FALLTHROUGH
#endif

#ifdef NCBI_PACKED
#  undef NCBI_PACKED
#endif
#if NCBI_HAS_CPP_ATTRIBUTE(packed)
#  define NCBI_PACKED [[packed]]
#elif __has_attribute(packed)
#  define NCBI_PACKED __attribute__((packed))
#else
#  define NCBI_PACKED
#endif

/* Definition of packed enum type, to save some memory */
/* enum EMyEnum NCBI_PACKED_ENUM_TYPE(Type) { ... } NCBI_PACKED_ENUM_END(); */
#ifndef NCBI_PACKED_ENUM_TYPE
#  define NCBI_PACKED_ENUM_TYPE(type)
#endif
#ifndef NCBI_PACKED_ENUM_END
#  ifdef NCBI_PACKED
#    define NCBI_PACKED_ENUM_END() NCBI_PACKED
#  else
#    define NCBI_PACKED_ENUM_END()
#  endif
#endif

#ifndef NCBI_UNUSED
#  if NCBI_HAS_CPP_ATTRIBUTE(maybe_unused)
#    define NCBI_UNUSED [[maybe_unused]]
#  elif NCBI_HAS_CPP_ATTRIBUTE(gnu::unused)
#    define NCBI_UNUSED [[gnu::unused]]
#  elif __has_attribute(unused)
#    define NCBI_UNUSED __attribute__((unused))
#  else
#    define NCBI_UNUSED
#  endif
#endif

#ifndef NCBI_WARN_UNUSED_RESULT
#  define NCBI_WARN_UNUSED_RESULT
#endif

#if defined(__SSE4_2__)  ||  defined(__AVX__)
#  define NCBI_SSE 42
#elif defined(__SSE4_1__)
#  define NCBI_SSE 41
#elif defined(__SSSE3__)
#  define NCBI_SSE 40
#elif defined(__SSE3__)
#  define NCBI_SSE 30
#elif defined(__SSE2__)  ||  defined(_M_AMD64)  ||  defined(_M_X64) \
    ||  (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#  define NCBI_SSE 20
#elif defined(__SSE__)  ||  (defined(_M_IX86_FP) && _M_IX86_FP >= 1)
#  define NCBI_SSE 10
#endif

#ifdef __cplusplus
#  if __cplusplus >= 201103L  ||  defined(__GXX_EXPERIMENTAL_CXX0X__) \
      ||  defined(__GXX_EXPERIMENTAL_CPP0X__)  \
      ||  (defined(NCBI_COMPILER_MSVC)  &&  _MSC_VER >= 1800)
#    define NCBI_HAVE_CXX11 1
#  endif
#  if defined(NCBI_HAVE_CXX11) \
      ||  (defined(NCBI_COMPILER_MSVC)  &&  _MSC_VER >= 1600)
#    define HAVE_IS_SORTED 1
#    define HAVE_NULLPTR 1
#  endif
#  if defined(NCBI_HAVE_CXX11)
#    if !defined(NCBI_COMPILER_ICC)  ||  NCBI_COMPILER_VERSION >= 1400
       /* Exclude ICC 13.x and below, which don't support using "enum class"
        * in conjunction with switch. */
#      define HAVE_ENUM_CLASS 1
#    endif
#  endif
#endif


/* Whether there is a proven sufficient support for the 'thread_local'.
 * NOTE that this is a very conservative estimation which can be extended if
 * needed (after proper vetting and testing, of course) to additional
 * platforms. FYI, the known (or at least suspected) issues with some other
 * platforms are:
 *  - Clang  - may not work well on MacOS (runtime; may depend on LIBC)
 *  - VS2015 - may not export well from DLLs
 */
#if (defined(_MSC_VER) && _MSC_VER >= 1914) || \
    (defined(NCBI_COMPILER_GCC) && NCBI_COMPILER_VERSION >= 730)
#  define HAVE_THREAD_LOCAL 1
#endif



#include <common/ncbi_skew_guard.h>


/* @} */

#endif  /* COMMON___NCBICONF_IMPL__H */
