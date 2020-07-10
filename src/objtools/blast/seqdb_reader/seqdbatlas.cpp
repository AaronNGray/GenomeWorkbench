/*  $Id: seqdbatlas.cpp 579505 2019-02-04 20:11:05Z boratyng $
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
 * Author:  Kevin Bealer
 *
 */

/// @file seqdbatlas.cpp
/// Implementation for the CSeqDBAtlas class and several related
/// classes, which provide control of a set of memory mappings.
#include <ncbi_pch.hpp>

#include <objtools/blast/seqdb_reader/impl/seqdbatlas.hpp>
#include <objtools/blast/seqdb_reader/impl/seqdbgeneral.hpp>
#include <memory>
#include <algorithm>
#include <objtools/blast/seqdb_reader/seqdbcommon.hpp>

#include <corelib/ncbi_system.hpp>

#if defined(NCBI_OS_UNIX)
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <unistd.h>
#endif

BEGIN_NCBI_SCOPE

#ifdef SEQDB_TRACE_LOGFILE

// By default, the first 16 trace classes are enabled

ofstream * seqdb_logfile  = 0;
int        seqdb_logclass = 0xFFFF;

void seqdb_log(const char * s)
{
    seqdb_log(1, s);
}

void seqdb_log(const char * s1, const string & s2)
{
    seqdb_log(1, s1, s2);
}

inline bool seqdb_log_disabled(int cl)
{
    return ! (seqdb_logfile && (cl & seqdb_logclass));
}

void seqdb_log(int cl, const char * s)
{
    if (seqdb_log_disabled(cl))
        return;

    (*seqdb_logfile) << s << endl;
}

void seqdb_log(int cl, const char * s1, const string & s2)
{
    if (seqdb_log_disabled(cl))
        return;

    (*seqdb_logfile) << s1 << s2 << endl;
}

void seqdb_log(int cl, const char * s1, int s2)
{
    if (seqdb_log_disabled(cl))
        return;

    (*seqdb_logfile) << s1 << s2 << endl;
}

void seqdb_log(int cl, const char * s1, int s2, const char * s3)
{
    if (seqdb_log_disabled(cl))
        return;

    (*seqdb_logfile) << s1 << s2 << s3 << endl;
}

void seqdb_log(int cl, const char * s1, int s2, const char * s3, int s4)
{
    if (seqdb_log_disabled(cl))
        return;

    (*seqdb_logfile) << s1 << s2 << s3 << s4 << endl;
}

void seqdb_log(int cl, const char * s1, int s2, const char * s3, int s4, const char * s5, int s6)
{
    if (seqdb_log_disabled(cl))
        return;

    (*seqdb_logfile) << s1 << s2 << s3 << s4 << s5 << s6 << endl;
}
#endif // SEQDB_TRACE_LOGFILE


// Further optimizations:

// 1. Regions could be stored in a map<>, sorted by file, then offset.
// This would allow a binary search instead of sequential and would
// vastly improve the "bad case" of 100_000s of buffers of file data.

// 2. "Scrounging" could be done in the file case.  It is bad to read
// 0-4096 then 4096 to 8192, then 4000-4220.  The third could use the
// first two to avoid reading.  It should either combine the first two
// regions into a new region, or else just copy to a new region and
// leave the old ones alone (possibly marking the old regions as high
// penalty).  Depending on refcnt, penalty, and region sizes.

// Throw function

void SeqDB_ThrowException(CSeqDBException::EErrCode code, const string & msg)
{
    switch(code) {
    case CSeqDBException::eArgErr:
        NCBI_THROW(CSeqDBException, eArgErr, msg);

    case CSeqDBException::eFileErr:
        NCBI_THROW(CSeqDBException, eFileErr, msg);

    default:
        NCBI_THROW(CSeqDBException, eMemErr, msg);
    }
}

/// Build and throw a file-not-found exception.
///
/// @param fname The name of the unfound file. [in]

static void s_SeqDB_FileNotFound(const string & fname)
{
    string msg("File [");
    msg += fname;
    msg += "] not found.";
    SeqDB_ThrowException(CSeqDBException::eFileErr, msg);
}


/// Check the size of a number relative to the scope of a numeric type.

template<class TIn, class TOut>
TOut SeqDB_CheckLength(TIn value)
{
    TOut result = TOut(value);

    if (sizeof(TOut) < sizeof(TIn)) {
        if (TIn(result) != value) {
            SeqDB_ThrowException(CSeqDBException::eFileErr,
                                 "Offset type does not span file length.");
        }
    }

    return result;
}

CSeqDBAtlas::CSeqDBAtlas(bool use_atlas_lock)
     :m_UseLock           (use_atlas_lock),
      m_CurAlloc          (0),
      m_Alloc             (false),
      m_MaxFileSize       (0),      
      m_SearchPath        (GenerateSearchPath())
{
    m_OpenedFilesCount = 0;
    m_MaxOpenedFilesCount = 0;
}

CSeqDBAtlas::~CSeqDBAtlas()
{
    // For now, and maybe permanently, enforce balance.

    _ASSERT(m_Pool.size() == 0);

    // Erase 'manually allocated' elements - In debug mode, this will
    // not execute, because of the above test.

    for(auto i = m_Pool.begin(); i != m_Pool.end(); i++) {
        delete[] (char*)((*i).first);
    }

    m_Pool.clear();
}

CMemoryFile* CSeqDBAtlas::GetMemoryFile(const string& fileName)
{
    {
        std::lock_guard<std::mutex> guard(m_FileMemMapMutex);
        auto it = m_FileMemMap.find(fileName);
        if (it != m_FileMemMap.end())
            return it->second.get();
    }

    unique_ptr<CMemoryFile> file(new CMemoryFile(fileName));

    {
        std::lock_guard<std::mutex> guard(m_FileMemMapMutex);
        auto it = m_FileMemMap.find(fileName);
        if (it != m_FileMemMap.end())
            return it->second.get();
        CMemoryFile* memFile = file.release();
        m_FileMemMap[fileName].reset(memFile);
        ChangeOpenedFilseCount(CSeqDBAtlas::eFileCounterIncrement);
        return memFile;
    }
}

bool CSeqDBAtlas::DoesFileExist(const string & fname, CSeqDBLockHold & locked)
{
    TIndx length(0);
    return GetFileSize(fname, length, locked);
}

bool CSeqDBAtlas::GetFileSize(const string   & fname,
                              TIndx          & length,
                              CSeqDBLockHold & locked)
{
    Lock(locked);
    

    return GetFileSizeL(fname, length);
}

bool CSeqDBAtlas::GetFileSizeL(const string & fname, TIndx &length)
{
    {
        std::lock_guard<std::mutex> guard(m_FileSizeMutex);
        auto it = m_FileSize.find(fname);
        if (it != m_FileSize.end()) {
            length = it->second.second;
            return it->second.first;
        }
    }

    pair<bool, TIndx> val;
    CFile whole(fname);
    Int8 file_length = whole.GetLength();

    if (file_length >= 0) {
        val.first = true;
        val.second = SeqDB_CheckLength<Int8, TIndx>(file_length);
    }
    else {
        val.first = false;
        val.second = 0;
    }

    {
        std::lock_guard<std::mutex> guard(m_FileSizeMutex);
        m_FileSize[fname] = val;

        if (file_length >= 0 && (Uint8)file_length > m_MaxFileSize)
            m_MaxFileSize = file_length;
    }

    length = val.second;
    return val.first;
}

/// Simple idiom for RIIA with malloc + free.
struct CSeqDBAutoFree {
    /// Constructor.
    CSeqDBAutoFree()
        : m_Array(0)
    {
    }

    /// Specify a malloced area of memory.
    void Set(const char * x)
    {
        m_Array = x;
    }

    /// Destructor will free that memory.
    ~CSeqDBAutoFree()
    {
        if (m_Array) {
            free((void*) m_Array);
        }
    }

private:
    /// Pointer to malloced memory.
    const char * m_Array;
};



/// Releases allocated memory
void CSeqDBAtlas::x_RetRegion(const char * datap)
{
    
    bool worked = x_Free(datap);
    _ASSERT(worked);

    if (! worked) {
        cerr << "Address leak in CSeqDBAtlas::RetRegion" << endl;
    }

}


char * CSeqDBAtlas::Alloc(size_t length, CSeqDBLockHold & locked, bool clear)
{
    // What should/will happen on allocation failure?

    Lock(locked);

    if (! length) {
        length = 1;
    }

    // Allocate/clear

    char * newcp = 0;

    try {
        newcp = new char[length];

        // new() should have thrown, but some old implementations are
        // said to be non-compliant in this regard:

        if (! newcp) {
            throw std::bad_alloc();
        }

        if (clear) {
            memset(newcp, 0, length);
        }
    }
    catch(std::bad_alloc) {
        NCBI_THROW(CSeqDBException, eMemErr,
                   "CSeqDBAtlas::Alloc: allocation failed.");
    }

    // Add to pool.

    _ASSERT(m_Pool.find(newcp) == m_Pool.end());

    m_Pool[newcp] = length;
    m_CurAlloc += length;
    m_Alloc = true;
    //cerr << "allocated " << m_CurAlloc << " memory" << endl;
    return newcp;
}


void CSeqDBAtlas::Free(const char * freeme, CSeqDBLockHold & locked)
{
    Lock(locked);

#ifdef _DEBUG
    bool found =
        x_Free(freeme);

    _ASSERT(found);
#else
    x_Free(freeme);
#endif
}


bool CSeqDBAtlas::x_Free(const char * freeme)
{
    if(!m_Alloc) return true;
    auto i = m_Pool.find((const char*) freeme);

    if (i == m_Pool.end()) {
        return false;
    }

    size_t sz = (*i).second;

    _ASSERT(m_CurAlloc >= (TIndx)sz);
    m_CurAlloc -= sz;    
    //cerr << "deallocated " << sz << " memory m_CurAlloc=" << m_CurAlloc << endl;
    if(m_CurAlloc == 0) m_Alloc = false;
    char * cp = (char*) freeme;
    delete[] cp;
    m_Pool.erase(i);

    return true;
}



void CSeqDBAtlas::RegisterExternal(CSeqDBMemReg   & memreg,
                                   size_t           bytes,
                                   CSeqDBLockHold & locked)
{
    if (bytes > 0) {
        Lock(locked);
        _ASSERT(memreg.m_Bytes == 0);        
	    memreg.m_Bytes = bytes;
    }
}

void CSeqDBAtlas::UnregisterExternal(CSeqDBMemReg & memreg)
{
    size_t bytes = memreg.m_Bytes;

    if (bytes > 0) {        
        memreg.m_Bytes = 0;
    }
}





CSeqDBAtlasHolder::CSeqDBAtlasHolder(CSeqDBLockHold * lockedp,
                                     bool use_atlas_lock)
    
{
    {{
    CFastMutexGuard guard(m_Lock);

    if (m_Count == 0) {
        m_Atlas = new CSeqDBAtlas(use_atlas_lock);
    }
    m_Count ++;
    }}
}


// FIXME: This constrctor is deprecated
CSeqDBAtlasHolder::CSeqDBAtlasHolder(bool use_atlas_lock,
                                     CSeqDBLockHold* locdep)
{
    {{
    CFastMutexGuard guard(m_Lock);

    if (m_Count == 0) {
        m_Atlas = new CSeqDBAtlas(use_atlas_lock);
    }
    m_Count ++;
    }}
}


DEFINE_CLASS_STATIC_FAST_MUTEX(CSeqDBAtlasHolder::m_Lock);

CSeqDBAtlasHolder::~CSeqDBAtlasHolder()
{
    
    CFastMutexGuard guard(m_Lock);
    m_Count --;

    if (m_Count == 0) {
        delete m_Atlas;
    }
}

CSeqDBAtlas & CSeqDBAtlasHolder::Get()
{
    _ASSERT(m_Atlas);
    return *m_Atlas;
}

CSeqDBLockHold::~CSeqDBLockHold()
{
    CHECK_MARKER();

    m_Atlas.Unlock(*this);
    BREAK_MARKER();
}

int CSeqDBAtlasHolder::m_Count = 0;
CSeqDBAtlas * CSeqDBAtlasHolder::m_Atlas = NULL;


CSeqDB_AtlasRegionHolder::
CSeqDB_AtlasRegionHolder(CSeqDBAtlas & atlas, const char * ptr)
    : m_Atlas(atlas), m_Ptr(ptr)
{
}

CSeqDB_AtlasRegionHolder::~CSeqDB_AtlasRegionHolder()
{
    if (m_Ptr) {
        CSeqDBLockHold locked(m_Atlas);
        m_Atlas.Lock(locked);

        //m_Atlas.RetRegion(m_Ptr);
        m_Ptr = NULL;
    }
}

END_NCBI_SCOPE
