/*  $Id: wx_aln_properties_dlg.cpp 40763 2018-04-10 16:08:12Z katargir $
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

////@begin includes
////@end includes

#include <gui/widgets/aln_multiple/wx_aln_properties_dlg.hpp>

#include <gui/widgets/wx/dialog_utils.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/color_picker_validator.hpp>

#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/statline.h>
#include <wx/statbox.h>
#include <wx/combobox.h>
#include <wx/sizer.h>
#include <wx/clrpicker.h>
#include <wx/checklst.h>
#include <wx/valgen.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/checkbox.h>

BEGIN_NCBI_SCOPE


IMPLEMENT_DYNAMIC_CLASS( CAlnPropertiesDlg, CDialog )

BEGIN_EVENT_TABLE( CAlnPropertiesDlg, CDialog )

////@begin CAlnPropertiesDlg event table entries
    EVT_BUTTON( wxID_OK, CAlnPropertiesDlg::OnOkClick )

////@end CAlnPropertiesDlg event table entries

END_EVENT_TABLE()

CAlnPropertiesDlg::CAlnPropertiesDlg()
{
    Init();
}

CAlnPropertiesDlg::CAlnPropertiesDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CAlnPropertiesDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAlnPropertiesDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAlnPropertiesDlg creation
    return true;
}

CAlnPropertiesDlg::~CAlnPropertiesDlg()
{
////@begin CAlnPropertiesDlg destruction
////@end CAlnPropertiesDlg destruction
}

void CAlnPropertiesDlg::Init()
{
    m_Style = NULL;

////@begin CAlnPropertiesDlg member initialisation
    m_VisColumnsList = NULL;
    m_TextFaceCombo = NULL;
    m_TextSizeCombo = NULL;
    m_SeqFaceCombo = NULL;
    m_SeqSizeCombo = NULL;
    m_ShowIdenticalBases = NULL;
    m_ShowConsensus = NULL;
////@end CAlnPropertiesDlg member initialisation
}

void CAlnPropertiesDlg::CreateControls()
{
////@begin CAlnPropertiesDlg content construction
    CAlnPropertiesDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxPanel* itemPanel4 = new wxPanel( itemCDialog1, ID_PANEL, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
    itemFlexGridSizer3->Add(itemPanel4, 1, wxGROW|wxGROW, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemPanel4->SetSizer(itemBoxSizer5);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer3->Add(itemFlexGridSizer6, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Visible Columns:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText7, 0, wxALIGN_RIGHT|wxALIGN_TOP|wxALL, 5);

    wxArrayString m_VisColumnsListStrings;
    m_VisColumnsList = new wxCheckListBox( itemCDialog1, ID_VIS_COLUMNS, wxDefaultPosition, wxSize(220, 130), m_VisColumnsListStrings, wxLB_SINGLE );
    itemFlexGridSizer6->Add(m_VisColumnsList, 3, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Text Font:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText9, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer6->Add(itemBoxSizer10, 1, wxGROW|wxALIGN_CENTER_VERTICAL, 5);

    wxArrayString m_TextFaceComboStrings;
    m_TextFaceCombo = new wxComboBox( itemCDialog1, ID_TEXT_FACE_COMBO, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_TextFaceComboStrings, wxCB_READONLY );
    itemBoxSizer10->Add(m_TextFaceCombo, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_TextSizeComboStrings;
    m_TextSizeCombo = new wxComboBox( itemCDialog1, ID_TEXT_SIZE_COMBO, wxEmptyString, wxDefaultPosition, wxSize(50, -1), m_TextSizeComboStrings, wxCB_READONLY );
    itemBoxSizer10->Add(m_TextSizeCombo, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText13 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Sequence Font:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText13, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer14 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer6->Add(itemBoxSizer14, 0, wxGROW|wxALIGN_CENTER_VERTICAL, 5);

    wxArrayString m_SeqFaceComboStrings;
    m_SeqFaceCombo = new wxComboBox( itemCDialog1, ID_SEQ_FACE_COMBO, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_SeqFaceComboStrings, wxCB_READONLY );
    itemBoxSizer14->Add(m_SeqFaceCombo, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_SeqSizeComboStrings;
    m_SeqSizeCombo = new wxComboBox( itemCDialog1, ID_FACE_SIZE_COMBO, wxEmptyString, wxDefaultPosition, wxSize(50, -1), m_SeqSizeComboStrings, wxCB_READONLY );
    itemBoxSizer14->Add(m_SeqSizeCombo, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText17 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Alignment:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText17, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ShowIdenticalBases = new wxCheckBox( itemCDialog1, ID_IDENTICAL_BASES_CHECKBOX, _("Show Identical Bases with Master Row"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ShowIdenticalBases->SetValue(false);
    m_ShowIdenticalBases->SetHelpText(_("If a master row is selected, show base letters instead of circle glyps"));
    if (CAlnPropertiesDlg::ShowToolTips())
        m_ShowIdenticalBases->SetToolTip(_("If a master row is selected, show base letters instead of circle glyps"));
    itemFlexGridSizer6->Add(m_ShowIdenticalBases, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText19 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Consensus:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText19, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ShowConsensus = new wxCheckBox( itemCDialog1, ID_CONSENSUS_CHECKBOX, _("Show When Available"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ShowConsensus->SetValue(false);
    m_ShowConsensus->SetHelpText(_("Show consensus for non-sparse alignments"));
    if (CAlnPropertiesDlg::ShowToolTips())
        m_ShowConsensus->SetToolTip(_("Show consensus for non-sparse alignments"));
    itemFlexGridSizer6->Add(m_ShowConsensus, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer6->AddGrowableCol(1);

    wxStaticLine* itemStaticLine21 = new wxStaticLine( itemCDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemFlexGridSizer3->Add(itemStaticLine21, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer3->Add(itemBoxSizer22, 0, wxGROW|wxALIGN_CENTER_VERTICAL, 5);

    wxFlexGridSizer* itemFlexGridSizer23 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer22->Add(itemFlexGridSizer23, 1, wxALIGN_TOP, 5);

    wxStaticText* itemStaticText24 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Text Color:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer23->Add(itemStaticText24, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxColourPickerCtrl* itemColourPickerCtrl25 = new wxColourPickerCtrl( itemCDialog1, ID_TEXT_COLOR, wxColour(0, 0, 0), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemFlexGridSizer23->Add(itemColourPickerCtrl25, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText26 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Back Color:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer23->Add(itemStaticText26, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxColourPickerCtrl* itemColourPickerCtrl27 = new wxColourPickerCtrl( itemCDialog1, ID_BACK_COLOR, wxColour(0, 0, 0), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemFlexGridSizer23->Add(itemColourPickerCtrl27, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText28 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Sequence Color:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer23->Add(itemStaticText28, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxColourPickerCtrl* itemColourPickerCtrl29 = new wxColourPickerCtrl( itemCDialog1, ID_SEQ_COLOR, wxColour(0, 0, 0), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemFlexGridSizer23->Add(itemColourPickerCtrl29, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText30 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Frame Color:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer23->Add(itemStaticText30, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxColourPickerCtrl* itemColourPickerCtrl31 = new wxColourPickerCtrl( itemCDialog1, ID_FRAME_COLOR, wxColour(0, 0, 0), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemFlexGridSizer23->Add(itemColourPickerCtrl31, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer23->AddGrowableCol(0);

    wxFlexGridSizer* itemFlexGridSizer32 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer22->Add(itemFlexGridSizer32, 1, wxALIGN_TOP, 5);

    wxStaticText* itemStaticText33 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Segments Color:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer32->Add(itemStaticText33, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxColourPickerCtrl* itemColourPickerCtrl34 = new wxColourPickerCtrl( itemCDialog1, ID_SEG_COLOR, wxColour(0, 0, 0), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemFlexGridSizer32->Add(itemColourPickerCtrl34, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText35 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Selected Text Color:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer32->Add(itemStaticText35, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxColourPickerCtrl* itemColourPickerCtrl36 = new wxColourPickerCtrl( itemCDialog1, ID_SEL_TEXT_COLOR, wxColour(0, 0, 0), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemFlexGridSizer32->Add(itemColourPickerCtrl36, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText37 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Selected Back Color:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer32->Add(itemStaticText37, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxColourPickerCtrl* itemColourPickerCtrl38 = new wxColourPickerCtrl( itemCDialog1, ID_SEL_BACK_COLOR, wxColour(0, 0, 0), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemFlexGridSizer32->Add(itemColourPickerCtrl38, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText39 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Focused Back Color:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer32->Add(itemStaticText39, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxColourPickerCtrl* itemColourPickerCtrl40 = new wxColourPickerCtrl( itemCDialog1, ID_FOCUSED_BACK_COLOR, wxColour(0, 0, 0), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemFlexGridSizer32->Add(itemColourPickerCtrl40, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer32->AddGrowableCol(0);

    wxStaticLine* itemStaticLine41 = new wxStaticLine( itemCDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemFlexGridSizer3->Add(itemStaticLine41, 0, wxALIGN_CENTER_HORIZONTAL|wxGROW|wxALL, 5);

    itemFlexGridSizer3->AddGrowableRow(0);
    itemFlexGridSizer3->AddGrowableCol(0);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer42 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer42, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton43 = new wxButton( itemCDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer42->AddButton(itemButton43);

    wxButton* itemButton44 = new wxButton( itemCDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer42->AddButton(itemButton44);

    itemStdDialogButtonSizer42->Realize();

    // Set validators
    m_VisColumnsList->SetValidator( wxGenericValidator(& m_VisColumns) );
    m_TextFaceCombo->SetValidator( wxGenericValidator(& m_TextFace) );
    m_TextSizeCombo->SetValidator( wxGenericValidator(& m_TextSize) );
    m_SeqFaceCombo->SetValidator( wxGenericValidator(& m_SeqFace) );
    m_SeqSizeCombo->SetValidator( wxGenericValidator(& m_SeqSize) );
    itemColourPickerCtrl25->SetValidator( CColorPickerValidator(& m_TextColor) );
    itemColourPickerCtrl27->SetValidator( CColorPickerValidator(& m_BackColor) );
    itemColourPickerCtrl29->SetValidator( CColorPickerValidator(& m_SeqColor) );
    itemColourPickerCtrl31->SetValidator( CColorPickerValidator(& m_FrameColor) );
    itemColourPickerCtrl34->SetValidator( CColorPickerValidator(& m_SegmentColor) );
    itemColourPickerCtrl36->SetValidator( CColorPickerValidator(& m_SelTextColor) );
    itemColourPickerCtrl38->SetValidator( CColorPickerValidator(& m_SelBackColor) );
    itemColourPickerCtrl40->SetValidator( CColorPickerValidator(& m_FocusedColor) );
////@end CAlnPropertiesDlg content construction
}

bool CAlnPropertiesDlg::ShowToolTips()
{
    return true;
}
wxBitmap CAlnPropertiesDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAlnPropertiesDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAlnPropertiesDlg bitmap retrieval
}
wxIcon CAlnPropertiesDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAlnPropertiesDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAlnPropertiesDlg icon retrieval
}


void CAlnPropertiesDlg::SetParams(CWidgetDisplayStyle& style)
{
    m_Style = &style;

    // check visible columns
    m_AllColumns.clear();
    m_VisColumns.Clear();

    size_t n = m_Style->m_Columns.size();
    for( size_t i = 0;  i < n;  i++ )  {
        CWidgetDisplayStyle::SColumn& col = m_Style->m_Columns[i];
        m_AllColumns.push_back(col.m_Name);
        if(col.m_Visible)   {
            m_VisColumns.Add((int)i);
        }
    }

    // initialize Columns Check List
    Init_wxControl(*m_VisColumnsList, m_AllColumns);

    vector<string> values;

    // Populate Font Face combos
    /// Fill font face and size choices
    wxArrayString font_faces;
    wxArrayString font_sizes;

    std::vector<string> faces;
    std::vector<string> sizes;

    CGlTextureFont::GetAllFaces(faces);
    CGlTextureFont::GetAllSizes(sizes);

    size_t idx;
    for (idx = 0; idx<faces.size(); ++idx)
        font_faces.Add(ToWxString(faces[idx]));
    for (idx = 0; idx<sizes.size(); ++idx)
        font_sizes.Add(ToWxString(sizes[idx]));

    m_TextFace = ToWxString(CGlTextureFont::FaceToString(m_Style->m_TextFont.GetFontFace()));
    m_TextSize = ToWxString(NStr::IntToString(m_Style->m_TextFont.GetFontSize()));

    m_SeqFace = ToWxString(CGlTextureFont::FaceToString(m_Style->m_SeqFont.GetFontFace()));
    m_SeqSize = ToWxString(NStr::IntToString(m_Style->m_SeqFont.GetFontSize()));

    m_TextFaceCombo->Clear();
    m_TextFaceCombo->Append(font_faces);
    m_TextFaceCombo->SetStringSelection(m_TextFace);

    m_SeqFaceCombo->Clear();
    m_SeqFaceCombo->Append(font_faces);
    m_SeqFaceCombo->SetStringSelection(m_SeqFace);

    m_TextSizeCombo->Clear();
    m_TextSizeCombo->Append(font_sizes);
    m_TextSizeCombo->SetStringSelection(m_TextSize);

    m_SeqSizeCombo->Clear();
    m_SeqSizeCombo->Append(font_sizes);
    m_SeqSizeCombo->SetStringSelection(m_SeqSize);

    m_ShowIdenticalBases->SetValue(m_Style->m_ShowIdenticalBases);
    m_ShowConsensus->SetValue(m_Style->m_ShowConsensus);

    m_TextColor = m_Style->GetColor(CWidgetDisplayStyle::eText);
    m_BackColor = m_Style->GetColor(CWidgetDisplayStyle::eBack);
    m_SelTextColor = m_Style->GetColor(CWidgetDisplayStyle::eSelectedText);
    m_SelBackColor = m_Style->GetColor(CWidgetDisplayStyle::eSelectedBack);
    m_FocusedColor = m_Style->GetColor(CWidgetDisplayStyle::eFocusedBack);
    m_FrameColor = m_Style->GetColor(CWidgetDisplayStyle::eFrame);
    m_SegmentColor = m_Style->GetColor(CWidgetDisplayStyle::eAlignSegs);
    m_SeqColor = m_Style->GetColor(CWidgetDisplayStyle::eSequence);
}


void CAlnPropertiesDlg::OnOkClick(wxCommandEvent& event)
{
    if(TransferDataFromWindow())    {
        CWidgetDisplayStyle::TColumns& columns = m_Style->m_Columns;

        // reset Visible by default
        NON_CONST_ITERATE(CWidgetDisplayStyle::TColumns, it, columns)   {
            it->m_Visible = false;
        }

        for(  size_t i = 0;  i < m_VisColumns.GetCount(); i++ )  {
            int index = m_VisColumns[i];
            m_Style->m_Columns[index].m_Visible = true;
        }

        CGlTextureFont::EFontFace ff = CGlTextureFont::FaceFromString(ToStdString(m_TextFace));
        unsigned int font_size = NStr::StringToInt(ToStdString(m_TextSize));
        m_Style->m_TextFont.SetFontFace(ff);
        m_Style->m_TextFont.SetFontSize(font_size);

        ff = CGlTextureFont::FaceFromString(ToStdString(m_SeqFace));
        font_size = NStr::StringToInt(ToStdString(m_SeqSize));
        m_Style->m_SeqFont.SetFontFace(ff);
        m_Style->m_SeqFont.SetFontSize(font_size);
        m_Style->m_ShowIdenticalBases = m_ShowIdenticalBases->GetValue();
        m_Style->m_ShowConsensus = m_ShowConsensus->GetValue();

        m_Style->SetColor(CWidgetDisplayStyle::eText, m_TextColor);
        m_Style->SetColor(CWidgetDisplayStyle::eBack, m_BackColor);
        m_Style->SetColor(CWidgetDisplayStyle::eSelectedText, m_SelTextColor);
        m_Style->SetColor(CWidgetDisplayStyle::eSelectedBack, m_SelBackColor);
        m_Style->SetColor(CWidgetDisplayStyle::eFrame, m_FrameColor);
        m_Style->SetColor(CWidgetDisplayStyle::eFocusedBack, m_FocusedColor);
        m_Style->SetColor(CWidgetDisplayStyle::eAlignSegs, m_SegmentColor);
        m_Style->SetColor(CWidgetDisplayStyle::eSequence, m_SeqColor);

        EndModal(wxID_OK);
    }
}


END_NCBI_SCOPE

