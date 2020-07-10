#ifndef GUI_OBJUTILS___CONVERT_RAW_TO_DELTA__HPP
#define GUI_OBJUTILS___CONVERT_RAW_TO_DELTA__HPP
/*  $Id: convert_raw_to_delta.hpp 43470 2019-07-11 14:54:19Z filippov $
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
 * Authors: Igor Filippov
 *
 * File Description: 
 *
 */

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <objects/seq/Seq_gap.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/general/Object_id.hpp>
#include <objmgr/seq_feat_handle.hpp>
#include <gui/gui_export.h>
#include <gui/objutils/cmd_composite.hpp>


BEGIN_NCBI_SCOPE

BEGIN_SCOPE(NRawToDeltaSeq)

struct NCBI_GUIOBJUTILS_EXPORT SGap
{
    int start;
    int length;
    bool is_known;
    bool is_replace;
    bool operator< (const SGap &other) const
    {
        return start < other.start;
    }
    int gap_type;
    int linkage;
    int linkage_evidence;
};

NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> ConvertRawToDeltaByNsCommand(const objects::CSeq_entry_Handle& seh,
                                                                         long min_unknown, int max_unknown, long min_known, int max_known, bool adjust_cds, bool keep_gap_length, bool& remove_alignments, int& count,
                                                                         bool is_assembly_gap = false, int gap_type = objects::CSeq_gap::eType_unknown, int linkage = -1, int linkage_evidence = -1);

NCBI_GUIOBJUTILS_EXPORT void GapLocationsFromNs(const objects::CSeq_inst& inst,
                                                size_t min_unknown, int max_unknown,
                                                size_t min_known, int max_known,
                                                vector < pair<int, int> > &start_length_locs,
                                                vector<bool> &vec_is_known);

NCBI_GUIOBJUTILS_EXPORT void UpdateDensegAlignment(objects::CSeq_align_Handle ah, objects::CBioseq_Handle bsh, CRef<CCmdComposite> composite,
                                                    const vector < pair<int, int> > &start_length_locs,
                                                    const vector<bool> &vec_is_known,
                                                    set<const objects::CSeq_align*> &deleted_aligns);

NCBI_GUIOBJUTILS_EXPORT void AdjustFeatureLocations(const map<objects::CBioseq_Handle, vector<SGap> > &map_gaps, CRef<CCmdComposite> composite, bool split_gene_locations, 
                                                    bool break_features, bool keep_gap_length, bool create_general_only, 
                                                    objects::CObject_id::TId &max_feat_id, map<objects::CObject_id::TId, objects::CObject_id::TId> &old_to_new, bool create_xref_map);

NCBI_GUIOBJUTILS_EXPORT void FindNewLocations(const objects::CSeq_loc &feat_loc, objects::CScope &scope, const map<objects::CBioseq_Handle, vector<SGap> > &gaps, 
                                              vector<CRef<objects::CSeq_loc> > &locations, bool &comment, bool break_features, bool keep_gap_length);

NCBI_GUIOBJUTILS_EXPORT CRef<objects::CSeq_feat> UpdateFeat(CRef<objects::CSeq_loc> changed_loc, const objects::CSeq_feat &feat, bool comment, objects::CScope &scope);

NCBI_GUIOBJUTILS_EXPORT void RemapOtherProtFeats(const objects::CSeq_feat &old_cds, objects::CSeq_feat &cds, objects::CBioseq_Handle bh, CRef<CCmdComposite> composite, bool &any_actions);
NCBI_GUIOBJUTILS_EXPORT void RemapOtherProtFeats(const objects::CSeq_feat &old_cds, objects::CSeq_feat &cds, objects::CSeq_feat_Handle fh, 
                                                vector<CRef<objects::CSeq_feat> > &other_prot_feats, 
                                                 objects::CObject_id::TId &max_feat_id, map<objects::CObject_id::TId, objects::CObject_id::TId> &old_to_new, bool create_xref_map); // adjustfeatureforgaps

END_SCOPE(NRawToDeltaSeq)
END_NCBI_SCOPE


#endif  // GUI_OBJUTILS___CONVERT_RAW_TO_DELTA__HPP
