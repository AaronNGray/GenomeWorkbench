/*  $Id: single_experiment_panel.cpp 38073 2017-03-23 16:34:10Z filippov $
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
#include <objects/seqfeat/Gb_qual.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

////@begin includes
////@end includes

#include <gui/widgets/edit/single_experiment_panel.hpp> 

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CSingleExperimentPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSingleExperimentPanel, CFormattedQualPanel )


/*!
 * CSingleExperimentPanel event table definition
 */

BEGIN_EVENT_TABLE( CSingleExperimentPanel, CFormattedQualPanel )

////@begin CSingleExperimentPanel event table entries
    EVT_TEXT( ID_EXPERIMENT_TXT, CSingleExperimentPanel::OnExperimentTxtTextUpdated )
////@end CSingleExperimentPanel event table entries

END_EVENT_TABLE()


/*!
 * CSingleExperimentPanel constructors
 */

CSingleExperimentPanel::CSingleExperimentPanel()
{
    Init();
}

CSingleExperimentPanel::CSingleExperimentPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CSingleExperimentPanel creator
 */

bool CSingleExperimentPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSingleExperimentPanel creation
    CFormattedQualPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSingleExperimentPanel creation
    return true;
}


/*!
 * CSingleExperimentPanel destructor
 */

CSingleExperimentPanel::~CSingleExperimentPanel()
{
////@begin CSingleExperimentPanel destruction
////@end CSingleExperimentPanel destruction
}


/*!
 * Member initialisation
 */

void CSingleExperimentPanel::Init()
{
////@begin CSingleExperimentPanel member initialisation
    m_Category = NULL;
    m_Experiment = NULL;
    m_PMID = NULL;
////@end CSingleExperimentPanel member initialisation
}


/*!
 * Control creation for CSingleExperimentPanel
 */

void CSingleExperimentPanel::CreateControls()
{    
////@begin CSingleExperimentPanel content construction
    CSingleExperimentPanel* itemCFormattedQualPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemCFormattedQualPanel1->SetSizer(itemBoxSizer2);

    wxArrayString m_CategoryStrings;
    m_CategoryStrings.Add(wxEmptyString);
    m_CategoryStrings.Add(_("COORDINATES"));
    m_CategoryStrings.Add(_("DESCRIPTION"));
    m_CategoryStrings.Add(_("EXISTENCE"));
    m_Category = new wxChoice( itemCFormattedQualPanel1, ID_CHOICE18, wxDefaultPosition, wxDefaultSize, m_CategoryStrings, 0 );
    itemBoxSizer2->Add(m_Category, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_Experiment = new wxTextCtrl( itemCFormattedQualPanel1, ID_EXPERIMENT_TXT, wxEmptyString, wxDefaultPosition, wxSize(250, -1), 0 );
    itemBoxSizer2->Add(m_Experiment, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_PMID = new wxTextCtrl( itemCFormattedQualPanel1, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxSize(120, -1), 0 );
    itemBoxSizer2->Add(m_PMID, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

////@end CSingleExperimentPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CSingleExperimentPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSingleExperimentPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSingleExperimentPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSingleExperimentPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSingleExperimentPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSingleExperimentPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSingleExperimentPanel icon retrieval
}


void CSingleExperimentPanel::SetValue(string val)
{
    string category;
    string experiment;
    string doi;
    objects::CGb_qual::ParseExperiment(val, category, experiment, doi);
    if (NStr::IsBlank(category)) {
        m_Category->SetSelection(0);
    } else if (NStr::EqualNocase(category, "COORDINATES")) {
        m_Category->SetSelection(1);
    } else if (NStr::EqualNocase(category, "DESCRIPTION")) {
        m_Category->SetSelection(2);
    } else if (NStr::EqualNocase(category, "EXISTENCE")) {
        m_Category->SetSelection(3);
    }
    m_Experiment->SetValue(experiment);
    m_PMID->SetValue(doi);
}


string CSingleExperimentPanel::GetValue()
{
    string category = ToStdString(m_Category->GetStringSelection());
    string experiment = ToStdString(m_Experiment->GetValue());
    string doi = ToStdString(m_PMID->GetValue());

    string rval = objects::CGb_qual::BuildExperiment(category, experiment, doi);
    return rval;
}


void CSingleExperimentPanel::GetDimensions(int *width, int *height)
{
    *width = 10;
    *height = 10;
    int w = 0, h = 0;
    m_Category->GetSize(&w, &h);
    *height += h;
    *width += w + 5;

    m_Experiment->GetSize(&w, &h);
    *width += w + 5;

    m_PMID->GetSize(&w, &h);
    *width += w + 5;
}


/*!
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_EXPERIMENT_TXT
 */

void CSingleExperimentPanel::OnExperimentTxtTextUpdated( wxCommandEvent& event )
{
    NotifyParentOfChange();
}


END_NCBI_SCOPE

