/* $Id: vectorscreen.hpp 44526 2020-01-13 15:10:28Z choi $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Yoon Choi
 */
#ifndef _VECTORSCREEN_H_
#define _VECTORSCREEN_H_

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include <objmgr/seq_entry_handle.hpp>
#include <algo/blast/api/uniform_search.hpp>

#include <wx/gauge.h>

/*!
 * Includes
 */

////@begin includes
////@end includes


/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations


BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
////@end control identifiers


struct SRangeMatch
{
    CRange<TSeqPos>   m_range;
    string            m_match_type;

    // Used only for sorting by m_match_type
    enum EMatchType {
        // Ascending order of display, e.g., strong before weak
        // Descending order of "strength"
        eStrong,
        eModerate,
        eWeak,
        eSuspect,
        eUnknown
    };
};


struct SVecscreenResult
{
    // Fasta format seqid string
    string              m_FastaSeqid;

    // Individual hit location and type
    vector<SRangeMatch> m_arrRangeMatch;

    // Sequence length 
    TSeqPos             m_SeqLen;

    SVecscreenResult(const string& sFastaSeqid,
                     const vector<SRangeMatch>& arrRangeMatch,
                     TSeqPos seqlen)
      : m_FastaSeqid(sFastaSeqid),
        m_arrRangeMatch(arrRangeMatch),
        m_SeqLen(seqlen)
    {}
};


// Re-organize CVecscreenRun::SVecscreenSummary around seqid
typedef map<const CSeq_id*, SVecscreenResult> TVecscreenSummaryBySeqid; 


/*!
 * CVectorScreen class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CVectorScreen
{    
public:
    /// Constructors
    CVectorScreen();

    /// Destructor
    ~CVectorScreen();

    void Run(objects::CSeq_entry_Handle seh, wxGauge& progress);

    const vector<CVecscreenRun::SVecscreenSummary>& 
        GetVecscreenSummary() const;

    const TVecscreenSummaryBySeqid&
        GetVecscreenSummaryBySeqid() const;

    static bool IsDbAvailable();

protected:
    CRef<CSeq_id> x_SelectSeqId(CBioseq_Handle bioseq_handle);
    CRef<CSeq_id> x_FindAccession(CBioseq_Handle bioseq_handle);
    CRef<CSeq_id> x_FindTypeGeneral(CBioseq_Handle bioseq_handle);
    CRef<CSeq_id> x_FindLocalId(CBioseq_Handle bioseq_handle);

private:
    objects::CSeq_entry_Handle m_TopSeqEntry;
    typedef vector< CRef<CSeq_loc> > TSearchLocVec;
    TSearchLocVec m_vecSearchLocs;

    // Standard vecscreen summary
    vector<CVecscreenRun::SVecscreenSummary> m_vecscreen_summary;

    // Vecscreen summary reorganized around seqid
    TVecscreenSummaryBySeqid m_seqidSummary;
};

END_NCBI_SCOPE

#endif
    // _VECTORSCREEN_H_
