/*  $Id: wx_histogram_dlg.cpp 30962 2014-08-13 18:38:46Z falkrb $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/hit_matrix/wx_histogram_dlg.hpp>
#include <gui/widgets/wx/wx_utils.hpp>


#include <wx/sizer.h>
#include <wx/clrpicker.h>
#include <wx/checklst.h>
#include <wx/stattext.h>


BEGIN_NCBI_SCOPE

typedef CHitMatrixRenderer::SGraphDescr TGraphDescr;

/*!
 * CwxHistogramDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CwxHistogramDlg, wxDialog )


/*!
 * CwxHistogramDlg event table definition
 */

BEGIN_EVENT_TABLE( CwxHistogramDlg, wxDialog )

////@begin CwxHistogramDlg event table entries
    EVT_LISTBOX( ID_SUBJECT_LIST, CwxHistogramDlg::OnSubjectListSelected )

    EVT_LISTBOX( ID_QUERY_LIST, CwxHistogramDlg::OnQueryListSelected )

    EVT_COLOURPICKER_CHANGED( ID_COLOURPICKERCTRL1, CwxHistogramDlg::OnColourpickerctrl1ColourPickerChanged )

////@end CwxHistogramDlg event table entries

END_EVENT_TABLE()


/*!
 * CwxHistogramDlg constructors
 */

CwxHistogramDlg::CwxHistogramDlg()
{
    Init();
}

CwxHistogramDlg::CwxHistogramDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CwxHistogramDlg creator
 */

bool CwxHistogramDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CwxHistogramDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
////@end CwxHistogramDlg creation
    return true;
}


/*!
 * CwxHistogramDlg destructor
 */

CwxHistogramDlg::~CwxHistogramDlg()
{
////@begin CwxHistogramDlg destruction
////@end CwxHistogramDlg destruction
}


/*!
 * Member initialisation
 */

void CwxHistogramDlg::Init()
{
////@begin CwxHistogramDlg member initialisation
    m_SubjectLabel = NULL;
    m_QueryLabel = NULL;
    m_SubjectList = NULL;
    m_QueryList = NULL;
    m_ColorBtn = NULL;
    m_ColorText = NULL;
////@end CwxHistogramDlg member initialisation
}


/*!
 * Control creation for CwxHistogramDlg
 */

void CwxHistogramDlg::CreateControls()
{
////@begin CwxHistogramDlg content construction
    CwxHistogramDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 1, wxGROW|wxALL, 5);

    m_SubjectLabel = new wxStaticText( itemDialog1, wxID_STATIC, _("Subject Graphs:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(m_SubjectLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 15);

    m_QueryLabel = new wxStaticText( itemDialog1, wxID_STATIC, _("Query Graphs:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(m_QueryLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 15);

    wxArrayString m_SubjectListStrings;
    m_SubjectList = new wxCheckListBox( itemDialog1, ID_SUBJECT_LIST, wxDefaultPosition, wxSize(220, 160), m_SubjectListStrings, wxLB_SINGLE );
    itemFlexGridSizer3->Add(m_SubjectList, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_QueryListStrings;
    m_QueryList = new wxCheckListBox( itemDialog1, ID_QUERY_LIST, wxDefaultPosition, wxSize(220, 160), m_QueryListStrings, wxLB_SINGLE );
    itemFlexGridSizer3->Add(m_QueryList, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer3->AddGrowableRow(1);
    itemFlexGridSizer3->AddGrowableCol(0);
    itemFlexGridSizer3->AddGrowableCol(1);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer8, 0, wxGROW|wxALL, 5);

    m_ColorBtn = new wxColourPickerCtrl( itemDialog1, ID_COLOURPICKERCTRL1 );
    itemBoxSizer8->Add(m_ColorBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ColorText = new wxStaticText( itemDialog1, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(m_ColorText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer8->Add(5, 5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer12 = new wxStdDialogButtonSizer;

    itemBoxSizer8->Add(itemStdDialogButtonSizer12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxButton* itemButton13 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer12->AddButton(itemButton13);

    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer12->AddButton(itemButton14);

    itemStdDialogButtonSizer12->Realize();

    itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM, 5);

////@end CwxHistogramDlg content construction
}


/*!
 * Should we show tooltips?
 */

bool CwxHistogramDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CwxHistogramDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CwxHistogramDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CwxHistogramDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CwxHistogramDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CwxHistogramDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CwxHistogramDlg icon retrieval
}



void CwxHistogramDlg::OnColourpickerctrl1ColourPickerChanged( wxColourPickerEvent& event )
{
    string name = m_GraphTypeName;
    NON_CONST_ITERATE(vector<TGraphDescr>, it, m_GraphTypes)  {
        if(it->m_Type == name)  {
            it->m_Color = ConvertColor(m_ColorBtn->GetColour());
        }
    }
    x_SelectGraph(name);
}

const static wxChar* kDisSubject = wxT("Subject : (data not available)");
const static wxChar* kDisQuery = wxT("Query : (data not available)");


void CwxHistogramDlg::SetItems(const vector<CHitMatrixRenderer::SGraphDescr>& graphs,
                             const vector<string>& s_sel,
                             const vector<string>& q_sel,
                             bool en_s, bool en_q)
{
    m_GraphTypes = graphs;

    x_FillList(m_SubjectList, s_sel);
    x_FillList(m_QueryList, q_sel);

    if( ! en_s)  {
        m_SubjectList->Enable(false);
        m_SubjectLabel->SetLabel(kDisSubject);
    }
    if( ! en_q)  {
        m_QueryList->Enable(false);
        m_QueryLabel->SetLabel(kDisQuery);
    }
}


void CwxHistogramDlg::x_FillList(wxCheckListBox* list, const vector<string>& items)
{
    list->Clear();

    for( size_t i = 0; i < m_GraphTypes.size(); i++ )  {
        const string& name = m_GraphTypes[i].m_Type;
        list->Append(ToWxString(name));

        if (std::find(items.begin(), items.end(), name) != items.end()) {
            list->Check((int)i, true);
        }
    }
}


void CwxHistogramDlg::GetSelected(vector<string>& s_items, vector<string>& q_items)
{
    int n = m_QueryList->GetCount();

    for( int i = 0; i < n; i++)    {
        if(m_SubjectList->IsChecked(i))   {
            string s = ToStdString(m_SubjectList->GetString(i));
            s_items.push_back(s);
        }
        if(m_QueryList->IsChecked(i))  {
            string s = ToStdString(m_QueryList->GetString(i));
            q_items.push_back(s);
        }
    }
}


void CwxHistogramDlg::x_SelectGraph(const string& name)
{
    ITERATE(vector<TGraphDescr>, it, m_GraphTypes)  {
        if(it->m_Type == name  &&  it->m_HasColor)  {
            m_ColorBtn->Enable(true);
            m_ColorBtn->SetColour(ConvertColor(it->m_Color));
            m_ColorText->SetLabel(ToWxString(it->m_Type));
            return;
        }
    }

    m_ColorBtn->SetColour(wxColour());
    m_ColorBtn->Enable(false);
    m_ColorText->SetLabel(wxT(""));
}


CRgbaColor CwxHistogramDlg::GetGraphColor(const string& name)
{
    ITERATE(vector<TGraphDescr>, it, m_GraphTypes)  {
        if(it->m_Type == name)  {
            return it->m_Color;
        }
    }
    return CRgbaColor();
}


/*!
 * wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_SUBJECT_LIST
 */

void CwxHistogramDlg::OnSubjectListSelected( wxCommandEvent& event )
{
    m_GraphTypeName = ToStdString(m_SubjectList->GetStringSelection());
    x_SelectGraph(m_GraphTypeName);
}


/*!
 * wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_QUERY_LIST
 */

void CwxHistogramDlg::OnQueryListSelected( wxCommandEvent& event )
{
    m_GraphTypeName = ToStdString(m_QueryList->GetStringSelection());
    x_SelectGraph(m_GraphTypeName);
}

END_NCBI_SCOPE


