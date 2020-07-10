#ifndef _GRID_RW_IMPL_HPP_
#define _GRID_RW_IMPL_HPP_


/*  $Id: grid_rw_impl.hpp 575325 2018-11-27 18:22:00Z ucko $
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
 *   Government have not placed any restriction on its use or reproduction.
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
 * Authors:  Maxim Didenko, Dmitry Kazimirov
 *
 */

#include <connect/services/netcache_api.hpp>

#include <connect/connect_export.h>

#include <corelib/reader_writer.hpp>

#include <functional>
#include <memory>

BEGIN_NCBI_SCOPE

class NCBI_XCONNECT_EXPORT CStringOrWriter : public IEmbeddedStreamWriter
{
public:
    using TWriterCreate = function<IEmbeddedStreamWriter*(string&)>;

    CStringOrWriter(size_t max_data_size, string& data_ref, TWriterCreate writer_create);

    ERW_Result Write(const void* buf, size_t count, size_t* bytes_written = 0) override;
    ERW_Result Flush() override;

    void Close() override;
    void Abort() override;

private:
    size_t m_MaxDataSize;
    string& m_Data;
    TWriterCreate m_WriterCreate;
    unique_ptr<IEmbeddedStreamWriter> m_Writer;
};

/// String or Blob Storage Writer
///
/// An implementation of the IWriter interface with a dual behavior.
/// It writes data into the "data_or_key" parameter until
/// the total number of written bytes reaches "max_string_size" parameter.
/// After that all data from "data_or_key" is stored into the Blob Storage
/// and all next calls to Write method will write data to the Blob Storage.
/// In this case "data_or_key" parameter holds a blob key for the written data.
///
class NCBI_XCONNECT_EXPORT CStringOrBlobStorageWriter :
    public CStringOrWriter
{
public:
    CStringOrBlobStorageWriter(size_t max_string_size,
                               SNetCacheAPIImpl* storage,
                               string& job_output_ref);
};

struct SGridWrite
{
    unique_ptr<IEmbeddedStreamWriter> writer;
    unique_ptr<CNcbiOstream> stream;

    CNcbiOstream& operator()(CNetCacheAPI nc_api, size_t embedded_max_size, string& data);
    void Reset(bool flush = false);
};

/// String or Blob Storage Reader
///
/// An implementation of the IReader interface with a dual behavior.
/// If "data_or_key" parameter can be interpreted as Blob Storage key and
/// a blob with given key is found in the storage, then the storage is
/// used as data source. Otherwise "data_or_key" is a data source.
///
class NCBI_XCONNECT_EXPORT CStringOrBlobStorageReader : public IReader
{
public:
    CStringOrBlobStorageReader(const string& data_or_key,
                               SNetCacheAPIImpl* storage,
                               size_t* data_size = NULL);

    virtual ERW_Result Read(void*   buf,
                            size_t  count,
                            size_t* bytes_read = 0);

    virtual ERW_Result PendingCount(size_t* count);

protected:
    enum EType { eNetCache, eEmbedded, eEmpty, eRaw };
    static EType x_GetDataType(string& data);

private:
    CNetCacheAPI m_Storage;
    unique_ptr<IReader> m_NetCacheReader;
    string m_Data;
    size_t m_BytesToRead;
};

struct SGridRead
{
    unique_ptr<CNcbiIstream> stream;

    CNcbiIstream& operator()(CNetCacheAPI nc_api, const string& data, size_t* data_size);
    void Reset();
};

class CStringOrBlobStorageRWException : public CException
{
public:
    enum EErrCode {
        eInvalidFlag
    };

    virtual const char* GetErrCodeString(void) const override
    {
        switch (GetErrCode())
        {
        case eInvalidFlag: return "eInvalidFlag";
        default:           return CException::GetErrCodeString();
        }
    }

    NCBI_EXCEPTION_DEFAULT(CStringOrBlobStorageRWException, CException);
};

END_NCBI_SCOPE

#endif // __GRID_RW_IMPL_HPP_
