/*  $Id: psl_params_panel.cpp 44439 2019-12-18 19:51:52Z katargir $
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

#include <gui/widgets/loaders/psl_params_panel.hpp>
#include <gui/widgets/loaders/assembly_sel_panel.hpp>

BEGIN_NCBI_SCOPE

/*!
 * CPslParamsPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CPslParamsPanel, wxPanel )

/*!
 * CPslParamsPanel event table definition
 */

BEGIN_EVENT_TABLE( CPslParamsPanel, wxPanel )

////@begin CPslParamsPanel event table entries
////@end CPslParamsPanel event table entries

END_EVENT_TABLE()


/*!
 * CPslParamsPanel constructors
 */

CPslParamsPanel::CPslParamsPanel()
{
    Init();
}

CPslParamsPanel::CPslParamsPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CPlsParamsPanel creator
 */

bool CPslParamsPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CPslParamsPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CPslParamsPanel creation
    return true;
}


/*!
 * CPslParamsPanel destructor
 */

CPslParamsPanel::~CPslParamsPanel()
{
////@begin CPslParamsPanel destruction
////@end CPslParamsPanel destruction
}


/*!
 * Member initialisation
 */

void CPslParamsPanel::Init()
{
////@begin CPslParamsPanel member initialisation
    m_AssemblyPanel = NULL;
////@end CPslParamsPanel member initialisation
}


/*!
 * Control creation for CPlsParamsPanel
 */

void CPslParamsPanel::CreateControls()
{    
////@begin CPslParamsPanel content construction
    CPslParamsPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_AssemblyPanel = new CAssemblySelPanel( itemPanel1, ID_PANEL8, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_AssemblyPanel, 1, wxALIGN_TOP|wxALL, wxDLG_UNIT(itemPanel1, wxSize(5, -1)).x);

////@end CPslParamsPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CPslParamsPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CPslParamsPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CPslParamsPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CPslParamsPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CPslParamsPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CPslParamsPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CPslParamsPanel icon retrieval
}
/*!
 * Transfer data to the window
 */

bool CPslParamsPanel::TransferDataToWindow()
{
    m_AssemblyPanel->SetData(GetData().GetMapAssembly());

    if (!wxPanel::TransferDataToWindow())
        return false;

    return true;
}

/*!
 * Transfer data from the window
 */

bool CPslParamsPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    GetData().SetMapAssembly(m_AssemblyPanel->GetData());

    return true;
}

END_NCBI_SCOPE
