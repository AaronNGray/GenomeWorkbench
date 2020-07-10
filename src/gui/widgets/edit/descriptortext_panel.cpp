/*  $Id: descriptortext_panel.cpp 43145 2019-05-21 17:08:04Z filippov $
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

#include "descriptortext_panel.hpp"

#include <gui/widgets/wx/wx_utils.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CDescriptorTextPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CDescriptorTextPanel, wxPanel )


/*!
 * CDescriptorTextPanel event table definition
 */

BEGIN_EVENT_TABLE( CDescriptorTextPanel, wxPanel )

////@begin CDescriptorTextPanel event table entries
////@end CDescriptorTextPanel event table entries

END_EVENT_TABLE()


/*!
 * CDescriptorTextPanel constructors
 */

CDescriptorTextPanel::CDescriptorTextPanel()
{
    Init();
}

CDescriptorTextPanel::CDescriptorTextPanel( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CDescriptorTextPanel creator
 */

bool CDescriptorTextPanel::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CDescriptorTextPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CDescriptorTextPanel creation
    return true;
}


/*!
 * CDescriptorTextPanel destructor
 */

CDescriptorTextPanel::~CDescriptorTextPanel()
{
////@begin CDescriptorTextPanel destruction
////@end CDescriptorTextPanel destruction
}


/*!
 * Member initialisation
 */

void CDescriptorTextPanel::Init()
{
////@begin CDescriptorTextPanel member initialisation
    m_Text = NULL;
////@end CDescriptorTextPanel member initialisation
}


/*!
 * Control creation for CDescriptorTextPanel
 */

void CDescriptorTextPanel::CreateControls()
{    
////@begin CDescriptorTextPanel content construction
    CDescriptorTextPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_Text = new wxTextCtrl( itemPanel1, ID_TEXTCTRL23, wxEmptyString, wxDefaultPosition, wxSize(400, 300), wxTE_MULTILINE );
    itemBoxSizer2->Add(m_Text, 1, wxGROW|wxALL, 5);

////@end CDescriptorTextPanel content construction
}


void CDescriptorTextPanel::SetText( const wxString& value)
{
    m_Text->SetValue (value);
}


const wxString CDescriptorTextPanel::GetText ()
{
    return m_Text->GetValue ();
}


/*!
 * Should we show tooltips?
 */

bool CDescriptorTextPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CDescriptorTextPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CDescriptorTextPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CDescriptorTextPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CDescriptorTextPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CDescriptorTextPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CDescriptorTextPanel icon retrieval
}


void CDescriptorTextPanel::ChangeSeqdesc(const objects::CSeqdesc& desc)
{
    switch (desc.Which()) {
        case CSeqdesc::e_Title:
            m_Text->SetValue(ToWxString(desc.GetTitle()));
            break;
        case CSeqdesc::e_Comment:
            m_Text->SetValue(ToWxString(desc.GetComment()));
            break;
        case CSeqdesc::e_Region:
            m_Text->SetValue(ToWxString(desc.GetRegion()));
            break;
        case CSeqdesc::e_Name:
            m_Text->SetValue(ToWxString(desc.GetName()));
            break;
        default:
            m_Text->SetValue(wxEmptyString);
            break;
    }
    m_Desc.Reset(new CSeqdesc);
    m_Desc->Assign(desc);
}


void CDescriptorTextPanel::UpdateSeqdesc(objects::CSeqdesc& desc)
{
    TransferDataFromWindow();
    string text = ToStdString (m_Text->GetValue());            
    switch (desc.Which()) {
        case CSeqdesc::e_Comment:
            desc.SetComment(text);
            break;
        case CSeqdesc::e_Title:
            desc.SetTitle(text);
            break;
        case CSeqdesc::e_Name:
            desc.SetName(text);
            break;
        case CSeqdesc::e_Region:
            desc.SetRegion(text);
            break;
        default:
            break;
    }
}

bool CDescriptorTextPanel::IsImportEnabled()
{
    return RunningInsideNCBI();
}

bool CDescriptorTextPanel::IsExportEnabled()
{
    return RunningInsideNCBI();
}

CRef<CSerialObject> CDescriptorTextPanel::OnExport()
{
    if (m_Desc)
    {
        TransferDataFromWindow();
        CRef<CSerialObject> so((CSerialObject*)(new CSeqdesc));
        UpdateSeqdesc(*m_Desc);
        so->Assign(*m_Desc);
        return so;
    }
    return CRef<CSerialObject>(NULL);
}

void CDescriptorTextPanel::OnImport( CNcbiIfstream &istr)
{
    CRef<CSeqdesc> desc(new CSeqdesc);
    istr >> MSerial_AsnText >> *desc;
    ChangeSeqdesc(*desc);
    TransferDataToWindow();
}

END_NCBI_SCOPE
