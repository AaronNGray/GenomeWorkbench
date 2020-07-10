/*  $Id: update_seq_worker.cpp 44179 2019-11-12 20:55:18Z asztalos $
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
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/update_seq_worker.hpp>
#include <gui/packages/pkg_sequence_edit/update_seq.hpp>
#include <gui/packages/pkg_sequence_edit/update_seq_dlg.hpp>

#include <gui/packages/pkg_sequence_edit/update_multi_seq_input.hpp>
#include <gui/packages/pkg_sequence_edit/update_multi_seq_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/sequpd_except.hpp>
#include <wx/msgdlg.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CSeqUpdateWorker::CSeqUpdateWorker()
    : m_Type(CSeqUpdateWorker::eUpdateNotSet)
{
}

static string s_WarnAboutIgnoredResidues(ILineErrorListener& msg_listener)
{
    string message;
    for (size_t i = 0; i < msg_listener.Count(); ++i) {
        const ILineError& line_err = msg_listener.GetError(i);
        if (line_err.Problem() == ILineError::eProblem_IgnoredResidue) {
            if (line_err.GetSeverity() == eDiag_Warning
                && NStr::Find(line_err.ErrorMessage(), "Hyphens") != NPOS) {
                message += ("Hyphens in the update sequence have been ignored.\n");
            }
        }
        else if (line_err.Problem() == ILineError::eProblem_InvalidResidue) {
            if (NStr::Find(line_err.ErrorMessage(), "Ignoring invalid residues") != NPOS) {
                SIZE_TYPE pos = NStr::Find(line_err.ErrorMessage(), "On line");
                if (pos != NPOS) {
                    message += ("Invalid character was found (and ignored):");
                    message += line_err.ErrorMessage().substr(pos + 2, NPOS);
                    message += "\n";
                }
            }
        }
    }

    return message;
}

void CSeqUpdateWorker::UpdateSingleSequence(IWorkbench* workbench, const CSeq_entry_Handle& seh, ICommandProccessor* cmdProcessor) const
{
    CUpdateSeq_Input updseq_in;
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateReplace);
    bool input_ok = false;
    string warn_message;
    try {
        input_ok = updseq_in.SetOldBioseqAndScope(workbench, seh);
        switch (m_Type) {
        case eSingle_File:
        {
            CMessageListenerLenient msg_container;
            input_ok = input_ok && updseq_in.ReadSingleUpdateSequence(&msg_container);
            warn_message = s_WarnAboutIgnoredResidues(msg_container);
            break;
        }
        case eSingle_Clipboard:
        {
            CMessageListenerLenient msg_container;
            input_ok = input_ok && updseq_in.ReadUpdateSequenceFromClipboard(&msg_container);
            warn_message = s_WarnAboutIgnoredResidues(msg_container);
            break;
        }
        case eSingle_Accession:
            input_ok = input_ok && updseq_in.ReadSingleAccession();
            params.m_SeqUpdateOption = SUpdateSeqParams::eSeqUpdateNoChange;
            break;
        default:
            input_ok = false;
            break;
        }
        // sequences are read at this point, but no alignment is generated for now

        if (!input_ok)
            return;
    }
    catch (const CSeqUpdateException& e) {
        NcbiMessageBox(e.GetMsg());
        return;
    }
    catch (const CException& e) {
        ERR_POST(e.what());
        return;
    }

    unsigned int count_bioseqs = updseq_in.CountBioseqs();
    if (count_bioseqs > 1) {
        string msg("You selected update single sequence but the update file contains " 
            + NStr::UIntToString(count_bioseqs) +" bioseqs. Do you want to continue and use the first record only?");
        if (eYes != NcbiMessageBox(msg, eDialog_YesNo, eIcon_Question, "Confirm")) {
            return;
        }
    }
    if (!warn_message.empty()) {
        NcbiWarningBox(warn_message);
    }

    x_LaunchSingleUpdateSequence(updseq_in, params, seh, cmdProcessor);
}

void CSeqUpdateWorker::UpdateSingleSequence_Ext(IWorkbench* workbench, const CSeq_entry_Handle& seh, ICommandProccessor* cmdProcessor) const
{
    CUpdateSeq_Input updseq_in;
    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateReplace);
    string warn_message;

    try {
        bool input_ok = updseq_in.SetOldBioseqAndScope(workbench, seh);
        if (!input_ok) return;
    }
    catch (const CSeqUpdateException& e) {
        NcbiMessageBox(e.GetMsg());
        return;
    }
    catch (const CException& e) {
        ERR_POST(e.what());
        return;
    }
    
    EDialogReturnValue res = eCancel;
    do {
        try {
            if (m_Type == eSingle_File) {
                CMessageListenerLenient msg_container;
                bool input_ok = updseq_in.ReadSingleUpdateSequence(&msg_container);
                warn_message = s_WarnAboutIgnoredResidues(msg_container);
                if (!input_ok) return;
            }
            // sequences are read at this point, but no alignment is generated for now
        }
        catch (const CSeqUpdateException& e) {
            NcbiMessageBox(e.GetMsg());
            return;
        }
        catch (const CException& e) {
            ERR_POST(e.what());
            return;
        }

        auto count_bioseqs = updseq_in.CountBioseqs();
        if (count_bioseqs > 1) {
            string msg("You selected update single sequence but the update file contains "
                + NStr::UIntToString(count_bioseqs) + " bioseqs. Do you want to continue and use the first record only?");

            res = NcbiMessageBox(msg, eDialog_YesNo, eIcon_Question, "Confirm");
            if (res == eNo) {
                updseq_in.ResetUpdateSequence();
                warn_message.clear();
            }
        }
    
    } while (res == eNo);

    if (!warn_message.empty()) {
        NcbiWarningBox(warn_message);
    }

    x_LaunchSingleUpdateSequence(updseq_in, params, seh, cmdProcessor);
}

void CSeqUpdateWorker::x_LaunchSingleUpdateSequence(
    CUpdateSeq_Input& updseq_in, 
    SUpdateSeqParams& params, 
    const CSeq_entry_Handle& seh, 
    ICommandProccessor* cmdProcessor) const
{
    const CBioseq_Handle& oldBsh = updseq_in.GetOldBioseq();
    const CBioseq_Handle& updBsh = updseq_in.GetUpdateBioseq();
    if ((CUpdateSeq_Input::s_IsDeltaWithNoGaps(*oldBsh.GetCompleteBioseq()) || CUpdateSeq_Input::s_IsDeltaWithFarPointers(*oldBsh.GetCompleteBioseq()))
        && updBsh.IsSetInst_Repr() && (updBsh.GetInst_Repr() == CSeq_inst::eRepr_raw)) {
        string msg("You are about to update a delta sequence with a raw sequence, which\n will convert the delta sequence to raw. Do you want to continue?");
        if (eYes != NcbiMessageBox(msg, eDialog_YesNo, eIcon_Question, "Confirm")) {
            return;
        }
    }
    if (CUpdateSeq_Input::s_IsDeltaWithFarPointers(*updBsh.GetCompleteBioseq())) {
        string msg("The update sequence is a delta sequence with far pointers. Do you want to continue?");
        if (eYes != NcbiMessageBox(msg, eDialog_YesNo, eIcon_Question, "Confirm")) {
            return;
        }
    }

    try {
        CUpdateSeq_Dlg dlg(NULL, updseq_in);
        dlg.SetData(params);

        if (dlg.ShowModal() == wxID_OK) {
            params = dlg.GetData();
            if (!params.AreUpdateParamsValid()) {
                NcbiMessageBox("Invalid options selected!");
                return;
            }
            CSequenceUpdater updater(updseq_in, params);
            bool create_general_only = edit::IsGeneralIdProtPresent(seh.GetTopLevelEntry());
            CRef<CCmdComposite> cmd = updater.Update(create_general_only);
            if (cmd) {
                cmdProcessor->Execute(cmd);
                const string& citsub_msg = updater.GetCitSubMessage();
                if (!citsub_msg.empty()) {
                    NcbiInfoBox(citsub_msg);
                }

                string report;
                CNcbiOstrstream oss;
                updater.GetNotImportedFeatsReport(oss);
                if (!IsOssEmpty(oss)) {
                    report = string(CNcbiOstrstreamToString(oss));
                    report.append("\n");
                }
                report.append(updater.GetRevCompReport());

                if (!report.empty()) {
                    CGenericReportDlg* reportdlg = new CGenericReportDlg(NULL);
                    reportdlg->SetTitle("Update Sequence Log");
                    reportdlg->SetText(ToWxString(report));
                    reportdlg->Show(true);
                }
            }
        }
    }
    catch (const CSeqUpdateException& e) {
        NcbiMessageBox(e.GetMsg());
    }
    catch (const CException& e) {
        LOG_POST(Error << e.what());
        NcbiMessageBox("Could not update the sequence");
    }
}

void CSeqUpdateWorker::UpdateMultipleSequences(IWorkbench* workbench, const CSeq_entry_Handle& seh, ICommandProccessor* cmdProcessor)
{
    CUpdateMultipleSeq_Input multiupdseq_in;
    multiupdseq_in.SetOldEntryAndScope(seh);
    bool input_ok = false;

    try {
        switch (m_Type) {
        case eMultiple_File:
        {
            CMessageListenerLenient msg_container;
            input_ok = multiupdseq_in.ReadUpdSeqs_FromFile(&msg_container);
            x_GetSeqsWithIgnoredResidues(msg_container);
            break;
        }
        case eMultiple_Clipboard:
        {
            CMessageListenerLenient msg_container;
            input_ok = multiupdseq_in.ReadUpdSeqs_FromClipboard(&msg_container);
            x_GetSeqsWithIgnoredResidues(msg_container);
            break;
        }
        default:
            input_ok = false;
            break;
        }
            
        if (!input_ok)
            return;
    }
    catch (const CSeqUpdateException& e) {
        NcbiMessageBox(e.GetMsg());
        return;
    }
    catch (const CException& e) {
        string msg = e.GetMsg();
        msg[0] = toupper(msg[0]);
        NcbiMessageBox(msg);
        return;
    }

    try {
        CRef<CCmdComposite> update_cmd(new CCmdComposite("Update multiple sequences"));
        CUpdateMultiSeq_Dlg dlg(NULL, multiupdseq_in, update_cmd);
        int retcode = dlg.ShowModal();
        if (retcode == wxOK || retcode == wxCLOSE) {
            // execute the command of updates
            if (update_cmd && dlg.AnyUpdated()) {
                cmdProcessor->Execute(update_cmd);
                dlg.ReportStats();
                string msg = dlg.GetRevCompReport();
                if (!msg.empty()) {
                    CGenericReportDlg* report = new CGenericReportDlg(NULL);
                    report->SetTitle("Update Sequence Log");
                    report->SetText(ToWxString(msg));
                    report->Show(true);
                }
            }
        }
    }
    catch (const CSeqUpdateException& e) {
        NcbiMessageBox(e.GetMsg());
    }
    catch (const CException& e) {
        LOG_POST(Error << e.what());
        NcbiMessageBox("Could not update the sequences");
    }
}

void CSeqUpdateWorker::x_GetSeqsWithIgnoredResidues(ILineErrorListener& msg_listener)
{
    string hyphens_msg;
    string invalidRes_msg;
    string duplicateids_msg;

    for (size_t i = 0; i < msg_listener.Count(); ++i) {
        const ILineError& line_err = msg_listener.GetError(i);
        auto type = line_err.Problem();
        auto msg = line_err.ErrorMessage();

        if (type == ILineError::eProblem_IgnoredResidue) {
            if (line_err.GetSeverity() == eDiag_Warning
                && NStr::Find(msg, "Hyphens") != NPOS) {
                if (NStr::Find(hyphens_msg, line_err.SeqId()) == NPOS) {
                    if (!hyphens_msg.empty()) {
                        hyphens_msg += ", ";
                    }
                    hyphens_msg += line_err.SeqId();
                }
            }
        }
        else if (type == ILineError::eProblem_InvalidResidue) {
            if (NStr::Find(msg, "Ignoring invalid residues") != NPOS) {
                SIZE_TYPE pos = NStr::Find(msg, "On line");
                if (pos != NPOS) {
                    if (!invalidRes_msg.empty()) {
                        invalidRes_msg += "\n";
                    }
                    invalidRes_msg += line_err.SeqId();
                    invalidRes_msg += msg.substr(pos + 2, NPOS);
                }
            }
        }
        else if (type == ILineError::eProblem_GeneralParsingError) {
            if (msg.find("Seq-id") != NPOS && msg.find("is a duplicate") != NPOS) {
                if (duplicateids_msg.find(line_err.SeqId()) == NPOS) {
                    if (!duplicateids_msg.empty()) {
                        duplicateids_msg += "\n";
                    }
                    duplicateids_msg += line_err.SeqId();
                }
            }
        }
    }

    string message;
    if (!hyphens_msg.empty()) {
        message += "Hyphens have been ignored in the following update sequences:\n";
        message += hyphens_msg;
    }

    if (!invalidRes_msg.empty()) {
        if (!message.empty()) {
            message += "\n";
        }
        message += "Invalid residues were found and ignored in the following update sequences:\n";
        message += invalidRes_msg;
    }

    if (!message.empty()) {
        NcbiWarningBox(message);
    }

    if (!duplicateids_msg.empty()) {
        duplicateids_msg = "Duplicate sequence ids were found in the update sequence file:\n" + duplicateids_msg;
        CGenericReportDlg* reportdlg = new CGenericReportDlg(NULL);
        reportdlg->SetTitle("Duplicate seq-ids");
        reportdlg->SetText(ToWxString(duplicateids_msg));
        reportdlg->Show(true);
    }
}


END_NCBI_SCOPE
