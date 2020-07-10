/*  $Id: uncul_tax_tool.cpp 43676 2019-08-14 14:28:05Z asztalos $
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
#include <objects/misc/sequence_macros.hpp>

#include <objmgr/seq_entry_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/utils.hpp>
#include <objects/taxon3/taxon3.hpp>
#include <objtools/edit/source_edit.hpp>
#include <objtools/cleanup/cleanup.hpp>
#include <gui/objutils/cmd_change_seq_entry.hpp>
#include <gui/framework/view_manager_service.hpp>
#include <gui/framework/gui_package.hpp>
#include <gui/framework/pkg_wb_connect.hpp>
#include <gui/framework/workbench_impl.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>

#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/tbl_edit_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/uncul_tax_tool.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_util.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_select.hpp>
#include <gui/packages/pkg_sequence_edit/apply_edit_convert_panel.hpp>

#include <gui/objutils/cmd_composite.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);


IMPLEMENT_DYNAMIC_CLASS( CUnculTaxTool, wxFrame )


/*!
 * CUnculTaxTool event table definition
 */

BEGIN_EVENT_TABLE( CUnculTaxTool, wxFrame )

EVT_BUTTON(ID_ADD_SP_UNCUL_TAXTOOL, CUnculTaxTool::OnAddSp)
EVT_BUTTON(ID_ADD_BACTERIUM_UNCUL_TAXTOOL, CUnculTaxTool::OnAddBacterium)
EVT_BUTTON( ID_CANCEL_BTN, CUnculTaxTool::OnCancel)
EVT_BUTTON( ID_APPLY_BTN, CUnculTaxTool::OnApply)
EVT_BUTTON( ID_REFRESH_BTN, CUnculTaxTool::OnRefreshBtn)
EVT_BUTTON( ID_TRIM_BTN, CUnculTaxTool::TrimSuggestions)

END_EVENT_TABLE()


/*!
 * CUnculTaxTool constructors
 */

CUnculTaxTool::CUnculTaxTool()
{
    Init();
}

CUnculTaxTool::CUnculTaxTool( wxWindow* parent, CSeq_entry_Handle seh, IWorkbench* workbench,
                          wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_TopSeqEntry(seh), m_Workbench(workbench)
{
    Init();
    Create(parent, id, caption, pos, size, style);
    NEditingStats::ReportUsage(caption);
}


/*!
 * CVectorTrimDlg creator
 */

bool CUnculTaxTool::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CUnculTaxTool creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre(wxBOTH|wxCENTRE_ON_SCREEN);
////@end CUnculTaxTool creation

    return true;
}


/*!
 * CUnculTaxTool destructor
 */

CUnculTaxTool::~CUnculTaxTool()
{
////@begin CUnculTaxTool destruction
////@end CUnculTaxTool destruction
}


/*!
 * Member initialisation
 */

void CUnculTaxTool::Init()
{
    m_GridPanel = NULL;
}


/*!
 * Control creation for CUnculTaxTool
 */

void CUnculTaxTool::CreateControls()
{    
    CUnculTaxTool* itemFrame1 = this;

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    SetSizer(itemBoxSizer1);

    wxPanel* itemDialog1 = new wxPanel( itemFrame1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemBoxSizer1->Add(itemDialog1, 1, wxGROW, 0);


    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxArrayString itemChoiceStrings, itemChoiceStringsWritable;

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP, 5);

  
    FindBioSource(m_TopSeqEntry);
    

    if (m_BioSource.empty()) 
    {
        wxMessageBox(wxT("No bad taxnames"), wxT("Error"), wxOK | wxICON_ERROR);
        NCBI_THROW( CException, eUnknown, "No bad taxnames" );
    }

    CRef<CSeq_table> values_table = GetValuesTableFromSeqEntry();
    if (values_table->GetNum_rows() < 1)
    {
        wxMessageBox(wxT("No bad taxnames"), wxT("Error"), wxOK | wxICON_ERROR);
        NCBI_THROW( CException, eUnknown, "No bad taxnames" );
    }
    CRef<CSeq_table> choices = GetChoices(values_table);
    int glyph_col =   GetCollapsible();
    m_GridPanel = new CSeqTableGridPanel(itemDialog1, values_table, choices, glyph_col);
    itemBoxSizer3->Add(m_GridPanel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_Grid = m_GridPanel->GetGrid();
    CSeqTableGrid *gridAdapter = new CSeqTableGrid(values_table);
    m_Grid->SetTable(gridAdapter, true);
    m_Grid->AutoSizeColumns();
    int l_height = m_Grid->GetColLabelSize();
    m_Grid->SetColLabelSize( 2 * l_height );
    
    int pos = 0;
    ITERATE (CSeq_table::TColumns, it, values_table->GetColumns()) 
    {
        if (pos > 0) 
        {
            if ((*it)->IsSetHeader() && (*it)->GetHeader().IsSetTitle() )
            {
                string title = (*it)->GetHeader().GetTitle();
                if (!title.empty())
                {
                    itemChoiceStrings.Add(wxString(title));
                    if (!IsReadOnlyColumn(title)) 
                        itemChoiceStringsWritable.Add(wxString(title));
                }
                if (IsReadOnlyColumn(title)) 
                    m_GridPanel->MakeColumnReadOnly(pos - 1, true);
            } 
        }
        pos++;
    }
    
    if (glyph_col >= 0 && glyph_col+2 < m_Grid->GetNumberCols())
    {
        m_GridPanel->InitColumnCollapse(glyph_col+2);  
    }
    
    
    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT, 5);

    CStringConstraintSelect *itemStringConstraintPanel = new CStringConstraintSelect( itemDialog1, m_GridPanel, itemChoiceStrings, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStringConstraintPanel, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxFIXED_MINSIZE, 0);
    
    CApplyEditconvertPanel *itemAecrPanel = new CApplyEditconvertPanel( itemDialog1, m_GridPanel, itemChoiceStringsWritable, 1, false, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    itemBoxSizer2->Add(itemAecrPanel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxFIXED_MINSIZE, 1);   
    
    wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer15, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);  

    wxButton* itemButton13 = new wxButton( itemDialog1, ID_APPLY_BTN, _("Apply Corrections"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton15 = new wxButton( itemDialog1, ID_REFRESH_BTN, _("Refresh"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(itemButton15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton11 = new wxButton( itemDialog1, ID_ADD_SP_UNCUL_TAXTOOL, _("Add sp."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(itemButton11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton12 = new wxButton( itemDialog1, ID_ADD_BACTERIUM_UNCUL_TAXTOOL, _("Add bacterium"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(itemButton12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton16 = new wxButton( itemDialog1, ID_TRIM_BTN, _("Trim Suggestion"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(itemButton16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemDialog1, ID_CANCEL_BTN, _("Dismiss"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}


CRef<CSeq_table> CUnculTaxTool::GetValuesTableFromSeqEntry() 
{

    CRef<CSeqTable_column> id_col(new CSeqTable_column());
    id_col->SetHeader().SetField_id(CSeqTable_column_info::eField_id_location_id);
    id_col->SetHeader().SetTitle(kSequenceIdColLabel);

    CRef<CSeqTable_column> expand_col(new CSeqTable_column());
    expand_col->SetHeader().SetTitle("");
    expand_col->SetHeader().SetField_name("expand");
    expand_col->SetData().SetString();

    CRef<CSeqTable_column> current_col(new CSeqTable_column());
    current_col->SetHeader().SetTitle("Taxname");
    current_col->SetHeader().SetField_name("current");
    current_col->SetData().SetString();

    CRef<CSeqTable_column> suggested_col(new CSeqTable_column());
    suggested_col->SetHeader().SetTitle("Suggested Correction");
    suggested_col->SetHeader().SetField_name("suggested");
    suggested_col->SetData().SetString();

   
    // bogus column to include last, otherwise deletion of the previous column will not work
    CRef<CSeqTable_column> bogus_col(new CSeqTable_column());
    bogus_col->SetHeader().SetTitle("");
    bogus_col->SetHeader().SetField_name("");
    bogus_col->SetData().SetString();

    CRef<CSeq_table> table(new CSeq_table());
    table->SetColumns().push_back(id_col);
    table->SetColumns().push_back(expand_col);
    table->SetColumns().push_back(current_col);
    table->SetColumns().push_back(suggested_col);   
    table->SetColumns().push_back(bogus_col);


    vector<string> suggestions;
    vector<bool> verified;
    LookupSuggestions(suggestions, verified);

    size_t row = 0;
    for (unsigned int i = 0; i<m_BioSource.size(); i++)
    {
        string taxname = m_BioSource[i].second->GetSource().GetTaxname();
        string suggested = suggestions[i];
        if (taxname == suggested && verified[i])
            continue;
        CRef<CSeq_id> id(new CSeq_id());
        id->SetLocal().SetId(row);
        id_col->SetData().SetId().push_back(id);
        expand_col->SetData().SetString().push_back("");
        current_col->SetData().SetString().push_back(taxname);
        suggested_col->SetData().SetString().push_back(suggested);
        bogus_col->SetData().SetString().push_back("");
        row++;
    }    
   
    table->SetNum_rows(row); 

    return table;
}

/*!
 * Should we show tooltips?
 */

bool CUnculTaxTool::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CUnculTaxTool::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CUnculTaxTool::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}


CRef<CCmdComposite> CUnculTaxTool::GetCommandFromValuesTable(CRef<CSeq_table> values_table) 
{
    CRef<CCmdComposite> composite( new CCmdComposite("Uncul Tax Tool") );
    m_UnindexedObjects.clear();
    bool select_all = false;
    if (!m_Grid->IsSelection() && values_table->GetNum_rows() > 0)
    {
        if (wxMessageBox(ToWxString("Select All?"), wxT("Nothing is selected"), wxOK | wxCANCEL, NULL) == wxOK) 
        {
            select_all = true;
        }
    }
 
    map<string,string> current_to_suggested;
    for (unsigned int i = 0; i < values_table->GetColumn("current").GetData().GetString().size(); i++) 
    {
        string current = values_table->GetColumn("current").GetData().GetString()[i];
        string suggested = values_table->GetColumn("suggested").GetData().GetString()[i];
        if (!current.empty() && !suggested.empty() && current != suggested  && (m_Grid->IsInSelection(i,0) || select_all))
        {
            current_to_suggested[current] = suggested;           
        }
    }
    
  

    for (unsigned int i=0; i<m_BioSource.size(); i++)
    {

        string taxname = m_BioSource[i].second->GetSource().GetTaxname();
        if (current_to_suggested.find(taxname) != current_to_suggested.end())
        {
            CRef<CSeqdesc> new_desc(new CSeqdesc);
            new_desc->Assign(m_BioSource[i].second.GetObject());
            CBioSource& edited_biosource = new_desc->SetSource();
            
            edited_biosource.SetOrg().SetTaxname(current_to_suggested[taxname]);
            edit::CleanupForTaxnameChange(edited_biosource);
            if (m_ConvertNote.find(i) != m_ConvertNote.end())
                ConvertSpeciesSpecificNote(edited_biosource,"amplified with species-specific primers");
            if (m_RemoveNote.find(i) != m_RemoveNote.end())
                ConvertSpeciesSpecificNote(edited_biosource,"");
            
            CSeq_entry_Handle seh = m_TopSeqEntry.GetScope().GetSeq_entryHandle(*m_BioSource[i].first);
            CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc( seh, *m_BioSource[i].second, *new_desc));
            composite->AddCommand(*cmd);

            SUnindexedObject sobj;
            const CBioSource& biosource = m_BioSource[i].second->GetSource();
            sobj.actual_obj.Reset(const_cast<CBioSource*>(&biosource));
            sobj.new_obj.Reset(&edited_biosource);
            
            CRef<CBioSource> orig_obj(new CBioSource());
            orig_obj->Assign(biosource);
            sobj.orig_obj.Reset(orig_obj);
            m_UnindexedObjects.push_back(sobj);
        }
    }
 
    return composite;
}


CRef<CCmdComposite> CUnculTaxTool::GetCommand()
{
    CRef<CSeq_table> values_table = m_GridPanel->GetValuesTable();
    CRef<CCmdComposite> cmd = GetCommandFromValuesTable(values_table);
    return cmd;
}


string CUnculTaxTool::StandardFixes(const CBioSource &biosource)
{
    string taxname = biosource.GetTaxname();
    string old;
    while (old != taxname)
    {
        old = taxname;
        const string uncultured = "uncultured ";
        const string sp = " sp";
        const string spdot = " sp.";
        if (NStr::StartsWith(taxname,uncultured,NStr::eNocase))
        {
            taxname = taxname.substr(uncultured.length());
        }
        if (NStr::EndsWith(taxname,sp,NStr::eNocase))
        {
            taxname = taxname.substr(0,taxname.length()-sp.length());
        }
        if (NStr::EndsWith(taxname,spdot,NStr::eNocase))
        {
            taxname = taxname.substr(0,taxname.length()-spdot.length());
        }
        NStr::ReplaceInPlace(taxname,", "," ");
        NStr::ReplaceInPlace(taxname,","," ");
    }
    return taxname;
}

static const char* sAmplifiedSpeciesSpecific[] = {
  "[BankIt_uncultured16S_wizard]; [species_specific primers]; [tgge]",
  "[BankIt_uncultured16S_wizard]; [species_specific primers]; [dgge]",
  "[BankIt_uncultured16S_wizard]; [species_specific primers]",
  "[uncultured (with species-specific primers)]",
  "[uncultured]; [amplified with species-specific primers]",
  "[uncultured (using species-specific primers) bacterial source]",
  "amplified with species-specific primers",
  NULL
};

bool CUnculTaxTool::IsSpeciesSpecific(const CBioSource &biosource)
{
    if (biosource.IsSetSubtype())
        for (CBioSource::TSubtype::const_iterator subtype = biosource.GetSubtype().begin(); subtype != biosource.GetSubtype().end(); ++subtype)
            if ((*subtype)->IsSetSubtype() && (*subtype)->GetSubtype() ==  CSubSource::eSubtype_other && (*subtype)->IsSetName())
            {
                string name = (*subtype)->GetName();
                int i=0;
                while (sAmplifiedSpeciesSpecific[i])
                {
                    if (NStr::FindNoCase(name,sAmplifiedSpeciesSpecific[i]) != NPOS)
                        return true;
                    i++;
                }
            }
    return false;
}

void CUnculTaxTool::ConvertSpeciesSpecificNote(CBioSource &biosource, const string &new_note)
{
    if (biosource.IsSetSubtype())
    {
        CBioSource::TSubtype::iterator subtype = biosource.SetSubtype().begin();
        while ( subtype != biosource.SetSubtype().end())
        {
            bool erased = false;
            if ((*subtype)->IsSetSubtype() && (*subtype)->GetSubtype() ==  CSubSource::eSubtype_other && (*subtype)->IsSetName())
            {
                string name = (*subtype)->GetName();
                int i=0;
                while (sAmplifiedSpeciesSpecific[i])
                {
                    if (NStr::FindNoCase(name,sAmplifiedSpeciesSpecific[i]) != NPOS)
                    {
                        if (new_note.empty())
                        {
                            subtype =  biosource.SetSubtype().erase(subtype);
                            erased = true;
                        }
                        else
                            (*subtype)->SetName(new_note);
                        break;
                    }
                    i++;
                }
            }
            if (!erased)
                ++subtype;
        }
    }
}


CRef<CT3Reply> CUnculTaxTool::GetReply(const CBioSource &biosource, const string &standard_taxname)
{
    if (m_ReplyCache.find(standard_taxname) != m_ReplyCache.end())
        return m_ReplyCache[standard_taxname];

    vector<CRef<COrg_ref> > rq_list;
    CRef<COrg_ref> org(new COrg_ref());
    org->Assign(biosource.GetOrg());
    org->SetTaxname(standard_taxname);
    rq_list.push_back(org);

    CTaxon3 taxon3;
    taxon3.Init();
    CRef<CTaxon3_reply> reply;
    reply = taxon3.SendOrgRefList(rq_list);
    CRef<CT3Reply> t3reply;
    if (reply->IsSetReply() && !reply->GetReply().empty())
    {
        t3reply =  reply->GetReply().front();
        m_ReplyCache[standard_taxname] = t3reply;
    }
    return t3reply;
}

string CUnculTaxTool::GetRank(CRef<CT3Reply> reply)
{
    string rank;
   
    if ( reply && reply->IsData() && reply->GetData().IsSetStatus()) 
        for (CT3Data::TStatus::const_iterator status = reply->GetData().GetStatus().begin(); status != reply->GetData().GetStatus().end(); ++status)
        {
            if ((*status)->IsSetProperty() && (*status)->GetProperty() == "rank" && (*status)->IsSetValue() && (*status)->GetValue().IsStr())
                rank = (*status)->GetValue().GetStr();
        }
    return rank;
}

string CUnculTaxTool::GetSuggestion(CRef<CT3Reply> reply)
{
    string suggestion;
    if (reply && reply->IsData()) 
    {
        suggestion = reply->GetData().GetOrg().GetTaxname();
    }

    return suggestion;
}

bool CUnculTaxTool::CompareOrgnameLineage(CRef<CT3Reply> reply, const string &lineage)
{
    bool result = false;
    if (reply && reply->IsData() && reply->GetData().IsSetOrg() && reply->GetData().GetOrg().IsSetOrgname() 
        && reply->GetData().GetOrg().GetOrgname().IsSetLineage() 
        && NStr::FindNoCase(reply->GetData().GetOrg().GetOrgname().GetLineage(),lineage) != NPOS)
    {
        result = true;
    }
    return result;
}


string CUnculTaxTool::MakeUnculturedName(const string &taxname, const string suffix)
{
    return "uncultured "+taxname+suffix;
}

bool CUnculTaxTool::CheckSuggestedFix(const CBioSource &biosource, string &suggestion)
{
    CRef<CT3Reply> reply2 = GetReply(biosource, suggestion);    
    string rank2 = GetRank(reply2);
    if (rank2 == "species")
    {
        suggestion = GetSuggestion(reply2); // Note that this does not only perform a check - it also potentially modifies suggestion. This is how it's done in the original sequin code.
        return true;
    }
    return false;
}

bool CUnculTaxTool::IsAmbiguous(CRef<CT3Reply> reply)
{
    bool result = false;
    if (reply && reply->IsError() && reply->GetError().IsSetMessage() && reply->GetError().GetMessage() == "Taxname is ambiguous")
    {
        result = true;
    }
    return result;
}

string CUnculTaxTool::TryRankFix(CRef<CT3Reply>  reply, unsigned int i, string &name) // In case of binomial truncation the input name will be modified. This is what's happening in the original sequin code
{
    const CBioSource &biosource = m_BioSource[i].second->GetSource();
    string suggestion;
    string rank = GetRank(reply);  
     
    bool is_species_specific = IsSpeciesSpecific(biosource);
    if (rank == "species")
    {
        if ( is_species_specific )
        {
            suggestion = GetSuggestion(reply);
            m_ConvertNote.insert(i);
        }
        else  if (NStr::FindNoCase(name," ") != NPOS)
        {
            string tmp,name2;
            NStr::SplitInTwo(name," ",name2,tmp);
            name = name2;
            CRef<CT3Reply> reply2 = GetReply(biosource,name);
            if (!reply2)
                return suggestion;
            return TryRankFix(reply2,i,name);
        }
    }
    else
    {
        if ( is_species_specific)
        {
            m_RemoveNote.insert(i);
        }
        if (rank == "genus")
        {
            if (CompareOrgnameLineage(reply,"archaea") || CompareOrgnameLineage(reply,"bacteria"))
                suggestion = MakeUnculturedName(GetSuggestion(reply), " sp.");
            else if (CompareOrgnameLineage(reply," Fungi;"))
                suggestion = MakeUnculturedName(GetSuggestion(reply));
        }
        else
        {
            if (CompareOrgnameLineage(reply,"archaea")) 
                suggestion = MakeUnculturedName (GetSuggestion(reply), " archaeon");
            else if (CompareOrgnameLineage(reply,"bacteria")) 
                suggestion = MakeUnculturedName(GetSuggestion(reply), " bacterium");
            else if (CompareOrgnameLineage(reply," Fungi;")) 
                suggestion = MakeUnculturedName(GetSuggestion(reply));            
        }
    }    
    return suggestion;
}

void CUnculTaxTool::PreloadCache()
{
    m_ReplyCache.clear();
    set<string> submit;
    for (unsigned int i=0; i<m_BioSource.size(); i++)
    {
        string standard_taxname = StandardFixes(m_BioSource[i].second->GetSource());
        if (standard_taxname.empty())
            continue;
        submit.insert(standard_taxname);
        submit.insert(MakeUnculturedName(standard_taxname," bacterium"));
        submit.insert(MakeUnculturedName(standard_taxname," archaeon"));
        submit.insert(MakeUnculturedName(standard_taxname, " sp."));
        submit.insert(MakeUnculturedName(standard_taxname));
        if (NStr::FindNoCase(standard_taxname," ") != NPOS)
        {
            string tmp,name2;
            NStr::SplitInTwo(standard_taxname," ",name2,tmp);
            standard_taxname = name2;
            submit.insert(standard_taxname);
            submit.insert(MakeUnculturedName(standard_taxname," bacterium"));
            submit.insert(MakeUnculturedName(standard_taxname," archaeon"));
            submit.insert(MakeUnculturedName(standard_taxname, " sp."));
            submit.insert(MakeUnculturedName(standard_taxname));
        }       
    }


    vector<CRef<COrg_ref> > rq_list;
    for (set<string>::iterator name = submit.begin(); name != submit.end(); ++name)
        {
            CRef<COrg_ref> org(new COrg_ref());
            org->SetTaxname(*name);
            rq_list.push_back(org);
        }
    CTaxon3 taxon3;
    taxon3.Init();
    CRef<CTaxon3_reply> reply;
    reply = taxon3.SendOrgRefList(rq_list);
    if (reply->IsSetReply() )
    {
        unsigned int i=0;
        for (CTaxon3_reply::TReply::const_iterator reply_it = reply->GetReply().begin(); reply_it != reply->GetReply().end(); ++reply_it)
        {
//            if ((*reply_it)->IsData())
            {
                m_ReplyCache[rq_list[i]->GetTaxname()] = *reply_it;
            }
            i++;
        }
    }
}

void CUnculTaxTool::LookupSuggestions(vector<string> &suggestions, vector<bool> &verified)
{
    PreloadCache();
    suggestions.clear();
    suggestions.resize(m_BioSource.size());
    verified.clear();
    verified.resize(m_BioSource.size(), true);
    for (unsigned int i=0; i<m_BioSource.size(); i++)
    {
        const CBioSource& bsrc = m_BioSource[i].second->GetSource();
        string standard_taxname = StandardFixes(bsrc);
        if (standard_taxname.empty())
            continue;
        CRef<CT3Reply> reply = GetReply(bsrc, standard_taxname);        
        if (!reply)
            continue;
        if (IsAmbiguous(reply))
        {
            suggestions[i] = MakeUnculturedName(standard_taxname," bacterium");
            if (CheckSuggestedFix(bsrc, suggestions[i]))
                continue;
            suggestions[i] = MakeUnculturedName(standard_taxname," archaeon");
            if (CheckSuggestedFix(bsrc, suggestions[i]))
                continue;
        }

        suggestions[i] = TryRankFix(reply,i,standard_taxname);        
        if (CheckSuggestedFix(bsrc, suggestions[i]))
            continue;

        suggestions[i] = MakeUnculturedName(standard_taxname);
        if (CheckSuggestedFix(bsrc, suggestions[i]))
            continue;
        suggestions[i] = MakeUnculturedName(standard_taxname, " sp.");
        if (CheckSuggestedFix(bsrc, suggestions[i]))
            continue;
        suggestions[i] = MakeUnculturedName(standard_taxname);
        verified[i] = CheckSuggestedFix(bsrc, suggestions[i]);
    }
}

static const char* s_UntrimmableWords[] = { "sp.", "cf.", "aff.", "bacterium", "archaeon", NULL };

void CUnculTaxTool::TrimSuggestions(wxCommandEvent& event )
{
    CRef<CSeq_table> values_table = m_GridPanel->GetValuesTable();

    for (unsigned int i = 0; i < values_table->GetColumn("suggested").GetData().GetString().size(); i++) 
    {
        string suggested = values_table->GetColumn("suggested").GetData().GetString()[i];
        if (suggested.empty())
            continue;
        int pos = NStr::FindNoCase(suggested," ");
        if ( pos != NPOS && pos !=0 && !NStr::StartsWith(suggested,"uncultured "))
        {
            bool no_fix = false;
            for (int j = 0; s_UntrimmableWords[j] != NULL && !no_fix; j++) 
                if ( NStr::EqualNocase(suggested,pos + 1, suggested.length()-pos-1,s_UntrimmableWords[j])) 
                    no_fix = true;
            if (!no_fix)
                values_table->SetColumns()[3]->SetData().SetString()[i] = suggested.substr(0,pos);
        }
    }
    int glyph_col = GetCollapsible();
    if (glyph_col >= 0 && glyph_col+2 < m_Grid->GetNumberCols())
    {        
        m_GridPanel->InitColumnCollapse(glyph_col+2);  
    }  
}

static const char* sUnfixable[] = {
  "rickettsia",
  "candidatus",
  "endosymbiont",
  "phytoplasma",
  "wolbachia",
  NULL
};


bool CUnculTaxTool::OkToTaxFix(const string& taxname)
{
    int i=0;
    while (sUnfixable[i])
    {
        if (NStr::FindNoCase(taxname,sUnfixable[i]) != NPOS)
            return false;
        i++;
    }
    return true;
}

void CUnculTaxTool::AddBioSource(const CSeq_entry& seq, const CSeqdesc& desc) //const CBioSource& biosource)
{
    const CBioSource& biosource = desc.GetSource();
    if (biosource.IsSetTaxname() && !biosource.GetTaxname().empty() && OkToTaxFix(biosource.GetTaxname()) )
    {
        bool taxon_present = false;
        if  (biosource.IsSetOrg() && biosource.GetOrg().IsSetDb())       
            {
                FOR_EACH_DBXREF_ON_ORGREF(db,biosource.GetOrg())
                    if ((*db)->IsSetDb() && NStr::Equal((*db)->GetDb(),"taxon",NStr::eNocase))
                    {
                        taxon_present = true;
                        break;
                    }
            }
        bool begins_with_uncultured =  NStr::StartsWith(biosource.GetTaxname(), "uncultured", NStr::eNocase);
        
        if (!taxon_present || !begins_with_uncultured)
            m_BioSource.emplace_back(ConstRef(&seq), ConstRef(&desc));
    }
}

void CUnculTaxTool::GetDesc(const CSeq_entry& se)  
{
    FOR_EACH_SEQDESC_ON_SEQENTRY(it, se) {
        if ((*it)->IsSource()) {
            AddBioSource(se, **it);
        }
    }

    if (se.IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET (it, se.GetSet()) {
            GetDesc (**it);
        }
    }
}


void  CUnculTaxTool::FindBioSource(CSeq_entry_Handle tse)
{
    m_BioSource.clear();
    m_ConvertNote.clear();
    m_RemoveNote.clear();
    if (!tse) 
        return;
    GetDesc (*(tse.GetCompleteSeq_entry()));
}

int CUnculTaxTool::GetColumn()
{
    int col = -1;
    for (int i=0; i < m_Grid->GetNumberCols(); i++)
        if (m_Grid->GetColLabelValue(i) == _("Suggested Correction"))
        {
            col = i;
            break;
        }
    return col;
}

void CUnculTaxTool::OnAddBacterium( wxCommandEvent& event )
{
    int col = GetColumn();
    if (col == -1)
        return;
    
    for (int i=0; i<m_Grid->GetNumberRows(); i++)
    {
        wxString value = m_Grid->GetCellValue(i,col);
        m_Grid->SetCellValue(i,col,value+" bacterium");
    }
}

void CUnculTaxTool::OnAddSp( wxCommandEvent& event )
{   
    int col = GetColumn();
    if (col == -1)
        return;
    
    for (int i=0; i<m_Grid->GetNumberRows(); i++)
    {
        wxString value = m_Grid->GetCellValue(i,col);
        m_Grid->SetCellValue(i,col,value+" sp.");
    }
}

void CUnculTaxTool::OnApply( wxCommandEvent& event )
{
    if (!m_TopSeqEntry) return;
    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    if (srv)
    {
        CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
        if (ws) 
        {
            CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(m_TopSeqEntry.GetScope()));
            if (doc)
            {
                ICommandProccessor* cmdProccessor = &doc->GetUndoManager(); 
                CRef<CCmdComposite> cmd = GetCommand();
                if (cmd)  
                {
                    NON_CONST_ITERATE(TUnindexedObjects, iter, m_UnindexedObjects) 
                    {
                        SUnindexedObject& sobj = *iter;
                        sobj.actual_obj->Assign(*sobj.new_obj);
                    } 
                    CRef<CCmdComposite> cleanup = CleanupCommand (m_TopSeqEntry, true, false); // extended, do_tax
                    NON_CONST_ITERATE(TUnindexedObjects, iter, m_UnindexedObjects) 
                     {
                         SUnindexedObject& sobj = *iter;
                         sobj.actual_obj->Assign(*sobj.orig_obj);
                     }
                    if (cleanup)
                    {
                        cmd->AddCommand(*cleanup);
                    }
                    cmdProccessor->Execute(cmd);    
                }
            }
        }
    }
}

void CUnculTaxTool::OnCancel( wxCommandEvent& event )
{
    Close();
}


void CUnculTaxTool::OnRefreshBtn( wxCommandEvent& event )
{
    m_TopSeqEntry.Reset();
    if (!m_Workbench) return;
    
    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (sel_srv) 
    {
        sel_srv->GetActiveObjects(objects);
    }
    if (objects.empty())
    {
        GetViewObjects(m_Workbench,objects);
    }

    NON_CONST_ITERATE (TConstScopedObjects, it, objects) {                
        CSeq_entry_Handle seh = GetTopSeqEntryFromScopedObject(*it);
        if (seh) {
            m_TopSeqEntry = seh;
        }
    }

    FindBioSource(m_TopSeqEntry);

    CRef<CSeq_table> values_table = GetValuesTableFromSeqEntry();
    m_GridPanel->SetValuesTable(values_table);
    m_Grid = m_GridPanel->GetGrid();
    CSeqTableGrid *gridAdapter = new CSeqTableGrid(values_table);
    m_Grid->SetTable(gridAdapter, true);
    m_Grid->AutoSizeColumns();
    if (values_table->GetNum_rows() > 0)
    {
        int glyph_col =   GetCollapsible();
        if (glyph_col >= 0 && glyph_col+2 < m_Grid->GetNumberCols())
        {
            m_GridPanel->InitColumnCollapse(glyph_col+2);  
        }
    }
    Refresh();
}

END_NCBI_SCOPE
