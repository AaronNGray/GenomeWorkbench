/*  $Id: common_commands.cpp 44490 2020-01-03 16:14:20Z filippov $
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


#include <ncbi_pch.hpp>
#include <objects/valerr/ValidErrItem.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/objects/GBWorkspace.hpp>
#include <gui/widgets/edit/init.hpp>
#include <gui/widgets/edit/edit_object.hpp>
#include <gui/widgets/edit/edit_object_seq_feat.hpp>
#include <gui/widgets/edit/edit_object_seq_desc.hpp>
#include <gui/packages/pkg_sequence_edit/common_commands.hpp>
#include <gui/packages/pkg_sequence_edit/create_seq_hist_for_tpa.hpp>
#include <gui/packages/pkg_sequence_edit/edit_history.hpp>
#include <gui/packages/pkg_sequence_edit/remove_sequences.hpp>
#include <gui/packages/pkg_sequence_edit/trim_n.hpp>
#include <gui/packages/pkg_sequence_edit/apply_gcode.hpp>
#include <gui/packages/pkg_sequence_edit/remove_structured_comment.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>
#include <gui/widgets/edit/bioseq_editor.hpp>
#include <gui/widgets/edit/edit_obj_view_dlg.hpp>
#include <gui/widgets/edit/alignment_assistant.hpp>
#include <gui/widgets/edit/edit_sequence.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/packages/pkg_sequence_edit/retranslate_cds.hpp>
#include <gui/packages/pkg_sequence_edit/apply_gcode.hpp>
#include <gui/packages/pkg_sequence_edit/rem_prot_titles.hpp>
#include <gui/packages/pkg_sequence_edit/raw_seq_to_delta_by_n.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <array>

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
#include "wx/imaglist.h"
#include <wx/display.h>
////@end includes

#include <wx/filename.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(valedit)

ICommandProccessor* CommandProcessorFromWorkbench(IWorkbench* wb, CScope& scope)
{
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(wb);
    if (!wb_frame)
        return NULL;

    CIRef<CProjectService> srv = wb_frame->GetServiceByType<CProjectService>();
    if (!srv)
        return NULL;

    CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
    if (!ws)
        return NULL;

    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(scope));
    if (!doc)
        return NULL;
    ICommandProccessor* cmdProcessor = &doc->GetUndoManager();
    return cmdProcessor;
}


wxWindow* MainWindowFromWorkbench(IWorkbench* wb)
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(wb);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    return main_window;
}


wxString GetWorkDirFromWorkbench(IWorkbench* wb, CScope& scope)
{
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(wb);
    if (!wb_frame)
        return wxEmptyString;

    CIRef<CProjectService> srv = wb_frame->GetServiceByType<CProjectService>();
    _ASSERT(srv);
    CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
    CGBDocument* doc = 0;
    if (ws)
        doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(scope));

    if (doc) {
        return doc->GetWorkDir();
    } else {
        return wxEmptyString;
    }
}


void LaunchDescEditor(CRef<CSeqdesc> seqdesc, CSeq_entry_Handle seh, ICommandProccessor* cmdProcessor, const wxString& workDir)
{    
    CIRef<IEditObject> editor(new CEditObjectSeq_desc(*seqdesc, seh, seh.GetScope(), true));
    CEditObjViewDlg* edit_dlg = new CEditObjViewDlg(NULL, true);
    edit_dlg->SetUndoManager(cmdProcessor);
    wxWindow* editorWindow = editor->CreateWindow(edit_dlg);
    edit_dlg->SetEditorWindow(editorWindow);
    edit_dlg->SetEditor(editor);
    edit_dlg->SetWorkDir(workDir);
    edit_dlg->Show(true);
}


void LaunchDescEditorForSeq
(CRef<CSeqdesc> seqdesc, 
 const objects::CBioseq& bioseq, 
 ICommandProccessor* cmdProcessor, 
 CScope& scope, 
 const wxString& workDir)
{
    CBioseq_Handle bsh = scope.GetBioseqHandle(bioseq);
    CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
    LaunchDescEditor(seqdesc, seh, cmdProcessor, workDir);
}


void LaunchMolInfoEditor(CSeq_entry_Handle seh, IWorkbench* wb)
{
    CRef<CSeqdesc> seqdesc(new CSeqdesc());
    seqdesc->SetMolinfo();
    ICommandProccessor* cmdProcessor = CommandProcessorFromWorkbench(wb, seh.GetScope());
    const wxString& workDir = GetWorkDirFromWorkbench(wb, seh.GetScope());
    LaunchDescEditor(seqdesc, seh, cmdProcessor, workDir);
}


void LaunchSrcEditor(CSeq_entry_Handle seh, IWorkbench* wb)
{
    CRef<CSeqdesc> seqdesc(new CSeqdesc());
    seqdesc->SetSource();
    ICommandProccessor* cmdProcessor = CommandProcessorFromWorkbench(wb, seh.GetScope());
    const wxString& workDir = GetWorkDirFromWorkbench(wb, seh.GetScope());
    LaunchDescEditor(seqdesc, seh, cmdProcessor, workDir);
}

void RemoveSequence(CSeq_entry_Handle seh, IWorkbench* wb)
{
    if (!seh.IsSeq()) {
        return;
    }
    wxWindow* main_window = MainWindowFromWorkbench(wb);
    ICommandProccessor* cmd_proc = CommandProcessorFromWorkbench(wb, seh.GetScope());

    CRemoveSequencesDlg dlg(main_window, seh.GetTopLevelEntry());
    dlg.ChooseSequence(seh.GetSeq());
    if (dlg.ShowModal() == wxID_OK) {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd) {
            cmd_proc->Execute(cmd);
        }
    }
}

void TrimTerminalNRichRegions(CSeq_entry_Handle seh, IWorkbench* wb)
{
    if (wxYES != wxMessageBox(wxT("Would you like to trim terminal regions with a high percentage of ambiguous characters from the sequences?"), wxT("Trim Ns"), wxYES_NO | wxICON_QUESTION)) {
        return;
    }

    wxWindow* main_window = MainWindowFromWorkbench(wb);
    ICommandProccessor* cmd_proc = CommandProcessorFromWorkbench(wb, seh.GetScope());
    const wxString& workDir = GetWorkDirFromWorkbench(wb, seh.GetScope());

    CTrimN worker;
    worker.apply(seh.GetTopLevelEntry(), cmd_proc, "Trim Ns Rich", main_window, workDir);
}


void OfferToDeleteDescriptor(const string& label, CSeq_entry_Handle seh, const CSeqdesc& seqDesc, ICommandProccessor* cmdProcessor)
{
    if (wxYES == wxMessageBox(wxT("Would you like to delete " + ToWxString(label) + wxT("?")), wxT("Remove descriptors"), wxYES_NO | wxICON_QUESTION)) {
        CIRef<IEditCommand> cmd(new CCmdDelDesc(seh, seqDesc));
        cmdProcessor->Execute(cmd);
    } 
}

void LaunchAlignmentAssistant(CSeq_entry_Handle seh, IWorkbench* wb, const CSeq_align *align)
{
    wxWindow* main_window = MainWindowFromWorkbench(wb);
    ICommandProccessor* cmdProcessor = CommandProcessorFromWorkbench(wb, seh.GetScope());
    CAlignmentAssistant *dlg = NULL;
    try
    {
        dlg = new CAlignmentAssistant( main_window, seh, cmdProcessor, ConstRef(align)); 
        dlg->Show(true);      
    } catch  (CException&) 
    {
        if (dlg)
            dlg->Destroy();
        wxMessageBox(wxT("Unable to load the alignment"), wxT("Error"), wxOK | wxICON_ERROR);
    }
}

void LaunchBioseqEditor(CSeq_entry_Handle seh, IWorkbench* wb)
{
    CBioseq_Handle bsh;
    if (seh.IsSeq())
        bsh = seh.GetSeq();
    if (bsh)
    {
        ICommandProccessor* cmdProcessor = CommandProcessorFromWorkbench(wb, seh.GetScope());
        CEditSequence *dlg = NULL;
        try
        {
            dlg = new CEditSequence( nullptr, bsh.GetRangeSeq_loc(0,0).GetPointer(), seh.GetScope(), cmdProcessor, 0); 
            dlg->Show(true);      
        } catch  (CException&) 
        {
            if (dlg)
                dlg->Destroy();
            wxMessageBox(wxT("No bioseq selected"), wxT("Error"), wxOK | wxICON_ERROR);
        }
    }
}

void LaunchDefaultEditor(CSeq_entry_Handle seh, IWorkbench* wb, const CSerialObject&  obj)
{
    const CObject* object = &obj;
    const CSeq_align *align = dynamic_cast<const CSeq_align *>(object);
    const CBioseq *bioseq = dynamic_cast<const CBioseq *>(object);

    if (align)
    {
        LaunchAlignmentAssistant(seh, wb, align);          
        return;
    }
    if (bioseq)
    {
        LaunchBioseqEditor(seh, wb);
        return;
    }

    CIRef<IEditObject> editor = CreateEditorForObject(ConstRef(object), seh, false);
    if (!editor)
        return;
    wxWindow* main_window = MainWindowFromWorkbench(wb);
    ICommandProccessor* cmdProcessor = CommandProcessorFromWorkbench(wb, seh.GetScope());
    CEditObjViewDlg* edit_dlg = new CEditObjViewDlg(main_window, false);
    wxWindow* editorWindow = editor->CreateWindow(edit_dlg);
    edit_dlg->SetUndoManager(cmdProcessor);
    edit_dlg->SetEditorWindow(editorWindow);
    edit_dlg->SetEditor(editor);
    editorWindow->TransferDataToWindow(); 
    edit_dlg->Show(true);  
}

using handler=void(*)(CSeq_entry_Handle seh, IWorkbench* wb, const CSerialObject&  obj);
static std::array<handler, eErr_MAX> valerr_to_handler{};

#define REGISTER_HANDLER(code)     \
    static void handler_##code (CSeq_entry_Handle seh, IWorkbench* wb, const CSerialObject&  obj); \
    static int dummy_##code = (valerr_to_handler[code] = handler_##code, 0);       \
    void handler_##code (CSeq_entry_Handle seh, IWorkbench* wb, const CSerialObject&  obj)     \

REGISTER_HANDLER(eErr_SEQ_DESCR_NoPubFound)
{
    CRef<CSeqdesc> seqdesc(new CSeqdesc());
    seqdesc->SetPub();
    ICommandProccessor* cmdProcessor = CommandProcessorFromWorkbench(wb, seh.GetScope());
    const wxString& workDir = GetWorkDirFromWorkbench(wb, seh.GetScope());
    LaunchDescEditor(seqdesc, seh, cmdProcessor, workDir);
}

REGISTER_HANDLER(eErr_GENERIC_MissingPubRequirement)
{
    CRef<CSeqdesc> seqdesc(new CSeqdesc());
    seqdesc->SetPub();
    CRef<CPub> pub(new CPub());
    pub->SetSub();
    seqdesc->SetPub().SetPub().Set().push_back(pub);
    ICommandProccessor* cmdProcessor = CommandProcessorFromWorkbench(wb, seh.GetScope());
    const wxString& workDir = GetWorkDirFromWorkbench(wb, seh.GetScope());
    LaunchDescEditor(seqdesc, seh, cmdProcessor, workDir);
}

REGISTER_HANDLER(eErr_SEQ_DESCR_NoSourceDescriptor)
{
    LaunchSrcEditor(seh, wb);
}

REGISTER_HANDLER(eErr_SEQ_INST_MitoMetazoanTooLong)
{
    LaunchSrcEditor(seh, wb);
}

REGISTER_HANDLER(eErr_SEQ_DESCR_NoMolInfoFound)
{
    LaunchMolInfoEditor(seh, wb);
}

REGISTER_HANDLER(eErr_SEQ_INST_MolNuclAcid)
{
    LaunchMolInfoEditor(seh, wb);
}

REGISTER_HANDLER(eErr_SEQ_INST_HistAssemblyMissing)
{
    wxWindow* main_window = MainWindowFromWorkbench(wb);
    ICommandProccessor* cmdProcessor = CommandProcessorFromWorkbench(wb, seh.GetScope());

    CSeq_entry_Handle tseh = seh.GetTopLevelEntry();

    string msg;
    CRef<CCmdComposite> cmd = CCreateSeqHistForTpa::GetCreateSeqHistFromTpaCommand(tseh, msg, 28, 0.000001);

    if (cmd)
    {
        cmdProcessor->Execute(cmd);
    }

    if (!msg.empty())
    {
        CGenericReportDlg* report = new CGenericReportDlg(main_window);
        report->SetTitle(wxT("TPA Alignment Assembly Problems"));
        report->SetText(wxString(msg));
        report->Show(true);
    } 
}

REGISTER_HANDLER(eErr_SEQ_INST_HistoryGiCollision)
{
    if (!seh.IsSeq()) {
        return;
    }
    wxWindow* main_window = MainWindowFromWorkbench(wb);
    ICommandProccessor* cmd_proc = CommandProcessorFromWorkbench(wb, seh.GetScope());
    try
    {
        CEditHistory dlg(main_window, seh.GetSeq());

        if (dlg.ShowModal() == wxID_OK)
        {
            CRef<CCmdComposite> cmd = dlg.GetCommand();
            if (cmd)
            {
                cmd_proc->Execute(cmd);
            }
        }
    } catch (CException&) {
    }
}

REGISTER_HANDLER(eErr_SEQ_INST_ShortSeq)
{
    RemoveSequence(seh, wb);
}

REGISTER_HANDLER(eErr_SEQ_PKG_OrphanedProtein)
{
    if (wxYES != wxMessageBox(wxT("Remove all orphaned proteins?"), wxT("Remove"), wxYES_NO | wxICON_QUESTION)) 
    {
        return;
    }
    ICommandProccessor* cmd_proc = CommandProcessorFromWorkbench(wb, seh.GetScope());
    CRef<CCmdComposite> cmd = CRemoveProteins::OrphanedProteins(seh.GetTopLevelEntry());
    if (cmd) 
    {
        cmd_proc->Execute(cmd);                       
    }     
}

REGISTER_HANDLER(eErr_SEQ_INST_TerminalNs)
{
    if (wxYES != wxMessageBox(wxT("Would you like to trim ambiguous characters from the ends of the sequences?"), wxT("Trim Ns"), wxYES_NO | wxICON_QUESTION)) {
        return;
    }

    wxWindow* main_window = MainWindowFromWorkbench(wb);
    ICommandProccessor* cmd_proc = CommandProcessorFromWorkbench(wb, seh.GetScope());
    const wxString& workDir = GetWorkDirFromWorkbench(wb, seh.GetScope());

    CSequenceAmbigTrimmer::TTrimRuleVec trim_rules;
    trim_rules.push_back(CSequenceAmbigTrimmer::STrimRule{ 1, 0 });
    
    CTrimN worker;
    worker.apply(seh.GetTopLevelEntry(), cmd_proc, "Trim Ns Terminal", main_window, workDir, trim_rules);
}

REGISTER_HANDLER(eErr_SEQ_INST_HighNpercent5Prime)
{
    TrimTerminalNRichRegions(seh, wb);
}

REGISTER_HANDLER(eErr_SEQ_INST_HighNpercent3Prime)
{
    TrimTerminalNRichRegions(seh, wb);
}

REGISTER_HANDLER(eErr_SEQ_DESCR_TitleNotAppropriateForSet)
{
    ICommandProccessor* cmdProcessor = CommandProcessorFromWorkbench(wb, seh.GetScope());
    const CSeqdesc* d = dynamic_cast<const CSeqdesc *>(&obj);
    OfferToDeleteDescriptor("the title descriptor", seh, *d, cmdProcessor);
}

REGISTER_HANDLER(eErr_SEQ_DESCR_MultipleStrucComms)
{
    if (wxYES != wxMessageBox(wxT("Remove duplicate structured comments?"), wxT("Remove"), wxYES_NO | wxICON_QUESTION)) {
        return;
    }
    CRemoveDuplicateStructuredComment worker;
    ICommandProccessor* cmdProcessor = CommandProcessorFromWorkbench(wb, seh.GetScope());
    worker.apply(seh.GetTopLevelEntry(), cmdProcessor,"Remove Duplicate Structured Comment");
}

REGISTER_HANDLER(eErr_SEQ_FEAT_GenCodeMismatch)
{
    seh = seh.GetTopLevelEntry();
    wxWindow* main_window = MainWindowFromWorkbench(wb);
    ICommandProccessor* cmd_proc = CommandProcessorFromWorkbench(wb, seh.GetScope());
    CApplyGCode dlg(main_window, seh);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd) 
        {
            cmd_proc->Execute(cmd);                       
            CRef<CCmdComposite> cleanup = CleanupCommand(seh, true, false);
            if (cleanup)
                cmd_proc->Execute(cleanup);
            string error;
            CRef<CCmdComposite> retranslate = CRetranslateCDS::GetCommand(seh, "Retranslate coding regions", CRetranslateCDS::eRetranslateCDSIgnoreStopExceptEnd, error);
            if (retranslate)
                cmd_proc->Execute(retranslate);
        } 
    }        
}

REGISTER_HANDLER(eErr_SEQ_DESCR_InconsistentProteinTitle)
{
    if (wxYES != wxMessageBox(wxT("Would you like to remove protein titles?"), wxT("Remove"), wxYES_NO | wxICON_QUESTION)) {
        return;
    }
    CRemProtTitles worker;
    ICommandProccessor* cmdProcessor = CommandProcessorFromWorkbench(wb, seh.GetScope());
    worker.apply(seh.GetTopLevelEntry(), cmdProcessor,"Remove Protein Titles");
}

REGISTER_HANDLER(eErr_SEQ_INST_AllNs)
{
    RemoveSequence(seh, wb);
}

REGISTER_HANDLER(eErr_SEQ_INST_InternalNsInSeqRaw)
{
    if (!seh.IsSeq()) {
        return;
    }
    wxWindow* main_window = MainWindowFromWorkbench(wb);
    ICommandProccessor* cmd_proc = CommandProcessorFromWorkbench(wb, seh.GetScope());

    CRawSeqToDeltaByN dlg(main_window);
    if (dlg.ShowModal() == wxID_OK) 
    {
        dlg.apply(seh.GetTopLevelEntry(), cmd_proc, "Raw Sequences to Delta by Ns");
    }
}

void LaunchEditor(int err_code, const CSerialObject&  obj, CSeq_entry_Handle seh, IWorkbench* wb)
{
    if (valerr_to_handler[err_code])
    {
        valerr_to_handler[err_code](seh, wb, obj);
        return;
    }
    LaunchDefaultEditor(seh, wb, obj);
}

bool IsEditable(int err_code, const CSerialObject&  obj, CSeq_entry_Handle seh)
{
    if (valerr_to_handler[err_code])
    {
        return true;
    }
    const CObject* object = &obj;
    const CSeq_align *align = dynamic_cast<const CSeq_align *>(object);
    const CBioseq *bioseq = dynamic_cast<const CBioseq *>(object);

    if (align)
    {
        return true;
    }
    if (bioseq)
    {
        return true;
    }

    CIRef<IEditObject> editor = CreateEditorForObject(ConstRef(object), seh, false);
    if (editor)
        return true;
    return false;
}

END_SCOPE(valedit)

END_NCBI_SCOPE

