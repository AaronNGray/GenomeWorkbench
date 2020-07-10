/*  $Id: validate_frame.cpp 43609 2019-08-08 16:12:53Z filippov $
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
#include <objects/submit/Seq_submit.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/objutils/project_item_extra.hpp>
#include <gui/core/project_task.hpp>
#include <gui/objutils/objects.hpp>
#include <gui/widgets/edit/table_data_validate_job.hpp>
#include <gui/widgets/edit/table_data_validate_params.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/table_data_status.hpp>
#include <objmgr/util/sequence.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/objutils/execute_lock_guard.hpp>
#include <gui/objects/GBWorkspace.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/simple_project_view.hpp>
#include <gui/widgets/edit/bioseq_editor.hpp>
#include <gui/packages/pkg_sequence_edit/segregate_sets.hpp>
#include <gui/packages/pkg_sequence_edit/sequester_sets.hpp>
#include <gui/widgets/edit/smart_save_to_client.hpp>
#include <gui/widgets/edit/sequence_editing_cmds.hpp>
#include <objtools/validator/validerror_format.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/packages/pkg_sequence_edit/validate_panel.hpp>
#include <gui/packages/pkg_sequence_edit/validate_frame.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/pub/Pub.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
// for validator response commands
#include <gui/packages/pkg_sequence_edit/common_commands.hpp>
#include <gui/objutils/seqdesc_title_edit.hpp>
#include <gui/objutils/gap_edit.hpp>
#include <gui/objutils/primary_edit.hpp>

#include "validator_report_cfg_dlg.hpp"

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
#include <wx/filename.h>
#include <wx/hyperlink.h>
////@end includes


////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE


/*
 * CValidateFrame type definition
 */

IMPLEMENT_CLASS( CValidateFrame, wxFrame )


/*
 * CValidateFrame event table definition
 */

BEGIN_EVENT_TABLE( CValidateFrame, wxFrame )

////@begin CValidateFrame event table entries
    EVT_BUTTON(ID_VALIDATE_FRAME_REFRESH, CValidateFrame::OnRefresh)
    EVT_BUTTON(ID_VALIDATE_FRAME_DISMISS, CValidateFrame::OnDismiss)
    EVT_BUTTON( ID_VALIDATE_FRAME_SEQUESTER_BTN, CValidateFrame::OnSequester )
    EVT_BUTTON( ID_VALIDATE_FRAME_SEGREGATE_BTN, CValidateFrame::OnSegregate )
    EVT_BUTTON( ID_VALIDATE_FRAME_REPORT_BTN, CValidateFrame::OnReport )
    EVT_BUTTON( ID_VALIDATE_FRAME_DONE_BTN, CValidateFrame::OnDone )
    EVT_UPDATE_UI(ID_VALIDATE_FRAME_DONE_BTN, CValidateFrame::OnUpdateDoneBtn)
////@end CValidateFrame event table entries
END_EVENT_TABLE()

/*
 * CValidateFrame constructors
 */

CValidateFrame::CValidateFrame()
    : m_Workbench(NULL)
{
    Init();
}

CValidateFrame::CValidateFrame( wxWindow* parent, IWorkbench*  wb, CRef<CUser_object> params, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_Workbench(wb), m_params(params)
{
    Init();
    Create( parent, id, caption, pos, size, style );

    NEditingStats::ReportUsage(caption);
    SetRegistryPath("Dialogs.Edit.Validate");
    LoadSettings();  
}


/*
 * CValidateFrame creator
 */

bool CValidateFrame::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CValidateFrame creation
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();   
    Centre(wxBOTH|wxCENTRE_ON_SCREEN);
////@end CValidateFrame creation
    return true;
}


/*
 * CValidateFrame destructor
 */

CValidateFrame::~CValidateFrame()
{
////@begin CValidateFrame destruction
    SaveSettings();
////@end CValidateFrame destruction
   
    m_Instance = NULL;
}

static const char* kFrameWidth = "Frame Width";
static const char* kFrameHeight = "Frame Height";
static const char* kFramePosX = "Frame Position X";
static const char* kFramePosY = "Frame Position Y";

void CValidateFrame::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}

void CValidateFrame::SaveSettings() const
{
    if (m_RegPath.empty())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

    view.Set(kFrameWidth,GetScreenRect().GetWidth());
    view.Set(kFrameHeight,GetScreenRect().GetHeight());
    view.Set(kFramePosX,GetScreenPosition().x);
    view.Set(kFramePosY,GetScreenPosition().y);
}


void CValidateFrame::LoadSettings()
{
    if (m_RegPath.empty())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

    int width = view.GetInt(kFrameWidth, -1);
    int height = view.GetInt(kFrameHeight, -1);
    if (width >= 0  && height >= 0)
        SetSize(wxSize(width,height));
    else
        SetSize(SYMBOL_VALIDATE_FRAME_SIZE);

    int pos_x = view.GetInt(kFramePosX, -1);
    int pos_y = view.GetInt(kFramePosY, -1);

   if (pos_x >= 0  && pos_y >= 0)
   {
       int max_x = 0;
       for (size_t i = 0; i < wxDisplay::GetCount(); i++) // also see gui/widgets/wx/wx_utils.cpp:CorrectWindowRect() for alternative window position validation
       {
        wxDisplay display(i);
        max_x += display.GetGeometry().GetWidth();
       }
       if (pos_x + width > max_x) pos_x = wxGetDisplaySize().GetWidth()-width-5;
       if (pos_y + height > wxGetDisplaySize().GetHeight()) pos_y = wxGetDisplaySize().GetHeight()-height-5;
       
       SetPosition(wxPoint(pos_x,pos_y));
   }
}


/*
 * Member initialisation
 */

void CValidateFrame::Init()
{
////@begin CValidateFrame member initialisation
    m_Panel = NULL;
    m_DoneButton = NULL;
////@end CValidateFrame member initialisation
}


/*
 * Control creation for CValidateFrame
 */

void CValidateFrame::CreateControls()
{    
////@begin CValidateFrame content construction
    // Generated by DialogBlocks, 18/10/2016 11:02:51 (unregistered)

    CValidateFrame* itemFrame1 = this;

    wxPanel* itemPanel2 = new wxPanel(itemFrame1);
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    itemPanel2->SetSizer(itemBoxSizer1);


    m_Panel = new CValidatePanel( itemPanel2, m_Workbench, m_params);
    itemBoxSizer1->Add(m_Panel, 1, wxGROW|wxALL, 0);
    m_Panel->LoadData();

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL,0);

    if (RunningInsideNCBI())
    {
        wxButton* itemButton8 = new wxButton( itemPanel2, ID_VALIDATE_FRAME_REPORT_BTN, _("Report"), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer6->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

        
        if (x_AllowDoneButton())
        {
            m_DoneButton = new wxButton( itemPanel2, ID_VALIDATE_FRAME_DONE_BTN, _("Done"), wxDefaultPosition, wxDefaultSize, 0 );
            itemBoxSizer6->Add(m_DoneButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
        }
        
        wxButton* itemButton11 = new wxButton( itemPanel2, ID_VALIDATE_FRAME_SEQUESTER_BTN, _("Sequester"), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer6->Add(itemButton11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
        
        wxButton* itemButton12 = new wxButton( itemPanel2, ID_VALIDATE_FRAME_SEGREGATE_BTN, _("Segregate"), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer6->Add(itemButton12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    }

    
    wxButton* itemButton7 = new wxButton( itemPanel2, ID_VALIDATE_FRAME_REFRESH, _("Refresh"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton10 = new wxButton( itemPanel2, ID_VALIDATE_FRAME_DISMISS, _("Dismiss"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxHyperlinkCtrl* itemHyperlinkCtrl = new wxHyperlinkCtrl( itemPanel2, wxID_HELP, _("Help"), wxT("https://www.ncbi.nlm.nih.gov/tools/gbench/manual8/#validator"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemHyperlinkCtrl->SetForegroundColour(wxColour(192, 192, 192));
    itemBoxSizer6->Add(itemHyperlinkCtrl, 0, wxALIGN_CENTER_VERTICAL, 5);

////@end CValidateFrame content construction
}

/*
 * Should we show tooltips?
 */

bool CValidateFrame::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CValidateFrame::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CValidateFrame bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CValidateFrame bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CValidateFrame::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CValidateFrame icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CValidateFrame icon retrieval
}

void CValidateFrame::OnDismiss(wxCommandEvent& event )
{
    Close();
}

void CValidateFrame::OnSequester( wxCommandEvent& event )
{  
    if (!m_Workbench)
        return;
    set<CBioseq_Handle> selected;

    m_Panel->GetSelected(selected);
    CConstRef<objects::CSeq_submit> seq_submit;
    CSeq_entry_Handle top_seq_entry;

    m_Panel->GetTopSeqAndSeqSubmit(top_seq_entry,seq_submit);
    if (!top_seq_entry)
        return;

    CSequesterSets * dlg = new CSequesterSets(this, top_seq_entry, seq_submit, m_Workbench);

    dlg->SetSubsets(selected);
    dlg->Show(true);      
}


void CValidateFrame::OnSegregate( wxCommandEvent& event )
{
    if (!m_Workbench)
        return;
    set<CBioseq_Handle> selected;
    m_Panel->GetSelected(selected);
    CConstRef<objects::CSeq_submit> seq_submit;
    CSeq_entry_Handle top_seq_entry;
    m_Panel->GetTopSeqAndSeqSubmit(top_seq_entry,seq_submit);
    if (!top_seq_entry)
        return;
    ICommandProccessor* cmdProcessor = NULL;
    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    if (srv)
    {
        CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
        if (!ws) return;

        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(top_seq_entry.GetScope()));
        if (doc)
            cmdProcessor = &doc->GetUndoManager(); 
    }
    if (!cmdProcessor)
        return;

    CSegregateSets* dlg = new CSegregateSets(this, top_seq_entry, cmdProcessor);
    dlg->SetSubsets(selected);
    dlg->Show(true);    
}

void CValidateFrame::OnReport( wxCommandEvent& event )
{
    CRef<CObjectFor<objects::CValidError::TErrs> > errs = m_Panel->GetErrors();
    if (!errs)
        return;

    CConstRef<objects::CSeq_submit> seq_submit;
    CSeq_entry_Handle top_seq_entry;
    m_Panel->GetTopSeqAndSeqSubmit(top_seq_entry,seq_submit);
    if (!top_seq_entry)
        return;
    TConstScopedObjects objects = m_Panel->GetObjects();
    CValidatorReportCfgDlg* report = new CValidatorReportCfgDlg(this, errs, objects.front().scope, top_seq_entry, seq_submit, m_Workbench);    
    report->SetWorkDir(m_Panel->GetWorkDir());
    report->Show(true);
}

void CValidateFrame::OnUpdateDoneBtn(wxUpdateUIEvent& event)
{    
    if (!m_DoneButton)
        return;
    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView("Dialogs.Edit.SequesterSets");
    bool enable = true;
    if (view.HasField("EnableDone"))
        enable = view.GetBool("EnableDone", true);
    
    event.Enable(enable);
}

void CValidateFrame::OnDone( wxCommandEvent& event )
{
    if (!m_DoneButton)
        return;
    CProjectService* projectService = m_Workbench->GetServiceByType<CProjectService>();
    if (!projectService) return;

    CRef<CGBWorkspace> ws = projectService->GetGBWorkspace();
    if (!ws) return;

    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*m_Panel->GetScope()));
    if (!doc) return;

    string errMsg;
    try {
        CProjectFolder& data_folder = doc->SetData();
        if (data_folder.CanGetItems()) {
            ITERATE(CProjectFolder::TItems, bit, data_folder.GetItems()) {
                int pitemId = (*bit)->GetId();
                CProjectItem* item = data_folder.FindProjectItemById(pitemId);
                if (item) {
                    string pipe;
                    CProjectItemExtra::GetStr(*item, "SmartPipe", pipe);
                    if (!pipe.empty()) {
                        // Smart in named piped communications mode
						CSmartClient::SendResult(doc->GetScope(), *item, CSmartClient::eUpdate);
					}
                    else {
                        // Smart in file mode
                        doc->Save(doc->GetFileName());
                    }
                }
            }
        }
    } catch (const CException& ex) {
        errMsg = ex.GetMsg();
    } catch (const exception& ex) {
        errMsg = ex.what();
    }
    if (!errMsg.empty()) {
        wxMessageBox(errMsg, wxT("Smart client communication error"), wxOK|wxICON_ERROR);
        return;
    }

    projectService->RemoveProject(*doc);

    LoadData();
    Refresh();
}


CValidateFrame *CValidateFrame::m_Instance = NULL;

void CValidateFrame::GetInstance( wxWindow* parent, IWorkbench* workbench, CRef<objects::CUser_object> params, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    if (!m_Instance)
    {
        m_Instance = new CValidateFrame(parent, workbench, params, id, caption, pos, size, style); 
        m_Instance->Show(true);    
        m_Instance->Raise();
        m_Instance->SetFocus();
    }
    else
    {
        m_Instance->Raise();
        m_Instance->SetFocus();     
        m_Instance->LoadData();
        m_Instance->Refresh();
    }
}

bool CValidateFrame::x_AllowDoneButton()
{  
    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(kInternalIndexerRegistry);

    int indexer = view.GetInt(kInternalIndexerRegistryType, e_GenBankIndexer);

    if (indexer == e_GenBankIndexer)
        return true;

    return false;
}

void CValidateFrame::LoadData()
{
    m_Panel->LoadData();
}

void CValidateFrame::OnRefresh(wxCommandEvent& event )
{   
    LoadData();
    Refresh();
}

END_NCBI_SCOPE

