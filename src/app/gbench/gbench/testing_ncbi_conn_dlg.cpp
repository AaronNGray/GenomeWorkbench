/*  $Id: testing_ncbi_conn_dlg.cpp 23981 2011-06-30 15:09:19Z katargir $
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

#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/icon.h>

#include "testing_ncbi_conn_dlg.hpp"

#include <gui/utils/conn_test_thread.hpp>

////@begin XPM images
////@end XPM images


BEGIN_NCBI_SCOPE

/*!
 * CTestingNcbiConnDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CTestingNcbiConnDlg, wxDialog )


/*!
 * CTestingNcbiConnDlg event table definition
 */

BEGIN_EVENT_TABLE( CTestingNcbiConnDlg, wxDialog )

////@begin CTestingNcbiConnDlg event table entries
    EVT_BUTTON( ID_BUTTON2, CTestingNcbiConnDlg::OnCancelClick )

////@end CTestingNcbiConnDlg event table entries

    EVT_TIMER(-1, CTestingNcbiConnDlg::OnTimer)
END_EVENT_TABLE()


/*!
 * CTestingNcbiConnDlg constructors
 */

CTestingNcbiConnDlg::CTestingNcbiConnDlg()
: m_ConnThread(0), m_Timer(this)
{
    Init();
}

CTestingNcbiConnDlg::CTestingNcbiConnDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: m_ConnThread(0), m_Timer(this)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CTestingNcbiConnDlg creator
 */

bool CTestingNcbiConnDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CTestingNcbiConnDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CTestingNcbiConnDlg creation
    return true;
}


/*!
 * CTestingNcbiConnDlg destructor
 */

CTestingNcbiConnDlg::~CTestingNcbiConnDlg()
{
////@begin CTestingNcbiConnDlg destruction
////@end CTestingNcbiConnDlg destruction
}


/*!
 * Member initialisation
 */

void CTestingNcbiConnDlg::Init()
{
////@begin CTestingNcbiConnDlg member initialisation
////@end CTestingNcbiConnDlg member initialisation
}


/*!
 * Control creation for CTestingNcbiConnDlg
 */

void CTestingNcbiConnDlg::CreateControls()
{    
////@begin CTestingNcbiConnDlg content construction
    CTestingNcbiConnDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxTextCtrl* itemTextCtrl3 = new wxTextCtrl( itemDialog1, ID_TEXTCTRL2, _("Genome Workbench is trying to initialize and configure network access to NCBI servers.\nYou can interrupt this process, in this case you may have unstable connection to NCBI services.\n"), wxDefaultPosition, wxSize(300, 100), wxTE_MULTILINE|wxTE_READONLY );
    itemBoxSizer2->Add(itemTextCtrl3, 1, wxGROW|wxALL, 5);

    wxButton* itemButton4 = new wxButton( itemDialog1, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemButton4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CTestingNcbiConnDlg content construction

    m_Timer.Start(300);
}


/*!
 * Should we show tooltips?
 */

bool CTestingNcbiConnDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CTestingNcbiConnDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CTestingNcbiConnDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CTestingNcbiConnDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CTestingNcbiConnDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CTestingNcbiConnDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CTestingNcbiConnDlg icon retrieval
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
 */

void CTestingNcbiConnDlg::OnCancelClick( wxCommandEvent& WXUNUSED(event) )
{
    _ASSERT(m_ConnThread);
    m_ConnThread->RequestCancel();
    FindWindow(ID_BUTTON2)->Enable(false);
    EndModal(wxID_CANCEL);
}

void CTestingNcbiConnDlg::OnTimer( wxTimerEvent& WXUNUSED(event) )
{
    if (m_ConnThread->Finished()) {
        m_Timer.Stop();
        if (FindWindow(ID_BUTTON2)->IsEnabled())
            EndModal(wxID_OK);
        else
            EndModal(wxID_CANCEL);
    }
}


END_NCBI_SCOPE
