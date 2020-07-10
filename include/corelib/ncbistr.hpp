#ifndef CORELIB___NCBISTR__HPP
#define CORELIB___NCBISTR__HPP

/*  $Id: ncbistr.hpp 607690 2020-05-06 16:46:12Z ivanov $
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
 * Authors:  Eugene Vasilchenko, Denis Vakatov
 *
 *
 */

/// @file ncbistr.hpp
/// The NCBI C++ standard methods for dealing with std::string


#include <corelib/ncbi_limits.hpp>
#include <corelib/tempstr.hpp>
#include <corelib/ncbierror.hpp>
#ifdef NCBI_OS_OSF1
#  include <strings.h>
#endif
#include <stdarg.h>
#include <time.h>
#include <set>
#include <functional>


BEGIN_NCBI_NAMESPACE;

/** @addtogroup String
 *
 * @{
 */

/// Empty "C" string (points to a '\0').
NCBI_XNCBI_EXPORT extern const char *const kEmptyCStr;
#define NcbiEmptyCStr NCBI_NS_NCBI::kEmptyCStr

#if defined(HAVE_WSTRING)
NCBI_XNCBI_EXPORT extern const wchar_t *const kEmptyWCStr;
#define NcbiEmptyWCStr NCBI_NS_NCBI::kEmptyWCStr
#endif

/// Empty "C++" string.
#if defined(NCBI_OS_MSWIN)  \
    ||  (defined(NCBI_OS_LINUX) \
         &&  (defined(NCBI_COMPILER_GCC) || defined(NCBI_COMPILER_ANY_CLANG)))
class CNcbiEmptyString
{
public:
    /// Get string.
    inline
    static const string& Get(void)
    {
        static string empty_str;
        return empty_str;
    }
};
#if defined(HAVE_WSTRING)
class CNcbiEmptyWString
{
public:
    /// Get string.
    static const wstring& Get(void)
    {
        static wstring empty_str;
        return empty_str;
    }
};
#endif
#else
class NCBI_XNCBI_EXPORT CNcbiEmptyString
{
public:
    /// Get string.
    static const string& Get(void);
private:
    /// Helper method to initialize private data member and return
    /// null string.
    static const string& FirstGet(void);
    static const string* m_Str;     ///< Null string pointer.
};

#  if defined(HAVE_WSTRING)
class NCBI_XNCBI_EXPORT CNcbiEmptyWString
{
public:
    /// Get string.
    static const wstring& Get(void);
private:
    /// Helper method to initialize private data member and return
    /// null string.
    static const wstring& FirstGet(void);
    static const wstring* m_Str;     ///< Null string pointer.
};
#  endif
#endif // NCBI_OS_MSWIN....


#define NcbiEmptyString NCBI_NS_NCBI::CNcbiEmptyString::Get()
#define kEmptyStr NcbiEmptyString

#if defined(HAVE_WSTRING)
#  define NcbiEmptyWString NCBI_NS_NCBI::CNcbiEmptyWString::Get()
#  define kEmptyWStr NcbiEmptyWString
#endif

// SIZE_TYPE and NPOS

typedef NCBI_NS_STD::string::size_type SIZE_TYPE;
#define NPOS NCBI_NS_STD::string::npos



/////////////////////////////////////////////////////////////////////////////
// Unicode-related definitions and conversions

/// Unicode character
typedef Uint4 TUnicodeSymbol;
/// Unicode string
typedef basic_string<TUnicodeSymbol> TStringUnicode;

#if defined(NCBI_OS_MSWIN) && defined(_UNICODE)

typedef wchar_t TXChar;
typedef wstring TXString;

#  if !defined(_TX)
#    define _TX(x) L ## x
#  endif

#  if defined(_DEBUG)
#    define _T_XSTRING(x) \
    ncbi::CUtf8::AsBasicString<ncbi::TXChar>(x, NULL, ncbi::CUtf8::eValidate)
#  else
#    define _T_XSTRING(x) \
    ncbi::CUtf8::AsBasicString<ncbi::TXChar>(x, NULL, ncbi::CUtf8::eNoValidate)
#  endif
#  define _T_STDSTRING(x)     ncbi::CUtf8::AsUTF8(x)
#  define _T_XCSTRING(x)      _T_XSTRING(x).c_str()
#  define _T_CSTRING(x)       _T_STDSTRING(x).c_str()

#  define NcbiEmptyXCStr   NcbiEmptyWCStr
#  define NcbiEmptyXString NcbiEmptyWString
#  define kEmptyXStr       kEmptyWStr
#  define kEmptyXCStr      kEmptyWCStr

#else

typedef char   TXChar;
typedef string TXString;

#  if !defined(_TX)
#    define _TX(x) x
#  endif

#  define _T_XSTRING(x)       (x)
#  define _T_STDSTRING(x)     (x)
#  define _T_XCSTRING(x)      ncbi::impl_ToCString(x)
#  define _T_CSTRING(x)       (x)

#  define NcbiEmptyXCStr   NcbiEmptyCStr
#  define NcbiEmptyXString NcbiEmptyString
#  define kEmptyXStr       kEmptyStr
#  define kEmptyXCStr      kEmptyCStr

inline const char* impl_ToCString(const char*   s) { return s; }
inline const char* impl_ToCString(const string& s) { return s.c_str(); }

#endif


/////////////////////////////////////////////////////////////////////////////
///

#if defined(NCBI_CUTF8_ENCODING_CLASSIC)  ||  !defined(HAVE_ENUM_CLASS)
enum EEncoding {
    eEncoding_Unknown,
    eEncoding_UTF8,
    eEncoding_Ascii,
    eEncoding_ISO8859_1,    ///< Note:  From the point of view of the C++
    ///< Toolkit, the ISO 8859-1 character set includes
    ///< symbols 0x00 through 0xFF except 0x80 through
    ///< 0x9F.
    eEncoding_Windows_1252
};
#else
// Temporary safeguard to protect against implicit conversion of EEncoding
// to size_t, etc
// @attention  Do not use "EEncoding::Xxx" values directly, as they will go
//             away eventually! Use the "eEncoding_Xxx" values instead.
enum class EEncoding {
    Unknown,      ///< Do not use this directly!  It will go away eventually!
    UTF8,         ///< Do not use this directly!  It will go away eventually!
    Ascii,        ///< Do not use this directly!  It will go away eventually!
    ISO8859_1,    ///< Do not use this directly!  It will go away eventually!
    Windows_1252  ///< Do not use this directly!  It will go away eventually!
};
#define eEncoding_Unknown      EEncoding::Unknown
#define eEncoding_UTF8         EEncoding::UTF8
#define eEncoding_Ascii        EEncoding::Ascii
#define eEncoding_ISO8859_1    EEncoding::ISO8859_1
#define eEncoding_Windows_1252 EEncoding::Windows_1252
#endif


/////////////////////////////////////////////////////////////////////////////
///
/// NStr --
///
/// Encapsulates class-wide string processing functions.

class NCBI_XNCBI_EXPORT NStr
{
public:
    /// Common conversion flags.
    enum EConvErrFlags {
        /// Do not throw an exception on error.
        /// Could be used with methods throwing an exception by default, ignored otherwise.
        /// Just return zero and set errno to non-zero instead of throwing an exception.
        /// We recommend the following technique to check against errors
        /// with minimum overhead when this flag is used:
        /// @code
        ///     if (!retval  &&  errno != 0)
        ///        ERROR;
        /// @endcode
        /// And for StringToDouble*() variants:
        /// @code
        ///     if (retval == HUGE_VAL  ||  retval == -HUGE_VAL  ||  
        ///        !retval  &&  errno != 0)
        ///        ERROR;
        /// @endcode
        fConvErr_NoThrow      = (1 <<  0),
        /*
        fConvErr_NoErrno      = (1 <<  1),  ///< Do not set errno at all.
                                            ///< If used together with fConvErr_NoThrow flag
                                            ///< returns 0 on error (-1 for StringToNonNegativeInt).
        */
        fConvErr_NoErrMessage = (1 <<  2)   ///< Set errno, but do not set CNcbiError message on error
    };
    typedef int TConvErrFlags;    ///< Bitwise OR of "EConvErrFlags"

    /// Number to string conversion flags.
    ///
    /// NOTE: 
    ///   If specified base in the *ToString() methods is not default 10,
    ///   that some flags like fWithSign and fWithCommas will be ignored.
    enum ENumToStringFlags {
        fUseLowercase            = (1 <<  4),     ///< Use lowercase letters for string representation for bases above 10
        fWithRadix               = (1 <<  5),     ///< Prefix the output value with radix for "well-known" bases like 8 ("0") and 16 ("0x")
        fWithSign                = (1 <<  6),     ///< Prefix the output value with a sign ('+'/'-')
        fWithCommas              = (1 <<  7),     ///< Use commas as thousands separator
        fDoubleFixed             = (1 <<  8),     ///< DoubleToString*(): Use n.nnnn format for double conversions
        fDoubleScientific        = (1 <<  9),     ///< DoubleToString*(): Use scientific format for double conversions
        fDoublePosix             = (1 << 10),     ///< DoubleToString*(): Use C locale  for double conversions
        fDoubleGeneral           = fDoubleFixed | fDoubleScientific,
        // Additional flags to convert "software" qualifiers (see UInt8ToString_DataSize)
        fDS_Binary               = (1 << 11),     ///< UInt8ToString_DataSize(): Use 1024 as a kilobyte factor, not 1000.
        fDS_NoDecimalPoint       = (1 << 12),     ///< UInt8ToString_DataSize(): Do not add a decimal point ("10KB" vs "10.0KB")
        fDS_PutSpaceBeforeSuffix = (1 << 13),     ///< UInt8ToString_DataSize(): Add space between value and qualifiers, like "10.0 KB"
        fDS_ShortSuffix          = (1 << 14),     ///< UInt8ToString_DataSize(): Use short suffix, like "10.0K"
        fDS_PutBSuffixToo        = (1 << 15)      ///< UInt8ToString_DataSize(): Use "B" suffix for small bytes values.
    };
    typedef int TNumToStringFlags;    ///< Bitwise OR of "ENumToStringFlags"

    /// String to number conversion flags.
    enum EStringToNumFlags {
        fMandatorySign           = (1 << 17),     ///< Check on mandatory sign. See 'ENumToStringFlags::fWithSign'.
        fAllowCommas             = (1 << 18),     ///< Allow commas. See 'ENumToStringFlags::fWithCommas'.
        fAllowLeadingSpaces      = (1 << 19),     ///< Ignore leading spaces in converted string.
        fAllowLeadingSymbols     = (1 << 20) | fAllowLeadingSpaces,
                                                  ///< Ignore leading non-numeric characters.
        fAllowTrailingSpaces     = (1 << 21),     ///< Ignore trailing space characters.
        fAllowTrailingSymbols    = (1 << 22) | fAllowTrailingSpaces,
                                                  ///< Ignore trailing non-numerics characters.
        fDecimalPosix            = (1 << 23),     ///< StringToDouble*(): For decimal point, use C locale.
        fDecimalPosixOrLocal     = (1 << 24),     ///< StringToDouble*(): For decimal point, try both C and current locale.
        fDecimalPosixFinite      = (1 << 25),     ///< StringToDouble*(): Keep result finite and normalized:
                                                  ///< if DBL_MAX < result < INF,     result becomes DBL_MAX
                                                  ///< if       0 < result < DBL_MIN, result becomes DBL_MIN
        // Additional flags to convert "software" qualifiers (see StringToUInt8_DataSize)
        fDS_ForceBinary          = (1 << 26),     ///< StringToUInt8_DataSize(): Use 1024 as a kilobyte factor regardless of suffix, like "KB" or "KiB".
        fDS_ProhibitFractions    = (1 << 27),     ///< StringToUInt8_DataSize(): Ignore any fraction part of a value, "1.2K" ~ "1K"
        fDS_ProhibitSpaceBeforeSuffix = (1 << 28) ///< StringToUInt8_DataSize(): Do not allow spaces between value and suffix, like "10 K".
    };
    typedef int TStringToNumFlags;   ///< Bitwise OR of "EStringToNumFlags"

    /// Convert string to a numeric value.
    ///
    /// @param str
    ///   String to be converted.
    /// @param flags
    ///   Optional flags to tune up how the string is converted to value.
    /// @param base
    ///   Radix base. Allowed values are 0, 2..36. Zero means to use the
    ///   first characters to determine the base - a leading "0x" or "0X"
    ///   means base 16; otherwise a leading 0 means base 8; otherwise base 10.
    /// @return
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted value.
    ///   - Otherwise, if fConvErr_NoThrow is not set, throw an exception.
    ///   - Otherwise, set errno to non-zero and return zero.
    template <typename TNumeric>
    static TNumeric StringToNumeric(const CTempString str,
                                    TStringToNumFlags flags = 0,
                                    int               base  = 10)
    {
        return x_StringToNumeric<TNumeric>(str, flags, base);
    }

    /// Convert string to a numeric value.
    ///
    /// @param str [in]
    ///   String to be converted.
    /// @param value [out]
    ///   The numeric value represented by "str". Zero on any error.
    /// @param flags [in]
    ///   Optional flags to tune up how the string is converted to value.
    /// @param base [in]
    ///   Radix base. Allowed values are 0, 2..36. Zero means to use the
    ///   first characters to determine the base - a leading "0x" or "0X"
    ///   means base 16; otherwise a leading 0 means base 8; otherwise base 10.
    /// @return
    ///   - If conversion succeeds, set errno to zero, set the value, and
    ///     return true.
    ///   - Otherwise, if fConvErr_NoThrow is not set, throw an exception.
    ///   - Otherwise, set errno to non-zero, set the value to zero, and
    ///     return false.
    template <typename TNumeric>
    static bool StringToNumeric(const CTempString str,
                                TNumeric*         value, /*[out]*/ 
                                TStringToNumFlags flags = 0,
                                int               base  = 10)
    {
        return x_StringToNumeric(str, value, flags, base);
    }

    /// Convert string to non-negative integer value.
    ///
    /// @param str
    ///   String containing only digits, representing non-negative 
    ///   decimal value in the int range: [0..kMax_Int].
    /// @param flags
    ///   How to convert string to value.
    ///   Only fConvErr_NoErrMessage flag is supported here.
    /// @return
    ///   - If conversion succeeds, set errno to zero and return the converted value.
    ///   - Otherwise, set errno to non-zero and return -1.
    static int StringToNonNegativeInt(const CTempString str, TConvErrFlags flags = 0);

    /// @deprecated
    ///   Use template-based StringToNumeric<> or StringToNonNegativeInt() instead.
    NCBI_DEPRECATED
    static int StringToNumeric(const string& str)
    {
        return StringToNonNegativeInt(str);
    }

    /// Convert string to int.
    ///
    /// @param str
    ///   String to be converted.
    /// @param flags
    ///   How to convert string to value.
    /// @param base
    ///   Radix base. Allowed values are 0, 2..36. Zero means to use the
    ///   first characters to determine the base - a leading "0x" or "0X"
    ///   means base 16; otherwise a leading 0 means base 8; otherwise base 10.
    /// @return
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted value.
    ///   - Otherwise, if fConvErr_NoThrow is not set, throw an exception.
    ///   - Otherwise, set errno to non-zero and return zero.
    static int StringToInt(const CTempString str,
                           TStringToNumFlags flags = 0,
                           int               base  = 10);

    /// Convert string to unsigned int.
    ///
    /// @param str
    ///   String to be converted.
    /// @param flags
    ///   How to convert string to value.
    /// @param base
    ///   Radix base. Allowed values are 0, 2..36. Zero means to use the
    ///   first characters to determine the base - a leading "0x" or "0X"
    ///   means base 16; otherwise a leading 0 means base 8; otherwise base 10.
    /// @return
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted value.
    ///   - Otherwise, if fConvErr_NoThrow is not set, throw an exception.
    ///   - Otherwise, set errno to non-zero and return zero.
    static unsigned int StringToUInt(const CTempString str,
                                     TStringToNumFlags flags = 0,
                                     int               base  = 10);

    /// Convert string to long.
    ///
    /// @param str
    ///   String to be converted.
    /// @param flags
    ///   How to convert string to value.
    /// @param base
    ///   Radix base. Allowed values are 0, 2..36. Zero means to use the
    ///   first characters to determine the base - a leading "0x" or "0X"
    ///   means base 16; otherwise a leading 0 means base 8; otherwise base 10.
    /// @return
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted value.
    ///   - Otherwise, if fConvErr_NoThrow is not set, throw an exception.
    ///   - Otherwise, set errno to non-zero and return zero.
    static long StringToLong(const CTempString str,
                             TStringToNumFlags flags = 0,
                             int               base  = 10);

    /// Convert string to unsigned long.
    ///
    /// @param str
    ///   String to be converted.
    /// @param flags
    ///   How to convert string to value.
    /// @param base
    ///   Radix base. Allowed values are 0, 2..36. Zero means to use the
    ///   first characters to determine the base - a leading "0x" or "0X"
    ///   means base 16; otherwise a leading 0 means base 8; otherwise base 10.
    /// @return
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted value.
    ///   - Otherwise, if fConvErr_NoThrow is not set, throw an exception.
    ///   - Otherwise, set errno to non-zero and return zero.
    static unsigned long StringToULong(const CTempString str,
                                       TStringToNumFlags flags = 0,
                                       int               base  = 10);

    /// Convert string to double-precision value (analog of strtod function)
    ///
    /// @param str
    ///   String to be converted.
    /// @param endptr
    ///   Pointer to character that stops scan.
    /// @return
    ///   Double-precision value.
    ///   This function always uses dot as decimal separator.
    ///   - on overflow, it returns HUGE_VAL and sets errno to ERANGE;
    ///   - on underflow, it returns 0 and sets errno to ERANGE;
    ///   - if conversion was impossible, it returns 0 and sets errno.
    ///   Also, when input string equals (case-insensitive) to
    ///   - "NAN", the function returns NaN;
    ///   - "INF" or "INFINITY", the function returns HUGE_VAL;
    ///   - "-INF" or "-INFINITY", the function returns -HUGE_VAL;
    /// @note
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted value.
    ///   - Otherwise, set errno to non-zero and return zero.
    ///   - Denormal or infinite results are considered successful conversion.
    ///   - To enforce finite and normalized result, use fDecimalPosixFinite flag.
    ///   - This function is meant to be more "low-level" than other
    ///     StringToXxx functions - for example, it allows trailing characters
    ///     (and doesn't include a flags parameter for tweaking such behavior).
    ///     This could result in strings like "nanosecond" being converted to
    ///     NaN, "-inf=input_file" being converted to -INF, or other unexpected
    ///     behavior. Therefore, please consider using StringToDouble unless
    ///     you specifically need this functionality.
    static double StringToDoublePosix(const char* str, char** endptr=0,
                                      TStringToNumFlags flags=0);


    /// Convert string to double.
    ///
    /// @param str
    ///   String to be converted.
    /// @param flags
    ///   How to convert string to value.
    ///   Do not support fAllowCommas flag.
    /// @return
    ///   - If invalid flags are passed, throw an exception.
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted value.
    ///   - Otherwise, if fConvErr_NoThrow is not set, throw an exception.
    ///   - Otherwise, set errno to non-zero and return zero.
    /// @note
    ///   - Denormal or infinite results are considered successful conversion.
    ///   - To enforce finite and normalized result, use fDecimalPosixFinite flag.
    static double StringToDouble(const CTempStringEx str,
                                 TStringToNumFlags   flags = 0);

    /// This version accepts zero-terminated string
    /// @deprecated
    ///   It is unsafe to use this method directly, please use StringToDouble()
    ///   instead.
    NCBI_DEPRECATED
    static double StringToDoubleEx(const char* str, size_t size,
                                   TStringToNumFlags flags = 0);

    /// Convert string to Int8.
    ///
    /// @param str
    ///   String to be converted.
    /// @param flags
    ///   How to convert string to value.
    /// @param base
    ///   Radix base. Allowed values are 0, 2..36. Zero means to use the
    ///   first characters to determine the base - a leading "0x" or "0X"
    ///   means base 16; otherwise a leading 0 means base 8; otherwise base 10.
    /// @return
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted value.
    ///   - Otherwise, if fConvErr_NoThrow is not set, throw an exception.
    ///   - Otherwise, set errno to non-zero and return zero.
    static Int8 StringToInt8(const CTempString str,
                             TStringToNumFlags flags = 0,
                             int               base  = 10);

    /// Convert string to Uint8.
    ///
    /// @param str
    ///   String to be converted.
    /// @param flags
    ///   How to convert string to value.
    /// @param base
    ///   Radix base. Allowed values are 0, 2..36. Zero means to use the
    ///   first characters to determine the base - a leading "0x" or "0X"
    ///   means base 16; otherwise a leading 0 means base 8; otherwise base 10.
    /// @return
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted value.
    ///   - Otherwise, if fConvErr_NoThrow is not set, throw an exception.
    ///   - Otherwise, set errno to non-zero and return zero.
    static Uint8 StringToUInt8(const CTempString str,
                               TStringToNumFlags flags = 0,
                               int               base  = 10);

    /// Convert string that can contain "software" qualifiers to Uint8. 
    ///
    /// String can contain "software" qualifiers: G(giga-), MB(mega-),
    /// KiB (kibi-) etc.
    /// Example: 100MB, 1024KiB, 5.7G.
    /// Meaning of qualifiers depends on flags and by default is 1000-based
    /// (i.e. K=1000, M=10^6 etc.) except in cases when qualifiers with "iB"
    /// are used, i.e. KiB=1024, MiB=1024^2 etc. When flags parameter contains
    /// fDS_ForceBinary then qualifiers without "iB" (i.e. "K" or "MB") will
    /// also be 1024-based.
    /// String can contain a decimal fraction (except when fDS_ProhibitFractions
    /// flag is used), in this case the resultant Uint8 number will be rounded
    /// to fit into integer value.
    ///
    /// @param str
    ///   String to be converted.
    /// @param flags
    ///   How to convert string to value.
    /// @return
    ///   - If invalid flags are passed, throw an exception.
    ///   - If conversion succeeds, return the converted value.
    ///   - Otherwise, if fConvErr_NoThrow is not set, throw an exception.
    ///   - Otherwise, set errno to non-zero and return zero.
    static Uint8 StringToUInt8_DataSize(const CTempString str,
                                        TStringToNumFlags flags = 0);

    /// Convert string to number of bytes. 
    ///
    /// String can contain "software" qualifiers: MB(megabyte), KB (kilobyte).
    /// Example: 100MB, 1024KB
    /// Note the qualifiers are power-of-2 based, aka kibi-, mebi- etc, so that
    /// 1KB = 1024B (not 1000B), 1MB = 1024KB = 1048576B, etc.
    ///
    /// @param str
    ///   String to be converted.
    /// @param flags
    ///   How to convert string to value.
    /// @param base
    ///   Numeric base of the number (before the qualifier). Allowed values
    ///   are 0, 2..20. Zero means to use the first characters to determine
    ///   the base - a leading "0x" or "0X" means base 16; otherwise a
    ///   leading 0 means base 8; otherwise base 10.
    ///   The base is limited to 20 to prevent 'K' from being interpreted as
    ///   a digit in the number.
    /// @return
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted value.
    ///   - Otherwise, if fConvErr_NoThrow is not set, throw an exception.
    ///   - Otherwise, set errno to non-zero and return zero.
    /// @deprecated  Use StringToUInt8_DataSize(str, flags) instead.
    NCBI_DEPRECATED
    static Uint8 StringToUInt8_DataSize(const CTempString str,
                                        TStringToNumFlags flags,
                                        int               base);

    /// Convert string to size_t.
    ///
    /// @param str
    ///   String to be converted.
    /// @param flags
    ///   How to convert string to value.
    /// @param base
    ///   Radix base. Allowed values are 0, 2..36. Zero means to use the
    ///   first characters to determine the base - a leading "0x" or "0X"
    ///   means base 16; otherwise a leading 0 means base 8; otherwise base 10.
    /// @return
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted value.
    ///   - Otherwise, if fConvErr_NoThrow is not set, throw an exception.
    ///   - Otherwise, set errno to non-zero and return zero.
    static size_t StringToSizet(const CTempString str,
                                TStringToNumFlags flags = 0,
                                int               base  = 10);

    /// Convert string to pointer.
    ///
    /// @param str
    ///   String to be converted.
    /// @param flags
    ///   How to convert string to value.
    ///   Only fConvErr_NoErrMessage flag is supported here.
    /// @return
    ///   Pointer value corresponding to its string representation.
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted value.
    ///   - Otherwise, set errno to non-zero and return NULL.
    static const void* StringToPtr(const CTempStringEx str, TConvErrFlags flags = 0);

    /// Convert character to integer.
    ///
    /// @param ch
    ///   Character to be converted.
    /// @return
    ///   Integer (0..15) corresponding to the "ch" as a hex digit.
    ///   Return -1 on error.
    static int HexChar(char ch);

    /// Convert numeric value to string.
    ///
    /// @param value
    ///   Numeric value to be converted.
    /// @param flags
    ///   How to convert value to string.
    /// @param base
    ///   Radix base. Default is 10. Allowed values are 2..36.
    ///   Bases 8 and 16 do not add leading '0' and '0x' accordingly.
    ///   If necessary you should add it yourself.
    ///   If value is float or double type, the parameter is ignored.
    /// @return
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted string value.
    ///   - Otherwise, set errno to non-zero and return empty string.
    template<typename TNumeric>
    static typename enable_if< is_arithmetic<TNumeric>::value || is_convertible<TNumeric, Int8>::value, string>::type
    NumericToString(TNumeric value, TNumToStringFlags flags = 0, int base = 10)
    {
        string ret;
        x_NumericToString(ret, value, flags, base);
        return ret;
    }

    /// Convert numeric value to string.
    ///
    /// @param out_str
    ///   Output string variable.
    /// @param value
    ///   Numeric value to be converted.
    /// @param flags
    ///   How to convert value to string.
    /// @param base
    ///   Radix base. Default is 10. Allowed values are 2..36.
    ///   Bases 8 and 16 do not add leading '0' and '0x' accordingly.
    ///   If necessary you should add it yourself.
    ///   If value is float or double type, the parameter is ignored.
    /// @note
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted string value in 'out_str'.
    ///   - Otherwise, set errno to non-zero, value of 'out_str' is undefined.
    template<typename TNumeric>
    static void NumericToString(string& out_str, TNumeric value,
                                TNumToStringFlags flags = 0, int base = 10)
    {
        x_NumericToString(out_str, value, flags, base);
    }
    
    /// Convert int to string.
    ///
    /// @param value
    ///   Integer value to be converted.
    /// @param flags
    ///   How to convert value to string.
    /// @param base
    ///   Radix base. Default is 10. Allowed values are 2..36.
    ///   Bases 8 and 16 do not add leading '0' and '0x' accordingly.
    ///   If necessary you should add it yourself.
    /// @return
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted string value.
    ///   - Otherwise, set errno to non-zero and return empty string.
    static string IntToString(int value, TNumToStringFlags flags = 0,
                              int base = 10);

    static string IntToString(unsigned int value, TNumToStringFlags flags = 0,
                              int base = 10);

    /// Convert int to string.
    ///
    /// @param out_str
    ///   Output string variable.
    /// @param value
    ///   Integer value to be converted.
    /// @param flags
    ///   How to convert value to string.
    /// @param base
    ///   Radix base. Default is 10. Allowed values are 2..36.
    ///   Bases 8 and 16 do not add leading '0' and '0x' accordingly.
    ///   If necessary you should add it yourself.
    /// @note
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted string value in 'out_str'.
    ///   - Otherwise, set errno to non-zero, value of 'out_str' is undefined.
    static void IntToString(string& out_str, int value, 
                            TNumToStringFlags flags = 0,
                            int               base  = 10);

    static void IntToString(string& out_str, unsigned int value, 
                            TNumToStringFlags flags = 0,
                            int               base  = 10);

    /// Convert UInt to string.
    ///
    /// @param value
    ///   Integer value (unsigned long) to be converted.
    /// @param flags
    ///   How to convert value to string.
    /// @param base
    ///   Radix base. Default is 10. Allowed values are 2..36.
    ///   Bases 8 and 16 do not add leading '0' and '0x' accordingly.
    ///   If necessary you should add it yourself.
    /// @return
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted string value.
    ///   - Otherwise, set errno to non-zero and return empty string.
    static string UIntToString(unsigned int      value,
                               TNumToStringFlags flags = 0,
                               int               base  = 10);

    static string UIntToString(int               value,
                               TNumToStringFlags flags = 0,
                               int               base  = 10);

    /// Convert UInt to string.
    ///
    /// @param out_str
    ///   Output string variable
    /// @param value
    ///   Integer value (unsigned long) to be converted.
    /// @param flags
    ///   How to convert value to string.
    /// @param base
    ///   Radix base. Default is 10. Allowed values are 2..36.
    ///   Bases 8 and 16 do not add leading '0' and '0x' accordingly.
    ///   If necessary you should add it yourself.
    /// @note
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted string value in 'out_str'.
    ///   - Otherwise, set errno to non-zero, value of 'out_str' is undefined.
    static void UIntToString(string& out_str, unsigned int value,
                             TNumToStringFlags flags = 0,
                             int               base  = 10);

    static void UIntToString(string& out_str, int value,
                             TNumToStringFlags flags = 0,
                             int               base  = 10);

    /// Convert Int to string.
    ///
    /// @param value
    ///   Integer value (long) to be converted.
    /// @param flags
    ///   How to convert value to string.
    /// @param base
    ///   Radix base. Default is 10. Allowed values are 2..36.
    ///   Bases 8 and 16 do not add leading '0' and '0x' accordingly.
    ///   If necessary you should add it yourself.
    /// @return
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted string value.
    ///   - Otherwise, set errno to non-zero and return empty string.
    static string LongToString(long value, TNumToStringFlags flags = 0,
                               int base = 10);

    /// Convert Int to string.
    ///
    /// @param out_str
    ///   Output string variable.
    /// @param value
    ///   Integer value (long) to be converted.
    /// @param flags
    ///   How to convert value to string.
    /// @param base
    ///   Radix base. Default is 10. Allowed values are 2..36.
    ///   Bases 8 and 16 do not add leading '0' and '0x' accordingly.
    ///   If necessary you should add it yourself.
    /// @note
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted string value in 'out_str'.
    ///   - Otherwise, set errno to non-zero, value of 'out_str' is undefined.
    static void LongToString(string& out_str, long value, 
                             TNumToStringFlags flags = 0,
                             int               base  = 10);

    /// Convert unsigned long to string.
    ///
    /// @param value
    ///   Integer value (unsigned long) to be converted.
    /// @param flags
    ///   How to convert value to string.
    /// @param base
    ///   Radix base. Default is 10. Allowed values are 2..36.
    ///   Bases 8 and 16 do not add leading '0' and '0x' accordingly.
    ///   If necessary you should add it yourself.
    /// @return
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted string value.
    ///   - Otherwise, set errno to non-zero and return empty string.
    static string ULongToString(unsigned long     value,
                                TNumToStringFlags flags = 0,
                                int               base  = 10);

    /// Convert unsigned long to string.
    ///
    /// @param out_str
    ///   Output string variable
    /// @param value
    ///   Integer value (unsigned long) to be converted.
    /// @param flags
    ///   How to convert value to string.
    /// @param base
    ///   Radix base. Default is 10. Allowed values are 2..36.
    ///   Bases 8 and 16 do not add leading '0' and '0x' accordingly.
    ///   If necessary you should add it yourself.
    /// @note
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted string value in 'out_str'.
    ///   - Otherwise, set errno to non-zero, value of 'out_str' is undefined.
    static void ULongToString(string& out_str, unsigned long value,
                              TNumToStringFlags flags = 0,
                              int               base  = 10);

    /// Convert Int8 to string.
    ///
    /// @param value
    ///   Integer value (Int8) to be converted.
    /// @param flags
    ///   How to convert value to string.
    /// @param base
    ///   Radix base. Default is 10. Allowed values are 2..36.
    ///   Bases 8 and 16 do not add leading '0' and '0x' accordingly.
    ///   If necessary you should add it yourself.
    /// @return
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted string value.
    ///   - Otherwise, set errno to non-zero and return empty string.
    static string Int8ToString(Int8 value,
                               TNumToStringFlags flags = 0,
                               int               base  = 10);

    /// Convert Int8 to string.
    ///
    /// @param out_str
    ///   Output string variable
    /// @param value
    ///   Integer value (Int8) to be converted.
    /// @param flags
    ///   How to convert value to string.
    /// @param base
    ///   Radix base. Default is 10. Allowed values are 2..36.
    ///   Bases 8 and 16 do not add leading '0' and '0x' accordingly.
    ///   If necessary you should add it yourself.
    /// @note
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted string value in 'out_str'.
    ///   - Otherwise, set errno to non-zero, value of 'out_str' is undefined.
    static void Int8ToString(string& out_str, Int8 value,
                             TNumToStringFlags flags = 0,
                             int               base  = 10);

    /// Convert UInt8 to string.
    ///
    /// @param value
    ///   Integer value (UInt8) to be converted.
    /// @param flags
    ///   How to convert value to string.
    /// @param base
    ///   Radix base. Default is 10. Allowed values are 2..36.
    ///   Bases 8 and 16 do not add leading '0' and '0x' accordingly.
    ///   If necessary you should add it yourself.
    /// @return
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted string value.
    ///   - Otherwise, set errno to non-zero and return empty string.
    static string UInt8ToString(Uint8 value,
                                TNumToStringFlags flags = 0,
                                int               base  = 10);

    /// Convert UInt8 to string.
    ///
    /// @param out_str
    ///   Output string variable
    /// @param value
    ///   Integer value (UInt8) to be converted.
    /// @param flags
    ///   How to convert value to string.
    /// @param base
    ///   Radix base. Default is 10. Allowed values are 2..36.
    ///   Bases 8 and 16 do not add leading '0' and '0x' accordingly.
    ///   If necessary you should add it yourself.
    /// @note
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted string value in 'out_str'.
    ///   - Otherwise, set errno to non-zero, value of 'out_str' is undefined.
    static void UInt8ToString(string& out_str, Uint8 value,
                              TNumToStringFlags flags = 0,
                              int               base  = 10);

    /// Convert UInt8 to string using "software" qualifiers.
    /// 
    /// Result of conversion will be limited to max_digits digits so that e.g.
    /// 1024 will be converted to 1.02KB. Conversion will be made using
    /// rounding so that 1025 will be converted to 1.03KB. By default function
    /// uses 1000-based qualifiers (as in examples above) but with fDS_Binary
    /// flag it will use 1024-based qualifiers, e.g. 1100 will be converted to
    /// 1.07KiB. With fDS_ShortSuffix flag function will omit "B" in 1000-based
    /// and "iB" in 1024-based qualifiers. When the result of conversion doesn't
    /// need any qualifiers then the result of this function will be equivalent
    /// to result of UInt8ToString() above except if fDS_PutBSuffixToo flag
    /// is passed. In the latter case "B" will be added to the number.
    /// 
    /// Function will always try to use a maximum possible qualifier and
    /// a number with decimal point except if fDS_NoDecimalPoint flag is passed.
    /// In that case function will return only whole number and try to use a
    /// minimum possible qualifier (which makes difference only if
    /// max_digits > 3).
    ///
    /// @param value
    ///   Integer value (UInt8) to be converted.
    /// @param flags
    ///   How to convert value to string.
    /// @param max_digits
    ///   Maximum number of digits to use (cannot be less than 3)
    /// @return
    ///   - If invalid flags are passed, throw an exception.
    ///   - If conversion succeeds, return the converted value.
    static string UInt8ToString_DataSize(Uint8 value,
                                         TNumToStringFlags flags = 0,
                                         unsigned int max_digits = 3);

    /// Convert UInt8 to string using "software" qualifiers.
    /// 
    /// See notes and details of how function works in the comments to 
    /// UInt8ToString_DataSize() above.
    ///
    /// @param out_str
    ///   Output string variable
    /// @param value
    ///   Integer value (UInt8) to be converted.
    /// @param flags
    ///   How to convert value to string.
    /// @param max_digits
    ///   Maximum number of digits to use (cannot be less than 3)
    static void UInt8ToString_DataSize(string& out_str,
                                       Uint8 value,
                                       TNumToStringFlags flags = 0,
                                       unsigned int max_digits = 3);
    /// Convert double to string.
    ///
    /// @param value
    ///   Double value to be converted.
    /// @param precision
    ///   Precision value for conversion. If precision is more that maximum
    ///   for current platform, then it will be truncated to this maximum.
    ///   If it is negative, that double will be converted to number in
    ///   scientific notation.
    /// @param flags
    ///   How to convert value to string.
    ///   If double format flags are not specified, that next output format
    ///   will be used by default:
    ///     - fDoubleFixed,   if 'precision' >= 0.
    ///     - fDoubleGeneral, if 'precision' < 0.
    /// @return
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted string value.
    ///   - Otherwise, set errno to non-zero and return empty string.
    static string DoubleToString(double value, int precision = -1,
                                 TNumToStringFlags flags = 0);

    /// Convert double to string.
    ///
    /// @param out_str
    ///   Output string variable
    /// @param value
    ///   Double value to be converted.
    /// @param precision
    ///   Precision value for conversion. If precision is more that maximum
    ///   for current platform, then it will be truncated to this maximum.
    ///   If it is negative, that double will be converted to number in
    ///   scientific notation.
    /// @param flags
    ///   How to convert value to string.
    ///   If double format flags are not specified, that next output format
    ///   will be used by default:
    ///     - fDoubleFixed,   if 'precision' >= 0.
    ///     - fDoubleGeneral, if 'precision' < 0.
    /// @note
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted string value in 'out_str'.
    ///   - Otherwise, set errno to non-zero, value of 'out_str' is undefined.
    static void DoubleToString(string& out_str, double value,
                               int precision = -1,
                               TNumToStringFlags flags = 0);

    /// Convert double to string with specified precision and place the result
    /// in the specified buffer.
    ///
    /// @param value
    ///   Double value to be converted.
    /// @param precision
    ///   Precision value for conversion. If precision is more that maximum
    ///   for current platform, then it will be truncated to this maximum.
    /// @param buf
    ///   Put result of the conversion into this buffer.
    /// @param buf_size
    ///   Size of buffer, "buf".
    /// @param flags
    ///   How to convert value to string.
    ///   Default output format is fDoubleFixed.
    /// @return
    ///   - If conversion succeeds, set errno to zero and return the
    ///     number of bytes stored in "buf", not counting the
    ///     terminating '\0'.
    ///   - Otherwise, set errno to non-zero, value of 'out_str' is undefined.
    static SIZE_TYPE DoubleToString(double value, unsigned int precision,
                                    char* buf, SIZE_TYPE buf_size,
                                    TNumToStringFlags flags = 0);

    /// Convert double to string with specified precision and put the result
    /// into a character buffer, in scientific format.
    ///
    /// NOTE:
    ///   The output character buffer is NOT zero-terminated.
    ///   The decimal separator is dot, always.
    ///   This function DOES NOT check 'value' for being finite or not-a-number;
    ///   if it is, the result is unpredictable.
    ///   This function is less precise for a small fraction of values
    ///   (the difference is in the last significant digit) than its
    ///   'DoubleToString' siblings, but it is much faster.
    ///
    /// @param value
    ///   Double value to be converted.
    /// @param precision
    ///   Maximum number of significant digits to preserve. If precision is greater than
    ///   maximum for the current platform, it will be truncated to this maximum.
    /// @param buf
    ///   Put result of the conversion into this buffer.
    /// @param buf_size
    ///   Size of buffer, "buf".
    /// @return
    ///   The number of bytes written into "buf".
    static SIZE_TYPE DoubleToStringPosix(double value,unsigned int precision,
                                         char* buf, SIZE_TYPE buf_size);


    /// Convert double to string with specified precision.
    /// 
    /// The result consists of three parts: significant digits, exponent and sign.
    /// For example, input value -12345.67 will produce
    /// buf = "1234567" , *dec = 4, and *sign = -1.
    /// NOTE:
    ///   The output character buffer is NOT zero-terminated.
    ///   The buffer is NOT padded with zeros.
    ///   This function DOES NOT check 'value' for being finite or not-a-number;
    ///   if it is, the result is unpredictable.
    ///   This function is less precise for a small fraction of values
    ///   (the difference is in the last significant digit) than its
    ///   'DoubleToString' siblings, but it is much faster.
    ///
    /// @param value
    ///   Double value to be converted.
    /// @param precision
    ///   Maximum number of significant digits to preserve. If precision is greater than
    ///   maximum for the current platform, it will be truncated to this maximum.
    /// @param buf
    ///   Put result of the conversion into this buffer.
    /// @param buf_size
    ///   Size of buffer, "buf".
    /// @param dec
    ///   Exponent
    /// @param sign
    ///   Sign of the value
    /// @return
    ///   The number of bytes written into "buf".
    static SIZE_TYPE DoubleToString_Ecvt(double value,unsigned int precision,
                                         char* buf, SIZE_TYPE buf_size,
                                         int* dec, int* sign);

    /// Convert size_t to string.
    ///
    /// @param value
    ///   Value to be converted.
    /// @param flags
    ///   How to convert value to string.
    /// @param base
    ///   Radix base. Default is 10. Allowed values are 2..36.
    ///   Bases 8 and 16 do not add leading '0' and '0x' accordingly.
    ///   If necessary you should add it yourself.
    /// @return
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted string value.
    ///   - Otherwise, set errno to non-zero and return empty string.
    static string SizetToString(size_t value,
                                TNumToStringFlags flags = 0,
                                int               base  = 10);

    /// Convert pointer to string.
    ///
    /// @param out_str
    ///   Output string variable.
    /// @param str
    ///   Pointer to be converted.
    /// @note
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted string value in 'out_str'.
    ///   - Otherwise, set errno to non-zero, value of 'out_str' is undefined.
    static void PtrToString(string& out_str, const void* ptr);

    /// Convert pointer to string.
    ///
    /// @param str
    ///   Pointer to be converted.
    /// @return
    ///   - If conversion succeeds, set errno to zero and return the
    ///     converted string value representing the pointer.
    ///   - Otherwise, set errno to non-zero and return empty string.
    static string PtrToString(const void* ptr);

    /// Convert bool to string.
    ///
    /// @param value
    ///   Boolean value to be converted.
    /// @return
    ///   One of: 'true, 'false'
    /// @note
    ///   Don't change errno.
    static const string BoolToString(bool value);

    /// Convert string to bool.
    ///
    /// @param str
    ///   Boolean string value to be converted.  Can recognize
    ///   case-insensitive version as one of:  
    ///   TRUE  - 'true, 't', 'yes', 'y', '1';
    ///   FALSE - 'false', 'f', 'no', 'n', '0'.
    /// @return
    ///   - If conversion succeeds, set errno to zero and return TRUE or FALSE.
    ///   - Otherwise, set errno to non-zero and throw an exception.
    static bool StringToBool(const CTempString str);


    /// Handle an arbitrary printf-style format string.
    ///
    /// This method exists only to support third-party code that insists on
    /// representing messages in this format; please stick to type-checked
    /// means of formatting such as the above ToString methods and I/O
    /// streams whenever possible.
    static string FormatVarargs(const char* format, va_list args);


    /// Which type of string comparison.
    enum ECase {
        eCase,      ///< Case sensitive compare
        eNocase     ///< Case insensitive compare
    };

    // ATTENTION.  Be aware that:
    //
    // 1) "Compare***(..., SIZE_TYPE pos, SIZE_TYPE n, ...)" functions
    //    follow the ANSI C++ comparison rules a la "basic_string::compare()":
    //       s1[pos:pos+n) == s2   --> return 0
    //       s1[pos:pos+n) <  s2   --> return negative value
    //       s1[pos:pos+n) >  s2   --> return positive value
    //
    // 2) "strn[case]cmp()" functions follow the ANSI C comparison rules:
    //       s1[0:n) == s2[0:n)   --> return 0
    //       s1[0:n) <  s2[0:n)   --> return negative value
    //       s1[0:n) >  s2[0:n)   --> return positive value


    /// Case-sensitive compare of a substring with another string.
    ///
    /// @param s1
    ///   String containing the substring to be compared.
    /// @param pos
    ///   Start position of substring to be compared.
    /// @param n
    ///   Number of characters in substring to be compared.
    /// @param s2
    ///   String (char*) to be compared with substring.
    /// @return
    ///   - 0, if s1[pos:pos+n) == s2;
    ///   - Negative integer, if s1[pos:pos+n) <  s2;
    ///   - Positive integer, if s1[pos:pos+n) >  s2.
    /// @sa
    ///   Other forms of overloaded CompareCase() with differences in argument
    ///   types: char* vs. CTempString[Ex]
    static int CompareCase(const CTempString s1, SIZE_TYPE pos, SIZE_TYPE n,
                           const char* s2);

    /// Case-sensitive compare of a substring with another string.
    ///
    /// @param s1
    ///   String containing the substring to be compared.
    /// @param pos
    ///   Start position of substring to be compared.
    /// @param n
    ///   Number of characters in substring to be compared.
    /// @param s2
    ///   String to be compared with substring.
    /// @return
    ///   - 0, if s1[pos:pos+n) == s2;
    ///   - Negative integer, if s1[pos:pos+n) <  s2;
    ///   - Positive integer, if s1[pos:pos+n) >  s2.
    /// @sa
    ///   Other forms of overloaded CompareCase() with differences in argument
    ///   types: char* vs. CTempString[Ex]
    static int CompareCase(const CTempString s1, SIZE_TYPE pos, SIZE_TYPE n,
                           const CTempString s2);

    /// Case-sensitive compare of two strings -- char* version.
    ///
    /// @param s1
    ///   String to be compared -- operand 1.
    /// @param s2
    ///   String to be compared -- operand 2.
    /// @return
    ///   - 0, if s1 == s2;
    ///   - Negative integer, if s1 < s2;
    ///   - Positive integer, if s1 > s2.
    /// @sa
    ///   CompareNocase(), Compare() versions with same argument types.
    static int CompareCase(const char* s1, const char* s2);

    /// Case-sensitive compare of two strings -- CTempStringEx version.
    ///
    /// @param s1
    ///   String to be compared -- operand 1.
    /// @param s2
    ///   String to be compared -- operand 2.
    /// @return
    ///   - 0, if s1 == s2;
    ///   - Negative integer, if s1 < s2;
    ///   - Positive integer, if s1 > s2.
    /// @sa
    ///   CompareNocase(), Compare() versions with same argument types.
    static int CompareCase(const CTempStringEx s1, const CTempStringEx s2);

    /// Case-insensitive compare of a substring with another string.
    ///
    /// @param s1
    ///   String containing the substring to be compared.
    /// @param pos
    ///   Start position of substring to be compared.
    /// @param n
    ///   Number of characters in substring to be compared.
    /// @param s2
    ///   String (char*) to be compared with substring.
    /// @return
    ///   - 0, if s1[pos:pos+n) == s2 (case-insensitive compare);
    ///   - Negative integer, if s1[pos:pos+n) < s2 (case-insensitive compare);
    ///   - Positive integer, if s1[pos:pos+n) > s2 (case-insensitive compare).
    /// @sa
    ///   Other forms of overloaded CompareNocase() with differences in
    ///   argument types: char* vs. CTempString[Ex]
    static int CompareNocase(const CTempString s1, SIZE_TYPE pos, SIZE_TYPE n,
                             const char* s2);

    /// Case-insensitive compare of a substring with another string.
    ///
    /// @param s1
    ///   String containing the substring to be compared.
    /// @param pos
    ///   Start position of substring to be compared.
    /// @param n
    ///   Number of characters in substring to be compared.
    /// @param s2
    ///   String to be compared with substring.
    /// @return
    ///   - 0, if s1[pos:pos+n) == s2 (case-insensitive compare);
    ///   - Negative integer, if s1[pos:pos+n) < s2 (case-insensitive compare);
    ///   - Positive integer, if s1[pos:pos+n) > s2 (case-insensitive compare).
    /// @sa
    ///   Other forms of overloaded CompareNocase() with differences in
    ///   argument types: char* vs. CTempString[Ex]
    static int CompareNocase(const CTempString s1, SIZE_TYPE pos, SIZE_TYPE n,
                             const CTempString s2);

    /// Case-insensitive compare of two strings -- char* version.
    ///
    /// @param s1
    ///   String to be compared -- operand 1.
    /// @param s2
    ///   String to be compared -- operand 2.
    /// @return
    ///   - 0, if s1 == s2 (case-insensitive compare);
    ///   - Negative integer, if s1 < s2 (case-insensitive compare);
    ///   - Positive integer, if s1 > s2 (case-insensitive compare).
    /// @sa
    ///   CompareCase(), Compare() versions with same argument types.
    static int CompareNocase(const char* s1, const char* s2);

    /// Case-insensitive compare of two strings -- CTempStringEx version.
    ///
    /// @param s1
    ///   String to be compared -- operand 1.
    /// @param s2
    ///   String to be compared -- operand 2.
    /// @return
    ///   - 0, if s1 == s2 (case-insensitive compare);
    ///   - Negative integer, if s1 < s2 (case-insensitive compare);
    ///   - Positive integer, if s1 > s2 (case-insensitive compare).
    /// @sa
    ///   CompareCase(), Compare() versions with same argument types.
    static int CompareNocase(const CTempStringEx s1, const CTempStringEx s2);

    /// Compare of a substring with another string.
    ///
    /// @param s1
    ///   String containing the substring to be compared.
    /// @param pos
    ///   Start position of substring to be compared.
    /// @param n
    ///   Number of characters in substring to be compared.
    /// @param s2
    ///   String (char*) to be compared with substring.
    /// @param use_case
    ///   Whether to do a case sensitive compare(eCase -- default), or a
    ///   case-insensitive compare (eNocase).
    /// @return
    ///   - 0, if s1[pos:pos+n) == s2;
    ///   - Negative integer, if s1[pos:pos+n) < s2;
    ///   - Positive integer, if s1[pos:pos+n) > s2.
    /// @sa
    ///   Other forms of overloaded Compare() with differences in argument
    ///   types: char* vs. CTempString[Ex]
    static int Compare(const CTempString s1, SIZE_TYPE pos, SIZE_TYPE n,
                       const char* s2, ECase use_case = eCase);

    /// Compare of a substring with another string.
    ///
    /// @param s1
    ///   String containing the substring to be compared.
    /// @param pos
    ///   Start position of substring to be compared.
    /// @param n
    ///   Number of characters in substring to be compared.
    /// @param s2
    ///   String to be compared with substring.
    /// @param use_case
    ///   Whether to do a case sensitive compare(default is eCase), or a
    ///   case-insensitive compare (eNocase).
    /// @return
    ///   - 0, if s1[pos:pos+n) == s2;
    ///   - Negative integer, if s1pos:pos+n) < s2;
    ///   - Positive integer, if s1[pos:pos+n) > s2.
    /// @sa
    ///   Other forms of overloaded Compare() with differences in argument
    ///   types: char* vs. CTempString[Ex]
    static int Compare(const CTempString s1, SIZE_TYPE pos, SIZE_TYPE n,
                       const CTempString s2, ECase use_case = eCase);

    /// Compare two strings -- char* version.
    ///
    /// @param s1
    ///   String to be compared -- operand 1.
    /// @param s2
    ///   String to be compared -- operand 2.
    /// @param use_case
    ///   Whether to do a case sensitive compare(default is eCase), or a
    ///   case-insensitive compare (eNocase).
    /// @return
    ///   - 0, if s1 == s2.
    ///   - Negative integer, if s1 < s2.
    ///   - Positive integer, if s1 > s2.
    /// @sa
    ///   Other forms of overloaded Compare() with differences in argument
    ///   types: char* vs. CTempString[Ex]
    static int Compare(const char* s1, const char* s2,
                       ECase use_case = eCase);

    /// Compare two strings -- CTempStringEx version.
    ///
    /// @param s1
    ///   String to be compared -- operand 1.
    /// @param s2
    ///   String to be compared -- operand 2.
    /// @param use_case
    ///   Whether to do a case sensitive compare(default is eCase), or a
    ///   case-insensitive compare (eNocase).
    /// @return
    ///   - 0, if s1 == s2;
    ///   - Negative integer, if s1 < s2;
    ///   - Positive integer, if s1 > s2.
    /// @sa
    ///   Other forms of overloaded Compare() with differences in argument
    ///   types: char* vs. CTempString[Ex]
    static int Compare(const CTempStringEx s1, const CTempStringEx s2,
                       ECase use_case = eCase);

    /// Case-sensitive equality of a substring with another string.
    ///
    /// @param s1
    ///   String containing the substring to be compared.
    /// @param pos
    ///   Start position of substring to be compared.
    /// @param n
    ///   Number of characters in substring to be compared.
    /// @param s2
    ///   String (char*) to be compared with substring.
    /// @return
    ///   - true, if s1[pos:pos+n) equals s2;
    ///   - false, otherwise
    /// @sa
    ///   Other forms of overloaded EqualCase() with differences in argument
    ///   types: char* vs. CTempString[Ex]
    static bool EqualCase(const CTempString s1, SIZE_TYPE pos, SIZE_TYPE n,
                          const char* s2);

    /// Case-sensitive equality of a substring with another string.
    ///
    /// @param s1
    ///   String containing the substring to be compared.
    /// @param pos
    ///   Start position of substring to be compared.
    /// @param n
    ///   Number of characters in substring to be compared.
    /// @param s2
    ///   String to be compared with substring.
    /// @return
    ///   - true, if s1[pos:pos+n) equals s2;
    ///   - false, otherwise
    /// @sa
    ///   Other forms of overloaded EqualCase() with differences in argument
    ///   types: char* vs. CTempString[Ex]
    static bool EqualCase(const CTempString s1, SIZE_TYPE pos, SIZE_TYPE n,
                          const CTempString s2);

    /// Case-sensitive equality of two strings -- char* version.
    ///
    /// @param s1
    ///   String to be compared -- operand 1.
    /// @param s2
    ///   String to be compared -- operand 2.
    /// @return
    ///   - true, if s1 equals s2
    ///   - false, otherwise
    /// @sa
    ///   EqualCase(), Equal() versions with same argument types.
    static bool EqualCase(const char* s1, const char* s2);

    /// Case-sensitive equality of two strings.
    ///
    /// @param s1
    ///   String to be compared -- operand 1.
    /// @param s2
    ///   String to be compared -- operand 2.
    /// @return
    ///   - true, if s1 equals s2
    ///   - false, otherwise
    /// @sa
    ///   EqualCase(), Equal() versions with same argument types.
    static bool EqualCase(const CTempStringEx s1, const CTempStringEx s2);

    /// Case-insensitive equality of a substring with another string.
    ///
    /// @param s1
    ///   String containing the substring to be compared.
    /// @param pos
    ///   Start position of substring to be compared.
    /// @param n
    ///   Number of characters in substring to be compared.
    /// @param s2
    ///   String (char*) to be compared with substring.
    /// @return
    ///   - true, if s1[pos:pos+n) equals s2 (case-insensitive compare);
    ///   - false, otherwise.
    /// @sa
    ///   Other forms of overloaded EqualNocase() with differences in
    ///   argument types: char* vs. CTempString[Ex]
    static bool EqualNocase(const CTempString s1, SIZE_TYPE pos, SIZE_TYPE n,
                            const char* s2);

    /// Case-insensitive equality of a substring with another string.
    ///
    /// @param s1
    ///   String containing the substring to be compared.
    /// @param pos
    ///   Start position of substring to be compared.
    /// @param n
    ///   Number of characters in substring to be compared.
    /// @param s2
    ///   String to be compared with substring.
    /// @return
    ///   - true, if s1[pos:pos+n) equals s2 (case-insensitive compare);
    ///   - false, otherwise.
    /// @sa
    ///   Other forms of overloaded EqualNocase() with differences in
    ///   argument types: char* vs. CTempString[Ex]
    static bool EqualNocase(const CTempString s1, SIZE_TYPE pos, SIZE_TYPE n,
                            const CTempString s2);

    /// Case-insensitive equality of two strings -- char* version.
    ///
    /// @param s1
    ///   String to be compared -- operand 1.
    /// @param s2
    ///   String to be compared -- operand 2.
    /// @return
    ///   - true, if s1 equals s2 (case-insensitive compare);
    ///   - false, otherwise.
    /// @sa
    ///   EqualCase(), Equal() versions with same argument types.
    static bool EqualNocase(const char* s1, const char* s2);

    /// Case-insensitive equality of two strings.
    ///
    /// @param s1
    ///   String to be compared -- operand 1.
    /// @param s2
    ///   String to be compared -- operand 2.
    /// @return
    ///   - true, if s1 equals s2 (case-insensitive compare);
    ///   - false, otherwise.
    /// @sa
    ///   EqualCase(), Equal() versions with same argument types.
    static bool EqualNocase(const CTempStringEx s1, const CTempStringEx s2);

    /// Test for equality of a substring with another string.
    ///
    /// @param s1
    ///   String containing the substring to be compared.
    /// @param pos
    ///   Start position of substring to be compared.
    /// @param n
    ///   Number of characters in substring to be compared.
    /// @param s2
    ///   String (char*) to be compared with substring.
    /// @param use_case
    ///   Whether to do a case sensitive compare(eCase -- default), or a
    ///   case-insensitive compare (eNocase).
    /// @return
    ///   - true, if s1[pos:pos+n) equals s2;
    ///   - false, otherwise.
    /// @sa
    ///   Other forms of overloaded Equal() with differences in argument
    ///   types: char* vs. CTempString[Ex]
    static bool Equal(const CTempString s1, SIZE_TYPE pos, SIZE_TYPE n,
                      const char* s2, ECase use_case = eCase);

    /// Test for equality of a substring with another string.
    ///
    /// @param s1
    ///   String containing the substring to be compared.
    /// @param pos
    ///   Start position of substring to be compared.
    /// @param n
    ///   Number of characters in substring to be compared.
    /// @param s2
    ///   String to be compared with substring.
    /// @param use_case
    ///   Whether to do a case sensitive compare (default is eCase), or a
    ///   case-insensitive compare (eNocase).
    /// @return
    ///   - 0, if s1[pos:pos+n) == s2;
    ///   - Negative integer, if s1[pos:pos+n) < s2;
    ///   - Positive integer, if s1[pos:pos+n) > s2.
    /// @sa
    ///   Other forms of overloaded Equal() with differences in argument
    ///   types: char* vs. CTempString[Ex]
    static bool Equal(const CTempString s1, SIZE_TYPE pos, SIZE_TYPE n,
                      const CTempString s2, ECase use_case = eCase);

    /// Test for equality of two strings -- char* version.
    ///
    /// @param s1
    ///   String to be compared -- operand 1.
    /// @param s2
    ///   String to be compared -- operand 2.
    /// @param use_case
    ///   Whether to do a case sensitive compare (default is eCase), or a
    ///   case-insensitive compare (eNocase).
    /// @return
    ///   - 0, if s1 == s2;
    ///   - Negative integer, if s1 < s2;
    ///   - Positive integer, if s1 > s2.
    /// @sa
    ///   EqualNocase(), Equal() versions with similar argument types.
    static bool Equal(const char* s1, const char* s2,
                      ECase use_case = eCase);

    /// Test for equality of two strings.
    ///
    /// @param s1
    ///   String to be compared -- operand 1.
    /// @param s2
    ///   String to be compared -- operand 2.
    /// @param use_case
    ///   Whether to do a case sensitive compare (default is eCase), or a
    ///   case-insensitive compare (eNocase).
    /// @return
    ///   - true, if s1 equals s2;
    ///   - false, otherwise.
    /// @sa
    ///   EqualNocase(), Equal() versions with similar argument types.
    static bool Equal(const CTempStringEx s1, const CTempStringEx s2,
                      ECase use_case = eCase);

    // NOTE.  On some platforms, "strn[case]cmp()" can work faster than their
    //        "Compare***()" counterparts.

    /// String compare.
    ///
    /// @param s1
    ///   String to be compared -- operand 1.
    /// @param s2
    ///   String to be compared -- operand 2.
    /// @return
    ///   - 0, if s1 == s2;
    ///   - Negative integer, if s1 < s2;
    ///   - Positive integer, if s1 > s2.
    /// @sa
    ///   strncmp(), strcasecmp(), strncasecmp()
    static int strcmp(const char* s1, const char* s2);

    /// String compare up to specified number of characters.
    ///
    /// @param s1
    ///   String to be compared -- operand 1.
    /// @param s2
    ///   String to be compared -- operand 2.
    /// @param n
    ///   Number of characters in string 
    /// @return
    ///   - 0, if s1 == s2;
    ///   - Negative integer, if s1 < s2;
    ///   - Positive integer, if s1 > s2.
    /// @sa
    ///   strcmp(), strcasecmp(), strncasecmp()
    static int strncmp(const char* s1, const char* s2, size_t n);

    /// Case-insensitive comparison of two zero-terminated strings.
    ///
    /// @param s1
    ///   String to be compared -- operand 1.
    /// @param s2
    ///   String to be compared -- operand 2.
    /// @return
    ///   - 0, if s1 == s2;
    ///   - Negative integer, if s1 < s2;
    ///   - Positive integer, if s1 > s2.
    /// @sa
    ///   strcmp(), strncmp(), strncasecmp()
    static int strcasecmp(const char* s1, const char* s2);

    /// Case-insensitive comparison of two zero-terminated strings,
    /// narrowed to the specified number of characters.
    ///
    /// @param s1
    ///   String to be compared -- operand 1.
    /// @param s2
    ///   String to be compared -- operand 2.
    /// @return
    ///   - 0, if s1 == s2;
    ///   - Negative integer, if s1 < s2.
    ///   - Positive integer, if s1 > s2.
    /// @sa
    ///   strcmp(), strcasecmp(), strcasecmp()
    static int strncasecmp(const char* s1, const char* s2, size_t n);

    /// Wrapper for the function strftime() that corrects handling %D and %T
    /// time formats on MS Windows.
    static size_t strftime(char* s, size_t maxsize, const char* format,
                           const struct tm* timeptr);

    /// Match "str" against the "mask".
    ///
    /// This function does not use regular expressions.
    /// Very similar to fnmatch(3), but there are differences (see also glob(7)).
    /// There's no special treatment for a slash character '/' in this call.
    ///
    /// @param str
    ///   String to match.
    /// @param mask
    ///   Mask used to match string "str".
    ///   This is a text pattern, which, along ordinary characters that must match
    ///   literally corresponding symbols in the string "str", can contains also
    ///   mext wildcard characters: \n
    ///     ? - matches to any single character in the string. \n
    ///     * - matches to any number of characters in the string (including none). \n
    ///
    ///   Mask also support POSIX character classes in the forms of "[...]" or "[!...]"
    ///   that must MATCH or NOT MATCH, respectively, a single character in "str".
    ///   To cancel the special meaning of '*', '?' or '[', they can be prepended with
    ///   a backslash '\\' (the backslash in front of other characters does not change
    ///   their meaning, so "\\\\" matches one graphical backslash in the "str").
    ///   Within a character class, to have its literal meaning a closing square bracket ']'
    ///   must be used at the first position, whereas '?', '*', '[, and '\\' stand
    ///   just for themselves. Two characters separated by a minus sign '-' denote
    ///   a range that can be used for contraction to include all characters in
    ///   between:  "[A-F]" is equivalent to "[ABCDEF]". 
    ///   For its literal meaning, the minus sign '-' can be used either at the very
    ///   first position, or the last position before the closing bracket ']'.
    ///   To have a range that begins with an exclamation point, one has to use 
    ///   a dummy empty range followed by that range with '!'.
    ///
    ///   Examples:
    ///     "!"        matches a single '!' (note that just "[!]" is invalid);
    ///     "[!!]"     matches any character, which is not an exclamation point '!';
    ///     "[][!]"    matches ']', '[', and '!';
    ///     "[!][-]"   matches any character except for ']', '[', and '-';
    ///     "[-]"      matches a minus sign '-' (same as '-' just by itself);
    ///     "[?*\\]"   matches either '?', or '*', or a backslash '\\';
    ///     "[]-\\]"   matches nothing as it defines an empty range (from ']' to '\\');
    ///     "\\[a]\\*" matches a literal substring "[a]*";
    ///     "[![a-]"   matches any char but '[', 'a' or '-' (same as "[!-[a]"; but not
    ///                "[![-a]", which defines an empty range, thus matches any char!);
    ///     "[]A]"     matches either ']' or 'A' (NB: "[A]]" matches a substring "A]");
    ///     "[0-9-]"   matches any decimal digit or a minus sign '-' (same: "[-0-9]");
    ///     "[9-0!-$]" matches '!', '"', '#', and '$' (as first range matches nothing).
    ///
    /// @note
    ///   In the above, each double backslash denotes a single graphical backslash
    ///   character (C string notation is used).
    /// @note
    ///   Unlike shell globbing, "[--0]" *does* match the slash character '/'
    ///  (along with '-', '.', and '0' that all fall within the range).
    /// @param use_case
    ///   Whether to do a case sensitive compare for letters (eCase -- default),
    ///   or a case-insensitive compare (eNocase).
    /// @return
    ///   Return TRUE if "str" matches "mask", and FALSE otherwise 
    ///  (including patter errors).
    /// @sa
    ///    CRegexp, CRegexpUtil
    ///
    static bool MatchesMask(CTempString str, CTempString mask, ECase use_case = eCase);

    /// Check if a string is blank (has no text).
    ///
    /// @param str
    ///   String to check.
    /// @param pos
    ///   starting position (default 0)
    static bool IsBlank(const CTempString str, SIZE_TYPE pos = 0);

    /// Checks if all letters in the given string have a lower case.
    ///
    /// @param str
    ///   String to be checked.
    /// @return
    ///   TRUE if all letter characters in the string are lowercase
    ///   according to the current C locale (std::islower()).
    ///   All non-letter characters will be ignored.
    ///   TRUE if empty or no letters.
    static bool IsLower(const CTempString str);

    /// Checks if all letters in the given string have a upper case.
    ///
    /// @param str
    ///   String to be checked.
    /// @return
    ///   TRUE if all letter characters in the string are uppercase
    ///   according to the current C locale (std::isupper()).
    ///   All non-letter characters will be skipped.
    ///   TRUE if empty or no letters.
    static bool IsUpper(const CTempString str);


    // The following 4 methods change the passed string, then return it

    /// Convert string to lower case -- string& version.
    /// 
    /// @param str
    ///   String to be converted.
    /// @return
    ///   Lower cased string.
    static string& ToLower(string& str);

    /// Convert string to lower case -- char* version.
    /// 
    /// @param str
    ///   String to be converted.
    /// @return
    ///   Lower cased string.
    static char* ToLower(char* str);

    /// Convert string to upper case -- string& version.
    /// 
    /// @param str
    ///   String to be converted.
    /// @return
    ///   Upper cased string.
    static string& ToUpper(string& str);

    /// Convert string to upper case -- char* version.
    /// 
    /// @param str
    ///   String to be converted.
    /// @return
    ///   Upper cased string.
    static char* ToUpper(char* str);

private:
    /// Privatized ToLower() with const char* parameter to prevent passing of 
    /// constant strings.
    static void/*dummy*/ ToLower(const char* /*dummy*/);

    /// Privatized ToUpper() with const char* parameter to prevent passing of 
    /// constant strings.
    static void/*dummy*/ ToUpper(const char* /*dummy*/);

public:

    /// Check if a string starts with a specified prefix value.
    ///
    /// @param str
    ///   String to check.
    /// @param start
    ///   Prefix value to check for.
    /// @param use_case
    ///   Whether to do a case sensitive compare(default is eCase), or a
    ///   case-insensitive compare (eNocase) while checking.
    static bool StartsWith(const CTempString str, const CTempString start,
                           ECase use_case = eCase);

    /// Check if a string starts with a specified character value.
    ///
    /// @param str
    ///   String to check.
    /// @param start
    ///   Character value to check for.
    /// @param use_case
    ///   Whether to do a case sensitive compare(default is eCase), or a
    ///   case-insensitive compare (eNocase) while checking.
    static bool StartsWith(const CTempString str, char start,
                           ECase use_case = eCase);

    /// Check if a string ends with a specified suffix value.
    ///
    /// @param str
    ///   String to check.
    /// @param end
    ///   Suffix value to check for.
    /// @param use_case
    ///   Whether to do a case sensitive compare(default is eCase), or a
    ///   case-insensitive compare (eNocase) while checking.
    static bool EndsWith(const CTempString str, const CTempString end,
                         ECase use_case = eCase);

    /// Check if a string ends with a specified character value.
    ///
    /// @param str
    ///   String to check.
    /// @param end
    ///   Character value to check for.
    /// @param use_case
    ///   Whether to do a case sensitive compare(default is eCase), or a
    ///   case-insensitive compare (eNocase) while checking.
    static bool EndsWith(const CTempString str, char end,
                         ECase use_case = eCase);

    /// Determine the common prefix of two strings.
    ///
    /// @param s1
    ///   String to be compared -- operand 1.
    /// @param s2
    ///   String to be compared -- operand 2.
    /// @return
    ///   The number of characters common to the start of each string.
    static SIZE_TYPE CommonPrefixSize(const CTempString s1, const CTempString s2);

    /// Determine the common suffix of two strings.
    ///
    /// @param s1
    ///   String to be compared -- operand 1.
    /// @param s2
    ///   String to be compared -- operand 2.
    /// @return
    ///   The number of characters common to the end of each string.
    static SIZE_TYPE CommonSuffixSize(const CTempString s1, const CTempString s2);

    /// Determine if the suffix of one string is the prefix of another.
    ///
    /// @param s1
    ///   String to be compared -- operand 1.
    /// @param s2
    ///   String to be compared -- operand 2.
    /// @return
    ///   The number of characters common to the end of the first string
    ///   and the start of the second string.
    static SIZE_TYPE CommonOverlapSize(const CTempString s1, const CTempString s2);


    /// Whether it is the first or last occurrence.
    /// @deprecated
    enum EOccurrence {
        eFirst,             ///< First occurrence
        eLast               ///< Last occurrence
    };

    /// Search direction for Find() methods.
    enum EDirection {
        eForwardSearch = 0,  ///< Search in a forward direction
        eReverseSearch       ///< Search in a backward direction
    };


    /// Find the pattern in the string.
    ///
    /// @param str
    ///   String to search.
    /// @param pattern
    ///   Pattern to search for in "str". 
    /// @param use_case
    ///   Whether to do a case sensitive compare (default is eCase), or a
    ///   case-insensitive compare (eNocase) while searching for the pattern.
    /// @param direction
    ///   Define a search direction of the requested "occurrence"
    ///   of "pattern" in "str".
    /// @param occurrence
    ///   Which occurrence of the pattern in the string to use (zero-based).
    ///   NOTE:  When an occurrence is found the next occurrence will be
    ///          searched for starting right *after* the found pattern.
    /// @return
    ///   Start of the found pattern in the string.
    ///   Or NPOS if there is no occurrence of the pattern in the string.
    static SIZE_TYPE Find(const CTempString str,
                          const CTempString pattern,
                          ECase             use_case   = eCase,
                          EDirection        direction  = eForwardSearch,
                          SIZE_TYPE         occurrence = 0);

    /// Find the pattern in the specified range of a string.
    ///
    /// @param str
    ///   String to search.
    /// @param pattern
    ///   Pattern to search for in "str". 
    /// @param start
    ///   Position in "str" to start search from.
    ///   0 means start the search from the beginning of the string.
    /// @param end
    ///   Position in "str" to perform search up to.
    ///   NPOS means to search to the end of the string.
    /// @param which
    ///   When set to eFirst, this means to find the first occurrence of 
    ///   "pattern" in "str". When set to eLast, this means to find the last
    ///    occurrence of "pattern" in "str".
    /// @param use_case
    ///   Whether to do a case sensitive compare (default is eCase), or a
    ///   case-insensitive compare (eNocase) while searching for the pattern.
    /// @return
    ///   - The start of the first or last (depending on "which" parameter)
    ///     occurrence of "pattern" in "str", within the string interval
    ///     ["start", "end"], or
    ///   - NPOS if there is no occurrence of the pattern.
    /// @sa FindCase, FindNoCase, FindWord
    ///
    /// @deprecated
    ///   Use
    ///   @code
    ///       Find(str, pattern, [use_case], [direction], [occurrence])
    ///   @endcode
    ///   method instead.
    ///   For example:
    ///   @code
    ///       Find(str, pattern, 0, NPOS, eLast, eCase)
    ///   @endcode
    ///   can be replaced by 
    ///   @code
    ///       Find(str, pattern, eCase, eReverseSearch, /* 0 */)
    ///   @endcode
    ///   If you doing a search on a substring of the 'str' and ["start", "end"] search
    ///   interval is not a default [0, NPOS], that mean a whole 'str' string, you may
    ///   need to pass a substring instead of 'str', like 
    ///   @code
    ///       Find(CTempString(str, start, len), pattern, ....)
    ///   @endcode
    ///  and after checking search result on NPOS, adjust it by 'start' yourself.
    NCBI_DEPRECATED
    static SIZE_TYPE Find(const CTempString str,
                          const CTempString pattern,
                          SIZE_TYPE   start, SIZE_TYPE end,
                          EOccurrence which = eFirst,
                          ECase       use_case = eCase);

    /// Wrapper for backward-compatibility
    inline
    static SIZE_TYPE Find(const CTempString str, const CTempString pattern, SIZE_TYPE start)
        { return FindCase(str, pattern, start); }


    /// Find the pattern in the specified range of a string using a case
    /// sensitive search.
    ///
    /// @param str
    ///   String to search.
    /// @param pattern
    ///   Pattern to search for in "str". 
    /// @param start
    ///   Position in "str" to start search from -- default of 0 means start
    ///   the search from the beginning of the string.
    /// @param end
    ///   Position in "str" to perform search up to -- default of NPOS means
    ///   to search to the end of the string.
    /// @param which
    ///   When set to eFirst, this means to find the first occurrence of 
    ///   "pattern" in "str". When set to eLast, this means to find the last
    ///    occurrence of "pattern" in "str".
    /// @return
    ///   - The start of the first or last (depending on "which" parameter)
    ///     occurrence of "pattern" in "str", within the string interval
    ///     ["start", "end"], or
    ///   - NPOS if there is no occurrence of the pattern.
    /// @sa Find
    ///
    /// @deprecated
    ///   Use Find() method without [start:end] range.
    /// @deprecated
    ///   Use one of the next methods instead:
    ///   @code
    ///       Find(str, pattern, [use_case], [direction], [occurrence])
    ///       FindCase(str, pattern, [start])
    ///   @endcode
    ///   For example:
    ///   @code
    ///       FindCase(str, pattern, 0, NPOS, eLast)
    ///   @endcode
    ///   can be replaced by 
    ///   @code
    ///       Find(str, pattern, eCase, eReverseSearch, /* 0 */)
    ///   @endcode
    ///   For simpler cases without range, or with default [0, NPOS] please use 
    ///   @code
    ///       FindCase(str, pattern, [start])
    ///   @endcode
    ///   But if you doing a search on a substring of the 'str' and ["start", "end"] search
    ///   interval is not a default [0, NPOS], that mean a whole 'str' string, you may
    ///   need to pass a substring instead of 'str', like 
    ///   @code
    ///       FindCase(CTempString(str, start, len), pattern, ....)
    ///   @endcode
    ///  and after checking search result on NPOS, adjust it by 'start' yourself.
    NCBI_DEPRECATED
    static SIZE_TYPE FindCase(const CTempString str, 
                              const CTempString pattern,
                              SIZE_TYPE   start, SIZE_TYPE end,
                              EOccurrence which = eFirst);

    /// Wrappers for backward-compatibility
    static SIZE_TYPE FindCase(const CTempString str, const CTempString pattern);
    static SIZE_TYPE FindCase(const CTempString str, const CTempString pattern, SIZE_TYPE start);

    /// Find the pattern in the specified range of a string using a case
    /// insensitive search.
    ///
    /// @param str
    ///   String to search.
    /// @param pattern
    ///   Pattern to search for in "str". 
    /// @param start
    ///   Position in "str" to start search from -- default of 0 means start
    ///   the search from the beginning of the string.
    /// @param end
    ///   Position in "str" to perform search up to -- default of NPOS means
    ///   to search to the end of the string.
    /// @param which
    ///   When set to eFirst, this means to find the first occurrence of 
    ///   "pattern" in "str". When set to eLast, this means to find the last
    ///    occurrence of "pattern" in "str".
    /// @return
    ///   - The start of the first or last (depending on "which" parameter)
    ///     occurrence of "pattern" in "str", within the string interval
    ///     ["start", "end"], or
    ///   - NPOS if there is no occurrence of the pattern.
    /// @sa Find
    ///
    /// @deprecated
    ///   Use one of the next methods instead:
    ///   @code
    ///       Find(str, pattern, [use_case], [direction], [occurrence])
    ///       FindNoCase(str, pattern, [start])
    ///   @endcode
    ///   For example:
    ///   @code
    ///       FindNoCase(str, pattern, 0, NPOS, eLast)
    ///   @endcode
    ///   can be replaced by 
    ///   @code
    ///       Find(str, pattern, eNocase, eReverseSearch, /* 0 */)
    ///   @endcode
    ///   For simpler cases without range, or with default [0, NPOS] please use 
    ///   @code
    ///       FindNoCase(str, pattern, [start])
    ///   @endcode
    ///   But if you doing a search on a substring of the 'str' and ["start", "end"] search
    ///   interval is not a default [0, NPOS], that mean a whole 'str' string, you may
    ///   need to pass a substring instead of 'str', like 
    ///   @code
    ///       FindNoCase(CTempString(str, start, len), pattern, ....)
    ///   @endcode
    ///  and after checking search result on NPOS, adjust it by 'start' yourself.
    NCBI_DEPRECATED
    static SIZE_TYPE FindNoCase(const CTempString str,
                                const CTempString pattern,
                                SIZE_TYPE   start, SIZE_TYPE end,
                                EOccurrence which = eFirst);

    /// Wrapper for backward-compatibility
    static SIZE_TYPE FindNoCase(const CTempString str, const CTempString pattern);
    static SIZE_TYPE FindNoCase(const CTempString str, const CTempString pattern, SIZE_TYPE start);

    /// Test for presence of a given string in a list or vector of strings

    static const string* Find      (const list<string>& lst,
                                    const CTempString val,
                                    ECase use_case = eCase);

    static const string* FindCase  (const list<string>& lst,
                                    const CTempString val);

    static const string* FindNoCase(const list<string>& lst, 
                                    const CTempString val);

    static const string* Find      (const vector<string>& vec, 
                                    const CTempString val,
                                    ECase use_case = eCase);

    static const string* FindCase  (const vector<string>& vec,
                                    const CTempString val);

    static const string* FindNoCase(const vector<string>& vec,
                                    const CTempString val);

    /// Find given word in the string.
    ///
    /// @param str
    ///   String to search.
    /// @param word
    ///   Word to search for in "str". The "word" can have any symbols,
    ///   not letters only. Function treat it as a pattern, even it have
    ///   any non-word characters.
    /// @param use_case
    ///   Whether to do a case sensitive compare (default is eCase), or a
    ///   case-insensitive compare (eNocase) while searching for the word.
    /// @param direction
    ///   Define a search direction of the occurrence of "word" in "str".
    /// @return
    ///   - Start of the found word in the string.
    ///   - NPOS if there is no occurrence of the word in the string.
    static SIZE_TYPE FindWord(const CTempString str,
                              const CTempString word,
                              ECase             use_case  = eCase,
                              EDirection        direction = eForwardSearch);

    /// Find given word in the string.
    ///
    /// This function honors word boundaries:
    ///   - starting or ending of the string,
    ///   - any non-word character, all except [a-zA-Z0-9_]. 
    ///
    /// @param str
    ///   String to search.
    /// @param word
    ///   Word to search for in "str". The "word" can have any symbols,
    ///   not letters only. Function treat it as a pattern, even it have
    ///   any non-word characters.
    /// @param which
    ///   When set to eFirst, this means to find the first occurrence of 
    ///   "word" in "str". When set to eLast, this means to find the last
    ///    occurrence of "word" in "str".
    /// @param use_case
    ///   Whether to do a case sensitive compare (default is eCase), or a
    ///   case-insensitive compare (eNocase) while searching for the word.
    /// @return
    ///   - The start of the first or last (depending on "which" parameter)
    ///     occurrence of "word" in "str", or
    ///   - NPOS if there is no occurrence of the word.
    /// @sa Find
    /// @deprecated
    ///   Use FindWord() variant with EDirection parameter:
    ///   @code
    ///       FindWord(str, word, [use_case], [direction])
    ///   @endcode
    inline
    NCBI_DEPRECATED
    static SIZE_TYPE FindWord(const CTempString str,
                              const CTempString word,
                              EOccurrence which,
                              ECase       use_case = eCase) {
        return FindWord(str, word, use_case, which == eFirst ? eForwardSearch : eReverseSearch);
    }


    /// Which end to truncate a string.
    enum ETrunc {
        eTrunc_Begin,  ///< Truncate leading spaces only
        eTrunc_End,    ///< Truncate trailing spaces only
        eTrunc_Both    ///< Truncate spaces at both begin and end of string
    };

    /// Truncate spaces in a string.
    ///
    /// @param str
    ///   String to truncate spaces from.
    /// @param where
    ///   Which end of the string to truncate space from. Default is to
    ///   truncate space from both ends (eTrunc_Both).
    /// @sa
    ///   TruncateSpaces_Unsafe
    static string TruncateSpaces(const string& str,
                                 ETrunc        where = eTrunc_Both);

    /// Truncate spaces in a string.
    /// It can be faster but it is also more dangerous than TruncateSpaces()
    ///
    /// @param str
    ///   String to truncate spaces from.
    /// @param where
    ///   Which end of the string to truncate space from. Default is to
    ///   truncate space from both ends (eTrunc_Both).
    /// @attention
    ///   The lifespan of the result string is the same as one of the source.
    ///   So, for example, if the source is temporary string, or it changes somehow,
    ///   then the result will be invalid right away (will point to already released
    ///   or wrong range in the memory).
    /// @sa
    ///   TruncateSpaces
    static CTempString TruncateSpaces_Unsafe(const CTempString str,
                                             ETrunc where = eTrunc_Both);

    /// @deprecated  Use TruncateSpaces_Unsafe() instead -- AND, do make sure
    ///              that you indeed use that in a safe manner!
    inline
    NCBI_DEPRECATED
    static CTempString TruncateSpaces(const CTempString str,
                                      ETrunc where = eTrunc_Both) {
        return TruncateSpaces_Unsafe(str, where);
    }

    /// @deprecated  Use TruncateSpaces_Unsafe() instead -- AND, do make sure
    ///              that you indeed use that in a safe manner!
    inline
    NCBI_DEPRECATED
    static CTempString TruncateSpaces(const char* str,
                                      ETrunc where = eTrunc_Both) {
        return TruncateSpaces_Unsafe(str, where);
    }

    /// Truncate spaces in a string (in-place)
    ///
    /// @param str
    ///   String to truncate spaces from.
    /// @param where
    ///   Which end of the string to truncate space from. Default is to
    ///   truncate space from both ends (eTrunc_Both).
    static void TruncateSpacesInPlace(string& str,  ETrunc where = eTrunc_Both);
    static void TruncateSpacesInPlace(CTempString&, ETrunc where = eTrunc_Both);
    

    /// Trim prefix from a string (in-place)
    ///
    /// @param str
    ///   String to trim from.
    /// @param prefix
    ///   Prefix to remove. 
    ///   If string doesn't have specified prefix, it doesn't changes.
    /// @param use_case
    ///   Whether to do a case sensitive compare (default is eCase), or a
    ///   case-insensitive compare (eNocase) while checking for a prefix.
    static void TrimPrefixInPlace(string& str, const CTempString prefix,
                                  ECase use_case = eCase);
    static void TrimPrefixInPlace(CTempString& str, const CTempString prefix,
                                  ECase use_case = eCase);

    /// Trim prefix from a string.
    ///
    /// "Unsafe" counterpart to TrimPrefixInPlace().
    /// @param str
    ///   String to trim from.
    /// @param prefix
    ///   Prefix to remove. 
    ///   If string doesn't have specified prefix, it doesn't changes.
    /// @param use_case
    ///   Whether to do a case sensitive compare (default is eCase), or a
    ///   case-insensitive compare (eNocase) while checking for a prefix.
    /// @attention
    ///   The lifespan of the result string is the same as one of the source.
    ///   So, for example, if the source is temporary string, or it changes somehow,
    ///   then the result will be invalid right away (will point to already released
    ///   or wrong range in the memory).
    /// @sa
    ///   TrimPrefixInPlace
    static CTempString TrimPrefix_Unsafe(const CTempString str,
                                         const CTempString prefix,
                                         ECase use_case = eCase);

    /// Trim suffix from a string (in-place)
    ///
    /// @param str
    ///   String to trim from.
    /// @param suffix
    ///   Suffix to remove. 
    ///   If string doesn't have specified suffix, it doesn't changes.
    /// @param use_case
    ///   Whether to do a case sensitive compare (default is eCase), or a
    ///   case-insensitive compare (eNocase) while checking for a suffix.
    static void TrimSuffixInPlace(string& str, const CTempString suffix,
                                  ECase use_case = eCase);
    static void TrimSuffixInPlace(CTempString& str, const CTempString suffix,
                                  ECase use_case = eCase);

    /// Trim suffix from a string.
    ///
    /// "Unsafe" counterpart to TrimSuffixInPlace().
    /// @param str
    ///   String to trim from.
    /// @param suffix
    ///   Suffix to remove. 
    ///   If string doesn't have specified suffix, it doesn't changes.
    /// @param use_case
    ///   Whether to do a case sensitive compare (default is eCase), or a
    ///   case-insensitive compare (eNocase) while checking for a suffix.
    /// @attention
    ///   The lifespan of the result string is the same as one of the source.
    ///   So, for example, if the source is temporary string, or it changes somehow,
    ///   then the result will be invalid right away (will point to already released
    ///   or wrong range in the memory).
    /// @sa
    ///   TrimSuffixInPlace
    static CTempString TrimSuffix_Unsafe(const CTempString str,
                                         const CTempString suffix,
                                         ECase use_case = eCase);

    /// Replace occurrences of a substring within a string.
    ///
    /// @param src
    ///   Source string from which specified substring occurrences are
    ///   replaced.
    /// @param search
    ///   Substring value in "src" that is replaced.
    /// @param replace
    ///   Replace "search" substring with this value.
    /// @param dst
    ///   Result of replacing the "search" string with "replace" in "src".
    ///   This value is also returned by the function.
    /// @param start_pos
    ///   Position to start search from.
    /// @param max_replace
    ///   Replace no more than "max_replace" occurrences of substring "search"
    ///   If "max_replace" is zero(default), then replace all occurrences with
    ///   "replace".
    /// @param num_replace
    ///   Optional pointer to a value which receives number of replacements occurred.
    /// @return
    ///   Result of replacing the "search" string with "replace" in "src". This
    ///   value is placed in "dst" as well.
    /// @sa
    ///   Version of Replace() that returns a new string.
    static string& Replace(const string& src,
                           const string& search,
                           const string& replace,
                           string&       dst,
                           SIZE_TYPE     start_pos = 0,
                           SIZE_TYPE     max_replace = 0,
                           SIZE_TYPE*    num_replace = 0);

    /// Replace occurrences of a substring within a string and returns the
    /// result as a new string.
    ///
    /// @param src
    ///   Source string from which specified substring occurrences are
    ///   replaced.
    /// @param search
    ///   Substring value in "src" that is replaced.
    /// @param replace
    ///   Replace "search" substring with this value.
    /// @param start_pos
    ///   Position to start search from.
    /// @param max_replace
    ///   Replace no more than "max_replace" occurrences of substring "search"
    ///   If "max_replace" is zero(default), then replace all occurrences with
    ///   "replace".
    /// @param num_replace
    ///   Optional pointer to a value which receives number of replacements occurred.
    /// @return
    ///   A new string containing the result of replacing the "search" string
    ///   with "replace" in "src"
    /// @sa
    ///   Version of Replace() that has a destination parameter to accept
    ///   result.
    static string Replace(const string& src,
                          const string& search,
                          const string& replace,
                          SIZE_TYPE     start_pos = 0,
                          SIZE_TYPE     max_replace = 0,
                          SIZE_TYPE*    num_replace = 0);
    
    /// Replace occurrences of a substring within a string.
    ///
    /// On some platforms this function is much faster than Replace()
    /// if sizes of "search" and "replace" strings are equal.
    /// Otherwise, the performance is mainly the same.
    /// @param src
    ///   String where the specified substring occurrences are replaced.
    ///   This value is also returned by the function.
    /// @param search
    ///   Substring value in "src" that is replaced.
    /// @param replace
    ///   Replace "search" substring with this value.
    /// @param start_pos
    ///   Position to start search from.
    /// @param max_replace
    ///   Replace no more than "max_replace" occurrences of substring "search"
    ///   If "max_replace" is zero(default), then replace all occurrences with
    ///   "replace".
    /// @param num_replace
    ///   Optional pointer to a value which receives number of replacements occurred.
    /// @return
    ///   Result of replacing the "search" string with "replace" in "src".
    /// @sa
    ///   Replace
    static string& ReplaceInPlace(string& src,
                                  const string& search,
                                  const string& replace,
                                  SIZE_TYPE     start_pos = 0,
                                  SIZE_TYPE     max_replace = 0,
                                  SIZE_TYPE*    num_replace = 0);

    /// Flags for Split*() methods. 
    /// 
    /// @note
    ///   With quote support enabled, doubling a quote character suppresses
    ///   its special meaning, as does escaping it if that's enabled too;
    ///   unescaped trailing backslashes and unbalanced quotes result in
    ///   exceptions.
    /// @note
    ///   All escape symbols, single or double quotes became removed
    ///   if a corresponding fSplit_Can* flag is used.
    enum ESplitFlags {
        fSplit_MergeDelimiters = 1 << 0,  ///< Merge adjacent delimiters
        fSplit_Truncate_Begin  = 1 << 1,  ///< Truncate leading delimiters
        fSplit_Truncate_End    = 1 << 2,  ///< Truncate trailing delimiters
        fSplit_Truncate        = fSplit_Truncate_Begin | fSplit_Truncate_End,
        fSplit_ByPattern       = 1 << 3,  ///< Require full delimiter strings
        fSplit_CanEscape       = 1 << 4,  ///< Allow \\... escaping
        fSplit_CanSingleQuote  = 1 << 5,  ///< Allow '...' quoting
        fSplit_CanDoubleQuote  = 1 << 6,  ///< Allow "..." quoting
        fSplit_CanQuote        = fSplit_CanSingleQuote | fSplit_CanDoubleQuote,
        /// All delimiters are merged and trimmed, to get non-empty tokens only
        fSplit_Tokenize        = fSplit_MergeDelimiters | fSplit_Truncate
    };
    typedef int TSplitFlags; ///< Bitwise OR of ESplitFlags

    /// Whether to merge adjacent delimiters.
    /// Used by some methods that don't need full functionality of ESplitFlags.
    enum EMergeDelims {
        eMergeDelims   = fSplit_MergeDelimiters | fSplit_Truncate,
        eNoMergeDelims = 0
    };

    /// Split a string using specified delimiters.
    ///
    /// @param str
    ///   String to be split.
    /// @param delim
    ///   Delimiter(s) used to split string "str". The interpretation of
    ///   multi-character values depends on flags: by default, any of those
    ///   characters marks a split point (when unquoted), but with
    ///   fSplit_ByPattern, the entire string must occur.  (Meanwhile,
    ///   an empty value disables splitting.)
    /// @param arr
    ///   The split tokens are added to the list "arr" and also returned
    ///   by the function.
    /// @param flags
    ///   Flags directing splitting, characterized under ESplitFlags.
    /// @param token_pos
    ///   Optional array for the tokens' positions in "str".
    /// @attention
    ///   Modifying source CTempString object or destroying it,
    ///   will invalidate results.
    /// @return 
    ///   The list "arr" is also returned.
    /// @sa
    ///   ESplitFlags, SplitInTwo, SplitByPattern
    static list<string>& Split( const CTempString    str,
                                const CTempString    delim,
                                list<string>&        arr,
                                TSplitFlags          flags = 0,
                                vector<SIZE_TYPE>*   token_pos = NULL);

    static vector<string>& Split(
                                const CTempString    str,
                                const CTempString    delim,
                                vector<string>&      arr,
                                TSplitFlags          flags = 0,
                                vector<SIZE_TYPE>*   token_pos = NULL);

    static list<CTempString>& Split(
                                const CTempString    str,
                                const CTempString    delim,
                                list<CTempString>&   arr,
                                TSplitFlags          flags = 0,
                                vector<SIZE_TYPE>*   token_pos = NULL,
                                CTempString_Storage* storage = NULL);

    static vector<CTempString>& Split(
                                const CTempString    str,
                                const CTempString    delim,
                                vector<CTempString>& arr,
                                TSplitFlags          flags = 0,
                                vector<SIZE_TYPE>*   token_pos = NULL,
                                CTempString_Storage* storage = NULL);

    static list<CTempStringEx>& Split(
                                const CTempString    str,
                                const CTempString    delim,
                                list<CTempStringEx>& arr,
                                TSplitFlags          flags = 0,
                                vector<SIZE_TYPE>*   token_pos = NULL,
                                CTempString_Storage* storage = NULL);

    static vector<CTempStringEx>& Split(
                                const CTempString      str,
                                const CTempString      delim,
                                vector<CTempStringEx>& arr,
                                TSplitFlags            flags = 0,
                                vector<SIZE_TYPE>*     token_pos = NULL,
                                CTempString_Storage*   storage = NULL);

    /// Split a string into two pieces using the specified delimiters
    ///
    /// @param str 
    ///   String to be split.
    /// @param delim
    ///   Delimiters used to split string "str".
    /// @param str1
    ///   The sub-string of "str" before the first character of "delim".
    ///   It will not contain any characters in "delim".
    ///   Will be empty if "str" begin with a delimiter.
    /// @param str2
    ///   The sub-string of "str" after the first character of "delim" found.
    ///   May contain "delim" characters.
    ///   Will be empty if "str" had no "delim" characters or ended
    ///   with the "delim" character.
    /// @param flags
    ///   Flags directing splitting, characterized under ESplitFlags.
    ///   Note, that fSplit_Truncate_End don't have any effect due nature
    ///   of this method.
    /// @attention
    ///   Modifying source CTempString object or destroying it,
    ///   will invalidate results.
    /// @return
    ///   true if a symbol from "delim" was found in "str", false if not.
    ///   This lets you distinguish when there were no delimiters and when
    ///   the very last character was the first delimiter.
    /// @sa
    ///   ESplitFlags, Split
    static bool SplitInTwo(const CTempString  str, 
                           const CTempString  delim,
                           string&            str1,
                           string&            str2,
                           TSplitFlags        flags = 0);

    static bool SplitInTwo(const CTempString  str, 
                           const CTempString  delim,
                           CTempString&       str1,
                           CTempString&       str2,
                           TSplitFlags        flags = 0,
                           CTempString_Storage* storage = NULL);

    static bool SplitInTwo(const CTempString  str, 
                           const CTempString  delim,
                           CTempStringEx&     str1,
                           CTempStringEx&     str2,
                           TSplitFlags        flags = 0,
                           CTempString_Storage* storage = NULL);


    /// Variation of Split() with fSplit_ByPattern flag applied by default

    static list<string>& SplitByPattern(
                                const CTempString    str,
                                const CTempString    delim,
                                list<string>&        arr,
                                TSplitFlags          flags = 0,
                                vector<SIZE_TYPE>*   token_pos = NULL);

    static vector<string>& SplitByPattern(
                                const CTempString    str,
                                const CTempString    delim,
                                vector<string>&      arr,
                                TSplitFlags          flags = 0,
                                vector<SIZE_TYPE>*   token_pos = NULL);

    static list<CTempString>& SplitByPattern(
                                const CTempString    str,
                                const CTempString    delim,
                                list<CTempString>&   arr,
                                TSplitFlags          flags = 0,
                                vector<SIZE_TYPE>*   token_pos = NULL,
                                CTempString_Storage* storage = NULL);

    static vector<CTempString>& SplitByPattern(
                                const CTempString    str,
                                const CTempString    delim,
                                vector<CTempString>& arr,
                                TSplitFlags          flags = 0,
                                vector<SIZE_TYPE>*   token_pos = NULL,
                                CTempString_Storage* storage = NULL);

    static list<CTempStringEx>& SplitByPattern(
                                const CTempString    str,
                                const CTempString    delim,
                                list<CTempStringEx>& arr,
                                TSplitFlags          flags = 0,
                                vector<SIZE_TYPE>*   token_pos = NULL,
                                CTempString_Storage* storage = NULL);

    static vector<CTempStringEx>& SplitByPattern(
                                const CTempString    str,
                                const CTempString    delim,
                                vector<CTempStringEx>& arr,
                                TSplitFlags          flags = 0,
                                vector<SIZE_TYPE>*   token_pos = NULL,
                                CTempString_Storage* storage = NULL);

    /// Join strings using the specified delimiter.
    ///
    /// @param arr
    ///   Array of strings to be joined.
    /// @param delim
    ///   Delimiter used to join the string.
    /// @return 
    ///   The strings in "arr" are joined into a single string, separated
    ///   with "delim".
    /// @sa Split
    template<typename TContainer>
    static string
    Join(const TContainer& arr, const CTempString& delim)
    {
        return x_Join(begin(arr), end(arr), delim);
    }
    template<typename TValue>
    static string
    Join(const initializer_list<TValue>& arr, const CTempString& delim)
    {
        return x_Join(begin(arr), end(arr), delim);
    }
    template<typename TInputIterator>
    static string
    Join( TInputIterator from, TInputIterator to, const CTempString& delim)
    {
        return x_Join(from, to, delim);
    }
    template<typename TInputIterator>
    static string
    JoinNumeric( TInputIterator from, TInputIterator to, const CTempString& delim)
    {
        return x_Join( from, to, delim);
    }
    template<typename TIterator, typename FTransform>
    static string
    TransformJoin( TIterator from, TIterator to, const CTempString& delim, FTransform fnTransform);


    /// How to display printable strings.
    ///
    /// Assists in making a printable version of "str".
    enum EPrintableMode {
        fNewLine_Quote     = 0,  ///< Display "\n" instead of actual linebreak
        eNewLine_Quote     = fNewLine_Quote,
        fNewLine_Passthru  = 1,  ///< Break the line at every "\n" occurrence
        eNewLine_Passthru  = fNewLine_Passthru,
        fNonAscii_Passthru = 0,  ///< Allow non-ASCII but printable characters
        fNonAscii_Quote    = 2,  ///< Octal for all non-ASCII characters
        fPrintable_Full    = 64  ///< Show all octal digits at all times
    };
    typedef int TPrintableMode;  ///< Bitwise OR of EPrintableMode flags

    /// Get a printable version of the specified string. 
    ///
    /// All non-printable characters will be represented as "\r", "\n", "\v",
    /// "\t", "\"", "\\\\", etc, or "\\ooo" where 'ooo' is an octal code of the
    /// character.  The resultant string is a well-formed C string literal,
    /// which, without alterations, can be compiled by a C/C++ compiler.
    /// In many instances, octal representations of non-printable characters
    /// can be reduced to take less than all 3 digits, if there is no
    /// ambiguity in the interpretation.  fPrintable_Full cancels the
    /// reduction, and forces to produce full 3-digit octal codes throughout.
    ///
    /// @param str
    ///   The string whose printable version is wanted.
    /// @param mode
    ///   How to display the string.  The default setting of fNewLine_Quote
    ///   displays the new lines as "\n", and uses the octal code reduction.
    ///   When set to fNewLine_Passthru, line breaks are actually
    ///   produced on output but preceded with trailing backslashes.
    /// @return
    ///   Return a printable version of "str".
    /// @sa
    ///   ParseEscapes, Escape, CEncode, CParse, Sanitize
    static string PrintableString(const CTempString str,
                                  TPrintableMode    mode = fNewLine_Quote | fNonAscii_Passthru);

    /// Escape string (generic version).
    ///
    /// Prefix any occurrences of the metacharacters with the escape character.
    /// @param str
    ///   The string to be escaped.
    /// @metacharacters
    ///   List of characters that need to be escaped.
    ///   Use NStr::Join() if you have metacharacters in list<>, vector<> or set<>. 
    /// @param escape_char
    ///   Character used for escaping metacharacters.
    ///   Each metacharacter will be replaced with pair "escape_char + metacharacter".
    ///   Each escape character will be replaced with pair "escape_char + escape_char".
    /// @return
    ///   Escaped string.
    /// @sa 
    ///   Unescape, PrintableString, Join
    static string Escape(const CTempString str, const CTempString metacharacters,
                         char escape_char = '\\');

    /// Unescape string (generic version).
    ///
    /// Remove escape characters added by Escape().
    /// @param str
    ///   The string to be processed.
    /// @param escape_char
    ///   Character used for escaping.
    /// @return
    ///   Unescaped string.
    /// @sa 
    ///   Escape
    static string Unescape(const CTempString str, char escape_char = '\\');


    /// Quote string (generic version).
    ///
    /// Prepend and append a specified quote character, but escaping any occurrence
    /// of the quote character using either a specified escape character (default '\')
    /// or as option, by doubling the quoting character if escape character is the same
    /// (e.g. like the single quote in SQL, double-quote in CSV).
    ///
    /// @param str
    ///   The string to be quoted.
    /// @param quote_char
    ///   Character used for quoting, default to double quote '"'.
    /// @param escape_char
    ///   Character used for escaping other quote characters inside string (default '\').
    ///   Each <quote_char>  in the string will be replaced with pair "escape_char + quote_char".
    ///   Each <escape_char> in the string will be replaced with pair "escape_char + escape_char".
    /// @return
    ///   Quoted string.
    /// @sa 
    ///   Unquote, ParseQuoted, CEncode
    static string Quote(const CTempString str, char quote_char = '"', char escape_char = '\\');

    /// Unquote string (generic version).
    ///
    /// Remove quotation added by Quote(). Uses first character as quoting character.
    /// @param str
    ///   The string to be processed.
    /// @param escape_char
    ///   Character used for escaping.
    /// @return
    ///   Unquoted string.
    /// @sa 
    ///   Quote, ParseQuoted, CEncode
    static string Unquote(const CTempString str, char escape_char = '\\');


    /// Flags for Sanitize().
    enum ESS_Flags {
        // Character filters
        fSS_alpha = 1 << 0,    ///< Check on ::isalpha()
        fSS_digit = 1 << 1,    ///< Check on ::isdigit()
        fSS_alnum = 1 << 2,    ///< Check on ::isalnum()
        fSS_print = 1 << 3,    ///< Check on ::isprint()
        fSS_cntrl = 1 << 4,    ///< Check on ::iscntrl()
        fSS_punct = 1 << 5,    ///< Check on ::ispunct()

        // Filter: in or out?
        fSS_Reject = 1 << 11,  ///< Reject specified characters, allow all other.
                               ///< Revert default behavior, that allow specified
                               ///< characters and reject all other.
        // Utility flags
        fSS_Remove           = 1 << 12,  ///< Remove (rather than replace) rejected chars
        fSS_NoMerge          = 1 << 13,  ///< Do not merge adjacent spaces (rejected chars)
        fSS_NoTruncate_Begin = 1 << 14,  ///< Do not truncate leading spaces
        fSS_NoTruncate_End   = 1 << 15,  ///< Do not truncate trailing spaces
        fSS_NoTruncate       = fSS_NoTruncate_Begin | fSS_NoTruncate_End
    };
    typedef int TSS_Flags;  ///< Bitwise OR of ESS_Flags
    
    /// Sanitize a string, allowing only specified classes of characters.
    ///
    /// By default:
    ///    - replace all non-printable characters with spaces;
    ///    - merge coalescent spaces;
    ///    - truncate leading and trailing spaces.
    /// @note
    ///   - All coalescent leading/trailing spaces also will be merged
    ///     by default if fSS_NoMerge has not specified.
    ///   - The truncation of leading/trailing spaces is doing after
    ///     allowing/rejecting characters. Depending on the specified flags,
    ///     all rejected characters adjacent to it can be treat as part
    ///     of leading/trailing spaces.
    /// @param str
    ///   String to sanitize
    /// @param flags
    ///   Alternative sanitation options
    /// @return
    ///   Sanitized string
    /// @sa
    ///   PrintableString
    static string Sanitize(CTempString str, TSS_Flags flags = fSS_print)
    {
        return Sanitize(str, CTempString(), CTempString(), ' ', flags);
    }


    /// Sanitize a string, allowing only specified characters or character classes.
    ///
    /// More customizable version of Sanitize():
    ///    - allow to specify custom sets of allowed and rejected characters, 
    ///      in addition to predefined classes if specified, see TSS_Flags;
    ///    - allow to specify replacement character for rejected symbols;
    /// By default:
    ///    - replace all rejected characters with <reject_replacement>;
    ///    - merge coalescent spaces and <reject_replacement>s (separately if differ);
    ///    - truncate leading and trailing spaces.
    /// Filters check order:
    ///    - character classes via flags. 
    ///      Note, that if no character classes are set, and no custom <allow_chars>
    ///      or <reject_chars>, fSS_print will be used;
    ///    - <allow_chars> if not empty, have priority over flags. 
    ///    - <reject_chars> if not empty, have priority over flags and <allow_chars> if have intersections.
    /// @note
    ///   - All coalescent leading/trailing spaces also will be merged
    ///     by default if fSS_NoMerge has not specified.
    ///   - The truncation of leading/trailing spaces is doing after
    ///     allowing/rejecting characters.
    /// @note
    ///   Spaces processes after checks on allowance, so if it isn't allowed
    ///   it will be threatened as regular rejected character.
    /// @param str
    ///   String to sanitize.
    /// @param allow_chars
    ///   Additional list of allowed characters, in addition to character classes in <flags>.
    ///   Have priority over character classes.
    ///   Use NStr::Join() if you have it in list<>, vector<> or set<>. 
    /// @param reject_chars
    ///   Additional list of rejected characters, in addition to character classes in <flags>.
    ///   Have priority over character classes and <allow_chars>.
    ///   Use NStr::Join() if you have it in list<>, vector<> or set<>. 
    /// @param reject_replacement
    ///   Replacement character for all rejected characters.
    /// @param flags
    ///   Alternative sanitation options.
    ///   If no custom <allow_chars> or <reject_chars>, and no character classes are set, then use fSS_print by default.
    ///   If <reject_chars>, no class, and no fSS_Reject flag, then all characters allowed except <reject_chars>.
    ///   If <allow_chars>,  no class, and fSS_Reject flag, then no any character allowed except <allow_chars>.
    /// @return
    ///   Sanitized string
    /// @sa
    ///   PrintableString, Join
    static string Sanitize(CTempString str,
                           CTempString allow_chars,
                           CTempString reject_chars,
                           char reject_replacement = ' ',
                           TSS_Flags flags = 0);

    /// C-style escape sequences parsing mode.
    /// For escape sequences with a value outside the range of [0-255] 
    /// the behavior of ParseEscapes() depends from this mode.
    /// By default all escape sequences within a out or range
    /// will be converted to the least significant byte, with no warning.
    enum EEscSeqRange {
        eEscSeqRange_Standard,   ///< Set char to the last (least significant
                                 ///< byte) of the escape sequence (default).
        eEscSeqRange_FirstByte,  ///< Set char to the first byte of the escape
                                 ///< sequence.
        eEscSeqRange_Throw,      ///< Throw an exception.
        eEscSeqRange_Errno,      ///< Set errno to ERANGE, return empty string.
        eEscSeqRange_User        ///< Set char to the user value
                                 ///< passed in another parameter.
    };

    /// Parse C-style escape sequences in the specified string.
    ///
    /// Parse escape sequences including all those produced by PrintableString.
    /// @param str
    ///   The string to be parsed.
    /// @param mode
    ///   Parsing mode.
    ///   By default all escape sequences with a value outside the range of [0-255]
    ///   will be converted to the least significant byte, with no warning.
    /// @param user_char
    ///   If 'mode' have eEscSeqRange_User, replace all out of range
    ///   escape sequences with this char.
    /// @return
    ///   String with parsed C-style escape sequences.
    ///   - If string have wrong format throw an CStringException exception.
    ///   - If parsing succeeds, return the converted value.
    ///     Set errno to zero only if eEscSeqRange_Errno is set.
    ///   - Otherwise, if escape sequence is out of range [0-255],
    ///     see eEscSeqRange* modes for behavior. 
    /// @sa 
    ///   EEscSeqFlags, PrintableString, CEncode, CParse
    static string ParseEscapes(const CTempString str, 
                               EEscSeqRange mode = eEscSeqRange_Standard,
                               char user_char = '?');

    /// Discard C-style backslash escapes and extract a quoted string.
    ///
    /// @param[in] str
    ///   The original string to extract a quoted string from.
    ///   It must start with a double quote.
    /// @param[out] n_read
    ///   How many symbols the quoted string occupied in the original string.
    /// @return
    ///   The extracted string, un-escaped and with the quotes removed.
    ///   Throw an exception on format error.
    static string ParseQuoted(const CTempString str, size_t* n_read = NULL);

    /// Define that string is quoted or not.
    enum EQuoted {
        eQuoted,       ///< String is quoted
        eNotQuoted     ///< String is not quoted
    };

    /// Encode a string for C/C++.
    ///
    /// @param str
    ///   The string to be parsed.
    /// @param quoted
    ///   Define, to 
    /// @sa
    ///   CParse, PrintableString
    static string CEncode(const CTempString str, EQuoted quoted = eQuoted);

    /// Discard C-style backslash escapes.
    ///
    /// @param str
    ///   The original string to parse.
    /// @param quoted
    ///   Define that parsing string is quoted or not.
    ///   If parameter "quoted" equal eQuoted and string is not started and
    ///   finished with a double-quote, the exception will be thrown,
    ///   otherwise quotes will be removed in result.
    /// @return
    ///   String with parsed C-style escape sequences.
    /// @sa
    ///   CEncode
    static string CParse(const CTempString str, EQuoted quoted = eQuoted);

    /// Encode a string for JavaScript.
    ///
    /// Replace relevant characters by predefined entities.
    /// Like to PrintableString(), but process some symbols in different way.
    /// @sa PrintableString
    static string JavaScriptEncode(const CTempString str);

    /// XML-encode flags
    enum EXmlEncode {
        /// Encode predefined entities only
        eXmlEnc_Contents = 0,
        /// Encode double hyphen and ending hyphen,
        /// making the result safe to put into XML comments.
        eXmlEnc_CommentSafe   = 1 << 0,
        /// Check each character to conform XML 1.1 standards,
        /// skip any not allowed character or throw an CStringException.
        /// https://www.w3.org/TR/xml11/#NT-Char
        eXmlEnc_Unsafe_Skip   = 1 << 1,
        eXmlEnc_Unsafe_Throw  = 1 << 2
    };
    typedef int TXmlEncode;   //<  bitwise OR of "EXmlEncode"

    /// Encode a string for XML.
    ///
    /// Replace relevant characters by predefined entities.
    static string XmlEncode(const CTempString str,
                            TXmlEncode flags = eXmlEnc_Contents);


    /// HTML-decode flags
    enum EHtmlEncode {
        fHtmlEnc_EncodeAll           = 0,       ///< Encode all symbols
        fHtmlEnc_SkipLiteralEntities = 1 << 1,  ///< Skip "&entity;"
        fHtmlEnc_SkipNumericEntities = 1 << 2,  ///< Skip "&#NNNN;"
        fHtmlEnc_SkipEntities        = fHtmlEnc_SkipLiteralEntities | fHtmlEnc_SkipNumericEntities,
        fHtmlEnc_CheckPreencoded     = 1 << 3   ///< Print warning if some pre-encoded
                                                ///< entity found in the string
    };
    typedef int THtmlEncode;   //<  bitwise OR of "EHtmlEncode"

    /// Encode a string for HTML.
    ///
    /// Replace relevant characters by predefined entities.
    /// @param str
    ///   Original string in UTF8 encoding.
    static string HtmlEncode(const CTempString str,
                             THtmlEncode flags = fHtmlEnc_EncodeAll);

    /// HTML-decode flags
    enum EHtmlDecode {
        fHtmlDec_CharRef_Entity   = 1,       ///< Character entity reference(s) was found
        fHtmlDec_CharRef_Numeric  = 1 << 1,  ///< Numeric character reference(s) was found
        fHtmlDec_Encoding_Changed = 1 << 2   ///< Character encoding changed
    };
    typedef int THtmlDecode;   //<  bitwise OR of "EHtmlDecode"

    /// Decode HTML entities and character references.
    ///
    /// @param str
    ///   String to be decoded, which contains characters or numeric HTML entities
    /// @param encoding
    ///   Encoding of the input string
    /// @return
    ///   UTF8 encoded string
    static string HtmlDecode(const CTempString str,
                             EEncoding encoding = eEncoding_Unknown,
                             THtmlDecode* result_flags = NULL);

    /// Returns HTML entity name for this symbol if one exists
    /// (without leading ampersand and trailing semicolon);
    /// or empty string if suitable HTML entity was not found
    static string HtmlEntity(TUnicodeSymbol uch);

    /// URL-encode flags
    enum EJsonEncode {
        eJsonEnc_UTF8,     ///< Encode all characters above 0x80 to \uXXXX form. 
                           ///< https://tools.ietf.org/html/rfc7159#section-8.1
        eJsonEnc_Quoted    ///< Quote resulting string. Keep all Unicode symbols ss is.
                           ///< https://tools.ietf.org/html/rfc7159#section-7
    };
    /// Encode a string for JSON.
    ///
    /// @param str
    ///   The string to encode
    /// @param encoding
    ///   Specifies how to encode string. There are 2 approaches, with representing whole
    ///   string as UTF-8 encoded string, or leave all Unicode symbols "as is", 
    ///   but the resulting string  will be put in double quotes.
    /// @return
    ///   JSON encoded string
    static string JsonEncode(const CTempString str, EJsonEncode encoding = eJsonEnc_UTF8);

    /// Quotes a string in Bourne Again Shell (BASH) syntax, in a way
    /// that disallows non-printable characters in the result.
    /// This function does NOT implement aesthetically optimal quoting,
    /// but does try to avoid redundant quoting in simpler cases.
    /// Also, since it implements BASH syntax, the result may be
    /// incompatible with Bourne syntax, and may be non-obvious to
    /// people who are not familiar with the extended quoting syntax.
    /// @note The BASH shell has extensions beyond Bourne Shell quoting.
    ///       Also, this is very different from C Shell quoting, and
    ///       MS Windows Command Prompt quoting rules.
    static string ShellEncode(const string& str);

    /// URL-encode flags
    enum EUrlEncode {
        eUrlEnc_SkipMarkChars,    ///< Do not convert chars like '!', '(' etc.
        eUrlEnc_ProcessMarkChars, ///< Convert all non-alphanumeric chars, spaces are converted to '+'
        eUrlEnc_PercentOnly,      ///< Convert all non-alphanumeric chars including space and '%' to %## format
        eUrlEnc_Path,             ///< Same as ProcessMarkChars but preserves valid path characters ('/', '.')
        eUrlEnc_URIScheme,        ///< Encode scheme part of an URI.
        eUrlEnc_URIUserinfo,      ///< Encode userinfo part of an URI.
        eUrlEnc_URIHost,          ///< Encode host part of an URI.
        eUrlEnc_URIPath,          ///< Encode path part of an URI.
        eUrlEnc_URIQueryName,     ///< Encode query part of an URI, arg name.
        eUrlEnc_URIQueryValue,    ///< Encode query part of an URI, arg value.
        eUrlEnc_URIFragment,      ///< Encode fragment part of an URI.
        eUrlEnc_Cookie,           ///< Same as SkipMarkChars with encoded ','
        eUrlEnc_None              ///< Do not encode
    };
    /// URL decode flags
    enum EUrlDecode {
        eUrlDec_All,              ///< Decode '+' to space
        eUrlDec_Percent           ///< Decode only %XX
    };
    /// URL-encode string
    static string URLEncode(const CTempString str,
                            EUrlEncode flag = eUrlEnc_SkipMarkChars);

    /// SQL encode flags
    enum ESqlEncode {
        eSqlEnc_Plain,       ///< Always produce '...', with no tag.
        eSqlEnc_TagNonASCII  ///< Produce N'...' when input's not pure ASCII.
    };
    /// SQL-encode string
    ///
    /// There are some assumptions/notes about the function:
    /// 1. Only for MS SQL and Sybase.
    /// 2. Only for string values in WHERE and LIKE clauses.
    /// 3. The ' symbol must not be used as an escape symbol in LIKE clause.
    /// 4. It must not be used for non-string values.
    /// 5. It expects a string without any outer quotes, and
    ///    it adds single quotes to the returned string.
    /// 6. It expects UTF-8 (including its subsets, ASCII and Latin1) or
    ///    Win1252 string, and the input encoding is preserved.
    /// @param str
    ///   The string to encode
    /// @param flag
    ///   Whether to tag the result with an N prefix if it contains any
    ///   non-ASCII characters.  Such tagging is generally advisable,
    ///   but off by default per historical practice, since there are
    ///   corner cases in which it may be inappropriate.
    /// @return
    ///   Encoded string with added outer single quotes
    static CStringUTF8 SQLEncode(const CStringUTF8& str, ESqlEncode flag);

    NCBI_DEPRECATED static CStringUTF8 SQLEncode(const CStringUTF8& str)
        { return SQLEncode(str, eSqlEnc_Plain); }

    /// URL-decode string
    static string URLDecode(const CTempString str, EUrlDecode flag = eUrlDec_All);
    /// URL-decode string to itself
    static void URLDecodeInPlace(string& str, EUrlDecode flag = eUrlDec_All);
    /// Check if the string needs the requested URL-encoding
    static bool NeedsURLEncoding(const CTempString str, EUrlEncode flag = eUrlEnc_SkipMarkChars);

    /// Check if the string contains a valid IP address
    static bool IsIPAddress(const CTempStringEx str);


    /// How to wrap the words in a string to a new line.
    enum EWrapFlags {
        fWrap_Hyphenate  = 0x1, ///< Add a hyphen when breaking words?
        fWrap_HTMLPre    = 0x2, ///< Wrap as pre-formatted HTML?
        fWrap_FlatFile   = 0x4  ///< Wrap for flat file use.
    };
    typedef int TWrapFlags;     ///< Bitwise OR of "EWrapFlags"

    /// Wrap the specified string into lines of a specified width.
    ///
    /// Split string "str" into lines of width "width" and add the
    /// resulting lines to the list "arr".  Normally, all
    /// lines will begin with "prefix" (counted against "width"),
    /// but the first line will instead begin with "prefix1" if
    /// you supply it.
    ///
    /// @param str
    ///   String to be split into wrapped lines.
    /// @param width
    ///   Width of each wrapped line.
    /// @param arr
    ///   List of strings containing wrapped lines.
    /// @param flags
    ///   How to wrap the words to a new line. See EWrapFlags documentation.
    /// @param prefix
    ///   The prefix string added to each wrapped line, except the first line,
    ///   unless "prefix1" is set.
    ///   If "prefix" is set to 0(default), do not add a prefix string to the
    ///   wrapped lines.
    /// @param prefix1
    ///   The prefix string for the first line. Use this for the first line
    ///   instead of "prefix".
    ///   If "prefix1" is set to 0(default), do not add a prefix string to the
    ///   first line.
    /// @return
    ///   Return "arr", the list of wrapped lines.
    template<typename _D>
    static void WrapIt(const string& str, SIZE_TYPE width,
        _D& dest, TWrapFlags flags = 0,
        const string* prefix = 0,
        const string* prefix1 = 0);

    class IWrapDest
    {
    public:
        virtual ~IWrapDest() {}
        virtual void Append(const string& s) = 0;
        virtual void Append(const CTempString& s) = 0;
    };

    class CWrapDestStringList : public IWrapDest
    {
    protected:
        list<string>& m_list;
    public:
        CWrapDestStringList(list<string>& l) : m_list(l) {};
        virtual void Append(const string& s)
        {
            m_list.push_back(s);
        }
        virtual void Append(const CTempString& s)
        {
            m_list.push_back(NcbiEmptyString);
            m_list.back().assign(s.data(), s.length());
        }
    };

    static void Wrap(const string& str, SIZE_TYPE width,
                              IWrapDest& dest, TWrapFlags flags,
                              const string* prefix,
                              const string* prefix1);

    static list<string>& Wrap(const string& str, SIZE_TYPE width,
                              list<string>& arr, TWrapFlags flags = 0,
                              const string* prefix = 0,
                              const string* prefix1 = 0);

    static list<string>& Wrap(const string& str, SIZE_TYPE width,
                              list<string>& arr, TWrapFlags flags,
                              const string& prefix,
                              const string* prefix1 = 0);

    static list<string>& Wrap(const string& str, SIZE_TYPE width,
                              list<string>& arr, TWrapFlags flags,
                              const string& prefix,
                              const string& prefix1);


    /// Wrap the list using the specified criteria.
    ///
    /// WrapList() is similar to Wrap(), but tries to avoid splitting any
    /// elements of the list to be wrapped.  Also, the "delim" only applies
    /// between elements on the same line; if you want everything to end with
    /// commas or such, you should add them first.
    ///
    /// @param l
    ///   The list to be wrapped.
    /// @param width
    ///   Width of each wrapped line.
    /// @param delim
    ///   Delimiters used to split elements on the same line.
    /// @param arr
    ///   List containing the wrapped list result.
    /// @param flags
    ///   How to wrap the words to a new line. See EWrapFlags documentation.
    /// @param prefix
    ///   The prefix string added to each wrapped line, except the first line,
    ///   unless "prefix1" is set.
    ///   If "prefix" is set to 0(default), do not add a prefix string to the
    ///   wrapped lines.
    /// @param prefix1
    ///   The prefix string for the first line. Use this for the first line
    ///   instead of "prefix".
    ///   If "prefix1" is set to 0(default), do not add a prefix string to the
    ///   first line.
    /// @return
    ///   Return "arr", the wrapped list.
    static list<string>& WrapList(const list<string>& l, SIZE_TYPE width,
                                  const string& delim, list<string>& arr,
                                  TWrapFlags    flags = 0,
                                  const string* prefix = 0,
                                  const string* prefix1 = 0);

    static list<string>& WrapList(const list<string>& l, SIZE_TYPE width,
                                  const string& delim, list<string>& arr,
                                  TWrapFlags    flags,
                                  const string& prefix,
                                  const string* prefix1 = 0);
        
    static list<string>& WrapList(const list<string>& l, SIZE_TYPE width,
                                  const string& delim, list<string>& arr,
                                  TWrapFlags    flags,
                                  const string& prefix,
                                  const string& prefix1);


    /// Justify the specified string into a series of lines of the same width.
    ///
    /// Split string "str" into a series of lines, all of which are to
    /// be "width" characters wide (by adding extra inner spaces between
    /// words), and store the resulting lines in the list "par".  Normally,
    /// all lines in "par" will begin with "pfx" (counted against "width"),
    /// but the first line will instead begin with "pfx1" if provided.
    ///
    /// @note Words exceeding the specified "width" will not be split between
    /// lines but occupy individual lines (which will be wider than "width").
    ///
    /// @param str
    ///   String to be split into justified lines.
    /// @param width
    ///   Width of every line (except for the last one).
    /// @param par
    ///   Resultant list of justified lines.
    /// @param pfx
    ///   The prefix string added to each line, except for the first line
    ///   if non-NULL "pfx1" is also set.  Empty(or NULL) "pfx" causes no
    ///   additions.
    /// @param pfx1
    ///   The prefix string for the first line, if non-NULL.
    /// @return
    ///   Return "par", the list of justified lines (a paragraph).
    static list<string>& Justify(const CTempString  str,
                                 SIZE_TYPE          width,
                                 list<string>&      par,
                                 const CTempString* pfx  = 0,
                                 const CTempString* pfx1 = 0);

    static list<string>& Justify(const CTempString  str,
                                 SIZE_TYPE          width,
                                 list<string>&      par,
                                 const CTempString  pfx,
                                 const CTempString* pfx1 = 0);

    static list<string>& Justify(const CTempString  str,
                                 SIZE_TYPE          width,
                                 list<string>&      par,
                                 const CTempString  pfx,
                                 const CTempString  pfx1);


    /// Search for a field.
    ///
    /// @param str
    ///   C or C++ string to search in.
    /// @param field_no
    ///   Zero-based field number.
    /// @param delimiters
    ///   A set of single-character delimiters.
    /// @param merge
    ///   Whether to merge or not adjacent delimiters. Default: not to merge.
    /// @return
    ///   Found field; or empty string if the required field is not found.
    /// @note
    ///   Field 0 spans up to the first-found delimiter or the end-of-string.
    static string GetField(const CTempString str,
                           size_t            field_no,
                           const CTempString delimiters,
                           EMergeDelims      merge = eNoMergeDelims);

    /// Search for a field.
    ///
    /// @param str
    ///   C or C++ string to search in.
    /// @param field_no
    ///   Zero-based field number.
    /// @param delimiter
    ///   A single-character delimiter.
    /// @param merge
    ///   Whether to merge or not adjacent delimiters. Default: not to merge.
    /// @return
    ///   Found field; or empty string if the required field is not found.
    /// @note
    ///   Field 0 spans up to the delimiter or the end-of-string.
    static string GetField(const CTempString str,
                           size_t            field_no,
                           char              delimiter,
                           EMergeDelims      merge = eNoMergeDelims);

    /// Search for a field.
    /// Avoid memory allocation at the expense of some usage safety.
    ///
    /// @param str
    ///   C or C++ string to search in.
    /// @param field_no
    ///   Zero-based field number.
    /// @param delimiters
    ///   A set of single-character delimiters.
    /// @param merge
    ///   Whether to merge or not adjacent delimiters. Default: not to merge.
    /// @return
    ///   Found field; or empty string if the required field is not found.
    /// @note
    ///   Field 0 spans up to the first-found delimiter or the end-of-string.
    /// @warning
    ///   The return value stores a pointer to the input string 'str' so
    ///   the return object validity time matches lifetime of the input 'str'.
    static
    CTempString GetField_Unsafe(const CTempString str,
                                size_t            field_no,
                                const CTempString delimiters,
                                EMergeDelims      merge = eNoMergeDelims);

    /// Search for a field.
    /// Avoid memory allocation at the expense of some usage safety.
    ///
    /// @param str
    ///   C or C++ string to search in.
    /// @param field_no
    ///   Zero-based field number.
    /// @param delimiter
    ///   A single-character delimiter.
    /// @param merge
    ///   Whether to merge or not adjacent delimiters. Default: not to merge.
    /// @return
    ///   Found field; or empty string if the required field is not found.
    /// @note
    ///   Field 0 spans up to the delimiter or the end-of-string.
    /// @warning
    ///   The return value stores a pointer to the input string 'str' so
    ///   the return object validity time matches lifetime of the input 'str'.
    static
    CTempString GetField_Unsafe(const CTempString str,
                                size_t            field_no,
                                char              delimiter,
                                EMergeDelims      merge = eNoMergeDelims);

private:
// implementations

// StringToNumeric
    static bool x_ReportLimitsError(const CTempString str, TStringToNumFlags flags);

    template< typename TNumeric, typename TSource>
    static bool x_VerifyIntLimits(TSource v, const CTempString str, TStringToNumFlags flags)
    {
        if (v < numeric_limits<TNumeric>::min()  ||  v > numeric_limits<TNumeric>::max()) {
            return x_ReportLimitsError(str, flags);
        }
        return true;
    }
    template< typename TNumeric, typename TSource>
    static bool x_VerifyFloatLimits(TSource v, const CTempString str, TStringToNumFlags flags)
    {
        // dont use ::min() for float types, it returns positive value
        if (v < -numeric_limits<TNumeric>::max()  ||  v > numeric_limits<TNumeric>::max()) {
            return x_ReportLimitsError(str, flags);
        }
        return true;
    }

    template <typename TNumeric>
    static typename enable_if< is_integral<TNumeric>::value && is_signed<TNumeric>::value && (sizeof(TNumeric) < sizeof(int)), TNumeric>::type
    x_StringToNumeric(const CTempString str, TStringToNumFlags flags, int base)
    {
        int n = StringToInt(str, flags, base);
        return x_VerifyIntLimits<TNumeric>(n, str, flags) ? (TNumeric)n : 0;
    }
    template <typename TNumeric>
    static typename enable_if< is_integral<TNumeric>::value && is_unsigned<TNumeric>::value && (sizeof(TNumeric) < sizeof(unsigned int)), TNumeric>::type
    x_StringToNumeric(const CTempString str, TStringToNumFlags flags, int base)
    {
        unsigned int n = StringToUInt(str, flags, base);
        return x_VerifyIntLimits<TNumeric>(n, str, flags) ? (TNumeric)n : 0;
    }

    template <typename TNumeric>
    static typename enable_if< is_integral<TNumeric>::value && is_signed<TNumeric>::value && (sizeof(TNumeric) == sizeof(int) && !is_same<TNumeric, long>::value), TNumeric>::type
    x_StringToNumeric(const CTempString str, TStringToNumFlags flags, int base)
    {
        return StringToInt(str, flags, base);
    }
    template <typename TNumeric>
    static typename enable_if< is_integral<TNumeric>::value && is_unsigned<TNumeric>::value && (sizeof(TNumeric) == sizeof(unsigned int) && !is_same<TNumeric, unsigned long>::value), TNumeric>::type
    x_StringToNumeric(const CTempString str, TStringToNumFlags flags, int base)
    {
        return StringToUInt(str, flags, base);
    }
    template <typename TNumeric>
    static typename enable_if< is_same<TNumeric, long>::value, TNumeric>::type
    x_StringToNumeric(const CTempString str, TStringToNumFlags flags, int base)
    {
        return StringToLong(str, flags, base);
    }
    template <typename TNumeric>
    static typename enable_if< is_same<TNumeric, unsigned long>::value, TNumeric>::type
    x_StringToNumeric(const CTempString str, TStringToNumFlags flags, int base)
    {
        return StringToULong(str, flags, base);
    }
    template <typename TNumeric>
    static typename enable_if< is_integral<TNumeric>::value && is_signed<TNumeric>::value && (sizeof(TNumeric) == sizeof(Int8) && !is_same<TNumeric, long>::value), TNumeric>::type
    x_StringToNumeric(const CTempString str, TStringToNumFlags flags, int base)
    {
        return StringToInt8(str, flags, base);
    }
    template <typename TNumeric>
    static typename enable_if< is_integral<TNumeric>::value && is_unsigned<TNumeric>::value && (sizeof(TNumeric) == sizeof(Uint8) && !is_same<TNumeric, unsigned long>::value), TNumeric>::type
    x_StringToNumeric(const CTempString str, TStringToNumFlags flags, int base)
    {
        return StringToUInt8(str, flags, base);
    }
    template <typename TNumeric>
    static typename enable_if< is_same<TNumeric, float>::value, TNumeric>::type
    x_StringToNumeric(const CTempString str, TStringToNumFlags flags, int /*base*/)
    {
        double n = StringToDouble(str, flags);
        return x_VerifyFloatLimits<TNumeric>(n, str, flags) ? (TNumeric)n : 0;
    }
    template <typename TNumeric>
    static typename enable_if< is_same<TNumeric, double>::value, TNumeric>::type
    x_StringToNumeric(const CTempString str, TStringToNumFlags flags, int /*base*/)
    {
        return StringToDouble(str, flags);
    }
#ifdef NCBI_STRICT_GI
    template <typename TNumeric>
    static typename enable_if< is_same<TNumeric, TGi>::value, TNumeric>::type
    x_StringToNumeric(const CTempString str, TStringToNumFlags flags, int base)
    {
        return x_StringToNumeric<TIntId>(str, flags, base);
    }
#endif

    template <typename TNumeric>
    static typename enable_if< is_integral<TNumeric>::value && is_signed<TNumeric>::value && (sizeof(TNumeric) < sizeof(int)), bool>::type
    x_StringToNumeric(const CTempString str, TNumeric* value, TStringToNumFlags flags, int base)
    {
        int n = StringToInt(str, flags, base);
        *value = 0;
        if (( !n && errno ) || !x_VerifyIntLimits<TNumeric>(n, str, flags)) {
            return false;
        }
        *value = (TNumeric) n;
        return true;
    }
    template <typename TNumeric>
    static typename enable_if< is_integral<TNumeric>::value && is_unsigned<TNumeric>::value && (sizeof(TNumeric) < sizeof(unsigned int)), bool>::type
    x_StringToNumeric(const CTempString str, TNumeric* value, TStringToNumFlags flags, int base)
    {
        unsigned int n = StringToUInt(str, flags, base);
        *value = 0;
        if (( !n && errno ) || !x_VerifyIntLimits<TNumeric>(n, str, flags)) {
            return false;
        }
        *value = (TNumeric) n;
        return true;
    }
    template <typename TNumeric>
    static typename enable_if< is_integral<TNumeric>::value && is_signed<TNumeric>::value && (sizeof(TNumeric) == sizeof(int) && !is_same<TNumeric, long>::value), bool>::type
    x_StringToNumeric(const CTempString str, TNumeric* value, TStringToNumFlags flags, int base)
    {
        *value = StringToInt(str, flags, base);
        return (*value || !errno);
    }
    template <typename TNumeric>
    static typename enable_if< is_integral<TNumeric>::value && is_unsigned<TNumeric>::value && (sizeof(TNumeric) == sizeof(unsigned int) && !is_same<TNumeric, unsigned long>::value), bool>::type
    x_StringToNumeric(const CTempString str, TNumeric* value, TStringToNumFlags flags, int base)
    {
        *value = StringToUInt(str, flags, base);
        return (*value || !errno);
    }
    static bool
    x_StringToNumeric(const CTempString str, long* value, TStringToNumFlags flags, int base)
    {
        *value = StringToLong(str, flags, base);
        return (*value || !errno);
    }
    static bool
    x_StringToNumeric(const CTempString str, unsigned long* value, TStringToNumFlags flags, int base)
    {
        *value = StringToULong(str, flags, base);
        return (*value || !errno);
    }
    template <typename TNumeric>
    static typename enable_if< is_integral<TNumeric>::value && is_signed<TNumeric>::value && (sizeof(TNumeric) == sizeof(Int8) && !is_same<TNumeric, long>::value), bool>::type
    x_StringToNumeric(const CTempString str, TNumeric* value, TStringToNumFlags flags, int base)
    {
        *value = StringToInt8(str, flags, base);
        return (*value || !errno);
    }
    template <typename TNumeric>
    static typename enable_if< is_integral<TNumeric>::value && is_unsigned<TNumeric>::value && (sizeof(TNumeric) == sizeof(Uint8) && !is_same<TNumeric, unsigned long>::value), bool>::type
    x_StringToNumeric(const CTempString str, TNumeric* value, TStringToNumFlags flags, int base)
    {
        *value = StringToUInt8(str, flags, base);
        return (*value || !errno);
    }
    static bool
    x_StringToNumeric(const CTempString str, float* value, TStringToNumFlags flags, int /*base*/)
    {
        double n = StringToDouble(str, flags);
        *value = 0;
        if (( !n && errno ) || !x_VerifyFloatLimits<float>(n, str, flags)) {
            return false;
        }
        *value = (float) n;
        return true;
    }
    static bool
    x_StringToNumeric(const CTempString str, double* value, TStringToNumFlags flags, int /*base*/)
    {
        *value = StringToDouble(str, flags);
        return (*value || !errno);
    }
#ifdef NCBI_STRICT_GI
    static bool
    x_StringToNumeric(const CTempString str, TGi* value, TStringToNumFlags flags, int base)
    {
        return x_StringToNumeric(str, reinterpret_cast<TIntId*>(value), flags, base);
    }
#endif

// NumericToString
    template<typename TNumeric>
    static typename enable_if< is_integral<TNumeric>::value && is_signed<TNumeric>::value && (sizeof(TNumeric) <= sizeof(int) && !is_same<TNumeric, long>::value), void>::type
    x_NumericToString(string& out_str, TNumeric value, TNumToStringFlags flags, int base)
    {
        IntToString(out_str, value, flags, base);
    }
    template<typename TNumeric>
    static typename enable_if< is_integral<TNumeric>::value && is_unsigned<TNumeric>::value && (sizeof(TNumeric) <= sizeof(unsigned int) && !is_same<TNumeric, unsigned long>::value), void>::type
    x_NumericToString(string& out_str, TNumeric value, TNumToStringFlags flags, int base)
    {
        UIntToString(out_str, value, flags, base);
    }
    static void
    x_NumericToString(string& out_str, long value, TNumToStringFlags flags, int base)
    {
        LongToString(out_str, value, flags, base);
    }
    static void
    x_NumericToString(string& out_str, unsigned long value, TNumToStringFlags flags, int base)
    {
        ULongToString(out_str, value, flags, base);
    }
#if NCBI_COMPILER_MSVC && (_MSC_VER < 1900)
    static void
    x_NumericToString(string& out_str, Int8 value, TNumToStringFlags flags, int base)
    {
        Int8ToString(out_str, value, flags, base);
    }
    static void
    x_NumericToString(string& out_str, Uint8 value, TNumToStringFlags flags, int base)
    {
        UInt8ToString(out_str, value, flags, base);
    }
#endif
    template<typename TNumeric>
    static typename enable_if< is_integral<TNumeric>::value && is_signed<TNumeric>::value && (sizeof(TNumeric) == sizeof(Int8) && !is_same<TNumeric, long>::value), void>::type
    x_NumericToString(string& out_str, TNumeric value, TNumToStringFlags flags, int base)
    {
        Int8ToString(out_str, value, flags, base);
    }
    template<typename TNumeric>
    static typename enable_if< is_integral<TNumeric>::value && is_unsigned<TNumeric>::value && (sizeof(TNumeric) == sizeof(Uint8) && !is_same<TNumeric, unsigned long>::value), void>::type
    x_NumericToString(string& out_str, TNumeric value, TNumToStringFlags flags, int base)
    {
        UInt8ToString(out_str, value, flags, base);
    }
    template<typename TNumeric>
    static typename enable_if< is_floating_point<TNumeric>::value, void>::type
    x_NumericToString(string& out_str, TNumeric value, TNumToStringFlags flags, int /*base*/)
    {
        DoubleToString(out_str, value, -1, flags);
    }
#ifdef NCBI_STRICT_GI
    static void
    x_NumericToString(string& out_str, TGi value, TNumToStringFlags flags, int base)
    {
        x_NumericToString(out_str, TIntId(value), flags, base);
    }
#endif


// Join
    template<typename TIterator>
    static string xx_Join( TIterator from, TIterator to, const CTempString& delim);

    template<typename TIterator>
    static typename enable_if<is_same<typename TIterator::iterator_category, input_iterator_tag>::value &&
                              is_convertible<typename TIterator::value_type, string>::value, string>::type
    x_Join( TIterator from, TIterator to, const CTempString& delim)
    {
        return TransformJoin(from, to, delim, [](const typename TIterator::value_type& i){ return i;});
    }

    template<typename TIterator>
    static typename enable_if<is_convertible<typename TIterator::iterator_category, forward_iterator_tag>::value &&
                              is_convertible<typename TIterator::value_type, string>::value, string>::type
    x_Join( TIterator from, TIterator to, const CTempString& delim)
    {
        return xx_Join(from, to, delim);
    }

    template<typename TValue>
    static typename enable_if<is_convertible<TValue, string>::value, string>::type
    x_Join( TValue* from, TValue* to, const CTempString& delim)
    {
        return xx_Join(from, to, delim);
    }

    template<typename TIterator>
    static typename enable_if<is_convertible<typename TIterator::iterator_category, input_iterator_tag>::value &&
                              is_arithmetic< typename TIterator::value_type>::value, string>::type
    x_Join( TIterator from, TIterator to, const CTempString& delim)
    {
        return TransformJoin( from, to, delim, [](const typename TIterator::value_type& i){ return NumericToString(i);});
    }

    template<typename TValue>
    static typename enable_if<is_arithmetic<TValue>::value, string>::type
    x_Join( TValue* from, TValue* to, const CTempString& delim)
    {
        return TransformJoin( from, to, delim, [](const TValue& i){ return NumericToString(i);});
    }
}; // class NStr



/////////////////////////////////////////////////////////////////////////////
///


#define NCBITOOLKIT_USE_LONG_UCS4 (SIZEOF_LONG == 4)
#if NCBITOOLKIT_USE_LONG_UCS4
/// UCS-4 character
typedef unsigned long TCharUCS4;
/// UCS-4 string
typedef basic_string<TCharUCS4> TStringUCS4;
#else
typedef TUnicodeSymbol TCharUCS4;
typedef TStringUnicode TStringUCS4;
#endif

/// Type for character in UCS-2 encoding
typedef Uint2 TCharUCS2;
/// Type for string in UCS-2 encoding
typedef basic_string<TCharUCS2> TStringUCS2;


/// Operator for writing TStringUCS2 to stream.
/// Operator is needed for using in SDBAPI.
inline CNcbiOstream& operator<< (CNcbiOstream& os, const TStringUCS2& str)
{
    os.write((const char*)str.data(), str.size() * sizeof(TCharUCS2));
    return os;
}



/////////////////////////////////////////////////////////////////////////////
///
/// CUtf8 --
///
///   Utility class to handle strings in UTF8 encoding.
///   Can convert data to and from the following encodings:
///      ISO 8859-1 (Latin1)
///      Microsoft Windows code page 1252
///      UCS-2, UCS-4 (no surrogates)

class NCBI_XNCBI_EXPORT CUtf8
{
public:
    /// How to verify character encoding of the source data
    enum EValidate {
        eNoValidate,
        eValidate
    };

    /// Convert into UTF8 from a C/C++ string
    ///
    /// @param src
    ///   Source string
    /// @param encoding
    ///   Character encoding of the source string
    /// @param validate
    ///   Verify the character encoding of the source
    static CStringUTF8 AsUTF8(const CTempString& src,
                              EEncoding          encoding,
                              EValidate          validate = eNoValidate)
    {
        CStringUTF8 u8;
        return x_Append(u8,src,encoding,validate);
    }

#if defined(HAVE_WSTRING)
    /// Convert into UTF8 from a C/C++ string
    ///
    /// @param src
    ///   Source string
    /// @param lcl
    ///   String locale
    static CStringUTF8 AsUTF8(const CTempString& src, const locale& lcl)
    {
        CStringUTF8 u8;
        return x_Append(u8,src,lcl);
    }
#endif

    /// Convert into UTF8 from a Unicode C++ string
    ///
    /// @param src
    ///   Source string
    /// @attention
    ///   Only for TStringUnicode, TStringUCS4, TStringUCS2, wstring types
    template <typename TChar>
    static typename enable_if< is_integral<TChar>::value  && (1 < sizeof(TChar)), CStringUTF8>::type
    AsUTF8(const basic_string<TChar>& src)
    {
        CStringUTF8 u8;
        return  x_Append(u8, src.data(), src.size());
    }

    /// Convert into UTF8 from a Unicode character buffer
    ///
    /// @param src
    ///   Source character buffer
    /// @param tchar_count
    ///   Number of characters in the buffer;
    ///   If it equals to NPOS, buffer is assumed to be zero-terminated
    template <typename TChar>
    static typename enable_if< is_integral<TChar>::value  && (1 < sizeof(TChar)), CStringUTF8>::type
    AsUTF8(const TChar* src, SIZE_TYPE tchar_count = NPOS)
    {
        CStringUTF8 u8;
        return  x_Append(u8, src, tchar_count);
    }

    /// Convert Unicode C++ string into UTF8 and append it to existing string
    ///
    /// @param dest
    ///   Existing UTF8 string
    /// @param src
    ///   Source Unicode string
    /// return
    ///   reference to modified dest string
    template <typename TChar>
    static typename enable_if< is_integral<TChar>::value  && (1 < sizeof(TChar)), CStringUTF8& >::type
    AppendAsUTF8(CStringUTF8& dest, const basic_string<TChar>& src)
    {
        return x_Append(dest, src.data(), src.size());
    }

    /// Convert Unicode character buffer into UTF8 and append it to existing string
    ///
    /// @param dest
    ///   Existing UTF8 string
    /// @param src
    ///   Source Unicode character buffer
    /// @param tchar_count
    ///   Number of characters in the buffer;
    ///   If it equals to NPOS, buffer is assumed to be zero-terminated
    /// return
    ///   reference to modified dest string
    template <typename TChar>
    static typename enable_if< is_integral<TChar>::value  && (1 < sizeof(TChar)), CStringUTF8& >::type
    AppendAsUTF8(CStringUTF8& dest, const TChar* src, SIZE_TYPE tchar_count = NPOS)
    {
        return x_Append(dest, src, tchar_count);
    }

    /// Convert Unicode symbol into UTF8 and append it to existing string
    ///
    /// @param dest
    ///   Existing UTF8 string
    /// @param ch
    ///   Unicode symbol
    /// return
    ///   reference to modified dest string
    template <typename TChar>
    static typename enable_if< is_integral<TChar>::value  && (1 < sizeof(TChar)), CStringUTF8& >::type
    AppendAsUTF8(CStringUTF8& dest, TChar ch)
    {
        return  x_Append(dest, &ch, 1);
    }

    /// Convert non-Unicode C++ string into UTF8 and append it to existing string
    ///
    /// @param dest
    ///   Existing UTF8 string
    /// @param src
    ///   Source string
    /// @param encoding
    ///   Character encoding of the source string
    /// @param validate
    ///   Verify the character encoding of the source
    /// return
    ///   reference to modified dest string
    static CStringUTF8& AppendAsUTF8(CStringUTF8&       dest,
                                     const CTempString& src,
                                     EEncoding          encoding, 
                                     EValidate          validate = eNoValidate)
    {
        return  x_Append(dest,src,encoding,validate);
    }

#if defined(HAVE_WSTRING)
    /// Convert non-Unicode C++ string into UTF8 and append it to existing string
    ///
    /// @param dest
    ///   Existing UTF8 string
    /// @param src
    ///   Source string
    /// @param lcl
    ///   Source string locale
    /// return
    ///   reference to modified dest string
    static CStringUTF8& AppendAsUTF8(CStringUTF8&       dest,
                                     const CTempString& src,
                                     const locale&      lcl)
    {
        return  x_Append(dest,src,lcl);
    }
#endif

    /// Convert non-Unicode character into UTF8 and append it to existing string
    ///
    /// @param dest
    ///   Existing UTF8 string
    /// @param ch
    ///   Character
    /// @param encoding
    ///   Character encoding
    /// @param validate
    ///   Verify the character encoding of the source
    /// return
    ///   reference to modified dest string
    static CStringUTF8& AppendAsUTF8(CStringUTF8& dest,
                                     char         ch,
                                     EEncoding    encoding, 
                                     EValidate    validate = eNoValidate)
    {
        return  x_Append(dest,CTempString(&ch,1),encoding,validate);
    }

#if defined(HAVE_WSTRING)
    /// Convert non-Unicode character into UTF8 and append it to existing string
    ///
    /// @param dest
    ///   Existing UTF8 string
    /// @param ch
    ///   Character
    /// @param lcl
    ///   Character locale
    /// return
    ///   reference to modified dest string
    static CStringUTF8& AppendAsUTF8(CStringUTF8&  dest,
                                     char          ch,
                                     const locale& lcl)
    {
        return  x_Append(dest,CTempString(&ch,1),lcl);
    }
#endif

    /// Convert UTF8 string into a single-byte character representation
    ///
    /// Can throw a CStringException if the conversion is impossible
    /// or the string has invalid UTF-8 encoding.
    ///
    /// @param src
    ///   Source UTF8 string   
    /// @param encoding
    ///   Encoding of the result
    /// @param substitute_on_error
    ///   If the conversion is impossible, append the provided string
    ///   or, if substitute_on_error equals 0, throw an exception
    /// @param validate
    ///   Verify UTF8 character encoding of the source
    /// @return
    ///   C++ string
    static string AsSingleByteString
        (const CTempString& src, EEncoding encoding,
         const char* substitute_on_error = 0, EValidate validate = eNoValidate);

#if defined(HAVE_WSTRING)
    static string AsSingleByteString
        (const CTempString& src, const locale& lcl,
         const char* substitute_on_error = 0, EValidate validate = eNoValidate);
#endif

    /// Convert UTF8 string into Unicode
    ///
    /// Can throw a CStringException if the conversion is impossible
    /// or the string has invalid UTF-8 encoding.
    ///
    /// @param src
    ///   Source UTF8 string   
    /// @param substitute_on_error
    ///   If the conversion is impossible, append the provided string
    ///   or, if substitute_on_error equals 0, throw an exception
    /// @param validate
    ///   Verify UTF8 character encoding of the source
    /// @attention
    ///   Only for TStringUnicode, TStringUCS4, TStringUCS2, wstring types
    template <typename TChar>
    static typename enable_if< is_integral<TChar>::value  && (1 < sizeof(TChar)), basic_string<TChar> >::type
    AsBasicString(const CTempString& src, const TChar* substitute_on_error, EValidate validate = eNoValidate)
    {
        return x_AsBasicString(src,substitute_on_error,validate);
    }

    template <typename TChar>
    static typename enable_if< is_integral<TChar>::value  && (1 < sizeof(TChar)), basic_string<TChar> >::type
    AsBasicString(const CTempString& src)
    {
        return x_AsBasicString<TChar>(src,nullptr,eNoValidate);
    }

    /// Get the number of symbols (code points) in UTF8 string
    ///
    /// @param src
    ///   Source UTF8 string   
    /// @return
    ///   Number of symbols (code points)
    static SIZE_TYPE GetSymbolCount(const CTempString& src);

    /// Get the number of valid UTF-8 symbols (code points) in buffer
    ///
    /// @param src
    ///   Character buffer
    /// @return
    ///   Number of valid symbols (no exception thrown)
    static SIZE_TYPE GetValidSymbolCount(const CTempString& src);

    /// Get the number of valid UTF-8 bytes (code units) in buffer
    ///
    /// @param src
    ///   Character buffer
    /// @return
    ///   Number of valid bytes (no exception thrown)
    static SIZE_TYPE GetValidBytesCount(const CTempString& src);

    /// Check buffer for presence of UTF-8 byte sequence and return length of first symbol
    ///
    /// @param src
    ///   Character buffer
    /// @return
    ///   Number of bytes
    static SIZE_TYPE EvaluateSymbolLength(const CTempString& src);

    /// Check that the character is valid first byte of an UTF8 byte sequence
    ///
    /// @param ch
    ///   Character
    /// @param more
    ///   Number of additional bytes to expect
    /// @return
    ///   true, if this is a valid first byte
    static bool EvaluateFirst(char ch, SIZE_TYPE& more) {
        return x_EvalFirst(ch, more);
    }

    /// Check that the character is valid continuation byte of an UTF8 byte sequence
    ///
    /// @param ch
    ///   Character
    /// @return
    ///   true, if this is a valid byte
    static bool EvaluateNext(char ch) {
        return x_EvalNext(ch);
    }

    /// Check the encoding of the C/C++ string
    ///
    /// Check that the encoding of the source is the same, or
    /// is compatible with the specified one
    /// @param src
    ///   Source string
    /// @param encoding
    ///   Character encoding form to check against
    /// @return
    ///   Boolean result: encoding is same or compatible
    static bool MatchEncoding(const CTempString& src, EEncoding encoding);
    
    /// Guess the encoding of the C/C++ string
    ///
    /// It can distinguish between UTF-8, Latin1, and Win1252 only
    /// @param src
    ///   Character buffer
    /// @return
    ///   Encoding as guessed;  eEncoding_Unknown if cannot guess
    static EEncoding GuessEncoding(const CTempString& src);

    /// Give Encoding name as string
    ///
    /// @param encoding
    ///   EEncoding enum. (Throw CStringException if passed eEncoding_Unknown.)
    /// @return
    ///   Encoding name
    static string EncodingToString(EEncoding encoding);

    /// Convert encoding name into EEncoding enum, taking into account synonyms
    /// as per  http://www.iana.org/assignments/character-sets
    ///
    /// @param encoding_name
    ///   Name of the encoding
    /// @return
    ///   EEncoding enum;  eEncoding_Unknown for unsupported encodings
    static EEncoding StringToEncoding(const CTempString& encoding_name);

    /// Convert encoded character into Unicode
    ///
    /// @param ch
    ///   Encoded character
    /// @param encoding
    ///   Character encoding
    /// @return
    ///   Unicode code point (symbol)
    static TUnicodeSymbol CharToSymbol(char ch, EEncoding encoding);

#if defined(HAVE_WSTRING)
    /// Convert encoded character into Unicode
    ///
    /// @param ch
    ///   Encoded character
    /// @param lcl
    ///   Character locale
    /// @return
    ///   Unicode code point (symbol)
    static TUnicodeSymbol CharToSymbol(char ch, const locale& lcl);
#endif

    /// Convert Unicode code point into encoded character
    ///
    /// @param sym
    ///   Unicode code point (symbol)
    /// @param encoding
    ///   Character encoding
    /// @return
    ///   Encoded character
    static char SymbolToChar(TUnicodeSymbol sym, EEncoding encoding);

#if defined(HAVE_WSTRING)
    /// Convert Unicode code point into encoded character
    ///
    /// @param sym
    ///   Unicode code point (symbol)
    /// @param lcl
    ///   Character locale
    /// @return
    ///   Encoded character
    static char SymbolToChar(TUnicodeSymbol sym, const locale& lcl);
#endif

    /// Determines if a symbol is whitespace
    /// per  http://unicode.org/charts/uca/chart_Whitespace.html
    ///
    /// @param sym
    ///   Unicode code point (symbol)
    /// @sa
    ///   TruncateSpacesInPlace, TruncateSpaces_Unsafe, TruncateSpaces
    static bool IsWhiteSpace(TUnicodeSymbol sym);

    /// Truncate spaces in the string (in-place)
    ///
    /// @param src
    ///   UTF8 string
    /// @param side
    ///   Which end of the string to truncate spaces from. Default is to
    ///   truncate spaces from both ends.
    /// @return
    ///   Reference to src
    /// @sa
    ///   IsWhiteSpace, TruncateSpaces_Unsafe, TruncateSpaces
    static CStringUTF8& TruncateSpacesInPlace
    (CStringUTF8& str, NStr::ETrunc side = NStr::eTrunc_Both);

    /// Truncate spaces in the string
    ///
    /// @param str
    ///   Source string, in UTF8 encoding
    /// @param side
    ///   Which end of the string to truncate spaces from. Default is to
    ///   truncate spaces from both ends.
    /// @sa
    ///   IsWhiteSpace, TruncateSpacesInPlace, TruncateSpaces_Unsafe
    static CStringUTF8 TruncateSpaces
        (const CTempString& str, NStr::ETrunc side = NStr::eTrunc_Both);

    /// Truncate spaces in the string
    ///
    /// @param str
    ///   Source string, in UTF8 encoding
    /// @param side
    ///   Which end of the string to truncate spaces from. Default is to
    ///   truncate spaces from both ends.
    /// @attention
    ///   The lifespan of the result string is the same as one of the source.
    ///   So, for example, if the source is temporary string, then the result
    ///   will be invalid right away (will point to already released memory).
    /// @sa
    ///   IsWhiteSpace, TruncateSpacesInPlace, TruncateSpaces
    static CTempString TruncateSpaces_Unsafe
        (const CTempString& str, NStr::ETrunc side = NStr::eTrunc_Both);

    /// Convert sequence of UTF8 code units into Unicode code point
    ///
    /// @param src
    ///   Zero-terminated buffer, in UTF8 encoding
    /// @return
    ///   Unicode code point
    static TUnicodeSymbol Decode(const char*& src);

#ifndef NCBI_COMPILER_WORKSHOP
    /// Convert sequence of UTF8 code units into Unicode code point
    ///
    /// @param src
    ///   C++ string iterator
    /// @return
    ///   Unicode code point
    static TUnicodeSymbol Decode(string::const_iterator& src);
#endif

    /// Begin converting first character of UTF8 sequence into Unicode
    ///
    /// @param ch
    ///   Character
    /// @param more
    ///   If the character is valid, - how many more characters to expect
    /// @return
    ///   Part of Unicode code point. Zero if the character is invalid.
    static TUnicodeSymbol DecodeFirst(char ch, SIZE_TYPE& more);

    /// Convert next character of UTF8 sequence into Unicode
    ///
    /// @param ch
    ///   Character
    /// @param chU
    ///   Incomplete Unicode code point
    /// @return
    ///   Accumulated Unicode code point. Zero if the character is invalid.
    static TUnicodeSymbol DecodeNext(TUnicodeSymbol chU, char ch);

private:
    static void x_Validate(const CTempString& str);

    static SIZE_TYPE x_GetValidSymbolCount
        (const CTempString& src, CTempString::const_iterator& err);

    static CStringUTF8& x_AppendChar(CStringUTF8& u8str, TUnicodeSymbol ch);

    static CStringUTF8& x_Append(CStringUTF8& u8str, const CTempString& src,
                                 EEncoding encoding, EValidate validate);
#if defined(HAVE_WSTRING)
    static CStringUTF8& x_Append(CStringUTF8& u8str, const CTempString& src, const locale& lcl);
#endif
    template <typename TChar>
    static CStringUTF8& x_Append(CStringUTF8& u8str, const TChar* src, SIZE_TYPE tchar_count);

    template <typename TChar>
    static basic_string<TChar> x_AsBasicString
        (const CTempString& src,
         const TChar* substitute_on_error, EValidate validate);
    
    template <typename TIterator>
    static TUnicodeSymbol x_Decode(TIterator& src);

    static SIZE_TYPE x_BytesNeeded(TUnicodeSymbol ch);
    static bool   x_EvalFirst(char ch, SIZE_TYPE& more);
    static bool   x_EvalNext(char ch);

    // returns part of the string around an error in Utf8 encoding
    static CTempString x_GetErrorFragment(const CTempString& src);

    friend class CStringUTF8_DEPRECATED;
};

// deprecated CStringUTF8 is there
#include <corelib/impl/stringutf8_deprecated.hpp>



/////////////////////////////////////////////////////////////////////////////
///
/// CParseTemplException --
///
/// Define template class for parsing exception. This class is used to define
/// exceptions for complex parsing tasks and includes an additional m_Pos
/// data member. The constructor requires that an additional positional
/// parameter be supplied along with the description message.

template <class TBase>
class CParseTemplException : EXCEPTION_VIRTUAL_BASE public TBase
{
public:
    /// Error types that for exception class.
    enum EErrCode {
        eErr        ///< Generic error 
    };

    /// Translate from the error code value to its string representation.
    virtual const char* GetErrCodeString(void) const override
    {
        switch (GetErrCode()) {
        case eErr: return "eErr";
        default:   return CException::GetErrCodeString();
        }
    }

    /// Constructor.
    ///
    /// Report "pos" along with "what".
    CParseTemplException(const CDiagCompileInfo &info,
        const CException* prev_exception,
        EErrCode err_code,const string& message,
        string::size_type pos, EDiagSev severity = eDiag_Error)
          : TBase(info, prev_exception, message, severity, 0), m_Pos(pos)
    {
        this->x_Init(info,
                     string("{") + NStr::SizetToString(m_Pos) +
                     "} " + message,
                     prev_exception,
                     severity);
        this->x_InitErrCode((CException::EErrCode) err_code);
    }

    /// Constructor.
    CParseTemplException(const CParseTemplException<TBase>& other)
        : TBase(other)
    {
        m_Pos = other.m_Pos;
        this->x_Assign(other);
    }

    /// Destructor.
    virtual ~CParseTemplException(void) throw() {}

    /// Report error position.
    virtual void ReportExtra(ostream& out) const override
    {
        out << "m_Pos = " << (unsigned long)m_Pos;
    }

    // Attributes.

    /// Get exception class type.
    virtual const char* GetType(void) const override
        { return "CParseTemplException"; }

    typedef int TErrCode;
    /// Get error code.
    TErrCode GetErrCode(void) const
    {
        return typeid(*this) == typeid(CParseTemplException<TBase>) ?
            (TErrCode) this->x_GetErrCode() :
            (TErrCode) CException::eInvalid;
    }

    /// Get error position.
    string::size_type GetPos(void) const throw() { return m_Pos; }

protected:
    CParseTemplException(const CDiagCompileInfo &info,
        const CException* prev_exception,
        const string& message,
        string::size_type pos, EDiagSev severity, CException::TFlags flags)
          : TBase(info, prev_exception, message, severity, flags), m_Pos(pos)
    {
        this->x_Init(info,
                     string("{") + NStr::SizetToString(m_Pos) +
                     "} " + message,
                     prev_exception,
                     severity);
    }
    /// Constructor.
    CParseTemplException(void)
    {
        m_Pos = 0;
    }

    /// Helper clone method.
    virtual const CException* x_Clone(void) const override
    {
        return new CParseTemplException<TBase>(*this);
    }

private:
    string::size_type m_Pos;    ///< Error position
};


/////////////////////////////////////////////////////////////////////////////
///
/// CStringException --
///
/// Define exceptions generated by string classes.
///
/// CStringException inherits its basic functionality from
/// CParseTemplException<CCoreException> and defines additional error codes
/// for string parsing.

class NCBI_XNCBI_EXPORT CStringException : public CParseTemplException<CCoreException>
{
public:
    /// Error types that string classes can generate.
    enum EErrCode {
        eConvert,       ///< Failure to convert string
        eBadArgs,       ///< Bad arguments to string methods 
        eFormat         ///< Wrong format for any input to string methods
    };

    /// Translate from the error code value to its string representation.
    virtual const char* GetErrCodeString(void) const override;

    // Standard exception boilerplate code.
    NCBI_EXCEPTION_DEFAULT2(CStringException,
        CParseTemplException<CCoreException>, std::string::size_type);
};



/////////////////////////////////////////////////////////////////////////////
///
/// CStringPairsParser --
///
/// Base class for parsing a string to a set of name-value pairs.


/// Decoder interface. Names and values can be decoded with different rules.
class IStringDecoder
{
public:
    /// Type of string to be decoded
    enum EStringType {
        eName,
        eValue
    };
    /// Decode the string. Must throw CStringException if the source string
    /// is not valid.
    virtual string Decode(const CTempString src, EStringType stype) const = 0;
    virtual ~IStringDecoder(void) {}
};


/// Encoder interface. Names and values can be encoded with different rules.
class IStringEncoder
{
public:
    /// Type of string to be decoded
    enum EStringType {
        eName,
        eValue
    };
    /// Encode the string.
    virtual string Encode(const CTempString src, EStringType stype) const = 0;
    virtual ~IStringEncoder(void) {}
};


/// URL-decoder for string pairs parser
class NCBI_XNCBI_EXPORT CStringDecoder_Url : public IStringDecoder
{
public:
    CStringDecoder_Url(NStr::EUrlDecode flag = NStr::eUrlDec_All);

    virtual string Decode(const CTempString src, EStringType stype) const;

private:
    NStr::EUrlDecode m_Flag;
};


/// URL-encoder for string pairs parser
class NCBI_XNCBI_EXPORT CStringEncoder_Url : public IStringEncoder
{
public:
    CStringEncoder_Url(NStr::EUrlEncode flag = NStr::eUrlEnc_SkipMarkChars);

    virtual string Encode(const CTempString src, EStringType stype) const;

private:
    NStr::EUrlEncode m_Flag;
};


/// Template for parsing string into pairs of name and value or merging
/// them back into a single string.
/// The container class must hold pairs of strings (pair<string, string>).
template<class TContainer>
class CStringPairs
{
public:
    typedef TContainer TStrPairs;
    /// The container's value type must be pair<string, string>
    /// or a compatible type.
    typedef typename TContainer::value_type TStrPair;

    /// Create parser with the specified decoder/encoder and default separators.
    ///
    /// @param decoder
    ///   String decoder (Url, Xml etc.)
    /// @param own_decoder
    ///   Decoder ownership flag
    /// @param decoder
    ///   String encoder (Url, Xml etc.), optional
    /// @param own_encoder
    ///   Encoder ownership flag, optional
    CStringPairs(IStringDecoder* decoder = NULL,
                 EOwnership      own_decoder = eTakeOwnership,
                 IStringEncoder* encoder = NULL,
                 EOwnership      own_encoder = eTakeOwnership)
        : m_ArgSep("&"),
          m_ValSep("="),
          m_Decoder(decoder, own_decoder),
          m_Encoder(encoder, own_encoder)
    {
    }

    /// Create parser with the specified parameters.
    ///
    /// @param arg_sep
    ///   Separator between name+value pairs
    /// @param val_sep
    ///   Separator between name and value
    /// @param decoder
    ///   String decoder (Url, Xml etc.)
    /// @param own_decoder
    ///   Decoder ownership flag
    /// @param encoder
    ///   String encoder (Url, Xml etc.)
    /// @param own_encoder
    ///   Encoder ownership flag
    CStringPairs(const CTempString arg_sep,
                 const CTempString val_sep,
                 IStringDecoder*   decoder = NULL,
                 EOwnership        own_decoder = eTakeOwnership,
                 IStringEncoder*   encoder = NULL,
                 EOwnership        own_encoder = eTakeOwnership)
        : m_ArgSep(arg_sep),
          m_ValSep(val_sep),
          m_Decoder(decoder, own_decoder),
          m_Encoder(encoder, own_encoder)
    {
    }

    /// Create parser with the selected URL-encoding/decoding options
    /// and default separators.
    ///
    /// @param decode_flag
    ///   URL-decoding flag
    /// @param encode_flag
    ///   URL-encoding flag
    CStringPairs(NStr::EUrlDecode decode_flag,
                 NStr::EUrlEncode encode_flag)
        : m_ArgSep("&"),
          m_ValSep("="),
          m_Decoder(new CStringDecoder_Url(decode_flag), eTakeOwnership),
          m_Encoder(new CStringEncoder_Url(encode_flag), eTakeOwnership)
    {
    }

    virtual ~CStringPairs(void) {}

    /// Set string decoder.
    ///
    /// @param decoder
    ///   String decoder (Url, Xml etc.)
    /// @param own
    ///   Decoder ownership flag
    void SetDecoder(IStringDecoder* decoder, EOwnership own = eTakeOwnership)
        { m_Decoder.reset(decoder, own); }
    /// Get decoder or NULL. Does not affect decoder ownership.
    IStringDecoder* GetDecoder(void) { return m_Decoder.get(); }

    /// Set string encoder.
    ///
    /// @param encoder
    ///   String encoder (Url, Xml etc.)
    /// @param own
    ///   Encoder ownership flag
    void SetEncoder(IStringEncoder* encoder, EOwnership own = eTakeOwnership)
        { m_Encoder.reset(encoder, own); }
    /// Get encoder or NULL. Does not affect encoder ownership.
    IStringEncoder* GetEncoder(void) { return m_Encoder.get(); }

    /// Parse the string.
    ///
    /// @param str
    ///   String to parse. The parser assumes the string is formatted like
    ///   "name1<valsep>value1<argsep>name2<valsep>value2...". Each name and
    ///   value is passed to the decoder (if not NULL) before storing the pair.
    /// @param merge_argsep
    ///   Flag for merging separators between pairs. By default the separators
    ///   are merged to prevent pairs where both name and value are empty.
    void Parse(const CTempString str,
               NStr::EMergeDelims merge_argsep = NStr::eMergeDelims)
    {
        Parse(m_Data, str, m_ArgSep, m_ValSep,
              m_Decoder.get(), eNoOwnership, merge_argsep);
    }

    /// Parse the string using the provided decoder, put data into the
    /// container.
    ///
    /// @param pairs
    ///   Container to be filled with the parsed name/value pairs
    /// @param str
    ///   String to parse. The parser assumes the string is formatted like
    ///   "name1<valsep>value1<argsep>name2<valsep>value2...". Each name and
    ///   value is passed to the decoder (if not NULL) before storing the pair.
    /// @param decoder
    ///   String decoder (Url, Xml etc.)
    /// @param own
    ///   Flag indicating if the decoder must be deleted by the function.
    /// @param merge_argsep
    ///   Flag for merging separators between pairs. By default the separators
    ///   are merged to prevent pairs where both name and value are empty.
    static void Parse(TStrPairs&         pairs,
                      const CTempString  str,
                      const CTempString  arg_sep,
                      const CTempString  val_sep,
                      IStringDecoder*    decoder = NULL,
                      EOwnership         own = eTakeOwnership,
                      NStr::EMergeDelims merge_argsep = NStr::eMergeDelims)
    {
        AutoPtr<IStringDecoder> decoder_guard(decoder, own);
        list<string> lst;
        NStr::Split(str, arg_sep, lst, (NStr::TSplitFlags)merge_argsep);
        pairs.clear();
        ITERATE(list<string>, it, lst) {
            string name, val;
            NStr::SplitInTwo(*it, val_sep, name, val);
            if ( decoder ) {
                try {
                    name = decoder->Decode(name, IStringDecoder::eName);
                    val = decoder->Decode(val, IStringDecoder::eValue);
                }
                catch (CStringException) {
                    // Discard all data
                    pairs.clear();
                    throw;
                }
            }
            pairs.insert(pairs.end(), TStrPair(name, val));
        }
    }

    /// Merge name-value pairs into a single string using the currently set
    /// separators and the provided encoder if any.
    string Merge(void) const
    {
        return Merge(m_Data, m_ArgSep, m_ValSep,
                     m_Encoder.get(), eNoOwnership);
    }

    /// Merge name-value pairs from the provided container, separators
    /// and encoder. Delete the encoder if the ownership flag allows.
    ///
    /// @param pairs
    ///   Container with the name/value pairs to be merged.
    /// @param arg_sep
    ///   Separator to be inserted between pairs.
    /// @param val_sep
    ///   Separator to be inserted between name and value.
    /// @param encoder
    ///   String encoder (Url, Xml etc.)
    /// @param own
    ///   Flag indicating if the encoder must be deleted by the function.
    static string Merge(const TStrPairs& pairs,
                        const string&    arg_sep,
                        const string&    val_sep,
                        IStringEncoder*  encoder = NULL,
                        EOwnership       own = eTakeOwnership)
    {
        AutoPtr<IStringEncoder> encoder_guard(encoder, own);
        string ret;
        ITERATE(typename TStrPairs, it, pairs) {
            if ( !ret.empty() ) {
                ret += arg_sep;
            }
            if ( encoder ) {
                ret += encoder->Encode(it->first, IStringEncoder::eName) +
                    val_sep +
                    encoder->Encode(it->second, IStringEncoder::eValue);
            }
            else {
                ret += it->first + val_sep + it->second;
            }
        }
        return ret;
    }

    /// Read data
    const TStrPairs& GetPairs(void) const { return m_Data; }
    /// Get non-const data
    TStrPairs& GetPairs(void) { return m_Data; }

private:
    string                  m_ArgSep;   // Separator between name+value pairs ("&")
    string                  m_ValSep;   // Separator between name and value ("=")
    AutoPtr<IStringDecoder> m_Decoder;  // String decoder (Url, Xml etc.)
    AutoPtr<IStringEncoder> m_Encoder;  // String encoder (Url, Xml etc.)
    TStrPairs               m_Data;     // Parsed data
};


typedef vector<pair<string, string> > TStringPairsVector;
typedef CStringPairs<TStringPairsVector> CStringPairsParser;


/////////////////////////////////////////////////////////////////////////////
///
/// CEncodedString --
///
/// Class to detect if a string needs to be URL-encoded and hold both
/// encoded and original versions.
///

class NCBI_XNCBI_EXPORT CEncodedString
{
public:
    CEncodedString(void) {}
    CEncodedString(const CTempString s,
                   NStr::EUrlEncode flag = NStr::eUrlEnc_SkipMarkChars);

    /// Set new original string
    void SetString(const CTempString s,
                   NStr::EUrlEncode flag = NStr::eUrlEnc_SkipMarkChars);

    /// Check if the original string was encoded.
    bool IsEncoded(void) const { return m_Encoded.get() != 0; }
    /// Get the original unencoded string
    const string& GetOriginalString(void) const { return m_Original; }
    /// Get encoded string
    const string& GetEncodedString(void) const
        { return IsEncoded() ? *m_Encoded : m_Original; }

    /// Check if the string is empty
    bool IsEmpty(void) const { return m_Original.empty(); }

private:
    string           m_Original;
    unique_ptr<string> m_Encoded;
};


/////////////////////////////////////////////////////////////////////////////
//  Predicates
//


/////////////////////////////////////////////////////////////////////////////
///
/// Define Case-sensitive string comparison methods.
///
/// Used as arguments to template functions for specifying the type of 
/// comparison.

template <typename T>
struct PCase_Generic
{
    /// Return difference between "s1" and "s2".
    int Compare(const T& s1, const T& s2) const;

    /// Return TRUE if s1 < s2.
    bool Less(const T& s1, const T& s2) const;

    /// Return TRUE if s1 == s2.
    bool Equals(const T& s1, const T& s2) const;

    /// Return TRUE if s1 < s2.
    bool operator()(const T& s1, const T& s2) const;
};

typedef PCase_Generic<string>       PCase;
typedef PCase_Generic<const char *> PCase_CStr;



/////////////////////////////////////////////////////////////////////////////
///
/// Define Case-insensitive string comparison methods.
///
/// Used as arguments to template functions for specifying the type of 
/// comparison.
///
/// @sa PNocase_Conditional_Generic

template <typename T>
struct PNocase_Generic
{
    /// Return difference between "s1" and "s2".
    int Compare(const T& s1, const T& s2) const;

    /// Return TRUE if s1 < s2.
    bool Less(const T& s1, const T& s2) const;

    /// Return TRUE if s1 == s2.
    bool Equals(const T& s1, const T& s2) const;

    /// Return TRUE if s1 < s2 ignoring case.
    bool operator()(const T& s1, const T& s2) const;
};

typedef PNocase_Generic<string>       PNocase;
typedef PNocase_Generic<const char *> PNocase_CStr;


/////////////////////////////////////////////////////////////////////////////
///
/// Define Case-insensitive string comparison methods.
/// Case sensitivity can be turned on and off at runtime.
///
/// Used as arguments to template functions for specifying the type of 
/// comparison.
///
/// @sa PNocase_Generic

template <typename T>
class PNocase_Conditional_Generic
{
public:
    /// Construction
    PNocase_Conditional_Generic(NStr::ECase case_sens = NStr::eCase);

    /// Get comparison type
    NStr::ECase GetCase() const { return m_CaseSensitive; }

    /// Set comparison type
    void SetCase(NStr::ECase case_sens) { m_CaseSensitive = case_sens; }

    /// Return difference between "s1" and "s2".
    int Compare(const T& s1, const T& s2) const;

    /// Return TRUE if s1 < s2.
    bool Less(const T& s1, const T& s2) const;

    /// Return TRUE if s1 == s2.
    bool Equals(const T& s1, const T& s2) const;

    /// Return TRUE if s1 < s2 ignoring case.
    bool operator()(const T& s1, const T& s2) const;
private:
    NStr::ECase m_CaseSensitive; ///< case sensitive when TRUE
};

typedef PNocase_Conditional_Generic<string>       PNocase_Conditional;
typedef PNocase_Conditional_Generic<const char *> PNocase_Conditional_CStr;


/////////////////////////////////////////////////////////////////////////////
///
/// PQuickStringLess implements an ordering of strings,
/// that is more efficient than usual lexicographical order.
/// It can be used in cases when no specific order is required,
/// e.g. only simple key lookup is needed.
/// Current implementation first compares lengths of strings,
/// and will compare string data only when lengths are the same.
///
struct PQuickStringLess
{
    bool operator()(const CTempString s1, const CTempString s2) const {
        size_t len1 = s1.size(), len2 = s2.size();
        return len1 < len2 ||
            (len1 == len2 && ::memcmp(s1.data(), s2.data(), len1) < 0);
    }
};


/////////////////////////////////////////////////////////////////////////////
//  Algorithms
//


/// Check equivalence of arguments using predicate.
template<class Arg1, class Arg2, class Pred>
inline
bool AStrEquiv(const Arg1& x, const Arg2& y, Pred pr)
{
    return pr.Equals(x, y);
}


/* @} */



/////////////////////////////////////////////////////////////////////////////
//
//  IMPLEMENTATION of INLINE functions
//
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//  CNcbiEmptyString::
//
#if !defined(NCBI_OS_MSWIN)  &&  \
    !(defined(NCBI_OS_LINUX)  &&  \
      (defined(NCBI_COMPILER_GCC)  ||  defined(NCBI_COMPILER_ANY_CLANG)))
inline
const string& CNcbiEmptyString::Get(void)
{
    const string* str = m_Str;
    return str ? *str: FirstGet();
}

#  ifdef HAVE_WSTRING
inline
const wstring& CNcbiEmptyWString::Get(void)
{
    const wstring* str = m_Str;
    return str ? *str: FirstGet();
}
#  endif
#endif



/////////////////////////////////////////////////////////////////////////////
//  NStr::
//

inline
string NStr::IntToString(int value,
                         TNumToStringFlags flags, int base)
{
    string ret;
    IntToString(ret, value, flags, base);
    return ret;
}

inline
string NStr::IntToString(unsigned int value,
                         TNumToStringFlags flags, int base)
{
    string ret;
    IntToString(ret, (int)value, flags, base);
    return ret;
}

inline
void NStr::IntToString(string& out_str, unsigned int value, 
                       TNumToStringFlags flags, int base)
{
    IntToString(out_str, (int)value, flags, base);
}

inline
string NStr::UIntToString(unsigned int value,
                          TNumToStringFlags flags, int base)
{
    string ret;
    ULongToString(ret, value, flags, base);
    return ret;
}

inline
string NStr::UIntToString(int value,
                          TNumToStringFlags flags, int base)
{
    string ret;
    UIntToString(ret, (unsigned int)value, flags, base);
    return ret;
}

inline
void NStr::UIntToString(string& out_str, unsigned int value,
                        TNumToStringFlags flags, int base)
{
    ULongToString(out_str, value, flags, base);
}

inline
void NStr::UIntToString(string& out_str, int value,
                        TNumToStringFlags flags, int base)
{
    UIntToString(out_str, (unsigned int)value, flags, base);
}

inline
string NStr::LongToString(long value,
                          TNumToStringFlags flags, int base)
{
    string ret;
    LongToString(ret, value, flags, base);
    return ret;
}

inline
string NStr::ULongToString(unsigned long value,
                           TNumToStringFlags flags, int base)
{
    string ret;
    ULongToString(ret, value, flags, base);
    return ret;
}

inline
string NStr::Int8ToString(Int8 value,
                          TNumToStringFlags flags, int base)
{
    string ret;
    NStr::Int8ToString(ret, value, flags, base);
    return ret;
}

inline
string NStr::UInt8ToString(Uint8 value,
                           TNumToStringFlags flags, int base)
{
    string ret;
    NStr::UInt8ToString(ret, value, flags, base);
    return ret;
}

inline
string NStr::UInt8ToString_DataSize(Uint8 value,
                                    TNumToStringFlags flags /* = 0 */,
                                    unsigned int max_digits /* = 3 */)
{
    string ret;
    NStr::UInt8ToString_DataSize(ret, value, flags, max_digits);
    return ret;
}

inline
string NStr::DoubleToString(double value, int precision,
                            TNumToStringFlags flags)
{
    string str;
    DoubleToString(str, value, precision, flags);
    return str;
}

inline
int NStr::HexChar(char ch)
{
    unsigned int rc = ch - '0';
    if (rc <= 9) {
        return rc;
    } else {
        rc = (ch | ' ') - 'a';
        return rc <= 5 ? int(rc + 10) : -1;
    }
}

inline
int NStr::strcmp(const char* s1, const char* s2)
{
    return ::strcmp(s1, s2);
}

inline
int NStr::strncmp(const char* s1, const char* s2, size_t n)
{
    return ::strncmp(s1, s2, n);
}

inline
int NStr::strcasecmp(const char* s1, const char* s2)
{
#if defined(HAVE_STRICMP)
#if NCBI_COMPILER_MSVC && (_MSC_VER >= 1400)
    return ::_stricmp(s1, s2);
#else
    return ::stricmp(s1, s2);
#endif

#elif defined(HAVE_STRCASECMP_LC)
    return ::strcasecmp(s1, s2);

#else
    int diff = 0;
    for ( ;; ++s1, ++s2) {
        char c1 = *s1;
        // calculate difference
        diff = tolower((unsigned char) c1) - tolower((unsigned char)(*s2));
        // if end of string or different
        if (!c1  ||  diff)
            break; // return difference
    }
    return diff;
#endif
}

inline
int NStr::strncasecmp(const char* s1, const char* s2, size_t n)
{
#if defined(HAVE_STRICMP)
#if NCBI_COMPILER_MSVC && (_MSC_VER >= 1400)
    return ::_strnicmp(s1, s2, n);
#else
    return ::strnicmp(s1, s2, n);
#endif

#elif defined(HAVE_STRCASECMP_LC)
    return ::strncasecmp(s1, s2, n);

#else
    int diff = 0;
    for ( ; ; ++s1, ++s2, --n) {
        if (n == 0)
            return 0;
        char c1 = *s1;
        // calculate difference
        diff = tolower((unsigned char) c1) - tolower((unsigned char)(*s2));
        // if end of string or different
        if (!c1  ||  diff)
            break; // return difference
    }
    return diff;
#endif
}

inline
size_t NStr::strftime(char* s, size_t maxsize, const char* format,
                      const struct tm* timeptr)
{
    string x_format = Replace(format, "%T", "%H:%M:%S");
    ReplaceInPlace(x_format,          "%D", "%m/%d/%y");
    return ::strftime(s, maxsize, x_format.c_str(), timeptr);
}

inline
int NStr::CompareCase(const char* s1, const char* s2)
{
    return NStr::strcmp(s1, s2);
}

inline
int NStr::CompareNocase(const char* s1, const char* s2)
{
    return NStr::strcasecmp(s1, s2);
}

inline
int NStr::Compare(const CTempString s1, SIZE_TYPE pos, SIZE_TYPE n,
                  const char* s2, ECase use_case)
{
    return use_case == eCase ? CompareCase(s1.substr(pos, n), s2)
                             : CompareNocase(s1.substr(pos, n), s2);
}

inline
int NStr::Compare(const CTempString s1, SIZE_TYPE pos, SIZE_TYPE n,
                  const CTempString s2, ECase use_case)
{
    return use_case == eCase ? CompareCase(s1.substr(pos, n), s2)
                             : CompareNocase(s1.substr(pos, n), s2);
}

inline
int NStr::Compare(const char* s1, const char* s2, ECase use_case)
{
    return use_case == eCase ? CompareCase(s1, s2) : CompareNocase(s1, s2);
}

inline
int NStr::Compare(const CTempStringEx s1, const CTempStringEx s2, ECase use_case)
{
    return use_case == eCase ? CompareCase(s1, s2) : CompareNocase(s1, s2);
}

inline
bool NStr::EqualCase(const CTempString s1, SIZE_TYPE pos, SIZE_TYPE n, const char* s2)
{
    return s1.substr(pos, n) == s2;
}

inline
bool NStr::EqualCase(const CTempString s1, SIZE_TYPE pos, SIZE_TYPE n, const CTempString s2)
{
    return s1.substr(pos, n) == s2;
}

inline
bool NStr::EqualCase(const char* s1, const char* s2)
{
    size_t n = strlen(s1);
    if (n != strlen(s2)) {
        return false;
    }
    return NStr::strncmp(s1, s2, n) == 0;
}

inline
bool NStr::EqualCase(const CTempStringEx s1, const CTempStringEx s2)
{
    return s1 == s2;
}

inline
bool NStr::EqualNocase(const CTempString s1, SIZE_TYPE pos, SIZE_TYPE n, const char* s2)
{
    return CompareNocase(s1.substr(pos, n), s2) == 0;
}

inline
bool NStr::EqualNocase(const CTempString s1, SIZE_TYPE pos, SIZE_TYPE n, const CTempString s2)
{
    return CompareNocase(s1.substr(pos, n), s2) == 0;
}

inline
bool NStr::EqualNocase(const char* s1, const char* s2)
{
    size_t n = strlen(s1);
    if (n != strlen(s2)) {
        return false;
    }
    return NStr::strncasecmp(s1, s2, n) == 0;
}

inline
bool NStr::EqualNocase(const CTempStringEx s1, const CTempStringEx s2)
{
    if (s1.length() != s2.length()) {
        return false;
    }
    return CompareNocase(s1, s2) == 0;
}

inline
bool NStr::Equal(const CTempString s1, SIZE_TYPE pos, SIZE_TYPE n,
                 const char* s2, ECase use_case)
{
    return use_case == eCase ? EqualCase(s1.substr(pos, n), s2) 
                             : EqualNocase(s1.substr(pos, n), s2);
}

inline
bool NStr::Equal(const CTempString s1, SIZE_TYPE pos, SIZE_TYPE n,
                 const CTempString s2, ECase use_case)
{
    return use_case == eCase ? EqualCase(s1.substr(pos, n), s2)
                             : EqualNocase(s1.substr(pos, n), s2);
}

inline
bool NStr::Equal(const char* s1, const char* s2, ECase use_case)
{
    return use_case == eCase ? EqualCase(s1, s2) : EqualNocase(s1, s2);
}

inline
bool NStr::Equal(const CTempStringEx s1, const CTempStringEx s2, ECase use_case)
{
    return use_case == eCase ? EqualCase(s1, s2) : EqualNocase(s1, s2);
}

inline
bool NStr::StartsWith(const CTempString str, const CTempString start, ECase use_case)
{
    return str.size() >= start.size()  &&
           Equal(str.substr(0, start.size()), start, use_case);
}

inline
bool NStr::StartsWith(const CTempString str, char start, ECase use_case)
{
    return !str.empty()  &&
           (use_case == eCase ? (str[0] == start)
                              : (str[0] == start  ||
                                 toupper((unsigned char) str[0]) == start  ||
                                 tolower((unsigned char) str[0]))
           );
}

inline
bool NStr::EndsWith(const CTempString str, const CTempString end, ECase use_case)
{
    return str.size() >= end.size()  &&
           Equal(str.substr(str.size() - end.size(), end.size()), end, use_case);
}

inline
bool NStr::EndsWith(const CTempString str, char end, ECase use_case)
{
    if (!str.empty()) {
        char last = str[str.length() - 1];
        return use_case == eCase ? (last == end)
                                 : (last == end  ||
                                    toupper((unsigned char) last) == end  ||
                                    tolower((unsigned char) last) == end);
    }
    return false;
}

inline
SIZE_TYPE NStr::CommonPrefixSize(const CTempString s1, const CTempString s2)
{
    const SIZE_TYPE n = min(s1.length(), s2.length());
    for (SIZE_TYPE i = 0;  i < n;  i++) {
        if (s1[i] != s2[i]) {
            return i;
        }
    }
    return n;
}

inline
SIZE_TYPE NStr::CommonSuffixSize(const CTempString s1, const CTempString s2)
{
    const SIZE_TYPE len1 = s1.length();
    const SIZE_TYPE len2 = s2.length();
    const SIZE_TYPE n = min(len1, len2);
    for (SIZE_TYPE i = 1; i <= n; i++) {
        if (s1[len1 - i] != s2[len2 - i]) {
            return i - 1;
        }
    }
    return n;
}

inline
SIZE_TYPE NStr::Find(const CTempString str, const CTempString pattern,
                     SIZE_TYPE start, SIZE_TYPE end, EOccurrence where,
                     ECase use_case)
{
    SIZE_TYPE pos = Find(CTempString(str, start, end - start), pattern, use_case,
                         where == eFirst ? eForwardSearch : eReverseSearch, 0);
    if (pos == NPOS) {
        return NPOS;
    }
    return pos + start;
}

// @deprecated
inline
SIZE_TYPE NStr::FindCase(const CTempString str, const CTempString pattern,
                         SIZE_TYPE start, SIZE_TYPE end, EOccurrence where)
{
    if (where == eFirst) {
        SIZE_TYPE pos = str.find(pattern, start);
        return (pos == NPOS  ||  (pos + pattern.length()) > end) ? NPOS : pos;
    } else {
        SIZE_TYPE pos = str.rfind(pattern, end);
        return (pos == NPOS  ||  pos < start) ? NPOS : pos;
    }
}

inline
SIZE_TYPE NStr::FindCase(const CTempString str, const CTempString pattern)
{
    return Find(str, pattern, eCase);
}

inline
SIZE_TYPE NStr::FindCase(const CTempString str, const CTempString pattern, SIZE_TYPE start)
{
    SIZE_TYPE pos = Find(CTempString(str, start), pattern, eCase);
    if (pos == NPOS) {
        return NPOS;
    }
    return pos + start;
}

inline
SIZE_TYPE NStr::FindNoCase(const CTempString str, const CTempString pattern)
{
    return Find(str, pattern, eNocase);
}

inline
SIZE_TYPE NStr::FindNoCase(const CTempString str, const CTempString pattern, SIZE_TYPE start)
{
    SIZE_TYPE pos = Find(CTempString(str, start), pattern, eNocase);
    if (pos == NPOS) {
        return NPOS;
    }
    return pos + start;
}

inline
const string* NStr::FindCase(const list<string>& lst, const CTempString val)
{
    return Find(lst, val, eCase);
}

inline
const string* NStr::FindNoCase(const list <string>& lst, const CTempString val)
{
    return Find(lst, val, eNocase);
}

inline
const string* NStr::FindCase(const vector <string>& vec, const CTempString val)
{
    return Find(vec, val, eCase);
}

inline
const string* NStr::FindNoCase(const vector <string>& vec, const CTempString val)
{
    return Find(vec, val, eNocase);
}

template<typename TIterator, typename FTransform>
string
NStr::TransformJoin( TIterator from, TIterator to, const CTempString& delim, FTransform fnTransform)
{
    if (from == to) {
        return kEmptyStr;
    }
    string result(fnTransform(*from++));
    for ( ; from != to; ++from) {
        result.append(delim).append(fnTransform(*from));
    }
    return result;    
}

template<typename TIterator>
string
NStr::xx_Join( TIterator from, TIterator to, const CTempString& delim)
{
    if (from == to) {
        return kEmptyStr;
    }
    string result(*from++);
    size_t sz_all = 0, sz_delim = delim.size();
    for ( TIterator f = from; f != to; ++f) {
        sz_all += string(*f).size() + sz_delim;
    }
    result.reserve(result.size() + sz_all);
    for ( ; from != to; ++from) {
        result.append(delim).append(string(*from));
    }
    return result;    
}

inline
list<string>& NStr::Wrap(const string& str, SIZE_TYPE width, list<string>& arr,
                         NStr::TWrapFlags flags, const string& prefix,
                         const string* prefix1)
{
    return Wrap(str, width, arr, flags, &prefix, prefix1);
}

inline
list<string>& NStr::Wrap(const string& str, SIZE_TYPE width, list<string>& arr,
                         NStr::TWrapFlags flags, const string& prefix,
                         const string& prefix1)
{
    return Wrap(str, width, arr, flags, &prefix, &prefix1);
}

inline
list<string>& NStr::WrapList(const list<string>& l, SIZE_TYPE width,
                             const string& delim, list<string>& arr,
                             NStr::TWrapFlags flags, const string& prefix,
                             const string* prefix1)
{
    return WrapList(l, width, delim, arr, flags, &prefix, prefix1);
}

inline
list<string>& NStr::WrapList(const list<string>& l, SIZE_TYPE width,
                             const string& delim, list<string>& arr,
                             NStr::TWrapFlags flags, const string& prefix,
                             const string& prefix1)
{
    return WrapList(l, width, delim, arr, flags, &prefix, &prefix1);
}

inline
list<string>& NStr::Justify(const CTempString str, SIZE_TYPE width,
                            list<string>& par, const CTempString pfx,
                            const CTempString* pfx1)
{
    return Justify(str, width, par, &pfx, pfx1);
}

inline
list<string>& NStr::Justify(const CTempString str, SIZE_TYPE width,
                            list<string>& par, const CTempString pfx,
                            const CTempString pfx1)
{
    return Justify(str, width, par, &pfx, &pfx1);
}



/////////////////////////////////////////////////////////////////////////////
//  CUtf8:: 
//

inline SIZE_TYPE CUtf8::GetValidSymbolCount(const CTempString& src) {
    CTempString::const_iterator err;
    return x_GetValidSymbolCount(src, err);
}

inline SIZE_TYPE CUtf8::GetValidBytesCount(const CTempString& src) {
    CTempString::const_iterator err;
    x_GetValidSymbolCount(src,err);
    return (err-src.begin());
}
inline TUnicodeSymbol CUtf8::Decode(const char*& src) {
    return x_Decode(src);
}
#ifndef NCBI_COMPILER_WORKSHOP
inline TUnicodeSymbol CUtf8::Decode(string::const_iterator& src) {
    return x_Decode(src);
}
#endif

template <typename TIterator> inline TUnicodeSymbol
CUtf8::x_Decode(TIterator& src)
{
    SIZE_TYPE more=0;
    TUnicodeSymbol sym = DecodeFirst(*src,more);
    while (more--) {
        sym = DecodeNext(sym, *(++src));
    }
    return sym;
}

template <typename TChar> basic_string<TChar>
CUtf8::x_AsBasicString(const CTempString& str,
    const TChar* substitute_on_error, EValidate validate)
{
    if (validate == eValidate) {
        x_Validate(str);
    }
    TUnicodeSymbol max_char = (TUnicodeSymbol)numeric_limits<TChar>::max();
    basic_string<TChar> result;
    result.reserve(CUtf8::GetSymbolCount(str) + 1);
    CTempString::const_iterator src = str.begin();
    CTempString::const_iterator to  = str.end();
    for (; src != to; ++src) {
        TUnicodeSymbol ch = Decode(src);
        if (ch > max_char) {
            if (substitute_on_error) {
                result.append(substitute_on_error);
                continue;
            } else {
                NCBI_THROW2(CStringException, eConvert,
                    "Failed to convert symbol to wide character",
                    (src - str.begin()));
            }
        }
        result.append(1, (TChar)ch);
    }
    return result;
}

template <typename TChar> CStringUTF8&
CUtf8::x_Append(CStringUTF8& u8str, const TChar* src, SIZE_TYPE to)
{
    const TChar* srcBuf;
    SIZE_TYPE needed = 0;
    SIZE_TYPE pos=0;

    for (pos=0, srcBuf=src;
            (to == NPOS) ? (*srcBuf != 0) : (pos<to); ++pos, ++srcBuf) {
        needed += x_BytesNeeded( *srcBuf );
    }
    if ( !needed ) {
        return u8str;
    }
    u8str.reserve(max(u8str.capacity(),u8str.length()+needed+1));
    for (pos=0, srcBuf=src;
            (to == NPOS) ? (*srcBuf != 0) : (pos<to); ++pos, ++srcBuf) {
        x_AppendChar( u8str, *srcBuf );
    }
    return u8str;
}

inline  CStringUTF8
CUtf8::TruncateSpaces(const CTempString& str, NStr::ETrunc side) {
    CStringUTF8 u8;
    return x_Append(u8, TruncateSpaces_Unsafe(str,side), eEncoding_UTF8, eNoValidate);
}

// deprecated CStringUTF8 is there
#include <corelib/impl/stringutf8_deprecated.inl>



/////////////////////////////////////////////////////////////////////////////
//  PCase_Generic::
//

template <typename T>
inline
int PCase_Generic<T>::Compare(const T& s1, const T& s2) const
{
    return NStr::Compare(s1, s2, NStr::eCase);
}

template <typename T>
inline
bool PCase_Generic<T>::Less(const T& s1, const T& s2) const
{
    return Compare(s1, s2) < 0;
}

template <typename T>
inline
bool PCase_Generic<T>::Equals(const T& s1, const T& s2) const
{
    return Compare(s1, s2) == 0;
}

template <typename T>
inline
bool PCase_Generic<T>::operator()(const T& s1, const T& s2) const
{
    return Less(s1, s2);
}



////////////////////////////////////////////////////////////////////////////
//  PNocase_Generic<T>::
//


template <typename T>
inline
int PNocase_Generic<T>::Compare(const T& s1, const T& s2) const
{
    return NStr::Compare(s1, s2, NStr::eNocase);
}

template <typename T>
inline
bool PNocase_Generic<T>::Less(const T& s1, const T& s2) const
{
    return Compare(s1, s2) < 0;
}

template <typename T>
inline
bool PNocase_Generic<T>::Equals(const T& s1, const T& s2) const
{
    return Compare(s1, s2) == 0;
}

template <typename T>
inline
bool PNocase_Generic<T>::operator()(const T& s1, const T& s2) const
{
    return Less(s1, s2);
}

////////////////////////////////////////////////////////////////////////////
//  PNocase_Conditional_Generic<T>::
//

template <typename T>
inline
PNocase_Conditional_Generic<T>::PNocase_Conditional_Generic(NStr::ECase cs)
    : m_CaseSensitive(cs)
{}

template <typename T>
inline
int PNocase_Conditional_Generic<T>::Compare(const T& s1, const T& s2) const
{
    return NStr::Compare(s1, s2, m_CaseSensitive);
}

template <typename T>
inline
bool PNocase_Conditional_Generic<T>::Less(const T& s1, const T& s2) const
{
    return Compare(s1, s2) < 0;
}

template <typename T>
inline
bool PNocase_Conditional_Generic<T>::Equals(const T& s1, const T& s2) const
{
    return Compare(s1, s2) == 0;
}

template <typename T>
inline
bool PNocase_Conditional_Generic<T>::operator()(const T& s1, const T& s2) const
{
    return Less(s1, s2);
}


END_NCBI_NAMESPACE;

#endif  /* CORELIB___NCBISTR__HPP */
