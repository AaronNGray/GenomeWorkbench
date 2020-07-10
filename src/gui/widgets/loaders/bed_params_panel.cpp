/*  $Id: bed_params_panel.cpp 28688 2013-08-16 19:42:43Z katargir $
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

#include <gui/widgets/loaders/bed_params_panel.hpp>
#include <gui/widgets/loaders/assembly_sel_panel.hpp>

BEGIN_NCBI_SCOPE

/*!
 * CBedParamsPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CBedParamsPanel, wxPanel )


/*!
 * CBedParamsPanel event table definition
 */

BEGIN_EVENT_TABLE( CBedParamsPanel, wxPanel )

////@begin CBedParamsPanel event table entries
////@end CBedParamsPanel event table entries

END_EVENT_TABLE()


/*!
 * CBedParamsPanel constructors
 */

CBedParamsPanel::CBedParamsPanel()
{
    Init();
}

CBedParamsPanel::CBedParamsPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CBedParamsPanel creator
 */

bool CBedParamsPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CBedParamsPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CBedParamsPanel creation
    return true;
}


/*!
 * CBedParamsPanel destructor
 */

CBedParamsPanel::~CBedParamsPanel()
{
////@begin CBedParamsPanel destruction
////@end CBedParamsPanel destruction
}


/*!
 * Member initialisation
 */

void CBedParamsPanel::Init()
{
////@begin CBedParamsPanel member initialisation
    m_AssemblyPanel = NULL;
////@end CBedParamsPanel member initialisation
}


/*!
 * Control creation for CBedParamsPanel
 */

// Fix Solaris/DialogBlocks issue
#ifndef _T
    #define _T wxT
#endif

void CBedParamsPanel::CreateControls()
{    
////@begin CBedParamsPanel content construction
    CBedParamsPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Load Parameters"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxALIGN_TOP|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
    itemStaticBoxSizer3->Add(itemFlexGridSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Stop when number of errors reaches"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText5, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxSpinCtrl* itemSpinCtrl6 = new wxSpinCtrl( itemPanel1, ID_SPINCTRL2, _T("10"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 1000, 10 );
    itemFlexGridSizer4->Add(itemSpinCtrl6, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AssemblyPanel = new CAssemblySelPanel( itemPanel1, ID_PANEL8, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_AssemblyPanel, 1, wxALIGN_TOP|wxALL, 5);

    // Set validators
    itemSpinCtrl6->SetValidator( wxGenericValidator(& GetData().m_NumErrors) );
////@end CBedParamsPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CBedParamsPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CBedParamsPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CBedParamsPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CBedParamsPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CBedParamsPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CBedParamsPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CBedParamsPanel icon retrieval
}

/*!
 * Transfer data to the window
 */

bool CBedParamsPanel::TransferDataToWindow()
{
    m_AssemblyPanel->SetData(GetData().GetMapAssembly());

    if (!wxPanel::TransferDataToWindow())
        return false;

    return true;
}

/*!
 * Transfer data from the window
 */

bool CBedParamsPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    GetData().SetMapAssembly(m_AssemblyPanel->GetData());

    return true;
}

END_NCBI_SCOPE
