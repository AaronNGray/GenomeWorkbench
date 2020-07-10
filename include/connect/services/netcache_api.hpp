#ifndef CONN___NETCACHE_API__HPP
#define CONN___NETCACHE_API__HPP

/*  $Id: netcache_api.hpp 574016 2018-11-05 16:55:15Z sadyrovr $
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
 * Authors:  Anatoliy Kuznetsov, Maxim Didenko, Dmitry Kazimirov
 *
 * File Description:
 *   Net cache client API.
 *
 */

/// @file netcache_api.hpp
/// NetCache client specs.
///

#include "netschedule_api.hpp"
#include "netcache_admin.hpp"
#include "netcache_api_expt.hpp"
#include "netcache_key.hpp"
#include "named_parameters.hpp"

#include <util/simple_buffer.hpp>

#include <corelib/plugin_manager.hpp>
#include <corelib/blob_storage.hpp>


BEGIN_NCBI_SCOPE


/** @addtogroup NetCacheClientParams
 *
 * @{
 */

/// Blob life span in seconds. If zero or greater than the
/// server-side value, then the server-side TTL is used.
/// @see CNetCacheAPI::TBlobTTL
#define nc_blob_ttl CNetCacheAPI::TBlobTTL()

/// Caching mode.
/// @see CNetCacheAPI::TCachingMode for details.
#define nc_caching_mode CNetCacheAPI::TCachingMode()

/// Mirroring mode.
/// @see CNetCacheAPI::EMirroringMode for details.
#define nc_mirroring_mode CNetCacheAPI::TMirroringMode()

/// For blob readers: whether to check if the primary
/// server that stores the blob is still in service.
/// @see CNetCacheAPI::TServerCheck
#define nc_server_check CNetCacheAPI::TServerCheck()

/// For blob writers: whether to advise the readers to check
/// if the primary server that stores the blob is still in service.
/// @see CNetCacheAPI::TServerCheckHint
#define nc_server_check_hint CNetCacheAPI::TServerCheckHint()

/// Blob password. Used to protect the blob when writing;
/// required for reading a password-protected blob.
/// @see CNetCacheAPI::TBlobPassword
#define nc_blob_password CNetCacheAPI::TBlobPassword()

/// The server to use for the operation. Overrides whatever is
/// defined in the constructor or the configuration file.
/// @see CNetCacheAPI::TServerToUse
#define nc_server_to_use CNetCacheAPI::TServerToUse()

/// Pointer to a variable for saving the CNetServer that was last used.
/// Works only with (some of) CNetICacheClient methods.
/// @see CNetCacheAPI::TServerLastUsedPtr
#define nc_server_last_used CNetCacheAPI::TServerLastUsedPtr()

/// Do not read the blob if its age is greater than the specified value.
/// @see CNetCacheAPI::TMaxBlobAge
#define nc_max_age CNetCacheAPI::TMaxBlobAge()

/// A pointer to an unsigned variable where the actual age of the blob
/// must be stored.
/// @see CNetCacheAPI::TActualBlobAgePtr
#define nc_actual_age CNetCacheAPI::TActualBlobAgePtr()

/// Whether to return NetCache keys in CompoundID format.
/// @see CNetCacheAPI::TUseCompoundID
#define nc_use_compound_id CNetCacheAPI::TUseCompoundID()

/// Whether to run a request through all NetCache servers in the
/// ICache service in an attempt to find the blob.
/// @see CNetCacheAPI::TTryAllServers
#define nc_try_all_servers CNetCacheAPI::TTryAllServers()

/// In ICache mode, override the name of the cache specified
/// in the CNetICacheClient constructor.
/// @see CNetCacheAPI::TCacheName
#define nc_cache_name CNetCacheAPI::TCacheName()

/* @} */

/** @addtogroup NetCacheClient
 *
 * @{
 */

struct SNetCacheAPIImpl;

/// Client API for NetCache server.
///
/// It is undesirable to create objects of this class on the heap
/// because they are essentially smart pointers to the implementation
/// objects allocated internally on the heap.
///
/// @note After sending blob data to a NetCache server,
/// this class waits for a confirmation from the server,
/// and the connection cannot be used before this
/// confirmation is read.
///
///
class NCBI_XCONNECT_EXPORT CNetCacheAPI
{
    NCBI_NET_COMPONENT(NetCacheAPI);

    /// Allows to define caching behavior on a per-call basis.
    enum ECachingMode {
        eCaching_AppDefault,
        eCaching_Disable,
        eCaching_Enable
    };

    /// Mirroring modes. eIfKeyMirrored unconditionally enables
    /// mirroring for blobs that were created in mirroring mode.
    enum EMirroringMode {
        eMirroringDisabled,
        eMirroringEnabled,
        eIfKeyMirrored
    };

    /// Defines how this object must be initialized.
    enum EAppRegistry {
        eAppRegistry
    };

    /// Creates an instance of CNetCacheAPI and initializes
    /// it with parameters read from the application registry.
    /// @param use_app_reg
    ///   Selects this constructor.
    ///   The parameter is not used otherwise.
    /// @param conf_section
    ///   Name of the registry section to look for the configuration
    ///   parameters in.  If empty string is passed, then the section
    ///   name "netcache_api" will be used.
    explicit CNetCacheAPI(EAppRegistry use_app_reg,
            const string& conf_section = kEmptyStr,
            CNetScheduleAPI::TInstance ns_api = NULL);

    /// Constructs a CNetCacheAPI object and initializes it with
    /// parameters read from the specified registry object.
    /// @param reg
    ///   Registry to get the configuration parameters from.
    /// @param conf_section
    ///   Name of the registry section to look for the configuration
    ///   parameters in.  If empty string is passed, then the section
    ///   name "netcache_api" will be used.
    explicit CNetCacheAPI(const IRegistry& reg,
            const string& conf_section = kEmptyStr,
            CNetScheduleAPI::TInstance ns_api = NULL);

    /// Constructs a CNetCacheAPI object and initializes it with
    /// parameters read from the specified configuration object.
    /// @param conf
    ///   A CConfig object to get the configuration parameters from.
    /// @param conf_section
    ///   Name of the configuration section where to look for the
    ///   parameters.  If empty string is passed, then the section
    ///   name "netcache_api" will be used.
    explicit CNetCacheAPI(CConfig* conf,
            const string& conf_section = kEmptyStr,
            CNetScheduleAPI::TInstance ns_api = NULL);

    explicit CNetCacheAPI(const string& client_name,
            CNetScheduleAPI::TInstance ns_api = NULL);

    /// Construct client, working with the specified service
    CNetCacheAPI(const string& service_name, const string& client_name,
            CNetScheduleAPI::TInstance ns_api = NULL);

    /// Named parameters that can be used when calling
    /// CNetCacheAPI methods that accept CNamedParameterList.
    enum ENamedParameterTag {
        eNPT_BlobTTL,
        eNPT_CachingMode,
        eNPT_MirroringMode,
        eNPT_ServerCheck,
        eNPT_ServerCheckHint,
        eNPT_Password,
        eNPT_ServerToUse,
        eNPT_ServerLastUsedPtr,
        eNPT_MaxBlobAge,
        eNPT_ActualBlobAgePtr,
        eNPT_UseCompoundID,
        eNPT_TryAllServers,
        eNPT_CacheName,
    };

    /// Override defaults used by this object.
    ///
    /// @param parameters
    ///    A list of named parameters, for example:
    ///        SetDefaultParameters(use_compound_id = true);
    ///
    /// @warning
    ///    This method is not thread-safe (it cannot be called
    ///    concurrently from multiple threads on the same object).
    ///
    void SetDefaultParameters(const CNamedParameterList* parameters);

    /// Put BLOB to server.  This method is blocking and waits
    /// for a confirmation from NetCache after all data is
    /// transferred.
    ///
    /// @param buf
    ///    Data to be written.
    /// @param size
    ///    Number of bytes to write.
    /// @param optional
    ///    An optional list of named blob creation parameters in the
    ///    form of (param_name = param_value, ...).
    ///    @see NetCacheClientParams
    ///
    /// @return NetCache blob key
    ///
    string PutData(const void* buf, size_t size,
            const CNamedParameterList* optional = NULL);

    /// Put BLOB to server.  This method is blocking, it
    /// waits for a confirmation from NetCache after all
    /// data is transferred. Since blob EOF marker is sent in the
    /// destructor, the blob will not be created until the stream
    /// is deleted.
    ///
    /// @param key
    ///    NetCache key, if empty new key is created
    /// @param optional
    ///    An optional list of named blob creation parameters in the
    ///    form of (param_name = param_value, ...).
    ///    @see NetCacheClientParams
    /// @return
    ///    IEmbeddedStreamWriter* (caller must delete it).
    IEmbeddedStreamWriter* PutData(string* key,
            const CNamedParameterList* optional = NULL);

    /// Update an existing BLOB.  Just like all other PutData
    /// methods, this one is blocking and waits for a confirmation
    /// from NetCache after all data is transferred.
    string PutData(const string& key,
                   const void*   buf,
                   size_t        size,
                   const CNamedParameterList* optional = NULL);

    /// Create a stream object for sending data to a blob.
    /// If the string "key" is empty, a new blob will be created
    /// and its ID will be returned via the "key" parameter.
    /// @note
    ///   The blob will not be available from NetCache until
    ///   the stream is destructed.
    CNcbiOstream* CreateOStream(string& key,
            const CNamedParameterList* optional = NULL);

    /// Check if the BLOB identified by the key "key" exists.
    ///
    /// @param key
    ///    Key of the BLOB to check for existence.
    /// @param optional
    ///    An optional list of named blob creation parameters in the
    ///    form of (param_name = param_value, ...).
    ///    @see NetCacheClientParams
    ///
    /// @return
    ///    True, if the BLOB exists; false otherwise.
    bool HasBlob(const string& blob_id,
            const CNamedParameterList* optional = NULL);

    /// Returns the size of the BLOB identified by the "key" parameter.
    ///
    /// @note
    ///    This updates the blob's expiration time.
    ///
    /// @param key
    ///    The key of the BLOB the size of which to be returned.
    /// @param optional
    ///    An optional list of named blob creation parameters in the
    ///    form of (param_name = param_value, ...).
    ///    @see NetCacheClientParams
    ///
    /// @return
    ///    Size of the BLOB in bytes.
    size_t GetBlobSize(const string& blob_id,
            const CNamedParameterList* optional = NULL);

    /// Get a pointer to the IReader interface to read blob contents.
    /// This is a safe version of the GetData method having the same
    /// signature. Unlike GetData, GetReader will throw an exception
    /// if the requested blob is not found.
    ///
    /// @note
    ///   The Read() method of the returned IReader object is not
    ///   blocking. A reading completion loop is required, see an
    ///   example below.
    ///
    /// @code
    /// size_t blob_size;
    /// unique_ptr<IReader> reader(nc_api.GetReader(key, &blob_size,
    ///         nc_caching_mode = CNetCacheAPI::eCaching_Disable));
    /// size_t bytes_read;
    /// size_t total_bytes_read = 0;
    ///
    /// while (buf_size > 0) {
    ///     ERW_Result rw_res = reader->Read(buf_ptr, buf_size, &bytes_read);
    ///     if (rw_res == eRW_Success) {
    ///         total_bytes_read += bytes_read;
    ///         buf_ptr += bytes_read;
    ///         buf_size -= bytes_read;
    ///     } else if (rw_res == eRW_Eof) {
    ///         break;
    ///     } else {
    ///         NCBI_THROW(CNetServiceException, eCommunicationError,
    ///             "Error while reading BLOB");
    ///     }
    /// }
    ///
    /// return total_bytes_read;
    /// @endcode
    IReader* GetReader(const string& key, size_t* blob_size = NULL,
            const CNamedParameterList* optional = NULL);

    /// Get a pointer to the IReader interface to read a portion of
    /// the blob contents. See the description of GetReader() for details.
    /// The Read() method of the returned IReader interface implementation
    /// is not blocking.
    /// @see CNetCacheAPI::GetReader() for details.
    IReader* GetPartReader(const string& key,
        size_t offset, size_t part_size, size_t* blob_size = NULL,
        const CNamedParameterList* optional = NULL);

    /// Read the blob pointed to by "key" and store its contents
    /// in "buffer". The output string is resized as required.
    ///
    /// @throw CNetCacheException
    ///    Thrown if either the blob was not found or
    ///    a protocol error occurred.
    /// @throw CNetServiceException
    ///    Thrown if a communication error occurred.
    void ReadData(const string& key, string& buffer,
        const CNamedParameterList* optional = NULL);

    /// Read a part of the blob pointed to by "key" and store its contents
    /// in "buffer". The output string is resized as required.
    ///
    /// @throw CNetCacheException
    ///    Thrown if either the blob was not found or
    ///    a protocol error occurred.
    /// @throw CNetServiceException
    ///    Thrown if a communication error occurred.
    void ReadPart(const string& key,
        size_t offset, size_t part_size, string& buffer,
        const CNamedParameterList* optional = NULL);

    /// Retrieve BLOB from server by key.
    //
    /// Caller is responsible for deletion of the IReader* object.
    /// It must be deleted before the destruction of CNetCacheAPI.
    ///
    /// @note
    ///   IReader implementation used here is based on TCP/IP
    ///   sockets; when reading the blob, please remember to check
    ///   IReader::Read return codes, it may not be able to read
    ///   the whole blob in one call because of network delays.
    /// @see CNetCacheAPI::GetReader() for details.
    ///
    /// @param key
    ///    BLOB key to read (returned by PutData)
    /// @param blob_size
    ///    Pointer to the memory location where the size
    ///    of the requested blob will be stored.
    /// @param optional
    ///    An optional list of named blob creation parameters in the
    ///    form of (param_name = param_value, ...).
    ///    @see NetCacheClientParams
    /// @return
    ///    If the requested blob is found, the method returns a pointer
    ///    to the IReader interface for reading the blob contents (the
    ///    caller must delete it). If the blob is not found (that is,
    ///    if it's expired), NULL is returned.
    IReader* GetData(const string& key, size_t* blob_size = NULL,
            const CNamedParameterList* optional = NULL);

    /// Status of GetData() call
    /// @sa GetData
    enum EReadResult {
        eReadComplete, ///< The whole BLOB has been read
        eNotFound,     ///< BLOB not found or error
        eReadPart      ///< Read part of the BLOB (buffer capacity)
    };

    /// Retrieve BLOB from server by key
    ///
    /// @note
    ///    Function waits for enough data to arrive.
    EReadResult GetData(const string&  key,
                        void*          buf,
                        size_t         buf_size,
                        size_t*        n_read    = 0,
                        size_t*        blob_size = 0,
                        const CNamedParameterList* optional = NULL);

    /// Retrieve BLOB from server by key
    /// This method retrieves BLOB size, allocates memory and gets all
    /// the data from the server.
    ///
    /// Blob size and binary data is placed into blob_to_read structure.
    /// Do not use this method if you are not sure you have memory
    /// to load the whole BLOB.
    ///
    /// @return
    ///    eReadComplete if BLOB found (eNotFound otherwise)
    EReadResult GetData(const string& key, CSimpleBuffer& buffer,
            const CNamedParameterList* optional = NULL);

    /// Create an istream object for reading blob data.
    /// @throw CNetCacheException
    ///    The requested blob does not exist.
    CNcbiIstream* GetIStream(const string& key, size_t* blob_size = NULL,
            const CNamedParameterList* optional = NULL);

    /// Remove BLOB by key
    void Remove(const string& blob_id,
            const CNamedParameterList* optional = NULL);

    /// Return a CNetServerMultilineCmdOutput object for reading
    /// meta information about the specified blob.
    ///
    /// @note
    ///    This does not update the blob's expiration time.
    CNetServerMultilineCmdOutput GetBlobInfo(const string& blob_id,
            const CNamedParameterList* optional = NULL);

    /// Print meta information about the specified blob.
    void PrintBlobInfo(const string& blob_key,
            const CNamedParameterList* optional = NULL);

    void ProlongBlobLifetime(const string& blob_key, unsigned ttl,
            const CNamedParameterList* optional = NULL);

    CNetCacheAdmin GetAdmin();

    CNetService GetService();

    void SetCommunicationTimeout(const STimeout& to)
        {GetService().GetServerPool().SetCommunicationTimeout(to);}

    /// Named parameter type for blob life span in seconds.
    /// If zero or greater than the server-side value,
    /// then the server-side TTL is used.
    /// This type of parameter can be defined through the nc_blob_ttl
    /// macro substitution, for example:
    ///     nc_blob_ttl = 600
    /// @see nc_blob_ttl
    typedef CNamedParameter<unsigned, eNPT_BlobTTL> TBlobTTL;

    /// Named parameter type for caching mode.
    /// This type of parameter can be defined through the
    /// nc_caching_mode macro substitution, for example:
    ///     nc_caching_mode = CNetCacheAPI::eCaching_Disable
    /// @see nc_caching_mode, ECachingMode
    typedef CNamedParameter<ECachingMode, eNPT_CachingMode> TCachingMode;

    /// Named parameter type for mirroring mode.
    /// This type of parameter can be defined through the
    /// nc_mirroring_mode macro substitution, for example:
    ///     nc_mirroring_mode = CNetCacheAPI::eMirroringEnabled
    /// @see nc_mirroring_mode, EMirroringMode
    typedef CNamedParameter<EMirroringMode, eNPT_MirroringMode> TMirroringMode;

    /// Named parameter type for whether blob reading methods
    /// must check if the primary server that stores the blob
    /// is still in service.
    /// This type of parameter can be defined through the nc_server_check
    /// macro substitution, for example:
    ///     nc_server_check = eOn
    /// @see nc_server_check
    typedef CNamedParameter<ESwitch, eNPT_ServerCheck> TServerCheck;

    /// Named parameter type for whether to advise the readers to check
    /// if the primary server that stores the blob is still in service.
    /// This type parameter only has effect during blob creation and
    /// can be defined through the nc_server_check_hint macro substitution,
    /// for example:
    ///     nc_server_check_hint = true
    /// @see nc_server_check_hint
    typedef CNamedParameter<bool, eNPT_ServerCheckHint> TServerCheckHint;

    /// Named parameter type for blob password. A blob created with a
    /// password cannot be read without specifying the same password.
    /// This type of parameter can be defined through the nc_blob_password
    /// macro substitution, for example:
    ///     nc_blob_password = "secret"
    /// @see nc_blob_password
    typedef CNamedParameter<string, eNPT_Password> TBlobPassword;

    /// Named parameter type for specifying the server to use
    /// for the operation. Overrides whatever is defined in the
    /// constructor or the configuration file.
    /// This type of parameter can be defined through the nc_server_to_use
    /// macro substitution, for example:
    ///     nc_server_to_use = netserver_object
    /// @see nc_server_to_use
    typedef CNamedParameter<CNetServer::TInstance,
            eNPT_ServerToUse> TServerToUse;

    /// Named parameter type to supply a pointer to a variable for saving
    /// the CNetServer object that was last used. This parameter works
    /// only with (some of) CNetICacheClient methods.
    /// This type of parameter can be defined through the nc_server_last_used
    /// macro substitution, for example:
    ///     nc_server_last_used = &netserver_object
    /// @see nc_server_last_used
    typedef CNamedParameter<CNetServer*,
            eNPT_ServerLastUsedPtr> TServerLastUsedPtr;

    /// Named parameter type to specify that the blob should not be read
    /// if its age in seconds is greater than the specified value.
    /// This type of parameter can be defined through the nc_max_age
    /// macro substitution, for example:
    ///     nc_max_age = 3600
    /// @see nc_max_age
    typedef CNamedParameter<unsigned, eNPT_MaxBlobAge> TMaxBlobAge;

    /// Named parameter type for a pointer to a variable where the actual
    /// age of the blob is to be stored.
    /// This type of parameter can be defined through the nc_actual_age
    /// macro substitution, for example:
    ///     nc_actual_age = &blob_age
    /// @see nc_actual_age
    typedef CNamedParameter<unsigned*, eNPT_ActualBlobAgePtr> TActualBlobAgePtr;

    /// Named parameter type to define whether to return NetCache
    /// keys in CompoundID format.
    /// This type of parameter can be defined through the
    /// nc_use_compound_id macro substitution, for example:
    ///     nc_use_compound_id = true
    /// @see nc_use_compound_id
    typedef CNamedParameter<bool, eNPT_UseCompoundID> TUseCompoundID;

    /// Named parameter type to define whether to run a request through all
    /// NetCache servers in the ICache service in an attempt to find the blob.
    /// This type of parameter can be defined through the
    /// nc_try_all_servers macro substitution, for example:
    ///     nc_try_all_servers = true
    /// @see nc_try_all_servers
    typedef CNamedParameter<bool, eNPT_TryAllServers> TTryAllServers;

    /// In ICache mode, override the name of the cache specified
    /// in the CNetICacheClient constructor.
    /// This type of parameter can be defined through the
    /// nc_cache_name macro substitution, for example:
    ///     nc_cache_name = "appdata"
    /// @see nc_cache_name
    typedef CNamedParameter<string, eNPT_CacheName> TCacheName;

    /// @internal
    CCompoundIDPool GetCompoundIDPool();
    /// @internal
    void SetCompoundIDPool(CCompoundIDPool::TInstance compound_id_pool);
};

NCBI_DECLARE_INTERFACE_VERSION(SNetCacheAPIImpl, "xnetcacheapi", 1, 1, 0);

extern NCBI_XCONNECT_EXPORT const char* const kNetCacheAPIDriverName;

void NCBI_XCONNECT_EXPORT NCBI_EntryPoint_xnetcacheapi(
     CPluginManager<SNetCacheAPIImpl>::TDriverInfoList&   info_list,
     CPluginManager<SNetCacheAPIImpl>::EEntryPointRequest method);


/// CBlobStorage_NetCache -- NetCache-based implementation of IBlobStorage
///
class NCBI_XCONNECT_EXPORT CBlobStorage_NetCache : public IBlobStorage
{
public:
    CBlobStorage_NetCache();

    /// Create Blob Storage
    /// @param[in] nc_client
    ///  NetCache client - an instance of CNetCacheAPI.
    CBlobStorage_NetCache(CNetCacheAPI::TInstance nc_client) :
        m_NCClient(nc_client) {}

    virtual ~CBlobStorage_NetCache();


    virtual bool IsKeyValid(const string& str);

    /// Get a blob content as a string
    ///
    /// @param[in] blob_key
    ///    Blob key to read
    virtual string        GetBlobAsString(const string& data_id);

    /// Get an input stream to a blob
    ///
    /// @param[in] blob_key
    ///    Blob key to read
    /// @param[out] blob_size_ptr
    ///    if blob_size_ptr if not NULL the size of a blob is returned
    /// @param[in] lock_mode
    ///    Blob locking mode
    virtual CNcbiIstream& GetIStream(const string& data_id,
                                     size_t* blob_size_ptr = 0,
                                     ELockMode lock_mode = eLockWait);

    /// Get an output stream to a blob
    ///
    /// @param[in,out] blob_key
    ///    Blob key to read. If a blob with a given key does not exist
    ///    an key of a newly create blob will be assigned to blob_key
    /// @param[in] lock_mode
    ///    Blob locking mode
    virtual CNcbiOstream& CreateOStream(string& data_id,
                                        ELockMode lock_mode = eLockNoWait);

    /// Create an new blob
    ///
    /// @return
    ///     Newly create blob key
    virtual string CreateEmptyBlob();

    /// Delete a blob
    ///
    /// @param[in] blob_key
    ///    Blob key to read
    virtual void DeleteBlob(const string& data_id);

    /// Close all streams and connections.
    virtual void Reset();

    CNetCacheAPI GetNetCacheAPI() const {return m_NCClient;}

private:
    CNetCacheAPI m_NCClient;

    unique_ptr<CNcbiIstream> m_IStream;
    unique_ptr<CNcbiOstream> m_OStream;

    CBlobStorage_NetCache(const CBlobStorage_NetCache&);
    CBlobStorage_NetCache& operator=(CBlobStorage_NetCache&);
};

/* @} */

END_NCBI_SCOPE

#endif  /* CONN___NETCACHE_API__HPP */
