/*  $Id: srcmod_list_panel.cpp 43642 2019-08-13 14:41:50Z asztalos $
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

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <ncbi_pch.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/hyperlink.hpp>
#include "srcmod_list_panel.hpp"
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objects/seqfeat/OrgMod.hpp>
#include <objects/seqfeat/SubSource.hpp>

#include <wx/checkbox.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*
 * CSrcModListPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSrcModListPanel, wxPanel )


/*
 * CSrcModListPanel event table definition
 */

BEGIN_EVENT_TABLE( CSrcModListPanel, wxPanel )
////@begin CSrcModListPanel event table entries
    EVT_HYPERLINK( ID_CLR_SRC_NOTE, CSrcModListPanel::OnClrSrcNoteHyperlinkClicked )
    EVT_HYPERLINK( ID_CLR_ORG_NOTE, CSrcModListPanel::OnClrOrgNoteHyperlinkClicked )

    EVT_HYPERLINK(wxID_ANY, CSrcModListPanel::OnDelete)
    EVT_CHECKBOX(ID_PCR_CHECKBOX, CSrcModListPanel::OnPCRCheckBox)
    EVT_BUTTON(ID_ADD_PCR_PRIMER, CSrcModListPanel::OnAddPCRPrimer)
////@end CSrcModListPanel event table entries
END_EVENT_TABLE()

const int kCtrlColNum = 2;

/*
 * CSrcModListPanel constructors
 */

CSrcModListPanel::CSrcModListPanel():
    m_Source(NULL)
{
    Init();
}

CSrcModListPanel::CSrcModListPanel( wxWindow* parent, CBioSource& source, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ):
    m_Source(new CBioSource())
{
    m_Source->Assign(source);
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CSrcModListPanel creator
 */

bool CSrcModListPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSrcModListPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSrcModListPanel creation
    return true;
}


/*
 * CSrcModListPanel destructor
 */

CSrcModListPanel::~CSrcModListPanel()
{
////@begin CSrcModListPanel destruction
////@end CSrcModListPanel destruction
}


/*
 * Member initialisation
 */

void CSrcModListPanel::Init()
{
////@begin CSrcModListPanel member initialisation
    m_ScrolledWindow = NULL;
    m_Sizer = NULL;
    m_SrcInfo = NULL;
    m_OrgNote = NULL;
////@end CSrcModListPanel member initialisation
    m_PCRPrimersBoxSizer1 = NULL;
    m_PCRPrimersBoxSizer2 = NULL;
    m_PCRPrimersBoxSizer3 = NULL;
    m_PCRCheckbox = NULL;
    m_PCRPrimersPanel = NULL;
}


/*
 * Control creation for CSrcModListPanel
 */

void CSrcModListPanel::CreateControls()
{    
////@begin CSrcModListPanel content construction
    CSrcModListPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_TOP|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_ScrolledWindow = new wxScrolledWindow( itemPanel1, ID_SCROLLEDWINDOW1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemBoxSizer4->Add(m_ScrolledWindow, 1, wxGROW|wxTOP|wxBOTTOM|wxFIXED_MINSIZE|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 0);
    m_ScrolledWindow->SetScrollbars(1, 1, 0, 0);

    m_Sizer = new wxFlexGridSizer(0, kCtrlColNum, 0, 0);
    m_ScrolledWindow->SetSizer(m_Sizer);
    m_ScrolledWindow->FitInside();

    itemBoxSizer3->Add(530, 0, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer9 = new wxFlexGridSizer(0, 3, 0, 0);
    itemBoxSizer3->Add(itemFlexGridSizer9, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText10 = new wxStaticText( itemPanel1, wxID_STATIC, _("Source Note"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
    itemFlexGridSizer9->Add(itemStaticText10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SrcInfo = new CRichTextCtrl( itemPanel1, ID_TEXTCTRL7, wxEmptyString, wxDefaultPosition, wxSize(350, -1), wxTE_MULTILINE );
    itemFlexGridSizer9->Add(m_SrcInfo, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxHyperlinkCtrl* itemHyperlinkCtrl12 = new wxHyperlinkCtrl( itemPanel1, ID_CLR_SRC_NOTE, _("Clear"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemFlexGridSizer9->Add(itemHyperlinkCtrl12, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText13 = new wxStaticText( itemPanel1, wxID_STATIC, _("Organism Note"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer9->Add(itemStaticText13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_OrgNote = new CRichTextCtrl( itemPanel1, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
    itemFlexGridSizer9->Add(m_OrgNote, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxHyperlinkCtrl* itemHyperlinkCtrl15 = new wxHyperlinkCtrl( itemPanel1, ID_CLR_ORG_NOTE, _("Clear"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemFlexGridSizer9->Add(itemHyperlinkCtrl15, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer2->Add(0, 500, 0, wxALIGN_TOP|wxALL, 0);

////@end CSrcModListPanel content construction
    m_TotalHeight = 0;
    m_TotalWidth = 0;
    m_NumRows = 0;
    m_ScrollRate = 0;

// PCR-Primers
    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer9, 1, wxGROW|wxALL, 5);
    
    
    m_PCRCheckbox = new wxCheckBox(itemPanel1, ID_PCR_CHECKBOX, wxT("PCR Primers"), wxDefaultPosition, wxDefaultSize);
    if (m_Source->IsSetPcr_primers())
        m_PCRCheckbox->SetValue(true);
    else
        m_PCRCheckbox->SetValue(false);
    itemBoxSizer9->Add(m_PCRCheckbox, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
   
    m_PCRPrimersBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer9->Add(m_PCRPrimersBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);
 
    m_PCRPrimersBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer9->Add(m_PCRPrimersBoxSizer1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    m_PCRPrimersBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer9->Add(m_PCRPrimersBoxSizer2, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

   
    
    wxStaticText* itemStaticTextPCRSet = new wxStaticText( this, wxID_STATIC, _("Set"), wxDefaultPosition, wxSize(25,-1), wxALIGN_CENTRE );
    m_PCRPrimersBoxSizer1->Add(itemStaticTextPCRSet, 0, wxALL, 0);
    
    wxStaticText* itemStaticTextPCRDir = new wxStaticText( this, wxID_STATIC, _("Direction"), wxDefaultPosition, wxSize(80,-1), wxALIGN_CENTRE );
    m_PCRPrimersBoxSizer1->Add(itemStaticTextPCRDir, 0, wxALL, 0);
    
    wxStaticText* itemStaticText14 = new wxStaticText( this, wxID_STATIC, _("Name"), wxDefaultPosition, wxSize(80,-1), wxALIGN_CENTRE );
    m_PCRPrimersBoxSizer1->Add(itemStaticText14, 0, wxALL, 0);
    
    wxStaticText* itemStaticText15 = new wxStaticText( this, wxID_STATIC, _("Sequence"), wxDefaultPosition, wxSize(80,-1), wxALIGN_CENTRE );
    m_PCRPrimersBoxSizer1->Add(itemStaticText15, 0, wxALL, 0);
    
    wxStaticText* itemStaticText16 = new wxStaticText( this, wxID_STATIC, _("      "), wxDefaultPosition, wxSize(80,-1), wxALIGN_CENTRE );
    m_PCRPrimersBoxSizer1->Add(itemStaticText16, 0, wxALL, 0);
    
    m_PCRPrimersPanel = new CPCRPrimersPanel( this, m_Source.GetPointer() );
    m_PCRPrimersBoxSizer2->Add(m_PCRPrimersPanel,  1, wxGROW|wxTOP|wxBOTTOM|wxFIXED_MINSIZE|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 0); 

    wxButton* itemButton1 = new wxButton( this, ID_ADD_PCR_PRIMER, _("Add Another Primer"), wxDefaultPosition, wxDefaultSize, 0 );
    m_PCRPrimersBoxSizer3->Add(itemButton1, 0, wxALL, 5);
    
    x_ShowPCRPrimerItems(m_PCRCheckbox->IsChecked());
    m_PCRPrimersPanel->TransferDataToWindow();
}

void CSrcModListPanel::x_ShowPCRPrimerItems(bool value)
{
    m_PCRPrimersBoxSizer1->ShowItems(value);
    m_PCRPrimersBoxSizer2->ShowItems(value);
    m_PCRPrimersBoxSizer3->ShowItems(value);
    Layout();
    Refresh();
}

void CSrcModListPanel::OnPCRCheckBox(wxCommandEvent& event)
{
    x_ShowPCRPrimerItems(m_PCRCheckbox->IsChecked());
}

void CSrcModListPanel::OnAddPCRPrimer(wxCommandEvent& event)
{
    m_PCRPrimersPanel->x_AddEmptyRow();
    m_PCRPrimersPanel->x_AdjustScrollWindow();
}

bool CSrcModListPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    m_SrcInfo->Clear();
    m_OrgNote->Clear();

    x_FillDataList();

    m_SrcInfo->SetValue(ToWxString(m_SrcNoteText));
    m_OrgNote->SetValue(ToWxString(m_OrgNoteText));

    x_AddTextSrcMods();

    return true;
}

static bool s_IsReadOnly(COrgMod::TSubtype subtype)
{
    if (subtype == COrgMod::eSubtype_gb_synonym
        || subtype == COrgMod::eSubtype_gb_acronym
        || subtype == COrgMod::eSubtype_gb_anamorph
        || subtype == COrgMod::eSubtype_type_material) {
        return true;
    }
    return false;
}

void CSrcModListPanel::x_FillDataList()
{
    m_Data.clear();
    m_OrgNoteText.clear();
    m_SrcNoteText.clear();
    m_ReadOnlyQuals.clear();

    if (m_Source->IsSetSubtype()) {
        ITERATE(CBioSource::TSubtype, src_it, m_Source->GetSubtype()) {
            if ((*src_it)->GetSubtype() == CSubSource::eSubtype_other) {
                if (!NStr::IsBlank(m_SrcNoteText)) {
                    m_SrcNoteText += "; ";
                }
                m_SrcNoteText += (*src_it)->GetName();
            }
            else {
                CSrcModPanel::SModData data;
                data.name = ToWxString(CSubSource::GetSubtypeName((*src_it)->GetSubtype()));
                data.value = ToWxString((*src_it)->GetName());
                m_Data.push_back(data);
            }
        }
    }

    if (m_Source->IsSetOrg() && m_Source->GetOrg().IsSetOrgname() &&
        m_Source->GetOrg().GetOrgname().IsSetMod())
    {
        ITERATE(COrgName::TMod, src_it, m_Source->GetOrg().GetOrgname().GetMod())
        {
            if ((*src_it)->GetSubtype() == COrgMod::eSubtype_other) {
                if (!NStr::IsBlank(m_OrgNoteText)) {
                    m_OrgNoteText += "; ";
                }
                m_OrgNoteText += (*src_it)->GetSubname();
            }
            else if (s_IsReadOnly((*src_it)->GetSubtype())) {
                CRef<COrgMod> om(new COrgMod());
                om->Assign(**src_it);
                m_ReadOnlyQuals.push_back(om);
            }
            else {
                CSrcModPanel::SModData data;
                data.name = ToWxString(COrgMod::GetSubtypeName((*src_it)->GetSubtype()));
                data.value = ToWxString((*src_it)->GetSubname());
                m_Data.push_back(data);
            }
        }
    }
}


static bool s_IsNonText (string label)
{
    if (NStr::EqualNocase (label, "Environmental-Sample")
      || NStr::EqualNocase (label, "Germline")
      || NStr::EqualNocase (label, "Metagenomic")
      || NStr::EqualNocase (label, "Rearranged")
      || NStr::EqualNocase (label, "Transgenic")
      || NStr::EqualNocase (label, "Metagenomic")) {
        return true;
    } else {
        return false;
    }
}


void CSrcModListPanel::x_AddTextSrcMods ()
{
    Freeze();	
    m_TotalHeight = 0;
    int num_text = 0;

    wxSizerItemList& children = m_Sizer->GetChildren();
    wxSizerItemList::iterator node = children.begin();
    for (;  node != children.end();  ++node)
        (**node).DeleteWindows();
    m_Sizer->Clear(true);

    for (auto&& it : m_Data) {
        if (NStr::EqualNocase(it.name, "note")) {
            //save note for later
        } else if (s_IsNonText(ToStdString(it.name))) {
            if (NStr::IsBlank(ToStdString(it.value))) {
                x_AddRow(it.name, wxT("TRUE"));
            } else {
                x_AddRow(it.name, it.value);
            }
            num_text++;
        } else {
                  x_AddRow(it.name, it.value);
            num_text++;
            }
    }
    x_AddEmptyRow ();

    //Adjusting size of the scroll window
 
    int m_MinHeight = 6 * (m_TotalHeight / (num_text + 1));    
    m_ScrolledWindow->SetVirtualSize(m_TotalWidth, m_TotalHeight);
    m_ScrolledWindow->SetScrollRate(0, m_ScrollRate); 
    m_ScrolledWindow->FitInside();   
//    m_ScrolledWindow->SetSize(m_TotalWidth + 20, m_MinHeight);
    m_ScrolledWindow->SetMinSize(wxSize(m_TotalWidth + 20, m_MinHeight));
    Layout();
    Thaw();
}

void CSrcModListPanel::x_AddRow(wxString name, wxString value)
{
    CSrcModPanel::SModData data;
    data.name = name;
    data.value = value;

    CSrcModPanel* panel = new CSrcModPanel(m_ScrolledWindow, data);
    panel->SetModifierData();
    m_Sizer->Add(panel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    wxHyperlinkCtrl* itemHyperlinkCtrl = new CHyperlink( m_ScrolledWindow, wxID_ANY, 
              wxT("Delete"), wxT(""), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    m_Sizer->Add(itemHyperlinkCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    
    int row_width;
    int row_height;
    panel->GetClientSize(&row_width, &row_height);

    int link_width;
    int link_height;
    itemHyperlinkCtrl->GetClientSize (&link_width, &link_height);
    if (link_height > row_height) {
      row_height = link_height;
    }
    row_width += link_width;

    if (row_width > m_TotalWidth) {
        m_TotalWidth = row_width;
    }

    m_TotalHeight += row_height;
    m_ScrollRate = row_height;
    m_NumRows++;
}

void CSrcModListPanel::x_AddEmptyRow()
{
    x_AddRow(wxEmptyString, wxEmptyString);
}

bool CSrcModListPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    //Reset modifiers
    if (m_Source->IsSetSubtype()) {
        m_Source->ResetSubtype();
    }
    if (m_Source->IsSetOrgMod()) {
        m_Source->SetOrg().SetOrgname().ResetMod();
    }

    string src_note = ToStdString(m_SrcInfo->GetValue());
    if (!NStr::IsBlank(src_note)) {
        x_SetSrcSubMod(CSubSource::eSubtype_other, src_note);
    }
    string org_note = ToStdString(m_OrgNote->GetValue());
    if (!NStr::IsBlank(org_note)) {
        x_SetSrcOrgMod(COrgMod::eSubtype_other, org_note);
    }
    x_CollectTextMods ();

    return true;
}

void CSrcModListPanel::x_SetSrcOrgMod(int subtype, const string& value)
{
    CRef<COrgMod> org_mod(new COrgMod(subtype, value)); 
    m_Source->SetOrg().SetOrgname().SetMod().push_back(org_mod);
}


void CSrcModListPanel::x_SetSrcSubMod(int subtype, const string& value)
{
    CRef<CSubSource> sub_src (new CSubSource(subtype, value));
    m_Source->SetSubtype().push_back(sub_src);
}

void CSrcModListPanel::x_CollectTextMods()
{
    wxSizerItemList& children = m_Sizer->GetChildren();
    wxSizerItemList::iterator node = children.begin();
    for (; node != children.end(); ++node) {
        wxWindow *w = (*node)->GetWindow();
        if (!w) {
            continue;
        }
        CSrcModPanel* singlemod_panel = dynamic_cast<CSrcModPanel*> (w);
        if (singlemod_panel) {
            CSrcModPanel::SModData single_modifier_data = singlemod_panel->GetModifierData();
            const string qual_name = ToStdString(single_modifier_data.name);
            const string qual_value = ToStdString(single_modifier_data.value);
            if (!NStr::IsBlank(qual_name) && !NStr::IsBlank(qual_value)) {
                if (s_IsNonText(qual_name) && NStr::EqualNocase(qual_value, "TRUE")) {
                    x_SetBioSourceModifier(qual_name, " ");
                }
                else {
                    x_SetBioSourceModifier(qual_name, qual_value);
                }
            }
        }
    }
}

void CSrcModListPanel::x_SetBioSourceModifier(const string& name, const string& value)
{
    if (CSubSource::IsValidSubtypeName(name)) {
        x_SetSrcSubMod(CSubSource::GetSubtypeValue(name), value);
    }
    else if (COrgMod::IsValidSubtypeName(name)) {
        x_SetSrcOrgMod(COrgMod::GetSubtypeValue(name), value);
    }
}

void CSrcModListPanel::x_UpdateList()
{
    wxSizerItemList& children = m_Sizer->GetChildren();
    wxSizerItemList::iterator node = children.begin();
    m_Data.clear();
    while (node != children.end()) {
        // add new items
        wxWindow *w = (*node)->GetWindow();
        if (!w) 
        {
            continue;
        }
        CSrcModPanel* singlemod_panel = dynamic_cast<CSrcModPanel*> (w);
        if (singlemod_panel) 
        {
            CSrcModPanel::SModData single_modifier_data = singlemod_panel->GetModifierData();
            if (!NStr::IsBlank(ToStdString(single_modifier_data.name)) || 
                !NStr::IsBlank(ToStdString(single_modifier_data.value)))
            {
                m_Data.push_back(single_modifier_data);
            }
        }
        node++;
    }
}


void CSrcModListPanel::PopulateModifiers(CBioSource& biosrc)
{
    biosrc.ResetSubtype();
    if (biosrc.IsSetOrg() && biosrc.GetOrg().IsSetOrgname()) {
        biosrc.SetOrg().SetOrgname().ResetMod();
    }
    if (m_Source->IsSetSubtype()) {
        ITERATE(CBioSource::TSubtype, src_it, m_Source->GetSubtype()) {
            CRef<CSubSource> sub_src (new CSubSource());
            sub_src->SetSubtype((*src_it)->GetSubtype());
            sub_src->SetName((*src_it)->GetName());
            biosrc.SetSubtype().push_back(sub_src);
        }
    }
    
    if (m_Source->IsSetOrgMod())
    {	
        biosrc.SetOrg().SetOrgname().ResetMod();
        ITERATE(COrgName::TMod, src_it, m_Source->GetOrg().GetOrgname().GetMod())
        {
            CRef<COrgMod> org_mod(new COrgMod()); 
            org_mod->SetSubtype((*src_it)->GetSubtype()); 
            org_mod->SetSubname((*src_it)->GetSubname()); 
            biosrc.SetOrg().SetOrgname().SetMod().push_back(org_mod);
        }
    }
    if (!m_ReadOnlyQuals.empty()) {
        for (auto&& it : m_ReadOnlyQuals) {
            CRef<COrgMod> org_mod(new COrgMod());
            org_mod->Assign(*it);
            biosrc.SetOrg().SetOrgname().SetMod().push_back(org_mod);
        }
    }
    if (m_PCRPrimersPanel) {
        m_PCRPrimersPanel->TransferDataFromWindow();
        m_PCRPrimersPanel->UpdateBioSourcePrimers(biosrc);
    }
}

void CSrcModListPanel::SetSource(const CBioSource& biosrc)
{
    m_Source->Assign(biosrc);
    TransferDataToWindow();
}


static bool s_IsLastRow(wxWindow* wnd, wxSizerItemList& itemList)
{
    bool is_last = false;
    if (NULL == wnd)
        return false;

    for(wxSizerItemList::iterator it = itemList.begin(); it != itemList.end(); ++it) {
        wxWindow* child = (**it).GetWindow();
        if (child && child == wnd) {
            is_last = true;
        } else {
            if (is_last) {
                CSrcModPanel* qual = dynamic_cast<CSrcModPanel*> (child);
                    if (qual) {
                    is_last = false;
                    return is_last;
                }
            }
        }
    }

    return is_last;
}


void CSrcModListPanel::AddLastSrcMod (wxWindow* link)
{
    // find the row that holds the author to be deleted
    wxSizerItemList& itemList = m_Sizer->GetChildren();

    if (s_IsLastRow(link, itemList)) {
        x_AddEmptyRow ();
        m_ScrolledWindow->SetVirtualSize(m_TotalWidth + 10, m_TotalHeight);
        m_ScrolledWindow->SetScrollRate(0, m_ScrollRate);
        m_ScrolledWindow->FitInside();
    }
}

void CSrcModListPanel::OnDelete(wxHyperlinkEvent& event)
{
    wxWindow* wnd = (wxWindow*)event.GetEventObject();
    if (wnd == NULL) {
        return;
    }

    wxSizerItemList& itemList = m_Sizer->GetChildren();
    wxSizerItemList::iterator item_it = x_FindRow(wnd, itemList);

    for (int i = 0; item_it != itemList.end() && i < kCtrlColNum; ++i) {
        (**item_it).DeleteWindows();
        item_it = itemList.erase(item_it);
    }

    if (itemList.empty()) {
        x_AddEmptyRow();
    }
    m_ScrolledWindow->FitInside();
}

wxSizerItemList::iterator CSrcModListPanel::x_FindRow(wxWindow* wnd, wxSizerItemList& itemList)
{
    if (wnd == nullptr)
        return itemList.end();

    wxSizerItemList::iterator row = itemList.end(), it = itemList.begin();

    for (; it != itemList.end(); ++it) {
        wxWindow* child = (**it).GetWindow();
        if (child) {
            if (dynamic_cast<CSrcModPanel*> (child)) {
                row = it;
            }
            if (child == wnd)
                break;
        }
    }
    return row;
}

/*
 * Should we show tooltips?
 */

bool CSrcModListPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CSrcModListPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSrcModListPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSrcModListPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CSrcModListPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSrcModListPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSrcModListPanel icon retrieval
}

void CSrcModListPanel::OnChangedTaxname(void)
{
    x_UpdateList();
    auto dit = m_Data.begin();
    while (dit != m_Data.end())
    {
        if (NStr::EqualNocase(ToStdString(dit->name), "old-name"))
        {
            dit = m_Data.erase(dit);
        }
        else {
            ++dit;
        }
    }
    auto qual_it = m_ReadOnlyQuals.begin();
    while (qual_it != m_ReadOnlyQuals.end()) {
        if ((*qual_it)->IsSetSubtype() && (*qual_it)->GetSubtype() == COrgMod::eSubtype_type_material) {
            qual_it = m_ReadOnlyQuals.erase(qual_it);
        }
        else {
            ++qual_it;
        }
    }

    x_AddTextSrcMods();
}

/*
 * wxEVT_COMMAND_HYPERLINK event handler for ID_CLR_SRC_NOTE
 */

void CSrcModListPanel::OnClrSrcNoteHyperlinkClicked( wxHyperlinkEvent& event )
{
    m_SrcInfo->Clear();
}


/*
 * wxEVT_COMMAND_HYPERLINK event handler for ID_CLR_ORG_NOTE
 */

void CSrcModListPanel::OnClrOrgNoteHyperlinkClicked( wxHyperlinkEvent& event )
{
    m_OrgNote->Clear(); 
}


END_NCBI_SCOPE
