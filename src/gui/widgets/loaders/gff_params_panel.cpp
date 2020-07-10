/*  $Id: gff_params_panel.cpp 39640 2017-10-20 20:41:12Z katargir $
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

#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/radiobut.h>
#include <wx/valtext.h>
#include <wx/valgen.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/bitmap.h>
#include <wx/icon.h>

////@begin includes
////@end includes

#include <gui/widgets/loaders/gff_params_panel.hpp>
#include <gui/widgets/loaders/assembly_sel_panel.hpp>

BEGIN_NCBI_SCOPE

////@begin XPM images
////@end XPM images


/*!
 * CGffParamsPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CGffParamsPanel, wxPanel )


/*!
 * CGffParamsPanel event table definition
 */

BEGIN_EVENT_TABLE( CGffParamsPanel, wxPanel )

////@begin CGffParamsPanel event table entries
    EVT_RADIOBUTTON( ID_SEQ_ID_NORMAL, CGffParamsPanel::OnParseSeqIdNormalSelected )

    EVT_RADIOBUTTON( IDSEQ_ID_NUMERIC, CGffParamsPanel::OnParseSeqIdNumericSelected )

    EVT_RADIOBUTTON( ID_SEQ_ID_LOCAL, CGffParamsPanel::OnParseSeqIdLocalSelected )

////@end CGffParamsPanel event table entries

END_EVENT_TABLE()


/*!
 * CGffParamsPanel constructors
 */

CGffParamsPanel::CGffParamsPanel()
{
    Init();
}

CGffParamsPanel::CGffParamsPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CGFFParamsPanel creator
 */

bool CGffParamsPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CGffParamsPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CGffParamsPanel creation
    return true;
}


/*!
 * CGffParamsPanel destructor
 */

CGffParamsPanel::~CGffParamsPanel()
{
////@begin CGffParamsPanel destruction
////@end CGffParamsPanel destruction
}


/*!
 * Member initialisation
 */

void CGffParamsPanel::Init()
{
////@begin CGffParamsPanel member initialisation
    m_AssemblyPanel = NULL;
////@end CGffParamsPanel member initialisation
}


/*!
 * Control creation for CGFFParamsPanel
 */

void CGffParamsPanel::CreateControls()
{
////@begin CGffParamsPanel content construction
    CGffParamsPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemPanel1, wxID_ANY, _("GFF Load Parameters"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxALIGN_TOP|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(3, 2, 0, 0);
    itemStaticBoxSizer3->Add(itemFlexGridSizer5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("File Format"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString itemChoice7Strings;
    itemChoice7Strings.Add(_("Auto"));
    itemChoice7Strings.Add(_("GFF3"));
    itemChoice7Strings.Add(_("GTF"));
    itemChoice7Strings.Add(_("GVF"));
    wxChoice* itemChoice7 = new wxChoice( itemPanel1, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, itemChoice7Strings, 0 );
    itemChoice7->SetStringSelection(_("Auto"));
    itemFlexGridSizer5->Add(itemChoice7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("Name for feature set"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl9 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemTextCtrl9, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer10Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Sequence Identifiers"));
    wxStaticBoxSizer* itemStaticBoxSizer10 = new wxStaticBoxSizer(itemStaticBoxSizer10Static, wxVERTICAL);
    itemStaticBoxSizer3->Add(itemStaticBoxSizer10, 0, wxGROW|wxALL, 5);

    wxPanel* itemPanel11 = new wxPanel( itemPanel1, ID_PANEL, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemStaticBoxSizer10->Add(itemPanel11, 1, wxGROW, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxVERTICAL);
    itemPanel11->SetSizer(itemBoxSizer12);

    wxRadioButton* itemRadioButton13 = new wxRadioButton( itemPanel11, ID_SEQ_ID_NORMAL, _("Parse Normally"), wxDefaultPosition, wxDefaultSize, 0 );
    itemRadioButton13->SetValue(false);
    if (CGffParamsPanel::ShowToolTips())
        itemRadioButton13->SetToolTip(_("Sequence identifiers that \"look like\" GenBank identifiers will be interpreted as GenBank identifiers (numbers will be interpreted as GI values, text that matches accession formats will be interpreted as accessions), and all other sequence identifiers will be treated as \"local\" identifiers, which may have meaning to the creator of the sequence but do not refer to a sequence in the GenBank archive."));
    itemBoxSizer12->Add(itemRadioButton13, 0, wxALIGN_LEFT|wxALL, 5);

    wxRadioButton* itemRadioButton14 = new wxRadioButton( itemPanel11, IDSEQ_ID_NUMERIC, _("Numeric Identifiers as Local Identifiers"), wxDefaultPosition, wxDefaultSize, 0 );
    itemRadioButton14->SetValue(false);
    if (CGffParamsPanel::ShowToolTips())
        itemRadioButton14->SetToolTip(_("Only text that matches accession formats will be interpreted as accessions. Numbers will not be interpreted as GI values, but will be treated as \"local\" identifiers."));
    itemBoxSizer12->Add(itemRadioButton14, 0, wxALIGN_LEFT|wxALL, 5);

    wxRadioButton* itemRadioButton15 = new wxRadioButton( itemPanel11, ID_SEQ_ID_LOCAL, _("All Identifiers as Local Identifiers"), wxDefaultPosition, wxDefaultSize, 0 );
    itemRadioButton15->SetValue(false);
    if (CGffParamsPanel::ShowToolTips())
        itemRadioButton15->SetToolTip(_("No attempt will be made to interpret sequence identifiers as accessions that refer to sequences in the GenBank archive."));
    itemBoxSizer12->Add(itemRadioButton15, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText16 = new wxStaticText( itemPanel1, wxID_STATIC, _("Every sequence must have a sequence identifier to distinguish it\nfrom other sequences that are being viewed/analyzed.\nSequence identifiers can also be used to refer\nto a sequence in the GenBank archive.\n\nHover mouse pointer over an option for explanation."), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer10->Add(itemStaticText16, 0, wxALIGN_LEFT|wxALL, 5);

    m_AssemblyPanel = new CAssemblySelPanel( itemPanel1, ID_PANEL7, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_AssemblyPanel, 1, wxALIGN_TOP|wxALL, 5);

    // Set validators
    itemChoice7->SetValidator( wxGenericValidator(& GetData().m_FileFormat) );
    itemTextCtrl9->SetValidator( wxTextValidator(wxFILTER_NONE, & GetData().m_NameFeatSet) );
////@end CGffParamsPanel content construction

    m_AssemblyPanel->SetMinSize(wxSize(300, 250));
}


/*!
 * Should we show tooltips?
 */

bool CGffParamsPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CGffParamsPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CGffParamsPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CGffParamsPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CGffParamsPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CGffParamsPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CGffParamsPanel icon retrieval
}

/*!
 * Transfer data to the window
 */

bool CGffParamsPanel::TransferDataToWindow()
{
    x_SetParseSeqId(GetData().GetParseSeqIds());

    m_AssemblyPanel->SetData(GetData().GetMapAssembly());

    if (!wxPanel::TransferDataToWindow())
        return false;

    return true;
}

/*!
 * Transfer data from the window
 */

bool CGffParamsPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    GetData().SetMapAssembly(m_AssemblyPanel->GetData());

    return true;
}

void CGffParamsPanel::OnParseSeqIdNormalSelected(wxCommandEvent& event)
{
    x_SetParseSeqId(0);
}

void CGffParamsPanel::OnParseSeqIdNumericSelected(wxCommandEvent& event)
{
    x_SetParseSeqId(1);
}

void CGffParamsPanel::OnParseSeqIdLocalSelected( wxCommandEvent& event )
{
    x_SetParseSeqId(2);
}

void CGffParamsPanel::x_SetParseSeqId(int parseSeqId)
{
    int ctrlId;

    switch (parseSeqId) {
    case 1:
        ctrlId = IDSEQ_ID_NUMERIC;
        break;
    case 2:
        ctrlId = ID_SEQ_ID_LOCAL;
        break;
    default:
        ctrlId = ID_SEQ_ID_NORMAL;
        parseSeqId = 0;
        break;
    }

    for (auto id : { ID_SEQ_ID_NORMAL, IDSEQ_ID_NUMERIC, ID_SEQ_ID_LOCAL }) {
        bool val = (id == ctrlId);
        wxRadioButton* rb = dynamic_cast<wxRadioButton*>(FindWindow(id));
        if (rb && rb->GetValue() != val)
            rb->SetValue(val);
    }

    GetData().SetParseSeqIds(parseSeqId);
}

END_NCBI_SCOPE
