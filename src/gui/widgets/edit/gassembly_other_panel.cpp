/*  $Id: gassembly_other_panel.cpp 44733 2020-03-03 17:13:24Z asztalos $
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
#include <gui/widgets/edit/gassembly_other_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>
#include <objtools/edit/struc_comm_field.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_set_handle.hpp>
#include <objects/valid/Comment_rule.hpp>
#include <objects/valid/Comment_set.hpp>

#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/combobox.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE

/*
 * CGAssemblyOtherPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CGAssemblyOtherPanel, wxPanel )


/*
 * CGAssemblyOtherPanel event table definition
 */

BEGIN_EVENT_TABLE( CGAssemblyOtherPanel, wxPanel )

////@begin CGAssemblyOtherPanel event table entries
    EVT_HYPERLINK( ID_SEQTECHHYPERLINK, CGAssemblyOtherPanel::OnAddNewSeqtechnology )
    EVT_HYPERLINK(ID_DELETESEQTECH, CGAssemblyOtherPanel::OnDeleteSeqtechnology)
////@end CGAssemblyOtherPanel event table entries

END_EVENT_TABLE()


/*
 * CGAssemblyOtherPanel constructors
 */

CGAssemblyOtherPanel::CGAssemblyOtherPanel()
{
    Init();
}

CGAssemblyOtherPanel::CGAssemblyOtherPanel( wxWindow* parent, ICommandProccessor* cmdproc, objects::CSeq_entry_Handle seh, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
: m_CmdProcessor(cmdproc), m_Seh(seh)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CGAssemblyOtherPanel creator
 */

bool CGAssemblyOtherPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CGAssemblyOtherPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CGAssemblyOtherPanel creation
    return true;
}


/*
 * CGAssemblyOtherPanel destructor
 */

CGAssemblyOtherPanel::~CGAssemblyOtherPanel()
{
////@begin CGAssemblyOtherPanel destruction
////@end CGAssemblyOtherPanel destruction
}


/*
 * Member initialisation
 */

void CGAssemblyOtherPanel::Init()
{
////@begin CGAssemblyOtherPanel member initialisation
    m_GenomeCoverage = NULL;
    m_ScrolledWindow = NULL;
    m_Sizer = NULL;
////@end CGAssemblyOtherPanel member initialisation
}


/*
 * Control creation for CGAssemblyOtherPanel
 */

void CGAssemblyOtherPanel::CreateControls()
{    
////@begin CGAssemblyOtherPanel content construction
    CGAssemblyOtherPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer2, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Genome coverage"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(itemStaticText6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GenomeCoverage = new wxTextCtrl( itemPanel1, ID_GENOMECOVERAGE, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    if (CGAssemblyOtherPanel::ShowToolTips())
        m_GenomeCoverage->SetToolTip(_("The estimated base coverage across the genome, eg 12x. This can be calculated by dividing the number of bases sequenced by the expected genome size and multiplying that by the percentage of bases that were placed in the final assembly. More simply it is the number of bases sequenced divided by the expected genome size."));
    itemFlexGridSizer2->Add(m_GenomeCoverage, 0, wxALIGN_LEFT |wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText1 = new wxStaticText( itemPanel1, wxID_STATIC, _("Sequencing technology"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(itemStaticText1, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ScrolledWindow = new wxScrolledWindow( itemPanel1, ID_SEQTECHSCROLLEDWND, wxDefaultPosition, wxSize(256, 92), wxTAB_TRAVERSAL );
    itemFlexGridSizer2->Add(m_ScrolledWindow, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_ScrolledWindow->SetScrollbars(1, 1, 0, 0);
    m_Sizer = new wxFlexGridSizer(0, 2, 0, 0);
    m_ScrolledWindow->SetSizer(m_Sizer);

    for (size_t index = 0; index < m_Rows; ++index) {
        x_AddEmptyRow();
    }
    wxHyperlinkCtrl* itemHyperlinkCtrl2 = new wxHyperlinkCtrl( itemPanel1, ID_SEQTECHHYPERLINK, _("Add another sequencing technology"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemFlexGridSizer2->Add(itemHyperlinkCtrl2, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CGAssemblyOtherPanel content construction
}

void CGAssemblyOtherPanel::x_AddEmptyRow()
{
    wxArrayString m_SeqTechnologyStrings;
    m_SeqTechnologyStrings.Add(_("ABI3730"));
    m_SeqTechnologyStrings.Add(_("Sanger"));
    m_SeqTechnologyStrings.Add(_("454"));
    m_SeqTechnologyStrings.Add(_("Illumina"));
    m_SeqTechnologyStrings.Add(_("Illumina GAII"));
    m_SeqTechnologyStrings.Add(_("Illumina GAIIx"));
    m_SeqTechnologyStrings.Add(_("Illumina HiSeq"));
    m_SeqTechnologyStrings.Add(_("Illumina iSeq"));
    m_SeqTechnologyStrings.Add(_("Illumina MiniSeq"));
    m_SeqTechnologyStrings.Add(_("Illumina MiSeq"));
    m_SeqTechnologyStrings.Add(_("Illumina NextSeq"));
    m_SeqTechnologyStrings.Add(_("Illumina NovaSeq"));
    m_SeqTechnologyStrings.Add(_("IonTorrent"));
    m_SeqTechnologyStrings.Add(_("Oxford Nanopore"));
    m_SeqTechnologyStrings.Add(_("Oxford Nanopore GridION"));
    m_SeqTechnologyStrings.Add(_("Oxford Nanopore MiniION"));
    m_SeqTechnologyStrings.Add(_("Oxford Nanopore PromethION"));
    m_SeqTechnologyStrings.Add(_("PacBio"));
    m_SeqTechnologyStrings.Add(_("PacBio RS"));
    m_SeqTechnologyStrings.Add(_("PacBio RSII"));
    m_SeqTechnologyStrings.Add(_("PacBio Sequel"));
    m_SeqTechnologyStrings.Add(_("Helicos"));
    m_SeqTechnologyStrings.Add(_("SOLiD"));
    m_SeqTechnologyStrings.Add(_("Complete Genomics"));
    wxComboBox* row = new wxComboBox(m_ScrolledWindow, ID_SEQTECHCOMBOBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_SeqTechnologyStrings, wxCB_DROPDOWN);
#ifdef __WXOSX_COCOA__
    row->SetSelection(-1);
#endif
    if (CGAssemblyOtherPanel::ShowToolTips())
        row->SetToolTip(_("The sequencing machine(s) used to generate the data, eg Illumina MiSeq or 454. Select the sequencing technology from the pulldown list, or type the name of the technology directly in the box."));

    if (m_RowHeight == 0) {
        int row_width;
        row->GetClientSize(&row_width, &m_RowHeight);
    }
    x_AddRowToWindow(row);
}

void CGAssemblyOtherPanel::x_AddRowToWindow(wxWindow* row)
{
    m_Sizer->Add(row, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 0);
    wxHyperlinkCtrl* itemHyperLink = new CNoTabHyperlinkCtrl(m_ScrolledWindow, ID_DELETESEQTECH, wxT("Delete"), wxT("delete"));
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

bool CGAssemblyOtherPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CGAssemblyOtherPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CGAssemblyOtherPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CGAssemblyOtherPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CGAssemblyOtherPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CGAssemblyOtherPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CGAssemblyOtherPanel icon retrieval
}

/*
 * wxEVT_COMMAND_HYPERLINK event handler for ID_SEQTECHHYPERLINK
 */

void CGAssemblyOtherPanel::OnAddNewSeqtechnology( wxHyperlinkEvent& event )
{
    x_AddEmptyRow();
    int x, y;
    m_ScrolledWindow->GetVirtualSize(&x, &y);

    int xUnit, yUnit;
    m_ScrolledWindow->GetScrollPixelsPerUnit(&xUnit, &yUnit);
    m_ScrolledWindow->Scroll(0, y / yUnit);
}


void CGAssemblyOtherPanel::OnDeleteSeqtechnology(wxHyperlinkEvent& event)
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


void CGAssemblyOtherPanel::ApplyUser(objects::CUser_object& user)
{
    m_User.Reset(&user);
    TransferDataToWindow();
}


void CGAssemblyOtherPanel::x_Reset()
{
    m_GenomeCoverage->SetValue(wxT(""));
    for (auto it : m_ScrolledWindow->GetChildren()) {
        wxComboBox* row = dynamic_cast<wxComboBox *>(it);
        if (row) {
            row->SetValue(wxT(""));
        }

    }
}

const string kGenomeCoverage = "Genome Coverage";
const string kSequencingTechnology = "Sequencing Technology";
bool CGAssemblyOtherPanel::TransferDataToWindow()
{
    x_Reset();
    if (m_User && m_User->IsSetData()) {
        for (auto it : m_User->GetData()) {
            if (it->IsSetLabel() && it->GetLabel().IsStr() &&
                it->IsSetData() && it->GetData().IsStr()) {
                const string& label = it->GetLabel().GetStr();
                if (NStr::Equal(label, kGenomeCoverage)) {
                    m_GenomeCoverage->SetValue(ToWxString(it->GetString()));
                } else if (NStr::Equal(label, kSequencingTechnology)) {
                    // Set value for sequencing technology
                    vector<string> techs;
                    NStr::Split(it->GetString(), ";", techs);
                    size_t num_rows = 0;
                    for (auto rit : m_ScrolledWindow->GetChildren()) {
                        wxComboBox* row = dynamic_cast<wxComboBox *>(rit);
                        if (row) {
                            num_rows++;
                        }
                    }
                    while (techs.size() > num_rows) {
                        x_AddEmptyRow();
                        num_rows++;
                    }
                    auto s = techs.begin();
                    for (auto rit : m_ScrolledWindow->GetChildren()) {
                        wxComboBox* row = dynamic_cast<wxComboBox *>(rit);
                        if (row) {
                            row->SetValue(*s);
                            s++;
                            if (s == techs.end()) {
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    return true;
}


bool CGAssemblyOtherPanel::TransferDataFromWindow()
{
    if (m_User) {
        if (m_GenomeCoverage->IsEmpty()) {
            m_User->RemoveNamedField(kGenomeCoverage);
        } else {
            objects::CUser_field& coverage = m_User->SetField(kGenomeCoverage);
            coverage.SetData().SetStr(ToStdString(m_GenomeCoverage->GetValue()));
        }

        //Get value for Sequencing Technology
        string tech = kEmptyStr;
        for (auto it : m_ScrolledWindow->GetChildren()) {
            wxComboBox* row = dynamic_cast<wxComboBox *>(it);
            if (row) {
                string new_val = row->GetValue().ToStdString();
                if (!new_val.empty()) {
                    tech += ToStdString(new_val);
                    tech += ";";
                }
            }
        }
        if (!tech.empty()) {
            // remove trailing semicolon
            tech = tech.substr(0, tech.length() - 1);
        }
        if (tech.empty()) {
            m_User->RemoveNamedField(kSequencingTechnology);
        } else {
            objects::CUser_field& seqtech = m_User->SetField(kSequencingTechnology);
            seqtech.SetData().SetStr(tech);
        }
        // sort fields
        CConstRef<objects::CComment_set> rules = objects::CComment_set::GetCommentRules();
        if (rules) {
            try {
                const objects::CComment_rule& rule = rules->FindCommentRule("Genome-Assembly-Data");
                rule.ReorderFields(*m_User);
            } catch (CException& ex) {
            }
        }
    }
    return true;
}

void CGAssemblyOtherPanel::ApplyCommand()
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
