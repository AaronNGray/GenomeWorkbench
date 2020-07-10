/*  $Id: featuretbl_frame.cpp 43605 2019-08-07 19:40:03Z filippov $
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

#include <objtools/format/flat_file_config.hpp>
#include <objtools/format/flat_file_generator.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/core/selection_service_impl.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>  
#include <gui/packages/pkg_sequence_edit/featuretbl_frame.hpp>

#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/filedlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CFeatureTblFr type definition
 */

IMPLEMENT_CLASS( CFeatureTblFr, wxFrame )


/*!
 * CFeatureTblFr event table definition
 */

BEGIN_EVENT_TABLE( CFeatureTblFr, wxFrame )

    EVT_CLOSE(CFeatureTblFr::OnCloseWindow)
    EVT_BUTTON(ID_FTBL_REFRESHBTN, CFeatureTblFr::OnRefreshClick)

END_EVENT_TABLE()


/*!
 * CFeatureTblFr constructors
 */

CFeatureTblFr::CFeatureTblFr()
    : m_Workbench(0)
{
    Init();
}

CFeatureTblFr::CFeatureTblFr(wxWindow* parent, IWorkbench* workbench, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
    : m_Workbench(workbench)
{
    Init();
    Create( parent, id, caption, pos, size, style );
    NEditingStats::ReportUsage(caption);
}


/*!
 * CFeatureTblFr creator
 */

bool CFeatureTblFr::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CFeatureTblFr creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxFrame::Create(parent, id, caption, pos, size, style);
    wxVisualAttributes attr = wxPanel::GetClassDefaultAttributes();
    if (attr.colBg != wxNullColour)
        SetOwnBackgroundColour(attr.colBg);

    CreateControls();
    Centre();
////@end CFeatureTblFr creation
    return true;
}


/*!
 * CFeatureTblFr destructor
 */

CFeatureTblFr::~CFeatureTblFr()
{
    wxWindow::TransferDataFromWindow();
    SaveSettings();
    // job is canceled if main window is destroyed
    if (m_JobAdapter) {
        m_JobAdapter->Cancel();
    }
}


/*!
 * Member initialisation
 */

void CFeatureTblFr::Init()
{
////@begin CFeatureTblFr member initialisation
    m_ProteinId = NULL;
    m_Source = NULL;
    m_ProteinFeats = NULL;
    m_GenericPanel = NULL;
    m_FindMode = 1;
    SetRegistryPath("5ColFeatureTable");
    LoadSettings();
////@end CFeatureTblFr member initialisation
}


/*!
 * Control creation for CFeatureTblFr
 */

void CFeatureTblFr::CreateControls()
{    
////@begin CFeatureTblFr content construction
    CFeatureTblFr* itemFrame1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemFrame1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemFrame1, wxID_STATIC, _("Show"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProteinId = new wxCheckBox( itemFrame1, ID_FTBL_CHCKBX1, _("Protein/Transcript Id"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_ProteinId, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Source = new wxCheckBox( itemFrame1, ID_FTBL_CHCKBX2, _("Source Descriptor/Feature"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_Source, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProteinFeats = new wxCheckBox(itemFrame1, ID_FTBL_CHCKBX2, _("Protein Features"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer3->Add(m_ProteinFeats, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxButton* itemButton1 = new wxButton( itemFrame1, ID_FTBL_REFRESHBTN, _("Refresh"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton1, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_GenericPanel = new CGenericPanel( itemFrame1, m_Workbench, ID_FTBL_GENPANEL, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(m_GenericPanel, 1, wxGROW|wxALL, 5);    
    m_GenericPanel->SetFindMode(m_FindMode);
    m_GenericPanel->SetHelpUrl(_("https://www.ncbi.nlm.nih.gov/tools/gbench/manual8/#show-feature-table"));

    m_ProteinId->SetValidator(wxGenericValidator( &m_ShowTranscriptId ));
    m_Source->SetValidator(wxGenericValidator( &m_ShowSources ));
    m_ProteinFeats->SetValidator(wxGenericValidator(&m_ShowProteinFeats));

    wxWindow::InitDialog();
    x_RefreshTable();
////@end CFeatureTblFr content construction
}

void CFeatureTblFr::SetWorkDir(const wxString& workDir)
{
    if (m_GenericPanel)
        m_GenericPanel->SetWorkDir(workDir);
}

/*!
 * Should we show tooltips?
 */

bool CFeatureTblFr::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CFeatureTblFr::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CFeatureTblFr bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CFeatureTblFr bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CFeatureTblFr::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CFeatureTblFr icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFeatureTblFr icon retrieval
}

static const char* kShowSources = "Show Sources";
static const char* kShowTranscriptID = "Show Transcript ID";
static const char* kShowProteinFeats = "Show Protein Features";
static const char* kCaseSensitive = "Search mode";

void CFeatureTblFr::SetRegistryPath(const string& path)
{
    m_RegPath = path;
}

void CFeatureTblFr::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kShowSources, m_ShowSources);
        view.Set(kShowTranscriptID, m_ShowTranscriptId);
        view.Set(kShowProteinFeats, m_ShowProteinFeats);
        if (m_GenericPanel)
            view.Set(kCaseSensitive, m_GenericPanel->GetFindMode());
    }
}


void CFeatureTblFr::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();

        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);
        m_ShowSources = view.GetBool(kShowSources, m_ShowSources);
        m_ShowTranscriptId = view.GetBool(kShowTranscriptID, m_ShowTranscriptId);
        m_ShowProteinFeats = view.GetBool(kShowProteinFeats, m_ShowProteinFeats);
        m_FindMode = view.GetInt(kCaseSensitive, 1);
     
    }
}

void CFeatureTblFr::OnRefreshClick(wxCommandEvent& event)
{
    x_RefreshTable();
}

void CFeatureTblFr::OnCloseWindow(wxCloseEvent& event)
{
    Destroy();
}


namespace {
    struct SFeatTableParams {
        TConstScopedObjects objects;
        string filename;

        bool showProteinId;
        bool showSrcFeature;
        bool showProteinFeats;

        SFeatTableParams(bool protein_id, bool src_feat, bool show_prot_feats)
            : filename(kEmptyStr),
            showProteinId(protein_id),
            showSrcFeature(src_feat),
            showProteinFeats(show_prot_feats) {}
    };
}


static bool s_Create5ColFeatTable(SFeatTableParams& input, string& output, string& error, ICanceled& canceled)
{
    CBioseq_Handle bsh;
    TConstScopedObjects& objects = input.objects;

    _ASSERT(!objects.empty());
    CSeq_entry_Handle seh = GetTopSeqEntryFromScopedObject(objects.front());
    CRef<CScope> scope = objects.front().scope;
    
    if (canceled.IsCanceled())
        return false;

    // check whether multiple features have been selected
    vector<CSeq_feat_Handle> handles;
    for (TConstScopedObjects::const_iterator it = objects.begin(); it != objects.end(); ++it) {
        const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(it->object.GetPointer());
        if (feat) {
            CSeq_feat_Handle fh = scope->GetSeq_featHandle(*feat);
            handles.push_back(fh);
        }
    }
    
    bool only_na = FocusedOnNucleotides(objects, seh);
    bool all = FocusedOnAll(objects, seh);
    if (handles.empty() && !only_na && !all) {
        NON_CONST_ITERATE(TConstScopedObjects, it, objects) {
            if (canceled.IsCanceled())
                return false;

            const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>(it->object.GetPointer());
            if (loc) {
                bsh = scope->GetBioseqHandle(*loc);
                if (bsh) {
                    break;
                }
            }
        }
    }

    if (canceled.IsCanceled())
        return false;

    LOG_POST(Info << "Generate feature table... ");
    try {
        CFlatFileConfig ff_config;
        ff_config.SetFormatFTable();

        ff_config.SetShowContigFeatures().SetShowContigSources();
        ff_config.SetShowPeptides();
        ff_config.SetShowFtablePeptides(input.showProteinFeats);
        if (only_na) {
            ff_config.SetViewNuc();
        } else {
            ff_config.SetViewAll();
        }

        if (!input.showProteinId) {
            ff_config.SetHideProteinID();
        }
        if (!input.showSrcFeature) {
            ff_config.SetHideSourceFeatures();
        }

        // default: show protein ids, transcript_ids, no source features
        CFlatFileGenerator ff(ff_config);
        ff.SetAnnotSelector() = CSeqUtils::GetAnnotSelector();
        
        
        if (!handles.empty()) {
            for (vector<CSeq_feat_Handle>::const_iterator it = handles.begin(); it != handles.end(); ++it) {
                CMappedFeat mapped_feat(*it);
                output.append(ff.GetSeqFeatText(mapped_feat, *scope, ff_config));
            }
            
        } else {
            CNcbiOstrstream ostr;
            if (bsh) {
                ff.Generate(bsh, ostr);
            }
            else {
                ff.Generate(seh, ostr);
            }
            output = CNcbiOstrstreamToString(ostr);
        }
        

        if (!input.filename.empty()) {
            CNcbiOfstream ofstr(input.filename.c_str());
            if (ofstr.good()) {
                ofstr << output;
            }
            else {
                LOG_POST("Unable to open file for writing: " << input.filename);
            }
        }
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


void CFeatureTblFr::OnJobResult(CObject* result, CJobAdapter&)
{
    CJobAdapterResult<string>* jobResult = dynamic_cast<CJobAdapterResult<string>*>(result);
    if (jobResult) {
        const string& data = jobResult->GetData();
        m_GenericPanel->SetText(ToWxString(data));
        LOG_POST(Info << "Finished generating feature table.");
    }
}

void CFeatureTblFr::OnJobFailed(const string& errMsg, CJobAdapter&)
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

void CFeatureTblFr::x_RefreshTable(const string& path)
{
    if (!m_Workbench) return;

    m_GenericPanel->SetText(ToWxString("Loading..."));
    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (sel_srv){
        sel_srv->GetActiveObjects(objects);
    }
    if (objects.empty()) {
        GetViewObjects(m_Workbench, objects);
    }
    

    if (objects.empty()) {
        NcbiWarningBox("No objects selected!");
        return;
    }

    if (m_JobAdapter) m_JobAdapter->Cancel();

    
    SFeatTableParams input(m_ProteinId->IsChecked(), m_Source->IsChecked(), m_ProteinFeats->IsChecked());
    input.objects = objects;
    input.filename = path;

    m_JobAdapter.Reset(LaunchAdapterJob<SFeatTableParams, string>
        (this, input, s_Create5ColFeatTable, "C5ColFeatTableJob", "Generate 5 Column tab delimited feature table"));
}


END_NCBI_SCOPE
