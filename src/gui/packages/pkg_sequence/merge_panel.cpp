/*  $Id: merge_panel.cpp 32655 2015-04-07 18:11:22Z evgeniev $
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
 * Authors:  Yury Voronov
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <gui/packages/pkg_sequence/merge_panel.hpp>
#include <gui/widgets/object_list/object_list_widget_sel.hpp>

#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/icon.h>


BEGIN_NCBI_SCOPE

////@begin XPM images
////@end XPM images


/*!
 * CMergePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CMergePanel, CAlgoToolManagerParamsPanel )


/*!
 * CMergePanel event table definition
 */

BEGIN_EVENT_TABLE( CMergePanel, CAlgoToolManagerParamsPanel )

////@begin CMergePanel event table entries
////@end CMergePanel event table entries

END_EVENT_TABLE()


/*!
 * CMergePanel constructors
 */

CMergePanel::CMergePanel()
{
    Init();
}

CMergePanel::CMergePanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CMergePanel creator
 */

bool CMergePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMergePanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    CAlgoToolManagerParamsPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CMergePanel creation
    return true;
}


/*!
 * CMergePanel destructor
 */

CMergePanel::~CMergePanel()
{
////@begin CMergePanel destruction
////@end CMergePanel destruction
}


/*!
 * Member initialisation
 */

void CMergePanel::Init()
{
////@begin CMergePanel member initialisation
    m_MasterItems = NULL;
    m_ChildItems = NULL;
////@end CMergePanel member initialisation
}


/*!
 * Control creation for CMergePanel
 */

void CMergePanel::CreateControls()
{    
////@begin CMergePanel content construction
    CMergePanel* itemCAlgoToolManagerParamsPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCAlgoToolManagerParamsPanel1->SetSizer(itemBoxSizer2);

    m_MasterItems = new CObjectListWidgetSel( itemCAlgoToolManagerParamsPanel1, ID_MASTER_ITEMS, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL|wxLC_SINGLE_SEL );
    itemBoxSizer2->Add(m_MasterItems, 1, wxGROW|wxALL, 1);

    m_ChildItems = new CObjectListWidgetSel( itemCAlgoToolManagerParamsPanel1, ID_CHILD_ITEMS, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_ChildItems, 1, wxGROW|wxALL, 1);

    wxGridSizer* itemGridSizer5 = new wxGridSizer(0, 2, 0, 10);
    itemBoxSizer2->Add(itemGridSizer5, 0, wxGROW|wxALL, 5);

    wxCheckBox* itemCheckBox6 = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_STRIP_ANNOT_DESC, _("Strip annotation descriptors"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox6->SetValue(true);
    itemGridSizer5->Add(itemCheckBox6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox7 = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CONVERT_ACC, _("Convert accessions to GIs"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox7->SetValue(true);
    itemGridSizer5->Add(itemCheckBox7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox8 = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_EXT_CLEANUP, _("Perform extended cleanup"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox8->SetValue(false);
    itemGridSizer5->Add(itemCheckBox8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemGridSizer5->Add(0, 0, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    // Set validators
    itemCheckBox6->SetValidator( wxGenericValidator(& m_data.mf_StripAnnotDescs) );
    itemCheckBox7->SetValidator( wxGenericValidator(& m_data.mf_ConvertAccs) );
    itemCheckBox8->SetValidator( wxGenericValidator(& m_data.mf_ExetendedCleanup) );
////@end CMergePanel content construction

    m_MasterItems->SetTitle(wxT("Select Master Item"));
    m_ChildItems->SetDoSelectAll( true );
    m_ChildItems->SetTitle(wxT("Select Child Items"));
}

void CMergePanel::SetObjects(
    map<string, TConstScopedObjects>& masters,
    map<string, TConstScopedObjects>& kids
){
    m_MasterItems->SetObjects( &masters );
    m_ChildItems->SetObjects( &kids );
}

/*!
 * Should we show tooltips?
 */

bool CMergePanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CMergePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMergePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CMergePanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CMergePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMergePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CMergePanel icon retrieval
}

void CMergePanel::RestoreDefaults()
{
    GetData().Init();
    TransferDataToWindow();
}

END_NCBI_SCOPE
