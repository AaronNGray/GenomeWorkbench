#ifndef GUI_OBJUTILS___ADJUST_CONSENSUS_SPLICESITE__HPP
#define GUI_OBJUTILS___ADJUST_CONSENSUS_SPLICESITE__HPP

/*  $Id: adjust_consensus_splicesite.hpp 43721 2019-08-22 17:22:05Z asztalos $
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
 * Authors: Colleen Bollin, Andrea Asztalos
 *
 * File Description:
 *
 */


#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <objects/seqloc/Seq_loc.hpp>
#include <gui/objutils/cmd_composite.hpp>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CSeq_feat;
    class CScope;
    class CBioseq_Handle;
END_SCOPE(objects);

class NCBI_GUIOBJUTILS_EXPORT CAdjustForConsensusSpliceSite
{
public:
    CAdjustForConsensusSpliceSite(objects::CScope& scope)
        : m_Scope(&scope) {}
    
    void SetScope(objects::CScope& scope) { m_Scope.Reset(&scope); }

    CRef<CCmdComposite> GetCommand(const objects::CSeq_feat& cds);
    CRef<CCmdComposite> GetCommandToAdjustCDSEnds(const objects::CSeq_feat& cds);

    static bool s_IsBioseqGood_Strict(const objects::CBioseq_Handle& bsh);
    static bool s_IsBioseqGood_Relaxed(const objects::CBioseq_Handle& bsh);
    static bool s_IsBioseqGood_AdjustEnds(const objects::CBioseq_Handle& bsh);
    static bool s_IsAdjustedSpliceSitePairOK(const string& seqdata, TSeqPos intron_start, TSeqPos intron_stop);

    bool AdjustCDS(objects::CSeq_feat& cds);
    bool AdjustmRNAToMatchCDS(const objects::CSeq_feat& edit_cds, objects::CSeq_feat& mrna);
    CRef<CCmdComposite> AdjustmRNAandExonFeatures(const objects::CSeq_feat& new_cds, const objects::CSeq_feat& orig_cds);

    // Splice adjustments can lead to extension or trimming by 2 nucleotides
    bool AdjustCDSEnds(objects::CSeq_feat& cds);
    bool AdjustmRNAToMatchCDSEnds(const objects::CSeq_feat& edit_cds, objects::CSeq_feat& mrna);
    CRef<CCmdComposite> AdjustmRNAandExonEnds(const objects::CSeq_feat& new_cds, const objects::CSeq_feat& orig_cds);
    
private:
    typedef vector<TSeqRange> TLocs;
    
    void x_InitRanges(const objects::CSeq_feat& cds);
    void x_UpdateMixLocations(const objects::CSeq_loc& orig_loc, objects::CSeq_loc::TLocations& locs);
    void x_UpdateInterval(objects::CSeq_interval& interval);
    void x_UpdateIntervals(objects::CSeq_loc::TIntervals& locs);
    CRef<objects::CSeq_loc> x_UpdateLocation(const objects::CSeq_feat& cds);

    TSeqPos x_IntronLength(const TSeqRange& rng_prev, const TSeqRange& rng);
    void x_ShiftExonPairForward(TSeqRange& range_prev, TSeqRange& range, TSeqPos offset);
    void x_ShiftExonPairBackward(TSeqRange& range_prev, TSeqRange& range, TSeqPos offset);

    CRef<objects::CSeq_loc> x_CreateNewLocation(const objects::CSeq_feat& cds);
    bool x_HasProteinChanged(const objects::CSeq_feat& cds, const string& orig_prot_seq);
    bool x_HasProteinChangedAt5End(objects::CSeq_feat& cds, const string& orig_prot_seq); 
    bool x_HasProteinChangedAt3End(objects::CSeq_feat& cds, const string& orig_prot_seq);
    bool x_AlsoAdjustmRNA(objects::CSeq_feat& mrna, const objects::CSeq_loc& loc);
    void x_UpdateExonFeatures(CCmdComposite* cmd, const objects::CSeq_feat& orig_cds, const objects::CSeq_feat& new_cds);

    bool x_AdjustCDS5End(objects::CSeq_feat& cds, const string& orig_prot_seq);
    bool x_AdjustCDS3End(objects::CSeq_feat& cds, const string& orig_prot_seq);
    
    bool x_ExtendStartOfExon(TSeqRange& range, TSeqPos offset, const objects::CSeq_loc& loc, objects::CBioseq_Handle bsh);
    void x_TrimStartOfExon(TSeqRange& range, TSeqPos offset);
    bool x_ExtendStopOfExon(TSeqRange& range, TSeqPos offset, const objects::CSeq_loc& loc, objects::CBioseq_Handle bsh);
    void x_TrimStopOfExon(TSeqRange& range, TSeqPos offset);

    // strand of cds and associated mRNA
    objects::ENa_strand m_Strand;
    // original location ranges of the cds
    TLocs m_Orig_Ranges;
    // updated location ranges of the cds
    TLocs m_New_Ranges;
    // indicates that the protein changed during location adjustment
    // however, this change is acceptable when the 5'end needs to be adjusted 
    // and possibly the frame is changed
    bool m_ProtChanged{ false };

    CRef<objects::CScope> m_Scope;
};

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___ADJUST_CONSENSUS_SPLICESITE__HPP



