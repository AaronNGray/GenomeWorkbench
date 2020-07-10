/*  $Id: update_multi_seq_input.cpp 44179 2019-11-12 20:55:18Z asztalos $
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
#include <serial/iterator.hpp>

#include <gui/packages/pkg_sequence_edit/update_multi_seq_input.hpp>

#include <objects/submit/Seq_submit.hpp>
#include <objmgr/bioseq_ci.hpp>

#include <objtools/readers/fasta.hpp>

#include <gui/widgets/wx/simple_clipboard.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/packages/pkg_sequence_edit/sequpd.hpp>
#include <gui/packages/pkg_sequence_edit/sequpd_except.hpp>

#include <objtools/edit/seq_entry_edit.hpp>

#include <wx/dataobj.h>
#include <wx/clipbrd.h>
#include <wx/filedlg.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CUpdateMultipleSeq_Input::CUpdateMultipleSeq_Input()
    : m_NumUpdated(0), m_NumSkipped(0), m_UpdEntry(0), m_Scope(0)
{
}

CUpdateMultipleSeq_Input::~CUpdateMultipleSeq_Input()
{
}

bool CUpdateMultipleSeq_Input::SetOldEntryAndScope(const CSeq_entry_Handle& tse)
{
    m_OldSeh = tse;
    m_Scope.Reset(&m_OldSeh.GetScope());
    return true;
}

bool CUpdateMultipleSeq_Input::ReadUpdSeqs_FromClipboard(ILineErrorListener* msg_listener)
{
    if (m_UpdEntry) {
        NCBI_THROW(CSeqUpdateException, eInternal, "The update sequence(s) has already been submitted");
    }

    string fasta_str = CSimpleClipboard::Instance().Get<string>();
    CNcbiIstrstream istr(fasta_str.c_str());
    
    return x_ReadFromStream(istr, msg_listener);
}

bool CUpdateMultipleSeq_Input::ReadUpdSeqs_FromFile(ILineErrorListener* msg_listener)
{
    if (m_UpdEntry) {
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

bool CUpdateMultipleSeq_Input::x_ReadFromStream(CNcbiIstream& istr, ILineErrorListener* msg_listener)
{
    CFormatGuess guesser(istr);
    guesser.GetFormatHints().DisableAllNonpreferred();
    guesser.GetFormatHints().AddPreferredFormat(CFormatGuess::eFasta);
    guesser.GetFormatHints().AddPreferredFormat(CFormatGuess::eTextASN);

    CFormatGuess::EFormat format = guesser.GuessFormat();

    if (format == CFormatGuess::eTextASN) {
        return x_ReadUpdateSeqs_ASN(istr);
    }
    else if (format == CFormatGuess::eFasta ||
            format == CFormatGuess::eUnknown) {
        // with high probability, it is a Fasta file
        try {
            return x_ReadUpdateSeqs_Fasta(istr, msg_listener);
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

static const char* kNASeqCouldNotRead = "Failed to read update sequences";

bool CUpdateMultipleSeq_Input::x_ReadUpdateSeqs_ASN(CNcbiIstream& istr)
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

    if (oi.GetName() == "Seq-entry") {
        CSeq_entry* entry = CTypeConverter<CSeq_entry>::SafeCast(oi.GetObjectPtr());
        if (entry) {
            m_UpdEntry = Ref(entry);
        }
    }
    else if (oi.GetName() == "Seq-submit") {
        CSeq_submit* seq_submit = CTypeConverter<CSeq_submit>::SafeCast(oi.GetObjectPtr());
        if (seq_submit) {
            m_UpdEntry = edit::SeqEntryFromSeqSubmit(*seq_submit);
        }
    }

    return x_MatchIdsAndAddUpdateToScope();
}

bool CUpdateMultipleSeq_Input::x_ReadUpdateSeqs_Fasta(CNcbiIstream& istr, ILineErrorListener* msg_listener)
{
    CFastaReader::TFlags flags =
        CFastaReader::fNoSplit
        | CFastaReader::fUniqueIDs
        | CFastaReader::fHyphensIgnoreAndWarn
        | CFastaReader::fAddMods;


    try {
        CFastaReader fasta_reader(istr, flags);
        m_UpdEntry = fasta_reader.ReadSet(kMax_Int, msg_listener);
    }
    catch (const CException& e) {
        NCBI_THROW(CSeqUpdateException, eReading, e.GetMsg());
    }

    bool halt = false;
    if (msg_listener) {
        for (auto i = 0; i < msg_listener->Count(); ++i) {
            const ILineError& line_err = msg_listener->GetError(i);
            auto type = line_err.Problem();
            if (type == ILineError::eProblem_IgnoredResidue
                || type == ILineError::eProblem_InvalidResidue) {
                continue;
            }

            if (type == ILineError::eProblem_GeneralParsingError) {
                halt = true;
            }
            else {
                EDiagSev severity = line_err.GetSeverity();
                if (severity == eDiag_Warning
                    || severity == eDiag_Error
                    || severity == eDiag_Critical
                    || severity == eDiag_Fatal) {
                    NCBI_THROW(CSeqUpdateException, eReading, line_err.ErrorMessage());
                }
            }
        }
    }

    if (halt)
        return false;

    return x_MatchIdsAndAddUpdateToScope();
}

bool CUpdateMultipleSeq_Input::x_MatchIdsAndAddUpdateToScope()
{
    if (m_UpdEntry.IsNull()) {
        NCBI_THROW(CSeqUpdateException, eReading, "Update sequences could not be read");
    }

    m_UpdEntry->Parentize();

    _ASSERT(m_UpdEntry);
    _ASSERT(m_Scope);
    m_UnmatchedUpdates.clear();

    m_MolType = CSeq_inst::eMol_na;

    sequpd::TSeqIdHMap nmatches, pmatches;
    TSeqIDHVector nunmathed, punmatched;
    sequpd::FindMatches(CSeq_inst::eMol_na, m_OldSeh, *m_UpdEntry, nmatches, nunmathed);
    sequpd::FindMatches(CSeq_inst::eMol_aa, m_OldSeh, *m_UpdEntry, pmatches, punmatched);
    m_Scope->AddTopLevelSeqEntry(*m_UpdEntry);

    bool only_proteins = true;
    for (CTypeConstIterator<CBioseq> it(ConstBegin(*m_UpdEntry)); it; ++it) {
        if (!it->IsAa()) {
            only_proteins = false;
            break;
        }
    }

    if (only_proteins) {
        m_MolType = CSeq_inst::eMol_aa;
    }

    sequpd::TSeqIdHMap& matches = (only_proteins) ? pmatches : nmatches;
    m_UnmatchedUpdates = (only_proteins) ? punmatched : nunmathed;

    m_WithoutUpdates.clear();

    for (CBioseq_CI b_iter(m_OldSeh, m_MolType); b_iter; ++b_iter) {
        CSeq_id_Handle idh = sequpd::GetGoodSeqIdHandle(*b_iter);
        if (matches.find(idh) == matches.end())
            m_WithoutUpdates.push_back(idh);
    }

    m_NonIdentPairs.clear();
    m_IdenticalPairs.clear();

    for (const auto& m : matches) {
        CBioseq_Handle bsh1 = m_Scope->GetBioseqHandle(m.first);
        CBioseq_Handle bsh2 = m_Scope->GetBioseqHandle(m.second);
        _ASSERT(bsh1 && bsh2);
        if (sequpd::HaveIdenticalResidues(bsh1, bsh2))
            m_IdenticalPairs.insert(m);
        else
            m_NonIdentPairs.insert(m);
    }

    return true;
}

bool CUpdateMultipleSeq_Input::SetUpdateEntry(CRef<CSeq_entry> update)
{
    if (!update)
        return false;

    m_UpdEntry = Ref(update.GetNCPointer());
    return x_MatchIdsAndAddUpdateToScope();
}

bool CUpdateMultipleSeq_Input::AppendOld_UpdateSeqPairs(const TOldToUpdateMap& update_pairs)
{
    if (update_pairs.empty()) {
        return false;
    }

    // first check which sequences are identical
    TOldToUpdateMap toAppendNonIdent;
    TOldToUpdateMap toAppendIdent;

    for (auto&& it : update_pairs) {
        CSeq_id_Handle old_idh = it.first;
        CSeq_id_Handle upd_idh = it.second;
        if (m_NonIdentPairs.find(old_idh) != m_NonIdentPairs.end()
            || m_IdenticalPairs.find(old_idh) != m_IdenticalPairs.end()) {
            NCBI_THROW(CSeqUpdateException, eReading, "Non-unique sequence IDs in update sequences!");
        }
        else {
            CBioseq_Handle old_bsh = m_Scope->GetBioseqHandle(old_idh);
            CBioseq_Handle upd_bsh = m_Scope->GetBioseqHandle(upd_idh);
            if (!old_bsh || !upd_bsh) {
                NCBI_THROW(CSeqUpdateException, eInternal, "Failed to match update sequence with old one.");
            }
            if (sequpd::HaveIdenticalResidues(old_bsh, upd_bsh)) {
                m_IdenticalPairs.emplace(old_idh, upd_idh);
                toAppendIdent.emplace(old_idh, upd_idh);
            }
            else {
                m_NonIdentPairs.emplace(old_idh, upd_idh);
                toAppendNonIdent.emplace(old_idh, upd_idh);
            }

            auto del_it = find(m_WithoutUpdates.begin(), m_WithoutUpdates.end(), old_idh);
            if (del_it != m_WithoutUpdates.end()) {
                m_WithoutUpdates.erase(del_it);
            }

            del_it = find(m_UnmatchedUpdates.begin(), m_UnmatchedUpdates.end(), upd_idh);
            if (del_it != m_UnmatchedUpdates.end()) {
                m_UnmatchedUpdates.erase(del_it);
            }
        }
    }

    if (toAppendNonIdent.empty() && toAppendIdent.empty()) {
        LOG_POST(Info << "AppendSequenceInputs was unnecessarily called.");
        return true;
    }

    bool nonidenticalsOk = x_CreateUpdateSeqInput(toAppendNonIdent, m_NonIdentUpdates);
    bool identicalsOk = x_CreateUpdateSeqInput(toAppendIdent, m_IdenticalUpdates);
    return nonidenticalsOk && identicalsOk;
}

void CUpdateMultipleSeq_Input::PrepareMultipleSequenceInputs()
{
    x_CreateUpdateSeqInput(m_NonIdentPairs, m_NonIdentUpdates);
    x_CreateUpdateSeqInput(m_IdenticalPairs, m_IdenticalUpdates);
}

bool CUpdateMultipleSeq_Input::HaveIdenticalResidues(const CSeq_id_Handle& idh1, const CSeq_id_Handle& idh2)
{
    CBioseq_Handle bsh1 = m_Scope->GetBioseqHandle(idh1);
    CBioseq_Handle bsh2 = m_Scope->GetBioseqHandle(idh2);
    return sequpd::HaveIdenticalResidues(bsh1, bsh2);
}

bool CUpdateMultipleSeq_Input::x_CreateUpdateSeqInput(const TOldToUpdateMap& updatepairs, TIDToUpdInputMap& inputmap)
{
    if (updatepairs.empty()) {
        return true;
    }

    for (auto&& it : updatepairs) {
        CBioseq_Handle oldBsh = m_Scope->GetBioseqHandle(it.first);
        CBioseq_Handle updBsh = m_Scope->GetBioseqHandle(it.second);
        _ASSERT(oldBsh && updBsh);

        CRef<CUpdateSeq_Input> updInput(new CUpdateSeq_Input(oldBsh, updBsh));
        // no alignment is generated at this point
        inputmap.insert(make_pair(it.first, updInput));
    }
    return true;
}


bool CUpdateMultipleSeq_Input::PrepareMultipleSequenceInputsForUnitTest()
{
    bool nonidenticalsOk = x_CreateUpdateSeqInputForUnitTest(m_NonIdentPairs, m_NonIdentUpdates);
    bool identicalsOk = x_CreateUpdateSeqInputForUnitTest(m_IdenticalPairs, m_IdenticalUpdates);
    return nonidenticalsOk && identicalsOk;
}

bool CUpdateMultipleSeq_Input::x_CreateUpdateSeqInputForUnitTest(const TOldToUpdateMap& updatepairs, TIDToUpdInputMap& inputmap)
{
    if (updatepairs.empty()) {
        return true;
    }

    for (auto&& it : updatepairs) {
        CBioseq_Handle oldBsh = m_Scope->GetBioseqHandle(it.first);
        CBioseq_Handle updBsh = m_Scope->GetBioseqHandle(it.second);
        _ASSERT(oldBsh && updBsh);

        CRef<CUpdateSeq_Input> updInput(new CUpdateSeq_Input(oldBsh, updBsh));
        try {
            updInput->CalculateAlignmentForUnitTest();
        }
        catch (exception &) {
            string msg("Blasting between ");
            msg.append(it.first.AsString());
            msg.append(" and ");
            msg.append(it.second.AsString());
            msg.append(" has failed. ");
            LOG_POST(Error << msg);
            NCBI_THROW(CSeqUpdateException, eAlignment, msg);
        }
        inputmap.insert(make_pair(it.first, updInput));
    }
    return true;
}

CRef<CUpdateSeq_Input> CUpdateMultipleSeq_Input::GetUpdateInput(const CSeq_id_Handle& idh) const
{
    auto nonident_it = m_NonIdentUpdates.find(idh);
    if (nonident_it != m_NonIdentUpdates.end()) {
        return nonident_it->second;
    } else {
        auto ident_it = m_IdenticalUpdates.find(idh);
        if (ident_it != m_IdenticalUpdates.end()) {
            return ident_it->second;
        }
    }

    return CRef<CUpdateSeq_Input>();
}

END_NCBI_SCOPE
