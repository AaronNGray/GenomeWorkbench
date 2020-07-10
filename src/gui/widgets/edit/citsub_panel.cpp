/*  $Id: citsub_panel.cpp 36886 2016-11-09 18:08:53Z filippov $
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
#include <gui/utils/command_processor.hpp>
#include <gui/widgets/edit/edit_object.hpp>

#include <objects/biblio/Cit_sub.hpp>
#include <objects/general/Date.hpp>

#include <gui/widgets/edit/serial_member_primitive_validators.hpp>

////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/radiobox.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/datectrl.h>

#include "citsub_panel.hpp"
#include <gui/widgets/edit/flexibledate_panel.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*
 * CCitSubPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CCitSubPanel, wxPanel )


/*
 * CCitSubPanel event table definition
 */

BEGIN_EVENT_TABLE( CCitSubPanel, wxPanel )

////@begin CCitSubPanel event table entries
    EVT_CHOICE( ID_CITSUB_ADD_STANDARD_REMARK, CCitSubPanel::OnCitsubAddStandardRemarkSelected )

////@end CCitSubPanel event table entries

END_EVENT_TABLE()


/*
 * CCitSubPanel constructors
 */

CCitSubPanel::CCitSubPanel()
{
    Init();
}

CCitSubPanel::CCitSubPanel( wxWindow* parent, CSerialObject& object,
         wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) :
     m_Object(0)
{
    Init();
    m_Object = dynamic_cast<CCit_sub*>(&object);
    const CCit_sub& citsub = dynamic_cast<const CCit_sub&>(*m_Object);
    m_EditedCitSub.Reset((CSerialObject*)CCit_sub::GetTypeInfo()->Create());
    m_EditedCitSub->Assign(citsub);
    Create(parent, id, pos, size, style);
}


/*
 * CCitSubPanel creator
 */

bool CCitSubPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CCitSubPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CCitSubPanel creation
    return true;
}


/*
 * CCitSubPanel destructor
 */

CCitSubPanel::~CCitSubPanel()
{
////@begin CCitSubPanel destruction
////@end CCitSubPanel destruction
}


/*
 * Member initialisation
 */

void CCitSubPanel::Init()
{
////@begin CCitSubPanel member initialisation
    m_Remark = NULL;
    m_StandardRemark = NULL;
////@end CCitSubPanel member initialisation
}


/*
 * Control creation for CCitSubPanel
 */

void CCitSubPanel::CreateControls()
{    
////@begin CCitSubPanel content construction
    CCitSubPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("Description"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Remark = new wxTextCtrl( itemPanel1, ID_CITSUB_DESC, wxEmptyString, wxDefaultPosition, wxSize(400, 85), wxTE_MULTILINE );
    itemBoxSizer2->Add(m_Remark, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Add standard remark text"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_StandardRemarkStrings;
    m_StandardRemarkStrings.Add(wxEmptyString);
    m_StandardRemarkStrings.Add(_("AA by submitter"));
    m_StandardRemarkStrings.Add(_("NT by submitter"));
    m_StandardRemarkStrings.Add(_("NT and AA by submitter"));
    m_StandardRemarkStrings.Add(_("AA by database"));
    m_StandardRemarkStrings.Add(_("NT by database"));
    m_StandardRemarkStrings.Add(_("NT and AA by database"));
    m_StandardRemark = new wxChoice( itemPanel1, ID_CITSUB_ADD_STANDARD_REMARK, wxDefaultPosition, wxDefaultSize, m_StandardRemarkStrings, 0 );
    itemFlexGridSizer5->Add(m_StandardRemark, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("Submission Date"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer5->Add(itemBoxSizer9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CCitSubPanel content construction

    CCit_sub& citsub = dynamic_cast<CCit_sub&>(*m_EditedCitSub);
    m_Remark->SetValidator( CSerialTextValidator(citsub, "descr") );
    CRef<objects::CDate> subdate(&(citsub.SetDate()));
    m_date_ctrl = new CFlexibleDatePanel(itemPanel1, subdate);
    itemBoxSizer9->Add(m_date_ctrl);

}

bool CCitSubPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow()) {
        return false;
    }
    m_date_ctrl->TransferDataFromWindow();
    return true;
}

bool CCitSubPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow()) {
        return false;
    }
    m_date_ctrl->TransferDataToWindow();
    return true;
}

CRef<CCit_sub> CCitSubPanel::GetCit_sub() const
{
    const CCit_sub& sub = dynamic_cast<const CCit_sub&>(*m_EditedCitSub);

    CRef<CCit_sub> ret(new CCit_sub());
    ret->Assign(sub);
    return ret;
}


/*
 * Should we show tooltips?
 */

bool CCitSubPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CCitSubPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CCitSubPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CCitSubPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CCitSubPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CCitSubPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CCitSubPanel icon retrieval
}


/*
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CITSUB_ADD_STANDARD_REMARK
 */

void CCitSubPanel::OnCitsubAddStandardRemarkSelected( wxCommandEvent& event )
{
    wxString std_remark = m_StandardRemark->GetStringSelection();
    wxString remark = std_remark;
    if (std_remark == _("AA by submitter"))	
        remark = _("Amino acid sequence updated by submitter");
    if (std_remark == _("NT by submitter"))	
        remark = _("Nucleotide sequence updated by submitter");
    if (std_remark == _("NT and AA by submitter")) 
        remark = _("Nucleotide and amino acid sequences updated by submitter");
    if (std_remark == _("AA by database"))
        remark = _("Amino acid sequence updated by database staff");
    if (std_remark == _("NT by database"))
        remark = _("Nucleotide sequence updated by database staff");
    if (std_remark == _("NT and AA by database"))
        remark = _("Nucleotide and amino acid sequences updated by database staff");
    
    m_Remark->SetValue(remark);
}

END_NCBI_SCOPE
