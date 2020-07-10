/*  $Id: select_feature_dlg.cpp 31553 2014-10-22 16:28:19Z katargir $
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
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>
////@begin includes
////@end includes

#include "select_feature_dlg.hpp"

#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/feat_ci.hpp>
#include <objtools/data_loaders/genbank/gbloader.hpp>
#include <gui/objutils/label.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/objects.hpp>
#include <gui/utils/command_processor.hpp>
#include <gui/widgets/edit/edit_object.hpp>
#include <gui/widgets/edit/edit_object_dlg.hpp>

#include <serial/typeinfo.hpp>
#include <serial/objistr.hpp>
#include <serial/objostr.hpp>
#include <serial/serial.hpp>

#include <objects/seq/Bioseq.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>

BEGIN_NCBI_SCOPE

////@begin XPM images
////@end XPM images


/*!
 * CSelectFeatureDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSelectFeatureDlg, wxDialog )


/*!
 * CSelectFeatureDlg event table definition
 */

BEGIN_EVENT_TABLE( CSelectFeatureDlg, wxDialog )

////@begin CSelectFeatureDlg event table entries
    EVT_CHOICE( ID_CHOICE5, CSelectFeatureDlg::OnFeatureTypeSelected )

    EVT_LISTBOX( ID_LISTBOX2, CSelectFeatureDlg::OnFeatureSelected )
    EVT_LISTBOX_DCLICK( ID_LISTBOX2, CSelectFeatureDlg::OnFeatureDoubleClicked )

    EVT_BUTTON( ID_BUTTON2, CSelectFeatureDlg::OnFeatureEdit )

    EVT_BUTTON( ID_BUTTON3, CSelectFeatureDlg::OnButtonLoadFile )

    EVT_BUTTON( ID_BUTTON4, CSelectFeatureDlg::OnButtonSaveFile )

    EVT_BUTTON( ID_BUTTON1, CSelectFeatureDlg::OnLoadAccession )

////@end CSelectFeatureDlg event table entries

END_EVENT_TABLE()


/*!
 * CSelectFeatureDlg constructors
 */

CSelectFeatureDlg::CSelectFeatureDlg()
{
    Init();
}

CSelectFeatureDlg::CSelectFeatureDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CSelectFeatureDlg creator
 */

bool CSelectFeatureDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSelectFeatureDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSelectFeatureDlg creation
    return true;
}


/*!
 * CSelectFeatureDlg destructor
 */

CSelectFeatureDlg::~CSelectFeatureDlg()
{
////@begin CSelectFeatureDlg destruction
////@end CSelectFeatureDlg destruction
}


/*!
 * Member initialisation
 */

void CSelectFeatureDlg::Init()
{
////@begin CSelectFeatureDlg member initialisation
    m_FeatureType = 0;
    m_FeatureList = NULL;
    m_LoadedStatus = NULL;
    m_EditFeatureBtn = NULL;
    m_SaveFileBtn = NULL;
    m_AccInput = NULL;
////@end CSelectFeatureDlg member initialisation

    m_ObjMgr = CObjectManager::GetInstance();
    CGBDataLoader::RegisterInObjectManager(*m_ObjMgr);

    m_Scope.Reset(new CScope(*m_ObjMgr));
    m_Scope->AddDefaults();
}


/*!
 * Control creation for CSelectFeatureDlg
 */

void CSelectFeatureDlg::CreateControls()
{
////@begin CSelectFeatureDlg content construction
    CSelectFeatureDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_RIGHT|wxALL, 2);

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Feature type"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString itemChoice5Strings;
    itemChoice5Strings.Add(_("Gene"));
    itemChoice5Strings.Add(_("RNA"));
    itemChoice5Strings.Add(_("mRNA"));
    itemChoice5Strings.Add(_("CD region"));
    wxChoice* itemChoice5 = new wxChoice( itemDialog1, ID_CHOICE5, wxDefaultPosition, wxDefaultSize, itemChoice5Strings, 0 );
    itemChoice5->SetStringSelection(_("Gene"));
    itemBoxSizer3->Add(itemChoice5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 1, wxGROW|wxALL, 2);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer6->Add(itemBoxSizer7, 1, wxGROW|wxALL, 0);

    wxArrayString m_FeatureListStrings;
    m_FeatureList = new wxListBox( itemDialog1, ID_LISTBOX2, wxDefaultPosition, wxSize(-1, itemDialog1->ConvertDialogToPixels(wxSize(-1, 150)).y), m_FeatureListStrings, wxLB_SINGLE|wxLB_HSCROLL );
    itemBoxSizer7->Add(m_FeatureList, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer7->Add(itemBoxSizer9, 0, wxGROW|wxALL, 0);

    m_LoadedStatus = new wxStaticText( itemDialog1, wxID_STATIC, _("Not loaded..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(m_LoadedStatus, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer9->Add(7, 8, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_EditFeatureBtn = new wxButton( itemDialog1, ID_BUTTON2, _("Edit Feature..."), wxDefaultPosition, wxDefaultSize, 0 );
    m_EditFeatureBtn->Enable(false);
    itemBoxSizer9->Add(m_EditFeatureBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticLine* itemStaticLine13 = new wxStaticLine( itemDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine13, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer14 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer14, 0, wxALIGN_RIGHT|wxALL, 0);

    wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer14->Add(itemBoxSizer15, 0, wxALIGN_RIGHT|wxALL, 2);

    wxButton* itemButton16 = new wxButton( itemDialog1, ID_BUTTON3, _("Load File..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(itemButton16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SaveFileBtn = new wxButton( itemDialog1, ID_BUTTON4, _("Save File..."), wxDefaultPosition, wxDefaultSize, 0 );
    m_SaveFileBtn->Enable(false);
    itemBoxSizer15->Add(m_SaveFileBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer18 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer14->Add(itemBoxSizer18, 0, wxALIGN_RIGHT|wxALL, 2);

    m_AccInput = new wxTextCtrl( itemDialog1, ID_TEXTCTRL7, _("NT_029998"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(m_AccInput, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton20 = new wxButton( itemDialog1, ID_BUTTON1, _("Load Accession"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemButton20, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton21 = new wxButton( itemDialog1, wxID_CANCEL, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemButton21, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CSelectFeatureDlg content construction
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE5
 */

void CSelectFeatureDlg::OnFeatureTypeSelected( wxCommandEvent& event )
{
    if (event.GetInt() != m_FeatureType) {
        m_FeatureType = event.GetInt();
        x_LoadFeatures();
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
 */

void CSelectFeatureDlg::OnLoadAccession( wxCommandEvent& )
{
    m_FeatureList->Clear();

    {{
        wxBusyCursor wait;

        CSeq_id id;

        string acc = ToStdString(m_AccInput->GetValue());
        try {
            id.Set(acc);
        }
        catch (CSeqIdException&) {
            string msg("Accession '");
            msg += acc + " not recognized as a valid accession";
            wxMessageBox(ToWxString(msg), wxT("Cannot load accesion"),
                         wxOK | wxICON_ERROR);
            m_LoadedStatus->SetLabel(wxT("Not loaded..."));
            m_SaveFileBtn->Enable(false);
            return;
        }

        m_BioseqHandle = m_Scope->GetBioseqHandle(id);
        string label;
        CLabel::GetLabel(id, &label, CLabel::eDefault, m_Scope.GetPointer());
        m_LoadedStatus->SetLabel(ToWxString("Accession: " + label));
        m_SaveFileBtn->Enable(true);
    }}

    x_LoadFeatures();
}


/*!
 * Should we show tooltips?
 */

bool CSelectFeatureDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSelectFeatureDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSelectFeatureDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSelectFeatureDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSelectFeatureDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSelectFeatureDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSelectFeatureDlg icon retrieval
}


/*!
 * wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX2
 */

void CSelectFeatureDlg::OnFeatureSelected( wxCommandEvent& )
{
    if (m_EditFeatureBtn) {
        m_EditFeatureBtn->Enable(m_FeatureList != 0 &&
                                 m_FeatureList->GetSelection() != wxNOT_FOUND);
    }
}

void CSelectFeatureDlg::x_LoadFeatures()
{
    if (m_EditFeatureBtn)
        m_EditFeatureBtn->Enable(false);

    if (m_FeatureList == 0)
        return;

    m_FeatureList->Clear();
    if (!m_BioseqHandle)
        return;

    SAnnotSelector sel;
    switch(m_FeatureType) {
        default:
            sel.SetFeatType(CSeqFeatData::e_Gene);
            break;
        case 1:
            sel.SetFeatType(CSeqFeatData::e_Rna);
            break;
        case 2:
            sel.SetFeatSubtype(CSeqFeatData::eSubtype_mRNA);
            break;
        case 3:
            sel.SetFeatType(CSeqFeatData::e_Cdregion);
            break;
    }

    m_FeatureList->Freeze();

    CFeat_CI it(m_BioseqHandle, sel);
    for ( ;  it;  ++it) {
        const CMappedFeat& mp_feat = *it;
        const CSeq_feat& feat = mp_feat.GetMappedFeature();

        string title;
        CLabel::GetLabel(feat, &title, CLabel::eUserTypeAndContent,
                         m_Scope.GetPointer());
        m_FeatureList->Append(ToWxString(title));
    }

    m_FeatureList->Thaw();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
 */

namespace {
    CSeq_feat_Handle sx_GetSeq_feat_Handle(CScope& scope, const CSeq_feat& feat)
    {
        SAnnotSelector sel(feat.GetData().GetSubtype());
        sel.SetResolveAll().SetNoMapping().SetSortOrder(sel.eSortOrder_None);
        for (CFeat_CI mf(scope, feat.GetLocation(), sel); mf; ++mf) {
            if (mf->GetOriginalFeature().Equals(feat)) {
                return mf->GetSeq_feat_Handle();
            }
        }
        return CSeq_feat_Handle();
    }
}

void CSelectFeatureDlg::OnFeatureEdit( wxCommandEvent& )
{
    int index = m_FeatureList->GetSelection();
    if (index == wxNOT_FOUND)
        return;

    SAnnotSelector sel;
    switch(m_FeatureType) {
        default:
            sel.SetFeatType(CSeqFeatData::e_Gene);
            break;
        case 1:
            sel.SetFeatType(CSeqFeatData::e_Rna);
            break;
        case 2:
            sel.SetFeatSubtype(CSeqFeatData::eSubtype_mRNA);
            break;
        case 3:
            sel.SetFeatType(CSeqFeatData::e_Cdregion);
            break;
    }

    CFeat_CI it(m_BioseqHandle, sel);
    for ( ;  it && index > 0;  ++it, --index);

    if (index != 0)
        return;

    const CMappedFeat& mp_feat = *it;
    const CSeq_feat& feat = mp_feat.GetMappedFeature();

    SConstScopedObject sso(feat, *m_Scope);
    CIRef<IEditObject>
        editor(CreateObjectInterface<IEditObject>(sso, NULL));
    if (!editor) {
        string type_name = feat.GetThisTypeInfo()->GetName();
        wxMessageBox(wxT("No editor found for object of type \'") +
                     ToWxString(type_name) +
                     wxT("\'."),
                     wxT("Error"), wxOK|wxICON_ERROR);
        return;
    }

    CEditObjectDlg edit_dlg(NULL);
    wxWindow* editorWindow = editor->CreateWindow(&edit_dlg);
    edit_dlg.SetEditorWindow(editorWindow);
    if (edit_dlg.ShowModal() != wxID_OK)
        return;

    CIRef<IEditCommand> edit_command(editor->GetEditCommand());
    if (!edit_command)
        wxMessageBox(wxT("Editor returned no action!"), wxT("Error"), wxOK|wxICON_ERROR);

    try {
        edit_command->Execute();
    }
    catch(const CException& e) {
        wxMessageBox(ToWxString(e.GetMsg()), wxT("Error"), wxOK|wxICON_ERROR);
        return;
    }

    index = m_FeatureList->GetSelection();
    x_LoadFeatures();
    m_FeatureList->SetSelection(index);
    m_EditFeatureBtn->Enable(m_FeatureList->GetSelection() != wxNOT_FOUND);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON4
 */

void CSelectFeatureDlg::OnButtonSaveFile( wxCommandEvent& )
{
    if (!m_BioseqHandle)
        return;

    CConstRef<CBioseq> bioseq = m_BioseqHandle.GetCompleteBioseq();

    const CSeq_id* pId = bioseq->GetFirstId();
    string defaultFile;
    if (pId) {
        CLabel::GetLabel(*pId, &defaultFile,
            CLabel::eDefault, m_Scope.GetPointer());
    }

    wxFileDialog dlg(this, wxT("Select a file"), wxEmptyString,
                     ToWxString(defaultFile),
                     wxT("ASN files (*.asn)|*.asn|All files (*.*)|*.*"),
                     wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

    if (dlg.ShowModal() != wxID_OK)
        return;

    string fileName = ToStdString(dlg.GetPath());
    if (fileName.empty())
        return;

    try {
        wxBusyCursor wait;
        auto_ptr<CObjectOStream> os(CObjectOStream::Open(eSerial_AsnText, fileName));
        *os << *bioseq;
    } catch(CException& e)  {
        LOG_POST(Error << e.ReportAll());
        wxString s = ToWxString(e.GetMsg());
        wxMessageBox(s, wxT("Error writing file."), wxOK | wxICON_ERROR);
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON3
 */

void CSelectFeatureDlg::OnButtonLoadFile( wxCommandEvent& )
{
    wxFileDialog dlg(this, wxT("Select a file"), wxEmptyString, wxEmptyString,
                     wxT("ASN files (*.asn)|*.asn|All files (*.*)|*.*"),
                     wxFD_OPEN);

    if (dlg.ShowModal() != wxID_OK)
        return;

    wxString fileName = dlg.GetPath();
    if (fileName.empty())
        return;

    CRef<CBioseq> bioseq(new CBioseq());

    try {
        wxBusyCursor wait;
        CNcbiIfstream istr(fileName.fn_str());
        auto_ptr<CObjectIStream> os(CObjectIStream::Open(eSerial_AsnText, istr));
        *os >> *bioseq;
    } catch(CException& e)  {
        LOG_POST(Error << e.ReportAll());
        wxString s = ToWxString(e.GetMsg());
        wxMessageBox(s, wxT("Error loading file."), wxOK | wxICON_ERROR);
        return;
    }

    m_FeatureList->Clear();
    m_BioseqHandle.Reset();
    m_Scope->ResetHistory();
    m_BioseqHandle = m_Scope->AddBioseq(*bioseq);
    m_LoadedStatus->SetLabel(ToWxString("File: ") + dlg.GetFilename());
    m_SaveFileBtn->Enable(true);

    x_LoadFeatures();
}

/*!
 * wxEVT_COMMAND_LISTBOX_DOUBLECLICKED event handler for ID_LISTBOX2
 */

void CSelectFeatureDlg::OnFeatureDoubleClicked( wxCommandEvent& event )
{
    OnFeatureEdit(event);
}

END_NCBI_SCOPE
