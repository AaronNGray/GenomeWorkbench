#ifndef GUI_WIDGETS_ALN_MULTIPLE___ALNMULTI_DS__HPP
#define GUI_WIDGETS_ALN_MULTIPLE___ALNMULTI_DS__HPP

/*  $Id: alnmulti_ds.hpp 44485 2019-12-27 18:49:45Z shkeda $
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
 * Authors:  Mike DiCuccio, Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>

#include <util/range_coll.hpp>

#include <gui/objutils/ui_data_source_notif.hpp>

#include <objtools/alnmgr/aln_explorer.hpp>

#include <gui/widgets/aln_score/scoring_method.hpp>

#include <objects/seqloc/Seq_loc.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/scope.hpp>


BEGIN_NCBI_SCOPE


/// IAlignRowHandle provides an abstract way to access alignment row data.
///
/// IAlignRowHandle-inherited classes are supposed to provide proprietary
/// interface depending on the nature of data being represented.
class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT IAlignRowHandle
{
public:
    typedef IAlnExplorer::TNumrow     TNumrow;
    typedef IAlnExplorer::TResidue    TResidue;

public:
    virtual TNumrow GetRowNum() const = 0;

    virtual TSignedSeqPos   GetSeqAlnStart() const = 0;
    virtual TSignedSeqPos   GetSeqAlnStop() const = 0;
    virtual IAlnExplorer::TSignedRange    GetSeqAlnRange() const = 0;

    virtual TSignedSeqPos   GetSeqStart() const = 0;
    virtual TSignedSeqPos   GetSeqStop() const = 0;
    virtual bool            IsNegativeStrand() const = 0;

    virtual TSignedSeqPos  GetAlnPosFromSeqPos(TSeqPos seq_pos,
                                IAlnExplorer::ESearchDirection dir = IAlnExplorer::eNone,
                                bool try_reverse_dir = true) const = 0;
    virtual TSignedSeqPos  GetSeqPosFromAlnPos(TSeqPos aln_pos,
                                IAlnExplorer::ESearchDirection dir = IAlnExplorer::eNone,
                                bool try_reverse_dir = true) const = 0;

    virtual IAlnSegmentIterator*
        CreateSegmentIterator(const IAlnExplorer::TSignedRange& range,
                              IAlnSegmentIterator::EFlags flags) const = 0;

    virtual bool     CanGetSeqString() const = 0;
    virtual string&  GetSeqString (string &buffer, const IAlnExplorer::TRange &seq_rng) const = 0;
    virtual string&  GetAlnSeqString(string &buffer, const IAlnExplorer::TSignedRange &aln_rng) const = 0;


    virtual bool    CanGetBioseqHandle() const = 0;
    virtual const objects::CBioseq_Handle&  GetBioseqHandle() const = 0;
    virtual const objects::CSeq_id& GetSeq_id() const = 0;
    virtual objects::CScope& GetScope() const = 0;

	virtual bool	UsesAATranslation() const = 0;

    // nuc-prot alignemnt
    virtual bool IsNucProtAlignment() const = 0;

    virtual size_t GetBaseWidth() const = 0;

    virtual bool IsAnchor() const
    {
        return false;
    }
    virtual bool IsConsensus() const;


protected:
    virtual ~IAlignRowHandle() {}
};

////////////////////////////////////////////////////////////////////////////////
/// Class  CConsensusRow - Simple interface to a row that represents the 
///                         consensus for rendering purposes
///
/// Rows need access to a consensus row, which may be a consensus or selected
/// master row, for rendering. It gives the row the option to not render characters
/// when they match the consensus
class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CConsensusRow :
    public CObjectEx
{
public:
    typedef IAlnExplorer::TSignedRange    TSignedRange;

public:
    CConsensusRow(const IAlignRowHandle* handle)
        : m_Handle(handle)
    {}
    ~CConsensusRow() {}

    const IAlignRowHandle& GetHandle() const { return *m_Handle; }
    void SetHandle(const IAlignRowHandle* h)  { m_Handle = h; }

    bool IsValid() const { return m_Handle != NULL; }

    virtual string& GetAlnSeqString(string &buffer, const IAlnExplorer::TSignedRange &aln_rng) const;
    virtual string& GetStringAtPos(string& buffer, TSignedSeqPos aln_pos) const;

protected:
    const IAlignRowHandle*  m_Handle;
};

inline string& CConsensusRow::GetAlnSeqString(string &buffer, const IAlnExplorer::TSignedRange &aln_rng) const
{
    if (!m_Handle)
        return buffer;

    return m_Handle->GetAlnSeqString(buffer, aln_rng);
}

inline string& CConsensusRow::GetStringAtPos(string& buffer, TSignedSeqPos aln_pos) const
{
    if (!m_Handle)
        return buffer;

    if (m_Handle->UsesAATranslation() && m_Handle->IsNucProtAlignment()) {
        auto pos = m_Handle->GetSeqPosFromAlnPos(aln_pos, IAlnExplorer::eRight);
        pos = (pos / 3) * 3;
        return m_Handle->GetSeqString(buffer, IAlnExplorer::TRange(pos, pos + 3));
    }
    else {
        return GetAlnSeqString(buffer, IAlnExplorer::TSignedRange(aln_pos, aln_pos));
    }
}


///////////////////////////////////////////////////////////////////////////////
/// IAlnMultiDataSource - interface to a data source representing an abstract
/// multiple alignment.
class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT IAlnMultiDataSource
    :   public CDataChangeNotifier,
        public IScoringAlignment
{
public:
    typedef IAlnExplorer::TNumrow    TNumrow;
    typedef IAlnExplorer::TResidue   TResidue;
    typedef IAlnExplorer::ESearchDirection   TSearchDirection;
    typedef CRangeCollection<TSeqPos>   TRangeColl;


public:
    virtual bool    IsEmpty() const = 0;

    virtual void  SetGapChar(TResidue gap_char) = 0;

    virtual IAlnExplorer::EAlignType  GetAlignType() const = 0;

    /// number of rows in alignment
    virtual TNumrow  GetNumRows(void) const = 0;

    virtual TSeqPos  GetAlnStart(void) const = 0;
    virtual TSeqPos  GetAlnStop(void) const = 0;

    virtual TSeqPos GetSeqStart(TNumrow row) const = 0;
    virtual TSeqPos GetSeqStop(TNumrow row) const = 0;

    /// returns index of the Consensus row or -1 if it doesn't exist
    virtual int     GetConsensusRow() const = 0;

    virtual bool    CanCreateConsensus() = 0;
    virtual void    CreateConsensus() = 0;
    virtual void    SetCreateConsensus(bool b) = 0;

    virtual const IAlignRowHandle*  GetRowHandle(TNumrow row) const = 0;

    /// Anchoring methods
    virtual bool  IsSetAnchor(void) const = 0;
    virtual TNumrow  GetAnchor(void) const = 0;
    virtual bool  CanChangeAnchor(void) const = 0;
    virtual bool  SetAnchor(TNumrow anchor) = 0;
    virtual bool  UnsetAnchor(void) = 0;

    virtual bool    CanGetId(TNumrow row) const = 0;
    virtual const objects::CSeq_id&  GetSeqId(TNumrow row) const = 0;

    virtual bool    IsPositiveStrand(TNumrow row) const = 0;
    virtual bool    IsNegativeStrand(TNumrow row) const = 0;

    virtual TSignedSeqPos  GetSeqPosFromAlnPos(TNumrow for_row, TSeqPos aln_pos,
                                IAlnExplorer::ESearchDirection dir = IAlnExplorer::eNone,
                                bool try_reverse_dir = true) const = 0;

    virtual TSignedSeqPos  GetAlnPosFromSeqPos(TNumrow row, TSeqPos seq_pos,
                                IAlnExplorer::ESearchDirection dir = IAlnExplorer::eNone,
                                bool try_reverse_dir = true) const = 0;

    virtual string& GetAlnSeqString(TNumrow row, string &buffer,
                                    const IAlnExplorer::TSignedRange& aln_range) const = 0;

    virtual IAlnSegmentIterator*    CreateSegmentIterator(TNumrow row,
                                       const IAlnExplorer::TSignedRange& range,
                                       IAlnSegmentIterator::EFlags flags) const = 0;

    virtual void  GetAlnFromSeq(TNumrow row, const TRangeColl& seq_coll,
                                TRangeColl& aln_coll) const = 0;

    virtual void  GetSeqFromAln(TNumrow row, const TRangeColl& aln_coll,
                                TRangeColl& seq_coll) const = 0;

    virtual bool  IsDataReadSync() const {return false;}

    using TStatGraphs = map<char, vector<unsigned>>;
    void CollectAlignStats(const TSignedSeqRange& range, TStatGraphs& graphs, bool translate_sequence = false) const;
    
};


inline
bool IAlignRowHandle::IsConsensus() const
{
    return (GetSeq_id().GetSeqIdString() == "consensus");
}


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_ALN_MULTIPLE___ALNMULTI_DS__HPP
