/*  $Id: feature_check_dialog.cpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/srchctrl.h>

////@begin includes
////@end includes

#include <gui/widgets/seq/feature_check_panel.hpp>
#include <gui/widgets/seq/feature_check_dialog.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/objutils/registry.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

static const char *  kSelectedNodes = "SelectedNodes";
static const char *  kExpandedNodes = "ExpandedNodes";

/*!
 * CFeatureCheckDialog type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CFeatureCheckDialog, wxDialog )


/*!
 * CFeatureCheckDialog event table definition
 */

BEGIN_EVENT_TABLE( CFeatureCheckDialog, wxDialog )

////@begin CFeatureCheckDialog event table entries
    EVT_SEARCHCTRL_SEARCH_BTN( ID_SEARCHCTRL, CFeatureCheckDialog::OnSearchctrlSearchButtonClick )
    EVT_TEXT( ID_SEARCHCTRL, CFeatureCheckDialog::OnSearchctrlTextUpdated )

////@end CFeatureCheckDialog event table entries

END_EVENT_TABLE()


/*!
 * CFeatureCheckDialog constructors
 */

CFeatureCheckDialog::CFeatureCheckDialog()
{
    Init();
}

CFeatureCheckDialog::CFeatureCheckDialog( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CFeatureSelectDialog creator
 */

bool CFeatureCheckDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CFeatureCheckDialog creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CFeatureCheckDialog creation   
    return true;
}


/*!
 * CFeatureCheckDialog destructor
 */

CFeatureCheckDialog::~CFeatureCheckDialog()
{
////@begin CFeatureCheckDialog destruction
////@end CFeatureCheckDialog destruction
}


/*!
 * Member initialisation
 */

void CFeatureCheckDialog::Init()
{
////@begin CFeatureCheckDialog member initialisation
    m_Filter = NULL;
    m_Panel = NULL;
////@end CFeatureCheckDialog member initialisation

    m_RegPath = "";

}


/*!
 * Control creation for CFeatureSelectDialog
 */

void CFeatureCheckDialog::CreateControls()
{
////@begin CFeatureCheckDialog content construction
    CFeatureCheckDialog* itemDialog1 = this;

    wxFlexGridSizer* itemFlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
    itemDialog1->SetSizer(itemFlexGridSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Type any part of feature name to filter list"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemFlexGridSizer2->Add(itemStaticBoxSizer3, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Filter = new wxSearchCtrl( itemDialog1, ID_SEARCHCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer3->Add(m_Filter, 0, wxGROW|wxALL, 5);

    m_Panel = new CFeatureCheckPanel( itemDialog1, ID_PANEL1, wxDefaultPosition, wxSize(240, 200), wxTAB_TRAVERSAL );
    itemFlexGridSizer2->Add(m_Panel, 1, wxGROW|wxGROW|wxALL, 5);

    wxStaticLine* itemStaticLine6 = new wxStaticLine( itemDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemFlexGridSizer2->Add(itemStaticLine6, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer7 = new wxStdDialogButtonSizer;

    itemFlexGridSizer2->Add(itemStdDialogButtonSizer7, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxButton* itemButton8 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer7->AddButton(itemButton8);

    wxButton* itemButton9 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer7->AddButton(itemButton9);

    itemStdDialogButtonSizer7->Realize();

    itemFlexGridSizer2->AddGrowableRow(1);
    itemFlexGridSizer2->AddGrowableCol(0);

////@end CFeatureCheckDialog content construction   
    //m_Panel->SetFocus();
}


/*!
 * Should we show tooltips?
 */

bool CFeatureCheckDialog::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CFeatureCheckDialog::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CFeatureCheckDialog bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CFeatureCheckDialog bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CFeatureCheckDialog::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CFeatureCheckDialog icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFeatureCheckDialog icon retrieval
}

void CFeatureCheckDialog::GetSelected(TFeatTypeItemSet& feat_types)
{
    ((CFeatureCheckPanel*)m_Panel)->GetSelected(feat_types);    
}

void CFeatureCheckDialog::SetSelected(TFeatTypeItemSet& feat_types)
{
    ((CFeatureCheckPanel*)m_Panel)->SetSelected(feat_types);
}

void CFeatureCheckDialog::SetRegistryPath(const string & rpath)
{
    m_RegPath = rpath;
}

int CFeatureCheckDialog::ShowModal()
{
    // restore state
    x_Serialize(true);

    int action = wxDialog::ShowModal();

    if (action == wxID_OK ) {
        x_Serialize(false);
    }

    return action;
}

void CFeatureCheckDialog::x_Serialize(bool bRead)
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();

        CFeatureCheckPanel::TStateVector check, expand;     

        if (bRead){
            CRegistryReadView view = gui_reg.GetReadView(m_RegPath);
            if (view.HasField(kSelectedNodes)) {
                view.GetStringVec(kSelectedNodes, check);
            }
            if (view.HasField(kExpandedNodes)) {
                view.GetStringVec(kExpandedNodes, expand);
            }
        }
        
        ((CFeatureCheckPanel*)m_Panel)->AccessState(check, expand, bRead);      

        if (!bRead) {
            CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);
            view.Set(kSelectedNodes, check);
            view.Set(kExpandedNodes, expand);
        }
    }
}



/*!
 * wxEVT_COMMAND_SEARCHCTRL_SEARCH_BTN event handler for ID_SEARCHCTRL
 */

void CFeatureCheckDialog::OnSearchctrlSearchButtonClick( wxCommandEvent& event )
{   
    m_Panel->Filter(ToStdString(m_Filter->GetValue()));
////@begin wxEVT_COMMAND_SEARCHCTRL_SEARCH_BTN event handler for ID_SEARCHCTRL in CFeatureCheckDialog.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_SEARCHCTRL_SEARCH_BTN event handler for ID_SEARCHCTRL in CFeatureCheckDialog.
}


/*!
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_SEARCHCTRL
 */

void CFeatureCheckDialog::OnSearchctrlTextUpdated( wxCommandEvent& event )
{       
    m_Panel->Filter(ToStdString(m_Filter->GetValue()));             
////@begin wxEVT_COMMAND_TEXT_UPDATED event handler for ID_SEARCHCTRL in CFeatureCheckDialog.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_TEXT_UPDATED event handler for ID_SEARCHCTRL in CFeatureCheckDialog.
}

END_NCBI_SCOPE
