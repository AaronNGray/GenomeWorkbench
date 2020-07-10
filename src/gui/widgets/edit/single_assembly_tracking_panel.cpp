/*  $Id: single_assembly_tracking_panel.cpp 43639 2019-08-13 13:29:52Z asztalos $
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
////@begin includes
////@end includes

#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/single_assembly_tracking_panel.hpp>
#include <gui/widgets/edit/assembly_tracking_list_panel.hpp>
#include <wx/sizer.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CSingleAssemblyTrackingPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSingleAssemblyTrackingPanel, CSingleUserFieldPanel )


/*!
 * CSingleAssemblyTrackingPanel event table definition
 */

BEGIN_EVENT_TABLE( CSingleAssemblyTrackingPanel, CSingleUserFieldPanel )

////@begin CSingleAssemblyTrackingPanel event table entries
    EVT_TEXT( ID_ACCESSION_TXT, CSingleAssemblyTrackingPanel::OnAccessionTxtTextUpdated )

    EVT_TEXT( ID_START_TXT, CSingleAssemblyTrackingPanel::OnStartTxtTextUpdated )

    EVT_TEXT( ID_STOP_TXT, CSingleAssemblyTrackingPanel::OnStopTxtTextUpdated )

////@end CSingleAssemblyTrackingPanel event table entries

END_EVENT_TABLE()


/*!
 * CSingleAssemblyTrackingPanel constructors
 */

CSingleAssemblyTrackingPanel::CSingleAssemblyTrackingPanel()
{
    Init();
}

CSingleAssemblyTrackingPanel::CSingleAssemblyTrackingPanel( wxWindow* parent, CUser_field& field, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    m_Field = new CUser_field();
    m_Field->Assign(field);   
    m_Field->SetLabel().SetId(0);
    Create(parent, id, pos, size, style);
}


/*!
 * CSingleAssemblyTrackingPanel creator
 */

bool CSingleAssemblyTrackingPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSingleAssemblyTrackingPanel creation
    CSingleUserFieldPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSingleAssemblyTrackingPanel creation
    return true;
}


/*!
 * CSingleAssemblyTrackingPanel destructor
 */

CSingleAssemblyTrackingPanel::~CSingleAssemblyTrackingPanel()
{
////@begin CSingleAssemblyTrackingPanel destruction
////@end CSingleAssemblyTrackingPanel destruction
}


/*!
 * Member initialisation
 */

void CSingleAssemblyTrackingPanel::Init()
{
////@begin CSingleAssemblyTrackingPanel member initialisation
    m_Accession = NULL;
    m_Start = NULL;
    m_Stop = NULL;
////@end CSingleAssemblyTrackingPanel member initialisation
}


/*!
 * Control creation for CSingleAssemblyTrackingPanel
 */

void CSingleAssemblyTrackingPanel::CreateControls()
{    
////@begin CSingleAssemblyTrackingPanel content construction
    CSingleAssemblyTrackingPanel* itemCSingleUserFieldPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemCSingleUserFieldPanel1->SetSizer(itemBoxSizer2);

    m_Accession = new wxTextCtrl( itemCSingleUserFieldPanel1, ID_ACCESSION_TXT, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer2->Add(m_Accession, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_Start = new wxTextCtrl( itemCSingleUserFieldPanel1, ID_START_TXT, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer2->Add(m_Start, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_Stop = new wxTextCtrl( itemCSingleUserFieldPanel1, ID_STOP_TXT, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer2->Add(m_Stop, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

////@end CSingleAssemblyTrackingPanel content construction
}


/*!
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_ACCESSION_TXT
 */

void CSingleAssemblyTrackingPanel::OnAccessionTxtTextUpdated( wxCommandEvent& event )
{
    x_OnChange(); 
}


/*!
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_START_TXT
 */

void CSingleAssemblyTrackingPanel::OnStartTxtTextUpdated( wxCommandEvent& event )
{
    x_OnChange(); 
}


/*!
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_STOP_TXT
 */

void CSingleAssemblyTrackingPanel::OnStopTxtTextUpdated( wxCommandEvent& event )
{
    x_OnChange(); 
}


/*!
 * Should we show tooltips?
 */

bool CSingleAssemblyTrackingPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSingleAssemblyTrackingPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSingleAssemblyTrackingPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSingleAssemblyTrackingPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSingleAssemblyTrackingPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSingleAssemblyTrackingPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSingleAssemblyTrackingPanel icon retrieval
}


bool CSingleAssemblyTrackingPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    string accession = "";
    int from = 0;
    int to = 0;

    if (m_Field->GetData().IsFields()) {
        ITERATE(CUser_field::TData::TFields, it, m_Field->GetData().GetFields()) {
            if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr()) {
                const string& label = (*it)->GetLabel().GetStr();
                if (NStr::EqualNocase(label, "accession")) {
                    if ((*it)->GetData().IsStr()) {
                        accession = (*it)->GetData().GetStr();
                    }
                } else if (NStr::EqualNocase(label, "from")) {
                    if ((*it)->GetData().IsInt()) {
                        from = (*it)->GetData().GetInt() + 1;
                    }
                } else if (NStr::EqualNocase(label, "to")) {
                    if ((*it)->GetData().IsInt()) {
                        to = (*it)->GetData().GetInt() + 1;
                    }
                }
            }
        }
    }

    m_Accession->SetValue(ToWxString(accession));
    if (from > 0) {
        m_Start->SetValue(ToWxString(NStr::NumericToString(from)));
    } else {
        m_Start->SetValue(wxEmptyString);
    }
    if (to > 0) {
        m_Stop->SetValue(ToWxString(NStr::NumericToString(to)));
    } else {
        m_Stop->SetValue(wxEmptyString);
    }

    return true;
}


bool CSingleAssemblyTrackingPanel::x_IsControlled (const string& field)
{
    if (NStr::EqualNocase(field, "accession")
        || NStr::EqualNocase(field, "to")
        || NStr::EqualNocase(field, "from")) {
        return true;
    } else {
        return false;
    }
}


bool CSingleAssemblyTrackingPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    string accession = ToStdString(m_Accession->GetValue());
    int from = -1;
    int to = -1;

    try {
        from = NStr::StringToInt(ToStdString(m_Start->GetValue())) - 1;
    } catch(const CException&) {} catch (const exception&) {}

    try {
        to = NStr::StringToInt(ToStdString(m_Stop->GetValue())) - 1;
    } catch(const CException&) {} catch (const exception&) {}


    CRef<CUser_field> new_field = s_CreateTPAAssemblyField(accession, from, to);
    m_Field.Reset(new_field);

    return true;
}


CRef<CUser_field> CSingleAssemblyTrackingPanel::GetUser_field()
{
    TransferDataFromWindow();
    if (m_Field)
        return m_Field;

    return CRef<CUser_field>();
}


CRef<CUser_field> CSingleAssemblyTrackingPanel::s_CreateTPAAssemblyField (const string& accession, int from, int to)
{
    CRef<CUser_field> new_field(new CUser_field());

    new_field->SetLabel().SetId(0);
    if (!NStr::IsBlank(accession)) {
        CRef<CUser_field> acc_field(new CUser_field());
        acc_field->SetLabel().SetStr("accession");
        acc_field->SetData().SetStr(accession);
        new_field->SetData().SetFields().push_back(acc_field);
    }

    if (to > -1 && from > -1) {
        CRef<CUser_field> from_field(new CUser_field());
        from_field->SetLabel().SetStr("from");
        from_field->SetData().SetInt(from);
        new_field->SetData().SetFields().push_back(from_field);
        CRef<CUser_field> to_field(new CUser_field());
        to_field->SetLabel().SetStr("to");
        to_field->SetData().SetInt(to);
        new_field->SetData().SetFields().push_back(to_field);
    }
    return new_field;
}


END_NCBI_SCOPE

