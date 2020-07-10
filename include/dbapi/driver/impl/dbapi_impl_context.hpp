#ifndef DBAPI_DRIVER_IMPL___DBAPI_IMPL_CONTEXT__HPP
#define DBAPI_DRIVER_IMPL___DBAPI_IMPL_CONTEXT__HPP

/* $Id: dbapi_impl_context.hpp 555020 2018-01-11 16:43:38Z satskyse $
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
 * Author:  Sergey Sikorskiy
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <corelib/ncbimtx.hpp>

#include <dbapi/driver/interfaces.hpp>

#include <dbapi/driver/impl/handle_stack.hpp>


BEGIN_NCBI_SCOPE

class CDB_Connection;
class IConnValidator;
class CDB_UserHandler;
class CDB_Connection;

namespace impl
{

/////////////////////////////////////////////////////////////////////////////
// Forward declarations

class CConnection;


struct NCBI_DBAPIDRIVER_EXPORT SDBConfParams
{
    enum EFlags {
        fServerSet         = 0x0001,
        fPortSet           = 0x0002,
        fDatabaseSet       = 0x0004,
        fUsernameSet       = 0x0008,
        fPasswordSet       = 0x0010,
        fLoginTimeoutSet   = 0x0020,
        fIOTimeoutSet      = 0x0040,
        fCancelTimeoutSet  = 0x0080,
        fSingleServerSet   = 0x0100,
        fIsPooledSet       = 0x0200,
        fPoolMinSizeSet    = 0x0400,
        fPoolMaxSizeSet    = 0x0800,
        fArgsSet           = 0x1000,
        fPasswordFileSet   = 0x2000,
        fPasswordKeySet    = 0x4000,
        fPoolIdleTimeSet   = 0x8000,
        fPoolWaitTimeSet   = 0x10000,
        fPoolAllowTempSet  = 0x20000,
        fContRaiserrorSet  = 0x40000,
        fPoolMaxConnUseSet = 0x80000
        // , fPoolNameSet     = 0x40000
    };
    typedef unsigned int  TFlags;

    TFlags  flags;
    string  server;
    string  port;
    string  database;
    string  username;
    string  password;
    string  password_file;   // just used by SDBAPI, at least for now
    string  password_key_id; // just used by SDBAPI, at least for now
    string  login_timeout;
    string  io_timeout;
    string  cancel_timeout;
    string  single_server;
    string  is_pooled;
    string  pool_name;
    string  pool_minsize;
    string  pool_maxsize;
    string  pool_idle_time;
    string  pool_wait_time;
    string  pool_allow_temp_overflow;
    string  continue_after_raiserror;
    string  pool_max_conn_use;
    string  args;


    bool IsFlagSet(EFlags one_flag) { return (flags & one_flag) != 0; }

    bool IsServerSet(void)         { return IsFlagSet(fServerSet);         }
    bool IsPortSet(void)           { return IsFlagSet(fPortSet);           }
    bool IsDatabaseSet(void)       { return IsFlagSet(fDatabaseSet);       }
    bool IsUsernameSet(void)       { return IsFlagSet(fUsernameSet);       }
    bool IsPasswordSet(void)       { return IsFlagSet(fPasswordSet);       }
    bool IsPasswordFileSet(void)   { return IsFlagSet(fPasswordFileSet);   }
    bool IsPasswordKeySet(void)    { return IsFlagSet(fPasswordKeySet);    }
    bool IsLoginTimeoutSet(void)   { return IsFlagSet(fLoginTimeoutSet);   }
    bool IsIOTimeoutSet(void)      { return IsFlagSet(fIOTimeoutSet);      }
    bool IsCancelTimeoutSet(void)  { return IsFlagSet(fCancelTimeoutSet);  }
    bool IsSingleServerSet(void)   { return IsFlagSet(fSingleServerSet);   }
    bool IsPooledSet(void)         { return IsFlagSet(fIsPooledSet);       }
    // bool IsPoolNameSet(void)     { return IsFlagSet(fPoolNameSet);      }
    bool IsPoolMinSizeSet(void)    { return IsFlagSet(fPoolMinSizeSet);    }
    bool IsPoolMaxSizeSet(void)    { return IsFlagSet(fPoolMaxSizeSet);    }
    bool IsPoolIdleTimeSet(void)   { return IsFlagSet(fPoolIdleTimeSet);   }
    bool IsPoolWaitTimeSet(void)   { return IsFlagSet(fPoolWaitTimeSet);   }
    bool IsPoolAllowTempOverflowSet(void)
                                   { return IsFlagSet(fPoolAllowTempSet);  }
    bool IsContinueAfterRaiserrorSet(void)
                                   { return IsFlagSet(fContRaiserrorSet);  }
    bool IsPoolMaxConnUseSet(void) { return IsFlagSet(fPoolMaxConnUseSet); }
    bool IsArgsSet(void)           { return IsFlagSet(fArgsSet);           }

    void Clear(void);
};


/////////////////////////////////////////////////////////////////////////////
///
///  CDriverContext::
///

class NCBI_DBAPIDRIVER_EXPORT CDriverContext : public I_DriverContext
{
    friend class impl::CConnection;
    friend class CDBPoolBalancer;

protected:
    CDriverContext(void);

public:
    virtual ~CDriverContext(void);

public:
    virtual bool SetMaxBlobSize(size_t nof_bytes);
    bool SetMaxTextImageSize(size_t nof_bytes)
        { return SetMaxBlobSize(nof_bytes); }

    virtual bool SetLoginTimeout(unsigned int nof_secs = 0);
    virtual unsigned int GetLoginTimeout(void) const;

    virtual bool SetTimeout(unsigned int nof_secs = 0);
    virtual unsigned int GetTimeout(void) const;

    virtual bool SetCancelTimeout(unsigned int nof_secs);
    virtual unsigned int GetCancelTimeout(void) const;

    virtual void SetApplicationName(const string& app_name);
    virtual string GetApplicationName(void) const;

    virtual void SetHostName(const string& host_name);
    virtual string GetHostName(void) const;

    virtual CDB_Connection* MakeConnection(const CDBConnParams& params);

    /// Return number of currently open connections in this context.
    /// If "srv_name" is not NULL, then return # of conn. open to that server.
    virtual unsigned int NofConnections(const string& srv_name  = "",
                                        const string& pool_name = "")
        const;
    virtual unsigned int NofConnections(const TSvrRef& svr_ref,
                                        const string& pool_name = "") const;

    /// Add message handler "h" to process 'context-wide' (not bound
    /// to any particular connection) error messages.
    virtual void PushCntxMsgHandler(CDB_UserHandler* h,
                            EOwnership ownership = eNoOwnership);

    /// Remove message handler "h" and all handlers above it in the stack
    virtual void PopCntxMsgHandler(CDB_UserHandler* h);

    /// Add `per-connection' err.message handler "h" to the stack of default
    /// handlers which are inherited by all newly created connections.
    virtual void PushDefConnMsgHandler(CDB_UserHandler* h,
                               EOwnership ownership = eNoOwnership);

    /// Remove `per-connection' mess. handler "h" and all above it in the stack.
    virtual void PopDefConnMsgHandler(CDB_UserHandler* h);

    /// close reusable deleted connections for specified server and/or pool
    void CloseUnusedConnections(const string& srv_name  = kEmptyStr,
                                const string& pool_name = kEmptyStr,
                                unsigned int max_closings = kMax_UInt);

    const CDBHandlerStack& GetConnHandlerStack(void) const
    {
        return m_ConnHandlers;
    }
    const CDBHandlerStack& GetCtxHandlerStack(void) const
    {
        return m_CntxHandlers;
    }

    virtual void SetClientCharset(const string& charset);
    const string& GetClientCharset(void) const
    {
        return m_ClientCharset;
    }
    EEncoding GetClientEncoding(void) const
    {
        return m_ClientEncoding;
    }
    bool IsMultibyteClientEncoding(void) const
    {
        return (GetClientEncoding() == eEncoding_UTF8);
    }

    size_t GetMaxBlobSize(void) const
    {
        return m_MaxBlobSize;
    }
    size_t GetMaxTextImageSize(void) const { return GetMaxBlobSize(); }

    static void ResetEnvSybase(void);

    void ReadDBConfParams  (const string& service_name, SDBConfParams* params);
    bool SatisfyPoolMinimum(const CDBConnParams& params);

    // Provides the number of the invalidated connections
    size_t CloseConnsForPool (const string& pool_name, Uint4 keep_host_ip=0,
                              Uint2 keep_port=0);
    void CloseOldIdleConns (unsigned int max_closings,
                            const string& pool_name = kEmptyStr);

    typedef map<string, unsigned int> TCounts;    // by server name
    typedef map<string, TCounts>      TCountsMap; // by pool or service name

    void GetCountsForPool(const string& pool_name, TCounts* counts) const
    {
        x_GetCounts(m_CountsByPool, pool_name, counts);
    }
    void GetCountsForService(const string& service, TCounts* counts) const
    {
        x_GetCounts(m_CountsByService, service, counts);
    }
    
protected:
    typedef list<CConnection*> TConnPool;

    virtual SSystemMutex& x_GetCtxMtx(void) const
    {
        return m_DefaultCtxMtx;
    }

    // To allow children of CDriverContext to create CDB_Connection
    CDB_Connection* MakeCDBConnection(CConnection* connection,
                                      int delta /* = 1 */);
    CDB_Connection* MakePooledConnection(const CDBConnParams& params);
    virtual CConnection* MakeIConnection(const CDBConnParams& params) = 0;
    void DestroyConnImpl(CConnection* impl);

    void CloseAllConn(void);
    void DeleteAllConn(void);

    CDBHandlerStack& GetCtxHandlerStack(void)
    {
        return m_CntxHandlers;
    }

    void UpdateConnMaxBlobSize(void) const;
    void UpdateConnTimeout(void) const;

    void SetExtraMsg(const string& msg) const
    {
        m_ExtraMsg = msg;
    }
    const string& GetExtraMsg(void) const
    {
        return m_ExtraMsg;
    }


private:
    mutable CMutex  m_DefaultCtxMtx;
    mutable CMutex  m_PoolMutex; //< for m_PoolSem*, m_(Not)InUse, & m_Counts*
    CSemaphore      m_PoolSem;
    string          m_PoolSemSubject;
    bool            m_PoolSemSubjectHasPoolName;
    CConnection*    m_PoolSemConn;

    unsigned int    m_LoginTimeout; //< Login timeout.
    unsigned int    m_Timeout;      //< Connection timeout.
    unsigned int    m_CancelTimeout;//< Canceling timeout.

    string          m_AppName;  //< Application name.
    string          m_HostName; //< Host name

    /// Unused(reserve) connections
    TConnPool       m_NotInUse;
    /// Used connections
    TConnPool       m_InUse;

    TCountsMap      m_CountsByPool;
    TCountsMap      m_CountsByService;

    /// Stack of `per-context' err.message handlers
    CDBHandlerStack m_CntxHandlers;
    /// Stacks of `per-connection' err.message handlers
    CDBHandlerStack m_ConnHandlers;

    mutable string  m_ExtraMsg;

    string          m_ClientCharset;
    size_t          m_MaxBlobSize;
    EEncoding       m_ClientEncoding;

    /// Return unused connection "conn" to the driver context for future
    /// reuse (if "conn_reusable" is TRUE) or utilization
    void x_Recycle(CConnection* conn, bool conn_reusable);

    void x_AdjustCounts(const CConnection* conn, int delta);
    void x_GetCounts(const TCountsMap& main_map, const string& name,
                     TCounts* counts) const;
};


class NCBI_DBAPIDRIVER_EXPORT CWinSock
{
protected:
    CWinSock(void);
    ~CWinSock(void);
};

} //namespace impl

END_NCBI_SCOPE


#endif  /* DBAPI_DRIVER_IMPL___DBAPI_IMPL_CONTEXT__HPP */
