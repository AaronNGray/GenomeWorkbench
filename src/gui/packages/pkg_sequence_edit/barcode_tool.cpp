/*  $Id: barcode_tool.cpp 44314 2019-11-27 14:31:38Z filippov $
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
 * Authors:  Igor Filippov
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

#include <wx/filename.h>
#include <wx/hyperlink.h>
#include <wx/graphics.h>
#include <wx/renderer.h>

#include <objects/misc/sequence_macros.hpp>
#include <objects/seq/MolInfo.hpp>

#include <objmgr/seq_entry_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <objects/seqblock/GB_block.hpp>
#include <objects/seqblock/seqblock_macros.hpp>

#include <gui/core/selection_service_impl.hpp>
#include <gui/core/project_service.hpp>
#include <gui/widgets/seq/flat_file_ctrl.hpp>

#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>

#include <gui/widgets/data/report_dialog.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp> 
#include <gui/packages/pkg_sequence_edit/barcode_tool.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

IMPLEMENT_DYNAMIC_CLASS( CBarcodeTool, wxFrame )

BEGIN_EVENT_TABLE( CBarcodeTool, wxFrame )
    EVT_BUTTON( ID_BARCODE_TOOL_SELECT, CBarcodeTool::OnSelect)
    EVT_BUTTON( ID_BARCODE_TOOL_SELECT_ALL, CBarcodeTool::OnSelectAll)
    EVT_BUTTON( ID_BARCODE_TOOL_UNSELECT_ALL, CBarcodeTool::OnUnselectAll)
    EVT_BUTTON( ID_BARCODE_TOOL_DISMISS, CBarcodeTool::OnDismiss)
    EVT_BUTTON( ID_BARCODE_TOOL_ADD_KEYWORD, CBarcodeTool::OnAddKeyword)
    EVT_BUTTON( ID_BARCODE_TOOL_REMOVE_KEYWORD, CBarcodeTool::OnRemoveKeyword)
    EVT_BUTTON( ID_BARCODE_TOOL_APPLY_DBXREFS, CBarcodeTool::OnApplyDbxrefs)
END_EVENT_TABLE()

CBarcodeTool::CBarcodeTool( wxWindow* parent, objects::CSeq_entry_Handle seh, ICommandProccessor* cmdProcessor, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
: m_CmdProcessor(cmdProcessor), m_TopSeqEntry(seh), m_Workbench(wb)
{ 
    GetValues();
    Init(); 
    Create(parent, id, caption, pos, size, style); 
    NEditingStats::ReportUsage(caption);
}

void CBarcodeTool::GetValues()
{
    wxBusyCursor wait;
    m_barcode.clear();
    m_selected.clear();
    if (m_TopSeqEntry)
    {
        objects::CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_na);                
        for ( ; b_iter ; ++b_iter ) 
        {
            FindBarcodeFeats(*b_iter);
        }
    }
}

void CBarcodeTool::CreateControls() 
{
    wxPanel* itemPanel1 = new wxPanel(this, wxID_ANY);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxGridSizer *header = new wxFlexGridSizer(0, 14, 0, 0);
    itemBoxSizer2->Add(header, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticHeader0 = new wxStaticText( itemPanel1, wxID_STATIC,  _(" "), wxDefaultPosition, wxSize(30,-1), 0 );
    header->Add(itemStaticHeader0, 0, wxALIGN_CENTER|wxALL, 5);
    wxStaticText* itemStaticHeader1 = new wxStaticText( itemPanel1, wxID_STATIC,  _("Barcode ID"), wxDefaultPosition, wxSize(150,-1), 0 );
    header->Add(itemStaticHeader1, 0, wxALIGN_CENTER|wxALL, 5);
    wxStaticText* itemStaticHeader2 = new wxStaticText( itemPanel1, wxID_STATIC,  _("Genbank Accession"), wxDefaultPosition, wxSize(100,-1), 0 );
    itemStaticHeader2->Wrap(100);
    header->Add(itemStaticHeader2, 0, wxALIGN_CENTER|wxALL, 5);
    wxStaticText* itemStaticHeader3 = new wxStaticText( itemPanel1, wxID_STATIC,  _("Length"), wxDefaultPosition, wxSize(80,-1), 0 );
    header->Add(itemStaticHeader3, 0, wxALIGN_CENTER|wxALL, 5);
    wxStaticText* itemStaticHeader4 = new wxStaticText( itemPanel1, wxID_STATIC,  _("Primers"), wxDefaultPosition, wxSize(80,-1), 0 );
    header->Add(itemStaticHeader4, 0, wxALIGN_CENTER|wxALL, 5);
    wxStaticText* itemStaticHeader5 = new wxStaticText( itemPanel1, wxID_STATIC,  _("Country"), wxDefaultPosition, wxSize(80,-1), 0 );
    header->Add(itemStaticHeader5, 0, wxALIGN_CENTER|wxALL, 5);
    wxStaticText* itemStaticHeader6 = new wxStaticText( itemPanel1, wxID_STATIC,  _("Voucher"), wxDefaultPosition, wxSize(80,-1), 0 );
    header->Add(itemStaticHeader6, 0, wxALIGN_CENTER|wxALL, 5);
    wxStaticText* itemStaticHeader7 = new wxStaticText( itemPanel1, wxID_STATIC,  _("Structured Voucher"), wxDefaultPosition, wxSize(80,-1), 0 );
    itemStaticHeader7->Wrap(80);
    header->Add(itemStaticHeader7, 0, wxALIGN_CENTER|wxALL, 5);
    wxStaticText* itemStaticHeader8 = new wxStaticText( itemPanel1, wxID_STATIC,  _("Percent Ns"), wxDefaultPosition, wxSize(80,-1), 0 );
    itemStaticHeader8->Wrap(80);
    header->Add(itemStaticHeader8, 0, wxALIGN_CENTER|wxALL, 5);
    wxStaticText* itemStaticHeader9 = new wxStaticText( itemPanel1, wxID_STATIC,  _("Collection Date"), wxDefaultPosition, wxSize(80,-1), 0 );
    itemStaticHeader9->Wrap(80);
    header->Add(itemStaticHeader9, 0, wxALIGN_CENTER|wxALL, 5);
    wxStaticText* itemStaticHeader10 = new wxStaticText( itemPanel1, wxID_STATIC,  _("Order Assignment"), wxDefaultPosition, wxSize(80,-1), 0 );
    itemStaticHeader10->Wrap(80);
    header->Add(itemStaticHeader10, 0, wxALIGN_CENTER|wxALL, 5);
    wxStaticText* itemStaticHeader11 = new wxStaticText( itemPanel1, wxID_STATIC,  _("Low Trace"), wxDefaultPosition, wxSize(80,-1), 0 );
    itemStaticHeader11->Wrap(80);
    header->Add(itemStaticHeader11, 0, wxALIGN_CENTER|wxALL, 5);
    wxStaticText* itemStaticHeader12 = new wxStaticText( itemPanel1, wxID_STATIC,  _("Frame Shift"), wxDefaultPosition, wxSize(80,-1), 0 );
    itemStaticHeader12->Wrap(80);
    header->Add(itemStaticHeader12, 0, wxALIGN_CENTER|wxALL, 5);
    wxStaticText* itemStaticHeader13 = new wxStaticText( itemPanel1, wxID_STATIC,  _("Has BARCODE Keyword"), wxDefaultPosition, wxSize(80,-1), 0 );
    itemStaticHeader13->Wrap(80);
    header->Add(itemStaticHeader13, 0, wxALIGN_CENTER|wxALL, 5);


    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 5);

    m_scrolled = new CBarcodePanel(itemPanel1, &m_barcode, &m_selected, wxID_ANY, wxDefaultPosition, wxSize(-1, 170));
    itemBoxSizer3->Add(m_scrolled, 1, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer17, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);


    wxArrayString radioboxStrings;
    radioboxStrings.Add(_("Check features where"));
    radioboxStrings.Add(_("Uncheck features where"));
    m_Radiobox = new wxRadioBox( itemPanel1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, radioboxStrings, 0, wxRA_SPECIFY_ROWS );
    m_Radiobox->SetSelection(0);
    itemBoxSizer17->Add(m_Radiobox, 0, wxALIGN_TOP|wxRIGHT|wxLEFT, 5);

    wxArrayString choice_strings;
    choice_strings.Add(_("Barcode ID"));
    choice_strings.Add(_("Genbank Accession"));
    choice_strings.Add(_("Percent Ns"));
    m_Choice = new wxChoice( itemPanel1, wxID_ANY, wxDefaultPosition, wxDefaultSize, choice_strings, 0 );
    m_Choice->SetSelection(0);
    itemBoxSizer17->Add(m_Choice, 0, wxALIGN_TOP|wxALL, 7);

    m_StringConstraintPanel = new CStringConstraintPanel( itemPanel1, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer17->Add(m_StringConstraintPanel, 0, wxALIGN_TOP|wxALL|wxFIXED_MINSIZE, 0);
    m_StringConstraintPanel->SetStringSelection(_("Contains"));

   
    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    
    wxButton* itemButton13 = new wxButton(itemPanel1, ID_BARCODE_TOOL_SELECT, _("Select"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer12->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    
    wxButton* itemButton14 = new wxButton(itemPanel1, ID_BARCODE_TOOL_SELECT_ALL, _("Select All"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    
    wxButton* itemButton15 = new wxButton(itemPanel1, ID_BARCODE_TOOL_UNSELECT_ALL, _("Unselect All"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer12->Add(itemButton15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    
    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer16, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton16 = new wxButton( itemPanel1, ID_BARCODE_TOOL_APPLY_DBXREFS, _("Apply Dbxrefs"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(itemButton16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton17 = new wxButton( itemPanel1, ID_BARCODE_TOOL_REMOVE_KEYWORD, _("Remove BARCODE Keyword from Selected"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(itemButton17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    
    wxButton* itemButton18 = new wxButton( itemPanel1, ID_BARCODE_TOOL_ADD_KEYWORD, _("Add BARCODE Keyword to BARCODE Tech"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(itemButton18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton19 = new wxButton( itemPanel1, ID_BARCODE_TOOL_DISMISS, _("Dismiss"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(itemButton19, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);


}



bool CBarcodeTool::GetHasKeyword(CBioseq_Handle bsh)
{
    bool has_keyword(false);
    for (CSeqdesc_CI desc_ci(bsh, CSeqdesc::e_Genbank); desc_ci; ++desc_ci)
    {
        FOR_EACH_KEYWORD_ON_GENBANKBLOCK(qual_it, desc_ci->GetGenbank())
        {
            const string &keyword = *qual_it;
            if (NStr::EqualNocase(keyword, "BARCODE"))
                has_keyword = true;
        }
    }
    return has_keyword;
}


void CBarcodeTool::FindBarcodeFeats(CBioseq_Handle bsh)
{
    if (!validator::IsTechBarcode(bsh))
        return;

    validator::SBarcode b;

    // After propagation, call this:
    // remember to remove all the copy-pasted code as well
    validator::BarcodeTestBioseq(bsh, b);
    if (validator::BarcodeTestFails(b)) {
        m_barcode.push_back(b);
    }
}


void CBarcodeTool::OnSelectAll( wxCommandEvent& event )
{
    m_selected.clear();
    std::set<int>::iterator it = m_selected.end();
    for (size_t i = 0; i < m_barcode.size(); i++)
        it = m_selected.insert(it, i);
    m_scrolled->Refresh();
}

void CBarcodeTool::OnUnselectAll( wxCommandEvent& event )
{
    m_selected.clear();
    m_scrolled->Refresh();
}

void CBarcodeTool::OnSelect( wxCommandEvent& event )
{
    bool select = m_Radiobox->GetSelection() == 0;
    string choice = m_Choice->GetStringSelection().ToStdString();
    CRef<edit::CStringConstraint> constraint(m_StringConstraintPanel->GetStringConstraint());
    if (!constraint)
        return;
    for (size_t i = 0; i < m_barcode.size(); i++)
    {
        string value;
        if (choice =="Barcode ID")
        {
            value = m_barcode[i].barcode;
        }
        else if (choice == "Genbank Accession")
        {
            value = m_barcode[i].genbank;
        }
        else if (choice == "Percent Ns")
        {
            value = m_barcode[i].percent_n;
        }
        if (value.empty())
            continue;
        if (constraint->DoesTextMatch(value))
        {
            if (select)
            {
                m_selected.insert(i);
            }
            else
            {
                m_selected.erase(i);
            }
        }
    }
    m_scrolled->Refresh();
}

void CBarcodeTool::CheckRow(int i) 
{
    if (m_selected.find(i) == m_selected.end())
        m_selected.insert(i);
    else
        m_selected.erase(i);
    m_scrolled->Refresh();
}

void CBarcodeTool::Init() 
{
}


bool CBarcodeTool::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
    wxFrame::Create( parent, id, caption, pos, size, style );
    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    return true;
}

void CBarcodeTool::OnDismiss( wxCommandEvent& event )
{
    Close();
}

void CBarcodeTool::OnAddKeyword( wxCommandEvent& event ) // Add BARCODE keyword should act on all bioseqs (not only those in the window) which have tech = barcode
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Add BARCODE Keyword"));
    objects::CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_na);                
    for ( ; b_iter ; ++b_iter ) 
    {
        CBioseq_Handle bsh = *b_iter;

        if (!validator::IsTechBarcode(bsh))
            continue;
        if (GetHasKeyword(bsh))
            continue;

        CSeqdesc_CI desc_ci(bsh, CSeqdesc::e_Genbank); 
        if (desc_ci)
        {
            CRef<CSeqdesc> new_desc(new CSeqdesc);
            new_desc->Assign(*desc_ci);
            new_desc->SetGenbank().SetKeywords().push_back("BARCODE");
            CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(desc_ci.GetSeq_entry_Handle(), *desc_ci, *new_desc));
            cmd->AddCommand(*ecmd);
        }
        else
        {
            CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
            CRef<CSeqdesc> new_desc(new CSeqdesc);
            new_desc->SetGenbank().SetKeywords().push_back("BARCODE");
            cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(seh, *new_desc)) );
        }
    }
    m_CmdProcessor->Execute(cmd);
    GetValues();
    m_scrolled->Refresh();
}

void CBarcodeTool::OnRemoveKeyword( wxCommandEvent& event ) // remove BARCODE keyword should act on selected bioseqs. If nothing is selected it should ask if all should be selected. Acts only on bioseqs in the window.
{
    bool selected_all(false);
    if (m_selected.empty())
    {
        int answer = wxMessageBox(_("You have not selected any Bioseqs - remove BARCODE keyword from all?"), wxT("Remove from all"), wxYES_NO | wxICON_QUESTION, this);
        if (answer == wxYES)
        {
            m_selected.clear();
            std::set<int>::iterator it = m_selected.end();
            for (size_t i = 0; i < m_barcode.size(); i++)
                it = m_selected.insert(it, i);
            selected_all = true;
        }
        else
        {
            return;
        }
    }

    CRef<CCmdComposite> cmd(new CCmdComposite("Remove BARCODE Keyword"));
    for (set<int>::iterator j = m_selected.begin(); j != m_selected.end(); ++j)
    {
        int i = *j;
        if (!m_barcode[i].has_keyword)
            continue;
        CBioseq_Handle bsh = m_barcode[i].bsh;
        CSeqdesc_CI desc_ci(bsh, CSeqdesc::e_Genbank); 
        if (desc_ci)
        {
            CRef<CSeqdesc> new_desc(new CSeqdesc);
            new_desc->Assign(*desc_ci);
            EDIT_EACH_KEYWORD_ON_GENBANKBLOCK(qual_it, new_desc->SetGenbank())
            {
                const string &keyword = *qual_it;
                if (NStr::EqualNocase(keyword, "BARCODE"))
                    ERASE_KEYWORD_ON_GENBANKBLOCK(qual_it, new_desc->SetGenbank());
            }
            CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(desc_ci.GetSeq_entry_Handle(), *desc_ci, *new_desc));
            cmd->AddCommand (*ecmd);
        }
    }   
    m_CmdProcessor->Execute(cmd);
// When using remove BARCODE button a report should be presented
    ShowReport();
    if (selected_all)
    {
        m_selected.clear();
    }
    GetValues();
    m_scrolled->Refresh();
}

void CBarcodeTool::ShowReport()
{
    string msg;
    for (set<int>::iterator j = m_selected.begin(); j != m_selected.end(); ++j)
    {
        int i = *j;

        const validator::SBarcode &b = m_barcode[i];
        msg += b.barcode;
        msg += "\t";
        msg += b.genbank;
       
        vector<string> values;
        if (b.length)
            values.push_back("Too Short");
        if (b.primers)
            values.push_back("Missing Primers");
        if (b.country)
            values.push_back("Missing Country");
        if (b.voucher)
            values.push_back("Missing Voucher");
        if (b.structured_voucher)
            values.push_back("Structured Voucher");
        if (!b.percent_n.empty())
            values.push_back("Too Many Ns:"+b.percent_n+"%");
        if (b.collection_date)
            values.push_back("Bad Collection Date");
        if (b.order_assignment)
            values.push_back("Missing Order Assignment");
        if (b.low_trace)
            values.push_back("Low Trace");
        if (b.frame_shift)
            values.push_back("Frame Shift");

        msg += "\t";
        msg += NStr::Join(values, ",");
        msg += "\n";
    }

    if (!msg.empty())
    {
        CGenericReportDlg* report = new CGenericReportDlg(this);
        report->SetTitle(wxT("BARCODE Keywords Removed"));
        report->SetText(wxString(msg));
        report->Show(true);
    }
}

void CBarcodeTool::OnApplyDbxrefs( wxCommandEvent& event ) // apply dbxrefs should act on all bioseqs
{
    CRef<CCmdComposite> cmd = ApplyDbxrefs(m_TopSeqEntry);
    m_CmdProcessor->Execute(cmd);
}

CRef<CCmdComposite> CBarcodeTool::ApplyDbxrefs(CSeq_entry_Handle seh)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Apply Dbxrefs"));
    
    for ( objects::CBioseq_CI b_iter(seh, objects::CSeq_inst::eMol_na); b_iter ; ++b_iter ) 
    {
        CBioseq_Handle bsh = *b_iter;

        string barcode;
        for (CBioseq_Handle::TId::const_iterator it = bsh.GetId().begin(); it != bsh.GetId().end(); ++it)
        {
            const CSeq_id &id = *(it->GetSeqId());
            if (id.IsGeneral() && id.GetGeneral().IsSetDb() && NStr::EqualNocase(id.GetGeneral().GetDb(), "uoguelph") && id.GetGeneral().IsSetTag())
            {
                const CObject_id &obj_id = id.GetGeneral().GetTag();
                if (obj_id.IsStr())
                    barcode = obj_id.GetStr();
                if (obj_id.IsId())
                    barcode = NStr::IntToString(obj_id.GetId());
            }
        }

        if (barcode.empty())
            continue;

        for (CSeqdesc_CI source_ci(bsh, CSeqdesc::e_Source); source_ci; ++source_ci)
        {
            bool found(false);
            if (source_ci->GetSource().IsSetOrg())
            {
                FOR_EACH_DBXREF_ON_ORGREF(dbxref, source_ci->GetSource().GetOrg())
                {
                    if ((*dbxref)->IsSetDb() && (*dbxref)->GetDb() == "BOLD" && (*dbxref)->IsSetTag())
                    {
                        string tag;
                        const CObject_id &obj_id = (*dbxref)->GetTag();
                        if (obj_id.IsStr())
                            tag = obj_id.GetStr();
                        if (obj_id.IsId())
                            tag = NStr::IntToString(obj_id.GetId());
                        if (tag == barcode)
                            found = true;
                    }
                }
            }
            if (!found)
            {
                CRef<CSeqdesc> new_desc(new CSeqdesc);
                new_desc->Assign(*source_ci);
                CRef< CDbtag > dbtag(new CDbtag);
                dbtag->SetDb("BOLD");
                dbtag->SetTag().SetStr(barcode);
                new_desc->SetSource().SetOrg().SetDb().push_back(dbtag);
                CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(source_ci.GetSeq_entry_Handle(), *source_ci, *new_desc));
                cmd->AddCommand (*ecmd);
            }
        }
    }
    return cmd;
}

void CBarcodeTool::ClickLink( int i ) // Clicking on record should take the user to that record in sequin 
{
    CBioseq_Handle bsh = m_barcode[i].bsh;
    CSeq_id_Handle best = sequence::GetId(bsh, sequence::eGetId_Best);
    string accession;
    int version;
    best.GetSeqId()->GetLabel(&accession, &version, CSeq_id::eContent);
    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    if (!srv) return;
    CIRef<IProjectView> pTextView(srv->FindView(*m_TopSeqEntry.GetObjectCore(), "Text View"));
    CIRef<IFlatFileCtrl> FlatFileCtrl(dynamic_cast<IFlatFileCtrl*>(pTextView.GetPointer()));
    
    if (FlatFileCtrl)
    {
        FlatFileCtrl->SetPosition(bsh, bsh.GetBioseqCore().GetPointer());
    }
}




IMPLEMENT_DYNAMIC_CLASS( CBarcodePanel, wxVScrolledWindow )


BEGIN_EVENT_TABLE(CBarcodePanel, wxVScrolledWindow)
    EVT_PAINT(CBarcodePanel::OnPaint)
    EVT_ERASE_BACKGROUND(CBarcodePanel::OnEraseBackground)
    EVT_SIZE(CBarcodePanel::OnResize)
    EVT_LEFT_DOWN(CBarcodePanel::OnMouseClick)
END_EVENT_TABLE()


CBarcodePanel::CBarcodePanel(wxWindow *parent, validator::TBarcodeResults *barcode, set<int> *selected,
                               wxWindowID id, const wxPoint &pos, const wxSize &size) 
: wxVScrolledWindow(parent,id, pos, size, wxFULL_REPAINT_ON_RESIZE), m_barcode(barcode), m_selected(selected)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    wxGraphicsContext *gc = wxGraphicsContext::Create();
    gc->SetFont(GetFont(), *wxBLACK);
    wxDouble width;
    wxDouble height;
    wxDouble descent;
    wxDouble externalLeading;
    gc->GetTextExtent(_("A"), &width, &height, &descent, &externalLeading);
    m_RowHeight = ceil(height) + 3;
    delete gc;
    SetRowCount(m_barcode->size());
}

wxCoord CBarcodePanel::OnGetRowHeight( size_t row ) const
{
    return m_RowHeight;
}

CBarcodePanel::~CBarcodePanel() 
{ 
}


// Empty implementation, to prevent flicker
void CBarcodePanel::OnEraseBackground(wxEraseEvent& event)
{
}

////////////////////////////////////////////////////////////
/// Notification for the derived class that moment is good
/// for doing its update and drawing stuff
////////////////////////////////////////////////////////////
void CBarcodePanel::OnUpdate()
{
}

void CBarcodePanel::OnResize(wxSizeEvent& event)
{
}

void CBarcodePanel::OnPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC dc(this);

    wxGraphicsContext *gc = wxGraphicsContext::Create( dc );
    wxColour backgroundColour = GetBackgroundColour();
    if (!backgroundColour.Ok())
        backgroundColour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);    
    wxSize sz = GetClientSize();
    wxRect windowRect(wxPoint(0,0), sz);    
    gc->SetBrush(wxBrush(backgroundColour));
    gc->SetPen(wxPen(backgroundColour, 1));
    gc->DrawRectangle(windowRect.GetX(),windowRect.GetY(),windowRect.GetWidth(),windowRect.GetHeight());
    delete gc;

    m_checkboxes.clear();
    m_links.clear();
    if (!m_barcode || !m_selected || m_barcode->empty())
        return;
    SetRowCount(m_barcode->size());
    int start = GetVisibleRowsBegin();
    int stop = GetVisibleRowsEnd();
    if (start >= m_barcode->size())
        start = m_barcode->size() - 1;
    if (stop >= m_barcode->size())
        stop = m_barcode->size() - 1;
    
    wxRendererNative &renderer = wxRendererNative::Get();
    wxRect rect(wxPoint(5, 5), renderer.GetCheckBoxSize(this));
    for (size_t i = start; i <= stop; i++)
    {
        renderer.DrawCheckBox(this, dc, rect, m_selected->find(i) != m_selected->end() ? wxCONTROL_CHECKED : 0);
        m_checkboxes.push_back(pair<size_t, wxRect>(i, rect));
        rect.x += 30 + 10;

        const validator::SBarcode &b = (*m_barcode)[i];
        rect.width += 150;
        dc.DrawLabel(wxString(b.barcode), rect);
        rect.x += 150 + 10;
        rect.width -= 150;
       

        rect.width += 100;
        dc.DrawLabel(wxString(b.genbank), rect);
        m_links.push_back(pair<size_t, wxRect>(i, rect));
        rect.x += 100 + 10;
        rect.width -= 100;

        renderer.DrawCheckBox(this, dc, rect, b.length ? wxCONTROL_CHECKED : 0);
        rect.x += 80 + 10;

        renderer.DrawCheckBox(this, dc, rect, b.primers ? wxCONTROL_CHECKED : 0);
        rect.x += 80 + 10;
        
        renderer.DrawCheckBox(this, dc, rect, b.country ? wxCONTROL_CHECKED : 0);
        rect.x += 80 + 10;

        renderer.DrawCheckBox(this, dc, rect, b.voucher ? wxCONTROL_CHECKED : 0);
        rect.x += 80 + 10;

        renderer.DrawCheckBox(this, dc, rect, b.structured_voucher ? wxCONTROL_CHECKED : 0);
        rect.x += 80 + 10;
       
        rect.width += 80;
        dc.DrawLabel(wxString(b.percent_n), rect);
        rect.x += 80 + 10;
        rect.width -= 80;

        renderer.DrawCheckBox(this, dc, rect, b.collection_date ? wxCONTROL_CHECKED : 0);
        rect.x += 80 + 10;

        renderer.DrawCheckBox(this, dc, rect, b.order_assignment ? wxCONTROL_CHECKED : 0);
        rect.x += 80 + 10;

        renderer.DrawCheckBox(this, dc, rect, b.low_trace ? wxCONTROL_CHECKED : 0);
        rect.x += 80 + 10;

        renderer.DrawCheckBox(this, dc, rect, b.frame_shift ? wxCONTROL_CHECKED : 0);
        rect.x += 80 + 10;

        renderer.DrawCheckBox(this, dc, rect, b.has_keyword ? wxCONTROL_CHECKED : 0);
        rect.x += 80 + 10;

        rect.y += m_RowHeight;
        rect.x = 5;
    }
}

void CBarcodePanel::OnMouseClick(wxMouseEvent& evt)
{  
    wxPoint p = evt.GetPosition();
    wxWindow *win = GetGrandParent();
    CBarcodeTool *parent = dynamic_cast<CBarcodeTool *>(win);
    if (!parent)
    {
        evt.Skip();
        return;
    }
    bool found(false);
    for (size_t k = 0; k < m_checkboxes.size(); k++)
    {
        if (m_checkboxes[k].second.Contains(p))
        {
            int i = m_checkboxes[k].first;            
            parent->CheckRow(i);
            found = true;
            break;
        }
    }
    
    if (!found)
    {
        for (size_t k = 0; k < m_links.size(); k++)
        {
            if (m_links[k].second.Contains(p))
            {
                int i = m_links[k].first;            
                parent->ClickLink(i);
                found = true;
                break;
            }
        }
    }

    evt.Skip();
}


CRef<CCmdComposite> CApplyFBOL::apply(objects::CSeq_entry_Handle seh) 
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Apply FBOL Dbxrefs"));
    objects::CBioseq_CI b_iter(seh, objects::CSeq_inst::eMol_na);                
    for ( ; b_iter ; ++b_iter ) 
    {
        CBioseq_Handle bsh = *b_iter;

        string fbol;
        for (CBioseq_Handle::TId::const_iterator it = bsh.GetId().begin(); it != bsh.GetId().end(); ++it)
        {
            const CSeq_id &id = *(it->GetSeqId());
            if (id.IsGeneral() && id.GetGeneral().IsSetDb() && NStr::EqualNocase(id.GetGeneral().GetDb(), "FBOL") && id.GetGeneral().IsSetTag())
            {
                const CObject_id &obj_id = id.GetGeneral().GetTag();
                if (obj_id.IsStr())
                    fbol = obj_id.GetStr();
                if (obj_id.IsId())
                    fbol = NStr::IntToString(obj_id.GetId());
            }
        }

        if (fbol.empty())
            continue;

        for (CSeqdesc_CI source_ci(bsh, CSeqdesc::e_Source); source_ci; ++source_ci)
        {
            bool found(false);
            if (source_ci->GetSource().IsSetOrg())
            {
                FOR_EACH_DBXREF_ON_ORGREF(dbxref, source_ci->GetSource().GetOrg())
                {
                    if ((*dbxref)->IsSetDb() && (*dbxref)->GetDb() == "FBOL")
                    {
                        found = true;
                    }
                }
            }
            if (!found)
            {
                CRef<CSeqdesc> new_desc(new CSeqdesc);
                new_desc->Assign(*source_ci);
                CRef< CDbtag > dbtag(new CDbtag);
                dbtag->SetDb("FBOL");
                dbtag->SetTag().SetStr(fbol);
                new_desc->SetSource().SetOrg().SetDb().push_back(dbtag);
                CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(source_ci.GetSeq_entry_Handle(), *source_ci, *new_desc));
                cmd->AddCommand (*ecmd);
            }
        }
    }
    return cmd;
}

END_NCBI_SCOPE
