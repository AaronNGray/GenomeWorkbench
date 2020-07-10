/*  $Id: update_seq_input.cpp 44068 2019-10-18 18:57:05Z asztalos $
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


#include <ncbi_pch.hpp>
#include <serial/objistr.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/seq/seqport_util.hpp>
#include <objects/seq/seq_macros.hpp>
#include <objects/seqalign/Dense_seg.hpp>

#include <objtools/readers/fasta.hpp>
#include <objtools/edit/seq_entry_edit.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/widgets/wx/simple_clipboard.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/loadentrez_seq_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/update_seq_input.hpp>
#include <gui/packages/pkg_sequence_edit/sequpd.hpp>
#include <gui/packages/pkg_sequence_edit/sequpd_except.hpp>

#include <wx/dataobj.h>
#include <wx/clipbrd.h>
#include <wx/filedlg.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CUpdateSeq_Input::CUpdateSeq_Input(void)
    : m_Align(0), m_Scope(0)
{
}

CUpdateSeq_Input::CUpdateSeq_Input(const CBioseq_Handle& old_bsh, const CBioseq_Handle& upd_bsh)
    : m_OldSeq(old_bsh), m_UpdSeq(upd_bsh), m_Align(0)
{
    m_Scope.Reset(&old_bsh.GetScope());
    if (&(m_UpdSeq.GetScope()) != m_Scope.GetNCPointer()) {
        NCBI_THROW(CSeqUpdateException, eInternal, "Both sequences should be in the same scope");
    }
}

CUpdateSeq_Input::~CUpdateSeq_Input()
{
    // the update sequence should be removed from the scope
    if (m_UpdSeq) {
        m_UpdSeq.GetTopLevelEntry().GetEditHandle().Remove();
    }
}

bool CUpdateSeq_Input::HaveIdenticalResidues(void) const
{
    return sequpd::HaveIdenticalResidues(m_OldSeq, m_UpdSeq);
}

CRef<CSeq_annot> CUpdateSeq_Input::s_Align2IdenticalSeq(const CBioseq_Handle& subject, const CBioseq_Handle& query)
{
    if (!sequpd::HaveIdenticalResidues(subject, query)) {
        return CRef<CSeq_annot>();
    }

    CRef<CDense_seg> dense_seg(new CDense_seg);
    dense_seg->SetDim(2);
    dense_seg->SetNumseg(1);

    CRef<CSeq_id> query_id(new CSeq_id);
    query_id->Assign(*query.GetSeqId());
    CRef<CSeq_id> subject_id(new CSeq_id);
    subject_id->Assign(*subject.GetSeqId());
    dense_seg->SetIds().push_back(query_id);
    dense_seg->SetIds().push_back(subject_id);

    dense_seg->SetStarts().push_back(0);
    dense_seg->SetStarts().push_back(0);
    dense_seg->SetLens().push_back(subject.GetBioseqLength() - 1);

    CRef<CSeq_align> align(new CSeq_align);
    align->SetType(CSeq_align::eType_partial);
    align->SetDim(2);
    align->SetSegs().SetDenseg(*dense_seg);

    CRef<CSeq_annot> annot(new CSeq_annot);
    annot->SetData().SetAlign().push_back(align);
    CRef<CAnnotdesc> desc(new CAnnotdesc);
    desc->SetName("Alignment generated between two identical sequences");
    annot->SetDesc().Set().push_back(desc);

    return annot;
}

bool CUpdateSeq_Input::SetOldBioseqAndScope(IWorkbench* workbench, const CSeq_entry_Handle& tse)
{
    TConstScopedObjects objects;
    if (!workbench) {
        return false;
    }

    CIRef<CSelectionService> sel_srv = workbench->GetServiceByType<CSelectionService>();
    if (sel_srv) {
        sel_srv->GetActiveObjects(objects);
    }
    if (objects.empty()) {
        GetViewObjects(workbench, objects);
    }

    if (objects.empty() || objects.size() == 1 || FocusedOnAll(objects, tse)) {
        NCBI_THROW(CSeqUpdateException, eInternal, "No bioseq selected.");
    }

    const CObject* ptr = objects[0].object.GetPointer();
    const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>(ptr);
    const CBioseq* bioseq = dynamic_cast<const CBioseq*>(ptr);
    if (loc)  {
        m_OldSeq = objects[0].scope->GetBioseqHandle(*loc);
    } else if (bioseq) {
        m_OldSeq = objects[0].scope->GetBioseqHandle(*bioseq);
    }

    if (m_OldSeq) {
        m_Scope.Reset(&m_OldSeq.GetScope());
        return true;
    }
    
    return false;
}

bool CUpdateSeq_Input::SetOldBioseqAndScope(const CBioseq_Handle& bsh)
{
    m_OldSeq = bsh;
    m_Scope.Reset(&m_OldSeq.GetScope());
    return true;
}

bool CUpdateSeq_Input::SetUpdateBioseq(const CBioseq_Handle& bsh)
{
    m_UpdSeq = bsh;
    // colliding IDs have been resolved by now
    if (&(m_UpdSeq.GetScope()) != m_Scope.GetNCPointer()) {
        NCBI_THROW(CSeqUpdateException, eInternal, "Both sequences should be in the same scope");
    }
    return true;
}

void CUpdateSeq_Input::ResetUpdateSequence()
{
    m_UpdSeq.Reset();
    if (m_Align) {
        m_Align.Reset();
    }
}

void CUpdateSeq_Input::CalculateAlignmentForUnitTest()
{
    _ASSERT(m_OldSeq);
    _ASSERT(m_UpdSeq);
    bool accept_atleast_one = false;
    if (m_OldSeq.GetBioseqLength() < m_UpdSeq.GetBioseqLength()) {
        accept_atleast_one = true;
    }
    auto align_vector = sequpd::RunBlast2Seq(m_OldSeq, m_UpdSeq, accept_atleast_one);
    if (align_vector.size() > 1) {
        LOG_POST(Info << "More than one alignment was found");
    } 

    if (align_vector.empty()) {
        LOG_POST(Info << "Could not form alignment between old and update sequence");
        m_Align.Reset();
        _ASSERT(m_Align.IsNull());
    }
    else {
        // choose the 'better' alignment
        size_t best_align = 0;
        for (size_t i = 0; i < align_vector.size(); ++i) {
            for (size_t j = i + 1; j < align_vector.size(); ++j) {
                best_align = sequpd::CompareAlignments(*align_vector[i], *align_vector[j]) ? i : j;
            }
        }
        _ASSERT(best_align < align_vector.size());
        m_Align.Reset(align_vector[best_align]);
    }
}

bool CUpdateSeq_Input::IsReadyForUpdate() const
{
    bool set_seqs = (m_OldSeq && m_UpdSeq);
    bool no_mismatch = (m_OldSeq.IsNa() && m_UpdSeq.IsNa()) || (m_OldSeq.IsAa() && m_UpdSeq.IsAa());
    return (set_seqs && no_mismatch);
}

bool CUpdateSeq_Input::UpdateSeqHasFeatures() const
{
    if (!m_UpdSeq)
        return false;

    return CFeat_CI(m_UpdSeq);
}

bool CUpdateSeq_Input::OldSeqHasFeatures() const
{
    if (!m_OldSeq)
        return false;

    return CFeat_CI(m_OldSeq);
}

bool CUpdateSeq_Input::OldSeqHasProteinFeats() const
{
    if (!m_OldSeq)
        return false;

    CBioseq_set_Handle bssh = m_OldSeq.GetParentBioseq_set();
    if (!bssh)
        return false;

    bool has_feat = false;
    for (CBioseq_CI bseq_it(bssh, CSeq_inst::eMol_aa); bseq_it && !has_feat; ++bseq_it) {
        for (CFeat_CI feat_it(*bseq_it); feat_it && !has_feat; ++feat_it) {
            auto feat = feat_it->GetOriginalSeq_feat();
            if (feat->IsSetData() && feat->GetData().IsProt() && !feat->GetData().GetProt().IsSetProcessed())
                continue;
            else
                has_feat = true;
        }
    }

    return has_feat;
}

bool CUpdateSeq_Input::ReadSingleUpdateSequence(ILineErrorListener* msg_listener)
{
    if (m_UpdSeq) {
        NCBI_THROW(CSeqUpdateException, eInternal, "The update sequence has already been submitted");
    }

    wxString ext = CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles) + wxT("|") +
                   CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|") +
                   CFileExtensions::GetDialogFilter(CFileExtensions::kFASTA);
                   
    
    wxFileDialog openDlg(NULL, wxT("Open a file"), wxEmptyString, wxEmptyString, ext, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openDlg.ShowModal() != wxID_OK)
        return false;

    wxString path = openDlg.GetPath();
    CNcbiIfstream fstream(path.fn_str(), ios_base::binary);
    return x_ReadFromStream(fstream, msg_listener);
}

bool CUpdateSeq_Input::x_ReadFromStream(CNcbiIstream& istr, ILineErrorListener* msg_listener)
{
    CFormatGuess guesser(istr);
    guesser.GetFormatHints().DisableAllNonpreferred();
    guesser.GetFormatHints().AddPreferredFormat(CFormatGuess::eFasta);
    guesser.GetFormatHints().AddPreferredFormat(CFormatGuess::eTextASN);

    CFormatGuess::EFormat format = guesser.GuessFormat();

    if (format == CFormatGuess::eTextASN) {
        return x_ReadUpdateSeq_ASN(istr);
    }
    else if (format == CFormatGuess::eFasta || 
            format == CFormatGuess::eUnknown) {
        // With high probability, it is a Fasta file without defline
        try {
            return x_ReadUpdateSeq_Fasta(istr, msg_listener);
        }
        catch (const CSeqUpdateException& e) {
            if (e.GetErrCode() == CSeqUpdateException::eReading) {
                NCBI_THROW(CSeqUpdateException, eReading,
                    "Failed to recognize the format of update sequence. Please try ASN.1 or Fasta.\n" + e.GetMsg());
            }
        }
    }
    return false;
}

bool CUpdateSeq_Input::ReadUpdateSequenceFromClipboard(ILineErrorListener* msg_listener)
{
    if (m_UpdSeq) {
        NCBI_THROW(CSeqUpdateException, eInternal, "The update sequence has already been submitted");
    }

    string file_str = CSimpleClipboard::Instance().Get<string>();
    CNcbiIstrstream istr(file_str.c_str());
    
    return x_ReadFromStream(istr, msg_listener);
}

static const char* kNASeqCouldNotRead = "Failed to read update sequence";

bool CUpdateSeq_Input::x_ReadUpdateSeq_ASN(CNcbiIstream& istr)
{
    
    CObjectInfo oi;
    try {
        auto_ptr<CObjectIStream> objIstrm(CObjectIStream::Open(eSerial_AsnText, istr, eNoOwnership));
        oi = objIstrm->ReadObject();
    }
    catch (const CException& e) {
        LOG_POST(Error << e.GetMsg());
        NCBI_THROW(CSeqUpdateException, eReading, kNASeqCouldNotRead);
    }

    if (oi.GetName() == "Bioseq") {
        CBioseq* bseq = CTypeConverter<CBioseq>::SafeCast(oi.GetObjectPtr());
        if (bseq) {
            s_ConvertDeltaToRawBioseq(*bseq);
            x_FixCollidingIDs_Bioseq(*bseq);
            m_UpdSeq = m_Scope->AddBioseq(*bseq);
        }
    }
    else {
        CRef<CSeq_entry> seq_entry;
        if (oi.GetName() == "Seq-entry") {
            CSeq_entry* entry = CTypeConverter<CSeq_entry>::SafeCast(oi.GetObjectPtr());
            seq_entry = Ref(entry);
        }
        else if (oi.GetName() == "Seq-submit") {
            CSeq_submit* seq_submit = CTypeConverter<CSeq_submit>::SafeCast(oi.GetObjectPtr());
            if (seq_submit) {
                seq_entry = edit::SeqEntryFromSeqSubmit(*seq_submit);
            }
        }

        if (seq_entry) {
            s_ConvertDeltaToRawEntry(*seq_entry);
            x_AddNewEntryToScope(seq_entry); 
        }
    }

    if (!m_UpdSeq) {
        NCBI_THROW(CSeqUpdateException, eReading, kNASeqCouldNotRead);
    }
    
    return true;
}

bool CUpdateSeq_Input::x_ReadUpdateSeq_Fasta(CNcbiIstream& istr, ILineErrorListener* msg_listener)
{
    CFastaReader::TFlags flags = 0;
    flags |= CFastaReader::fAssumeNuc
            | CFastaReader::fNoSplit
            | CFastaReader::fHyphensIgnoreAndWarn
            | CFastaReader::fDLOptional;
    
    CFastaReader fasta_reader(istr, flags);
    CRef<CSeq_entry> new_entry;
    try {
        new_entry = fasta_reader.ReadSet(kMax_Int, msg_listener);
    }
    catch (const CException&) {
        NCBI_THROW(CSeqUpdateException, eReading, kNASeqCouldNotRead);
    }


    x_AddNewEntryToScope(new_entry);

    if (!m_UpdSeq) {
        if (msg_listener && msg_listener->Count() > 0) {
            // display only the first error to the user
            const ILineError& line_err = msg_listener->GetError(0);
            NCBI_THROW(CSeqUpdateException, eReading, line_err.ErrorMessage());
        }
        else {
            NCBI_THROW(CSeqUpdateException, eReading, kNASeqCouldNotRead);
        }
    }

    return true;
}

void CUpdateSeq_Input::x_AddNewEntryToScope(CRef<CSeq_entry> entry)
{
    if (!entry) {
        return;
    }

    entry->Parentize();
    x_FixCollidingIDs_Entry(*entry);
    CSeq_entry_Handle seh = m_Scope->AddTopLevelSeqEntry(*entry);
    CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
    if (b_iter) {
        m_UpdSeq = *b_iter;
    }
}

unsigned int CUpdateSeq_Input::CountBioseqs()
{
    unsigned int count = 0;
    if (m_UpdSeq) {
        for (CBioseq_CI b_iter(m_UpdSeq.GetTopLevelEntry(), CSeq_inst::eMol_na); b_iter; ++b_iter)
            ++count;
    }
    return count;
}

bool CUpdateSeq_Input::s_IsGoodLiteral(const CSeq_literal& lit)
{
    if (!lit.IsSetSeq_data()) {
        return true;
    }

    const CSeq_data& data = lit.GetSeq_data();
    if (!data.IsGap()){
        return true;
    }

    const CSeq_gap& gap = data.GetGap();
    if (gap.IsSetType() && gap.GetType() == CSeq_gap::eType_unknown) {
        return !(gap.IsSetLinkage());
    }

    return false;
}

bool CUpdateSeq_Input::s_IsDeltaWithNoGaps(const CBioseq& bseq)
{
    if (!bseq.IsSetInst())
        return false;

    const CBioseq::TInst& inst = bseq.GetInst();
    if (!inst.IsSetRepr()
        || inst.GetRepr() != CSeq_inst::eRepr_delta
        || !inst.IsSetExt()
        || !inst.GetExt().IsDelta()) {
        return false;
    }

    for (auto&& it : inst.GetExt().GetDelta().Get()) {
        if (!it->IsLiteral()) {
            return false;
        }
        const CSeq_literal& lit = it->GetLiteral();
        bool ok = lit.GetLength() > 0 && s_IsGoodLiteral(lit);
        if (!ok) return false;
    }

    return true;
}

bool CUpdateSeq_Input::s_IsDeltaWithFarPointers(const CBioseq& bseq)
{
    if (!bseq.IsSetInst())
        return false;

    const CBioseq::TInst& inst = bseq.GetInst();
    if (!inst.IsSetRepr()
        || inst.GetRepr() != CSeq_inst::eRepr_delta
        || !inst.IsSetExt()
        || !inst.GetExt().IsDelta()) {
        return false;
    }

    for (auto&& it : inst.GetExt().GetDelta().Get()) {
        if (!it->IsLoc()) {
            return false;
        }
    }
    return true;
}

void CUpdateSeq_Input::s_ConvertDeltaToRawEntry(CSeq_entry& entry)
{
    if (entry.IsSet() && entry.GetSet().IsSetSeq_set()) {
        NON_CONST_ITERATE(CBioseq_set::TSeq_set, set, entry.SetSet().SetSeq_set()) {
            s_ConvertDeltaToRawEntry(**set);
        }
    }
    else if (entry.IsSeq() && entry.GetSeq().IsNa()) {
        CBioseq& bseq = entry.SetSeq();
        s_ConvertDeltaToRawBioseq(bseq);
    }
}

void CUpdateSeq_Input::s_ConvertDeltaToRawBioseq(CBioseq& bseq)
{
    _ASSERT(bseq.IsNa());
    if (!s_IsDeltaWithNoGaps(bseq)) {
        return;
    }
    
    CSeqVector seq_vec(bseq);
    seq_vec.SetCoding(CSeq_data::e_Iupacna);
    string seqdata;
    seq_vec.GetSeqData(0, bseq.GetLength(), seqdata);

    bseq.SetInst().ResetExt();
    bseq.SetInst().SetRepr(objects::CSeq_inst::eRepr_raw);
    bseq.SetInst().SetSeq_data().SetIupacna().Set(seqdata);
    CSeqportUtil::Pack(&bseq.SetInst().SetSeq_data());
    bseq.SetInst().SetLength(TSeqPos(seqdata.length()));
    
}

void CUpdateSeq_Input::s_FixCollidingIDs_Entry(CSeq_entry& entry, const CBioseq::TId& seq_ids)
{
    if (entry.IsSet() && entry.GetSet().IsSetSeq_set()) {
        NON_CONST_ITERATE(CBioseq_set::TSeq_set, set, entry.SetSet().SetSeq_set()) {
            s_FixCollidingIDs_Entry(**set, seq_ids);
        }
    }
    else if (entry.IsSeq() && entry.GetSeq().IsNa()) {
        CBioseq& bseq = entry.SetSeq();
        sequpd::FixCollidingIDs_Bioseq(bseq, seq_ids);
    }
}

static void s_CollectCollidingIDs_Entry(CSeq_entry& entry, CBioseq::TId& seq_ids, CScope &scope)
{
    if (entry.IsSet() && entry.GetSet().IsSetSeq_set()) {
        NON_CONST_ITERATE(CBioseq_set::TSeq_set, set, entry.SetSet().SetSeq_set()) {
            s_CollectCollidingIDs_Entry(**set, seq_ids, scope);
        }
    }
    else if (entry.IsSeq() && entry.GetSeq().IsNa()) {
        CBioseq& bseq = entry.SetSeq();
        ITERATE(CBioseq::TId, upd_id, bseq.GetId()) 
        {
            CBioseq_Handle bsh = scope.GetBioseqHandle(**upd_id);
            if (bsh)
                seq_ids.push_back(*upd_id);
        }
    }
}

void CUpdateSeq_Input::x_FixCollidingIDs_Entry(objects::CSeq_entry& entry)
{
    CBioseq::TId old_ids;
    s_CollectCollidingIDs_Entry(entry, old_ids, m_OldSeq.GetScope());
    s_FixCollidingIDs_Entry(entry, old_ids);
    entry.ReassignConflictingIds(); // for fixing possible duplicates among protein ids
}

void CUpdateSeq_Input::x_FixCollidingIDs_Bioseq(CBioseq& bseq)
{
    CBioseq::TId old_ids;
    CScope &scope = m_OldSeq.GetScope();
    ITERATE(CBioseq::TId, upd_id, bseq.GetId()) 
    {
        CBioseq_Handle bsh = scope.GetBioseqHandle(**upd_id);
        if (bsh)
            old_ids.push_back(*upd_id);
    }
    
    sequpd::FixCollidingIDs_Bioseq(bseq, old_ids);
}

bool CUpdateSeq_Input::ReadSingleAccession()
{
    if (m_UpdSeq) {
        NCBI_THROW(CSeqUpdateException, eInternal, "The update sequence has already been submitted");
    }

    CLoadSeqFromEntrez_dlg seq_dlg(NULL);
    if (seq_dlg.ShowModal() == wxID_OK) {
        string acc_id = seq_dlg.GetData();
        if (!NStr::IsBlank(acc_id)) {
            return x_ReadAccession(acc_id);
        }
    } 
    return false;
}


bool CUpdateSeq_Input::x_ReadAccession(const string& acc_id)
{
    CSeq_id id;
    try {
        id.Set(acc_id);
    }
    catch (const CSeqIdException&) {
        string msg = "'" + acc_id + "' is not a valid accession";
        NcbiMessageBox(msg);
        return false;
    }
   
    // retrieve the sequence
    m_UpdSeq = m_Scope->GetBioseqHandle(id);
    if(!m_UpdSeq) {
        string msg = "Failed to load sequence for accession '" + acc_id + "'";
        NcbiMessageBox(msg);
        return false;
    }
    

    CSeq_id::E_Choice feat_id_type = CSeq_id::e_not_set;
    CFeat_CI feat_it(m_UpdSeq);
    if (feat_it) {
        feat_id_type = feat_it->GetLocationId().Which();
    }

    if (id.Which() != feat_id_type && feat_id_type != CSeq_id::e_not_set) {
        CConstRef<CBioseq> bseq = m_UpdSeq.GetCompleteBioseq();
        ITERATE (CBioseq::TId, id_iter, bseq->GetId()) {
            if ((*id_iter)->Which() == feat_id_type){
                m_UpdSeq = m_Scope->GetBioseqHandle(**id_iter);
            }
        }
    }
    if (m_UpdSeq.IsNa() && s_IsDeltaWithNoGaps(*m_UpdSeq.GetCompleteBioseq()))
    {
        CSeqVector seq_vec = m_UpdSeq.GetSeqVector(CBioseq_Handle::eCoding_Iupac);
        string seqdata;
        seq_vec.GetSeqData(0, m_UpdSeq.GetBioseqLength(), seqdata);
        CRef<CSeq_inst> new_inst(new CSeq_inst);
        new_inst->Assign(m_UpdSeq.GetInst());
        new_inst->ResetExt();
        new_inst->SetRepr(objects::CSeq_inst::eRepr_raw);
        new_inst->SetSeq_data().SetIupacna().Set(seqdata);
        CSeqportUtil::Pack(&(new_inst->SetSeq_data()));
        new_inst->SetLength(TSeqPos(seqdata.length()));
        CBioseq_EditHandle bseh = m_UpdSeq.GetEditHandle();
        bseh.SetInst(*new_inst);
    }
    return true;
}

END_NCBI_SCOPE
