/*  $Id: biosourcepanel.cpp 43089 2019-05-13 21:05:28Z filippov $
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
 * Authors:  Vasuki Palanigobu
 */

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include <ncbi_pch.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objects/taxon3/taxon3.hpp>
#include <objtools/edit/source_edit.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>

#include <gui/widgets/wx/auto_complete_text_ctrl.hpp>
#include <gui/widgets/wx/taxon_text_completer.hpp>
#include <gui/widgets/edit/utilities.hpp>
#include "biosourcepanel.hpp"
#include "src_other_pnl.hpp"
#include "loc_gcode_panel.hpp"

#include <wx/statbox.h>
#include <wx/treebook.h>

#ifdef _DEBUG
#include <serial/objostr.hpp>
#include <serial/serial.hpp>
#endif


////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*
 * CBioSourcePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CBioSourcePanel, wxPanel )


/*
 * CBioSourcePanel event table definition
 */

BEGIN_EVENT_TABLE( CBioSourcePanel, wxPanel )

////@begin CBioSourcePanel event table entries
    EVT_BUTTON( ID_ORGPANEL_BTN, CBioSourcePanel::OnTaxonomyLookup )
////@end CBioSourcePanel event table entries

END_EVENT_TABLE()


/*
 * CBioSourcePanel constructors
 */

static CBioSource dummy;

CBioSourcePanel::CBioSourcePanel() : m_Source(dummy)
{
    Init();
}


CBioSourcePanel::CBioSourcePanel(wxWindow* parent, objects::CBioSource& source, IDescEditorPanel* desc_editor, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) :
    m_Source(source), m_ParentPanel(desc_editor)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * OrganismNamePanel creator
 */

bool CBioSourcePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CBioSourcePanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    m_ParentTreebook = x_GetTree();
////@end CBioSourcePanel creation
    return true;
}


/*
 * CBioSourcePanel destructor
 */

CBioSourcePanel::~CBioSourcePanel()
{
////@begin CBioSourcePanel destruction
////@end CBioSourcePanel destruction
}


/*
 * Member initialisation
 */

void CBioSourcePanel::Init()
{
////@begin CBioSourcePanel member initialisation
    m_ScientificNameCtrl = NULL;
    m_DisableStrainForwardingBtn = NULL;
////@end CBioSourcePanel member initialisation
}

/*
 * Control creation for OrganismNamePanel
 */

void CBioSourcePanel::CreateControls()
{    
////@begin CBioSourcePanel content construction
    CBioSourcePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Scientific Name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ScientificNameCtrl = new CAutoCompleteTextCtrl(itemPanel1, ID_ORGPANEL_TXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(200, -1), wxTE_MULTILINE);
    itemBoxSizer3->Add(m_ScientificNameCtrl, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_ScientificNameCtrl->AutoComplete(new CTaxonTextCompleter());

    wxButton* itemButton6 = new wxButton( itemPanel1, ID_ORGPANEL_BTN, _("Taxonomy Lookup"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DisableStrainForwardingBtn = new wxCheckBox( itemPanel1, ID_ORGPANEL_CHCKBOX, _("Disable Strain Forwarding"), wxDefaultPosition, wxDefaultSize, 0 );
    m_DisableStrainForwardingBtn->SetValue(false);
    itemBoxSizer2->Add(m_DisableStrainForwardingBtn, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer8Static = new wxStaticBox(itemPanel1, ID_ORGPANEL_SIZER, _("Additional Modifiers"));
    wxStaticBoxSizer* itemStaticBoxSizer8 = new wxStaticBoxSizer(itemStaticBoxSizer8Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer8, 1, wxGROW|wxALL, 5);
    
    m_SrcModList = new CSrcModListPanel(itemStaticBoxSizer8->GetStaticBox(), m_Source, wxID_ANY, wxDefaultPosition, wxSize(100, 250), 0 );
    itemStaticBoxSizer8->Add(m_SrcModList, 1, wxGROW | wxALL, 5);

    m_ScientificNameCtrl->Bind(wxEVT_LEAVE_WINDOW, &CBioSourcePanel::OnUpdatedTaxname, this);
    m_ScientificNameCtrl->Bind(wxEVT_TEXT, &CBioSourcePanel::OnEvtText, this);
////@end CBioSourcePanel content construction
}

bool CBioSourcePanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;
    x_GetOrganismInfo();
    return true;
}

bool CBioSourcePanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;
    x_SetOrganismInfo();
    return true;
}


void CBioSourcePanel::x_GetOrganismInfo()
{
    if (m_Source.IsSetOrg()) {
        if (m_Source.GetOrg().IsSetTaxname()) {
            m_ScientificNameCtrl->ChangeValue(ToWxString(m_Source.GetOrg().GetTaxname()));
        }
        else {
            m_ScientificNameCtrl->ChangeValue(wxEmptyString);
        }

    }
    bool disable_strain_forwarding = m_Source.GetDisableStrainForwarding();
    m_DisableStrainForwardingBtn->SetValue(disable_strain_forwarding);
    m_DisableStrainForwardingBtn->Show(disable_strain_forwarding);
}

static void s_MoreCleanupForTaxnameChange(CBioSource& source)
{
    if (source.IsSetOrgname()) {
        COrgName& orgname = source.SetOrg().SetOrgname();
        if (orgname.IsSetName()) {
            orgname.ResetName();
        }
    }
    source.RemoveOrgMod(COrgMod::eSubtype_gb_acronym);
    source.RemoveOrgMod(COrgMod::eSubtype_gb_anamorph);
    source.RemoveOrgMod(COrgMod::eSubtype_gb_synonym);
}

void CBioSourcePanel::x_SetOrganismInfo() 
{
    string old_taxname;
    if (m_Source.IsSetOrg() && m_Source.GetOrg().IsSetTaxname()) {
        old_taxname = m_Source.GetOrg().GetTaxname();
    }
    string new_taxname = ToStdString(m_ScientificNameCtrl->GetValue());
    NStr::ReplaceInPlace(new_taxname, "\n", " ");
    NStr::ReplaceInPlace(new_taxname, "\r", " ");
    NStr::TruncateSpacesInPlace(new_taxname);
    m_Source.SetOrg().SetTaxname(new_taxname);
    m_SrcModList->TransferDataFromWindow();
    m_SrcModList->PopulateModifiers(m_Source);
    if (!NStr::IsBlank(old_taxname) && !NStr::Equal(old_taxname, new_taxname)) {
        edit::CleanupForTaxnameChange(m_Source);
        s_MoreCleanupForTaxnameChange(m_Source);
        m_SrcModList->SetSource(m_Source);
    }

    m_Source.SetDisableStrainForwarding(m_DisableStrainForwardingBtn->GetValue());
}


void CBioSourcePanel::PopulateBioSource(CBioSource& source)
{
    if (m_Source.IsSetOrg()) {
        source.SetOrg().Assign(m_Source.GetOrg());
    }
    m_SrcModList->PopulateModifiers(source);

#ifdef _DEBUG
    CNcbiOstrstream ostr;
    {{
        auto_ptr<CObjectOStream>
            out(CObjectOStream::Open(eSerial_AsnText, ostr));
        *out << m_Source;
    }}
    _TRACE(string(CNcbiOstrstreamToString(ostr)));
#endif

}

/*
 * Should we show tooltips?
 */

bool CBioSourcePanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CBioSourcePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CBioSourcePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CBioSourcePanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CBioSourcePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CBioSourcePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CBioSourcePanel icon retrieval
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
 */

void CBioSourcePanel::OnTaxonomyLookup( wxCommandEvent& event )
{
    x_SetOrganismInfo();
    if (!m_Source.IsSetOrg()) {
        return;
    }

    vector<CRef<COrg_ref> > rq_list;
    CRef<COrg_ref> org(new COrg_ref());
    org->Assign(m_Source.GetOrg());
    rq_list.push_back(org);

    CTaxon3 taxon3;
    taxon3.Init();
    CRef<CTaxon3_reply> reply = taxon3.SendOrgRefList(rq_list);
    if (reply) {
        CTaxon3_reply::TReply::const_iterator reply_it = reply->GetReply().begin();
        if ((*reply_it)->IsData()) {
            m_Source.SetOrg().Assign((*reply_it)->GetData().GetOrg());
            m_Source.SetOrg().ResetSyn();
            TransferDataToWindow();
            if (!m_ParentTreebook) {
                m_ParentTreebook = x_GetTree();
            }
            for (size_t i = 0; i < m_ParentTreebook->GetPageCount(); ++i) {
                CSourceOtherPanel* other_panel = dynamic_cast<CSourceOtherPanel*>(m_ParentTreebook->GetPage(i));
                if (other_panel) {
                    other_panel->TransferFromOrgRef(m_Source.GetOrg());
                }
                else {
                    m_ParentTreebook->GetPage(i)->TransferDataToWindow();
                }
            }
        }
    }
    event.Skip();
}

wxTreebook* CBioSourcePanel::x_GetTree()
{
    wxWindow* w = this->GetParent();
    while (w != NULL) {
        wxTreebook* parent = dynamic_cast<wxTreebook*>(w);
        if (parent) {
            return parent;
        }
        else {
            w = w->GetParent();
        }
    }
    return NULL;
}

void CBioSourcePanel::OnUpdatedTaxname (wxMouseEvent& event)
{
    if (m_ScientificNameCtrl->IsModified()) {
        m_SrcModList->OnChangedTaxname();
        m_ScientificNameCtrl->SetModified(false);
    }
}

void CBioSourcePanel::OnEvtText(wxCommandEvent& event)
{
    if (m_ParentPanel) {
        m_ParentPanel->UpdateTaxname();
    }
    else if (m_ParentTreebook) {
        for (size_t i = 0; i < m_ParentTreebook->GetPageCount(); ++i) {
            if (CSourceOtherPanel* otherPanel = dynamic_cast<CSourceOtherPanel*>(m_ParentTreebook->GetPage(i))) {
                otherPanel->OnChangedTaxname();
            }
            else if (CLocAndGCodePanel* loc_gc_panel = dynamic_cast<CLocAndGCodePanel*>(m_ParentTreebook->GetPage(i))) {
                loc_gc_panel->OnChangedTaxname();
            }
        }
    }
}


END_NCBI_SCOPE
