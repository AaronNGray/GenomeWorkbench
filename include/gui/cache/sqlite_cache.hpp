#ifndef GUI_CACHE___SQLITE_CACHE__HPP
#define GUI_CACHE___SQLITE_CACHE__HPP

/*  $Id: sqlite_cache.hpp 43271 2019-06-06 18:32:38Z evgeniev $
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
 * Author: Mike DiCuccio
 *
 * File Description:  SQLite3 based ICache interface
 *
 */

/// @file sqlite3_cache.hpp
/// ICache interface implementation on top of SQLite3

#include <corelib/ncbiexpt.hpp>
#include <util/cache/icache.hpp>
#include <util/sync_queue.hpp>
#include <util/simple_buffer.hpp>

/*
 * Export specifier for library ncbi_xcache_sqlite3
 */

#if defined(NCBI_OS_MSWIN)  &&  defined(NCBI_DLL_BUILD)

#ifdef NCBI_SQLITE3_CACHE_EXPORTS
#  define NCBI_SQLITE3_CACHE_EXPORT __declspec(dllexport)
#else
#  define NCBI_SQLITE3_CACHE_EXPORT __declspec(dllimport)
#endif

#else

#  define NCBI_SQLITE3_CACHE_EXPORT

#endif


struct sqlite3;

BEGIN_NCBI_SCOPE

/** @addtogroup DbAPI
 *
 * @{
 */

/// Register NCBI_EntryPoint_SQLITE3_BlobCache
void SQLITE3_Register_Cache(void);

/// SQLITE3 ICache exception
///
class NCBI_SQLITE3_CACHE_EXPORT CSQLITE3_ICacheException : public CException
{
public:
    enum EErrCode {
        eUnknown,
        eInitError,
        eNotImplemented
    };

    virtual const char* GetErrCodeString(void) const;

    NCBI_EXCEPTION_DEFAULT(CSQLITE3_ICacheException, CException);
};



class CSQLITE3_Statement;

/// SQLITE3 cache implementation.
///
/// Class implements ICache (BLOB cache) interface using SQLITE3 (RDBMS)
/// This implementation stores BLOBs in the RDBMS and intended to play
/// role of an enterprise wide BLOB storage
///
class NCBI_SQLITE3_CACHE_EXPORT CSQLITE3_Cache : public ICache
{
public:
    enum {
        kDefaultTimestampPolicy = fTimeStampOnCreate | fTimeStampOnRead |
                                  fTrackSubKey | fPurgeOnStartup
    };

    CSQLITE3_Cache();
    virtual ~CSQLITE3_Cache();

    /// Open cache. Does not take IConnection ownership.
    /// Connection should be already logged into the destination server
    ///
    /// Implementation uses temporary files to keep local BLOBs.
    /// If you temp directory is not specified system default is used
    ///
    /// @param database
    ///    DB name with path
    /// @param remove
    ///    Delete the database first?
    void Open(const string& database, bool remove=false);

    /// Vacuum the database (should be open first)
    /// @return true if success
    bool Vacuum();

    /// @return Size of the intermediate BLOB memory buffer
    unsigned GetMemBufferSize() const;

    /// Set size of the intermidiate BLOB memory buffer
    void SetMemBufferSize(unsigned int buf_size);

    /// @name ICache interface
    /// @{

    virtual void SetTimeStampPolicy(TTimeStampFlags policy,
                                    unsigned int    timeout,
                                    unsigned int    max_timeout = 0);
    virtual TTimeStampFlags GetTimeStampPolicy() const;

    virtual int GetTimeout() const;

    virtual bool IsOpen() const;

    virtual void SetVersionRetention(EKeepVersions policy);

    virtual EKeepVersions GetVersionRetention() const;

    virtual void Store(const string&  key,
                       int            version,
                       const string&  subkey,
                       const void*    data,
                       size_t         size,
                       unsigned int   time_to_live = 0,
                       const string&  owner = kEmptyStr);

    virtual size_t GetSize(const string&  key,
                           int            version,
                           const string&  subkey);

    virtual void GetBlobOwner(const string&  key,
                              int            version,
                              const string&  subkey,
                              string*        owner);

    virtual bool Read(const string& key,
                      int           version,
                      const string& subkey,
                      void*         buf,
                      size_t        buf_size);

    virtual IReader* GetReadStream(const string&  key,
                                   int            version,
                                   const string&  subkey);

    virtual IReader* GetReadStream(const string&  key,
                                   const string&  subkey,
                                   int*           version,
                                   EBlobValidity* validity);

    virtual void SetBlobVersionAsValid(const string&  key,
                                       const string&  subkey,
                                       int            version);

    virtual void GetBlobAccess(const string&     key,
                               int               version,
                               const string&     subkey,
                               SBlobAccessDescr*  blob_descr);

    /// Specifics of this IWriter implementation is that IWriter::Flush here
    /// cannot be called twice, because it finalises transaction
    /// Also you cannot call Write after Flush...
    /// All this is because MSSQL (and Sybase) wants to know exact
    /// BLOB size before writing it to the database
    /// Effectively IWriter::Flush in this case works as "Close"...
    ///
    virtual IWriter* GetWriteStream(const string&   key,
                                    int             version,
                                    const string&   subkey,
                                    unsigned int    time_to_live = 0,
                                    const string&   owner = kEmptyStr);

    virtual void Remove(const string& key);

    virtual void Remove(const string&    key,
                        int              version,
                        const string&    subkey);

    virtual time_t GetAccessTime(const string&  key,
                                 int            version,
                                 const string&  subkey);

    virtual bool HasBlobs(const string&  key,
                          const string&  subkey);

    virtual void Purge(time_t           access_timeout);

    virtual void Purge(const string&    key,
                       const string&    subkey,
                       time_t           access_timeout);

    virtual bool SameCacheParams(const TCacheParams* params) const;
    virtual string GetCacheName(void) const;

    virtual TFlags GetFlags() { return (TFlags) 0; }
    virtual void SetFlags(TFlags flags) {}

    /// @}

    void StoreSynchronous(const string&  key,
                          int            version,
                          const string&  subkey,
                          const void*    data,
                          size_t         size);

    /// Get DB name/path
    const string& GetDatabase() const { return m_Database; }

private:

    /// delayed write request object
    struct SWriteRequest : public CObject
    {
        string key;
        int version;
        string subkey;
        int timestamp;
        CSimpleBuffer buffer;
    };

    /// queue for delayed writes
    typedef CSyncQueue< CRef<SWriteRequest> > TWriteQueue;
    TWriteQueue m_WriteQueue;

    /// thread for delayed writes
    class CWriterThread : public CThread
    {
    public:
        CWriterThread(CSQLITE3_Cache& cache, TWriteQueue& write_q);
        void* Main();

        /// Queue a request to stop the background writer
        /// Asyncronous! Thread may not stop yet when it gets back from this call.
        void Stop();
    private:
        CWriterThread(const CWriterThread&);
        CWriterThread& operator=(const CWriterThread&);

    private:
        CRef<SWriteRequest> m_StopRequest;
        CSQLITE3_Cache& m_Cache;
        TWriteQueue& m_WriteQueue;
    };

    CRef<CWriterThread> m_WriterThread;

private:

    bool  m_ReadOnly;

    unsigned int m_Timeout;
    TTimeStampFlags m_TimeStampFlag;
    EKeepVersions m_VersionFlag;

    /// filename of the database
    string m_Database;

    CMutex m_Mutex;

    /// sqlite3 database handle
    typedef struct sqlite3 TSqliteDb;
    TSqliteDb* m_DB;

    /// precompiled statements
    /// these are used to speed up time-critical accesses
    ///
    /// AK: looks like keeping this statements for a long time is incorrect 
    /// because it also holds locks and prevents multi-process access to the database :(
    /// 

    unique_ptr<CSQLITE3_Statement> m_Stmt_Store;
    unique_ptr<CSQLITE3_Statement> m_Stmt_HasBlobs_key;
    unique_ptr<CSQLITE3_Statement> m_Stmt_HasBlobs_key_subkey;
    unique_ptr<CSQLITE3_Statement> m_Stmt_GetBlobAccess;
    unique_ptr<CSQLITE3_Statement> m_Stmt_GetReadStream;
    unique_ptr<CSQLITE3_Statement> m_Stmt_SetTimestamp;

    void x_SetTimestamp(const string& key, int version, const string& subkey);

private:
    CSQLITE3_Cache(const CSQLITE3_Cache&);
    CSQLITE3_Cache& operator=(const CSQLITE3_Cache&);
};

/* @} */

extern NCBI_SQLITE3_CACHE_EXPORT const string kSQLITE3_BlobCacheDriverName;

END_NCBI_SCOPE

#endif
