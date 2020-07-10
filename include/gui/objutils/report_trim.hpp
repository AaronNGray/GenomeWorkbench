/*  $Id: report_trim.hpp 41252 2018-06-22 17:58:22Z asztalos $
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
 * Authors:  Igor Filippov
 */
#ifndef _REPORT_TRIM_H_
#define _REPORT_TRIM_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/graph_ci.hpp>
#include <objmgr/scope.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objmgr/util/sequence.hpp>


BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT CReportTrim : public objects::CSequenceAmbigTrimmer
{
public:
    CReportTrim(EMeaningOfAmbig eMeaningOfAmbig, TFlags fFlags = 0, const TTrimRuleVec & vecTrimRules = GetDefaultTrimRules(), TSignedSeqPos uMinSeqLen = 50)
        : CSequenceAmbigTrimmer(eMeaningOfAmbig,fFlags,vecTrimRules,uMinSeqLen) {}
    virtual ~CReportTrim() {}
    virtual EResult Trim( objects::CBioseq_Handle &bioseq_handle, TSignedSeqPos &left, TSignedSeqPos &right);

    void AdjustAndTranslate(objects::CBioseq_Handle bsh, int length, int from, int to, CRef<CCmdComposite> command, objects::CScope &scope, map<objects::CBioseq_Handle, set<objects::CSeq_feat_Handle> > &product_to_cds);
    void TrimAlignments(CRef<CCmdComposite> command, objects::CBioseq_Handle bsh, const objects::CBioseq::TId &seqids, TSeqPos from, TSeqPos to);
    void TrimSeqGraphs(CRef<CCmdComposite> command, objects::CBioseq_Handle bsh, const objects::CBioseq::TId &seqids, TSeqPos from, TSeqPos to, int length);
private:
    
    void x_AdjustCdregionFrame(CRef<objects::CSeq_feat> feat, int diff);
    void x_RetranslateCDS(objects::CScope& scope, CRef<CCmdComposite> command, CRef<objects::CSeq_feat> cds, const objects::CSeq_feat &old_cds);
    bool x_AdjustLocation(objects::CSeq_loc &loc, int length, int from, int to, int &diff);
    bool x_AdjustLocation(objects::CSeq_loc &loc, int length, int from, int to);
    void x_AdjustInt(int &begin, int &end, int from, int to, int length, int &diff, objects::ENa_strand strand);
    void x_UpdateVariation(objects::CVariation_ref &var, int length, int from, int to);
    bool x_IsProtLocation(const objects::CSeq_loc& loc, objects::CBioseq_Handle bsh, objects::CScope &scope);
    void x_AdjustDensegAlignment(CRef<objects::CSeq_align> align, objects::CDense_seg::TDim row, TSeqPos cut_from, TSeqPos cut_to);
    void x_CutDensegSegment(CRef<objects::CSeq_align> align, objects::CDense_seg::TDim row, TSeqPos pos);
    bool x_FindSegment(const objects::CDense_seg& denseg, objects::CDense_seg::TDim row, TSeqPos pos, objects::CDense_seg::TNumseg& seg, TSeqPos& seg_start) const;
    bool x_UpdateSeqGraphLoc(CRef<objects::CSeq_graph> new_graph, TSeqPos from, TSeqPos to, int length, const CRef<objects::CSeq_id> seqid);
    bool x_TrimSeqGraphData(CRef<objects::CSeq_graph> new_graph, const objects::CMappedGraph& orig_graph, TSeqPos trim_start, TSeqPos trim_stop, int length, const CRef<objects::CSeq_id> seqid);
    bool x_IsThereGapNotAtTheEnd(objects::CBioseq_Handle bsh, TSignedSeqPos from, TSignedSeqPos to, int length);
};

END_NCBI_SCOPE

#endif
    // _REPORT_TRIM_H_
