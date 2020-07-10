#ifndef _GUI_PACKAGES_SEQUPDATE_INPUT_HPP_
#define _GUI_PACKAGES_SEQUPDATE_INPUT_HPP_
/*  $Id: update_seq_input.hpp 44068 2019-10-18 18:57:05Z asztalos $
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
#include <corelib/ncbiexpt.hpp>
#include <corelib/ncbiobj.hpp>
#include <util/format_guess.hpp>
#include <gui/gui_export.h>
#include <objects/seqalign/Seq_align.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objtools/readers/message_listener.hpp>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects);
    class CSeq_entry_Handle;
    class CSeq_id;
    class CSeq_literal;
END_SCOPE(objects);

class IWorkbench;

/// Sets up the old and the update sequences, and generates the alignment between them. 
class NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT CUpdateSeq_Input : public CObject
{
public:
    CUpdateSeq_Input(void);
    CUpdateSeq_Input(const objects::CBioseq_Handle& old_bsh, const objects::CBioseq_Handle& upd_bsh);
    ~CUpdateSeq_Input();

    const objects::CBioseq_Handle& GetOldBioseq(void) const { return m_OldSeq; }
    objects::CBioseq_Handle GetNCOldBioseq(void) const { return m_OldSeq; }
    const objects::CBioseq_Handle& GetUpdateBioseq(void) const { return m_UpdSeq; }
    CConstRef<objects::CSeq_align> GetAlignment(void) const { return m_Align; }
    void SetAlignment(const objects::CSeq_align* align) { m_Align.Reset(align); }
    
    bool SetOldBioseqAndScope(IWorkbench* workbench, const objects::CSeq_entry_Handle& tse);
    bool SetOldBioseqAndScope(const objects::CBioseq_Handle& bsh);
    bool SetUpdateBioseq(const objects::CBioseq_Handle& bsh);
    
    /// Reads single update sequence from the clipboard. It supports FASTA and text ASN formats.
    bool ReadUpdateSequenceFromClipboard(objects::ILineErrorListener* msg_listener);
    /// Reads (ASN.1 or FASTA) single update sequence from a file
    bool ReadSingleUpdateSequence(objects::ILineErrorListener* msg_listener);
    /// Reads single update sequence identified by an accession
    bool ReadSingleAccession(void);
    
    bool HaveIdenticalResidues(void) const;

    bool IsReadyForUpdate(void) const;

    bool UpdateSeqHasFeatures(void) const;
    bool OldSeqHasFeatures(void) const;
    bool OldSeqHasProteinFeats() const;

    /// Calculates the alignment between the old and the update sequence, sets m_Align member.
    /// To be used only in the unit tests
    void CalculateAlignmentForUnitTest(void);

    static CRef<objects::CSeq_annot> s_Align2IdenticalSeq(const objects::CBioseq_Handle& subject, const objects::CBioseq_Handle& query);

    /// Converts delta literals to raw sequence, applies only to NA sequences
    static void s_ConvertDeltaToRawEntry(objects::CSeq_entry& entry);
    static void s_ConvertDeltaToRawBioseq(objects::CBioseq& bseq);
    
    // Functions used to determine whether the sequence can be converted to raw sequence
    static bool s_IsGoodLiteral(const objects::CSeq_literal& lit);
    static bool s_IsDeltaWithNoGaps(const objects::CBioseq& bseq); 

    static bool s_IsDeltaWithFarPointers(const objects::CBioseq& bseq);

    /// When the old and the update sequences have the same ID, the ID of the update sequence is modified
    static void s_FixCollidingIDs_Entry(objects::CSeq_entry& entry, const objects::CBioseq::TId& seq_ids);

    unsigned int CountBioseqs();
    // will discard the update sequence and the alignment
    void ResetUpdateSequence(); 

private:
    bool x_ReadFromStream(CNcbiIstream& istr, objects::ILineErrorListener* msg_listener);
    bool x_ReadUpdateSeq_Fasta(CNcbiIstream& istr, objects::ILineErrorListener* msg_listener);
    bool x_ReadUpdateSeq_ASN(CNcbiIstream& istr);
    bool x_ReadAccession(const string& acc_id);

    // These functions in turn call the static functions with similar names
    void x_FixCollidingIDs_Entry(objects::CSeq_entry& entry);
    void x_FixCollidingIDs_Bioseq(objects::CBioseq& bioseq);

    /// Function fixes colliding IDs, add the update entry to the scope and sets m_UpdSeq member
    void x_AddNewEntryToScope(CRef<objects::CSeq_entry> entry);

   
    objects::CBioseq_Handle m_OldSeq;
    objects::CBioseq_Handle m_UpdSeq;
    CConstRef<objects::CSeq_align> m_Align;
    ///@note
    ///  both the old and the update sequence should be placed in the same scope (NgAligner + visualization purposes)
    CRef<objects::CScope> m_Scope;
};

END_NCBI_SCOPE

#endif
    // _GUI_PACKAGES_SEQUPDATE_INPUT_HPP_
