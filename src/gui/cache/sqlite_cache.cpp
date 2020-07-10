/*  $Id: sqlite_cache.cpp 43271 2019-06-06 18:32:38Z evgeniev $
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
 * File Description:  SQLITE3 based ICache interface
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbistr.hpp>
#include <corelib/ncbimtx.hpp>
#include <corelib/ncbitime.hpp>
#include <corelib/ncbifile.hpp>
#include <util/simple_buffer.hpp>

#include <gui/cache/sqlite_cache.hpp>

#include <sqlite3.h>


BEGIN_NCBI_SCOPE


/////////////////////////////////////////////////////////////////////////////

CSQLITE3_Cache::CWriterThread::CWriterThread(CSQLITE3_Cache& cache,
                                             CSQLITE3_Cache::TWriteQueue& write_q)
    : m_StopRequest(new SWriteRequest),
      m_Cache(cache),
      m_WriteQueue(write_q)
{
}

void CSQLITE3_Cache::CWriterThread::Stop()
{
    LOG_POST(Info << "CSQLITE3_Cache::CWriterThread: shutting down writer thread...");
    m_WriteQueue.Push(m_StopRequest);
}


void* CSQLITE3_Cache::CWriterThread::Main()
{
    m_Cache.Purge(m_Cache.GetTimeout());

    bool vac_ok = m_Cache.Vacuum();
    if (!vac_ok) {
        // re-open db with file removal
        LOG_POST("SQLLite Vacuum failed use recovery procedure");
        m_Cache.Open(m_Cache.GetDatabase(), true);
    }
    

    bool done = false;
    while ( !done ) {
        CRef<SWriteRequest> req;
        try {
            req = m_WriteQueue.Pop();
        }
        catch (CException& ex) {
            LOG_POST(Error << ex);
        }

        if ( !req ) {
            continue;
        }

        if (m_StopRequest == req) {
            done = true;
            continue;
        }

        //
        // write it!
        //
        m_Cache.StoreSynchronous(req->key, req->version, req->subkey,
                                 req->buffer.data(), req->buffer.size());
/*
#ifdef _DEBUG
        size_t size = m_WriteQueue.GetSize();
        if (size > 0) {
            _TRACE("CSQLITE3_Cache::CWriterThread::Main(): "
                << size << " requests pending...");
        }
#endif
*/
    }
    LOG_POST(Info << "CSQLITE3_Cache::CWriterThread: writer thread stopped");
    return NULL;
}


/////////////////////////////////////////////////////////////////////////////

struct SStats {
    CAtomicCounter bytes_read;
    CAtomicCounter objects_read;
    CAtomicCounter total_time;
};

static SStats s_CacheStats;

/// Add BLOB key specific where condition
static
void s_MakeKeyCondition(const string&  key,
                        int            version,
                        const string&  subkey,
                        string*        out_str)
{
    *out_str += " key = ";
    *out_str += "'" + key + "'";
    *out_str += " AND version = ";
    *out_str += NStr::IntToString(version);
    *out_str += " AND subkey = ";
    *out_str += "'" + subkey + "'";
}

///////////////////////////////////////////////////////////////////////////////

class CSQLITE3_Statement
{
public:
    /// compile a statement
    CSQLITE3_Statement(sqlite3* db, const string& sql);
    ~CSQLITE3_Statement();

    sqlite3_stmt* GetStatement();
    void ForgetStatement() { m_Stmt = nullptr; }

    void Bind(int col_or_id, const void* data, size_t size);
    void Bind(int col_or_id, const string& val);
    void Bind(int col_or_id, int val);

    bool Execute();
    void Reset();
    int Step();

    /// return / extract an integer value for a column
    int GetInt(int col);
    CSQLITE3_Statement& operator>> (int& i);

private:
    sqlite3*      m_DB;
    sqlite3_stmt* m_Stmt;
    string        m_Sql;

    void x_Log(int ret, const string& msg)
    {
        LOG_POST(Error << msg << ": [" << ret << "] "
                 << sqlite3_errmsg(m_DB));
    }

    void x_Throw(int ret, const string& msg)
    {
        CNcbiOstrstream ostr;
        ostr << msg << ": [" << ret << "] "
            << sqlite3_errmsg(m_DB);
        string s = string(CNcbiOstrstreamToString(ostr));
        NCBI_THROW(CSQLITE3_ICacheException, eUnknown, s);
    }

private:
    CSQLITE3_Statement(const CSQLITE3_Statement&);
    CSQLITE3_Statement& operator=(const CSQLITE3_Statement&);
};


CSQLITE3_Statement::CSQLITE3_Statement(sqlite3* db, const string& sql)
: m_DB(db), m_Stmt(NULL), m_Sql(sql)
{
    _TRACE("sql: " << sql);
    int ret = 0;
#if (SQLITE_VERSION_NUMBER > 3005001)
    if ( (ret = sqlite3_prepare_v2(m_DB, sql.c_str(), -1,
                                   &m_Stmt, NULL)) != SQLITE_OK) {
#else
    if ( (ret = sqlite3_prepare(m_DB, sql.c_str(), -1,
                                &m_Stmt, NULL)) != SQLITE_OK) {
#endif
        m_Stmt = NULL;
        x_Throw(ret, "error preparing statement for \"" + sql + "\"");
    }
    //_TRACE("exec sql: " << sql);
}

CSQLITE3_Statement::~CSQLITE3_Statement()
{
    if (m_Stmt) {
        sqlite3_finalize(m_Stmt);
    }
}

sqlite3_stmt* CSQLITE3_Statement::GetStatement()
{
    return m_Stmt;
}

int CSQLITE3_Statement::GetInt(int col)
{
    return sqlite3_column_int(m_Stmt, col);
}

CSQLITE3_Statement& CSQLITE3_Statement::operator>>(int& i)
{
    i = GetInt(0);
    return *this;
}

void CSQLITE3_Statement::Bind(int col_or_id, const void* data, size_t size)
{
    _ASSERT(m_Stmt);
    int ret = 0;
    if ( (ret = sqlite3_bind_blob(m_Stmt, col_or_id,
                                  data, size, NULL)) != SQLITE_OK) {
        x_Throw(ret, "error binding blob");
    }
}

void CSQLITE3_Statement::Bind(int col_or_id, const string& val)
{
    _ASSERT(m_Stmt);
    int ret = 0;
    if ( (ret = sqlite3_bind_text(m_Stmt, col_or_id,
                                  val.data(), val.size(), NULL)) != SQLITE_OK) {
        x_Throw(ret, "error binding string");
    }
}

void CSQLITE3_Statement::Bind(int col_or_id, int val)
{
    _ASSERT(m_Stmt);
    int ret = 0;
    if ( (ret = sqlite3_bind_int(m_Stmt, col_or_id, val)) != SQLITE_OK) {
        x_Throw(ret, "error binding int");
    }
}

bool CSQLITE3_Statement::Execute()
{
    size_t count = 0;
    for (;;  ++count) {
        int ret = 0;
        switch ( (ret = sqlite3_step(m_Stmt)) ) {
        case SQLITE_ROW:
            break;

        case SQLITE_DONE:
            return true;

        default:
            return false;
        }
    }

    return true;
}

int CSQLITE3_Statement::Step()
{
    return sqlite3_step(m_Stmt);
}

void CSQLITE3_Statement::Reset()
{
    sqlite3_reset(m_Stmt);
#if (SQLITE_VERSION_NUMBER > 3005001)
    sqlite3_clear_bindings(m_Stmt);
#endif
}


//////////////////////////////////////////////////////////////////////////////

static IReader* GetBlobReader(CSQLITE3_Statement& stmt, int col)
{
    /// local IReader implementation
    class CBlobReader : public IReader
    {
    public:
        CBlobReader(const unsigned char* buf, size_t size)
            : m_Pos(0)
        {
            m_Buf.resize(size);
            memcpy(m_Buf.data(), buf, size);
        }

        virtual ~CBlobReader()
        {
            /**
            _TRACE(NCBI_CURRENT_FUNCTION << ": read "
                   << m_Pos << "/" << m_Buf.size() << " bytes");
                   **/
        }

        ERW_Result Read(void*   buf,
                        size_t  count,
                        size_t* bytes_read = 0)
        {
            if ( !bytes_read ) {
                return eRW_Success;
            }

            count = min(count, m_Buf.size() - m_Pos);
            *bytes_read = count;
            if (count) {
                memcpy(buf, &m_Buf[m_Pos], count);
                m_Pos += count;
                return eRW_Success;
            } else {
                return eRW_Eof;
            }
        }

        ERW_Result PendingCount(size_t* count)
        {
            *count = m_Buf.size() - m_Pos;
            return eRW_Success;
        }

    private:
        CSimpleBuffer m_Buf;
        size_t m_Pos;
    };

    ///
    /// retrieve our row and create a blob reader
    /// FIXME: use incremental I/O in the future
    ///

    CStopWatch sw;
    sw.Start();

    auto_ptr<IReader> reader;
    int size = sqlite3_column_bytes(stmt.GetStatement(), col);
    const void* data = sqlite3_column_blob(stmt.GetStatement(), col);
    if (data) {
        reader.reset(new CBlobReader((const unsigned char*)data, size));
        s_CacheStats.bytes_read.Add(size);
        s_CacheStats.objects_read.Add(1);
    }
    double e = sw.Elapsed();
    s_CacheStats.total_time.Add((int)(e * 1000));
    //LOG_POST(Info << NCBI_CURRENT_FUNCTION << "(): read = " << sw.Elapsed() * 1000 << " msec");

    return reader.release();
}


///////////////////////////////////////////////////////////////////////////////
const char* CSQLITE3_ICacheException::GetErrCodeString(void) const
{
    switch (GetErrCode())
    {
    case eUnknown:          return "eUnknown";
    case eInitError:        return "eInitError";
    case eNotImplemented:   return "eNotImplemented";
    default:                return  CException::GetErrCodeString();
    }
}

///////////////////////////////////////////////////////////////////////////////

CSQLITE3_Cache::CSQLITE3_Cache()
    : m_Timeout(7 * (24*60*60))
    , m_TimeStampFlag(kDefaultTimestampPolicy)
    , m_VersionFlag(eKeepAll)
    , m_DB(NULL)
{
//    m_WriterThread.Reset(new CWriterThread(*this, m_WriteQueue));
//    m_WriterThread->Run();
}

CSQLITE3_Cache::~CSQLITE3_Cache()
{
    LOG_POST(Info << NCBI_CURRENT_FUNCTION);

    // stop the write thread and queue
    if (!m_WriterThread.IsNull()) {
        m_WriterThread->Stop();
        m_WriterThread->Join();
    }

    if (m_DB) {
        
        if (m_Stmt_Store)
            m_Stmt_Store->ForgetStatement();
        if (m_Stmt_HasBlobs_key)
            m_Stmt_HasBlobs_key->ForgetStatement();
        if (m_Stmt_HasBlobs_key_subkey)
            m_Stmt_HasBlobs_key_subkey->ForgetStatement();
        if (m_Stmt_GetBlobAccess)
            m_Stmt_GetBlobAccess->ForgetStatement();
        if (m_Stmt_GetReadStream)
            m_Stmt_GetReadStream->ForgetStatement();
        if (m_Stmt_SetTimestamp)
            m_Stmt_SetTimestamp->ForgetStatement();

        
        // close all pending statements
        size_t count = 0;
        sqlite3_stmt *stmt = NULL;
        while ( (stmt = sqlite3_next_stmt(m_DB, 0))!=0 ){
            sqlite3_finalize(stmt);
            ++count;
        }
        if (count) {
            LOG_POST(Warning << "CSQLITE3_Cache::~CSQLITE3_Cache(): flushed "
                     << count << " pending statements");
        } else {
            _TRACE("CSQLITE3_Cache::~CSQLITE3_Cache(): no pending statements");
        }
        

        // close the database
        int ret = sqlite3_close(m_DB);
        m_DB = NULL;
        if (ret != SQLITE_OK) {
            _ASSERT(ret != SQLITE_BUSY);
            LOG_POST(Error << "CSQLITE3_Cache::~CSQLITE3_Cache(): "
                     "error closing database '" << m_Database << "'");
        }
    }

    size_t items = s_CacheStats.objects_read.Get();
    size_t bytes = s_CacheStats.bytes_read.Get();
    size_t msec  = s_CacheStats.total_time.Get();

    if (items) {
        LOG_POST(Info << "CSQLITE3_Cache::~CSQLITE3_Cache(): read "
            << items << " items / "
            << bytes << " bytes / "
            << msec << " msec / "
            << bytes / double(items) << " bytes/item / "
            << msec / double(items) << " msec/item / "
            );
    }
}

bool CSQLITE3_Cache::Vacuum()
{
    CMutexGuard guard(m_Mutex);

    CSQLITE3_Statement stmt(m_DB, "VACUUM");
    if (!stmt.Execute() ) {
        LOG_POST(Warning << "Failed to vacuum the sqllite3 database:" << m_Database);
        return false;
    }
    return true;
}


void CSQLITE3_Cache::Open(const string& database, bool remove)
{
    CStopWatch sw;
    sw.Start();

    LOG_POST(Info << NCBI_CURRENT_FUNCTION << "(" << database << ")");



    //
    // open the database
    m_Database = database;

    if (m_DB) {
        /*int ret = */sqlite3_close(m_DB);
        m_DB = 0;
    }

    if (remove) {
        CDirEntry de(m_Database);
        de.Remove();
    }

    // check if target dir is present
    //
    {
        string dir;
        CDirEntry::SplitPath(m_Database, &dir);

        CDir de(dir);
        if (!de.Exists()) {
            de.Create();
        }
    }


    #if (SQLITE_VERSION_NUMBER > 3005001)
        int ret = sqlite3_open_v2(m_Database.c_str(), &m_DB,
                                  SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                  NULL);
    #else
        int ret = sqlite3_open(m_Database.c_str(), &m_DB);
    #endif
    if (ret != SQLITE_OK) {
        string msg("error opening database '");
        msg += m_Database;
        msg += "': ";
        msg += sqlite3_errmsg(m_DB);

        try {
            CDirEntry de(m_Database);
            de.Remove();
        } 
        catch (std::exception& e) 
        {
            LOG_POST(Error << "Attempt to delete damaged DB failed: " << e.what());
        }

        NCBI_THROW(CSQLITE3_ICacheException, eInitError, msg);
    }



    //
    // standard db connection setup fior SQLite
    //
    if ( !CSQLITE3_Statement(m_DB, "PRAGMA journal_mode = OFF").Execute() ) {
        NCBI_THROW(CSQLITE3_ICacheException, eInitError,
                   "failed to set journaling mode");
    }

    if ( !CSQLITE3_Statement(m_DB, "PRAGMA temp_store = MEMORY").Execute() ) {
        NCBI_THROW(CSQLITE3_ICacheException, eInitError,
                   "failed to set temp store");
    }

    if ( !CSQLITE3_Statement(m_DB, "PRAGMA page_size = 32768").Execute() ) {
        NCBI_THROW(CSQLITE3_ICacheException, eInitError,
                   "failed to set page_size");
    }

    // synchronous = OFF gives major speed difference when we update timestamps
    //
    if ( !CSQLITE3_Statement(m_DB, "PRAGMA synchronous = OFF").Execute() ) {
        NCBI_THROW(CSQLITE3_ICacheException, eInitError,
                   "failed to set synchronous mode");
    }
    if ( !CSQLITE3_Statement(m_DB, "PRAGMA count_changes = OFF").Execute() ) {
        NCBI_THROW(CSQLITE3_ICacheException, eInitError,
                   "failed to disable count chnages mode");
    }


    //
    // we now have an open connection
    // we can try to purge some data
    //
    if (CSQLITE3_Statement(m_DB, "PRAGMA table_info(CacheBlobs)").Step() == SQLITE_ROW) {
    } else {
        // create our table
        CSQLITE3_Statement stmt
            (m_DB,
             "CREATE TABLE CacheBlobs " \
             "(" \
             "   key       varchar(256) NOT NULL, " \
             "   version   int          NOT NULL, " \
             "   subkey    varchar(256) NOT NULL, " \
             "" \
             "   timestamp int          NOT NULL, " \
             "   data      blob         NULL " \
             ")");
        if ( !stmt.Execute() ) {
            NCBI_THROW(CSQLITE3_ICacheException, eInitError,
                       "failed to initialize cache");
        }
    }

    // check our indices as well
    // some earlier versions were created without indices
    if (CSQLITE3_Statement(m_DB, "PRAGMA index_info(CacheBlobs_pk)").Step() != SQLITE_ROW) {
        CSQLITE3_Statement stmt
            (m_DB,
            "CREATE UNIQUE INDEX CacheBlobs_pk ON CacheBlobs(key, version, subkey)");
        if ( !stmt.Execute() ) {
            NCBI_THROW(CSQLITE3_ICacheException, eInitError,
                       "failed to initialize cache: failed to create PK index");
        }
    }

    if (CSQLITE3_Statement(m_DB, "PRAGMA index_info(CacheBlobs_timestamp)").Step() != SQLITE_ROW) {
        CSQLITE3_Statement stmt
            (m_DB,
            "CREATE INDEX CacheBlobs_timestamp ON CacheBlobs(timestamp)");
        if ( !stmt.Execute() ) {
            NCBI_THROW(CSQLITE3_ICacheException, eInitError,
                "failed to initialize cache: failed to create timestamp index");
        }
    }

    // start background writer (if it is not started yet)
    if (m_WriterThread.IsNull()) {
        m_WriterThread.Reset(new CWriterThread(*this, m_WriteQueue));
        m_WriterThread->Run();
    }

    LOG_POST(Info << "CSQLITE3_Cache::Open(): " << sw.Elapsed() << " seconds");
}


bool CSQLITE3_Cache::IsOpen() const
{
    //LOG_POST(Info << NCBI_CURRENT_FUNCTION);
    return (m_DB != NULL);
}


string CSQLITE3_Cache::GetCacheName() const
{
    //LOG_POST(Info << NCBI_CURRENT_FUNCTION);
    return m_Database;
}


void CSQLITE3_Cache::SetTimeStampPolicy(TTimeStampFlags policy,
                                        unsigned int    timeout,
                                        unsigned int    max_timeout)
{
    //LOG_POST(Info << NCBI_CURRENT_FUNCTION);
    CMutexGuard guard(m_Mutex);
    if (policy) {
        m_TimeStampFlag = policy;
    } else {
        m_TimeStampFlag = kDefaultTimestampPolicy;
    }

    m_Timeout = timeout;
}


CSQLITE3_Cache::TTimeStampFlags CSQLITE3_Cache::GetTimeStampPolicy() const
{
    //LOG_POST(Info << NCBI_CURRENT_FUNCTION);
    return m_TimeStampFlag;
}


int CSQLITE3_Cache::GetTimeout() const
{
    //LOG_POST(Info << NCBI_CURRENT_FUNCTION);
    return m_Timeout;
}


void CSQLITE3_Cache::SetVersionRetention(EKeepVersions policy)
{
    //LOG_POST(Info << NCBI_CURRENT_FUNCTION);
    m_VersionFlag = policy;
}


CSQLITE3_Cache::EKeepVersions CSQLITE3_Cache::GetVersionRetention() const
{
    //LOG_POST(Info << NCBI_CURRENT_FUNCTION);
    return m_VersionFlag;
}


void CSQLITE3_Cache::x_SetTimestamp(const string& key,
                                    int           version,
                                    const string& subkey)
{
    CMutexGuard LOCK(m_Mutex);
    time_t time = CTime(CTime::eCurrent).GetTimeT();

    if ( !m_Stmt_SetTimestamp.get() ) {
        string sql =
            "UPDATE CacheBlobs SET timestamp = ?1 WHERE "
            "key = ?2 AND version = ?3 AND subkey = ?4";
        //s_MakeKeyCondition(key, version, subkey, &sql);
        m_Stmt_SetTimestamp.reset(new CSQLITE3_Statement(m_DB, sql));
    } else {
        m_Stmt_SetTimestamp->Reset();
    }

    //CSQLITE3_Statement stmt2(m_DB, sql);
    m_Stmt_SetTimestamp->Bind(1, int(time));
    m_Stmt_SetTimestamp->Bind(2, key);
    m_Stmt_SetTimestamp->Bind(3, version);
    m_Stmt_SetTimestamp->Bind(4, subkey);
    if ( !m_Stmt_SetTimestamp->Execute() ) {
        LOG_POST(Error << "failed to update timestamp on cache blob: "
            << "\"" << key << "\", " << version << ", \""
            << subkey << "\": " << sqlite3_errmsg(m_DB));
    } else {
        /**
        LOG_POST(Info
            << "\"" << key << "\", " << version << ", \""
            << subkey << "\": timestamp=" << time);
            **/
    }
}

void CSQLITE3_Cache::Store(const string&  key,
                           int            version,
                           const string&  subkey,
                           const void*    data,
                           size_t         size,
                           unsigned int   time_to_live,
                           const string&  owner)
{
    //
    // prepare our write reque    
    CRef<SWriteRequest> req(new SWriteRequest);
    req->key = key;
    req->version = version;
    req->subkey = subkey;
    req->buffer.resize(size);
    memcpy(req->buffer.data(), data, size);

    // push to our queue
    m_WriteQueue.Push(req);
}



void CSQLITE3_Cache::StoreSynchronous(const string&  key,
                                      int            version,
                                      const string&  subkey,
                                      const void*    data,
                                      size_t         size)
{
    CMutexGuard LOCK(m_Mutex);

    /**
    LOG_POST(Info << NCBI_CURRENT_FUNCTION
        << "(\"" << key << "\", " << version << ", \""
        << subkey << "\", " << data << ", " << size << "): thread="
        << CThread::GetSelf());
        **/
    _ASSERT(m_DB);

    if (m_VersionFlag == eDropAll || m_VersionFlag == eDropOlder) {
        Purge(key, subkey, 0);
    }

    string sql;
    int ret = 0;
    time_t timestamp = CTime(CTime::eCurrent).GetTimeT();

    //
    // insert the row into cache data
    // we scan first to see if the row already exists
    //
    if ( !m_Stmt_Store.get() ) {
        m_Stmt_Store.reset(new CSQLITE3_Statement(m_DB,
            "INSERT OR REPLACE INTO CacheBlobs (key, version, subkey, timestamp, data) "
            "VALUES( ?1, ?2, ?3, ?4, ?5 )"));
    }

    m_Stmt_Store->Reset();
    m_Stmt_Store->Bind(1, key);
    m_Stmt_Store->Bind(2, version);
    m_Stmt_Store->Bind(3, subkey);
    m_Stmt_Store->Bind(4, int(timestamp));
    m_Stmt_Store->Bind(5, data, size);

     // execute...
     if ( (ret = m_Stmt_Store->Step()) != SQLITE_DONE) {
         LOG_POST(Error << "failed to write " << size << " bytes: "
                  << sql << ": [" << ret << "] " << sqlite3_errmsg(m_DB));
     }
}


size_t CSQLITE3_Cache::GetSize(const string&  key,
                               int            version,
                               const string&  subkey)
{
    LOG_POST(Info << NCBI_CURRENT_FUNCTION);

    string sql = "SELECT data FROM CacheBlobs WHERE ";
    s_MakeKeyCondition(key, version, subkey, &sql);
    CSQLITE3_Statement stmt(m_DB, sql);
    if (stmt.Step() == SQLITE_ROW) {
        return sqlite3_column_bytes(stmt.GetStatement(), 0);
    }

    return 0;
}


bool CSQLITE3_Cache::Read(const string& key,
                          int           version,
                          const string& subkey,
                          void*         buf,
                          size_t        buf_size)
{
    LOG_POST(Info << NCBI_CURRENT_FUNCTION);

    string sql = "SELECT data FROM CacheBlobs WHERE ";
    s_MakeKeyCondition(key, version, subkey, &sql);

    CSQLITE3_Statement stmt(m_DB, sql);
    if (stmt.Step() == SQLITE_ROW) {
        size_t size = sqlite3_column_bytes(stmt.GetStatement(), 0);
        size = min(size, buf_size);
        memcpy(buf,
               sqlite3_column_blob(stmt.GetStatement(), 0),
               size);

        /// set timestamp
        if (m_TimeStampFlag & fTimeStampOnRead) {
            x_SetTimestamp(key, version, subkey);
        }
        return true;
    }

    return false;
}


IReader* CSQLITE3_Cache::GetReadStream(const string&  key,
                                       int            version,
                                       const string&  subkey)
{
    CMutexGuard LOCK(m_Mutex);
    //LOG_POST(Info << NCBI_CURRENT_FUNCTION << "(" << key << ", " << version << ", " << subkey << ")");

    //
    // retrieve our row and create a blob reader
    // FIXME: use incremental I/O in the future
    //
    if ( !m_Stmt_GetReadStream.get() ) {
        string sql =
            "SELECT data FROM CacheBlobs WHERE "
            "key = ?1 AND version = ?2 AND subkey = ?3"
            ;
        m_Stmt_GetReadStream.reset(new CSQLITE3_Statement(m_DB, sql));
    } else {
        m_Stmt_GetReadStream->Reset();
    }
    //s_MakeKeyCondition(key, version, subkey, &sql);
    //CSQLITE3_Statement stmt(m_DB, sql);
    m_Stmt_GetReadStream->Bind(1, key);
    m_Stmt_GetReadStream->Bind(2, version);
    m_Stmt_GetReadStream->Bind(3, subkey);
    if (m_Stmt_GetReadStream->Step() == SQLITE_ROW) {
        auto_ptr<IReader> reader(GetBlobReader(*m_Stmt_GetReadStream, 0));

        /// set timestamp
        if (m_TimeStampFlag & fTimeStampOnRead) {
            x_SetTimestamp(key, version, subkey);
        }
        return reader.release();
    }

    return NULL;
}

IReader* CSQLITE3_Cache::GetReadStream(
    const string&  /*key*/,
    const string&  /*subkey*/,
    int*           /*version*/,
    EBlobValidity* /*validity*/)
{
    // ICache last valid version protocol is not implemented in GBench
    // (this is ok it is optimization only important for network cache)
    NCBI_THROW(CSQLITE3_ICacheException, eNotImplemented,
        "CSQLITE3_Cache::GetReadStream(key, subkey, version, validity) "
        "is not implemented");
}

void CSQLITE3_Cache::SetBlobVersionAsValid(const string&  /* key */,
                                           const string&  /* subkey */,
                                           int            /* version */)
{
    // ICache last valid version protocol is not implemented in GBench
    // (this is ok it is optimization only important for network cache)
    NCBI_THROW(CSQLITE3_ICacheException, eNotImplemented,
        "CSQLITE3_Cache::SetBlobVersionAsValid(key, subkey, version) "
        "is not implemented");
}


IWriter* CSQLITE3_Cache::GetWriteStream(const string& key,
                                        int           version,
                                        const string& subkey,
                                        unsigned int  time_to_live,
                                        const string& owner)
{
    //LOG_POST(Info << NCBI_CURRENT_FUNCTION << "(" << key << ", " << version << ", " << subkey << ", " << time_to_live << ", " << owner << ")");

    class CBlobWriter : public IWriter
    {
    public:
        CBlobWriter(CSQLITE3_Cache& cache,
                    const string&   key,
                    int             version,
                    const string&   subkey)
            : m_Cache(cache)
            , m_Key(key)
            , m_Version(version)
            , m_Subkey(subkey)
            , m_Flushed(false)
        {
        }

        ~CBlobWriter()
        {
            if ( !m_Flushed ) {
                Flush();
            }
        }

        ERW_Result Write(const void* buf,
                         size_t      count,
                         size_t*     bytes_written = 0)
        {
            if ( !m_Flushed ) {
                m_Data.insert(m_Data.end(),
                              (const unsigned char*)(buf),
                              (const unsigned char*)(buf) + count);
                if (bytes_written) {
                    *bytes_written = count;
                }
                return eRW_Success;
            } else {
                return eRW_Error;
            }
        }

        ERW_Result Flush()
        {
            if (m_Data.size()) {
                m_Cache.Store(m_Key, m_Version, m_Subkey,
                              &m_Data[0], m_Data.size());
                //LOG_POST(Info << NCBI_CURRENT_FUNCTION << "(): wrote " << m_Data.size() << " bytes");
            }
            m_Flushed = true;
            return eRW_Success;
        }

    private:
        CSQLITE3_Cache& m_Cache;
        string m_Key;
        int m_Version;
        string m_Subkey;
        vector<unsigned char> m_Data;
        bool m_Flushed;
    };

    return new CBlobWriter(*this, key, version, subkey);
}


void CSQLITE3_Cache::Remove(const string& key)
{
    //LOG_POST(Info << NCBI_CURRENT_FUNCTION);

    // create transaction
    string sql = "DELETE FROM CacheBlobs WHERE key = '";
    sql += key;
    sql += "'";
    CSQLITE3_Statement stmt(m_DB, sql);
    stmt.Execute();
}


void CSQLITE3_Cache::Remove(const string&    key,
                            int              version,
                            const string&    subkey)
{
    //LOG_POST(Info << NCBI_CURRENT_FUNCTION);

    /// create transaction
    string sql = "DELETE FROM CacheBlobs WHERE ";
    s_MakeKeyCondition(key, version, subkey, &sql);
    CSQLITE3_Statement stmt(m_DB, sql);
    stmt.Execute();
}


time_t CSQLITE3_Cache::GetAccessTime(const string&  key,
                                     int            version,
                                     const string&  subkey)
{
    LOG_POST(Info << NCBI_CURRENT_FUNCTION);

    string sql = "SELECT timestamp FROM CacheBlobs WHERE ";
    s_MakeKeyCondition(key, version, subkey, &sql);
    CSQLITE3_Statement stmt(m_DB, sql);
    if (stmt.Step() == SQLITE_ROW) {
        return stmt.GetInt(0);
    }

    return 0;
}


void CSQLITE3_Cache::Purge(time_t           access_timeout)
{
    LOG_POST(Info << NCBI_CURRENT_FUNCTION);

    if (access_timeout == 0) {
        //x_TruncateDB();
        return;
    }

    CTime time_stamp(CTime::eCurrent);
    time_t curr = time_stamp.GetTimeT();
    int timeout = GetTimeout();
    curr -= timeout;

    string sql = "DELETE FROM CacheBlobs WHERE timestamp < ?1";
    CSQLITE3_Statement stmt(m_DB, sql);
    stmt.Bind(1, int(curr));
    if (stmt.Step() == SQLITE_DONE) {
        int count = sqlite3_changes(m_DB);
        LOG_POST(Info << "CSQLITE3_Cache::Purge(): "
            << count << " items purged");
    }
}


void CSQLITE3_Cache::Purge(const string&    key,
                           const string&    subkey,
                           time_t           access_timeout)
{
    LOG_POST(Info << NCBI_CURRENT_FUNCTION);

    if (access_timeout == 0) {
        //x_TruncateDB();
        return;
    }

    CTime time_stamp(CTime::eCurrent);
    time_t curr = time_stamp.GetTimeT();
    int timeout = GetTimeout();
    curr -= timeout;

    string sql =
        "DELETE FROM CacheBlobs WHERE "
        " timestamp < ?1 ";

    if (!key.empty()) {
        sql += " AND key = '";
        sql += key;
        sql += "'";
    }

    if (!subkey.empty()) {
        sql += " AND subkey = '";
        sql += subkey;
        sql += "'";
    }
    CSQLITE3_Statement stmt(m_DB, sql);
    stmt.Bind(1, int(curr));
    if (stmt.Step() == SQLITE_DONE) {
        int count = sqlite3_changes(m_DB);
        LOG_POST(Info << "CSQLITE3_Cache::Purge(): "
            << count << " items purged");
    }
}


bool CSQLITE3_Cache::SameCacheParams(const TCacheParams* params) const
{
    if ( !params ) {
        return false;
    }
    const TCacheParams* driver = params->FindNode("driver");
    if (!driver  ||  driver->GetValue().value != kSQLITE3_BlobCacheDriverName) {
        return false;
    }
    const TCacheParams* driver_params =
        params->FindNode(kSQLITE3_BlobCacheDriverName);
    if ( !driver_params ) {
        return false;
    }
    const TCacheParams* path = driver_params->FindNode("database");
    if (!path) {
        return false;
    }
    const string& database = path->GetValue().value;

    string base1, base2;
    CDirEntry::SplitPath(database, 0, &base1, 0);
    CDirEntry::SplitPath(m_Database, 0, &base2, 0);

    if (base1 == base2) 
        return true;
    return false;
}


bool CSQLITE3_Cache::HasBlobs(const string&  key,
                              const string&  subkey)
{
    CMutexGuard LOCK(m_Mutex);
    //LOG_POST(Info << NCBI_CURRENT_FUNCTION << "(\"" << key << "\", \"" << subkey << "\")");

    CSQLITE3_Statement* stmt = NULL;
    if ( !m_Stmt_HasBlobs_key.get() ) {
        m_Stmt_HasBlobs_key.reset
            (new CSQLITE3_Statement(m_DB,
                                    "SELECT timestamp FROM CacheBlobs WHERE "
                                    "key = ?1"));
    }

    if ( !m_Stmt_HasBlobs_key_subkey.get() ) {
        m_Stmt_HasBlobs_key_subkey.reset
            (new CSQLITE3_Statement(m_DB,
                                    "SELECT timestamp FROM CacheBlobs WHERE "
                                    "key = ?1 AND subkey = ?2"));
    }

    if (subkey.empty()) {
        stmt = &*m_Stmt_HasBlobs_key;
    } else {
        stmt = &*m_Stmt_HasBlobs_key_subkey;
    }

    stmt->Reset();
    stmt->Bind(1, key);
    if ( !subkey.empty() ) {
        stmt->Bind(2, subkey);
    }
    return (stmt->Step() == SQLITE_ROW);
}


void CSQLITE3_Cache::GetBlobAccess(const string&     key,
                                   int               version,
                                   const string&     subkey,
                                   SBlobAccessDescr*  blob_descr)
{
    CMutexGuard LOCK(m_Mutex);
    /**
    LOG_POST(Info << NCBI_CURRENT_FUNCTION
             << "(\"" << key << "\", " << version << ", \"" << subkey << "\"): thread="
             << CThread::GetSelf());
             **/

    blob_descr->reader.reset();
    blob_descr->blob_size = 0;
    blob_descr->blob_found = false;

    string sql;
    time_t time    = CTime(CTime::eCurrent).GetTimeT();
    time_t timeout = time - GetTimeout();

    // check to see if the row exists in the cache attributes
    // if so, we intend on updating the timestamp to be now
    int this_timestamp = 0;

    // we have a blob
    if ( !m_Stmt_GetBlobAccess.get() ) {
        sql =
            "SELECT timestamp, data FROM CacheBlobs WHERE "
            "key = ?1 AND version = ?2 AND subkey = ?3";
        //s_MakeKeyCondition(key, version, subkey, &sql);
        m_Stmt_GetBlobAccess.reset(new CSQLITE3_Statement(m_DB, sql));
    } else {
        m_Stmt_GetBlobAccess->Reset();
    }

    m_Stmt_GetBlobAccess->Bind(1, key);
    m_Stmt_GetBlobAccess->Bind(2, version);
    m_Stmt_GetBlobAccess->Bind(3, subkey);
    if (m_Stmt_GetBlobAccess->Step() == SQLITE_ROW) {
        // read and process our timestamp
        this_timestamp = m_Stmt_GetBlobAccess->GetInt(0);
        if (this_timestamp < timeout) {
            Remove(key, version, subkey);
        } else {
            // read and process our blob
            blob_descr->blob_size =
                sqlite3_column_bytes(m_Stmt_GetBlobAccess->GetStatement(), 1);
            blob_descr->blob_found = true;

            if (blob_descr->buf  &&
                blob_descr->buf_size >= blob_descr->blob_size) {
                memcpy(blob_descr->buf,
                       sqlite3_column_blob(m_Stmt_GetBlobAccess->GetStatement(), 1),
                       blob_descr->blob_size);
                //LOG_POST(Info << "  direct copy: " << blob_descr->blob_size << " bytes");
            } else {
                blob_descr->reader.reset(GetBlobReader(*m_Stmt_GetBlobAccess, 1));
                /**
                LOG_POST(Info << "  stream read: " << blob_descr->blob_size
                         << " bytes / " << blob_descr->buf_size << " bytes in buffer");
                         **/
            }
        }

        // set timestamp
        if (m_TimeStampFlag & fTimeStampOnRead) {
            x_SetTimestamp(key, version, subkey);
        }
    }
}


/// @name unimplemented
/// @{
void CSQLITE3_Cache::GetBlobOwner(const string&  key,
                                  int            version,
                                  const string&  subkey,
                                  string*        owner)
{
    LOG_POST(Info << NCBI_CURRENT_FUNCTION);
    _ASSERT(owner);
    owner->erase(); // not supported in this implementation
}

void CSQLITE3_Cache::SetMemBufferSize(unsigned int buf_size)
{
    /// noop
}


/// @}

END_NCBI_SCOPE
