/*  $Id: singlegbqual_subpanel.cpp 37632 2017-01-30 19:34:30Z filippov $
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
#include <objects/seqfeat/Gb_qual.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

////@begin includes
////@end includes

#include "singlegbqual_subpanel.hpp"
#include "gbqualtext_panel.hpp"
#include "twopartqual_panel.hpp"
#include "gbqual_semicontrolled.hpp"
#include "rptunitrange_panel.hpp"
#include "gbqualdirection_panel.hpp"
#include "rpttype_panel.hpp"
#include <gui/widgets/edit/inference_panel.hpp>   

#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*!
 * CSingleGbQualSubPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSingleGbQualSubPanel, wxPanel )


/*!
 * CSingleGbQualSubPanel event table definition
 */

BEGIN_EVENT_TABLE( CSingleGbQualSubPanel, wxPanel )

////@begin CSingleGbQualSubPanel event table entries
////@end CSingleGbQualSubPanel event table entries

END_EVENT_TABLE()


/*!
 * CSingleGbQualSubPanel constructors
 */

CSingleGbQualSubPanel::CSingleGbQualSubPanel()
{
    Init();
}

CSingleGbQualSubPanel::CSingleGbQualSubPanel( wxWindow* parent, CGb_qual& qual, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: m_Qual(0)
{
    Init();
    m_Qual = new CGb_qual();
    m_Qual->Assign(qual);
    Create(parent, id, caption, pos, size, style);
}


/*!
 * SimpleGbQualSubPanel creator
 */

bool CSingleGbQualSubPanel::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSingleGbQualSubPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSingleGbQualSubPanel creation
    return true;
}


/*!
 * CSingleGbQualSubPanel destructor
 */

CSingleGbQualSubPanel::~CSingleGbQualSubPanel()
{
////@begin CSingleGbQualSubPanel destruction
////@end CSingleGbQualSubPanel destruction
}


/*!
 * Member initialisation
 */

void CSingleGbQualSubPanel::Init()
{
////@begin CSingleGbQualSubPanel member initialisation
    m_TheSizer = NULL;
    m_KeyCtrl = NULL;
////@end CSingleGbQualSubPanel member initialisation
}


/*!
 * Control creation for SimpleGbQualSubPanel
 */

void CSingleGbQualSubPanel::CreateControls()
{    
////@begin CSingleGbQualSubPanel content construction
    CSingleGbQualSubPanel* itemPanel1 = this;

    m_TheSizer = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(m_TheSizer);

    m_KeyCtrl = new wxStaticText( itemPanel1, wxID_STATIC, _("Static text"), wxDefaultPosition, wxSize(160, -1), 0 );
    m_TheSizer->Add(m_KeyCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

////@end CSingleGbQualSubPanel content construction

    // for now, only text
    // later, implement specialized editors

    string key = m_Qual->GetQual();
    if ( NStr::EqualNocase(key, "mobile_element") )
    {
        key =  "mobile_element_type";
        m_Qual->SetQual(key);
    }

    key = NStr::ReplaceInPlace(key, "_", "-");
    m_KeyCtrl->SetLabel(ToWxString(key));
    string val;
    if (m_Qual->IsSetVal()) {
        val = m_Qual->GetVal();
    }

    if (NStr::EqualNocase(key, "mobile-element-type") || NStr::EqualNocase(key, "satellite")) {  
        CTwoPartQualPanel* ctrl = new CTwoPartQualPanel(this);
        m_ValueCtrl = ctrl;
        if (NStr::EqualNocase(key, "mobile-element-type")) {
            vector<string> choices;
            choices.push_back("insertion sequence");
            choices.push_back("retrotransposon");
            choices.push_back("non-LTR retrotransposon");
            choices.push_back("transposon");
            choices.push_back("integron");
            choices.push_back("other");
            choices.push_back("SINE");
            choices.push_back("MITE");
            choices.push_back("LINE");
            ctrl->SetControlledList(choices);
        } else if (NStr::EqualNocase(key, "satellite")) {
            vector<string> choices;
            choices.push_back("satellite");
            choices.push_back("microsatellite");
            choices.push_back("minisatellite");
            ctrl->SetControlledList(choices);
        }
    } else if (NStr::EqualNocase(key, "regulatory-class")) {
        CGBQualSemicontrolledTextPanel* ctrl = new CGBQualSemicontrolledTextPanel(this);
        m_ValueCtrl = ctrl;
        vector<string> choices = CSeqFeatData::GetRegulatoryClassList();
        ctrl->SetControlledList(choices);
    } else if (NStr::EqualNocase(key, "recombination-class")) {
        CGBQualSemicontrolledTextPanel* ctrl = new CGBQualSemicontrolledTextPanel(this);
        m_ValueCtrl = ctrl;
        vector<string> choices;
        CGb_qual::TLegalRecombinationClassSet recomb_classes = CGb_qual::GetSetOfLegalRecombinationClassValues();
        ITERATE(CGb_qual::TLegalRecombinationClassSet, it, recomb_classes) {
            choices.push_back(*it);
        }
        ctrl->SetControlledList(choices);
    } else if (NStr::EqualNocase(key, "rpt-unit-range")) {
        CRptUnitRangePanel* ctrl = new CRptUnitRangePanel(this);
        m_ValueCtrl = ctrl;
    } else if (NStr::EqualNocase(key, "direction")) {
        CGBQualDirectionPanel* ctrl = new CGBQualDirectionPanel(this);
        m_ValueCtrl = ctrl;
    } else if (NStr::EqualNocase(key, "rpt-type") && CRptTypePanel::IsParseable(val)) {
        CRptTypePanel* ctrl = new CRptTypePanel(this);
        m_ValueCtrl = ctrl; 
    } else if (NStr::EqualNocase(key, "inference")) {
        // temporary
        m_ValueCtrl = new CInferencePanel(this);
    } else {    
        m_ValueCtrl = new CGBQualTextPanel(itemPanel1);
    }
    m_TheSizer->Add(m_ValueCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
  
}


bool CSingleGbQualSubPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    if (m_Qual->IsSetVal()) {    
        if (m_Qual->IsSetQual() && NStr::Equal(m_Qual->GetQual(), "replace") && m_Qual->GetVal().empty())
            m_ValueCtrl->SetValue("\"\"");
        else
            m_ValueCtrl->SetValue(m_Qual->GetVal());
    } else {
        m_ValueCtrl->SetValue(kEmptyStr);
    }

    return true;
}


bool CSingleGbQualSubPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    string val = m_ValueCtrl->GetValue();
    if (NStr::IsBlank(val)) {
        if (m_Qual->IsSetQual() && NStr::Equal(m_Qual->GetQual(), "regulatory")) {
            m_Qual->SetVal("other");
        } else {
            m_Qual->ResetQual();
            m_Qual->ResetVal();
        }
    } else {
        if (m_Qual->IsSetQual() && NStr::Equal(m_Qual->GetQual(), "replace") && val == "\"\"")
            m_Qual->SetVal(kEmptyStr);
        else
            m_Qual->SetVal(val);
    }
    return true;
}


CRef<CGb_qual> CSingleGbQualSubPanel::GetGbQual()
{
    TransferDataFromWindow();
    if (m_Qual)
        return m_Qual;

    return CRef<CGb_qual>();
}

void CSingleGbQualSubPanel::SetGbQual(CRef<CGb_qual>& qual)
{
    m_Qual = qual;
    TransferDataToWindow();
}

/*!
 * Should we show tooltips?
 */

bool CSingleGbQualSubPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSingleGbQualSubPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSingleGbQualSubPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSingleGbQualSubPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSingleGbQualSubPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSingleGbQualSubPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSingleGbQualSubPanel icon retrieval
}

END_NCBI_SCOPE
