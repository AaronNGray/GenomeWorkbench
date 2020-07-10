#ifndef _GUI_PACKAGES_UPDATE_SEQ_HPP_
#define _GUI_PACKAGES_UPDATE_SEQ_HPP_
/*  $Id: update_seq.hpp 42185 2019-01-09 18:49:13Z filippov $
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
 * Authors:  Andrea Asztalos
 */

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objmgr/seq_feat_handle.hpp>

#include <gui/gui_export.h>
#include <gui/utils/command_processor.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/packages/pkg_sequence_edit/update_seq_input.hpp>
#include <gui/packages/pkg_sequence_edit/update_seq_params.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CSeq_id;
    class CSeq_inst;
    class CSeq_entry;
    class CSeq_annot;
    class CSeq_loc_Mapper_Base;
    class CSeqdesc;
END_SCOPE(objects);

/// Class responsible for executing the sequence update for one old-update sequence pair.
///
class NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT CSequenceUpdater
{
public:
    CSequenceUpdater(const CUpdateSeq_Input& updseq_in, const SUpdateSeqParams& params);
    ~CSequenceUpdater();

    void SetUpdateSeqParams(const SUpdateSeqParams& params);
    /// Main function responsible to update the old sequence with the update sequence.
    CRef<CCmdComposite> Update(bool create_general_only);

    bool IsOldSequenceOK() const;
    bool IsUpdateSequenceRaw() const;
    bool IsAlignmentOK() const;
    bool CheckParameters() const;

    // static functions:

    static objects::CSeq_align::TDim FindRowInAlignment(const objects::CBioseq_Handle& bsh, const objects::CSeq_align& align);

    /// Extends the 5' or 3' end of a NA sequence (no alignment is necessary).
    /// Only valid residues are kept and used for sequence extension
    static CRef<objects::CSeq_inst> s_ExtendOneEndOfSequence(const objects::CBioseq_Handle& bsh,
        const string& extension, SUpdateSeqParams::ESequenceUpdateType update_type);

    static CRef<objects::CSeq_inst> s_UpdateSeqInst(const objects::CBioseq_Handle& bsh, const string& upd_str);

    static string s_GetValidExtension(const string& extension);

    /// Offsets the location of the feature.
    /// If newId is not Null, it also changes the feature location ID
    static CRef<objects::CSeq_feat> s_OffsetFeature(const objects::CSeq_feat& feat, const TSeqPos offset, const objects::CSeq_id* newId = NULL);

    static void s_Shift_CDSCodeBreaks(CRef<objects::CSeq_feat> feat, const TSeqPos& offset, const objects::CSeq_id* targetId = NULL);

    static void s_Shift_tRNAAntiCodon(CRef<objects::CSeq_feat> feat, const TSeqPos& offset, const objects::CSeq_id* targetId = NULL);

    static CRef<objects::CSeq_loc> s_OffsetLocation(const objects::CSeq_loc& sourceLoc, const TSeqPos& offset, const objects::CSeq_id* targetId = NULL);

    const string& GetCitSubMessage() const { return m_CitSubMessage; }
    void ShowCitSubMessage(void) const;

    bool IsReverseComplemented() const { return m_Reversed; }
    string GetRevCompReport() const;

    void GetNotImportedFeatsReport(CNcbiOstream& out) const;

private:
    // functions for changing the sequence
    void NoChange(bool create_general_only);
    void Replace(bool create_general_only);
    void Patch(bool create_general_only);
    void ExtendOneEndOfSequence(bool create_general_only);

    CRef<objects::CSeq_inst> x_ReplaceSequence();
    CRef<objects::CSeq_inst> x_PatchSequence();
    CRef<objects::CSeq_inst> x_ExtendOneEnd();
    CRef<objects::CSeq_inst> x_UpdateSeqInst(const string& upd_str);


    // Functions for building the new seq-entry
    /// Creates the new seq-entry. It is not added to the scope, yet.
    void x_MakeNewEntry(const objects::CSeq_inst& newInst);
    void x_PrepareNewEntry(CRef<objects::CSeq_entry> entry, const objects::CSeq_inst& newInst);

    objects::CSeq_entry_Handle x_GetOldBseq_EntryHandle(void);

    /// Creates command that swaps the old sequence with the new one
    CIRef<IEditCommand> x_SwapOldWithNewSeq();

    // functions for handling features 
    bool x_ShouldRemoveFeature(const objects::CSeq_feat_Handle& fh);

    /// Determines whether the feature from update sequence should be imported to the new one
    bool x_ShouldImportFeature(const objects::CSeq_feat_Handle& fh);

    /// Replaces the location ID in the feature to match the alignment ID corresponding to the given row
    void x_ChangeIDInFeature(objects::CSeq_feat_Handle fh, const objects::CSeq_align& align, objects::CSeq_align::TDim row);

    /// Changes only the location ID of the feature
    CRef<objects::CSeq_feat> x_MappedFeature_ChangeId(const objects::CSeq_feat_Handle& orig_fh);

    /// Maps feature's location through the alignment
    CRef<objects::CSeq_feat> x_MappedFeature_ThroughAlign(const objects::CSeq_feat_Handle& orig_fh, const objects::CSeq_align& align);

    void x_ReplaceIdInOldFeatures();

    void x_MapOldAndImportedFeatsThroughNewAlign();

    CConstRef<objects::CSeq_align> x_GetNewAlign_Replace(const objects::CBioseq_Handle& bsh);

    /// Generates alignment between subject_bsh and the new sequence (query)
    vector<CConstRef<objects::CSeq_align> > x_FormNewAlignment(const objects::CBioseq_Handle& subject);

    /// Decides which duplicate features will end up on the new sequence
    void x_HandleDuplicateFeatures();

    /// Merges duplicate features
    CRef<objects::CSeq_feat> x_FuseFeatures(const objects::CSeq_feat& feat1, const objects::CSeq_feat& feat2);

    /// Merges protein sequence features for duplicate coding regions
    void x_FuseProtFeatsForCDS(const objects::CSeq_feat& fusedFeat, const objects::CSeq_feat& feat_old);


    // functions for attaching the feature lists to the New Sequence
    void x_FixID_AttachFeatures(bool create_general_only);

    /// Removes newId in features and stores them in two seq-annots: 
    /// one that will be attached to the sequence and another one that will be attached to the set
    void x_AttachFeaturesToNewSeq(bool create_general_only);

    void x_FixIDInNewEntry(CRef<objects::CSeq_entry> entry);

    void x_RemoveNewIDInFeature(CRef<objects::CSeq_feat> feat);

    void x_UpdateProteinID(CRef<objects::CSeq_entry> protein, CRef<objects::CSeq_id> newId);

    void x_AdjustProteinFeature(CRef<objects::CSeq_entry> protein, CRef<objects::CSeq_id> newId = CRef<objects::CSeq_id>());

    void x_SetOldIDInFeatures();

    CRef<objects::CSeq_annot> x_GetSeqAnnot_WithoutCDS(bool& has_cds);

    void x_AdjustOldCDSProduct(CRef<objects::CSeq_feat> cds, objects::CSeq_entry_EditHandle& eh);

    void x_ImportCDSProduct(CRef<objects::CSeq_feat> cds, objects::CSeq_entry_EditHandle& eh, objects::CBioseq_Handle& newBsh, bool create_general_only);

    void x_RetranslateOldCDSProduct(CRef<objects::CSeq_feat> cds, objects::CSeq_entry_EditHandle& eh, objects::CBioseq_Handle& newBsh, bool create_general_only);

    void x_RetranslateImportedCDSProduct(CRef<objects::CSeq_feat> cds, objects::CSeq_entry_EditHandle& eh, objects::CBioseq_Handle& newBsh, bool create_general_only);

    /// Removes RefGeneTracking user-object and create-date descriptors from imported cds products
    void x_RemoveDescriptorsFromImportedProducts(CRef<objects::CSeq_entry> protein);

    /// Attaches a cit-sub to the New sequence
    void x_AddCitSubToUpdatedSequence();

    // function for debugging purposes
    void x_PrintNewEntryIds();

    TSeqPos x_GetNewSeqLength();
private:

    objects::CBioseq_Handle m_OldBsh;
    const objects::CBioseq_Handle& m_UpdBsh;
    CConstRef<objects::CSeq_align> m_Align;
    SUpdateSeqParams m_Params;

    /// The new seq-entry
    CRef<objects::CSeq_entry> m_NewEntry;
    /// Temporary seq-id of the new seq-entry
    CConstRef<objects::CSeq_id> m_NewId;

    typedef list<CRef<objects::CSeq_feat> > TFeatList;
    /// List of features, originally belonging to the Old Sequence and update sequence, respectively.
    /// Their locations have been adjusted (including new SeqId), ready to be attached to the New Sequence
    /// Lists are ready to be searched for duplicates 
    TFeatList m_MappedOldFeats;
    TFeatList m_ImportUpdFeats;

    // members used for reporting:
    string m_CitSubMessage;
    bool m_Reversed;
    /// List of features that were not imported as they were outside the alignment range
    vector<string> m_NotImportedFeats;

    // prohibit copy constructor and assignment operator
    CSequenceUpdater(const CSequenceUpdater&);
    CSequenceUpdater& operator=(const CSequenceUpdater&);
};

END_NCBI_SCOPE

#endif // _GUI_PACKAGES_UPDATE_SEQ_HPP_
