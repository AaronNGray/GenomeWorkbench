#ifndef CONNECT_SERVICES__NETSTORAGE__HPP
#define CONNECT_SERVICES__NETSTORAGE__HPP

/*  $Id: netstorage.hpp 575325 2018-11-27 18:22:00Z ucko $
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
 * Authors:  Dmitry Kazimirov
 *
 * File Description:
 *   A generic API for accessing heterogeneous storage services.
 *
 */

/// @file netstorage_api.hpp
///

#include "json_over_uttp.hpp"
#include "srv_connections.hpp"
#include "netservice_api_expt.hpp"

#include <corelib/ncbitime.hpp>

#include <functional>


BEGIN_NCBI_SCOPE


struct SNetStorageObjectImpl;           ///< @internal
struct SNetStorageImpl;                 ///< @internal
struct SNetStorageByKeyImpl;            ///< @internal
struct SNetStorageObjectInfoImpl;       ///< @internal
class  CNetStorageObjectLoc;            ///< @internal


/** @addtogroup NetStorage
 *
 * @{
 */

/// Exception class for use by CNetStorage, CNetStorageByKey,
/// and CNetStorageObject
///
class NCBI_XCONNECT_EXPORT CNetStorageException : public CNetServiceException
{
public:
    enum EErrCode {
        eInvalidArg,    ///< Caller passed invalid arguments to the API
        eNotExists,     ///< Illegal op applied to non-existent object
        eAuthError,     ///< Authentication error (e.g. no FileTrack API key)
        eIOError,       ///< I/O error encountered while performing an op
        eServerError,   ///< NetStorage server error
        eTimeout,       ///< Timeout encountered while performing an op
        eExpired,       ///< Object has expired on server
        eNotSupported,  ///< Feature is not supported
        eInterrupted,   ///< Operation has been interrupted
        eUnknown        ///< Unknown error
    };
    virtual const char* GetErrCodeString() const override;
    NCBI_EXCEPTION_DEFAULT(CNetStorageException, CNetServiceException);
};

/// Enumeration that indicates the current location of the object.
enum ENetStorageObjectLocation {
    eNFL_Unknown,
    eNFL_NotFound,
    eNFL_NetCache,
    eNFL_FileTrack
};

/// Detailed information about a CNetStorage object.
/// This structure is returned by CNetStorageObject::GetInfo().
class NCBI_XCONNECT_EXPORT CNetStorageObjectInfo
{
    NCBI_NET_COMPONENT(NetStorageObjectInfo);

    /// Return a ENetStorageObjectLocation constant that corresponds to the
    /// storage back-end where the object currently resides. If the
    /// object cannot be found in any of the predictable locations,
    /// eNFL_NotFound is returned.  If the object is found, the
    /// returned value reflects the storage back-end that supplied
    /// the rest of information about the object.
    ENetStorageObjectLocation GetLocation() const;

    /// Return a JSON object containing the fields of the object ID.
    /// A valid value is returned even if GetLocation() == eNFL_NotFound.
    CJsonNode GetObjectLocInfo() const;

    /// Return object creation time reported by the storage back-end.
    /// @note Valid only if GetLocation() != eNFL_NotFound.
    CTime GetCreationTime() const;

    /// Return object size in bytes.
    /// @note Valid only if GetLocation() != eNFL_NotFound.
    Uint8 GetSize() const;

    /// Return a JSON object containing storage-specific information
    /// about the object.
    /// @note Valid only if GetLocation() != eNFL_NotFound.
    CJsonNode GetStorageSpecificInfo() const;

    /// Pack the whole structure in a single JSON object.
    CJsonNode ToJSON();
};

/// Blob storage allocation and access strategy
enum ENetStorageFlags {
    fNST_NetCache   = (1 << 0), ///< Use NetCache as the primary storage
    fNST_FileTrack  = (1 << 1), ///< Use FileTrack as the primary storage

    /**@{ @internal */ fNST_V1, fNST_V2 = (fNST_V1 - 1) << 1, /**@}*/
    fNST_Fast       = fNST_NetCache,
    fNST_Persistent = fNST_FileTrack,
    fNST_AnyLoc     = (fNST_V2 - 1),  ///< Any location (all location bits are set)

    fNST_Movable    = (fNST_V2 << 0), ///< Allow the object to move between storages
    fNST_Cacheable  = (fNST_V2 << 1), ///< Has no effect at the moment
    fNST_NoMetaData = (fNST_V2 << 2), ///< Do not use NetStorage relational database
                                      ///< to track ownership & changes. Attributes
                                      ///< and querying will also be disabled.

    /**@{ @internal */ fNST_V3, fNST_V4 = (fNST_V3 - 1) << 1, /**@}*/
    fNST_AnyAttr    = (fNST_V4 - 1) ^ fNST_AnyLoc
};
typedef unsigned TNetStorageFlags;  ///< Bitwise OR of ENetStorageFlags



/////////////////////////////////////////////////////////////////////////////
///
/// Basic network-based data object I/O
///
/// Sequential I/O only
/// Can switch between reading and writing but only explicitly, using Close()
///
/// @warning Different I/O intefaces cannot be used at the same time.
/// E.g. calls to Write(buf) and Write(string) cannot be mixed.
/// Different IO can be used after calling Close().
///
/// @warning Once I/O started non-I/O methods cannot be called until Close() is called.
/// E.g. calls to Write() and GetSize() cannot be mixed.
///

class NCBI_XCONNECT_EXPORT CNetStorageObject
{
    NCBI_NET_COMPONENT(NetStorageObject);

    /// Return object locator
    string GetLoc(void) const;

    /// Read no more than 'buf_size' bytes of the object contents
    /// (starting at the current position)
    ///
    /// @param buffer
    ///  Pointer to the receiving buffer. NULL means to skip bytes (advance
    ///  the current object reading position by 'buf_size' bytes).
    /// @param buf_size
    ///  Number of bytes to attempt to read (or to skip)
    /// @return
    ///  Number of bytes actually read into the receiving buffer (or skipped)
    ///
    /// @throw CNetStorageException
    ///  On any error -- and only if no data at all has been actually read.
    ///  Also, if 'CNetStorageObject' is in writing mode.
    ///
    size_t Read(void* buffer, size_t buf_size);

    /// Read object (starting at the current position) and put the read data
    /// into a string
    ///
    /// @param data
    ///  The string in which to store the read data
    ///
    /// @throw CNetStorageException
    ///  If unable to read ALL of the data, or if 'CNetStorageObject'
    ///  is in writing mode
    ///
    void Read(string* data);

    /// Get a reference to the IReader interface for reading the data
    /// stream of this object.
    ///
    /// The returned reference can be fed to a CRStream object to turn
    /// it into an std::istream. In this case, please make sure the
    /// CRWStreambuf::fOwnReader flag is NOT used. It is, however,
    /// better to use the GetRWStream() method for this purpose.
    ///
    /// @see GetRWStream()
    ///
    IReader& GetReader();

    /// Check if the last Read() has hit EOF
    ///
    /// @return
    ///  TRUE if if the last Read() has hit EOF;  FALSE otherwise
    ///
    /// @throw CNetStorageException
    ///  If the object doesn't exist, or if 'CNetStorageObject'
    ///  is in writing mode
    ///
    bool Eof(void);

    /// Write data to the object (starting at the current position)
    ///
    /// @param buffer
    ///  Pointer to the data to write
    /// @param buf_size
    ///  Data length in bytes
    ///
    /// @throw CNetStorageException
    ///  If unable to write ALL of the data, or if CNetStorageObject
    ///  is in reading mode
    ///
    void Write(const void* buffer, size_t buf_size);

    /// Get a reference to the IWriter interface for storing data into
    /// this object.
    ///
    /// The returned reference can be fed to a CWStream object to turn
    /// it into an std::ostream. In this case, please make sure the
    /// CRWStreambuf::fOwnWriter flag is NOT used. It is, however,
    /// better to use the GetRWStream() method for this purpose.
    ///
    /// @note The IEmbeddedStreamWriter::Close() method must be
    /// explicitly called in order to catch the errors that may
    /// happen during object finalization.
    ///
    /// @see GetRWStream()
    ///
    IEmbeddedStreamWriter& GetWriter();

    /// Get an iostream instance for reading the data stream of this
    /// NetStorage object as well as storing data into this object.
    /// The returned iostream object must be deleted by the caller.
    ///
    /// @warning Empty writes are ignored by the returned iostream instance!
    ///
    CNcbiIostream* GetRWStream();

    /// Write string to the object (starting at the current position)
    ///
    /// @param data
    ///  Data to write to the object
    ///
    /// @throw CNetStorageException
    ///  If unable to write ALL of the data, or if CNetStorageObject
    ///  is in reading mode
    ///
    void Write(const string& data);

    /// Finalize and close the current object stream.
    /// If the operation is successful, then the state (including the current
    /// position, if applicable) of the 'CNetStorageObject' is reset, and
    /// you can start reading from (or writing to) the object all
    /// anew, as if the 'CNetStorageObject' object had just been
    /// created by 'CNetStorage'.
    ///
    /// @throw CNetStorageException
    ///  If cannot finalize writing operations
    void Close(void);

    /// Return size of the object
    ///
    /// @return
    ///  Size of the object in bytes
    ///
    /// @throw CNetStorageException
    ///  On any error (including if the object does not yet exist)
    ///
    Uint8 GetSize(void);

    /// Get list of names of all attributes set on this object.
    /// To get value of particular attribute, GetAttribute should be called.
    ///
    /// @throw If the underlying storage does not support attributes,
    /// a CNetStorageException will be thrown.
    ///
    typedef list<string> TAttributeList;
    TAttributeList GetAttributeList() const;

    /// Get the current value of the specified attribute. Attribute
    /// values are not cached by this method.
    ///
    /// @throw If the underlying storage does not support attributes,
    /// a CNetStorageException will be thrown.
    ///
    string GetAttribute(const string& attr_name) const;

    /// Set the new value for the specified attribute.
    ///
    /// @throw If the underlying storage does not support attributes,
    /// a CNetStorageException will be thrown.
    ///
    void SetAttribute(const string& attr_name, const string& attr_value);

    /// Return detailed information about the object.
    ///
    /// @return
    ///  A CNetStorageObjectInfo object. If the object is not found, a
    ///  valid object is returned, which returns eNFL_NotFound for
    ///  GetLocation().
    ///
    /// @throw CNetStorageException on error
    ///
    /// @see CNetStorageObjectInfo
    ///
    CNetStorageObjectInfo GetInfo(void);

    /// Updates expiration on the object.
    ///
    /// @param ttl
    ///  Expiration timeout, may be infinite
    /// 
    /// @throw If the underlying storage does not support expiration,
    /// a CNetStorageException will be thrown.
    ///
    void SetExpiration(const CTimeout& ttl);
};


/// Progress callback
///
/// @param CJsonNode
///  progress info (depends on operation)
///
typedef function<void(CJsonNode)> TNetStorageProgressCb;


/// Result returned by Remove() methods
///
/// @see CNetStorage::Remove(), CNetStorageByKey::Remove()
///
enum ENetStorageRemoveResult
{
    eNSTRR_NotFound,    ///< Removing failed due to object not found
    eNSTRR_Removed      ///< Object has been removed successfully
};


/////////////////////////////////////////////////////////////////////////////
///
/// Network BLOB storage API
///

class NCBI_XCONNECT_EXPORT CNetStorage
{
    NCBI_NET_COMPONENT_IMPL(NetStorage);

    /// Construct a CNetStorage object
    ///
    /// @param init_string
    ///  Initialization string that contains client identification,
    ///  network service locations, etc.
    ///  The string must be a sequence of apersand-separated pairs of
    ///  attribute names and their values. Within each pair, the name
    ///  and the value of the attribute must be separated by the equality
    ///  sign, and the value must be URL-encoded.
    ///  The following attributes are recoginzed:
    ///  * namespace  - Application domain. This parameter is required.
    ///  * client     - Application name. If omitted, generated from
    ///                 the executable pathname.
    ///  * nst        - NetStorage server address or LBSM service name
    ///                 pointing to a group of NetStorage servers.
    ///  * metadata   - Whether to request or disable the metadata DB.
    ///                 Valid options:
    ///                 - required    - Database access must be enabled
    ///                                 for the specified NetStorage service
    ///                                 name in the server configuration.
    ///                 - disabled    - Database access is disabled for
    ///                                 all operations.
    ///                 - monitoring  - Read-only access to the metadata
    ///                                 database.
    ///  * nc         - NetCache service name or server address.
    ///  * cache      - Synonym for "namespace".
    ///  Example: "client=MyApp&nst=NST_Test&nc=NC_MyApp_TEST&cache=myapp"
    ///
    /// @param default_flags
    ///  Default storage preferences for the created objects.
    ///
    explicit CNetStorage(const string& init_string,
            TNetStorageFlags default_flags = 0);

    /// Create new NetStorage object.
    /// The physical storage is allocated during the first
    /// CNetStorageObject::Write() operation.
    ///
    /// @param flags
    ///  Combination of flags that defines object location (storage) and caching
    /// @return
    ///  New CNetStorageObject
    ///
    CNetStorageObject Create(TNetStorageFlags flags = 0);

    /// Open an existing NetStorage object for reading.
    /// @note
    ///  The object is not checked for existence until
    ///  CNetStorageObject::Read() is called
    ///
    /// @param object_loc
    ///  File to open
    /// @return
    ///  New CNetStorageObject
    ///
    CNetStorageObject Open(const string& object_loc);

    /// Relocate a object according to the specified combination of flags
    ///
    /// @param object_loc
    ///  An existing object to relocate
    /// @param flags
    ///  Combination of flags that defines the new object location (storage)
    /// @param cb
    ///  Relocation progress callback (@see TNetStorageProgressCb).
    ///  CJsonNode input argument (object) will contain two subnodes:
    ///  number 'BytesRelocated' and string 'Message'
    /// @return
    ///  New object ID that fully reflects the new object location.
    ///  If possible, this new ID should be used for further access to the
    ///  object. Note however that its original ID still can be used as well.
    ///
    string Relocate(const string& object_loc, TNetStorageFlags flags,
            TNetStorageProgressCb cb = TNetStorageProgressCb());

    /// Check if the object addressed by 'object_loc' exists.
    ///
    /// @param object_loc
    ///  File to check for existence
    /// @return
    ///  TRUE if the object exists; FALSE otherwise
    ///
    bool Exists(const string& object_loc);

    /// Remove the object addressed by 'object_loc'.
    ///
    /// @param object_loc
    ///  File to remove
    ///
    ENetStorageRemoveResult Remove(const string& object_loc);
};



/////////////////////////////////////////////////////////////////////////////
///
/// Network BLOB storage API -- with access by user-defined keys
///

class NCBI_XCONNECT_EXPORT CNetStorageByKey
{
    NCBI_NET_COMPONENT_IMPL(NetStorageByKey);

    /// Construct a CNetStorageByKey object.
    ///
    /// @param init_string
    ///  Initialization string that contains client identification,
    ///  network service locations, etc.
    ///  The string must be a sequence of apersand-separated pairs of
    ///  attribute names and their values. Within each pair, the name
    ///  and the value of the attribute must be separated by the equality
    ///  sign, and the value must be URL-encoded.
    ///  The following attributes are recoginzed:
    ///  * namespace  - Namespace name, within which the keys passed
    ///                 to the methods of this class must be unique.
    ///                 This parameter is required.
    ///  * domain     - Synonym for "namespace".
    ///  * client     - Application name.
    ///  * nst        - NetStorage server address or LBSM service name
    ///                 pointing to a group of NetStorage servers.
    ///  Example: "client=MyApp&nst=NST_Test&namespace=myapp"
    ///
    /// @param default_flags
    ///  Default storage preferences for objects created by this object.
    ///
    explicit CNetStorageByKey(const string& init_string,
            TNetStorageFlags default_flags = 0);

    /// Create a new object or open an existing object using the supplied
    /// unique key. The returned object object can be either written
    /// or read.
    ///
    /// @param key
    ///  User-defined unique key that, in combination with the domain name
    ///  specified in the constructor, can be used to address the object
    /// @param flags
    ///  Combination of flags that defines object location and caching.
    /// @return
    ///  New CNetStorageObject.
    ///
    CNetStorageObject Open(const string& unique_key,
            TNetStorageFlags flags = 0);

    /// Relocate a object according to the specified combination of flags
    ///
    /// @param flags
    ///  Combination of flags that defines the new object location
    /// @param old_flags
    ///  Combination of flags that defines the current object location
    /// @param cb
    ///  Relocation progress callback (@see TNetStorageProgressCb).
    ///  CJsonNode input argument (object) will contain two subnodes:
    ///  number 'BytesRelocated' and string 'Message'
    /// @return
    ///  A unique full object ID that reflects the new object location (storage)
    ///  and which can be used with CNetStorage::Open(). Note however that the
    ///  original ID 'unique_key' still can be used as well.
    ///
    string Relocate(const string&    unique_key,
                    TNetStorageFlags flags,
                    TNetStorageFlags old_flags = 0,
                    TNetStorageProgressCb cb = TNetStorageProgressCb());

    /// Check if a object with the specified key exists in the storage
    /// hinted by 'flags'
    ///
    /// @return TRUE if the object exists; FALSE otherwise
    ///
    bool Exists(const string& key, TNetStorageFlags flags = 0);

    /// Remove a object addressed by a key and a set of flags
    ///
    /// @param key
    ///  User-defined unique key that, in combination with the domain name
    ///  specified in the constructor, addresses the object
    /// @param flags
    ///  Combination of flags that hints on the current object location
    ///
    ENetStorageRemoveResult Remove(const string& key, TNetStorageFlags flags = 0);
};


/* @} */

END_NCBI_SCOPE

#include "impl/netstorage_int.hpp"

#endif  /* CONNECT_SERVICES__NETSTORAGE__HPP */
