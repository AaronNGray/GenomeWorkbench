/*  $Id: extend_partial_features.cpp 42192 2019-01-10 16:52:15Z filippov $
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

////@begin includes
////@end includes

#include <wx/stattext.h>
#include <wx/msgdlg.h>

#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <objtools/edit/loc_edit.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/extend_partial_features.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);

/*!
 * CExtendPartialFeatDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CExtendPartialFeatDlg, CBulkCmdDlg )


/*!
 * CExtendPartialFeatDlg event table definition
 */

BEGIN_EVENT_TABLE( CExtendPartialFeatDlg, CBulkCmdDlg )

////@begin CExtendPartialFeatDlg event table entries
   
    EVT_UPDATE_FEATURE_LIST(wxID_ANY, CExtendPartialFeatDlg::ProcessUpdateFeatEvent )

////@end CExtendPartialFeatDlg event table entries

END_EVENT_TABLE()


/*!
 * CExtendPartialFeatDlg constructors
 */

CExtendPartialFeatDlg::CExtendPartialFeatDlg()
{
    Init();
}

CExtendPartialFeatDlg::CExtendPartialFeatDlg( wxWindow* parent, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
  : CBulkCmdDlg(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CExtendPartialFeatDlg creator
 */

bool CExtendPartialFeatDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CExtendPartialFeatDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CExtendPartialFeatDlg creation
    return true;
}


/*!
 * CExtendPartialFeatDlg destructor
 */

CExtendPartialFeatDlg::~CExtendPartialFeatDlg()
{
////@begin CExtendPartialFeatDlg destruction
////@end CExtendPartialFeatDlg destruction
}


/*!
 * Member initialisation
 */

void CExtendPartialFeatDlg::Init()
{
////@begin CExtendPartialFeatDlg member initialisation
    m_FeatureType = NULL;
    m_OkCancel = NULL;
////@end CExtendPartialFeatDlg member initialisation
    m_ErrorMessage = "";
    m_Extend5Btn = NULL;
    m_Extend3Btn = NULL;
    m_FeatureType = NULL;
    m_StringConstraintPanel = NULL;
}


/*!
 * Control creation for CExtendPartialFeatDlg
 */

void CExtendPartialFeatDlg::CreateControls()
{    
////@begin CExtendPartialFeatDlg content construction
    CExtendPartialFeatDlg* itemCBulkCmdDlg1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);

    m_FeatureType = new CFeatureTypePanel( itemCBulkCmdDlg1, ID_EFL_FEATURETYPE, wxDefaultPosition, wxSize(100, 100), wxSIMPLE_BORDER );
    itemBoxSizer2->Add(m_FeatureType, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    GetTopLevelSeqEntryAndProcessor();
    m_FeatureType->ListPresentFeaturesFirst(m_TopSeqEntry); 
    m_FeatureType->ListAnyOption();

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Extend5Btn = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("Extend partial 5'"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Extend5Btn->SetValue(true);
    itemBoxSizer4->Add(m_Extend5Btn, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Extend3Btn = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("Extend partial 3'"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Extend3Btn->SetValue(true);
    itemBoxSizer4->Add(m_Extend3Btn, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxPanel *container1 = new wxPanel( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxSize(800,260), 0 );
    itemBoxSizer2->Add(container1, 1, wxGROW|wxALL|wxFIXED_MINSIZE, 0);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    container1->SetSizer(itemBoxSizer5);

    vector<CFieldNamePanel::EFieldType> field_types;
    /*field_types.push_back(CFieldNamePanel::eFieldType_Source);
    field_types.push_back(CFieldNamePanel::eFieldType_Misc);
    field_types.push_back(CFieldNamePanel::eFieldType_Feature);
    field_types.push_back(CFieldNamePanel::eFieldType_CDSGeneProt);
    field_types.push_back(CFieldNamePanel::eFieldType_RNA);
    field_types.push_back(CFieldNamePanel::eFieldType_MolInfo);
    field_types.push_back(CFieldNamePanel::eFieldType_Pub);
    field_types.push_back(CFieldNamePanel::eFieldType_DBLink);
    field_types.push_back(CFieldNamePanel::eFieldType_SeqId);
    */
    field_types.push_back(CFieldNamePanel::eFieldType_Feature);
    m_FieldType = new CFieldChoicePanel( container1, field_types, true, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(m_FieldType, 1, wxGROW|wxALL|wxFIXED_MINSIZE, 0);
    m_FieldType->SetFieldType(CFieldNamePanel::eFieldType_Feature);

    wxPanel *container2 = new wxPanel( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxSize(800,90), 0 );
    itemBoxSizer2->Add(container2, 0, wxALIGN_LEFT|wxALL|wxFIXED_MINSIZE, 0);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    container2->SetSizer(itemBoxSizer6);

    m_StringConstraintPanel = new CStringConstraintPanel( container2, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(m_StringConstraintPanel, 0, wxALIGN_LEFT|wxALL|wxFIXED_MINSIZE, 0); 

    m_OkCancel = new COkCancelPanel( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(m_OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CExtendPartialFeatDlg content construction
}


/*!
 * Should we show tooltips?
 */

bool CExtendPartialFeatDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CExtendPartialFeatDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CExtendPartialFeatDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CExtendPartialFeatDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CExtendPartialFeatDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CExtendPartialFeatDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CExtendPartialFeatDlg icon retrieval
}


void CExtendPartialFeatDlg::ProcessUpdateFeatEvent( wxCommandEvent& event )
{
    UpdateChildrenFeaturePanels(this->GetSizer());
    m_FeatureType->ListAnyOption();
}



string CExtendPartialFeatDlg::GetErrorMessage()
{
    return m_ErrorMessage;
}


CRef<CCmdComposite> CExtendPartialFeatDlg::GetCommand()
{
    CRef<CCmdComposite> cmd(NULL);

    string feat_type = m_FeatureType->GetFieldName();
    if (NStr::EqualNocase(feat_type, "any")) {
        feat_type = "";
    }
    string field_name = "";
    if (NStr::IsBlank(feat_type)) {
        field_name = kPartialStart;
    } else {
        field_name = feat_type + " " + kPartialStart;
    }

    CRef<CMiscSeqTableColumn> col(new CMiscSeqTableColumn(field_name));

    CRef<edit::CStringConstraint> string_constraint(m_StringConstraintPanel->GetStringConstraint());
    string constraint_field = m_FieldType->GetFieldName(false);

    vector<CConstRef<CObject> > objs = col->GetObjects(m_TopSeqEntry, constraint_field, string_constraint);

    if (objs.size() == 0) {
        wxMessageBox(wxT("No features found!"), wxT("Error"),
                     wxOK | wxICON_ERROR, NULL);
        return cmd;
    }

    bool any_change = false;
    CScope &scope =  m_TopSeqEntry.GetScope();
  
  
    bool extend5 = m_Extend5Btn->GetValue();
    bool extend3 = m_Extend3Btn->GetValue();
    cmd.Reset(new CCmdComposite("Extend Partial Features"));
    int offset = 1;
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(m_TopSeqEntry);
    ITERATE(vector<CConstRef<CObject> >, it, objs) 
    {
        const CSeq_feat* f = dynamic_cast<const CSeq_feat* >((*it).GetPointer());
        if (f) 
        {   
            if (f->IsSetData() && f->GetData().IsCdregion() && 
                f->IsSetExcept_text() && NStr::Find(f->GetExcept_text(), "RNA editing") != string::npos) 
            {
                continue;
            }

            const CSeq_loc& loc = f->GetLocation();
            CBioseq_Handle bsh = scope.GetBioseqHandle(loc);

            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->Assign(*f);
            
            if (extend5 && f->GetLocation().IsPartialStart(eExtreme_Biological)  && !Is5AtStartOfSeq(loc, bsh))
            {
                edit::CLocationEditPolicy::Extend5(*new_feat, scope);
                any_change = true;
            }
            if (extend3 && f->GetLocation().IsPartialStop(eExtreme_Biological)  && !Is3AtEndOfSeq(loc, bsh))
            {                
                edit::CLocationEditPolicy::Extend3(*new_feat, scope);
                any_change = true;
            }
            // command to change feature
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(scope.GetSeq_featHandle(*f), *new_feat)));

            // retranslate or resynch if coding region
            if (new_feat->IsSetProduct() && new_feat->GetData().IsCdregion()) 
            {
                CRef<CCmdComposite> retranslate_cmd = GetRetranslateCDSCommand(scope, *new_feat, offset, create_general_only);
                if (retranslate_cmd) {
                    cmd->AddCommand(*retranslate_cmd);
                }
            }
            
        }
    }
                
    
      
    if (!any_change) {
        wxMessageBox(wxT("No effect!"), wxT("Error"),
                     wxOK | wxICON_ERROR, NULL);
        cmd.Reset(NULL);
    }

    return cmd;
}

bool CExtendPartialFeatDlg::Is5AtStartOfSeq(const CSeq_loc& loc, CBioseq_Handle bsh)
{
    bool rval = false;

    ENa_strand strand = loc.GetStrand();
    if (strand == eNa_strand_minus) {
        if (bsh && loc.GetStart(eExtreme_Biological) == bsh.GetInst_Length() - 1) {
            rval = true;
        }
    } else {
        if (loc.GetStart(eExtreme_Biological) == 0) {
            rval = true;
        }
    }
    return rval;
}


bool CExtendPartialFeatDlg::Is3AtEndOfSeq(const CSeq_loc& loc, CBioseq_Handle bsh)
{
    bool rval = false;
    ENa_strand strand = loc.GetStrand();

    if (strand == eNa_strand_minus) {
        if (loc.GetStop(eExtreme_Biological) == 0) {
            rval = true;
        }
    } else {
        if (bsh && loc.GetStop(eExtreme_Biological) == bsh.GetInst_Length() - 1) {                        
            rval = true;
        }
    }
    return rval;
}


void CExtendPartialFeatDlg::ApplyToAllAndDie()
{
    GetTopLevelSeqEntryAndProcessor();
    m_FeatureType->SetFieldName("Any");
    m_Extend5Btn->SetValue(true);
    m_Extend3Btn->SetValue(true);
    CRef<CCmdComposite> cmd = GetCommand();
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);            
    }
    Destroy();
}

CRef<CCmdComposite> CExtendPartialFeatDlg::ExtendCDS(CSeq_entry_Handle seh)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Extend CDS"));
    if (!seh)
        return cmd;
    CScope& scope = seh.GetScope();
    int offset = 1;
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(seh);
    for (CFeat_CI fi(seh, CSeqFeatData::eSubtype_cdregion); fi; ++fi)
    {   
            const CSeq_loc& loc = fi->GetLocation();
            CBioseq_Handle bsh = scope.GetBioseqHandle(loc);

            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->Assign(fi->GetOriginalFeature());
            bool extend_up = (loc.GetStart(eExtreme_Positional) <= 5) && loc.IsPartialStart(eExtreme_Positional);
            bool extend_down = (bsh.GetInst_Length() - 1 <= loc.GetStop(eExtreme_Positional) + 5) && loc.IsPartialStop(eExtreme_Positional);
            if (extend_up) {
                if (loc.IsSetStrand() && loc.GetStrand() == eNa_strand_minus) {
                    edit::CLocationEditPolicy::Extend3(*new_feat, scope);
                } else {
                    edit::CLocationEditPolicy::Extend5(*new_feat, scope);
                }
            }
            if (extend_down) {
                if (loc.IsSetStrand() && loc.GetStrand() == eNa_strand_minus) {
                    edit::CLocationEditPolicy::Extend5(*new_feat, scope);
                } else {
                    edit::CLocationEditPolicy::Extend3(*new_feat, scope);
                }
            }
            if (extend_up || extend_down)
            {
                cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fi->GetSeq_feat_Handle(), *new_feat)));
                cmd->AddCommand(*GetRetranslateCDSCommand(scope, *new_feat, offset, create_general_only));
            }
    }
    return cmd;        
}

END_NCBI_SCOPE

