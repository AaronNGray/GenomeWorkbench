#ifndef OBJTOOLS_DATA_LOADERS_CSRA___CSRALOADER_IMPL__HPP
#define OBJTOOLS_DATA_LOADERS_CSRA___CSRALOADER_IMPL__HPP

/*  $Id: csraloader_impl.hpp 562583 2018-04-24 16:07:23Z vasilche $
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
 * Author: Eugene Vasilchenko
 *
 * File Description: CSRA file data loader
 *
 * ===========================================================================
 */


#include <corelib/ncbistd.hpp>
#include <corelib/ncbimtx.hpp>
#include <sra/data_loaders/csra/csraloader.hpp>
#include <sra/readers/sra/csraread.hpp>
#include <objtools/readers/iidmapper.hpp>
#include <util/limited_size_map.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)

class CDataLoader;
class CCSRADataLoader_Impl;
class CCSRARefSeqChunkInfo;
class CCSRARefSeqInfo;
class CCSRAFileInfo;

template<class Key, class Value, class Less = less<Key> >
class CCacheWithLock : public CObject
{
public:
    typedef Key key_type;
    typedef Value mapped_type;

protected:
    class CSlot;
    typedef Less TLess;
    typedef map<key_type, CRef<CSlot>, TLess> TMap;
    typedef typename TMap::iterator TMapIterator;
    typedef typename TMap::const_iterator TMapConstIterator;
    typedef list<TMapIterator> TRemoveList;
    typedef typename TRemoveList::iterator TRemoveListIterator;

    class CSlot : public CObject {
    public:
        CSlot() {
            m_LockCounter.Set(1);
        }
        TMapIterator        m_MapIter;
        TRemoveListIterator m_RemoveListIter;
        CAtomicCounter      m_LockCounter;
        CFastMutex          m_ValueMutex;
        mapped_type         m_Value;
    };

    TMap m_Map;
    size_t m_SizeLimit;
    size_t m_RemoveSize;
    TRemoveList m_RemoveList;
    CMutex m_Mutex;

public:
    class CLock {
    protected:
        CRef<CCacheWithLock> m_Cache;
        CRef<CSlot> m_Slot;
        friend class CCacheWithLock<key_type, mapped_type, TLess>;
        
        CLock(CCacheWithLock* cache, CSlot* slot)
            : m_Cache(cache),
              m_Slot(slot)
            {
                _ASSERT(cache);
                _ASSERT(slot->m_LockCounter.Get() > 0);
            }
        
    public:
        CLock() {
        }
        ~CLock() {
            Reset();
        }
        CLock(const CLock& lock)
            : m_Cache(lock.m_Cache),
              m_Slot(lock.m_Slot)
            {
                if ( m_Slot ) {
                    m_Slot->m_LockCounter.Add(1);
                }
            }
        CLock& operator=(const CLock& lock)
            {
                if ( m_Slot != lock.m_Slot ) {
                    if ( m_Slot ) {
                        m_Cache->Unlock(m_Slot);
                    }
                    m_Cache = lock.m_Cache;
                    m_Slot = lock.m_Slot;
                    if ( m_Slot ) {
                        m_Slot->m_LockCounter.Add(1);
                    }
                }
                return *this;
            }
        CLock(CLock&& lock)
            : m_Cache(move(lock.m_Cache)),
              m_Slot(move(lock.m_Slot))
            {
            }
        CLock& operator=(CLock&& lock)
            {
                if ( m_Slot != lock.m_Slot ) {
                    Reset();
                    m_Cache.Swap(lock.m_Cache);
                    m_Slot.Swap(lock.m_Slot);
                }
                return *this;
            }

        void Reset() {
            if ( m_Slot ) {
                m_Cache->Unlock(m_Slot);
                m_Slot = null;
                m_Cache = null;
            }
        }

        CFastMutex& GetValueMutex() { return m_Slot.GetNCObject().m_ValueMutex; }
        
        mapped_type& operator*() const { return m_Slot.GetNCObject().m_Value; }
        mapped_type* operator->() const { return m_Slot.GetNCPointer().m_Value; }
        
        bool operator==(CLock a) const {
            return m_Slot == a.m_Slot;
        }
        bool operator!=(CLock a) const {
            return !(*this == a);
        }
    };

    CCacheWithLock(size_t size_limit = 0)
        : m_SizeLimit(size_limit),
          m_RemoveSize(0)
        {
        }
    
    CLock get_lock(const key_type& key) {
        CMutexGuard guard(m_Mutex);
        TMapIterator iter = m_Map.lower_bound(key);
        if ( iter == m_Map.end() || m_Map.key_comp()(key, iter->first) ) {
            // insert
            typedef typename TMap::value_type TValue;
            iter = m_Map.insert(iter, TValue(key, Ref(new CSlot())));
            iter->second->m_MapIter = iter;
        }
        else if ( iter->second->m_LockCounter.Add(1) == 1 ) {
            // first lock from remove list
            _ASSERT(m_RemoveSize > 0);
            _ASSERT(m_RemoveSize == m_RemoveList.size());
            m_RemoveList.erase(iter->second->m_RemoveListIter);
            --m_RemoveSize;
        }
        return CLock(this, iter->second);
    }

    size_t get_size_limit(void) const {
        return m_SizeLimit;
    }
    void set_size_limit(size_t size_limit) {
        if ( size_limit != m_SizeLimit ) {
            CMutexGuard guard(m_Mutex);
            m_SizeLimit = size_limit;
            x_GC();
        }
    }

protected:
    void Unlock(CSlot* slot) {
        CMutexGuard guard(m_Mutex);
        _ASSERT(slot);
        _ASSERT(slot->m_MapIter->second == slot);
        if ( slot->m_LockCounter.Add(-1) == 0 ) {
            // last lock removed
            slot->m_RemoveListIter =
                m_RemoveList.insert(m_RemoveList.end(), slot->m_MapIter);
            ++m_RemoveSize;
            x_GC();
        }
    }
    
    void x_GC() {
        while ( m_RemoveSize > m_SizeLimit ) {
            m_Map.erase(m_RemoveList.front());
            m_RemoveList.pop_front();
            --m_RemoveSize;
        }
    }
    
public:
};


class CCSRABlobId : public CBlobId
{
public:
    enum EBlobType {
        eBlobType_annot, // refseq coverage/pileup graphs and alignments
        eBlobType_refseq, // refseq itself
        eBlobType_reads, // short reads
        eBlobType_reads_align // short reads primary alignments
    };
    typedef CCacheWithLock<string, CRef<CCSRAFileInfo> > TSRRFiles;
    typedef pair<CRef<CCSRAFileInfo>, TSRRFiles::CLock> TFileLock;
    typedef pair<CRef<CCSRARefSeqInfo>, TSRRFiles::CLock> TRefLock;

    explicit CCSRABlobId(const CTempString& str);
    CCSRABlobId(EBlobType blob_type,
                const TRefLock& ref);
    CCSRABlobId(const TFileLock& file,
                TVDBRowId first_spot_id);
    ~CCSRABlobId(void);

    EBlobType m_BlobType;
    CCSraDb::ERefIdType m_RefIdType;
    // cSRA file name or SRR accession
    string m_File;
    // Ref Seq-id for annot blobs
    // First short read Seq-id for reads' blobs
    CSeq_id_Handle m_SeqId;
    TVDBRowId m_FirstSpotId;
    TSRRFiles::CLock m_FileLock;

    // returns length of accession part or NPOS
    static SIZE_TYPE ParseReadId(CTempString str,
                                 TVDBRowId* spot_id_ptr = 0,
                                 Uint4* read_id_ptr = 0);
    static bool GetGeneralSRAAccLabel(const CSeq_id_Handle& idh,
                                      string* srr_acc_ptr = 0,
                                      string* label_ptr = 0);
    static bool GetGeneralSRAAccReadId(const CSeq_id_Handle& idh,
                                       string* srr_acc_ptr = 0,
                                       TVDBRowId* spot_id_ptr = 0,
                                       Uint4* read_id_ptr = 0);

    enum EGeneralIdType {
        eNotGeneralIdType      = 0,
        eGeneralIdType_refseq  = 1<<0,
        eGeneralIdType_read    = 1<<1,
        eGeneralIdType_both    = eGeneralIdType_refseq|eGeneralIdType_read
    };
    static EGeneralIdType GetGeneralIdType(const CSeq_id_Handle& idh,
                                           EGeneralIdType allow_type,
                                           const string* srr = 0);
    static EGeneralIdType GetGeneralIdType(const CSeq_id_Handle& idh,
                                           EGeneralIdType allow_type,
                                           const string& srr)
    {
        return GetGeneralIdType(idh, allow_type, &srr);
    }

    // string blob id representation:
    // eBlobType_annot_plain_id
    string ToString(void) const;
    void FromString(CTempString str);

    bool operator<(const CBlobId& id) const;
    bool operator==(const CBlobId& id) const;
};


class CCSRARefSeqChunkInfo
{
public:
    typedef CRange<TSeqPos> TRange;

    const TRange& GetRefSeqRangeStart(void) const
        {
            return m_RefSeqRangeStart;
        }

protected:
    friend class CCSRARefSeqInfo;

    TRange m_RefSeqRangeStart; // range of alignments' start positions
};


enum ECSRAAnnotChunkIdType {
    eCSRAAnnotChunk_align,
    eCSRAAnnotChunk_pileup_graph,
    eCSRAAnnotChunk_mul
};

    
class CCSRARefSeqInfo : public CObject
{
public:
    CCSRARefSeqInfo(CCSRAFileInfo* csra_file,
                    const CSeq_id_Handle& seq_id);

    const CSeq_id_Handle& GetRefSeqId(void) const
        {
            return m_RefSeqId;
        }

    CCSraRefSeqIterator GetRefSeqIterator(void) const;

    //CRef<CCSRABlobId> GetBlobId(CCSRABlobId::EBlobType type) const;
    int GetAnnotChunkId(TSeqPos ref_pos) const;

    void LoadRanges(void);

    void LoadAnnotBlob(CTSE_LoadLock& load_lock);
    void LoadAnnotChunk(CTSE_Chunk_Info& chunk_info);

    void LoadAnnotMainSplit(CTSE_LoadLock& load_lock);
    void LoadAnnotMainChunk(CTSE_Chunk_Info& chunk_info);
    void LoadAnnotAlignChunk(CTSE_Chunk_Info& chunk_info);
    void LoadAnnotPileupChunk(CTSE_Chunk_Info& chunk_info);

    void LoadRefSeqBlob(CTSE_LoadLock& load_lock);
    void LoadRefSeqChunk(CTSE_Chunk_Info& chunk_info);

    void LoadRefSeqMainEntry(CTSE_LoadLock& load_lock);

protected:
    friend class CCSRADataLoader_Impl;
    friend class CCSRABlobId;

    // start of chunk and number of alignments in the chunk
    struct SChunkInfo {
        TSeqPos start_pos;
        unsigned align_count;

        bool operator()(TSeqPos pos, const SChunkInfo& chunk) const
            { return pos < chunk.start_pos; }
    };
    typedef vector<SChunkInfo> TChunks;

    void x_LoadRangesStat(void);

    CCSRAFileInfo* m_File;
    CSeq_id_Handle m_RefSeqId;
    CRef<CSeq_annot> m_CovAnnot;
    int m_MinMapQuality;
    TChunks m_AlignChunks;
    TChunks m_GraphChunks;
};


class CCSRAFileInfo : public CObject
{
public:
    CCSRAFileInfo(CCSRADataLoader_Impl& impl,
                  const string& csra,
                  CCSraDb::ERefIdType ref_id_type);
    
    const string& GetCSRAName(void) const
        {
            return m_CSRAName;
        }
    const string& GetBaseAnnotName(void) const
        {
            return m_AnnotName;
        }
    string GetAnnotName(const string& spot_group,
                        ECSRAAnnotChunkIdType type) const;
    string GetAlignAnnotName(void) const;
    string GetAlignAnnotName(const string& spot_group) const;
    string GetPileupAnnotName(void) const;
    string GetPileupAnnotName(const string& spot_group) const;

    CCSraDb::ERefIdType GetRefIdType(void) const
        {
            return m_RefIdType;
        }
    int GetMinMapQuality(void) const
        {
            return m_MinMapQuality;
        }
    bool GetPileupGraphs(void) const
        {
            return m_PileupGraphs;
        }
    bool GetQualityGraphs(void) const
        {
            return m_QualityGraphs;
        }

    bool IsValidReadId(TVDBRowId spot_id, Uint4 read_id,
                       CRef<CCSRARefSeqInfo>* ref_ptr = 0,
                       TSeqPos* ref_pos_ptr = 0);
    //CRef<CCSRABlobId> GetReadsBlobId(TVDBRowId spot_id) const;

    void GetAnnotBlobId(CRef<CCSRABlobId>& ret,
                        const CSeq_id_Handle& idh);

    CRef<CCSRARefSeqInfo> GetRefSeqInfo(const CSeq_id_Handle& seq_id);
    CRef<CCSRARefSeqInfo> GetRefSeqInfo(const CCSRABlobId& blob_id)
        {
            return GetRefSeqInfo(blob_id.m_SeqId);
        }

    CCSraDb& GetDb(void)
        {
            return m_CSRADb;
        }
    operator CCSraDb&(void)
        {
            return GetDb();
        }

    void AddRefSeq(const string& refseq_label,
                   const CSeq_id_Handle& refseq_id);

    const vector<string>& GetSeparateSpotGroups(void) const
        {
            return m_SeparateSpotGroups;
        }

    typedef CCSRADataLoader::TAnnotNames TAnnotNames;
    void GetPossibleAnnotNames(TAnnotNames& names) const;
    
    void LoadReadsBlob(const CCSRABlobId& blob_id,
                       CTSE_LoadLock& load_lock);

protected:
    friend class CCSRADataLoader_Impl;

    typedef map<CSeq_id_Handle, CRef<CCSRARefSeqInfo> > TRefSeqs;

    void x_Initialize(CCSRADataLoader_Impl& impl,
                      const string& csra,
                      CCSraDb::ERefIdType ref_id_type);

    string m_CSRAName;
    CCSraDb::ERefIdType m_RefIdType;
    string m_AnnotName;
    int m_MinMapQuality;
    bool m_PileupGraphs;
    bool m_QualityGraphs;
    CCSraDb m_CSRADb;
    vector<string> m_SeparateSpotGroups;
    TRefSeqs m_RefSeqs;
};


class CCSRADataLoader_Impl : public CObject
{
public:
    explicit CCSRADataLoader_Impl(const CCSRADataLoader::SLoaderParams& params);
    ~CCSRADataLoader_Impl(void);

    void AddSrzDef(void);
    void AddCSRAFile(const string& csra);
    
    typedef CCacheWithLock<string, CRef<CCSRAFileInfo> > TSRRFiles;
    typedef pair<CRef<CCSRAFileInfo>, TSRRFiles::CLock> TFileLock;
    typedef pair<CRef<CCSRARefSeqInfo>, TSRRFiles::CLock> TRefLock;
    
    TFileLock GetSRRFile(const string& acc);

    int GetMinMapQuality(void) const
        {
            return m_MinMapQuality;
        }
    bool GetPileupGraphs(void) const
        {
            return m_PileupGraphs;
        }
    bool GetQualityGraphs(void) const
        {
            return m_QualityGraphs;
        }
    bool GetSpotReadAlign(void) const
        {
            return m_SpotReadAlign;
        }
    void SetSpotReadAlign(bool value);
    int GetPathInId(void) const
        {
            return m_PathInId;
        }
    int GetSpotGroups(void) const
        {
            return m_SpotGroups;
        }

    TRefLock GetRefSeqInfo(const CSeq_id_Handle& idh);
    TFileLock GetReadsFileInfo(const CSeq_id_Handle& idh,
                               TVDBRowId* spot_id_ptr = 0,
                               Uint4* read_id_ptr = 0,
                               CRef<CCSRARefSeqInfo>* ref_ptr = 0,
                               TSeqPos* ref_pos_ptr = 0);
    TFileLock GetFileInfo(const CCSRABlobId& blob_id);
    CCSraRefSeqIterator GetRefSeqIterator(const CSeq_id_Handle& idh);
    CCSraShortReadIterator GetShortReadIterator(const CSeq_id_Handle& idh);

    CDataLoader::TTSE_LockSet GetRecords(CDataSource* data_source,
                                         const CSeq_id_Handle& idh,
                                         CDataLoader::EChoice choice);
    CRef<CCSRABlobId> GetBlobId(const CSeq_id_Handle& idh);
    CRef<CCSRABlobId> GetBlobId(const TRefLock& lock, CCSRABlobId::EBlobType type);
    CRef<CCSRABlobId> GetReadsBlobId(const TFileLock& lock, TVDBRowId spot_id);
    CTSE_LoadLock GetBlobById(CDataSource* data_source,
                              const CCSRABlobId& blob_id);
    void LoadBlob(const CCSRABlobId& blob_id,
                  CTSE_LoadLock& load_lock);
    void LoadChunk(const CCSRABlobId& blob_id,
                   CTSE_Chunk_Info& chunk);

    typedef CCSRADataLoader::TAnnotNames TAnnotNames;
    TAnnotNames GetPossibleAnnotNames(void) const;

    typedef vector<CSeq_id_Handle> TIds;
    void GetIds(const CSeq_id_Handle& idh, TIds& ids);
    CDataSource::SAccVerFound GetAccVer(const CSeq_id_Handle& idh);
    CDataSource::SGiFound GetGi(const CSeq_id_Handle& idh);
    string GetLabel(const CSeq_id_Handle& idh);
    int GetTaxId(const CSeq_id_Handle& idh);
    TSeqPos GetSequenceLength(const CSeq_id_Handle& idh);
    CDataSource::STypeFound GetSequenceType(const CSeq_id_Handle& idh);

protected:
    friend class CCSRAFileInfo;
    struct SDirSeqInfo {
        CSeq_id_Handle m_SeqId;
        string m_CSRAFileName;
        string m_CSRASeqLabel;
        string m_Label;
    };

private:
    // first:
    //   false if explicitly listed file in the loader params
    //   true if dynamically loaded SRA
    // second: SRA accession or csra file path
        
    typedef map<string, CRef<CCSRAFileInfo> > TFixedFiles;

    // mutex guarding input into the map
    CVDBMgr m_Mgr;
    string  m_DirPath;
    int m_MinMapQuality;
    bool m_PileupGraphs;
    bool m_QualityGraphs;
    bool m_SpotReadAlign;
    int m_PathInId;
    int m_SpotGroups;
    TFixedFiles m_FixedFiles;
    CRef<TSRRFiles> m_SRRFiles;
    AutoPtr<IIdMapper> m_IdMapper;
    string m_AnnotName;
};

END_SCOPE(objects)
END_NCBI_SCOPE

#endif  // OBJTOOLS_DATA_LOADERS_CSRA___CSRALOADER_IMPL__HPP
