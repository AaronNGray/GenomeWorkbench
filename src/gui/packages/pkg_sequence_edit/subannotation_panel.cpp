/*  $Id: subannotation_panel.cpp 43170 2019-05-23 16:41:07Z filippov $
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
#include <gui/widgets/seq/text_panel.hpp>
#include <gui/objutils/registry.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <objtools/format/flat_file_config.hpp>
#include <objtools/format/flat_file_generator.hpp>
#include <gui/packages/pkg_sequence_edit/import_feat_table.hpp>
#include <gui/packages/pkg_sequence_edit/subannotation_panel.hpp>


#include <wx/sizer.h>
#include <wx/cshelp.h>
#include <wx/button.h>
#include <wx/icon.h>


BEGIN_NCBI_SCOPE

/*
 * CSubAnnotationPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSubAnnotationPanel, wxPanel )


/*
 * CSubAnnotationPanel event table definition
 */

BEGIN_EVENT_TABLE( CSubAnnotationPanel, wxPanel )

////@begin CSubAnnotationPanel event table entries
    EVT_BUTTON( ID_IMPORT_FTABLE_BTN, CSubAnnotationPanel::OnImportFeatTableClick )
    EVT_BUTTON( wxID_CONTEXT_HELP, CSubAnnotationPanel::OnContextHelpClick )
////@end CSubAnnotationPanel event table entries

END_EVENT_TABLE()


/*
 * CSubAnnotationPanel constructors
 */

CSubAnnotationPanel::CSubAnnotationPanel()
{
    Init();
}

CSubAnnotationPanel::CSubAnnotationPanel( wxWindow* parent, ICommandProccessor* proc, const wxString &dir, objects::CSeq_entry_Handle seh, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    :  m_CmdProcessor(proc), m_WorkDir(dir), m_Seh(seh)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CSubAnnotationPanel creator
 */

bool CSubAnnotationPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSubAnnotationPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSubAnnotationPanel creation
    return true;
}


/*
 * CSubAnnotationPanel destructor
 */

CSubAnnotationPanel::~CSubAnnotationPanel()
{
////@begin CSubAnnotationPanel destruction
////@end CSubAnnotationPanel destruction
}


/*
 * Member initialisation
 */

void CSubAnnotationPanel::Init()
{
////@begin CSubAnnotationPanel member initialisation
    m_FeatureTable = NULL;
////@end CSubAnnotationPanel member initialisation
}


/*
 * Control creation for CSubAnnotationPanel
 */

void CSubAnnotationPanel::CreateControls()
{    
////@begin CSubAnnotationPanel content construction
    CSubAnnotationPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer1, 0, wxALIGN_LEFT|wxALL, 0);

    wxButton* itemButton2 = new wxButton( itemPanel1, ID_IMPORT_FTABLE_BTN, _("Import from feature table"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer1->Add(itemButton2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    //wxContextHelpButton* itemContextHelpButton3 = new wxContextHelpButton( itemPanel1, wxID_CONTEXT_HELP, wxDefaultPosition, wxSize(20, -1), wxBU_AUTODRAW );
    //itemBoxSizer1->Add(itemContextHelpButton3, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    m_FeatureTable = new CGenericPanel(itemPanel1, true, NULL, ID_SUBANNOTFEATTBL, wxDefaultPosition, wxSize(100, 100), 0);
    itemBoxSizer2->Add(m_FeatureTable, 1, wxGROW | wxALL, 5);
    m_FeatureTable->SetFindMode(1);

////@end CSubAnnotationPanel content construction
}

void CSubAnnotationPanel::x_SetFontAsTextPanel()
{
    wxFont font = CTextPanel::GetFont(CGuiRegistry::GetInstance().GetInt("GBENCH.TextView.FontSize", 1));
    m_FeatureTable->SetInitialSize(wxSize(100 * font.GetPointSize() / 10, 100));
    m_FeatureTable->SetFont(font);
}


/*
 * Should we show tooltips?
 */

bool CSubAnnotationPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CSubAnnotationPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSubAnnotationPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSubAnnotationPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CSubAnnotationPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSubAnnotationPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSubAnnotationPanel icon retrieval
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CONTEXT_HELP
 */

void CSubAnnotationPanel::OnContextHelpClick( wxCommandEvent& event )
{

}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_ORGINFOBTN
 */

void CSubAnnotationPanel::OnImportFeatTableClick( wxCommandEvent& event )
{
    if (!m_Seh || !m_CmdProcessor) return;

    CImportFeatTable worker(m_Seh);
    CIRef<IEditCommand> cmd = worker.ImportFeaturesFromFile(m_WorkDir);
    if (!cmd)
        return;
    
    m_CmdProcessor->Execute(cmd);
}


bool CSubAnnotationPanel::TransferDataToWindow()
{
    m_FeatureTable->SetText(ToWxString("Loading..."));

    if (m_JobAdapter) m_JobAdapter->Cancel();

    // show protein IDs, do not show sources
    SFeatTableParams input(true, false);
    input.seh = m_Seh;

    m_JobAdapter.Reset(LaunchAdapterJob<SFeatTableParams, string>
        (this, input, Create5ColFeatTable, "C5ColFeatTableJob", "Generate 5 Column tab delimited feature table"));
    return true;
}


bool CSubAnnotationPanel::Create5ColFeatTable(SFeatTableParams& input, string& output, string& error, ICanceled& canceled)
{
    if (canceled.IsCanceled())
        return false;

    if (canceled.IsCanceled())
        return false;

    LOG_POST(Info << "Generate feature table... ");
    try {
        objects::CFlatFileConfig ff_config;
        ff_config.SetFormatFTable();

        ff_config.SetShowContigFeatures().SetShowContigSources();
        ff_config.SetShowPeptides();
        ff_config.SetViewNuc();

        if (!input.showProteinId) {
            ff_config.SetHideProteinID();
        }
        if (!input.showSrcFeature) {
            ff_config.SetHideSourceFeatures();
        }

        // default: show protein ids, transcript_ids, no source features
        objects::CFlatFileGenerator ff(ff_config);
        ff.SetAnnotSelector() = CSeqUtils::GetAnnotSelector();

        CNcbiOstrstream ostr;
        ff.Generate(input.seh, ostr);
        output = CNcbiOstrstreamToString(ostr);

    }
    catch (const CException& e) {
        error = e.GetMsg();
        LOG_POST(Error << e.GetMsg());
        return false;
    }
    catch (const std::exception& e) {
        error = e.what();
        LOG_POST(Error << e.what());
        return false;
    }

    return true;
}


void CSubAnnotationPanel::OnJobResult(CObject* result, CJobAdapter&)
{
    CJobAdapterResult<string>* jobResult = dynamic_cast<CJobAdapterResult<string>*>(result);
    if (jobResult) {
        const string& data = jobResult->GetData();
        m_FeatureTable->SetText(ToWxString(data));
        LOG_POST(Info << "Finished generating feature table.");
    }
}

void CSubAnnotationPanel::OnJobFailed(const string& errMsg, CJobAdapter&)
{
    string err_msg = "Failed: ";
    if (!errMsg.empty()) {
        err_msg += errMsg;
    }
    else {
        err_msg += "Unknown fatal error";
    }

    NcbiErrorBox(err_msg);
}


END_NCBI_SCOPE
