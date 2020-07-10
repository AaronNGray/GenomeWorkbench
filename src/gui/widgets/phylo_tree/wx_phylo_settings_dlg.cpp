/*  $Id: wx_phylo_settings_dlg.cpp 38346 2017-04-27 13:24:57Z falkrb $
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
 * Authors: Vladimir Tereshkov
 *
 * File Description:
 *
 */
#include <ncbi_pch.hpp>

#include <corelib/ncbitime.hpp>
#include <corelib/ncbistr.hpp>


#include <gui/widgets/phylo_tree/wx_phylo_settings_dlg.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_label.hpp>

#include <gui/widgets/wx/dialog_utils.hpp>
#include <gui/widgets/wx/color_picker_validator.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <util/random_gen.hpp>

#include <wx/editlbox.h>



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
#include <wx/listbook.h>
#include <wx/radiobut.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/hyperlink.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CwxPhyloSettingsDlg, wxDialog )

BEGIN_EVENT_TABLE( CwxPhyloSettingsDlg, wxDialog )

////@begin CwxPhyloSettingsDlg event table entries
    EVT_NOTEBOOK_PAGE_CHANGED( ID_LISTBOOK1, CwxPhyloSettingsDlg::OnListbook1PageChanged )

    EVT_RADIOBOX( ID_COLORATION_RADIOBOX, CwxPhyloSettingsDlg::OnColorationRadioboxSelected )

    EVT_SLIDER( ID_SELECTION_ALPHA_SLIDER, CwxPhyloSettingsDlg::OnSelectionAlphaSliderUpdated )

    EVT_RADIOBUTTON( ID_RADIOBUTTON1, CwxPhyloSettingsDlg::OnRadiobutton1Selected )

    EVT_CHOICE( ID_CHOICE1, CwxPhyloSettingsDlg::OnChoice1Selected )

    EVT_RADIOBUTTON( ID_RADIOBUTTON2, CwxPhyloSettingsDlg::OnRadiobutton2Selected )

    EVT_BUTTON( ID_BUTTON1, CwxPhyloSettingsDlg::OnButton1Click )

    EVT_TEXT( ID_TEXTCTRL1, CwxPhyloSettingsDlg::OnTextctrl1Updated )

    EVT_CHECKBOX( ID_MAX_LABEL_LEN_CHECK, CwxPhyloSettingsDlg::OnMaxLabelLenCheckClick )

    EVT_BUTTON( ID_BUTTON2, CwxPhyloSettingsDlg::OnRandomizeButtonClick )

    EVT_CHOICE( ID_CHOICE, CwxPhyloSettingsDlg::OnFontFaceChoiceSelected )

    EVT_CHOICE( m_CHOICE, CwxPhyloSettingsDlg::OnFontSizeChoiceSelected )

    EVT_BUTTON( wxID_OK, CwxPhyloSettingsDlg::OnOkClick )

////@end CwxPhyloSettingsDlg event table entries

END_EVENT_TABLE()

CwxPhyloSettingsDlg::CwxPhyloSettingsDlg()
{
    Init();
}

CwxPhyloSettingsDlg::CwxPhyloSettingsDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: m_SampleNode(CPhyloTree::Null())
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CwxPhyloSettingsDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CwxPhyloSettingsDlg creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CwxPhyloSettingsDlg creation
    return true;
}

CwxPhyloSettingsDlg::~CwxPhyloSettingsDlg()
{
////@begin CwxPhyloSettingsDlg destruction
////@end CwxPhyloSettingsDlg destruction
}

void CwxPhyloSettingsDlg::Init()
{
////@begin CwxPhyloSettingsDlg member initialisation
    m_ResizableCollapsed = false;
    m_ColorPanel = NULL;
    m_Coloration = NULL;
    m_NodeColorDefault = NULL;
    m_EdgeColorDefault = NULL;
    m_LabelColorDefault = NULL;
    m_NodeColorSelection = NULL;
    m_EdgeColorSelection = NULL;
    m_LabelColorSelection = NULL;
    m_NodeColorTrace = NULL;
    m_EdgeColorTrace = NULL;
    m_LabelColorTrace = NULL;
    m_NodeColorCommon = NULL;
    m_EdgeColorCommon = NULL;
    m_LabelColorCommon = NULL;
    m_SelectionAlpha = NULL;
    m_SelectionAlphaText = NULL;
    m_rbSimple = NULL;
    m_Feature1 = NULL;
    m_rbBuilder = NULL;
    m_Feature2 = NULL;
    m_btnInsert = NULL;
    m_FormatString = NULL;
    m_SetMaxLabelLenCheck = NULL;
    m_MaxLabelLenValue = NULL;
    m_SampleLabel = NULL;
    m_FontFace = NULL;
    m_FontSize = NULL;
    m_HelpHyperLink = NULL;
////@end CwxPhyloSettingsDlg member initialisation
}

void CwxPhyloSettingsDlg::CreateControls()
{
////@begin CwxPhyloSettingsDlg content construction
    CwxPhyloSettingsDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(2, 1, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 1, wxGROW|wxALL, 5);

    wxNotebook* itemNotebook4 = new wxNotebook( itemDialog1, ID_LISTBOOK1, wxDefaultPosition, wxDefaultSize, wxBK_DEFAULT );

    m_ColorPanel = new wxPanel( itemNotebook4, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_ColorPanel->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxVERTICAL);
    m_ColorPanel->SetSizer(itemBoxSizer6);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer6->Add(itemBoxSizer7, 0, wxGROW|wxLEFT|wxRIGHT, 5);
    wxArrayString m_ColorationStrings;
    m_ColorationStrings.Add(_("&Use Color palette"));
    m_ColorationStrings.Add(_("&Perform coloration of tree clusters"));
    m_ColorationStrings.Add(_("&Use monochrome palette"));
    m_Coloration = new wxRadioBox( m_ColorPanel, ID_COLORATION_RADIOBOX, _("Coloration Options"), wxDefaultPosition, wxDefaultSize, m_ColorationStrings, 1, wxRA_SPECIFY_COLS );
    m_Coloration->SetSelection(0);
    itemBoxSizer7->Add(m_Coloration, 1, wxALIGN_TOP|wxLEFT|wxRIGHT, 5);

    wxStaticBox* itemStaticBoxSizer9Static = new wxStaticBox(m_ColorPanel, wxID_ANY, _("Color Matrix"));
    wxStaticBoxSizer* itemStaticBoxSizer9 = new wxStaticBoxSizer(itemStaticBoxSizer9Static, wxVERTICAL);
    itemBoxSizer6->Add(itemStaticBoxSizer9, 0, wxGROW, 5);
    wxFlexGridSizer* itemFlexGridSizer10 = new wxFlexGridSizer(5, 4, 2, 2);
    itemStaticBoxSizer9->Add(itemFlexGridSizer10, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT, 5);
    wxStaticText* itemStaticText11 = new wxStaticText( itemStaticBoxSizer9->GetStaticBox(), wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer10->Add(itemStaticText11, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText12 = new wxStaticText( itemStaticBoxSizer9->GetStaticBox(), wxID_STATIC, _("Node Color"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer10->Add(itemStaticText12, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText13 = new wxStaticText( itemStaticBoxSizer9->GetStaticBox(), wxID_STATIC, _("Edge Color"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer10->Add(itemStaticText13, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText14 = new wxStaticText( itemStaticBoxSizer9->GetStaticBox(), wxID_STATIC, _("Label Color"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer10->Add(itemStaticText14, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText15 = new wxStaticText( itemStaticBoxSizer9->GetStaticBox(), wxID_STATIC, _("Default"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer10->Add(itemStaticText15, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_NodeColorDefault = new wxColourPickerCtrl( itemStaticBoxSizer9->GetStaticBox(), ID_TEXTCTRL2, wxColour(), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemFlexGridSizer10->Add(m_NodeColorDefault, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_EdgeColorDefault = new wxColourPickerCtrl( itemStaticBoxSizer9->GetStaticBox(), ID_TEXTCTRL3, wxColour(), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemFlexGridSizer10->Add(m_EdgeColorDefault, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_LabelColorDefault = new wxColourPickerCtrl( itemStaticBoxSizer9->GetStaticBox(), ID_TEXTCTRL4, wxColour(), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemFlexGridSizer10->Add(m_LabelColorDefault, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText19 = new wxStaticText( itemStaticBoxSizer9->GetStaticBox(), ID_TEXTCTRL5, _("Selection"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer10->Add(itemStaticText19, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_NodeColorSelection = new wxColourPickerCtrl( itemStaticBoxSizer9->GetStaticBox(), ID_TEXTCTRL6, wxColour(), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemFlexGridSizer10->Add(m_NodeColorSelection, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_EdgeColorSelection = new wxColourPickerCtrl( itemStaticBoxSizer9->GetStaticBox(), ID_TEXTCTRL7, wxColour(), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemFlexGridSizer10->Add(m_EdgeColorSelection, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_LabelColorSelection = new wxColourPickerCtrl( itemStaticBoxSizer9->GetStaticBox(), ID_TEXTCTRL8, wxColour(), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemFlexGridSizer10->Add(m_LabelColorSelection, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText23 = new wxStaticText( itemStaticBoxSizer9->GetStaticBox(), ID_TEXTCTRL9, _("Trace to Root"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer10->Add(itemStaticText23, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_NodeColorTrace = new wxColourPickerCtrl( itemStaticBoxSizer9->GetStaticBox(), ID_TEXTCTRL10, wxColour(), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemFlexGridSizer10->Add(m_NodeColorTrace, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_EdgeColorTrace = new wxColourPickerCtrl( itemStaticBoxSizer9->GetStaticBox(), ID_TEXTCTRL11, wxColour(), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemFlexGridSizer10->Add(m_EdgeColorTrace, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_LabelColorTrace = new wxColourPickerCtrl( itemStaticBoxSizer9->GetStaticBox(), ID_TEXTCTRL12, wxColour(), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemFlexGridSizer10->Add(m_LabelColorTrace, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText27 = new wxStaticText( itemStaticBoxSizer9->GetStaticBox(), wxID_STATIC, _("Common Path"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer10->Add(itemStaticText27, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_NodeColorCommon = new wxColourPickerCtrl( itemStaticBoxSizer9->GetStaticBox(), ID_COLOURPICKERCTRL, wxColour(), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemFlexGridSizer10->Add(m_NodeColorCommon, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_EdgeColorCommon = new wxColourPickerCtrl( itemStaticBoxSizer9->GetStaticBox(), ID_COLOURPICKERCTRL1, wxColour(), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemFlexGridSizer10->Add(m_EdgeColorCommon, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_LabelColorCommon = new wxColourPickerCtrl( itemStaticBoxSizer9->GetStaticBox(), ID_COLOURPICKERCTRL2, wxColour(), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemFlexGridSizer10->Add(m_LabelColorCommon, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer31Static = new wxStaticBox(m_ColorPanel, wxID_ANY, _("Selection Options"));
    wxStaticBoxSizer* itemStaticBoxSizer31 = new wxStaticBoxSizer(itemStaticBoxSizer31Static, wxVERTICAL);
    itemBoxSizer6->Add(itemStaticBoxSizer31, 1, wxGROW|wxLEFT|wxRIGHT|wxTOP, 5);
    wxPanel* itemPanel32 = new wxPanel( itemStaticBoxSizer31->GetStaticBox(), ID_PANEL, wxDefaultPosition, wxSize(-1, 60), wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemStaticBoxSizer31->Add(itemPanel32, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxBoxSizer* itemBoxSizer33 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel32->SetSizer(itemBoxSizer33);

    wxStaticText* itemStaticText34 = new wxStaticText( itemPanel32, wxID_STATIC, _("Transparency factor when not selected"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer33->Add(itemStaticText34, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SelectionAlpha = new wxSlider( itemPanel32, ID_SELECTION_ALPHA_SLIDER, 10, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
    if (CwxPhyloSettingsDlg::ShowToolTips())
        m_SelectionAlpha->SetToolTip(_("Transparency value used when choosing to hide non-selected elements"));
    itemBoxSizer33->Add(m_SelectionAlpha, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SelectionAlphaText = new wxStaticText( itemPanel32, wxID_STATIC, _("0.1"), wxDefaultPosition, wxDefaultSize, 0 );
    if (CwxPhyloSettingsDlg::ShowToolTips())
        m_SelectionAlphaText->SetToolTip(_("Transparency value used when choosing to hide non-selected elements"));
    itemBoxSizer33->Add(m_SelectionAlphaText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 10);

    itemBoxSizer33->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5);

    itemNotebook4->AddPage(m_ColorPanel, _("Colors"));

    wxPanel* itemPanel38 = new wxPanel( itemNotebook4, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer39 = new wxBoxSizer(wxVERTICAL);
    itemPanel38->SetSizer(itemBoxSizer39);

    wxStaticBox* itemStaticBoxSizer40Static = new wxStaticBox(itemPanel38, wxID_ANY, _("Label Selection"));
    wxStaticBoxSizer* itemStaticBoxSizer40 = new wxStaticBoxSizer(itemStaticBoxSizer40Static, wxVERTICAL);
    itemBoxSizer39->Add(itemStaticBoxSizer40, 0, wxGROW|wxALL, 5);
    wxBoxSizer* itemBoxSizer41 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer40->Add(itemBoxSizer41, 0, wxALIGN_LEFT|wxALL, 5);
    m_rbSimple = new wxRadioButton( itemStaticBoxSizer40->GetStaticBox(), ID_RADIOBUTTON1, _("Simple Labels Using Property:"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_rbSimple->SetValue(true);
    itemBoxSizer41->Add(m_rbSimple, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);

    wxArrayString m_Feature1Strings;
    m_Feature1 = new wxChoice( itemStaticBoxSizer40->GetStaticBox(), ID_CHOICE1, wxDefaultPosition, wxDefaultSize, m_Feature1Strings, 0 );
    itemBoxSizer41->Add(m_Feature1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer44 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer40->Add(itemBoxSizer44, 0, wxALIGN_LEFT|wxALL, 5);
    m_rbBuilder = new wxRadioButton( itemStaticBoxSizer40->GetStaticBox(), ID_RADIOBUTTON2, _("Custom Labels Using Properties:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_rbBuilder->SetValue(false);
    itemBoxSizer44->Add(m_rbBuilder, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);

    wxArrayString m_Feature2Strings;
    m_Feature2 = new wxChoice( itemStaticBoxSizer40->GetStaticBox(), ID_CHOICE2, wxDefaultPosition, wxDefaultSize, m_Feature2Strings, 0 );
    itemBoxSizer44->Add(m_Feature2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_btnInsert = new wxButton( itemStaticBoxSizer40->GetStaticBox(), ID_BUTTON1, _("Insert"), wxDefaultPosition, wxSize(60, -1), 0 );
    itemBoxSizer44->Add(m_btnInsert, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);

    wxStaticText* itemStaticText48 = new wxStaticText( itemStaticBoxSizer40->GetStaticBox(), wxID_STATIC, _("Format String"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer40->Add(itemStaticText48, 0, wxALIGN_LEFT|wxALL, 5);

    m_FormatString = new wxTextCtrl( itemStaticBoxSizer40->GetStaticBox(), ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer40->Add(m_FormatString, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer50 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer40->Add(itemBoxSizer50, 0, wxALIGN_LEFT|wxALL, 5);
    m_SetMaxLabelLenCheck = new wxCheckBox( itemStaticBoxSizer40->GetStaticBox(), ID_MAX_LABEL_LEN_CHECK, _("Set Maximum Label Length"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SetMaxLabelLenCheck->SetValue(false);
    itemBoxSizer50->Add(m_SetMaxLabelLenCheck, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_MaxLabelLenValue = new wxTextCtrl( itemStaticBoxSizer40->GetStaticBox(), ID_MAX_LABEL_LEN_VALUE, _("-1"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT );
    itemBoxSizer50->Add(m_MaxLabelLenValue, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer53Static = new wxStaticBox(itemPanel38, wxID_ANY, _("Testing Area"));
    wxStaticBoxSizer* itemStaticBoxSizer53 = new wxStaticBoxSizer(itemStaticBoxSizer53Static, wxVERTICAL);
    itemBoxSizer39->Add(itemStaticBoxSizer53, 0, wxGROW|wxALL, 5);
    wxStaticText* itemStaticText54 = new wxStaticText( itemStaticBoxSizer53->GetStaticBox(), wxID_STATIC, _("Sample Label (random tree node)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer53->Add(itemStaticText54, 0, wxALIGN_LEFT|wxALL, 5);

    m_SampleLabel = new wxTextCtrl( itemStaticBoxSizer53->GetStaticBox(), ID_TEXTCTRL13, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer53->Add(m_SampleLabel, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer56 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer53->Add(itemBoxSizer56, 0, wxALIGN_LEFT|wxALL, 5);
    wxButton* itemButton57 = new wxButton( itemStaticBoxSizer53->GetStaticBox(), ID_BUTTON2, _("Randomize Node"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer56->Add(itemButton57, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook4->AddPage(itemPanel38, _("Labels"));

    wxPanel* itemPanel58 = new wxPanel( itemNotebook4, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer59 = new wxBoxSizer(wxVERTICAL);
    itemPanel58->SetSizer(itemBoxSizer59);

    wxStaticBox* itemStaticBoxSizer60Static = new wxStaticBox(itemPanel58, wxID_ANY, _("Margins"));
    wxStaticBoxSizer* itemStaticBoxSizer60 = new wxStaticBoxSizer(itemStaticBoxSizer60Static, wxVERTICAL);
    itemBoxSizer59->Add(itemStaticBoxSizer60, 0, wxGROW|wxALL, 5);
    wxGridSizer* itemGridSizer61 = new wxGridSizer(2, 2, 0, 0);
    itemStaticBoxSizer60->Add(itemGridSizer61, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxBoxSizer* itemBoxSizer62 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizer61->Add(itemBoxSizer62, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    wxStaticText* itemStaticText63 = new wxStaticText( itemStaticBoxSizer60->GetStaticBox(), wxID_STATIC, _("Left"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer62->Add(itemStaticText63, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl64 = new wxTextCtrl( itemStaticBoxSizer60->GetStaticBox(), ID_TEXTCTRL19, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer62->Add(itemTextCtrl64, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer65 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizer61->Add(itemBoxSizer65, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    wxStaticText* itemStaticText66 = new wxStaticText( itemStaticBoxSizer60->GetStaticBox(), wxID_STATIC, _("Top"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer65->Add(itemStaticText66, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl67 = new wxTextCtrl( itemStaticBoxSizer60->GetStaticBox(), ID_TEXTCTRL20, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer65->Add(itemTextCtrl67, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer68 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizer61->Add(itemBoxSizer68, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    wxStaticText* itemStaticText69 = new wxStaticText( itemStaticBoxSizer60->GetStaticBox(), wxID_STATIC, _("Bottom"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer68->Add(itemStaticText69, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl70 = new wxTextCtrl( itemStaticBoxSizer60->GetStaticBox(), ID_TEXTCTRL21, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer68->Add(itemTextCtrl70, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer71 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizer61->Add(itemBoxSizer71, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    wxStaticText* itemStaticText72 = new wxStaticText( itemStaticBoxSizer60->GetStaticBox(), wxID_STATIC, _("Right"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer71->Add(itemStaticText72, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl73 = new wxTextCtrl( itemStaticBoxSizer60->GetStaticBox(), ID_TEXTCTRL22, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer71->Add(itemTextCtrl73, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer74Static = new wxStaticBox(itemPanel58, wxID_ANY, _("Sizes"));
    wxStaticBoxSizer* itemStaticBoxSizer74 = new wxStaticBoxSizer(itemStaticBoxSizer74Static, wxVERTICAL);
    itemBoxSizer59->Add(itemStaticBoxSizer74, 0, wxGROW|wxALL, 5);
    wxGridSizer* itemGridSizer75 = new wxGridSizer(2, 2, 0, 0);
    itemStaticBoxSizer74->Add(itemGridSizer75, 0, wxGROW|wxALL, 5);
    wxBoxSizer* itemBoxSizer76 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizer75->Add(itemBoxSizer76, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    wxStaticText* itemStaticText77 = new wxStaticText( itemStaticBoxSizer74->GetStaticBox(), wxID_STATIC, _("Node Size"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer76->Add(itemStaticText77, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl78 = new wxTextCtrl( itemStaticBoxSizer74->GetStaticBox(), ID_TEXTCTRL17, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer76->Add(itemTextCtrl78, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer79 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizer75->Add(itemBoxSizer79, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    wxStaticText* itemStaticText80 = new wxStaticText( itemStaticBoxSizer74->GetStaticBox(), wxID_STATIC, _("Leaf Size"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer79->Add(itemStaticText80, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl81 = new wxTextCtrl( itemStaticBoxSizer74->GetStaticBox(), ID_TEXTCTRL23, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer79->Add(itemTextCtrl81, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer82 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizer75->Add(itemBoxSizer82, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    wxStaticText* itemStaticText83 = new wxStaticText( itemStaticBoxSizer74->GetStaticBox(), wxID_STATIC, _("Max Node Size"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer82->Add(itemStaticText83, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl84 = new wxTextCtrl( itemStaticBoxSizer74->GetStaticBox(), m_TEXTCTRL24, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer82->Add(itemTextCtrl84, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer85 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizer75->Add(itemBoxSizer85, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    wxStaticText* itemStaticText86 = new wxStaticText( itemStaticBoxSizer74->GetStaticBox(), wxID_STATIC, _("Line Width"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer85->Add(itemStaticText86, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl87 = new wxTextCtrl( itemStaticBoxSizer74->GetStaticBox(), ID_TEXTCTRL18, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer85->Add(itemTextCtrl87, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox88 = new wxCheckBox( itemStaticBoxSizer74->GetStaticBox(), ID_CHECKBOX, _("Resizable Collapsed Nodes"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox88->SetValue(false);
    itemStaticBoxSizer74->Add(itemCheckBox88, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer89Static = new wxStaticBox(itemPanel58, wxID_ANY, _("Font"));
    wxStaticBoxSizer* itemStaticBoxSizer89 = new wxStaticBoxSizer(itemStaticBoxSizer89Static, wxVERTICAL);
    itemBoxSizer59->Add(itemStaticBoxSizer89, 0, wxGROW|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer90 = new wxFlexGridSizer(0, 2, 0, 0);
    itemStaticBoxSizer89->Add(itemFlexGridSizer90, 0, wxGROW|wxALL, 5);
    wxStaticText* itemStaticText91 = new wxStaticText( itemStaticBoxSizer89->GetStaticBox(), wxID_STATIC, _("Font Face"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer90->Add(itemStaticText91, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_FontFaceStrings;
    m_FontFace = new wxChoice( itemStaticBoxSizer89->GetStaticBox(), ID_CHOICE, wxDefaultPosition, wxDefaultSize, m_FontFaceStrings, 0 );
    itemFlexGridSizer90->Add(m_FontFace, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText93 = new wxStaticText( itemStaticBoxSizer89->GetStaticBox(), wxID_STATIC, _("Font Size"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer90->Add(itemStaticText93, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_FontSizeStrings;
    m_FontSizeStrings.Add(_("10"));
    m_FontSize = new wxChoice( itemStaticBoxSizer89->GetStaticBox(), m_CHOICE, wxDefaultPosition, wxDefaultSize, m_FontSizeStrings, 0 );
    m_FontSize->SetStringSelection(_("10"));
    itemFlexGridSizer90->Add(m_FontSize, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook4->AddPage(itemPanel58, _("Sizes"));

    itemFlexGridSizer3->Add(itemNotebook4, 1, wxGROW|wxGROW|wxALL, 5);

    itemFlexGridSizer3->AddGrowableRow(0);
    itemFlexGridSizer3->AddGrowableCol(0);

    wxBoxSizer* itemBoxSizer95 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer95, 0, wxGROW|wxALL, 5);

    m_HelpHyperLink = new wxHyperlinkCtrl( itemDialog1, ID_HYPERLINKCTRL, _("Help"), _T("http://www.ncbi.nlm.nih.gov/tools/gbench/tutorial3A/#Ch1Step6"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    m_HelpHyperLink->Enable(false);
    itemBoxSizer95->Add(m_HelpHyperLink, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer97 = new wxStdDialogButtonSizer;

    itemBoxSizer95->Add(itemStdDialogButtonSizer97, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxButton* itemButton98 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer97->AddButton(itemButton98);

    wxButton* itemButton99 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer97->AddButton(itemButton99);

    itemStdDialogButtonSizer97->Realize();

    // Set validators
    m_NodeColorDefault->SetValidator( CColorPickerValidator(& m_DefNode) );
    m_EdgeColorDefault->SetValidator( CColorPickerValidator(& m_DefEdge) );
    m_LabelColorDefault->SetValidator( CColorPickerValidator(& m_DefLabel) );
    m_NodeColorSelection->SetValidator( CColorPickerValidator(& m_SelNode) );
    m_EdgeColorSelection->SetValidator( CColorPickerValidator(& m_SelEdge) );
    m_LabelColorSelection->SetValidator( CColorPickerValidator(& m_SelLabel) );
    m_NodeColorTrace->SetValidator( CColorPickerValidator(& m_TraceNode) );
    m_EdgeColorTrace->SetValidator( CColorPickerValidator(& m_TraceEdge) );
    m_LabelColorTrace->SetValidator( CColorPickerValidator(& m_TraceLabel) );
    m_NodeColorCommon->SetValidator( CColorPickerValidator(& m_CommonNode) );
    m_EdgeColorCommon->SetValidator( CColorPickerValidator(& m_CommonEdge) );
    m_LabelColorCommon->SetValidator( CColorPickerValidator(& m_CommonLabel) );
    itemTextCtrl64->SetValidator( wxGenericValidator(& m_MarginLeft) );
    itemTextCtrl67->SetValidator( wxGenericValidator(& m_MarginTop) );
    itemTextCtrl70->SetValidator( wxGenericValidator(& m_MarginBottom) );
    itemTextCtrl73->SetValidator( wxGenericValidator(& m_MarginRight) );
    itemTextCtrl78->SetValidator( wxGenericValidator(& m_NodeSize) );
    itemTextCtrl81->SetValidator( wxGenericValidator(& m_LeafNodeSize) );
    itemTextCtrl84->SetValidator( wxGenericValidator(& m_MaxNodeSize) );
    itemTextCtrl87->SetValidator( wxGenericValidator(& m_LineWidth) );
    itemCheckBox88->SetValidator( wxGenericValidator(& m_ResizableCollapsed)  );
////@end CwxPhyloSettingsDlg content construction  

     m_HelpHyperLink->Hide();
}

void CwxPhyloSettingsDlg::OnOkClick( wxCommandEvent& event )
{
    m_pSL->SetLabelFormat() =
        ( (m_rbSimple->GetValue())?
        ("$(" + ToStdString(m_Feature1->GetStringSelection())+")")
        : ToStdString(m_FormatString->GetValue()));

    if(TransferDataFromWindow()) {

        m_pSL->SetColor(CPhyloTreeScheme::eNode, CPhyloTreeScheme::eColor)
            = m_DefNode;
    
        m_pSL->SetColor(CPhyloTreeScheme::eNode, CPhyloTreeScheme::eSelColor)
            = m_SelNode;

        m_pSL->SetColor(CPhyloTreeScheme::eNode, CPhyloTreeScheme::eTraceColor)
            = m_TraceNode;

        m_pSL->SetColor(CPhyloTreeScheme::eNode, CPhyloTreeScheme::eSharedColor)
            = m_CommonNode;

        m_pSL->SetColor(CPhyloTreeScheme::eLine, CPhyloTreeScheme::eColor)
            = m_DefEdge;

        m_pSL->SetColor(CPhyloTreeScheme::eLine, CPhyloTreeScheme::eSelColor)
            = m_SelEdge;

        m_pSL->SetColor(CPhyloTreeScheme::eLine, CPhyloTreeScheme::eTraceColor)
            = m_TraceEdge;

        m_pSL->SetColor(CPhyloTreeScheme::eLine, CPhyloTreeScheme::eSharedColor)
            = m_CommonEdge;

        m_pSL->SetColor(CPhyloTreeScheme::eLabel, CPhyloTreeScheme::eColor)
            = m_DefLabel;

        m_pSL->SetColor(CPhyloTreeScheme::eLabel, CPhyloTreeScheme::eSelColor)
            = m_SelLabel;

        m_pSL->SetColor(CPhyloTreeScheme::eLabel, CPhyloTreeScheme::eTraceColor)
            = m_TraceLabel;

        m_pSL->SetColor(CPhyloTreeScheme::eLabel, CPhyloTreeScheme::eSharedColor)
            = m_CommonLabel;

        m_pSL->SetNonSelectedAlpha( ((float)m_SelectionAlpha->GetValue())/100.0f );

        if (m_SetMaxLabelLenCheck->GetValue()) {
            m_pSL->SetMaxLabelLength(NStr::StringToInt(ToStdString(m_MaxLabelLenValue->GetValue()), NStr::fConvErr_NoThrow));
        }
        else {
            m_pSL->SetMaxLabelLength(-1);
        }

        m_pSL->SetSize(CPhyloTreeScheme::eNodeSize) = m_NodeSize;
        m_pSL->SetSize(CPhyloTreeScheme::eMaxNodeSize) = m_MaxNodeSize;
        m_pSL->SetSize(CPhyloTreeScheme::eLeafNodeSize) = m_LeafNodeSize;
        m_pSL->SetSize(CPhyloTreeScheme::eLineWidth) = m_LineWidth;
        m_pSL->SetSize(CPhyloTreeScheme::eLeftMargin) = m_MarginLeft;
        m_pSL->SetSize(CPhyloTreeScheme::eRightMargin) = m_MarginRight;
        m_pSL->SetSize(CPhyloTreeScheme::eTopMargin) = m_MarginTop;
        m_pSL->SetSize(CPhyloTreeScheme::eBottomMargin) = m_MarginBottom;

		
		if (m_Coloration->GetSelection() ==  0) {
            m_pSL->SetColoration(CPhyloTreeScheme::eColored);
		}
        else if (m_Coloration->GetSelection() ==  1) {
			m_pSL->SetColoration(CPhyloTreeScheme::eClusters);
		}
        else  if (m_Coloration->GetSelection() ==  2) {
            m_pSL->SetColoration(CPhyloTreeScheme::eMonochrome);
        }

		CGlTextureFont font(CGlTextureFont::FaceFromString(ToStdString(m_FontFace->GetStringSelection())),
                            NStr::StringToNonNegativeInt((ToStdString(m_FontSize->GetStringSelection()))));

		m_pSL->SetFont(font);

        m_pSL->SetBoaNodes(m_ResizableCollapsed);

        m_pSL->SaveCurrentSettings();
    }

    event.Skip();
}

bool CwxPhyloSettingsDlg::ShowToolTips()
{
    return true;
}
wxBitmap CwxPhyloSettingsDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CwxPhyloSettingsDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CwxPhyloSettingsDlg bitmap retrieval
}
wxIcon CwxPhyloSettingsDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CwxPhyloSettingsDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CwxPhyloSettingsDlg icon retrieval
}

void CwxPhyloSettingsDlg::SetParams(CPhyloTreeDataSource* ds, CPhyloTreeScheme* sl)
{
    m_pDS = ds;
    m_pSL = sl;


    // Create map of disallowed font face/size combinations. These combinations 
    // look bad enough (dropped characters etc) that user would generally not
    // want to see them.
    vector<string> sizes1, sizes2, sizes3;
    sizes1.push_back("6");
    sizes2.push_back("7");
    sizes3.push_back("6");
    sizes3.push_back("7");

    m_InvalidFaceSizes["Helvetica Bold"] = sizes1;
    m_InvalidFaceSizes["Helvetica Oblique"] = sizes1;
    m_InvalidFaceSizes["Helvetica Bold Oblique"] = sizes1;

    m_InvalidFaceSizes["Lucida Bold"] = sizes1;
    m_InvalidFaceSizes["Lucida Italic"] = sizes1;
    m_InvalidFaceSizes["Lucida Bold Italic"] = sizes1;

    m_InvalidFaceSizes["Courier"] = sizes2;
    m_InvalidFaceSizes["Courier Oblique"] = sizes2;

    m_InvalidFaceSizes["Fixed"] = sizes2;
    m_InvalidFaceSizes["Fixed Bold"] = sizes2;
    m_InvalidFaceSizes["Fixed Oblique"] = sizes2;
    m_InvalidFaceSizes["Fixed Bold Oblique"] = sizes2;

    m_InvalidFaceSizes["Times-Roman"] = sizes1;
    m_InvalidFaceSizes["Times-Roman Bold"] = sizes1;
    m_InvalidFaceSizes["Times-Roman Italic"] = sizes1;
    m_InvalidFaceSizes["Times-Roman Bold Italic"] = sizes1;

    m_InvalidFaceSizes["BPMono"] = sizes2;
    m_InvalidFaceSizes["BPMono Bold"] = sizes2;
    m_InvalidFaceSizes["BPMono Bold Stencil"] = sizes2;

    m_InvalidFaceSizes["BPMono Italic"] = sizes3;


    if (sl->GetColoration()==CPhyloTreeScheme::eMonochrome) {
        m_Coloration->SetSelection(2);
    }
    else if (sl->GetColoration()==CPhyloTreeScheme::eClusters) {
        m_Coloration->SetSelection(1);
    }
    else {
        m_Coloration->SetSelection(0);
    }

    m_DefNode  = sl->SetColor(CPhyloTreeScheme::eNode,  CPhyloTreeScheme::eColor);
    m_DefEdge  = sl->SetColor(CPhyloTreeScheme::eLine,  CPhyloTreeScheme::eColor);
    m_DefLabel = sl->SetColor(CPhyloTreeScheme::eLabel, CPhyloTreeScheme::eColor);

    m_SelNode  = sl->SetColor(CPhyloTreeScheme::eNode,  CPhyloTreeScheme::eSelColor);
    m_SelEdge  = sl->SetColor(CPhyloTreeScheme::eLine,  CPhyloTreeScheme::eSelColor);
    m_SelLabel = sl->SetColor(CPhyloTreeScheme::eLabel, CPhyloTreeScheme::eSelColor);   

    m_TraceNode  = sl->SetColor(CPhyloTreeScheme::eNode,  CPhyloTreeScheme::eTraceColor);
    m_TraceEdge  = sl->SetColor(CPhyloTreeScheme::eLine,  CPhyloTreeScheme::eTraceColor);
    m_TraceLabel = sl->SetColor(CPhyloTreeScheme::eLabel, CPhyloTreeScheme::eTraceColor);   

    m_CommonNode  = sl->SetColor(CPhyloTreeScheme::eNode,  CPhyloTreeScheme::eSharedColor);
    m_CommonEdge  = sl->SetColor(CPhyloTreeScheme::eLine,  CPhyloTreeScheme::eSharedColor);
    m_CommonLabel = sl->SetColor(CPhyloTreeScheme::eLabel, CPhyloTreeScheme::eSharedColor); 

    float alpha = sl->GetNonSelectedAlpha();    
    m_SelectionAlpha->SetValue( (int)(alpha*100.0f) );
    wxCommandEvent dummy_evt;
    dummy_evt.SetInt(m_SelectionAlpha->GetValue());
    OnSelectionAlphaSliderUpdated(dummy_evt);      
    
    GLdouble left, top, right, bottom;
    sl->GetMargins(left, top, right, bottom);

    m_MarginLeft   = left;
    m_MarginTop    = top;
    m_MarginRight  = right;
    m_MarginBottom = bottom;  

    m_NodeSize  = sl->SetSize(CPhyloTreeScheme::eNodeSize);
    m_MaxNodeSize = sl->SetSize(CPhyloTreeScheme::eMaxNodeSize);
    m_LeafNodeSize = sl->SetSize(CPhyloTreeScheme::eLeafNodeSize);
    m_LineWidth = sl->SetSize(CPhyloTreeScheme::eLineWidth);
    m_ResizableCollapsed = sl->GetBoaNodes();

    const CGlTextureFont & font = sl->GetFont();

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

    m_FontFace->Clear();
    m_FontFace->Append(font_faces);
    m_FontSize->Clear();
    m_FontSize->Append(font_sizes);

	m_FontFace->SetStringSelection(ToWxString(CGlTextureFont::FaceToString(font.GetFontFace())));
    m_FontSize->SetStringSelection(ToWxString(NStr::NumericToString(font.GetFontSize())));

    // Make sure it is a valid face/size combination
    wxCommandEvent non_event;
    OnFontFaceChoiceSelected(non_event);

    // labels
    const   CBioTreeFeatureDictionary & dict = m_pDS->GetDictionary();

    int max_label_len = sl->GetMaxLabelLength();
    m_MaxLabelLenValue->SetValue(ToWxString(NStr::IntToString(max_label_len)));
    if (max_label_len > 0) {
        m_MaxLabelLenValue->Enable();
        m_SetMaxLabelLenCheck->SetValue(true);
    }
    else {
        m_MaxLabelLenValue->SetValue("-1");
        m_MaxLabelLenValue->Disable(); 
        m_SetMaxLabelLenCheck->SetValue(false);
    }

    // determining format type
    string & lblFormat =  sl->SetLabelFormat();
    string   smplLabel = "";
    m_FormatString->Clear();

    if (lblFormat.find_first_of("$") == lblFormat.find_last_of("$")) {
        m_rbSimple->SetValue(true);
       
        string::size_type pos1 = lblFormat.find_first_not_of("$(");
        string::size_type pos2 = lblFormat.find_last_not_of(")");
        if ( pos1!=string::npos &&  pos2!=string::npos && pos2>pos1){
            smplLabel = lblFormat.substr(pos1, pos2-pos1+1);
        }       
    }
    else {
         m_rbBuilder->SetValue(true);
    }
    m_FormatString->WriteText(ToWxString(lblFormat));

    int item = 0;
    ITERATE(CBioTreeFeatureDictionary::TFeatureDict, it, dict.GetFeatureDict()){
        wxString s = ToWxString(it->second);
        m_Feature1->Append(s);
        m_Feature2->Append(s);

        if (smplLabel == it->second) {
            m_Feature1->SetSelection(item);
        }
        item++;
    }
    m_Feature2->SetSelection(0);
    SwitchMode(m_rbSimple->GetValue());
    UpdateSample();
}


void CwxPhyloSettingsDlg::SwitchMode(bool bmode)
{
    m_Feature1->Enable(bmode);
    m_Feature2->Enable(!bmode);
    m_btnInsert->Enable(!bmode);
    m_FormatString->Enable(!bmode);
}

void CwxPhyloSettingsDlg::GetRandomNode()
{
    CPhyloTree& tree = *m_pDS->GetTree();
    m_SampleNode  = tree.GetRootIdx();

    CTime t;
    t.SetCurrent();
    CRandom r(t.MilliSecond());
    
    while (!tree[m_SampleNode].IsLeaf()) {
        int child_count = 0;

        CPhyloTree::TNodeList_I it = tree[m_SampleNode].SubNodeBegin();
        for(;it != tree[m_SampleNode].SubNodeEnd(); child_count++, it++ ) {}

        int sub_node_idx = r.GetRand(0,child_count-1);
        int idx = 0;
     
        for(it=tree[m_SampleNode].SubNodeBegin(); idx!=sub_node_idx; idx++, it++ ) {}

        m_SampleNode = *it;
    }
}

void CwxPhyloSettingsDlg::UpdateSample()
{
    if (!this->IsVisible())
        return;

    CPhyloTreeLabel lbl;
    const string  fmt( (m_rbSimple->GetValue())?
        ("$(" + ToStdString(m_Feature1->GetStringSelection())+")") :
        ToStdString(m_FormatString->GetValue()));

    if (m_SampleNode == CPhyloTree::Null())
        GetRandomNode();

    m_SampleLabel->Clear();
    string full_label = lbl.GetLabelForNode(*m_pDS->GetTree(),
                        (m_pDS->GetTree()->GetNode(m_SampleNode)), fmt);

    if (m_SetMaxLabelLenCheck->GetValue()) {
        int max_label_len = NStr::StringToInt(ToStdString(m_MaxLabelLenValue->GetValue()), NStr::fConvErr_NoThrow);

        if (max_label_len > 0 && int(full_label.length()) > max_label_len) {
            full_label = full_label.substr(0, max_label_len) + "...";
        }
    }

    m_SampleLabel->ChangeValue(ToWxString(full_label));
}


void CwxPhyloSettingsDlg::OnRadiobutton1Selected( wxCommandEvent& event )
{
    SwitchMode(true);
    UpdateSample();
    event.Skip();
}

void CwxPhyloSettingsDlg::OnRadiobutton2Selected( wxCommandEvent& event )
{
    SwitchMode(false);
    UpdateSample();
    event.Skip();
}

void CwxPhyloSettingsDlg::OnChoice1Selected( wxCommandEvent& event )
{
    UpdateSample();
    event.Skip();
}

void CwxPhyloSettingsDlg::OnTextctrl1Updated( wxCommandEvent& event )
{
    UpdateSample();
    event.Skip();
}

void CwxPhyloSettingsDlg::OnButton1Click( wxCommandEvent& event )
{
    wxString value = wxT("$(") + m_Feature2->GetStringSelection() + wxT(")");
    m_FormatString->WriteText(value);
    event.Skip();
}

void CwxPhyloSettingsDlg::OnColorationRadioboxSelected( wxCommandEvent& event )
{
    // Switch the color set we are editing (monochrome vs color)
    if (m_Coloration->GetSelection() == 0 || m_Coloration->GetSelection() == 1 ) {
        // colors for clustering are automatic, so include that with colored even though
        // the values won't be used.
        m_pSL->SetColoration(CPhyloTreeScheme::eColored);
        m_DefNode  = m_pSL->SetColor(CPhyloTreeScheme::eNode,  CPhyloTreeScheme::eColor);
        m_DefEdge  = m_pSL->SetColor(CPhyloTreeScheme::eLine,  CPhyloTreeScheme::eColor);
        m_DefLabel = m_pSL->SetColor(CPhyloTreeScheme::eLabel, CPhyloTreeScheme::eColor);

        m_SelNode  = m_pSL->SetColor(CPhyloTreeScheme::eNode,  CPhyloTreeScheme::eSelColor);
        m_SelEdge  = m_pSL->SetColor(CPhyloTreeScheme::eLine,  CPhyloTreeScheme::eSelColor);
        m_SelLabel = m_pSL->SetColor(CPhyloTreeScheme::eLabel, CPhyloTreeScheme::eSelColor);   

        m_TraceNode  = m_pSL->SetColor(CPhyloTreeScheme::eNode,  CPhyloTreeScheme::eTraceColor);
        m_TraceEdge  = m_pSL->SetColor(CPhyloTreeScheme::eLine,  CPhyloTreeScheme::eTraceColor);
        m_TraceLabel = m_pSL->SetColor(CPhyloTreeScheme::eLabel, CPhyloTreeScheme::eTraceColor);   

        m_CommonNode  = m_pSL->SetColor(CPhyloTreeScheme::eNode,  CPhyloTreeScheme::eSharedColor);
        m_CommonEdge  = m_pSL->SetColor(CPhyloTreeScheme::eLine,  CPhyloTreeScheme::eSharedColor);
        m_CommonLabel = m_pSL->SetColor(CPhyloTreeScheme::eLabel, CPhyloTreeScheme::eSharedColor); 		
	}
    else {
        m_pSL->SetColoration(CPhyloTreeScheme::eMonochrome);
        m_DefNode  = m_pSL->SetColor(CPhyloTreeScheme::eNode,  CPhyloTreeScheme::eColor);
        m_DefEdge  = m_pSL->SetColor(CPhyloTreeScheme::eLine,  CPhyloTreeScheme::eColor);
        m_DefLabel = m_pSL->SetColor(CPhyloTreeScheme::eLabel, CPhyloTreeScheme::eColor);

        m_SelNode  = m_pSL->SetColor(CPhyloTreeScheme::eNode,  CPhyloTreeScheme::eSelColor);
        m_SelEdge  = m_pSL->SetColor(CPhyloTreeScheme::eLine,  CPhyloTreeScheme::eSelColor);
        m_SelLabel = m_pSL->SetColor(CPhyloTreeScheme::eLabel, CPhyloTreeScheme::eSelColor);   

        m_TraceNode  = m_pSL->SetColor(CPhyloTreeScheme::eNode,  CPhyloTreeScheme::eTraceColor);
        m_TraceEdge  = m_pSL->SetColor(CPhyloTreeScheme::eLine,  CPhyloTreeScheme::eTraceColor);
        m_TraceLabel = m_pSL->SetColor(CPhyloTreeScheme::eLabel, CPhyloTreeScheme::eTraceColor);   

        m_CommonNode  = m_pSL->SetColor(CPhyloTreeScheme::eNode,  CPhyloTreeScheme::eSharedColor);
        m_CommonEdge  = m_pSL->SetColor(CPhyloTreeScheme::eLine,  CPhyloTreeScheme::eSharedColor);
        m_CommonLabel = m_pSL->SetColor(CPhyloTreeScheme::eLabel, CPhyloTreeScheme::eSharedColor); 	
    }

    m_ColorPanel->TransferDataToWindow();
 
    Refresh();

    event.Skip();
}


void CwxPhyloSettingsDlg::OnRandomizeButtonClick( wxCommandEvent& /* event */ )
{
    GetRandomNode();
    UpdateSample();
    Refresh();
}

void CwxPhyloSettingsDlg::OnSelectionAlphaSliderUpdated( wxCommandEvent& event )
{
    //wxGetApp().ShowHelp(_(""), this);
    double val = (double)(event.GetInt());
    val /= 100.0;

    string s = NStr::DoubleToString(val, 2);
    wxString wstr(s.c_str(), wxConvUTF8);

    m_SelectionAlphaText->SetLabel(wstr);
}

void CwxPhyloSettingsDlg::OnMaxLabelLenCheckClick( wxCommandEvent& event )
{
    if (event.IsChecked()) {
        m_MaxLabelLenValue->Enable();
    }
    else {
        m_MaxLabelLenValue->Disable(); 
    }
}

void CwxPhyloSettingsDlg::OnListbook1PageChanged( wxNotebookEvent& event )
{
    // Enable help only for those pages tha currently have it (and we should set the
    // help to a top-level help for all settings or update the url in this function when
    // more than one page gets help)
    if (m_HelpHyperLink == NULL) 
        return;

    if (event.GetSelection() != 3) {
        m_HelpHyperLink->Disable();
        m_HelpHyperLink->Hide();
    }
    else {
        m_HelpHyperLink->Show(true);
        m_HelpHyperLink->Enable();
        this->Layout();
    }

}

void CwxPhyloSettingsDlg::OnFontFaceChoiceSelected(wxCommandEvent& event)
{
    /// Fill font size choices
    wxArrayString font_sizes;

    std::vector<string> sizes;
    CGlTextureFont::GetAllSizes(sizes);

    size_t idx;
    string current_face = ToStdString(m_FontFace->GetStringSelection());
    string current_size = ToStdString(m_FontSize->GetStringSelection());

    if (m_InvalidFaceSizes.find(current_face) != m_InvalidFaceSizes.end())
    {
        vector<string> invalid_sizes = m_InvalidFaceSizes[current_face];
        bool increment_current_size = false;

        for (idx = 0; idx < sizes.size(); ++idx) {
            if (find(invalid_sizes.begin(), invalid_sizes.end(), sizes[idx]) == invalid_sizes.end()) {
                font_sizes.Add(ToWxString(sizes[idx]));
                if (increment_current_size) {
                    current_size = sizes[idx];
                    increment_current_size = false;
                }
            }
            else if (current_size == sizes[idx]) {
                increment_current_size = true;
            }
        }
    }
    else {        
        for (idx = 0; idx < sizes.size(); ++idx)
            font_sizes.Add(ToWxString(sizes[idx]));
    }

    m_FontSize->Clear();
    m_FontSize->Append(font_sizes);

    m_FontSize->SetStringSelection(ToWxString(current_size));
}

void CwxPhyloSettingsDlg::OnFontSizeChoiceSelected(wxCommandEvent& event)
{
    event.Skip();
}

END_NCBI_SCOPE


