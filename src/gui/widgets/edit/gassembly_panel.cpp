/*  $Id: gassembly_panel.cpp 44734 2020-03-03 18:47:44Z asztalos $
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
#include <gui/widgets/edit/gassembly_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <objects/general/User_field.hpp>
#include <objtools/edit/struc_comm_field.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_set_handle.hpp>
#include <objects/valid/Comment_rule.hpp>

#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/datectrl.h>
#include <wx/datetime.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE

/*
 * CGAssemblyPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CGAssemblyPanel, wxPanel )


/*
 * CGAssemblyPanel event table definition
 */

BEGIN_EVENT_TABLE( CGAssemblyPanel, wxPanel )

////@begin CGAssemblyPanel event table entries
    EVT_HYPERLINK( ID_GAASSEMBLYHYPERLINK, CGAssemblyPanel::OnAddNewAssemblyMethod )
    EVT_HYPERLINK(ID_GAASSEMBLYDELETE, CGAssemblyPanel::OnDeleteAssembly )
////@end CGAssemblyPanel event table entries

END_EVENT_TABLE()


/*
 * CGAssemblyPanel constructors
 */

CGAssemblyPanel::CGAssemblyPanel()
{
    Init();
}

CGAssemblyPanel::CGAssemblyPanel( wxWindow* parent, ICommandProccessor* cmdproc, objects::CSeq_entry_Handle seh, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
: m_CmdProcessor(cmdproc), m_Seh(seh)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CGAssemblyPanel creator
 */

bool CGAssemblyPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CGAssemblyPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CGAssemblyPanel creation
    return true;
}


/*
 * CGAssemblyPanel destructor
 */

CGAssemblyPanel::~CGAssemblyPanel()
{
////@begin CGAssemblyPanel destruction
////@end CGAssemblyPanel destruction
}


/*
 * Member initialisation
 */

void CGAssemblyPanel::Init()
{
////@begin CGAssemblyPanel member initialisation
    m_AssemblyDate = NULL;
    m_AssemblyName = NULL;
    m_ScrolledWindow = NULL;
    m_Sizer = NULL;
////@end CGAssemblyPanel member initialisation
}


/*
 * Control creation for CGAssemblyPanel
 */

void CGAssemblyPanel::CreateControls()
{    
////@begin CGAssemblyPanel content construction
    CGAssemblyPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL |wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer3->Add(itemFlexGridSizer4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Assembly date"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText6, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("Assembly name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText7, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    CRef<objects::CDate> date(new objects::CDate);
    m_AssemblyDate = new CFlexibleDatePanel(itemPanel1, date);
    //m_AssemblyDate = new wxDatePickerCtrl( itemPanel1, ID_GAASSEMBLYDATE, wxDateTime(), wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
    if (CGAssemblyPanel::ShowToolTips())
        m_AssemblyDate->SetToolTip(_("Year, month or day the assembly was made. Date formats:\nYYYY-MM-DD; YYYY-MM; YYYY"));
    itemFlexGridSizer4->Add(m_AssemblyDate, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_BOTTOM|wxALL, 0);

    m_AssemblyName = new wxTextCtrl( itemPanel1, ID_GAASSEMBLYNAME, wxEmptyString, wxDefaultPosition, wxSize(150, -1), 0 );
    if (CGAssemblyPanel::ShowToolTips())
        m_AssemblyName->SetToolTip(_("This field is usually left blank, especially for prokaryotes, but is available for the name that you are using in your manuscript or abstract to describe this assembly. A good assembly name generally includes:\n - a short 3-4 letter abbreviation for the institution,\n - a 4-6 letter abbreviation of the organism name,\n - and a version.\nFor example, UCLA_Agam_2.1 for version 2.1 of the Anopheles gambiae genome from a lab in UCLA. For the organism name abbreviation, avoid the 3+3 \"genus + species\" format (e.g. \"PanTro\" for Pan troglodytes) since that could cause conflicts with the UCSC database name; we recommend formats such as 1+3 or 2+4 for the \"genus + species\" instead (e.g. PTro or PaTrog). Do not use the full scientific name, in case that name changes in the future."));
    itemFlexGridSizer4->Add(m_AssemblyName, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_BOTTOM|wxALL, 5);

    itemFlexGridSizer3->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer3->Add(itemBoxSizer10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText11 = new wxStaticText( itemPanel1, wxID_STATIC, _("Assembly method"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemStaticText11, 0, wxALIGN_LEFT |wxALL, 5);

#ifdef __WXMSW__
    itemBoxSizer10->Add(32, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
#elif __WXOSX_COCOA__
    itemBoxSizer10->Add(55, 5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
#else
    itemBoxSizer10->Add(65, 5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
#endif
    wxStaticText* itemStaticText13 = new wxStaticText( itemPanel1, wxID_STATIC, _("Version or Date program was run"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemStaticText13, 0, wxALIGN_LEFT |wxALL, 5);

    m_ScrolledWindow = new wxScrolledWindow( itemPanel1, ID_GASCROLLEDWND, wxDefaultPosition, wxSize(-1, 92), wxTAB_TRAVERSAL);
    itemFlexGridSizer3->Add(m_ScrolledWindow, 1, wxGROW|wxALL, 5);

    m_ScrolledWindow->SetScrollbars(1, 1, 0, 0);
    m_Sizer = new wxFlexGridSizer(0, 2, 0, 0);
    m_ScrolledWindow->SetSizer(m_Sizer);

    for (size_t index = 0; index < m_Rows; ++index) {
        x_AddEmptyRow();
    }

    wxHyperlinkCtrl* itemHyperlinkCtrl16 = new wxHyperlinkCtrl( itemPanel1, ID_GAASSEMBLYHYPERLINK, _("Add another assembly method"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemFlexGridSizer3->Add(itemHyperlinkCtrl16, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CGAssemblyPanel content construction
}

void CGAssemblyPanel::x_AddEmptyRow()
{
    CSingleAssemblyMethod* row = new CSingleAssemblyMethod(m_ScrolledWindow);
    if (m_RowHeight == 0) {
        int row_width;
        row->GetClientSize(&row_width, &m_RowHeight);
    }
    x_AddRowToWindow(row);
}

void CGAssemblyPanel::x_AddRowToWindow(wxWindow* row)
{
    m_Sizer->Add(row, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 0);
    wxHyperlinkCtrl* itemHyperLink = new CNoTabHyperlinkCtrl(m_ScrolledWindow, ID_GAASSEMBLYDELETE, wxT("Delete"), wxT("delete"));
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
    m_ScrolledWindow->SetMinSize(wxSize(m_TotalWidth + 10, m_MaxRows * m_ScrollRate));
    m_ScrolledWindow->Refresh();
#endif
}


void CGAssemblyPanel::OnDeleteAssembly(wxHyperlinkEvent& event)
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

/*
 * Should we show tooltips?
 */

bool CGAssemblyPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CGAssemblyPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CGAssemblyPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CGAssemblyPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CGAssemblyPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CGAssemblyPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CGAssemblyPanel icon retrieval
}


/*
 * wxEVT_COMMAND_HYPERLINK event handler for ID_GAASSEMBLYHYPERLINK
 */

void CGAssemblyPanel::OnAddNewAssemblyMethod( wxHyperlinkEvent& event )
{
    x_AddEmptyRow();
    int x, y;
    m_ScrolledWindow->GetVirtualSize(&x, &y);

    int xUnit, yUnit;
    m_ScrolledWindow->GetScrollPixelsPerUnit(&xUnit, &yUnit);
    m_ScrolledWindow->Scroll(0, y / yUnit);
}


void CGAssemblyPanel::ApplyUser(objects::CUser_object& user)
{
    m_User.Reset(&user);
    TransferDataToWindow();
}


void CGAssemblyPanel::x_Reset()
{
    m_AssemblyName->SetValue(wxT(""));
    m_AssemblyDate->Clear();
    for (auto it : m_ScrolledWindow->GetChildren()) {
        CSingleAssemblyMethod* method = dynamic_cast<CSingleAssemblyMethod *>(it);
        if (method) {
            method->SetValue("");
        }
    }
}

const string kAssemblyName = "Assembly Name";
const string kAssemblyMethod = "Assembly Method";
const string kAssemblyDate = "Assembly Date";

bool CGAssemblyPanel::TransferDataToWindow()
{
    x_Reset();
    if (m_User && m_User->IsSetData()) {
        CConstRef<objects::CUser_field> aname = m_User->GetFieldRef(kAssemblyName);
        if (aname && aname->IsSetData() && aname->GetData().IsStr()) {
            m_AssemblyName->SetValue(ToWxString(aname->GetData().GetStr()));
        }
        CConstRef<objects::CUser_field> adate = m_User->GetFieldRef(kAssemblyDate);
        if (adate && adate->IsSetData() && adate->GetData().IsStr() && m_AssemblyDate) {
            m_AssemblyDate->SetDate(adate->GetData().GetStr());
        }

        CConstRef<objects::CUser_field> field = m_User->GetFieldRef(kAssemblyMethod);
        if (field && field->IsSetData() && field->GetData().IsStr()) {
            string vallist = field->GetData().GetStr();
            vector<string> vals;
            NStr::Split(vallist, ";", vals);
            size_t num_rows = 0;
            for (auto rit : m_ScrolledWindow->GetChildren()) {
                CSingleAssemblyMethod* row = dynamic_cast<CSingleAssemblyMethod *>(rit);
                if (row) {
                    num_rows++;
                }
            }
            while (vals.size() > num_rows) {
                x_AddEmptyRow();
                num_rows++;
            }
            auto s = vals.begin();
            for (auto rit : m_ScrolledWindow->GetChildren()) {
                CSingleAssemblyMethod* row = dynamic_cast<CSingleAssemblyMethod *>(rit);
                if (row) {
                    row->SetValue(*s);
                    s++;
                    if (s == vals.end()) {
                        break;
                    }
                }
            }

        }
    }
    return true;
}


bool CGAssemblyPanel::TransferDataFromWindow()
{
    if (!m_User)
        return true;
    string date;
    vector<string> dmy(3);
    dmy[0] = m_AssemblyDate->GetDay();
    string month = m_AssemblyDate->GetMonth();
    dmy[1] = NStr::ToUpper(month);
    dmy[2] = m_AssemblyDate->GetYear();
    if (!dmy[0].empty() && !dmy[1].empty() && !dmy[2].empty())
    {
        // nothing to do
    }
    else if (!dmy[1].empty() && !dmy[2].empty())
    {
        dmy.erase(dmy.begin());
    }
    else if (!dmy[2].empty())
    {
        dmy.erase(dmy.begin());
        dmy.erase(dmy.begin());
    }
    else
    {
        dmy.clear();
    }
    date = NStr::Join(dmy, "-");
    // set assembly name
    if (date.empty()) {
        m_User->RemoveNamedField(kAssemblyDate);
    } else {
        objects::CUser_field& adate = m_User->SetField(kAssemblyDate);
        adate.SetData().SetStr(date);
    }

    // set assembly name
    if (m_AssemblyName->IsEmpty()) {
        m_User->RemoveNamedField(kAssemblyName);
    } else {
        objects::CUser_field& aname = m_User->SetField(kAssemblyName);
        aname.SetData().SetStr(ToStdString(m_AssemblyName->GetValue()));
    }

    // set assembly method
    string val = kEmptyStr;
    for (auto it : m_ScrolledWindow->GetChildren()) {
        CSingleAssemblyMethod* row = dynamic_cast<CSingleAssemblyMethod *>(it);
        if (row) {
            string new_val = row->GetValue();
            if (!new_val.empty()) {
                val += row->GetValue();
                val += ";";
            }
        }
    }
    if (!val.empty()) {
        // remove trailing semicolon
        val = val.substr(0, val.length() - 1);
    }
    if (val.empty()) {
        m_User->RemoveNamedField(kAssemblyMethod);
    } else {
        objects::CUser_field& method = m_User->SetField(kAssemblyMethod);
        method.SetData().SetStr(val);
    }

    // sort fields
    CConstRef<objects::CComment_set> rules = objects::CComment_set::GetCommentRules();
    if (rules) {
        try {
            const objects::CComment_rule& rule = rules->FindCommentRule("Genome-Assembly-Data");
            rule.ReorderFields(*m_User);
        }
        catch (CException& ex) {
        }
    }

    return true;
}

void CGAssemblyPanel::ApplyCommand()
{
    if (!m_User)
        return;

    TransferDataFromWindow();

    CRef<objects::CUser_object> empty = objects::edit::CStructuredCommentField::MakeUserObject("Genome-Assembly-Data");
    if (m_User->Equals(*empty))
        return;

    CRef<CCmdComposite> cmd(new CCmdComposite("update genome assembly structured comment"));
    bool any_changes = false;

    for (objects::CBioseq_CI bi(m_Seh, objects::CSeq_inst::eMol_na); bi; ++bi) 
    {
        bool found = false;
        for (objects::CSeqdesc_CI di(*bi, objects::CSeqdesc::e_User); di; ++di)
        {
            if (di && di->IsUser() && di->GetUser().GetObjectType() == objects::CUser_object::eObjectType_StructuredComment &&
                NStr::Equal(objects::CComment_rule::GetStructuredCommentPrefix(di->GetUser()), "Genome-Assembly-Data"))
            {
            
                // edit existing descriptor
                CRef<objects::CSeqdesc> cpy(new objects::CSeqdesc());
                cpy->SetUser(*m_User);
                if (!di->Equals(*cpy)) 
                {
                    CRef<CCmdChangeSeqdesc> chg(new CCmdChangeSeqdesc(di.GetSeq_entry_Handle(), *di, *cpy));
                    cmd->AddCommand(*chg);
                    any_changes = true;
                }
                found = true;
            } 
        }
        
        if (!found) 
        {
            // create new source descriptor on this sequence or on the nuc-prot that contains it
            CRef<objects::CSeqdesc> new_desc(new objects::CSeqdesc());

            new_desc->SetUser(*m_User);
            objects::CBioseq_set_Handle parent = bi->GetParentBioseq_set();
            auto entry = bi->GetParentEntry();
            if (parent && parent.IsSetClass() && parent.GetClass() == objects::CBioseq_set::eClass_nuc_prot) 
                entry = parent.GetParentEntry();
            CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(entry, *new_desc));
            cmd->AddCommand(*cmdAddDesc);
            any_changes = true;
        }
    }

    if (any_changes) 
    {
        m_CmdProcessor->Execute(cmd);
    }
}

END_NCBI_SCOPE
