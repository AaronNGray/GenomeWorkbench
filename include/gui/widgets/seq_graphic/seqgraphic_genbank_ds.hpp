#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_GENBANK_DS__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_GENBANK_DS__HPP

/*  $Id: seqgraphic_genbank_ds.hpp 38204 2017-04-10 20:46:50Z rudnev $
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
 * Authors:  Vlad Lebedev, Liangshou Wu
 *
 * File Description:
 *
 */

#include <corelib/ncbimtx.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/seq_vector.hpp>

#include <gui/gui_export.h>
#include <gui/utils/event_translator.hpp>
#include <gui/utils/extension.hpp>

#include <gui/widgets/seq_graphic/seqgraphic_data_source.hpp>
#include <gui/widgets/seq_graphic/coord_mapper.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CSGGenBankDS
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGGenBankDS :
    public CSGDataSource
{
public:
    typedef CAppJobDispatcher::TJobID   TJobID;
    typedef int   TJobToken;

    CSGGenBankDS(objects::CScope& scope, const objects::CSeq_id& id);

    virtual ~CSGGenBankDS();

    bool IsRefSeq() const;

    /// Set the annotation selector resolving depth.
    void SetDepth(int depth);
    int GetDepth() const;

    void SetAdaptive(bool flag);
    bool GetAdaptive() const;

    /// Get the underlying bioseq handle.
    objects::CBioseq_Handle&    GetBioseqHandle(void);

    const objects::CBioseq_Handle&    GetBioseqHandle(void) const;

    /// Get the scope from the handle.
    objects::CScope&           GetScope(void) const;

    /// Set JobDispatcher listener.
    void SetJobListener(CEventHandler* listener);

    virtual void ClearJobID(TJobID job_id);
    bool IsJobNeeded(TJobID job_id) const;
    virtual bool AllJobsFinished() const;
    /// Remove waiting jobs from queue or cancel the unfinished jobs.
    virtual void DeleteAllJobs();

    void SetCoordMapper(ICoordMapper* mapper);

    /// @name help methods about coverage graph levels.
    /// @{
    /// Return the nearest level available for a given zoom level.
    /// @param zoom is measured as number of bases per screen pixel
    /// Initialize levels on which coverage graphs are avaiable.
    /// @param annot the original annotation name
    void SetGraphLevels(const string& annot);
    /// @}

protected:

    void x_ForegroundJob(IAppJob& job);

    /// Jobs on thread pool.
    TJobID x_BackgroundJob(IAppJob& job, int report_period, const string& pool);

    /// Launch either a background or foreground job.
    /// The pool is used only for background jobs.
    TJobID x_LaunchJob(IAppJob& job, int report_period = 1,
        const string& pool = "ObjManagerEngine");

private:
    /// @name Forbidden methods
    /// @{
    CSGGenBankDS(const CSGGenBankDS&);
    CSGGenBankDS& operator=(const CSGGenBankDS&);
    /// @}

protected:

    typedef set<int> TGraphLevels;

    objects::CBioseq_Handle m_Handle;
    CEventHandler*  m_JobListener;   ///< our listener to JobDispatcher events
    vector<TJobID>  m_ActiveJobs;    ///< all currently active jobs for this DS
    int             m_Depth;         ///< annotation resolving depth
    bool            m_Adaptive;      ///< adaptive/exact annot selector
    /// coordinate mapper between mapped features coord. and sequence coord.
    CIRef<ICoordMapper> m_Mapper;

    /// Existing coverage graph levels.
    /// There is a feature in ID2 to store multiple zoom level
    /// representations for the same annotation track. This is
    /// used to avoid retrieving all the annotation details while
    /// exploring data at a high level. Viewer should be able to
    /// decide data for what zoom level to request depending on
    /// its current resolution.
    /// m_GraphLevels stores a list of available zoom levels
    /// sorted from low to high.
    TGraphLevels     m_GraphLevels;
};


///////////////////////////////////////////////////////////////////////////////
/// CSGSequenceDS
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGSequenceDS :
    public CObjectEx,
    public ISGDataSource
{
public:
    CSGSequenceDS(objects::CScope& scope, const objects::CSeq_id& id);
    virtual ~CSGSequenceDS();

    /// Get the underlying bioseq handle.
    objects::CBioseq_Handle    GetBioseqHandle(void) const;

    objects::CSeq_id_Handle    GetBestIdHandle() const;

    /// Get the scope from the handle.
    objects::CScope&           GetScope(void) const;

    const objects::CSeqVector& GetSeqVector(void) const;

    TSeqPos GetSequenceLength() const;

    /// retrieve a string representing the IUPAC characters for the sequence [from to].
    void GetSequence(TSeqPos from, TSeqPos to, string& buffer) const;

    string GetTitle() const;

    /// this is more suitable for display purposes since the returned string will not always be good
    /// to reconstruct the CSeq_id back (in case of gnl| or lcl|, see SV-2982)
    string GetAcc_Best() const;
    /// all accession info we can get in FASTA format
    string GetAcc_All() const;

    bool IsAccGenomic(objects::CSeq_id::EAccessionInfo acc_info) const;
    bool IsTraceAssembly() const;
    bool IsPopSet() const;
    bool IsRefSeq() const;
    bool IsRefSeqGene() const;


private:
    objects::CBioseq_Handle     m_Handle;
    objects::CSeq_id_Handle     m_Best_idh;  ///< best id handle (obtained with sequence::GetId(..., sequence::eGetId_Best))
    CRef<objects::CSeqVector>   m_SeqVector;
};

///////////////////////////////////////////////////////////////////////////////
/// CSGSequenceDSType
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGSequenceDSType :
    public CObject,
    public ISGDataSourceType,
    public IExtension
{
public:
    /// create an instance of the layout track type using default settings.
    virtual ISGDataSource* CreateDS(SConstScopedObject& object) const;

    /// @name IExtension interface implementation
    /// @{
    virtual string GetExtensionIdentifier() const;
    virtual string GetExtensionLabel() const;
    /// @}

    /// check if the data source can be shared.
    virtual bool IsSharable() const;
};


///////////////////////////////////////////////////////////////////////////////
// CSGGenBankDS inline methods
///
inline
void CSGGenBankDS::SetDepth(int depth)
{
    m_Depth = depth;
}

inline
int CSGGenBankDS::GetDepth() const
{
    return m_Depth;
}

inline
void CSGGenBankDS::SetAdaptive(bool flag)
{
    m_Adaptive = flag;
}

inline
bool CSGGenBankDS::GetAdaptive() const
{
    return m_Adaptive;
}

inline
objects::CBioseq_Handle& CSGGenBankDS::GetBioseqHandle(void)
{
    return m_Handle;
}

inline
const objects::CBioseq_Handle& CSGGenBankDS::GetBioseqHandle(void) const
{
    return m_Handle;
}

inline
objects::CScope& CSGGenBankDS::GetScope(void) const
{
    return m_Handle.GetScope();
}

inline
void CSGGenBankDS::SetJobListener(CEventHandler* listener)
{
    m_JobListener = listener;
}

inline
bool CSGGenBankDS::IsJobNeeded(TJobID job_id) const
{
    return find(m_ActiveJobs.begin(), m_ActiveJobs.end(), job_id)
        != m_ActiveJobs.end();
}

inline
bool CSGGenBankDS::AllJobsFinished() const
{
    return m_ActiveJobs.empty();
}

inline
void CSGGenBankDS::SetCoordMapper(ICoordMapper* mapper)
{
    m_Mapper.Reset(mapper);
}


///////////////////////////////////////////////////////////////////////////////
// CSGSequenceDS inline methods
///
inline
objects::CBioseq_Handle CSGSequenceDS::GetBioseqHandle(void) const
{
    return m_Handle;
}

inline
objects::CSeq_id_Handle CSGSequenceDS::GetBestIdHandle() const
{
    return m_Best_idh;
}

inline
objects::CScope& CSGSequenceDS::GetScope(void) const
{
    return m_Handle.GetScope();
}


inline
const objects::CSeqVector& CSGSequenceDS::GetSeqVector(void) const
{
    return *m_SeqVector;
}

inline
TSeqPos CSGSequenceDS::GetSequenceLength() const
{
    return m_Handle.GetBioseqLength();
}


END_NCBI_SCOPE

#endif  /* GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_GENBANK_DS__HPP */
