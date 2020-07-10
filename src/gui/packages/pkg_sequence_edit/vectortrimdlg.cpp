/* $Id: vectortrimdlg.cpp 38626 2017-06-05 13:53:11Z asztalos $
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
 * Authors:  Yoon Choi
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <gui/packages/pkg_sequence_edit/vectortrimdlg.hpp>

#include <wx/button.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CVectorTrimDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CVectorTrimDlg, wxDialog )


/*!
 * CVectorTrimDlg event table definition
 */

BEGIN_EVENT_TABLE( CVectorTrimDlg, wxDialog )

////@begin CVectorTrimDlg event table entries
////@end CVectorTrimDlg event table entries

END_EVENT_TABLE()


/*!
 * CVectorTrimDlg constructors
 */

CVectorTrimDlg::CVectorTrimDlg()
{
    Init();
}

CVectorTrimDlg::CVectorTrimDlg( wxWindow* parent, objects::CSeq_entry_Handle seh, ICommandProccessor* cmdProcessor,
				wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_TopSeqEntry(seh), m_CmdProcessor(cmdProcessor)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CVectorTrimDlg creator
 */

bool CVectorTrimDlg::Create( wxWindow* parent, 
			     wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CVectorTrimDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CVectorTrimDlg creation

    SetSize(wxSize(920, 730));
    CentreOnParent();
    return true;
}


/*!
 * CVectorTrimDlg destructor
 */

CVectorTrimDlg::~CVectorTrimDlg()
{
////@begin CVectorTrimDlg destruction
////@end CVectorTrimDlg destruction
}


/*!
 * Member initialisation
 */

void CVectorTrimDlg::Init()
{
////@begin CVectorTrimDlg member initialisation
    m_PanelSizer = NULL;
////@end CVectorTrimDlg member initialisation
}


/*!
 * Control creation for CVectorTrimDlg
 */

void CVectorTrimDlg::CreateControls()
{    
////@begin CVectorTrimDlg content construction
    CVectorTrimDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_PanelSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(m_PanelSizer, 1, wxEXPAND|wxALL, 5);

#if 0
    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxEXPAND|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
#endif

////@end CVectorTrimDlg content construction

    m_Panel = new CVectorTrimPanel( itemDialog1, m_TopSeqEntry, m_CmdProcessor, wxID_ANY, 
				    wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER );
    m_Panel->TransferDataToWindow();
    m_PanelSizer->Add(m_Panel, 1, wxEXPAND|wxALL, 5);
}


/*!
 * Should we show tooltips?
 */

bool CVectorTrimDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CVectorTrimDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CVectorTrimDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CVectorTrimDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CVectorTrimDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CVectorTrimDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CVectorTrimDlg icon retrieval
}


END_NCBI_SCOPE
