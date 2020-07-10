/*  $Id: validator_report_cfg_dlg.cpp 41057 2018-05-16 16:03:20Z katargir $
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
 * Authors:  Colleen Bollin
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <objects/valerr/ValidError.hpp>
#include <objects/valerr/ValidErrItem.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include "validator_report_cfg_dlg.hpp"
#include <gui/widgets/edit/generic_report_dlg.hpp>  
#include <objtools/validator/validerror_format.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/document.hpp>
#include <gui/framework/workbench.hpp>
#include <gui/packages/pkg_sequence_edit/segregate_sets.hpp>
#include <gui/packages/pkg_sequence_edit/sequester_sets.hpp>

#include <wx/sizer.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CValidatorReportCfgDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CValidatorReportCfgDlg, wxDialog )


/*!
 * CValidatorReportCfgDlg event table definition
 */

BEGIN_EVENT_TABLE( CValidatorReportCfgDlg, wxDialog )

////@begin CValidatorReportCfgDlg event table entries
    EVT_BUTTON( ID_REPORT_BTN, CValidatorReportCfgDlg::OnReportBtnClick )

    EVT_BUTTON( ID_VALID_REPORT_DISMISS, CValidatorReportCfgDlg::OnValidReportDismissClick )

    EVT_BUTTON( ID_VALID_REPORT_SEQUESTER_BTN, CValidatorReportCfgDlg::OnSequester )
    EVT_BUTTON( ID_VALID_REPORT_SEGREGATE_BTN, CValidatorReportCfgDlg::OnSegregate )

////@end CValidatorReportCfgDlg event table entries

END_EVENT_TABLE()


/*!
 * CValidatorReportCfgDlg constructors
 */

CValidatorReportCfgDlg::CValidatorReportCfgDlg()
    : m_Errs(), m_Scope()
{
    Init();
}

CValidatorReportCfgDlg::CValidatorReportCfgDlg( wxWindow* parent, 
                                                CObjectFor<objects::CValidError::TErrs>* errs, objects::CScope* scope,
                                                objects::CSeq_entry_Handle top_seq_entry, CConstRef<objects::CSeq_submit> seq_submit, IWorkbench* workbench,
                                                wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_Errs(errs), m_Scope(scope), m_TopSeqEntry(top_seq_entry), m_SeqSubmit(seq_submit), m_Workbench(workbench)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CValidatorReportCfgDlg creator
 */

bool CValidatorReportCfgDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CValidatorReportCfgDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CValidatorReportCfgDlg creation
    return true;
}


/*!
 * CValidatorReportCfgDlg destructor
 */

CValidatorReportCfgDlg::~CValidatorReportCfgDlg()
{
////@begin CValidatorReportCfgDlg destruction
////@end CValidatorReportCfgDlg destruction
}


/*!
 * Member initialisation
 */

void CValidatorReportCfgDlg::Init()
{
////@begin CValidatorReportCfgDlg member initialisation
////@end CValidatorReportCfgDlg member initialisation
    m_Verbose = NULL;
}


/*!
 * Control creation for CValidatorReportCfgDlg
 */

void CValidatorReportCfgDlg::CreateControls()
{    
////@begin CValidatorReportCfgDlg content construction
    CValidatorReportCfgDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Verbose = new wxCheckBox(itemDialog1, wxID_ANY, _("Verbose"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(m_Verbose, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, ID_REPORT_BTN, _("Report"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemDialog1, ID_VALID_REPORT_DISMISS, _("Dismiss"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    if (RunningInsideNCBI())
    {
        wxButton* itemButton11 = new wxButton( itemDialog1, ID_VALID_REPORT_SEQUESTER_BTN, _("Sequester"), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer4->Add(itemButton11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
        
        wxButton* itemButton12 = new wxButton( itemDialog1, ID_VALID_REPORT_SEGREGATE_BTN, _("Segregate"), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer4->Add(itemButton12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    }

////@end CValidatorReportCfgDlg content construction
    m_Opts.clear();
    if (m_Errs) {
        vector<string> errcodes;

        for (size_t row = 0; row < m_Errs->GetData().size(); row++) {
            CConstRef<CValidErrItem> pValidErrItem(m_Errs->GetData()[row]);
            errcodes.push_back(pValidErrItem->GetErrCode());
        }
        sort(errcodes.begin(), errcodes.end());
        vector<string>::iterator final = unique(errcodes.begin(), errcodes.end());
        errcodes.resize( std::distance(errcodes.begin(), final) );

        ITERATE(vector<string>, it, errcodes) {
            wxCheckBox* box = new wxCheckBox( itemDialog1, wxID_ANY, ToWxString(*it), wxDefaultPosition, wxDefaultSize, 0 );
            itemFlexGridSizer3->Add(box, 0, wxALIGN_LEFT|wxALL, 5);
            m_Opts.push_back(box);
        }
    }
}


/*!
 * Should we show tooltips?
 */

bool CValidatorReportCfgDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CValidatorReportCfgDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CValidatorReportCfgDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CValidatorReportCfgDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CValidatorReportCfgDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CValidatorReportCfgDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CValidatorReportCfgDlg icon retrieval
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_VALID_REPORT_DISMISS
 */

void CValidatorReportCfgDlg::OnValidReportDismissClick( wxCommandEvent& event )
{
    Close();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_REPORT_BTN
 */

void CValidatorReportCfgDlg::OnReportBtnClick( wxCommandEvent& event )
{
    CGenericReportDlg* report = new CGenericReportDlg(NULL);
    report->SetTitle(wxT("Validator Report"));
    report->SetWorkDir(m_WorkDir);
    bool any = false;
    ITERATE(vector<wxCheckBox *>, it, m_Opts) {
        if ((*it)->GetValue()) {
            any = true;
            break;
        }
    }
    string msg = "";
    if (m_Errs && m_Scope) {
        objects::CObjectManager& objmgr = m_Scope->GetObjectManager();
        objects::validator::CValidErrorFormat formatter(objmgr);
        ITERATE(vector<wxCheckBox *>, it, m_Opts) {
            if ((*it)->GetValue() || !any) {
                vector<CConstRef<CValidErrItem> > err_list;
                for (size_t row = 0; row < m_Errs->GetData().size(); row++) {
                    CConstRef<CValidErrItem> pValidErrItem(m_Errs->GetData()[row]);
                    if (NStr::EqualNocase(ToStdString((*it)->GetLabel()), pValidErrItem->GetErrCode())) {
                        err_list.push_back(pValidErrItem);
                    }
                }

                if (err_list.size() > 0) {
                    msg += err_list[0]->GetErrCode();
                    if (m_Verbose->GetValue()) {
                        msg += "\t" + err_list[0]->GetVerbose();
                    }
                    msg += "\n";
                }
                ITERATE(vector<CConstRef<objects::CValidErrItem> >, err, err_list) {
                    msg += "\t";
                    msg += formatter.FormatForSubmitterReport(**err, *m_Scope);
                    msg += "\n";
                }
            }
        }
    }
    report->SetText(ToWxString(msg));
    report->Show(true);
}

void CValidatorReportCfgDlg::GetSelected(set<CBioseq_Handle> &selected)
{
   if (m_Errs && m_Scope) {
        objects::CObjectManager& objmgr = m_Scope->GetObjectManager();
        objects::validator::CValidErrorFormat formatter(objmgr);
        ITERATE(vector<wxCheckBox *>, it, m_Opts) {
            if ((*it)->GetValue()) {
                vector<CConstRef<CValidErrItem> > err_list;
                for (size_t row = 0; row < m_Errs->GetData().size(); row++) {
                    CConstRef<CValidErrItem> pValidErrItem(m_Errs->GetData()[row]);
                    if (NStr::EqualNocase(ToStdString((*it)->GetLabel()), pValidErrItem->GetErrCode())) {
                        string accession = pValidErrItem->GetAccession();
                        CSeq_id acc_id(accession, CSeq_id::fParse_Default);
                        CBioseq_Handle bsh;
                        try
                        {
                            bsh = m_Scope->GetBioseqHandle(acc_id);
                        }
                        catch(exception &)
                        {}
                        if (bsh)
                            selected.insert(bsh);                        
                    }
                }
            }
        }
   }
}

void CValidatorReportCfgDlg::OnSequester( wxCommandEvent& event )
{
    if (!m_TopSeqEntry)
        return;
    set<CBioseq_Handle> selected;

    GetSelected(selected);
   
    CSequesterSets * dlg = new CSequesterSets(GetParent(), m_TopSeqEntry, m_SeqSubmit, m_Workbench);

    dlg->SetSubsets(selected);
    dlg->Show(true);      
}

void CValidatorReportCfgDlg::OnSegregate( wxCommandEvent& event )
{
    if (!m_TopSeqEntry)
        return;
    if (!m_Workbench)
        return;
    set<CBioseq_Handle> selected;
    GetSelected(selected);

    ICommandProccessor* cmdProcessor = NULL;
    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    if (srv)
    {
        CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
        if (!ws) return;

        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(m_TopSeqEntry.GetScope()));
        if (doc)
            cmdProcessor = &doc->GetUndoManager(); 
    }
    if (!cmdProcessor)
        return;
    wxWindow *main_window = GetParent();
    CSegregateSets* dlg = new CSegregateSets(main_window, m_TopSeqEntry, cmdProcessor);
    dlg->SetSubsets(selected);
    dlg->Show(true);    
}

END_NCBI_SCOPE
