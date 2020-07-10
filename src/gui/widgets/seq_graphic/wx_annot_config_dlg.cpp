/*  $Id: wx_annot_config_dlg.cpp 30962 2014-08-13 18:38:46Z falkrb $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:
 *
 * File Description:
 *
 */
#include <ncbi_pch.hpp>
#include <gui/widgets/wx/dialog_utils.hpp>
#include <gui/widgets/wx/color_picker_validator.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/bitmap.h>
#include <wx/icon.h>

////@begin includes
////@end includes

#include "wx_annot_config_dlg.hpp"
#include <gui/objutils/label.hpp>
#include <gui/objutils/utils.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


IMPLEMENT_DYNAMIC_CLASS( CwxAnnotConfigDlg, wxDialog )

BEGIN_EVENT_TABLE( CwxAnnotConfigDlg, wxDialog )

////@begin CwxAnnotConfigDlg event table entries
    EVT_LISTBOX( ID_LISTBOX1, CwxAnnotConfigDlg::OnAnnotBrowserSelected )

    EVT_CHECKBOX( ID_CHECKBOX1, CwxAnnotConfigDlg::OnSmearAlnClicked )

    EVT_BUTTON( wxID_OK, CwxAnnotConfigDlg::OnOkClick )

////@end CwxAnnotConfigDlg event table entries

END_EVENT_TABLE()

CwxAnnotConfigDlg::CwxAnnotConfigDlg()
{
    Init();
}

CwxAnnotConfigDlg::CwxAnnotConfigDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CwxAnnotConfigDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CwxAnnotConfigDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS|wxDIALOG_EX_CONTEXTHELP);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CwxAnnotConfigDlg creation
    return true;
}

CwxAnnotConfigDlg::~CwxAnnotConfigDlg()
{
////@begin CwxAnnotConfigDlg destruction
////@end CwxAnnotConfigDlg destruction
}

void CwxAnnotConfigDlg::Init()
{
////@begin CwxAnnotConfigDlg member initialisation
    m_AnnotBrowser = NULL;
    m_SmearAln = NULL;
    m_ShowLabel = NULL;
    m_AnnotColorPicker = NULL;
////@end CwxAnnotConfigDlg member initialisation
}

void CwxAnnotConfigDlg::CreateControls()
{
////@begin CwxAnnotConfigDlg content construction
    CwxAnnotConfigDlg* itemDialog1 = this;

    wxFlexGridSizer* itemFlexGridSizer2 = new wxFlexGridSizer(2, 2, 0, 0);
    itemDialog1->SetSizer(itemFlexGridSizer2);

    wxArrayString m_AnnotBrowserStrings;
    m_AnnotBrowser = new wxListBox( itemDialog1, ID_LISTBOX1, wxDefaultPosition, wxSize(250, 320), m_AnnotBrowserStrings, wxLB_SINGLE );
    itemFlexGridSizer2->Add(m_AnnotBrowser, 0, wxGROW|wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Options"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    itemFlexGridSizer2->Add(itemStaticBoxSizer4, 0, wxGROW|wxALIGN_TOP|wxALL, 5);

    m_SmearAln = new wxCheckBox( itemDialog1, ID_CHECKBOX1, _("Smear Alignments"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SmearAln->SetValue(false);
    itemStaticBoxSizer4->Add(m_SmearAln, 0, wxALIGN_LEFT|wxALL, 5);

    m_ShowLabel = new wxCheckBox( itemDialog1, ID_CHECKBOX2, _("Show Labels"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ShowLabel->SetValue(false);
    itemStaticBoxSizer4->Add(m_ShowLabel, 0, wxALIGN_LEFT|wxALL, 5);

    m_AnnotColorPicker = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer4->Add(m_AnnotColorPicker, 0, wxALIGN_LEFT|wxALL, 0);

    wxColourPickerCtrl* itemColourPickerCtrl8 = new wxColourPickerCtrl( itemDialog1, ID_COLOURPICKERCTRL );
    m_AnnotColorPicker->Add(itemColourPickerCtrl8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemDialog1, wxID_STATIC, _("Annotation Background Color"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AnnotColorPicker->Add(itemStaticText9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    itemFlexGridSizer2->Add(5, 5, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer11 = new wxStdDialogButtonSizer;

    itemFlexGridSizer2->Add(itemStdDialogButtonSizer11, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxButton* itemButton12 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton12->SetDefault();
    itemStdDialogButtonSizer11->AddButton(itemButton12);

    wxButton* itemButton13 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer11->AddButton(itemButton13);

    itemStdDialogButtonSizer11->Realize();

    itemFlexGridSizer2->AddGrowableRow(0);
    itemFlexGridSizer2->AddGrowableCol(0);

    // Set validators
    itemColourPickerCtrl8->SetValidator( CColorPickerValidator(& m_AnnotBgColor) );
////@end CwxAnnotConfigDlg content construction
}


void CwxAnnotConfigDlg::OnOkClick( wxCommandEvent& event )
{
    x_SaveWidgetValues();
    m_AnnotConfig.SaveConfig();
    event.Skip();
}



void CwxAnnotConfigDlg::SetBioseq(const objects::CBioseq_Handle& bsh)
{
    x_InitAnnotList(bsh);
    x_AnnotBrowserSelectionChanged();
}


bool CwxAnnotConfigDlg::x_GetSelectedAnnotName(string& name)
{
    int sel_idx = m_AnnotBrowser->GetSelection();
    if (sel_idx == wxNOT_FOUND) {
        return false;
    }

    wxString browser_text = m_AnnotBrowser->GetStringSelection();

    string the_state = ToStdString(browser_text);
    name.swap(the_state);
    return true;
}


CRef<objects::CSeq_annot> CwxAnnotConfigDlg::x_FindAnnot(const string& label)
{
    TAnnotMap::iterator am_it = m_LabelAnnots.find(label);
    if ( am_it == m_LabelAnnots.end()) {
        return CRef<objects::CSeq_annot>();
    }
    return am_it->second;
}


void CwxAnnotConfigDlg::x_InitAnnotList(const CBioseq_Handle& bsh)
{
    m_LabelAnnots.clear();

    set<CSeq_annot_Handle> annots;
    SAnnotSelector sel = CSeqUtils::GetAnnotSelector(CSeq_annot::TData::e_Align);
    CAlign_CI align_iter(bsh, sel);
    for ( ;  align_iter;  ++align_iter) {
        annots.insert(align_iter.GetSeq_align_Handle().GetAnnot());
    }

    ITERATE (set<CSeq_annot_Handle>, iter, annots) {
        const CSeq_annot_Handle& sah = *iter;
        // CSeq_annot_EditHandle saeh = sah.GetEditHandle();
        CConstRef<CSeq_annot> sannot_cref = sah.GetCompleteSeq_annot();
        /*****
            Following const_cast is a kludge to get around the
            lack of accessors in CSeq_annot_EditHandle.
        *****/
        CRef<CSeq_annot> sannot_ref( & const_cast<CSeq_annot&> (*sannot_cref) );

        string label;
        CLabel::GetLabel(*sannot_ref, &label, CLabel::eContent);
        m_LabelAnnots[label] = sannot_ref;

        m_AnnotBrowser->Append(ToWxString(label));
    }
    m_AnnotBrowser->SetSelection(0);
}


bool CwxAnnotConfigDlg::ShowToolTips()
{
    return true;
}


wxBitmap CwxAnnotConfigDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CwxAnnotConfigDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CwxAnnotConfigDlg bitmap retrieval
}
wxIcon CwxAnnotConfigDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CwxAnnotConfigDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CwxAnnotConfigDlg icon retrieval
}


void CwxAnnotConfigDlg::x_SaveWidgetValues()
{
    CAnnotWriteConfig::CAnnotConfigSettings settings;

    if (TransferDataFromWindow()) {
        settings.m_Smear = m_SmearAln->GetValue();
        settings.m_ShowLabel = m_ShowLabel->GetValue();
        settings.m_BGColor = m_AnnotBgColor;

        m_AnnotConfig.SetAnnotConfigSettings(settings);
    }
}


void CwxAnnotConfigDlg::x_LoadWidgetValues()
{
    CAnnotWriteConfig::CAnnotConfigSettings settings;
    m_AnnotConfig.GetAnnotConfigSettings(settings);

    m_SmearAln->SetValue(settings.m_Smear);
    m_ShowLabel->SetValue(settings.m_ShowLabel);
    m_AnnotBgColor = settings.m_BGColor;

    x_CheckControls();
}


void CwxAnnotConfigDlg::x_CheckControls()
{
    bool val = !m_SmearAln->GetValue();
    m_ShowLabel->Show(val);
    m_AnnotColorPicker->Show(val);
}

void CwxAnnotConfigDlg::x_AnnotBrowserSelectionChanged()
{
    string annot_name;
    if (x_GetSelectedAnnotName(annot_name)) {
        CRef<objects::CSeq_annot> annot = x_FindAnnot(annot_name);
        _ASSERT(annot);
        x_SaveWidgetValues();
        m_AnnotConfig.SetAnnot(*annot);
        x_LoadWidgetValues();
    }
}

void CwxAnnotConfigDlg::OnSmearAlnClicked( wxCommandEvent& event )
{
    x_CheckControls();
}

void CwxAnnotConfigDlg::OnAnnotBrowserSelected( wxCommandEvent& event )
{
    x_AnnotBrowserSelectionChanged();
}

END_NCBI_SCOPE



