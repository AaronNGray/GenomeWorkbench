/*  $Id: bulk_edit_feat_dlg_std.cpp 45101 2020-05-29 20:53:24Z asztalos $
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
#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/util/feature.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/macro_fn_feature.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>

#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>
#include <gui/packages/pkg_sequence_edit/constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_edit_feat_dlg_std.hpp>

#include <wx/stattext.h>
#include <wx/msgdlg.h>
#include <wx/display.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CBulkEditFeatDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CBulkEditFeatDlg, CBulkCmdDlg )


/*!
 * CBulkEditFeatDlg event table definition
 */

BEGIN_EVENT_TABLE( CBulkEditFeatDlg, CBulkCmdDlg )

////@begin CBulkEditFeatDlg event table entries

    EVT_UPDATE_FEATURE_LIST(wxID_ANY, CBulkEditFeatDlg::ProcessUpdateFeatEvent )

////@end CBulkEditFeatDlg event table entries

END_EVENT_TABLE()


/*!
 * CBulkEditFeatDlg constructors
 */

CBulkEditFeatDlg::CBulkEditFeatDlg()
{
    Init();
}

CBulkEditFeatDlg::CBulkEditFeatDlg( wxWindow* parent, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, wb, id, caption, pos, size, style);
    SetRegistryPath("Dialogs.Edit.BulkEditFeature");
    LoadSettings();
}


/*!
 * CBulkEditFeatDlg creator
 */

bool CBulkEditFeatDlg::Create( wxWindow* parent, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CBulkEditFeatDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, wb, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CBulkEditFeatDlg creation
    return true;
}


/*!
 * CBulkEditFeatDlg destructor
 */

CBulkEditFeatDlg::~CBulkEditFeatDlg()
{
////@begin CBulkEditFeatDlg destruction
    SaveSettings();
////@end CBulkEditFeatDlg destruction
}

static const char* kFrameWidth = "Frame Width";
static const char* kFrameHeight = "Frame Height";
static const char* kFramePosX = "Frame Position X";
static const char* kFramePosY = "Frame Position Y";

void CBulkEditFeatDlg::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}

void CBulkEditFeatDlg::SaveSettings() const
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


void CBulkEditFeatDlg::LoadSettings()
{
    if (m_RegPath.empty())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

    int width = view.GetInt(kFrameWidth, -1);
    int height = view.GetInt(kFrameHeight, -1);
    if (width >= 0  && height >= 0)
        SetSize(wxSize(width,height));

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

/*!
 * Member initialisation
 */

void CBulkEditFeatDlg::Init()
{
////@begin CBulkEditFeatDlg member initialisation
    m_FeatureType = NULL;
    m_Constraint = NULL;
    m_OkCancel = NULL;
////@end CBulkEditFeatDlg member initialisation
    m_ErrorMessage = "";
}


/*!
 * Control creation for CBulkEditFeatDlg
 */

void CBulkEditFeatDlg::CreateControls()
{    
////@begin CBulkEditFeatDlg content construction
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    SetSizer(itemBoxSizer1);

    wxPanel* itemCBulkCmdDlg1 = new wxPanel(this, wxID_ANY);
    itemBoxSizer1->Add(itemCBulkCmdDlg1, 1, wxGROW, 0);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);

    m_FeatureType = new CFeatureTypePanel( itemCBulkCmdDlg1, ID_EFL_FEATURETYPE, wxDefaultPosition, wxSize(100, 100), wxSIMPLE_BORDER );
    itemBoxSizer2->Add(m_FeatureType, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    GetTopLevelSeqEntryAndProcessor();
    if (!m_TopSeqEntry)
    {
        NCBI_THROW( CException, eUnknown, "No Top Seq-entry found" );
    }
    m_FeatureType->ListPresentFeaturesFirst(m_TopSeqEntry); 
    m_FeatureType->ListAnyOption();

    m_Notebook = new wxNotebook(itemCBulkCmdDlg1, wxID_ANY,wxDefaultPosition,wxDefaultSize);
    itemBoxSizer2->Add(m_Notebook, 1, wxGROW|wxALL, 5);
    
    CEvidencePanel *panel1 = new CEvidencePanel(m_Notebook);    
    m_Notebook->AddPage(panel1,_("Evidence"));

    CBlkEdtFeatLocationPanel *panel2 = new CBlkEdtFeatLocationPanel( m_Notebook);
    m_Notebook->AddPage(panel2,_("Location"));

    CStrandPanel *panel3 = new CStrandPanel( m_Notebook);
    m_Notebook->AddPage(panel3,_("Strand"));

    CRevIntPanel *panel4 = new CRevIntPanel( m_Notebook);
    m_Notebook->AddPage(panel4,_("Reverse Location Interval Order"));

    CResyncPanel *panel5 = new CResyncPanel( m_Notebook);
    m_Notebook->AddPage(panel5,_("Resynchronize Partials"));

    CPseudoPanel *panel6 = new CPseudoPanel( m_Notebook);
    m_Notebook->AddPage(panel6,_("Pseudo"));

    CExceptionPanel *panel7 = new CExceptionPanel( m_Notebook, x_IsRefSeq());
    m_Notebook->AddPage(panel7,_("Exceptions"));

    CExperimentPanel *panel8 = new CExperimentPanel (m_Notebook);
    m_Notebook->AddPage(panel8,_("Experiment"));

    CEditInferencePanel *panel9 = new CEditInferencePanel (m_Notebook);
    m_Notebook->AddPage(panel9,_("Inference"));

    m_Constraint = new CConstraintPanel( itemCBulkCmdDlg1, m_TopSeqEntry);
    itemBoxSizer2->Add(m_Constraint, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_OkCancel = new COkCancelPanel( itemCBulkCmdDlg1, ID_EFL_OKCANCEL, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(m_OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CBulkEditFeatDlg content construction
}

bool CBulkEditFeatDlg::x_IsRefSeq()
{
    for (CBioseq_CI bioseq_ci(m_TopSeqEntry, CSeq_inst::eMol_na); bioseq_ci; ++bioseq_ci)
    {
        CBioseq_Handle bsh = *bioseq_ci;
        if (bsh && bsh.IsSetId())
        {
            for (CBioseq_Handle::TId::const_iterator idh = bsh.GetId().begin(); idh != bsh.GetId().end(); ++idh)
            {
                CConstRef<CSeq_id> id = idh->GetSeqId();
                if (id->IsOther())
                {
                    return true;
                }
            }
        }
        break;
    }
    return false;
}

/*!
 * Should we show tooltips?
 */

bool CBulkEditFeatDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CBulkEditFeatDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CBulkEditFeatDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CBulkEditFeatDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CBulkEditFeatDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CBulkEditFeatDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CBulkEditFeatDlg icon retrieval
}



void CBulkEditFeatDlg::ProcessUpdateFeatEvent( wxCommandEvent& event )
{
    UpdateChildrenFeaturePanels(this);
    m_FeatureType->ListAnyOption();
}

void CBulkEditFeatDlg::UpdateChildrenFeaturePanels( wxWindow* win )
{
    wxWindowList &slist = win->GetChildren();
    for (wxWindowList::iterator iter = slist.begin(); iter != slist.end(); ++iter) 
    {
        wxWindow* child = *iter;
        CFeatureTypePanel* panel = dynamic_cast<CFeatureTypePanel*>(child);
        if (panel) 
        {
            GetTopLevelSeqEntryAndProcessor();
            panel->ListPresentFeaturesFirst(m_TopSeqEntry);
        } 
        else 
        {
            UpdateChildrenFeaturePanels(child);
        }
    }     
}


void CBulkEditFeatDlg::SetEditingType(EEditingType editing_type)
{
    int sel = editing_type;
    m_Notebook->SetSelection(sel);
}



string CBulkEditFeatDlg::GetErrorMessage()
{
    return m_ErrorMessage;
}

CRef<CCmdComposite> CBulkEditFeatDlg::GetCommand()
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Bulk Feature Edit"));

    string field_name = m_FeatureType->GetFieldName();
    CSeqFeatData::ESubtype subtype = CSeqFeatData::SubtypeNameToValue(field_name);

    CRef<CEditingActionConstraint> constraint = m_Constraint->GetConstraint(kEmptyStr, CFieldNamePanel::eFieldType_Feature, subtype);
    vector<CConstRef<CObject> > objs;
    for (CFeat_CI feat_ci(m_TopSeqEntry, subtype); feat_ci; ++feat_ci)
    {
        CSeq_feat_Handle fh = feat_ci->GetSeq_feat_Handle();

	bool match = false;
	CBioseq_Handle bsh;
	CScope &scope = fh.GetScope();

	for (CSeq_loc_CI subloc(fh.GetLocation(), objects::CSeq_loc_CI::eEmpty_Skip); subloc; ++subloc)
	{
            bsh = scope.GetBioseqHandle(subloc.GetSeq_id());
            if (bsh)
                match |= constraint->Match(bsh);
	}
        if (match && constraint->Match(fh))
        {
            CConstRef<CObject> object;
            object.Reset(fh.GetOriginalSeq_feat());
            objs.push_back(object);
        }
    }   


    if (objs.size() == 0) {
        wxMessageBox(wxT("No features found!"), wxT("Error"),
                     wxOK | wxICON_ERROR, NULL);
        return cmd;
    }

    bool any_change(false);
    int sel = m_Notebook->GetSelection();
    CBulkFeatEditCmdPanel *win = dynamic_cast<CBulkFeatEditCmdPanel*>(m_Notebook->GetPage(sel));
    if (win) {
        any_change = win->AddCommand(objs, cmd, m_TopSeqEntry.GetScope());
    } 

    if (!any_change) {
        wxMessageBox(wxT("No effect!"), wxT("Error"),
                     wxOK | wxICON_ERROR, NULL);
        cmd.Reset(NULL);
    }

    return cmd;
}



// Evidence Panel
IMPLEMENT_DYNAMIC_CLASS( CEvidencePanel, wxPanel )


/*!
 * CEvidencePanel event table definition
 */

BEGIN_EVENT_TABLE( CEvidencePanel, wxPanel )

////@begin CEvidencePanel event table entries
////@end CEvidencePanel event table entries

END_EVENT_TABLE()


/*!
 * CEvidencePanel constructors
 */

CEvidencePanel::CEvidencePanel()
{
    Init();
}

CEvidencePanel::CEvidencePanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CEvidencePanel creator
 */

bool CEvidencePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CEvidencePanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CEvidencePanel creation
    return true;
}


/*!
 * CEvidencePanel destructor
 */

CEvidencePanel::~CEvidencePanel()
{
////@begin CEvidencePanel destruction
////@end CEvidencePanel destruction
}


/*!
 * Member initialisation
 */

void CEvidencePanel::Init()
{
////@begin CEvidencePanel member initialisation
////@end CEvidencePanel member initialisation
}


/*!
 * Control creation for CEvidencePanel
 */

void CEvidencePanel::CreateControls()
{    
////@begin CEvidencePanel content construction
    CEvidencePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* text1 = new wxStaticText( itemPanel1, wxID_STATIC, _("Click OK to clear feature evidence"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(text1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CEvidencePanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CEvidencePanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CEvidencePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CEvidencePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CEvidencePanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CEvidencePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CEvidencePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CEvidencePanel icon retrieval
}

bool CEvidencePanel::AddCommand(vector<CConstRef<CObject> > &objs, CRef<CCmdComposite> cmd, CScope &scope)
{
    bool any_change(false);
    ITERATE(vector<CConstRef<CObject> >, it, objs) {
        const CSeq_feat* f = dynamic_cast<const CSeq_feat* >((*it).GetPointer());
        if (f && f->IsSetExp_ev()) {
            CRef<CObject> new_obj = GetNewObject(*it);
            CSeq_feat* new_feat = dynamic_cast<CSeq_feat* >(new_obj.GetPointer());
            new_feat->ResetExp_ev();
            CRef<CCmdComposite> ecmd = GetReplacementCommand(*it, new_obj, scope, "Clear Evidence");
            if (ecmd) {
                cmd->AddCommand(*ecmd);
                any_change = true;
            }
        }
    }
    return any_change;
}


// Location Panel
IMPLEMENT_DYNAMIC_CLASS( CBlkEdtFeatLocationPanel, wxPanel )


/*!
 * CBlkEdtFeatLocationPanel event table definition
 */

BEGIN_EVENT_TABLE( CBlkEdtFeatLocationPanel, wxPanel )

////@begin CBlkEdtFeatLocationPanel event table entries
////@end CBlkEdtFeatLocationPanel event table entries

END_EVENT_TABLE()


/*!
 * CBlkEdtFeatLocationPanel constructors
 */

CBlkEdtFeatLocationPanel::CBlkEdtFeatLocationPanel()
{
    Init();
}

CBlkEdtFeatLocationPanel::CBlkEdtFeatLocationPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CBlkEdtFeatLocationPanel creator
 */

bool CBlkEdtFeatLocationPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CBlkEdtFeatLocationPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CBlkEdtFeatLocationPanel creation
    return true;
}


/*!
 * CBlkEdtFeatLocationPanel destructor
 */

CBlkEdtFeatLocationPanel::~CBlkEdtFeatLocationPanel()
{
////@begin CBlkEdtFeatLocationPanel destruction
////@end CBlkEdtFeatLocationPanel destruction
}


/*!
 * Member initialisation
 */

void CBlkEdtFeatLocationPanel::Init()
{
////@begin CBlkEdtFeatLocationPanel member initialisation
////@end CBlkEdtFeatLocationPanel member initialisation
}


/*!
 * Control creation for CBlkEdtFeatLocationPanel
 */

void CBlkEdtFeatLocationPanel::CreateControls()
{    
////@begin CBlkEdtFeatLocationPanel content construction
    CBlkEdtFeatLocationPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_EditLoc = new CEditFeatLocPanel(itemPanel1);
    itemBoxSizer2->Add(m_EditLoc, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CBlkEdtFeatLocationPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CBlkEdtFeatLocationPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CBlkEdtFeatLocationPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CBlkEdtFeatLocationPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CBlkEdtFeatLocationPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CBlkEdtFeatLocationPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CBlkEdtFeatLocationPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CBlkEdtFeatLocationPanel icon retrieval
}

bool CBlkEdtFeatLocationPanel::AddCommand(vector<CConstRef<CObject> > &objs, CRef<CCmdComposite> cmd, CScope &scope)
{
    bool any_change(false);
    CRef<edit::CLocationEditPolicy> policy = m_EditLoc->GetPolicy();            
    if (policy) {
        int offset = 1;
        bool create_general_only = false;
        CScope::TTSE_Handles vec_tse;
        scope.GetAllTSEs(vec_tse, CScope::eAllTSEs);
        if (!vec_tse.empty())
            create_general_only = objects::edit::IsGeneralIdProtPresent(vec_tse.front());
        ITERATE(vector<CConstRef<CObject> >, it, objs) {
            const CSeq_feat* f = dynamic_cast<const CSeq_feat* >((*it).GetPointer());
            if (f) 
            {
                if (f->IsSetData() && f->GetData().IsCdregion() && 
                    f->IsSetExcept_text() && NStr::Find(f->GetExcept_text(), "RNA editing") != string::npos) 
                {
                    continue;
                }
                CRef<CCmdComposite> ecmd = GetEditLocationCommand(*policy,
                                                                  m_EditLoc->GetRetranslate(),
                                                                  m_EditLoc->GetAdjustGene(),
                                                                  *f, scope, offset, create_general_only);
                if (ecmd) {
                    cmd->AddCommand(*ecmd);
                    any_change = true;
                }
            }
        }
    }   
    return any_change;
}


// Strand Panel
IMPLEMENT_DYNAMIC_CLASS( CStrandPanel, wxPanel )


/*!
 * CStrandPanel event table definition
 */

BEGIN_EVENT_TABLE( CStrandPanel, wxPanel )

////@begin CStrandPanel event table entries
////@end CStrandPanel event table entries

END_EVENT_TABLE()


/*!
 * CStrandPanel constructors
 */

CStrandPanel::CStrandPanel()
{
    Init();
}

CStrandPanel::CStrandPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CStrandPanel creator
 */

bool CStrandPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CStrandPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CStrandPanel creation
    return true;
}


/*!
 * CStrandPanel destructor
 */

CStrandPanel::~CStrandPanel()
{
////@begin CStrandPanel destruction
////@end CStrandPanel destruction
}


/*!
 * Member initialisation
 */

void CStrandPanel::Init()
{
////@begin CStrandPanel member initialisation
    m_StrandFrom = NULL;
    m_StrandTo = NULL;
////@end CStrandPanel member initialisation
}


/*!
 * Control creation for CStrandPanel
 */

void CStrandPanel::CreateControls()
{    
////@begin CStrandPanel content construction
    CStrandPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);


    wxStaticText* stattext = new wxStaticText( itemPanel1, wxID_STATIC, _("Convert location strand from"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(stattext, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);       

    wxArrayString strand_strings;
    strand_strings.Add(_("Any"));
    strand_strings.Add(_("Unknown"));
    strand_strings.Add(_("Plus"));
    strand_strings.Add(_("Minus"));
    strand_strings.Add(_("Both"));
    m_StrandFrom = new wxChoice( itemPanel1, wxID_ANY, wxDefaultPosition, wxDefaultSize, strand_strings, 0 );
    m_StrandFrom->SetStringSelection(_("Plus"));
    itemBoxSizer2->Add(m_StrandFrom, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* stattext2 = new wxStaticText( itemPanel1, wxID_STATIC, _("To"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(stattext2, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);     

    strand_strings[0] = _("Reverse");
    m_StrandTo = new wxChoice( itemPanel1, wxID_ANY, wxDefaultPosition, wxDefaultSize, strand_strings, 0 );
    m_StrandTo->SetStringSelection(_("Minus"));
    itemBoxSizer2->Add(m_StrandTo, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CStrandPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CStrandPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CStrandPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CStrandPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CStrandPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CStrandPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CStrandPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CStrandPanel icon retrieval
}

bool CStrandPanel::AddCommand(vector<CConstRef<CObject> > &objs, CRef<CCmdComposite> cmd, CScope &scope)
{
    bool any_change(false);
  
    string strand_from = ToStdString(m_StrandFrom->GetStringSelection());
    string strand_to = ToStdString(m_StrandTo->GetStringSelection());
    ITERATE(vector<CConstRef<CObject> >, it, objs) {
        const CSeq_feat* feat = dynamic_cast<const CSeq_feat* >((*it).GetPointer());
        if (feat) {
	    CSeq_feat_Handle fh = scope.GetSeq_featHandle(*feat);
	    if (fh)
	    {
		CBioseq_Handle bsh;
		try
		{
		    bsh = scope.GetBioseqHandle(fh.GetLocation());
		}
		catch (const CObjMgrException&) {}
		if (!bsh || !bsh.IsAa())
		{
		    CRef<CObject> new_obj = GetNewObject(*it);
		    CSeq_feat* new_feat = dynamic_cast<CSeq_feat* >(new_obj.GetPointer());
		    bool this_change = macro::CMacroFunction_ConvertLocStrand::s_ConvertLocationStrand(*new_feat, scope, strand_from, strand_to);
		    if (this_change) {
			CRef<CCmdComposite> ecmd = GetReplacementCommand(*it, new_obj, scope, "Change strand");
			if (ecmd) {
			    cmd->AddCommand(*ecmd);
			    any_change = true;
			}
		    }
		}
	    }
	}
    }
    return any_change;
}


// RevInt Panel
IMPLEMENT_DYNAMIC_CLASS( CRevIntPanel, wxPanel )


/*!
 * CRevIntPanel event table definition
 */

BEGIN_EVENT_TABLE( CRevIntPanel, wxPanel )

////@begin CRevIntPanel event table entries
////@end CRevIntPanel event table entries

END_EVENT_TABLE()


/*!
 * CRevIntPanel constructors
 */

CRevIntPanel::CRevIntPanel()
{
    Init();
}

CRevIntPanel::CRevIntPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CRevIntPanel creator
 */

bool CRevIntPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CRevIntPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CRevIntPanel creation
    return true;
}


/*!
 * CRevIntPanel destructor
 */

CRevIntPanel::~CRevIntPanel()
{
////@begin CRevIntPanel destruction
////@end CRevIntPanel destruction
}


/*!
 * Member initialisation
 */

void CRevIntPanel::Init()
{
////@begin CRevIntPanel member initialisation
////@end CRevIntPanel member initialisation
}


/*!
 * Control creation for CRevIntPanel
 */

void CRevIntPanel::CreateControls()
{    
////@begin CRevIntPanel content construction
    CRevIntPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* text1 = new wxStaticText( itemPanel1, wxID_STATIC, _("Click OK to reverse location interval order"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(text1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CRevIntPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CRevIntPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CRevIntPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CRevIntPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CRevIntPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CRevIntPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CRevIntPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CRevIntPanel icon retrieval
}

bool CRevIntPanel::AddCommand(vector<CConstRef<CObject> > &objs, CRef<CCmdComposite> cmd, CScope &scope)
{
    bool any_change(false);
    ITERATE(vector<CConstRef<CObject> >, it, objs) {
        const CSeq_feat* f = dynamic_cast<const CSeq_feat* >((*it).GetPointer());
        if (f && f->IsSetLocation()) {
            CRef<CObject> new_obj = GetNewObject(*it);
            CSeq_feat* new_feat = dynamic_cast<CSeq_feat* >(new_obj.GetPointer());
            CSeq_loc* rc = sequence::SeqLocRevCmpl(new_feat->GetLocation(), &(scope));
            if (rc) {
                rc->FlipStrand();
                new_feat->SetLocation().Assign(*rc);
                delete rc;
                CRef<CCmdComposite> ecmd = GetReplacementCommand(*it, new_obj, scope, "Reverse Interval Order");
                if (ecmd) {
                    cmd->AddCommand(*ecmd);
                    any_change = true;
                }
            }
        }
    }

    
    return any_change;
}

// Resync Panel
IMPLEMENT_DYNAMIC_CLASS( CResyncPanel, wxPanel )


/*!
 * CResyncPanel event table definition
 */

BEGIN_EVENT_TABLE( CResyncPanel, wxPanel )

////@begin CResyncPanel event table entries
////@end CResyncPanel event table entries

END_EVENT_TABLE()


/*!
 * CResyncPanel constructors
 */

CResyncPanel::CResyncPanel()
{
    Init();
}

CResyncPanel::CResyncPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CResyncPanel creator
 */

bool CResyncPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CResyncPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CResyncPanel creation
    return true;
}


/*!
 * CResyncPanel destructor
 */

CResyncPanel::~CResyncPanel()
{
////@begin CResyncPanel destruction
////@end CResyncPanel destruction
}


/*!
 * Member initialisation
 */

void CResyncPanel::Init()
{
////@begin CResyncPanel member initialisation
////@end CResyncPanel member initialisation
}


/*!
 * Control creation for CResyncPanel
 */

void CResyncPanel::CreateControls()
{    
////@begin CResyncPanel content construction
    CResyncPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* text1 = new wxStaticText( itemPanel1, wxID_STATIC, _("Click OK to resynch partials"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(text1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CResyncPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CResyncPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CResyncPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CResyncPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CResyncPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CResyncPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CResyncPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CResyncPanel icon retrieval
}

bool CResyncPanel::AddCommand(vector<CConstRef<CObject> > &objs, CRef<CCmdComposite> cmd, CScope &scope)
{
    bool any_change(false);

    ITERATE(vector<CConstRef<CObject> >, it, objs) {
        const CSeq_feat* f = dynamic_cast<const CSeq_feat* >((*it).GetPointer());
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(*f);
        if (feature::AdjustFeaturePartialFlagForLocation(*new_feat)) {
            CSeq_feat_Handle fh = scope.GetSeq_featHandle(*f);
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh, *new_feat)));
            any_change = true;
        }
        
        CRef<CCmdComposite> synch = GetSynchronizeProteinPartialsCommand(scope, *f);
        if (synch) {
            cmd->AddCommand(*synch);
            any_change = true;
        }
    }

    return any_change;
}

// Pseudo Panel
IMPLEMENT_DYNAMIC_CLASS( CPseudoPanel, wxPanel )


/*!
 * CPseudoPanel event table definition
 */

BEGIN_EVENT_TABLE( CPseudoPanel, wxPanel )

////@begin CPseudoPanel event table entries
////@end CPseudoPanel event table entries

END_EVENT_TABLE()


/*!
 * CPseudoPanel constructors
 */

CPseudoPanel::CPseudoPanel()
{
    Init();
}

CPseudoPanel::CPseudoPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CPseudoPanel creator
 */

bool CPseudoPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CPseudoPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CPseudoPanel creation
    return true;
}


/*!
 * CPseudoPanel destructor
 */

CPseudoPanel::~CPseudoPanel()
{
////@begin CPseudoPanel destruction
////@end CPseudoPanel destruction
}


/*!
 * Member initialisation
 */

void CPseudoPanel::Init()
{
////@begin CPseudoPanel member initialisation
////@end CPseudoPanel member initialisation
}


/*!
 * Control creation for CPseudoPanel
 */

void CPseudoPanel::CreateControls()
{    
////@begin CPseudoPanel content construction
    CPseudoPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxArrayString pseudogeneChoiceStrings;
    pseudogeneChoiceStrings.Add(wxEmptyString);
    pseudogeneChoiceStrings.Add(_("Processed"));
    pseudogeneChoiceStrings.Add(_("Unprocessed"));
    pseudogeneChoiceStrings.Add(_("Unitary"));
    pseudogeneChoiceStrings.Add(_("Allelic"));
    pseudogeneChoiceStrings.Add(_("Unknown"));
    pseudogeneChoiceStrings.Add(_("Unqualified"));
    m_PseudogeneChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, pseudogeneChoiceStrings, 0 );
    itemBoxSizer2->Add(m_PseudogeneChoice, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CPseudoPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CPseudoPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CPseudoPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CPseudoPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CPseudoPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CPseudoPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CPseudoPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CPseudoPanel icon retrieval
}

static const char* kPseudogene = "pseudogene";

bool CPseudoPanel::AddCommand(vector<CConstRef<CObject> > &objs, CRef<CCmdComposite> cmd, CScope &scope)
{
    bool any_change(false);

    ITERATE(vector<CConstRef<CObject> >, it, objs) {
        const CSeq_feat* f = dynamic_cast<const CSeq_feat* >((*it).GetPointer());
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(*f);
        CSeq_feat_Handle fh = scope.GetSeq_featHandle(*f);
                
        string pseudo_choice = ToStdString(m_PseudogeneChoice->GetStringSelection());
        NStr::ToLower(pseudo_choice);
        if (new_feat->IsSetQual()) {
            CSeq_feat::TQual::iterator it = new_feat->SetQual().begin();
            while (it != new_feat->SetQual().end()) {
                if ((*it)->IsSetQual() && NStr::EqualNocase((*it)->GetQual(), kPseudogene)) {
                    it = new_feat->SetQual().erase(it);
                } else {
                    ++it;
                }
            }
            if (new_feat->SetQual().empty()) {
                new_feat->ResetQual();
            }
        } 
        if (!NStr::IsBlank(pseudo_choice)) {
            if (pseudo_choice != "unqualified")
            {
                CRef<CGb_qual> qual(new CGb_qual(kPseudogene, pseudo_choice));
                new_feat->SetQual().push_back(qual);
            }
            new_feat->SetPseudo(true);
        } else {
            new_feat->ResetPseudo();
        }
        
        cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh, *new_feat)));
        any_change = true;
    }
    
    

    return any_change;
}



// Exception Panel
IMPLEMENT_DYNAMIC_CLASS( CExceptionPanel, wxPanel )


/*!
 * CExceptionPanel event table definition
 */

BEGIN_EVENT_TABLE( CExceptionPanel, wxPanel )

////@begin CExceptionPanel event table entries
////@end CExceptionPanel event table entries

END_EVENT_TABLE()


/*!
 * CExceptionPanel constructors
 */

CExceptionPanel::CExceptionPanel()
: m_is_refseq(false)
{
    Init();
}

CExceptionPanel::CExceptionPanel( wxWindow* parent, bool is_refseq, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_is_refseq(is_refseq)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CExceptionPanel creator
 */

bool CExceptionPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CExceptionPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CExceptionPanel creation
    return true;
}


/*!
 * CExceptionPanel destructor
 */

CExceptionPanel::~CExceptionPanel()
{
////@begin CExceptionPanel destruction
////@end CExceptionPanel destruction
}


/*!
 * Member initialisation
 */

void CExceptionPanel::Init()
{
////@begin CExceptionPanel member initialisation
////@end CExceptionPanel member initialisation
}


/*!
 * Control creation for CExceptionPanel
 */

void CExceptionPanel::CreateControls()
{    
////@begin CExceptionPanel content construction
    CExceptionPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* text1 = new wxStaticText( itemPanel1, wxID_STATIC, _("Set Explanation to"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(text1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString exceptionStrings;
    /* exceptionStrings.Add(wxEmptyString);
    exceptionStrings.Add(_("RNA Editing"));
    exceptionStrings.Add(_("reasons given in citation"));
    exceptionStrings.Add(_("ribosomal slippage"));
    exceptionStrings.Add(_("trans splicing"));
    exceptionStrings.Add(_("artificial frameshift"));
    exceptionStrings.Add(_("nonconsensus splice site"));
    exceptionStrings.Add(_("rearrangement required"));
    */
    vector<string> explanation_strings = CSeq_feat::GetListOfLegalExceptions(m_is_refseq);
    for (vector<string>::const_iterator s = explanation_strings.begin(); s != explanation_strings.end(); ++s)
        exceptionStrings.Add(wxString(*s));

    m_Exception = new wxComboBox( itemPanel1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, exceptionStrings, wxCB_DROPDOWN);
    itemBoxSizer2->Add(m_Exception, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString featureProductStrings;
    featureProductStrings.Add(_("Present"));
    featureProductStrings.Add(_("Absent"));
    featureProductStrings.Add(_("Either"));
    m_FeatureProduct = new wxRadioBox( itemPanel1, wxID_ANY, _("Where feature product is"), wxDefaultPosition, wxDefaultSize, featureProductStrings, 0, wxRA_SPECIFY_COLS );
    m_FeatureProduct->SetSelection(2);
    itemBoxSizer2->Add(m_FeatureProduct, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    m_MoveToComment = new wxCheckBox( itemPanel1, wxID_ANY, _("Move explanation to comment"), wxDefaultPosition, wxDefaultSize, 0 );
    m_MoveToComment->SetValue(false);
    itemBoxSizer2->Add(m_MoveToComment, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

////@end CExceptionPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CExceptionPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CExceptionPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CExceptionPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CExceptionPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CExceptionPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CExceptionPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CExceptionPanel icon retrieval
}


bool CExceptionPanel::AddCommand(vector<CConstRef<CObject> > &objs, CRef<CCmdComposite> cmd, CScope &scope)
{
    bool any_change(false);
    string value = m_Exception->GetValue().ToStdString();
    bool move_to_comment = m_MoveToComment->GetValue();
    int feat_product_present =  m_FeatureProduct->GetSelection();

    ITERATE(vector<CConstRef<CObject> >, it, objs) {
        const CSeq_feat* f = dynamic_cast<const CSeq_feat* >((*it).GetPointer());
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(*f);
        CSeq_feat_Handle fh = scope.GetSeq_featHandle(*f);
        if (feat_product_present == 2 || (new_feat->IsSetProduct() && feat_product_present == 0) || (!new_feat->IsSetProduct() && feat_product_present == 1))
        {
            string old_value;
            if (new_feat->IsSetExcept_text())
            {
                old_value = new_feat->GetExcept_text();
            }

            string comment;
            if (new_feat->IsSetComment())
            {
                comment = new_feat->GetComment();
            }
            
            if (!old_value.empty() && move_to_comment)
            {
                if (!NStr::IsBlank(comment))
                    comment += "; " + old_value;
                else
                    comment = old_value;
                new_feat->SetComment(comment);
            }
            
            if (!NStr::IsBlank(value))
            {
                new_feat->SetExcept(true);
                new_feat->SetExcept_text(value);
            }
            else
            {
                new_feat->SetExcept(false);
                new_feat->ResetExcept_text();
            }
            
            
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh, *new_feat)));
            any_change = true;
        }
    }

    return any_change;
}




// Experiment Panel
IMPLEMENT_DYNAMIC_CLASS( CExperimentPanel, wxPanel )


/*!
 * CExperimentPanel event table definition
 */

BEGIN_EVENT_TABLE( CExperimentPanel, wxPanel )

////@begin CExperimentPanel event table entries
////@end CExperimentPanel event table entries

END_EVENT_TABLE()


/*!
 * CExperimentPanel constructors
 */

CExperimentPanel::CExperimentPanel()
{
    Init();
}

CExperimentPanel::CExperimentPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CExperimentPanel creator
 */

bool CExperimentPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CExperimentPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CExperimentPanel creation
    return true;
}


/*!
 * CExperimentPanel destructor
 */

CExperimentPanel::~CExperimentPanel()
{
////@begin CExperimentPanel destruction
////@end CExperimentPanel destruction
}


/*!
 * Member initialisation
 */

void CExperimentPanel::Init()
{
////@begin CExperimentPanel member initialisation
////@end CExperimentPanel member initialisation
}


/*!
 * Control creation for CExperimentPanel
 */

void CExperimentPanel::CreateControls()
{    
////@begin CExperimentPanel content construction
    CExperimentPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(itemBoxSizer2);   

    wxArrayString create_or_delete;
    create_or_delete.Add(_("Set Experiment"));
    create_or_delete.Add(_("Remove Experiments"));
    m_CreateOrDelete = new wxRadioBox( itemPanel1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, create_or_delete, 0, wxRA_SPECIFY_ROWS );
    m_CreateOrDelete->SetSelection(0);
    itemBoxSizer2->Add(m_CreateOrDelete, 0, wxALIGN_TOP, 0);

    m_SingleExperiment = new CSingleExperimentPanel(itemPanel1);
    m_SingleExperiment->SetValue(kEmptyStr);
    itemBoxSizer2->Add(m_SingleExperiment, 0, wxALIGN_TOP|wxALL, 5);

    m_CreateOrDelete->Bind(wxEVT_RADIOBOX, &CExperimentPanel::OnCreateOrDelete, this);

////@end CExperimentPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CExperimentPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CExperimentPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CExperimentPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CExperimentPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CExperimentPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CExperimentPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CExperimentPanel icon retrieval
}


bool CExperimentPanel::AddCommand(vector<CConstRef<CObject> > &objs, CRef<CCmdComposite> cmd, CScope &scope)
{
    bool any_change(false);

    int create_or_delete =  m_CreateOrDelete->GetSelection();

    string value;
    if (create_or_delete == 0)
        value = m_SingleExperiment->GetValue();



    ITERATE(vector<CConstRef<CObject> >, it, objs) 
    {
        const CSeq_feat* f = dynamic_cast<const CSeq_feat* >((*it).GetPointer());
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(*f);
        CSeq_feat_Handle fh = scope.GetSeq_featHandle(*f);
          
        if (create_or_delete == 0 && !value.empty())
        {
            CRef<CGb_qual> edited_qual (new CGb_qual("experiment", value));
            new_feat->SetQual().push_back(edited_qual);
            any_change = true;
        }
        if (create_or_delete == 1)
        {
            CSingleExperimentPanel::RemoveRepresentedQuals(*new_feat, "experiment");
            any_change = true;
        }
        
        cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh, *new_feat)));
    }

    return any_change;
}

void CExperimentPanel::OnCreateOrDelete(wxCommandEvent& event)
{
    if  (m_CreateOrDelete->GetSelection() == 0)
        m_SingleExperiment->Enable();
    else
        m_SingleExperiment->Disable();
}







// Inference Panel
IMPLEMENT_DYNAMIC_CLASS( CEditInferencePanel, wxPanel )


/*!
 * CEditInferencePanel event table definition
 */

BEGIN_EVENT_TABLE( CEditInferencePanel, wxPanel )

////@begin CEditInferencePanel event table entries
////@end CEditInferencePanel event table entries

END_EVENT_TABLE()


/*!
 * CEditInferencePanel constructors
 */

CEditInferencePanel::CEditInferencePanel()
{
    Init();
}

CEditInferencePanel::CEditInferencePanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CEditInferencePanel creator
 */

bool CEditInferencePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CEditInferencePanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CEditInferencePanel creation
    return true;
}


/*!
 * CEditInferencePanel destructor
 */

CEditInferencePanel::~CEditInferencePanel()
{
////@begin CEditInferencePanel destruction
////@end CEditInferencePanel destruction
}


/*!
 * Member initialisation
 */

void CEditInferencePanel::Init()
{
////@begin CEditInferencePanel member initialisation
////@end CEditInferencePanel member initialisation
}


/*!
 * Control creation for CEditInferencePanel
 */

void CEditInferencePanel::CreateControls()
{    
////@begin CEditInferencePanel content construction
    CEditInferencePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(itemBoxSizer2);   

    wxArrayString create_or_delete;
    create_or_delete.Add(_("Set Inference"));
    create_or_delete.Add(_("Remove Inferences"));
    m_CreateOrDelete = new wxRadioBox( itemPanel1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, create_or_delete, 0, wxRA_SPECIFY_ROWS );
    m_CreateOrDelete->SetSelection(0);
    itemBoxSizer2->Add(m_CreateOrDelete, 0, wxALIGN_TOP, 0);

    m_SingleInference = new CInferencePanel(itemPanel1);
    m_SingleInference->SetValue(kEmptyStr);
    itemBoxSizer2->Add(m_SingleInference, 0, wxALIGN_TOP|wxALL, 5);

    m_CreateOrDelete->Bind(wxEVT_RADIOBOX, &CEditInferencePanel::OnCreateOrDelete, this);

////@end CEditInferencePanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CEditInferencePanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CEditInferencePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CEditInferencePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CEditInferencePanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CEditInferencePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CEditInferencePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CEditInferencePanel icon retrieval
}


bool CEditInferencePanel::AddCommand(vector<CConstRef<CObject> > &objs, CRef<CCmdComposite> cmd, CScope &scope)
{
    bool any_change(false);

    int create_or_delete =  m_CreateOrDelete->GetSelection();

    string value;
    if (create_or_delete == 0)
        value = m_SingleInference->GetValue();



    ITERATE(vector<CConstRef<CObject> >, it, objs) 
    {
        const CSeq_feat* f = dynamic_cast<const CSeq_feat* >((*it).GetPointer());
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(*f);
        CSeq_feat_Handle fh = scope.GetSeq_featHandle(*f);
          
        if (create_or_delete == 0 && !value.empty())
        {
            CRef<CGb_qual> edited_qual (new CGb_qual("inference", value));
            new_feat->SetQual().push_back(edited_qual);
            any_change = true;
        }
        if (create_or_delete == 1)
        {
            CInferencePanel::RemoveRepresentedQuals(*new_feat, "inference");
            any_change = true;
        }
        
        cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh, *new_feat)));
    }

    return any_change;
}

void CEditInferencePanel::OnCreateOrDelete(wxCommandEvent& event)
{
    if  (m_CreateOrDelete->GetSelection() == 0)
        m_SingleInference->Enable();
    else
        m_SingleInference->Disable();
}

END_NCBI_SCOPE

