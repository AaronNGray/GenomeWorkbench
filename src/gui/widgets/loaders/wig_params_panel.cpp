/*  $Id: wig_params_panel.cpp 28586 2013-08-02 19:32:29Z katargir $
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
#include "wx/valgen.h"
#include "wx/valtext.h"
#include "wx/stattext.h"
#include "wx/statbox.h"
#include "wx/choice.h"
#include "wx/spinctrl.h"
#include "wx/bitmap.h"
#include "wx/icon.h"

#include <gui/widgets/loaders/wig_params_panel.hpp>
#include <gui/widgets/loaders/assembly_sel_panel.hpp>

BEGIN_NCBI_SCOPE

/*!
 * CWigParamsPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CWigParamsPanel, wxPanel )


/*!
 * CWigParamsPanel event table definition
 */

BEGIN_EVENT_TABLE( CWigParamsPanel, wxPanel )

////@begin CWigParamsPanel event table entries
////@end CWigParamsPanel event table entries

END_EVENT_TABLE()


/*!
 * CWigParamsPanel constructors
 */

CWigParamsPanel::CWigParamsPanel()
{
    Init();
}

CWigParamsPanel::CWigParamsPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CWigParamsPanel creator
 */

bool CWigParamsPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CWigParamsPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CWigParamsPanel creation
    return true;
}


/*!
 * CWigParamsPanel destructor
 */

CWigParamsPanel::~CWigParamsPanel()
{
////@begin CWigParamsPanel destruction
////@end CWigParamsPanel destruction
}


/*!
 * Member initialisation
 */

void CWigParamsPanel::Init()
{
////@begin CWigParamsPanel member initialisation
    m_AssemblyPanel = NULL;
////@end CWigParamsPanel member initialisation
}


/*!
 * Control creation for CWigParamsPanel
 */

// Fix Solaris/DialogBlocks issue
#ifndef _T
    #define _T wxT
#endif

void CWigParamsPanel::CreateControls()
{    
////@begin CWigParamsPanel content construction
    CWigParamsPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemPanel1, wxID_ANY, _("WIG Load Parameters"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxALIGN_TOP|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
    itemStaticBoxSizer3->Add(itemFlexGridSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Stop when number of errors reaches"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText5, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxSpinCtrl* itemSpinCtrl6 = new wxSpinCtrl( itemPanel1, ID_SPINCTRL1, _T("10"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 1000, 10 );
    itemFlexGridSizer4->Add(itemSpinCtrl6, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AssemblyPanel = new CAssemblySelPanel( itemPanel1, ID_PANEL10, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_AssemblyPanel, 1, wxALIGN_TOP|wxALL, 5);

    // Set validators
    itemSpinCtrl6->SetValidator( wxGenericValidator(& GetData().m_NumErrors) );
////@end CWigParamsPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CWigParamsPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CWigParamsPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CWigParamsPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CWigParamsPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CWigParamsPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CWigParamsPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CWigParamsPanel icon retrieval
}

/*!
 * Transfer data to the window
 */

bool CWigParamsPanel::TransferDataToWindow()
{
    m_AssemblyPanel->SetData(GetData().GetMapAssembly());

    if (!wxPanel::TransferDataToWindow())
        return false;

    return true;
}

/*!
 * Transfer data from the window
 */

bool CWigParamsPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    GetData().SetMapAssembly(m_AssemblyPanel->GetData());

    return true;
}

END_NCBI_SCOPE
