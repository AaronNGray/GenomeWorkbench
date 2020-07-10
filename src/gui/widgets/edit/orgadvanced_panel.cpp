/*  $Id: orgadvanced_panel.cpp 43218 2019-05-29 19:32:16Z asztalos $
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software / database is a "United States Government Work" under the
*  terms of the United States Copyright Act.It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.This software / database is freely available
*  to the public for use.The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data.The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Authors: Andrea Asztalos
*
*/

#include <ncbi_pch.hpp>
#include <gui/widgets/wx/unfocused_controls.hpp>
#include <gui/widgets/edit/single_assemblymethod.hpp>
#include <gui/widgets/edit/orgadvanced_panel.hpp>
#include <gui/widgets/edit/srcmod_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

// for command for changing descriptors
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_set_handle.hpp>

#include <objects/seqfeat/SubSource.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objects/seqfeat/OrgMod.hpp>

#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/stattext.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE

class COrgAdvancedMod : public CSrcModPanel
{
public:
    COrgAdvancedMod() : CSrcModPanel() {}
    COrgAdvancedMod(wxWindow* parent, SModData mod_data) : CSrcModPanel(parent, mod_data) {}

    virtual bool DoNotListOrgMod(int st);
    virtual bool DoNotListSubSrc(int st);

};


bool s_DoNotListOrgMod(int st)
{
    if (st == objects::COrgMod::eSubtype_old_lineage || 
        st == objects::COrgMod::eSubtype_old_name || 
        st == objects::COrgMod::eSubtype_other) {
        return true;
    }
    return objects::COrgMod::IsDiscouraged(st, true);

}


bool COrgAdvancedMod::DoNotListOrgMod(int st)
{
    return s_DoNotListOrgMod(st);
}


bool s_DoNotListSubSrc(int st)
{
    if (st >= objects::CSubSource::eSubtype_fwd_primer_seq && st <= objects::CSubSource::eSubtype_rev_primer_name) {
        return true;
    }
    if (st == objects::CSubSource::eSubtype_plasmid_name ||
        st == objects::CSubSource::eSubtype_chromosome || 
        st == objects::CSubSource::eSubtype_other) {
        return true;
    }
    return objects::CSubSource::IsDiscouraged(st);
}


bool COrgAdvancedMod::DoNotListSubSrc(int st)
{
    return s_DoNotListSubSrc(st);
}


/*
 * COrgAdvancedPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( COrgAdvancedPanel, wxPanel )


/*
 * COrgAdvancedPanel event table definition
 */

BEGIN_EVENT_TABLE( COrgAdvancedPanel, wxPanel )

////@begin COrgAdvancedPanel event table entries
    EVT_HYPERLINK( ID_ORGADVHYPERLINK, COrgAdvancedPanel::OnAddAttributeHyperlinkClicked )
    EVT_HYPERLINK( ID_ORGADVDELETE, COrgAdvancedPanel::OnDeleteQualifier)
////@end COrgAdvancedPanel event table entries

END_EVENT_TABLE()


/*
 * COrgAdvancedPanel constructors
 */

COrgAdvancedPanel::COrgAdvancedPanel()
{
    Init();
}

COrgAdvancedPanel::COrgAdvancedPanel( wxWindow* parent, 
    ICommandProccessor* cmdproc,
    objects::CSeq_entry_Handle seh,
    wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_CmdProcessor(cmdproc), m_Seh(seh)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * COrgAdvancedPanel creator
 */

bool COrgAdvancedPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin COrgAdvancedPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end COrgAdvancedPanel creation
    return true;
}


/*
 * COrgAdvancedPanel destructor
 */

COrgAdvancedPanel::~COrgAdvancedPanel()
{
////@begin COrgAdvancedPanel destruction
////@end COrgAdvancedPanel destruction
}


/*
 * Member initialisation
 */

void COrgAdvancedPanel::Init()
{
////@begin COrgAdvancedPanel member initialisation
    m_ScrolledWindow = NULL;
    m_Sizer = NULL;
////@end COrgAdvancedPanel member initialisation
}


/*
 * Control creation for COrgAdvancedPanel
 */

void COrgAdvancedPanel::CreateControls()
{    
////@begin COrgAdvancedPanel content construction
    COrgAdvancedPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer2, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer2->Add(itemBoxSizer3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Attribute name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText5, 0, wxALIGN_LEFT |wxALL, 5);

#ifdef __WXMSW__
    itemBoxSizer3->Add(60, 5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
#elif __WXOSX_COCOA__
    itemBoxSizer3->Add(80, 5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
#else
    itemBoxSizer3->Add(95, 5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
#endif
    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("Attribute value"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText7, 0, wxALIGN_LEFT |wxALL, 5);

    m_ScrolledWindow = new wxScrolledWindow( itemPanel1, ID_ORGADVSCROLLEDWND, wxDefaultPosition, wxSize(530, 92), wxTAB_TRAVERSAL);
    itemFlexGridSizer2->Add(m_ScrolledWindow, 0, wxALIGN_LEFT |wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_ScrolledWindow->SetScrollbars(1, 1, 0, 0);
   
    m_Sizer = new wxFlexGridSizer(0, 2, 0, 0);
    m_ScrolledWindow->SetSizer(m_Sizer);

    wxHyperlinkCtrl* itemHyperlinkCtrl2 = new wxHyperlinkCtrl( itemPanel1, ID_ORGADVHYPERLINK, _("Add another attribute"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemFlexGridSizer2->Add(itemHyperlinkCtrl2, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end COrgAdvancedPanel content construction
}

void COrgAdvancedPanel::x_AddEmptyRow()
{
    COrgAdvancedMod::SModData data{ wxEmptyString, wxEmptyString };
    COrgAdvancedMod* row = new COrgAdvancedMod(m_ScrolledWindow, data);
    if (m_RowHeight == 0) {
        int row_width;
        row->GetClientSize(&row_width, &m_RowHeight);
    }
    x_AddRowToWindow(row);
}

void COrgAdvancedPanel::x_AddRowToWindow(wxWindow* row)
{
    m_Sizer->Add(row, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 0);
    wxHyperlinkCtrl* itemHyperLink = new CNoTabHyperlinkCtrl(m_ScrolledWindow, ID_ORGADVDELETE, wxT("Delete"), wxT("delete"));
    itemHyperLink->SetVisitedColour(itemHyperLink->GetNormalColour());
    m_Sizer->Add(itemHyperLink, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 0);
#ifdef __WXMSW__
    m_ScrolledWindow->SetScrollRate(0, m_RowHeight);
    m_ScrolledWindow->FitInside();
    m_ScrolledWindow->Refresh();
#else 
    int row_width, row_height;
    row->GetClientSize(&row_width, &row_height);

    int link_width, link_height;
    itemHyperLink->GetClientSize(&link_width, &link_height);
    if (link_height > row_height) {
        row_height = link_height;
    }
    row_width += link_width;

    if (row_width > m_TotalWidth) {
        m_TotalWidth = row_width;
    }

    m_TotalHeight += row_height;
    m_ScrollRate = row_height;

    m_ScrolledWindow->SetVirtualSize(m_TotalWidth, m_TotalHeight);
    m_ScrolledWindow->SetScrollRate(0, m_ScrollRate);
    m_ScrolledWindow->FitInside();
    m_ScrolledWindow->SetMinSize(wxSize(m_TotalWidth + 20, m_MaxRows * m_ScrollRate));
    m_ScrolledWindow->Refresh();
#endif
}


/*
 * Should we show tooltips?
 */

bool COrgAdvancedPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap COrgAdvancedPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin COrgAdvancedPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end COrgAdvancedPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon COrgAdvancedPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin COrgAdvancedPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end COrgAdvancedPanel icon retrieval
}


/*
 * wxEVT_COMMAND_HYPERLINK event handler for ID_ORGADVHYPERLINK
 */

void COrgAdvancedPanel::OnAddAttributeHyperlinkClicked( wxHyperlinkEvent& event )
{
    x_AddEmptyRow();
    int x, y;
    m_ScrolledWindow->GetVirtualSize(&x, &y);

    int xUnit, yUnit;
    m_ScrolledWindow->GetScrollPixelsPerUnit(&xUnit, &yUnit);
    m_ScrolledWindow->Scroll(0, y / yUnit);
}

void COrgAdvancedPanel::OnDeleteQualifier(wxHyperlinkEvent& event)
{
    wxWindow *win = dynamic_cast<wxWindow*>(event.GetEventObject());
    if (!win)
        return;
    wxWindowList &children = m_ScrolledWindow->GetChildren();
    wxWindow *prev = NULL;
    for (wxWindowList::iterator child = children.begin(); child != children.end(); ++child)
    {
        if (*child == win)
        {
            wxSizer *sizer = win->GetContainingSizer();
            win->Destroy();
            prev->Destroy();
            m_Sizer->Remove(sizer);
            m_Sizer->Layout();
            m_ScrolledWindow->FitInside();
            break;
        }
        prev = *child;
    }
}

void COrgAdvancedPanel::ApplyBioSource(objects::CBioSource& src)
{
    m_Source.Reset(&src);
    TransferDataToWindow();
}


void COrgAdvancedPanel::x_Reset()
{
    for (auto it : m_ScrolledWindow->GetChildren()) {
        COrgAdvancedMod* mod = dynamic_cast<COrgAdvancedMod *>(it);
        if (mod) {
            mod->SetModifierData(kEmptyStr, kEmptyStr);
        }
    }

}

bool COrgAdvancedPanel::TransferDataToWindow()
{
    size_t num_rows_needed = 0;
    size_t num_subsrc = 0;
    size_t num_orgmod = 0;
    if (m_Source) {
        // first count how many we need
        if (m_Source->IsSetSubtype()) {
            for (auto it : m_Source->GetSubtype()) {
                if (it->IsSetSubtype() && 
                    !s_DoNotListSubSrc(it->GetSubtype()) &&
                    (objects::CSubSource::NeedsNoText(it->GetSubtype())
                                           || (it->IsSetName() && !NStr::IsBlank(it->GetName())))) {
                    num_subsrc++;
                }
            }
            num_rows_needed += num_subsrc;
        }
        if (m_Source->IsSetOrg() && m_Source->GetOrg().IsSetOrgname() && m_Source->GetOrg().GetOrgname().IsSetMod()) {
            for (auto it : m_Source->GetOrg().GetOrgname().GetMod()) {
                if (it->IsSetSubtype() && 
                    !s_DoNotListOrgMod(it->GetSubtype()) &&
                    it->IsSetSubname() && !NStr::IsBlank(it->GetSubname())) {
                    num_orgmod++;
                }
            }
            num_rows_needed += num_orgmod;
        }
    }
    size_t num_rows = 0;
    for (auto rit : m_ScrolledWindow->GetChildren()) {
        COrgAdvancedMod* row = dynamic_cast<COrgAdvancedMod *>(rit);
        if (row) {
            num_rows++;
        }
    }

    Freeze();
    while (num_rows < num_rows_needed) {
        x_AddEmptyRow();
        num_rows++;
    }

    wxWindowList &children = m_ScrolledWindow->GetChildren();
    auto rit = children.begin();
    auto end = children.end();

    if (num_subsrc > 0) {
        for (auto it : m_Source->GetSubtype()) {
            if (it->IsSetSubtype() &&
                !s_DoNotListSubSrc(it->GetSubtype()) &&
                (objects::CSubSource::NeedsNoText(it->GetSubtype())
                || (it->IsSetName() && !NStr::IsBlank(it->GetName())))) {
                COrgAdvancedMod* row = dynamic_cast<COrgAdvancedMod *>(*rit);
                while (!row) {
                    rit++;
                    row = dynamic_cast<COrgAdvancedMod *>(*rit);
                }
                string value = it->IsSetName() ? it->GetName() : "TRUE";
                row->SetModifierData(objects::CSubSource::GetSubtypeName(it->GetSubtype()), value);
                rit++;
            }
        }
    }
    if (num_orgmod > 0) {
        for (auto it : m_Source->GetOrg().GetOrgname().GetMod()) {
            if (it->IsSetSubtype() && 
                ! s_DoNotListOrgMod(it->GetSubtype()) &&
                it->IsSetSubname() && !NStr::IsBlank(it->GetSubname())) {
                COrgAdvancedMod* row = dynamic_cast<COrgAdvancedMod *>(*rit);
                while (!row) {
                    rit++;
                    row = dynamic_cast<COrgAdvancedMod *>(*rit);
                }
                row->SetModifierData(objects::COrgMod::GetSubtypeName(it->GetSubtype()), it->GetSubname());
                rit++;
            }
        }

    }

    while (rit != end) {
        if (COrgAdvancedMod* row = dynamic_cast<COrgAdvancedMod *>(*rit)) {
            auto current = rit;
            ++rit; ++rit;
            wxSizer *sizer = row->GetContainingSizer();
            ++current;
            row->Destroy();
            (*current)->Destroy();
            m_Sizer->Remove(sizer);
            m_Sizer->Layout();
        }
        else {
            ++rit;
        }
    }

    if (m_Sizer->IsEmpty()) {
        for (size_t index = 0; index < m_Rows; ++index) {
            x_AddEmptyRow();
        }
    }
    m_ScrolledWindow->FitInside();
    Thaw();
    return true;
}


static bool s_IsNonText(const string& label)
{
    bool rval = false;
    if (objects::CSubSource::IsValidSubtypeName(label)) {
        auto subtype = objects::CSubSource::GetSubtypeValue(label);
        rval = objects::CSubSource::NeedsNoText(subtype);
    }
    return rval;
}


void COrgAdvancedPanel::x_SetBioSourceModifier(objects::CBioSource& src, const string& name, const string& value)
{
    if (objects::CSubSource::IsValidSubtypeName(name)) {
        auto subtype = objects::CSubSource::GetSubtypeValue(name);
        CRef<objects::CSubSource> subsrc(new objects::CSubSource(subtype, value));
        src.SetSubtype().push_back(subsrc);
    }
    else if (objects::COrgMod::IsValidSubtypeName(name)) {
        auto subtype = objects::COrgMod::GetSubtypeValue(name);
        CRef<objects::COrgMod> org_mod(new objects::COrgMod(subtype, value));
        src.SetOrg().SetOrgname().SetMod().push_back(org_mod);
    }
}


void COrgAdvancedPanel::x_SetBioSourceModifier(const string& name, const string& value)
{
    if (!m_Source) {
        return;
    }

    x_SetBioSourceModifier(*m_Source, name, value);
}

bool s_DoEraseSubSource(objects::CSubSource::TSubtype subtype)
{
    return !s_DoNotListSubSrc(subtype);
}

bool s_DoEraseOrgMod(objects::COrgMod::TSubtype subtype)
{
    return !s_DoNotListOrgMod(subtype);
}

bool COrgAdvancedPanel::x_ApplyChangesToSource(objects::CBioSource& src)
{
    CRef<objects::CBioSource> original(new objects::CBioSource());
    original->Assign(src);
    // erase existing values
    if (src.IsSetSubtype()) {
        auto it = src.SetSubtype().begin();
        while (it != src.SetSubtype().end()) {
            if ((*it)->IsSetSubtype() && s_DoEraseSubSource((*it)->GetSubtype())) {
                it = src.SetSubtype().erase(it);
            } else {
                ++it;
            }
        }
    }
    if (src.IsSetOrg() && src.GetOrg().IsSetOrgname() && src.GetOrg().GetOrgname().IsSetMod()) {
        auto& mods = src.SetOrg().SetOrgname().SetMod();
        auto it = mods.begin();
        while (it != mods.end()) {
            if ((*it)->IsSetSubtype() && s_DoEraseOrgMod((*it)->GetSubtype())) {
                it = mods.erase(it);
            }
            else {
                ++it;
            }
        }
    }
    for (auto it : m_ScrolledWindow->GetChildren()) {
        COrgAdvancedMod* mod = dynamic_cast<COrgAdvancedMod *>(it);
        if (mod) {
            COrgAdvancedMod::SModData single_modifier_data = mod->GetModifierData();
            const string qual_name = ToStdString(single_modifier_data.name);
            const string qual_value = ToStdString(single_modifier_data.value);
            if (!NStr::IsBlank(qual_name) && !NStr::IsBlank(qual_value)) {
                if (s_IsNonText(qual_name) && NStr::EqualNocase(qual_value, "TRUE")) {
                    x_SetBioSourceModifier(src, qual_name, " ");
                }
                else {
                    x_SetBioSourceModifier(src, qual_name, qual_value);
                }
            }
        }
    }
    if (src.IsSetSubtype() && src.GetSubtype().empty()) {
        src.ResetSubtype();
    }
    if (src.IsSetOrg() && src.GetOrg().IsSetOrgname() &&
        src.GetOrg().GetOrgname().IsSetMod() &&
        src.GetOrg().GetOrgname().GetMod().empty()) {
        src.SetOrg().SetOrgname().ResetMod();
    }
    return !original->Equals(src);
}

bool COrgAdvancedPanel::TransferDataFromWindow()
{
    if (!m_Source) {
        return false;
    }
    x_ApplyChangesToSource(*m_Source);

    return true;
}


void COrgAdvancedPanel::ApplyCommand()
{
    CRef<objects::CSeqdesc> new_desc(new objects::CSeqdesc());
    new_desc->SetSource();
    x_ApplyChangesToSource(new_desc->SetSource());

    if (new_desc->GetSource().Equals(*m_Source))
        return;

    CRef<CCmdComposite> cmd(new CCmdComposite("update source"));
    bool any_changes = false;

    for (objects::CBioseq_CI bi(m_Seh, objects::CSeq_inst::eMol_na); bi; ++bi) {
        objects::CSeqdesc_CI di(*bi, objects::CSeqdesc::e_Source);
        if (di) {
            // edit existing descriptor
            CRef<objects::CSeqdesc> cpy(new objects::CSeqdesc());
            cpy->Assign(*di);
            x_ApplyChangesToSource(cpy->SetSource());
            if (!di->Equals(*cpy)) {
                CRef<CCmdChangeSeqdesc> chg(new CCmdChangeSeqdesc(di.GetSeq_entry_Handle(), *di, *cpy));
                cmd->AddCommand(*chg);
                any_changes = true;
            }
        }
        else {
            // create new source descriptor on this sequence or on the nuc-prot that contains it
            CRef<objects::CSeqdesc> cpy(new objects::CSeqdesc());
            cpy->Assign(*new_desc);
            auto entry = bi->GetParentEntry();
            objects::CBioseq_set_Handle parent = bi->GetParentBioseq_set();
            if (parent && parent.IsSetClass() && parent.GetClass() == objects::CBioseq_set::eClass_nuc_prot) 
                entry = parent.GetParentEntry();
            CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(entry, *cpy));
            cmd->AddCommand(*cmdAddDesc);            
            any_changes = true;
        }
    }

    if (any_changes) {
        m_CmdProcessor->Execute(cmd);
    }
}


END_NCBI_SCOPE
