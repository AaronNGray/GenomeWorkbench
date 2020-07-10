/*  $Id: tooltip_wnd.cpp 43716 2019-08-21 16:14:49Z katargir $
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
 * Authors:  
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/bitmap.h>
#include <wx/icon.h>

#include <gui/widgets/gl/tooltip_handler.hpp>
#include <gui/widgets/gl/tooltip_wnd.hpp>

BEGIN_NCBI_SCOPE

/*!
 * CTooltipWnd type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CTooltipWnd, wxFrame )


/*!
 * CTooltipWnd event table definition
 */

BEGIN_EVENT_TABLE( CTooltipWnd, wxFrame )
#ifndef __WXOSX_COCOA__
    EVT_MOUSE_EVENTS( CTooltipWnd::OnMouse )
#endif
END_EVENT_TABLE()


/*!
 * CTooltipWnd constructors
 */

 CTooltipWnd::CTooltipWnd() : m_Tooltip()
{
    Init();
}

CTooltipWnd::CTooltipWnd(CTooltip* tooltip) : m_Tooltip(tooltip)
{
    Init();
}

/*!
 * CTooltipWnd creator
 */

bool CTooltipWnd::Create(wxWindow* parent, 
                         wxWindowID id, 
                         const wxString& title,                         
                         const wxPoint& pos, 
                         const wxSize& size, 
                         long style)
{
    wxFrame::Create( parent, id, title, pos, size, style );

    this->SetBackgroundColour(wxColour(255, 0, 0));
    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Fit();
    return true;
}


/*!
 * CTooltipWnd destructor
 */

CTooltipWnd::~CTooltipWnd()
{
////@begin CTooltipWnd destruction
////@end CTooltipWnd destruction
}


/*!
 * Member initialisation
 */

void CTooltipWnd::Init()
{
////@begin CTooltipWnd member initialisation
    m_TooltipCtrl = NULL;
////@end CTooltipWnd member initialisation
}


/*!
 * Control creation for CTooltipWnd
 */

void CTooltipWnd::CreateControls()
{    
////@begin CTooltipWnd content construction
    CTooltipWnd* itemWin1 = this;

    this->SetBackgroundColour(wxColour(255, 255, 225));
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemWin1->SetSizer(itemBoxSizer2);

    wxPanel* itemPanel3 = new wxPanel( itemWin1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
    itemPanel3->SetBackgroundColour(wxColour(255, 255, 225));
    itemBoxSizer2->Add(itemPanel3, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemPanel3->SetSizer(itemBoxSizer4);

    m_TooltipCtrl = new wxStaticText( itemPanel3, wxID_STATIC, m_TooltipText, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(m_TooltipCtrl, 1, wxGROW|wxALL, 5);

////@end CTooltipWnd content construction
}


/*!
 * Should we show tooltips?
 */

bool CTooltipWnd::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CTooltipWnd::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CTooltipWnd bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CTooltipWnd bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CTooltipWnd::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CTooltipWnd icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CTooltipWnd icon retrieval
}


/*!
 * All mouse events event handler for ID_CTOOLTIPWND
 */

void CTooltipWnd::OnMouse( wxMouseEvent& event )
{
    if (m_Tooltip) {
        m_Tooltip->Hide();
        m_Tooltip = 0;
    }
}

END_NCBI_SCOPE
