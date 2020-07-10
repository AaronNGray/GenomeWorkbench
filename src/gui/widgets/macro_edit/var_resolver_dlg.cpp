/*  $Id: var_resolver_dlg.cpp 37718 2017-02-10 14:20:50Z asztalos $
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
 * Authors: Anatoly Osipov
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/macro_edit/var_resolver_dlg.hpp>
#include <wx/propgrid/propgrid.h>

#include <wx/stattext.h>
#include <wx/sizer.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(macro);

/*!
 * CVarResolverDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CVarResolverDlg, CDialog )


/*!
 * CVarResolverDlg event table definition
 */

BEGIN_EVENT_TABLE( CVarResolverDlg, CDialog )

////@begin CVarResolverDlg event table entries
    EVT_BUTTON( ID_BUTTON_DONE, CVarResolverDlg::OnButtonDoneClick )
    EVT_BUTTON( ID_BUTTON_CANCEL, CVarResolverDlg::OnButtonCancelClick )
    EVT_BUTTON( ID_BUTTON_RESET, CVarResolverDlg::OnButtonResetClick )
////@end CVarResolverDlg event table entries

END_EVENT_TABLE()

/*!
 * CVarResolverDlg constructors
 */

CVarResolverDlg::CVarResolverDlg( )
{
    Init();
}

CVarResolverDlg::CVarResolverDlg( CMacroRep& macro_rep, wxWindow* parent, wxWindowID id, 
    const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_MacroRep(&macro_rep)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CVarResolverDlg creator
 */

bool CVarResolverDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CVarResolverDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer()) {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CVarResolverDlg creation
    return true;
}


/*!
 * CVarResolverDlg destructor
 */

CVarResolverDlg::~CVarResolverDlg()
{
////@begin CVarResolverDlg destruction
    TPropPairsList::iterator it = m_PropPairsList.begin();
    for ( ; it != m_PropPairsList.end(); it++ )
        delete *it;
////@end CVarResolverDlg destruction
}


/*!
 * Member initialisation
 */

void CVarResolverDlg::Init()
{
////@begin CVarResolverDlg member initialisation
    m_Status = false;
    m_PropPairsList.clear();

    m_PropGridCtrl = NULL;
////@end CVarResolverDlg member initialisation

    SetRegistryPath( "Workbench.Dialogs.VarResolver" );
}

/*!
 * Control creation for CVarResolverDlg
 */

void CVarResolverDlg::CreateControls()
{    
////@begin CVarResolverDlg content construction
    wxBoxSizer* bs_vertical = new wxBoxSizer(wxVERTICAL);
    SetSizer(bs_vertical);

    const int width = 350;
    string msg = "Please choose values for variables for the macro:\n";
    msg += m_MacroRep->GetTitle();
    wxStaticText* label = new wxStaticText(this, wxID_STATIC, ToWxString(msg), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    label->Wrap(width);
    bs_vertical->Add(label, 0, wxALL, 5);

    m_PropGridCtrl = new wxPropertyGrid ( this, ID_PROPERTY_GRID_CTRL, wxDefaultPosition, wxSize(width, 250), wxPG_SPLITTER_AUTO_CENTER | wxPG_DEFAULT_STYLE );
    wxBoxSizer* bs_horizontal = new wxBoxSizer(wxHORIZONTAL);

    bs_vertical->Add(m_PropGridCtrl, 1, wxEXPAND | wxALL, 5);
    bs_vertical->Add(bs_horizontal,  0, wxALIGN_CENTER | wxALL, 5);

    wxButton* ResetButton = new wxButton( this, ID_BUTTON_RESET, _("Reset"), wxDefaultPosition, wxDefaultSize, 0 );
    bs_horizontal->Add(ResetButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxButton* CancelButton = new wxButton( this, ID_BUTTON_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    bs_horizontal->Add(CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxButton* DoneButton = new wxButton( this, ID_BUTTON_DONE, _("Done"), wxDefaultPosition, wxDefaultSize, 0 );
    bs_horizontal->Add(DoneButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    if (!m_MacroRep) 
        return;
    int blk; // var block number in macro
    IMacroVar* var = m_MacroRep->GetFirstVar(blk);
    wxPGProperty* property;
    while (var)  {
        if (var->IsGUIResolvable()) {
            CMacroVarAsk* pAsk = dynamic_cast<CMacroVarAsk*>(var);
            if (pAsk) {
                property = new wxStringProperty ( pAsk->GetName(), wxPG_LABEL, pAsk->GetDefaultValue() );
            } else {
                CMacroVarChoice* pChoice = dynamic_cast<CMacroVarChoice*>(var);
                if (pChoice) {
                    wxArrayString tchoices;
                    const string* stored_value = pChoice->GetFirstChoice();
                    while (stored_value) {
                        tchoices.Add(ToWxString(*stored_value));
                        stored_value = pChoice->GetNextChoice();
                    }
                    property = new wxEnumProperty ( ToWxString(pChoice->GetName()), wxPG_LABEL, tchoices );
                }
            }
            wxVariant variant = property->GetValue();
            m_PropPairsList.push_back(new SPropPair(property, var, property->ValueToString(variant)));
            m_PropGridCtrl->Append(property);
        }
        var = m_MacroRep->GetNextVar(blk);
    }
////@end CVarResolverDlg content construction
}

void CVarResolverDlg::OnButtonDoneClick( wxCommandEvent& event )
{
    TPropPairsList::iterator it = m_PropPairsList.begin();
    for (; it != m_PropPairsList.end(); ++it) {
        SPropPair* pair = *it;
        if (pair && pair->m_MacroVar && pair->m_WxProp) {
            pair->m_MacroVar->SetGUIResolvedValue(ToStdString(pair->m_WxProp->GetValueAsString()));
        }
    }
    m_Status = true;
    Close();
}

void CVarResolverDlg::OnButtonCancelClick( wxCommandEvent& event )
{
    Close();
}

void CVarResolverDlg::OnButtonResetClick( wxCommandEvent& event )
{
    TPropPairsList::iterator it = m_PropPairsList.begin();
    for (; it != m_PropPairsList.end(); ++it) {
        SPropPair* pair = *it;
        if (pair && pair->m_WxProp) {
            pair->m_WxProp->SetValue(pair->m_DefaultVisible);
        }
    }
}

END_NCBI_SCOPE
