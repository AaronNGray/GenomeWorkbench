/*  $Id: macro_loc_constraint_panel.cpp 40627 2018-03-21 15:34:12Z asztalos $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Igor Filippov
 *
 * File Description:
 *
 */
#include <ncbi_pch.hpp>

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <objects/macro/Location_pos_constraint.hpp>
#include <gui/widgets/edit/macro_constraint_panel.hpp>
#include <gui/widgets/edit/macro_loc_constraint_panel.hpp>


BEGIN_NCBI_SCOPE

////@begin XPM images
////@end XPM images


/*
 * CMacroLocationConstraintPanel type definition
 */

IMPLEMENT_CLASS( CMacroLocationConstraintPanel, wxPanel )


/*
 * CMacroLocationConstraintPanel event table definition
 */

BEGIN_EVENT_TABLE( CMacroLocationConstraintPanel, wxPanel )

////@begin CMacroLocationConstraintPanel event table entries
    EVT_CHOICE( wxID_ANY, CMacroLocationConstraintPanel::OnAnySelected )

////@end CMacroLocationConstraintPanel event table entries

END_EVENT_TABLE()


/*
 * CMacroLocationConstraintPanel constructors
 */

CMacroLocationConstraintPanel::CMacroLocationConstraintPanel()
{
    Init();
}

CMacroLocationConstraintPanel::CMacroLocationConstraintPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create( parent, id, pos, size, style );
}


/*
 * CMacroLocationConstraintPanel creator
 */

bool CMacroLocationConstraintPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMacroLocationConstraintPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    Centre();
////@end CMacroLocationConstraintPanel creation
    return true;
}


/*
 * CMacroLocationConstraintPanel destructor
 */

CMacroLocationConstraintPanel::~CMacroLocationConstraintPanel()
{
////@begin CMacroLocationConstraintPanel destruction
////@end CMacroLocationConstraintPanel destruction
}


/*
 * Member initialisation
 */

void CMacroLocationConstraintPanel::Init()
{
////@begin CMacroLocationConstraintPanel member initialisation
    m_Strand = NULL;
    m_NucOrProt = NULL;
    m_5Partial = NULL;
    m_3Partial = NULL;
    m_LocType = NULL;
    m_5DistType = NULL;
    m_5Dist = NULL;
    m_3DistType = NULL;
    m_3Dist = NULL;
////@end CMacroLocationConstraintPanel member initialisation
}


/*
 * Control creation for CMacroLocationConstraintPanel
 */

void CMacroLocationConstraintPanel::CreateControls()
{    
////@begin CMacroLocationConstraintPanel content construction
    CMacroLocationConstraintPanel* itemPanel2 = this;

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    itemPanel2->SetSizer(itemBoxSizer1);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer2, 0, wxALIGN_CENTER_HORIZONTAL, 0);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer4, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel2, wxID_STATIC, _("on"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_StrandStrings;
    m_StrandStrings.Add(_("Any strand"));
    m_StrandStrings.Add(_("Plus strand"));
    m_StrandStrings.Add(_("Minus strand"));
    m_Strand = new wxChoice( itemPanel2, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_StrandStrings, 0 );
    m_Strand->SetStringSelection(_("Any strand"));
    itemBoxSizer4->Add(m_Strand, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel2, wxID_STATIC, _("on"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_NucOrProtStrings;
    m_NucOrProtStrings.Add(_("nucleotide and protein sequences"));
    m_NucOrProtStrings.Add(_("nucleotide sequences only"));
    m_NucOrProtStrings.Add(_("protein sequences only"));
    m_NucOrProt = new wxChoice( itemPanel2, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_NucOrProtStrings, 0 );
    m_NucOrProt->SetStringSelection(_("nucleotide and protein sequences"));
    itemBoxSizer4->Add(m_NucOrProt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer9, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemPanel2, wxID_STATIC, _("5' end"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(itemStaticText10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_5PartialStrings;
    m_5PartialStrings.Add(_("Partial or Complete"));
    m_5PartialStrings.Add(_("Partial"));
    m_5PartialStrings.Add(_("Complete"));
    m_5Partial = new wxChoice( itemPanel2, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_5PartialStrings, 0 );
    m_5Partial->SetStringSelection(_("Partial or Complete"));
    itemBoxSizer9->Add(m_5Partial, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText12 = new wxStaticText( itemPanel2, wxID_STATIC, _("3' end"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(itemStaticText12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_3PartialStrings;
    m_3PartialStrings.Add(_("Partial or Complete"));
    m_3PartialStrings.Add(_("Partial"));
    m_3PartialStrings.Add(_("Complete"));
    m_3Partial = new wxChoice( itemPanel2, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_3PartialStrings, 0 );
    m_3Partial->SetStringSelection(_("Partial or Complete"));
    itemBoxSizer9->Add(m_3Partial, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer14 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer14, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText15 = new wxStaticText( itemPanel2, wxID_STATIC, _("location type"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer14->Add(itemStaticText15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_LocTypeStrings;
    m_LocTypeStrings.Add(_("Any"));
    m_LocTypeStrings.Add(_("Single interval"));
    m_LocTypeStrings.Add(_("Joined"));
    m_LocTypeStrings.Add(_("Ordered"));
    m_LocType = new wxChoice( itemPanel2, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_LocTypeStrings, 0 );
    m_LocType->SetStringSelection(_("Any"));
    itemBoxSizer14->Add(m_LocType, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer17, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText18 = new wxStaticText( itemPanel2, wxID_STATIC, _("where end of sequence is"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer17->Add(itemStaticText18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_5DistTypeStrings;
    m_5DistTypeStrings.Add(_("Any distance"));
    m_5DistTypeStrings.Add(_("Exactly"));
    m_5DistTypeStrings.Add(_("No more than"));
    m_5DistTypeStrings.Add(_("No less than"));
    m_5DistType = new wxChoice( itemPanel2, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_5DistTypeStrings, 0 );
    m_5DistType->SetStringSelection(_("Any distance"));
    itemBoxSizer17->Add(m_5DistType, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_5Dist = new wxTextCtrl( itemPanel2, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer17->Add(m_5Dist, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_5Dist->SetValidator( wxTextValidator( wxFILTER_NUMERIC ) );
    m_5Dist->Disable();

    wxStaticText* itemStaticText21 = new wxStaticText( itemPanel2, wxID_STATIC, _("from 5' end of the feature"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer17->Add(itemStaticText21, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer22, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText23 = new wxStaticText( itemPanel2, wxID_STATIC, _("where end of sequence is"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer22->Add(itemStaticText23, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_3DistTypeStrings;
    m_3DistTypeStrings.Add(_("Any distance"));
    m_3DistTypeStrings.Add(_("Exactly"));
    m_3DistTypeStrings.Add(_("No more than"));
    m_3DistTypeStrings.Add(_("No less than"));
    m_3DistType = new wxChoice( itemPanel2, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_3DistTypeStrings, 0 );
    m_3DistType->SetStringSelection(_("Any distance"));
    itemBoxSizer22->Add(m_3DistType, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_3Dist = new wxTextCtrl( itemPanel2, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer22->Add(m_3Dist, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_3Dist->SetValidator( wxTextValidator( wxFILTER_NUMERIC ) );
    m_3Dist->Disable();

    wxStaticText* itemStaticText26 = new wxStaticText( itemPanel2, wxID_STATIC, _("from 3' end of the feature"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer22->Add(itemStaticText26, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CMacroLocationConstraintPanel content construction
}


/*
 * Should we show tooltips?
 */

bool CMacroLocationConstraintPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CMacroLocationConstraintPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMacroLocationConstraintPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CMacroLocationConstraintPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CMacroLocationConstraintPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMacroLocationConstraintPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CMacroLocationConstraintPanel icon retrieval
}


/*
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for wxID_ANY
 */

void CMacroLocationConstraintPanel::OnAnySelected( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_CHOICE_SELECTED event handler for wxID_ANY in CMacroLocationConstraintPanel.
    // Before editing this code, remove the block markers.
    if ( m_5DistType->GetSelection() > 0)
    {
        m_5Dist->Enable();
    }
    else
    {
        m_5Dist->SetValue(wxEmptyString);
        m_5Dist->Disable();
    }
    if ( m_3DistType->GetSelection() > 0)
    {
        m_3Dist->Enable();
    }
    else
    {
        m_3Dist->SetValue(wxEmptyString);
        m_3Dist->Disable();
    }
    event.Skip(); 
////@end wxEVT_COMMAND_CHOICE_SELECTED event handler for wxID_ANY in CMacroLocationConstraintPanel. 
}


pair<string,string> CMacroLocationConstraintPanel::GetMatcher(const pair<string, string> &target, size_t num)
{
    return make_pair(GetDescription(), GetLocationConstraint("\"location\"")); // TODO
}

void CMacroLocationConstraintPanel::SetMatcher(const string &item)
{
// TODO
}

string CMacroLocationConstraintPanel::GetLocationConstraint(const string &location)
{
    vector<string> c;
    switch (m_Strand->GetSelection())
    {
    case 1 : c.push_back("STRAND() = \"plus\""); break;
    case 2 : c.push_back("STRAND() = \"minus\""); break;
    default : break;        
    }
    switch (m_NucOrProt->GetSelection())
    {
    case 1 : c.push_back("IS_SEQ_NA(" + location + ")"); break;
    case 2 : c.push_back("IS_SEQ_AA(" + location + ")"); break;
    default : break;
    }
    switch (m_5Partial->GetSelection())
    {
    case 1 : c.push_back("ISPARTIALSTART()"); break;
    case 2 : c.push_back("NOT ISPARTIALSTART()"); break;
    default : break;
    }
    switch (m_3Partial->GetSelection())
    {
    case 1 : c.push_back("ISPARTIALSTOP()"); break;
    case 2 : c.push_back("NOT ISPARTIALSTOP()"); break;
    default : break;
    }
    switch  (m_LocType->GetSelection())
    {
    case 1 : c.push_back("ISLOCATIONTYPE(\"single-interval\")"); break;
    case 2 : c.push_back("ISLOCATIONTYPE(\"joined\")"); break;
    case 3 : c.push_back("ISLOCATIONTYPE(\"ordered\")"); break;
    default : break;
    }

    
    if ( m_5DistType->GetSelection() > 0 && !m_5Dist->GetValue().IsEmpty() )
    {
        string d = m_5Dist->GetValue().ToStdString();
        switch (m_5DistType->GetSelection())
        {
        case 1: c.push_back("DISTFROMSTART() = " + d); break;
        case 2: c.push_back("DISTFROMSTART() <= " + d); break;
        case 3: c.push_back("DISTFROMSTART() >= " + d); break;
        default : break;   
        }
    }

    if ( m_3DistType->GetSelection() > 0 && !m_3Dist->GetValue().IsEmpty() )
    {
        string d = m_3Dist->GetValue().ToStdString();
        switch (m_3DistType->GetSelection())
        {
        case 1: c.push_back("DISTFROMSTOP() = " + d); break;
        case 2: c.push_back("DISTFROMSTOP() <= " + d); break;
        case 3: c.push_back("DISTFROMSTOP() >= " + d); break;
        default : break;   
        }
    }

    return NStr::Join(c, " AND ");
}

string CMacroLocationConstraintPanel::GetDescription() const
{
    string label = "only objects on ";
    switch (m_Strand->GetSelection())
    {
    case 1 : label += "plus strands of "; break;
    case 2 : label += "minus strands of "; break;
    default : break;        
    }
    switch (m_NucOrProt->GetSelection())
    {
    case 1 : label += "nucleotide "; break;
    case 2 : label += "protein "; break;
    default : break;
    }
    label += "sequences ";
    string label2;
    switch (m_5Partial->GetSelection())
    {
    case 1 : label2 += "5' partial "; break;
    case 2 : label2 += "5' complete "; break;
    default : break;
    }
    switch (m_3Partial->GetSelection())
    {
    case 1 : label2 += "3' partial "; break;
    case 2 : label2 += "3' complete "; break;
    default : break;
    }
    switch  (m_LocType->GetSelection())
    {
    case 1 : label2 += "with single interval "; break;
    case 2 : label2 += "with joined intervals "; break;
    case 3 : label2 += "with ordered intervals "; break;
    default : break;
    }
    if ( m_5DistType->GetSelection() > 0 && !m_5Dist->GetValue().IsEmpty() )
    {
        label2 += "with 5' end ";
        switch (m_5DistType->GetSelection())
        {
        case 1: label2 += "exactly "; break;
        case 2: label2 += "no more than "; break;
        case 3: label2 += "no less than "; break;
        }
        label2 += m_5Dist->GetValue().ToStdString();
        label2 += " from end of sequence ";
    }
    if ( m_3DistType->GetSelection() > 0 && !m_3Dist->GetValue().IsEmpty() )
    {
        label2 += "with 3' end ";
        switch (m_3DistType->GetSelection())
        {
        case 1: label2 += "exactly "; break;
        case 2: label2 += "no more than "; break;
        case 3: label2 += "no less than "; break;
        }
        label2 += m_3Dist->GetValue().ToStdString();
        label2 += " from end of sequence ";
    }
    if (!label2.empty())
        label += "that are " + label2;
    return label;
}


END_NCBI_SCOPE
