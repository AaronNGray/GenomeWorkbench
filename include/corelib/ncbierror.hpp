#ifndef CORELIB___NCBIERROR__HPP
#define CORELIB___NCBIERROR__HPP

/*  $Id: ncbierror.hpp 566948 2018-07-10 19:41:29Z lavr $
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
 * Author:  Andrei Gourianov
 *
 *
 */

/// @file ncbierror.hpp
/// Defines NCBI C++ Toolkit portable error codes.

#include <corelib/ncbimisc.hpp>
#include <errno.h>
#include <corelib/impl/ncbierror_impl.hpp>


/** @addtogroup Exception
 *
 * @{
 */

BEGIN_NCBI_SCOPE


class NCBI_XNCBI_EXPORT CNcbiError
{
public:
    enum ECode {
        eNotSet                         = -1,
        eSuccess                        = 0,

        /// Generic error codes are set based on errno error codes.
        eAddressFamilyNotSupported      = EAFNOSUPPORT,
        eAddressInUse                   = EADDRINUSE,
        eAddressNotAvailable            = EADDRNOTAVAIL,
        eAlreadyConnected               = EISCONN,
        eArgumentListTooLong            = E2BIG,
        eArgumentOutOfDomain            = EDOM,
        eBadAddress                     = EFAULT,
        eBadFileDescriptor              = EBADF,
        eBadMessage                     = EBADMSG,
        eBrokenPipe                     = EPIPE,
        eConnectionAborted              = ECONNABORTED,
        eConnectionAlreadyInProgress    = EALREADY,
        eConnectionRefused              = ECONNREFUSED,
        eConnectionReset                = ECONNRESET,
        eCrossDeviceLink                = EXDEV,
        eDestinationAddressRequired     = EDESTADDRREQ,
        eDeviceOrResourceBusy           = EBUSY,
        eDirectoryNotEmpty              = ENOTEMPTY,
        eExecutableFormatError          = ENOEXEC,
        eFileExists                     = EEXIST,
        eFileTooLarge                   = EFBIG,
        eFilenameTooLong                = ENAMETOOLONG,
        eFunctionNotSupported           = ENOSYS,
        eHostUnreachable                = EHOSTUNREACH,
        eIdentifierRemoved              = EIDRM,
        eIllegalByteSequence            = EILSEQ,
        eInappropriateIoControlOperation= ENOTTY,
        eInterrupted                    = EINTR,
        eInvalidArgument                = EINVAL,
        eInvalidSeek                    = ESPIPE,
        eIoError                        = EIO,
        eIsADirectory                   = EISDIR,
        eMessageSize                    = EMSGSIZE,
        eNetworkDown                    = ENETDOWN,
        eNetworkReset                   = ENETRESET,
        eNetworkUnreachable             = ENETUNREACH,
        eNoBufferSpace                  = ENOBUFS,
        eNoChildProcess                 = ECHILD,
        eNoLink                         = ENOLINK,
        eNoLockAvailable                = ENOLCK,
//        eNoMessageAvailable             = ENODATA,
        eNoMessage                      = ENOMSG,
        eNoProtocolOption               = ENOPROTOOPT,
        eNoSpaceOnDevice                = ENOSPC,
//        eNoStreamResources              = ENOSR,
        eNoSuchDeviceOrAddress          = ENXIO,
        eNoSuchDevice                   = ENODEV,
        eNoSuchFileOrDirectory          = ENOENT,
        eNoSuchProcess                  = ESRCH,
        eNotADirectory                  = ENOTDIR,
        eNotASocket                     = ENOTSOCK,
//        eNotAStream                     = ENOSTR,
        eNotConnected                   = ENOTCONN,
        eNotEnoughMemory                = ENOMEM,
        eNotSupported                   = ENOTSUP,
        eOperationCanceled              = ECANCELED,
        eOperationInProgress            = EINPROGRESS,
        eOperationNotPermitted          = EPERM,
        eOperationNotSupported          = EOPNOTSUPP,
        eOperationWouldBlock            = EWOULDBLOCK,
//        eOwnerDead                      = EOWNERDEAD,
        ePermissionDenied               = EACCES,
        eProtocolError                  = EPROTO,
        eProtocolNotSupported           = EPROTONOSUPPORT,
        eReadOnlyFileSystem             = EROFS,
        eResourceDeadlockWouldOccur     = EDEADLK,
        eResourceUnavailableTryAgain    = EAGAIN,
        eResultOutOfRange               = ERANGE,
//        eStateNotRecoverable            = ENOTRECOVERABLE,
//        eStreamTimeout                  = ETIME,
        eTextFileBusy                   = ETXTBSY,
        eTimedOut                       = ETIMEDOUT,
        eTooManyFilesOpenInSystem       = ENFILE,
        eTooManyFilesOpen               = EMFILE,
        eTooManyLinks                   = EMLINK,
        eTooManySymbolicLinkLevels      = ELOOP,
        eValueTooLarge                  = EOVERFLOW,
        eWrongProtocolType              = EPROTOTYPE,

        /// Unknown error
        eUnknown = 0x1000

        /// NCBI-specific error codes
    };

    /// Error code category.
    enum ECategory {
        eGeneric   = 0,
        eNcbi      = 1,
        eMsWindows = 2
    };

    /// Get error code.
    ECode Code(void) const;

    /// Get error code category.
    ECategory Category(void) const {
        return m_Category;
    } 
   
    /// Get native numeric value of the error.
    int Native(void) const {
        return m_Native;
    }    

    /// Get string information provided when this error was set.
    const string& Extra(void) const {
        return m_Extra;
    }    

    /// Copy constructor
    CNcbiError(const CNcbiError& err)
        : m_Code(err.m_Code), 
          m_Category(err.m_Category),
          m_Native(err.m_Native),
          m_Extra(err.m_Extra) 
    {}

    ~CNcbiError(void) {}

    /// Assignment.
    CNcbiError& operator= (const CNcbiError& err) {
        m_Code     = err.m_Code;
        m_Category = err.m_Category;
        m_Native   = err.m_Native;
        m_Extra    = err.m_Extra;
        return *this;
    }

    /// Comparison.
    bool operator== (ECode err) const {
        return Code() == err;
    }

    /// Operator bool: returns TRUE if error was not set to "success"
    DECLARE_OPERATOR_BOOL(Code() != eSuccess);

    /// Get the error that was last set (in the current thread)
    /// @sa Set(), SetErrno(), SetFromErrno(), SetWindowsError()
    /// @sa SetFromWindowsError()
    static const CNcbiError& GetLast(void);

    /// Set last error using native error code enum
    ///
    /// @param code
    ///   Error code
    /// @param extra
    ///   Additional information
    static void Set(ECode code);
    /// @copydoc CNcbiError::Set(ECode)
    static void Set(ECode code, const CTempString extra);
    /// @copydoc CNcbiError::Set(ECode)
    static void Set(ECode code, const char* extra);
    /// @copydoc CNcbiError::Set(ECode)
    static void Set(ECode code, const string& extra);
    /// @copydoc CNcbiError::Set(ECode)
    static void Set(ECode code, string&& extra);

    /// Set last error using errno code
    ///
    /// @param errno_code
    ///   "errno" code
    /// @param extra
    ///   Additional information
    static void SetErrno(int errno_code);
    /// @copydoc CNcbiError::SetErrno(int)
    static void SetErrno(int errno_code, const CTempString extra);
    /// @copydoc CNcbiError::SetErrno(int)
    static void SetErrno(int errno_code, const string& extra);
    /// @copydoc CNcbiError::SetErrno(int)
    static void SetErrno(int errno_code, const char* extra);
    /// @copydoc CNcbiError::SetErrno(int)
    static void SetErrno(int errno_code, string&& extra);

    /// Set last error using current "errno" code
    ///
    /// @param extra
    ///   Additional information
    static void SetFromErrno(void);
    /// @copydoc CNcbiError::SetFromErrno(void)
    static void SetFromErrno(const CTempString extra);
    /// @copydoc CNcbiError::SetFromErrno(void)
    static void SetFromErrno(const string& extra);
    /// @copydoc CNcbiError::SetFromErrno(void)
    static void SetFromErrno(const char* extra);
    /// @copydoc CNcbiError::SetFromErrno(void)
    static void SetFromErrno(string&& extra);

#if defined(NCBI_OS_MSWIN)
    /// Set last error using Windows-specific error code
    ///
    /// @param native_err_code
    ///   Windows-specific error code
    /// @param extra
    ///   Additional information
    /// @note
    ///   Not all Windows errors can be translated into ECode enum.
    ///   In this case, Code() will return 'eUnknown'
    static void SetWindowsError(int native_err_code);
    /// @copydoc CNcbiError::SetWindowsError(int)
    static void SetWindowsError(int native_err_code, const CTempString extra);
    /// @copydoc CNcbiError::SetWindowsError(int)
    static void SetWindowsError(int native_err_code, const string& extra);
    /// @copydoc CNcbiError::SetWindowsError(int)
    static void SetWindowsError(int native_err_code, const char* extra);
    /// @copydoc CNcbiError::SetWindowsError(int)
    static void SetWindowsError(int native_err_code, string&& extra);

    /// Set last error on MS Windows using GetLastError()
    ///
    /// @param extra
    ///   Additional information
    /// @note
    ///   Not all Windows errors can be translated into ECode enum.
    ///   In this case, Code() will return 'eUnknown'
    static void SetFromWindowsError(void);
    /// @copydoc CNcbiError::SetFromWindowsError(void)
    static void SetFromWindowsError(const CTempString extra);
    /// @copydoc CNcbiError::SetFromWindowsError(void)
    static void SetFromWindowsError(const string& extra);
    /// @copydoc CNcbiError::SetFromWindowsError(void)
    static void SetFromWindowsError(const char* extra);
    /// @copydoc CNcbiError::SetFromWindowsError(void)
    static void SetFromWindowsError(string&& extra);
#endif  /* NCBI_OS_MSWIN */

protected:
    // Prohibit creation of 'empty' error object
    CNcbiError(void);

private:
    static CNcbiError* x_Init(int err_code);
    template<class Ty>
    static CNcbiError* x_Init(int err_code, Ty extra);
#ifdef NCBI_OS_MSWIN
    static void x_SetWindowsCodeCategory(CNcbiError* e);
#endif

private:
    mutable ECode m_Code;
    ECategory     m_Category;
    int           m_Native;
    string        m_Extra;
};


/// Serialize error code + description provided by OS + extra string data
NCBI_XNCBI_EXPORT CNcbiOstream& operator<< (CNcbiOstream&     str,
                                            const CNcbiError& err);


END_NCBI_SCOPE


/* @} */

#endif  /* CORELIB___NCBIERROR__HPP */
