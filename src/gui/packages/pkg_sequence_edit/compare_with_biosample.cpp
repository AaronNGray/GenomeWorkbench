/*  $Id: compare_with_biosample.cpp 43987 2019-10-03 20:31:08Z filippov $
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
#include <corelib/ncbi_system.hpp>
#include <misc/jsonwrapp/jsonwrapp.hpp>
#include <misc/xmlwrapp/xmlwrapp.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objects/seqtable/seq_table_exception.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/packages/pkg_sequence_edit/tbl_edit_dlg.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/existingtextdlg.hpp>
#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>
#include <gui/packages/pkg_sequence_edit/compare_with_biosample.hpp>

#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include <sstream>
BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);



IMPLEMENT_DYNAMIC_CLASS( CCompareWithBiosample, wxFrame )

BEGIN_EVENT_TABLE( CCompareWithBiosample, wxFrame )
    EVT_BUTTON( ID_UPDATE_BIOSOURCE, CCompareWithBiosample::OnClickOk )
    EVT_BUTTON( wxID_CANCEL, CCompareWithBiosample::OnClickCancel )
    EVT_BUTTON( ID_SYNC, CCompareWithBiosample::OnSyncButtonClick )
    EVT_BUTTON( ID_COPY_TO_SOURCE, CCompareWithBiosample::OnCopyToSourceButtonClick )
    EVT_BUTTON( ID_COPY_TO_SAMPLE, CCompareWithBiosample::OnCopyToSampleButtonClick )
    EVT_BUTTON( ID_ADD_TO_SOURCE, CCompareWithBiosample::OnAddToSource )
    EVT_BUTTON( ID_ADD_TO_SAMPLE, CCompareWithBiosample::OnAddToSample )
    EVT_BUTTON( ID_MAKE_REPORT, CCompareWithBiosample::MakeReport )
    EVT_BUTTON( ID_CREATE_BIOSAMPLE, CCompareWithBiosample::CreateBiosampleUpdateWebService )
    EVT_BUTTON(ID_REFRESH_BIOSAMPLE, CCompareWithBiosample::OnRefresh)
    EVT_BUTTON(ID_LOGOUT_BIOSAMPLE, CCompareWithBiosample::OnLogout)
END_EVENT_TABLE()

CCompareWithBiosample::CCompareWithBiosample()
{
    Init();
}

// https://svn.ncbi.nlm.nih.gov/viewvc/toolkit/trunk/c%2B%2B/src/app/biosample_chk/
CCompareWithBiosample::CCompareWithBiosample( wxWindow* parent, objects::CSeq_entry_Handle seh, ICommandProccessor* cmdProcessor, bool all_records,
                    wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_TopSeqEntry(seh), m_CmdProcessor(cmdProcessor), m_all_records(all_records)
{
    LoadData();
   
    Init();
    Create(parent, id, caption, pos, size, style);
    NEditingStats::ReportUsage(caption);
    LoadSettings();
}

void CCompareWithBiosample::LoadData()
{
    m_DiffVec.clear();
    m_Cache.clear();
    m_ids.clear();
    
    size_t num_processed;
    vector<string> unprocessed_ids;
//    bool use_dev_server(true); // DEBUG
    bool use_dev_server(false);  
    bool compare_structured_comments(false);
    biosample_util::TBioSamples cache;
    for (CBioseq_CI b_iter(m_TopSeqEntry, CSeq_inst::eMol_na); b_iter; ++b_iter)
    {
        biosample_util::TBiosampleFieldDiffList diff_vec = biosample_util::GetBioseqDiffs(*b_iter, kEmptyStr, num_processed, unprocessed_ids, use_dev_server, compare_structured_comments, kEmptyStr, &cache);
        for (auto diff : diff_vec)
        {
            string seq_id = diff->GetSequenceId();
            string field = diff->GetFieldName();
            string source = diff->GetSrcVal();
            string sample = diff->GetPureSampleVal();
            string sample_id = diff->GetBioSample();

            set<string> set_source, set_sample;
            tie(set_source, set_sample) = m_DiffVec[{sample_id, field}][seq_id];
            if (!source.empty())
                set_source.insert(source);
            if (!sample.empty())
                set_sample.insert(sample);
            m_DiffVec[{sample_id, field}][seq_id] = make_pair(set_source, set_sample);
        }

        string label;
        CConstRef<CSeq_id> id(NULL);
        vector<CRef <CSeq_id> > id_list;
        ITERATE(CBioseq_Handle::TId, it, b_iter->GetId()) 
        {
            CConstRef<CSeq_id> ir = (*it).GetSeqId();
            if (ir->IsGenbank()) 
            {
                id = ir;
            }
            CRef<CSeq_id> ic(const_cast<CSeq_id *>(ir.GetPointer()));
            id_list.push_back(ic);
        }
        if (!id) 
        {
            id = FindBestChoice(id_list, CSeq_id::BestRank);
        }
        if (id) 
        {  
            id->GetLabel(&label);
            m_ids[label] = id;
        }

        if (!m_all_records)
            break;
    }
}

bool CCompareWithBiosample::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxFrame::Create( parent, id, caption, pos, size, style );
    wxVisualAttributes attr = wxPanel::GetClassDefaultAttributes();
    if (attr.colBg != wxNullColour) {
        SetOwnBackgroundColour(attr.colBg);
    }
    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    return true;
}


CCompareWithBiosample::~CCompareWithBiosample()
{
    SaveSettings();
}


/*!
 * Member initialisation
 */

void CCompareWithBiosample::Init()
{
    m_GridPanelSizer = NULL;
    m_Grid=NULL;
    m_GridPanel = NULL;
    m_source_col = 0;
    m_sample_col = 0;
    m_field_col = 0;
    m_sample_id_col = 0;
    m_modified_sample = false;
    m_modified_source = false;
    m_RefreshButton = NULL;
}


void CCompareWithBiosample::CreateControls()
{    
    CCompareWithBiosample* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_GridPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(m_GridPanelSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
   
    if (m_DiffVec.empty()) 
    {
        wxMessageBox(wxT("No differences found for resolved Biosample IDs"), wxT("Info"), wxOK);
        NCBI_THROW( CException, eUnknown, "No differences found for compare with Biosample" );
    }

    CRef<objects::CSeq_table> values_table = GetValuesTableFromSeqEntry();
    if (values_table->GetNum_rows() < 1)
    {
        wxMessageBox(wxT("No differences found for resolved Biosample IDs"), wxT("Info"), wxOK);
        NCBI_THROW( CException, eUnknown, "No differences found for compare with Biosample" );
    }
    
    AddTablePanel(values_table);
    wxArrayString itemChoiceStrings, itemChoiceStringsWritable;
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
            } 
        }
        pos++;
    }

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer16, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    
    wxButton* itemButton24 = new wxButton( itemDialog1, ID_ADD_TO_SOURCE, _("+<<"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(itemButton24, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    if (ShowToolTips())
        itemButton24->SetToolTip(_("Append to BioSource values"));

    wxButton* itemButton17 = new wxButton( itemDialog1, ID_COPY_TO_SOURCE, _("<<"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(itemButton17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    if (ShowToolTips())
        itemButton17->SetToolTip(_("Overwrite BioSource values"));

    wxButton* itemButton18 = new wxButton( itemDialog1, ID_SYNC, _("sync"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(itemButton18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    if (ShowToolTips())
        itemButton18->SetToolTip(_("Fill the blanks in either BioSample or BioSource values"));

    wxButton* itemButton19 = new wxButton( itemDialog1, ID_COPY_TO_SAMPLE, _(">>"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(itemButton19, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    if (ShowToolTips())
        itemButton19->SetToolTip(_("Overwrite BioSample values"));

    wxButton* itemButton25 = new wxButton( itemDialog1, ID_ADD_TO_SAMPLE, _(">>+"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(itemButton25, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    if (ShowToolTips())
        itemButton25->SetToolTip(_("Append to BioSample values"));

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);


    m_StringConstraintPanel = new CStringConstraintSelect( itemDialog1, m_GridPanel, itemChoiceStrings, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(m_StringConstraintPanel, 0, wxALIGN_TOP|wxALL|wxFIXED_MINSIZE, 0);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_AecrPanel = new CApplyEditconvertPanel( itemDialog1, m_GridPanel, itemChoiceStringsWritable, 0, true, wxID_ANY, wxDefaultPosition, wxSize(1128, 219));
    itemBoxSizer5->Add(m_AecrPanel, 0, wxALIGN_TOP|wxALL|wxFIXED_MINSIZE, 1);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    
    wxButton* itemButton14 = new wxButton( itemDialog1, ID_UPDATE_BIOSOURCE, _("Update BioSource"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton20 = new wxButton( itemDialog1, ID_CREATE_BIOSAMPLE, _("Create Biosample Update"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemButton20, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton21 = new wxButton( itemDialog1, ID_MAKE_REPORT, _("Make Report"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemButton21, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_RefreshButton = new wxButton(itemDialog1, ID_REFRESH_BIOSAMPLE, _("Refresh"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer13->Add(m_RefreshButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_timer.SetButton(m_RefreshButton);

    wxButton* itemButton23 = new wxButton(itemDialog1, ID_LOGOUT_BIOSAMPLE, _("Logout"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer13->Add(itemButton23, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxButton* itemButton15 = new wxButton( itemDialog1, wxID_CANCEL, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemButton15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

void CCompareWithBiosample::AddTablePanel(CRef<objects::CSeq_table> values_table)
{
    m_GridPanelSizer->Clear(true);
  
    CRef<CSeq_table> choices = GetChoices(values_table);
    int glyph_col =   GetCollapsible();
    m_GridPanel = new CSeqTableGridPanel(this, values_table, choices, glyph_col);
    m_GridPanelSizer->Add(m_GridPanel, 0, wxALIGN_TOP|wxALL, 5);
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
                if (IsReadOnlyColumn(title)) 
                    m_GridPanel->MakeColumnReadOnly(pos - 1, true);
            } 
        }
        pos++;
    }
    
    if (glyph_col >= 0 && glyph_col+1 < m_Grid->GetNumberCols())
    {
        m_GridPanel->InitColumnCollapse(glyph_col+1);  
    }
    Layout();
    Fit();
}

CRef<objects::CSeq_table> CCompareWithBiosample::GetValuesTableFromSeqEntry() 
{
    m_id_col.Reset(new objects::CSeqTable_column());
    m_id_col->SetHeader().SetField_id(objects::CSeqTable_column_info::eField_id_location_id);
    m_id_col->SetHeader().SetTitle(kSequenceIdColLabel);

    CRef<objects::CSeqTable_column> expand_col(new objects::CSeqTable_column());
    expand_col->SetHeader().SetTitle("");
    expand_col->SetHeader().SetField_name("expand");
    expand_col->SetData().SetString();

    CRef<objects::CSeqTable_column> name_col(new objects::CSeqTable_column());
    name_col->SetHeader().SetTitle("Field");
    name_col->SetHeader().SetField_name("field");
    name_col->SetData().SetString();

    CRef<objects::CSeqTable_column> source_col(new objects::CSeqTable_column());
    source_col->SetHeader().SetTitle("Source");
    source_col->SetHeader().SetField_name("source");
    source_col->SetData().SetString();

    CRef<objects::CSeqTable_column> sample_col(new objects::CSeqTable_column());
    sample_col->SetHeader().SetTitle("Sample");
    sample_col->SetHeader().SetField_name("Sample");
    sample_col->SetData().SetString();

    CRef<objects::CSeqTable_column> sample_id_col(new objects::CSeqTable_column());
    sample_id_col->SetHeader().SetTitle("Biosample Id");
    sample_id_col->SetHeader().SetField_name("biosample_id");
    sample_id_col->SetData().SetString();
    
    CRef<objects::CSeq_table> table(new objects::CSeq_table());
    table->SetColumns().push_back(m_id_col);
    table->SetColumns().push_back(expand_col);
    table->SetColumns().push_back(name_col);
    table->SetColumns().push_back(source_col);
    table->SetColumns().push_back(sample_col);
    table->SetColumns().push_back(sample_id_col);
     
    m_field_col = 1;
    m_source_col = 2;
    m_sample_col = 3;
    m_sample_id_col = 4;
    size_t num_rows = 0;
    for (const auto &diff : m_DiffVec)
    {
        string field, sample_id;
        tie(sample_id, field) = diff.first;
        for (const auto &row : diff.second)
        {
            string seq_id = row.first;
            set<string> set_source, set_sample;
            tie(set_source, set_sample) = row.second;

            string source = NStr::Join(set_source, "|");
            string sample = NStr::Join(set_sample, "|");
            
            CRef<objects::CSeq_id> id;
            if (m_ids.find(seq_id) != m_ids.end())
            {
                id.Reset(new objects::CSeq_id);
                id->Assign(*m_ids[seq_id]);
            }
            else
            {
                id.Reset(new objects::CSeq_id(seq_id, CSeq_id::fParse_Default));
            }

            m_id_col->SetData().SetId().push_back(id);
            expand_col->SetData().SetString().push_back("");
            name_col->SetData().SetString().push_back(field);
            source_col->SetData().SetString().push_back(source);
            sample_col->SetData().SetString().push_back(sample);
            sample_id_col->SetData().SetString().push_back(sample_id);
            ++num_rows;
        }
    }
      
    table->SetNum_rows(num_rows); 

    return table;
}

void CCompareWithBiosample::OnClickOk( wxCommandEvent& event )
{
    if (!m_TopSeqEntry)
        return;
    CScope &scope = m_TopSeqEntry.GetScope();
    map<CBioseq_Handle, map<string, string> > values;
    Freeze();
    int collapsed_col = m_GridPanel->GetCollapseColAndExpand();  
    size_t num_rows = m_Grid->GetNumberRows();
    for (size_t row = 0; row < num_rows; ++row)
    {
        CBioseq_Handle bsh = scope.GetBioseqHandle(*m_id_col->GetData().GetId()[row]);
        string field = m_Grid->GetCellValue(row, m_field_col).ToStdString();
        string value = m_Grid->GetCellValue(row, m_source_col).ToStdString();
        values[bsh][field] = value;
    }
    m_GridPanel->CollapseByCol(collapsed_col);   
    Thaw();
    m_Grid->Refresh();

    CRef<CCmdComposite> composite(new CCmdComposite("Update Biosource from Biosample"));

    for (map<CBioseq_Handle, map<string, string> >::iterator it = values.begin(); it != values.end(); ++it)
    {
        CSeqdesc_CI src_desc_ci(it->first, CSeqdesc::e_Source);
        CRef<CSeqdesc> new_desc(new CSeqdesc);
        new_desc->Assign(*src_desc_ci);
        CBioSource& biosource = new_desc->SetSource();

        for (map<string, string>::iterator fi = it->second.begin(); fi != it->second.end(); ++fi)
        {
            string field = fi->first;
            string value = fi->second;

            if (NStr::EqualNocase(field, "Organism Name")) 
            {
                if (!NStr::IsBlank(value))
                { 
                    biosource.SetOrg().SetTaxname(value);
                }
                else
                {
                    biosource.SetOrg().ResetTaxname();
                }
                if (biosource.GetOrg().IsSetOrgname() && biosource.GetOrg().GetOrgname().IsSetName()) 
                {
                    biosource.SetOrg().SetOrgname().ResetName();
                }
            } 
            else if (NStr::EqualNocase(field, "Tax ID")) 
            {
                int taxid = NStr::StringToInt(value, NStr::fConvErr_NoThrow);
                if (taxid > 0)
                    biosource.SetOrg().SetTaxId(taxid);
            } 
            else 
            {
                list<string> vals;
                NStr::Split(value, "|", vals, NStr::fSplit_Tokenize);
              
                bool found = false;
                try 
                {
                    COrgMod::TSubtype subtype = COrgMod::GetSubtypeValue(field);
                    biosource.RemoveOrgMod(subtype);
                    for (const auto &v : vals)
                    {
                        if (CBioSource::IsStopWord(v)) 
                            continue;
                        if (!NStr::IsBlank(v)) 
                        {
                            CRef<COrgMod> mod(new COrgMod());
                            mod->SetSubtype(subtype);
                            mod->SetSubname(v);
                            biosource.SetOrg().SetOrgname().SetMod().push_back(mod);
                        }
                    }
                    found = true;
                } catch(const CException&) 
                {
                    found = false;
                }
                catch (const exception&) 
                {
                    found = false;
                }
                if (!found)
                {
                    try 
                    {
                        CSubSource::TSubtype subtype = CSubSource::GetSubtypeValue(field);
                        biosource.RemoveSubSource(subtype);
                        for (const auto &v : vals)
                        {
                            if (CBioSource::IsStopWord(v)) 
                                continue;
                            if (!NStr::IsBlank(v)) 
                            {
                                CRef<CSubSource> sub(new CSubSource());
                                sub->SetSubtype(subtype);
                                sub->SetName(v);
                                biosource.SetSubtype().push_back(sub);
                            }
                        }
                        found = true;
                    } 
                    catch(const CException&) 
                    {
                        found = false;
                    }
                    catch (const exception&) 
                    {
                        found = false;
                    }
                }
                if (!found)
                {
                    wxMessageBox(wxT("Unknown field name ") + wxString(field), wxT("Error"), wxOK);
                    return;
                }            
            }
        }
        
        biosource.AutoFix();
        
        CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(src_desc_ci.GetSeq_entry_Handle(), *src_desc_ci, *new_desc));
        composite->AddCommand(*cmd);
    }
    
    m_CmdProcessor->Execute(composite);  
    m_modified_source = false;
}


string CCompareWithBiosample::GetErrorMessage()
{
    return "Invalid operation in Compare with Biosample";
}


/*!
 * Should we show tooltips?
 */

bool CCompareWithBiosample::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CCompareWithBiosample::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CCompareWithBiosample bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CCompareWithBiosample bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CCompareWithBiosample::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CCompareWithBiosample icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CCompareWithBiosample icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
 */



/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
 */

void CCompareWithBiosample::OnClickCancel( wxCommandEvent& event )
{    
    bool modified = m_GridPanel->GetModified() | m_AecrPanel->GetModified() | m_modified_sample || m_modified_source;
    if (modified)
    {
        wxMessageDialog dlg(this,_("Discard modifications?"), _("Attention"),wxOK|wxCANCEL|wxCENTRE);
        if (dlg.ShowModal() == wxID_OK)
        {
            Close();
        }
    }
    else
        Close();

}

void CCompareWithBiosample::GetSelectedRows(std::set<int> &selected)
{
    if (!m_Grid->IsSelection()) 
    {
        return;
    }
    wxArrayInt wx_selected = m_Grid->GetSelectedRows();
    if (wx_selected.IsEmpty())
    {
            if (m_Grid->GetSelectionBlockTopLeft().size() == 1 &&  m_Grid->GetSelectionBlockTopLeft().size() == m_Grid->GetSelectionBlockBottomRight().size())
            {
                int top_row = m_Grid->GetSelectionBlockTopLeft()[0].GetRow();
                int bot_row = m_Grid->GetSelectionBlockBottomRight()[0].GetRow();
                for (int i = top_row; i <= bot_row; i++)
                {
                    selected.insert(i);
                }
            }
        }
    else
    {
        for (unsigned int i = 0; i < wx_selected.GetCount(); i++)
        {
            selected.insert(wx_selected.Item(i));
        }
    }
}


void CCompareWithBiosample::OnSyncButtonClick( wxCommandEvent& event )
{   
    Freeze();
    int collapsed_col = m_GridPanel->GetCollapseColAndExpand();  

    std::set<int> selected;
    GetSelectedRows(selected);
    if (selected.empty())
    {
        wxMessageBox(wxT("Nothing selected"), wxT("Info"), wxOK);
        m_GridPanel->CollapseByCol(collapsed_col);
        Thaw();
        m_Grid->Refresh();
        return;
    }

    for (std::set<int>::const_iterator i = selected.begin(); i != selected.end(); ++i)
        if (m_Grid->IsRowShown(*i))
        {
            wxString source_value = m_Grid->GetCellValue(*i, m_source_col);
            wxString sample_value = m_Grid->GetCellValue(*i, m_sample_col);
            
            if (sample_value.IsEmpty() && !source_value.IsEmpty())
            {
                m_Grid->SetCellTextColour(*i, m_sample_col, wxColour(255, 160, 0));
                m_Grid->SetCellValue(*i, m_sample_col, source_value);
                m_modified_sample = true;
            }

            if (!sample_value.IsEmpty() && source_value.IsEmpty())
            {
                m_Grid->SetCellTextColour(*i, m_source_col, wxColour(255, 160, 0));
                m_Grid->SetCellValue(*i, m_source_col, sample_value);
                m_modified_source = true;
            }
        }
    
    m_GridPanel->CollapseByCol(collapsed_col);   
    Thaw();
    m_Grid->Refresh();
}

void CCompareWithBiosample::OnCopyToSourceButtonClick( wxCommandEvent& event )
{   
    Freeze();
    int collapsed_col = m_GridPanel->GetCollapseColAndExpand();  

    std::set<int> selected;
    GetSelectedRows(selected);
    if (selected.empty())
    {
        wxMessageBox(wxT("Nothing selected"), wxT("Info"), wxOK);
        m_GridPanel->CollapseByCol(collapsed_col);
        Thaw();
        m_Grid->Refresh();
        return;
    }

    for (std::set<int>::const_iterator i = selected.begin(); i != selected.end(); ++i)
        if (m_Grid->IsRowShown(*i))
        {
            wxString source_value = m_Grid->GetCellValue(*i, m_source_col);
            wxString sample_value = m_Grid->GetCellValue(*i, m_sample_col);

            if (!sample_value.IsEmpty() && source_value != sample_value)
            {
                m_Grid->SetCellTextColour(*i, m_source_col, wxColour(255, 160, 0));
                m_Grid->SetCellValue(*i, m_source_col, sample_value);
                m_modified_source = true;
            }
        }
    
    m_GridPanel->CollapseByCol(collapsed_col);   
    Thaw();
    m_Grid->Refresh();
}

void CCompareWithBiosample::OnCopyToSampleButtonClick( wxCommandEvent& event )
{   
    Freeze();
    int collapsed_col = m_GridPanel->GetCollapseColAndExpand();  

    std::set<int> selected;
    GetSelectedRows(selected);
    if (selected.empty())
    {
        wxMessageBox(wxT("Nothing selected"), wxT("Info"), wxOK);
        m_GridPanel->CollapseByCol(collapsed_col);
        Thaw();
        m_Grid->Refresh();
        return;
    }

    for (std::set<int>::const_iterator i = selected.begin(); i != selected.end(); ++i)
        if (m_Grid->IsRowShown(*i))
        {
            wxString source_value = m_Grid->GetCellValue(*i, m_source_col);
            wxString sample_value = m_Grid->GetCellValue(*i, m_sample_col);
            
            if (sample_value != source_value && !source_value.IsEmpty())
            {
                m_Grid->SetCellTextColour(*i, m_sample_col, wxColour(255, 160, 0));
                m_Grid->SetCellValue(*i, m_sample_col, source_value);
                m_modified_sample = true;
            }
           
        }
    
    m_GridPanel->CollapseByCol(collapsed_col);   
    Thaw();
    m_Grid->Refresh();
}

void CCompareWithBiosample::OnAddToSource( wxCommandEvent& event )
{   
    Freeze();
    int collapsed_col = m_GridPanel->GetCollapseColAndExpand();  

    std::set<int> selected;
    GetSelectedRows(selected);
    if (selected.empty())
    {
        wxMessageBox(wxT("Nothing selected"), wxT("Info"), wxOK);
        m_GridPanel->CollapseByCol(collapsed_col);
        Thaw();
        m_Grid->Refresh();
        return;
    }

    for (std::set<int>::const_iterator i = selected.begin(); i != selected.end(); ++i)
        if (m_Grid->IsRowShown(*i))
        {
            wxString source_value = m_Grid->GetCellValue(*i, m_source_col);
            wxString sample_value = m_Grid->GetCellValue(*i, m_sample_col);
            wxString field = m_Grid->GetCellValue(*i, m_field_col);

            if (!sample_value.IsEmpty() && source_value != sample_value && field != _("Organism Name") && field != _("Tax ID"))
            {
                list<string> old_vals;
                NStr::Split(source_value.ToStdString(), "|", old_vals, NStr::fSplit_Tokenize);
                list<string> new_vals;
                NStr::Split(sample_value.ToStdString(), "|", new_vals, NStr::fSplit_Tokenize);
                set<string> vals(old_vals.begin(), old_vals.end());
                vals.insert(new_vals.begin(), new_vals.end());
                string new_val = NStr::Join(vals, "|");
                if (new_val != source_value)
                {
                    m_Grid->SetCellTextColour(*i, m_source_col, wxColour(255, 160, 0));
                    m_Grid->SetCellValue(*i, m_source_col, new_val);
                    m_modified_source = true;
                }
            }
        }
    
    m_GridPanel->CollapseByCol(collapsed_col);   
    Thaw();
    m_Grid->Refresh();
}

void CCompareWithBiosample::OnAddToSample( wxCommandEvent& event )
{   
    Freeze();
    int collapsed_col = m_GridPanel->GetCollapseColAndExpand();  

    std::set<int> selected;
    GetSelectedRows(selected);
    if (selected.empty())
    {
        wxMessageBox(wxT("Nothing selected"), wxT("Info"), wxOK);
        m_GridPanel->CollapseByCol(collapsed_col);
        Thaw();
        m_Grid->Refresh();
        return;
    }

    for (std::set<int>::const_iterator i = selected.begin(); i != selected.end(); ++i)
        if (m_Grid->IsRowShown(*i))
        {
            wxString source_value = m_Grid->GetCellValue(*i, m_source_col);
            wxString sample_value = m_Grid->GetCellValue(*i, m_sample_col);
            wxString field = m_Grid->GetCellValue(*i, m_field_col);

            if (sample_value != source_value && !source_value.IsEmpty() && field != _("Organism Name") && field != _("Tax ID"))
            {
                list<string> old_vals;
                NStr::Split(sample_value.ToStdString(), "|", old_vals, NStr::fSplit_Tokenize);
                list<string> new_vals;
                NStr::Split(source_value.ToStdString(), "|", new_vals, NStr::fSplit_Tokenize);
                set<string> vals(old_vals.begin(), old_vals.end());
                vals.insert(new_vals.begin(), new_vals.end());
                string new_val = NStr::Join(vals, "|");
                if (new_val != sample_value)
                {
                    m_Grid->SetCellTextColour(*i, m_sample_col, wxColour(255, 160, 0));
                    m_Grid->SetCellValue(*i, m_sample_col, new_val);
                    m_modified_sample = true;
                }
            }
           
        }
    
    m_GridPanel->CollapseByCol(collapsed_col);   
    Thaw();
    m_Grid->Refresh();
}

void CCompareWithBiosample::MakeReport(wxCommandEvent& event)
{
    wxString msg;
    msg << "Seq-id\tField\tSource\tSample\tBiosample Id\n";
    for (const auto &diff : m_DiffVec)
    {
        string field, sample_id;
        tie(sample_id, field) = diff.first;
        for (const auto &row : diff.second)
        {
            string seq_id = row.first;
            set<string> set_source, set_sample;
            tie(set_source, set_sample) = row.second;

            string source = NStr::Join(set_source, "|");
            string sample = NStr::Join(set_sample, "|");
            
            msg << seq_id <<"\t"<<field<<"\t"<<source<<"\t"<<sample<<"\t"<<sample_id<<"\n";
        }
    }

    CGenericReportDlg* report = new CGenericReportDlg(NULL);  
    report->SetTitle(wxT("Compare with Biosample Report")); 
    report->SetText(msg);
    report->Show(true);
}

bool CCompareWithBiosample::Authenticate()
{
    m_cookie.Reset();  
    CLoginDialog dlg(this);
    if (dlg.ShowModal() != wxID_OK)
        return false;

    string username = dlg.GetUsername();
    string password = dlg.GetPassword();
      // MyNCBI signin
    CHttpSession session;
    string sUrl = "https://www.ncbi.nlm.nih.gov/portal/signin.cgi?js";
    CHttpRequest request = session.NewRequest(sUrl, CHttpSession::ePost);
    request.SetRetries(0);
    int response_timeout = CGuiRegistry::GetInstance().GetInt("GBENCH.System.SeqConfigTimeout", 40);
    request.SetDeadline(CTimeout(response_timeout));

    CHttpFormData& data = request.FormData();
    data.AddEntry("cmd", "signin");
    data.AddEntry("surl", "dummy");
    data.AddEntry("furl", "dummy");
    data.AddEntry("rrme", "1");
    data.AddEntry("uname", username);
    data.AddEntry("upasswd", password);

    CHttpResponse response = request.Execute(); // get authentication cookie

    if (response.GetStatusCode() != 200)
    {
        wxMessageBox(wxT("Unable to login to MyNCBI"), wxT("Error"), wxOK);
        return false;
    }

   // Getting cookies - need WebCubbyUser
    ITERATE(CHttpCookies, it, session.Cookies()) 
    {
        if ( it->GetName() == "WebCubbyUser")
        {
            m_cookie = *it;
            break;
        }
    }
    if (!TestCookie())
    {
        wxMessageBox(wxT("Unable to login to MyNCBI"), wxT("Error"), wxOK);
        return false;
    }

    return true;
}

bool CCompareWithBiosample::TestCookie()
{
    if (!m_cookie.Validate() || m_cookie.IsExpired())
        return false;

    CHttpSession session;
    session.Cookies().Add(m_cookie);
    int response_timeout = CGuiRegistry::GetInstance().GetInt("GBENCH.System.SeqConfigTimeout", 40);
    CHttpResponse response = session.Get(CUrl("http://portlet.ncbi.nlm.nih.gov/ptools/emyncbi.cgi?cmd=whoami"), CTimeout(response_timeout), 0);
    if (response.GetStatusCode() != 200)
        return false;
    xml::error_messages errors;
    xml::document doc(response.ContentStream(), &errors);

    xml::node_set nodes ( doc.get_root_node().run_xpath_query("//eMyNCBIResult") );
    ITERATE(xml::node_set, it, nodes)    
    {
        xml::node::const_iterator it2 = it->find("UserId");
        if (it2 != it->end())
            return true;
    }
    return false;
}

static const char* kRegPath = "Dialogs.Edit.CompareWithBiosample";
static const char* kCookie = "Cookie";

void CCompareWithBiosample::SaveSettings() const
{
    if (!m_cookie.Validate() || m_cookie.IsExpired())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(kRegPath);

    view.Set(kCookie, m_cookie.AsString(CHttpCookie::eHTTPResponse));
}


void CCompareWithBiosample::LoadSettings()
{
    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(kRegPath);

    string cookie = view.GetString(kCookie, kEmptyStr);
    if (cookie.empty())
        return;
    m_cookie.Parse(cookie);
}

static const int chunk_size = 100;

void CCompareWithBiosample::CreateBiosampleUpdateWebService(wxCommandEvent& event)
{
    if (!TestCookie())
    {
        bool auth = Authenticate();
        if (!auth)
            return;
    }
    
    wxString msg;
    {
        wxBusyCursor wait;
        map<pair<string, string>, list<string> >  new_values;
        {
            Freeze();
            int collapsed_col = m_GridPanel->GetCollapseColAndExpand();
            size_t num_rows = m_Grid->GetNumberRows();
            for (size_t row = 0; row < num_rows; ++row)
            {
                string seq_id_str = m_id_col->GetData().GetId()[row]->GetSeqIdString();
                string field = m_Grid->GetCellValue(row, m_field_col).ToStdString();
                string sample = m_Grid->GetCellValue(row, m_sample_col).ToStdString();
                string sample_id = m_Grid->GetCellValue(row, m_sample_id_col).ToStdString();
                list<string> sample_list;
                NStr::Split(sample, "|", sample_list, NStr::fSplit_Tokenize);
                sample_list.sort();
                new_values[pair<string, string>(sample_id, field)] = sample_list;
            }
            m_GridPanel->CollapseByCol(collapsed_col);
            Thaw();
            m_Grid->Refresh();
        }

        unordered_map<string, map<string, vector<string> > > delete_value, add_value;
        set<string> ids;
        unordered_map<string, string> change_organism;
        for (const auto &diff : m_DiffVec)
        {
            string field, sample_id;
            tie(sample_id, field) = diff.first;
            list<string> new_sample;
            if (new_values.find(diff.first) != new_values.end())
                new_sample = new_values[diff.first];

            set<string> set_source, old_sample;
            tie(set_source, old_sample) = diff.second.begin()->second;


            NStr::ReplaceInPlace(field, "-", "_");

            vector<string> deleted_values;
            set_difference(old_sample.begin(), old_sample.end(), new_sample.begin(), new_sample.end(), inserter(deleted_values, deleted_values.end()));

            vector<string> added_values;
            set_difference(new_sample.begin(), new_sample.end(), old_sample.begin(), old_sample.end(), inserter(added_values, added_values.end()));
           
            if (field == "Organism Name")
            {
                if (added_values.size() == 1 && deleted_values.size() == 1)
                {
                    change_organism[sample_id] = added_values.front();
                    ids.insert(sample_id);
                }
                continue;
            }
            if (!deleted_values.empty())
            {
                delete_value[sample_id][field] = deleted_values;
                ids.insert(sample_id);
            }
           
            if (!added_values.empty())
            {
                add_value[sample_id][field] = added_values;
                ids.insert(sample_id);
            }            
        }
        for (const auto &diff : new_values)
        {
            string field, sample_id;
            tie(sample_id, field) = diff.first;
            vector<string> new_sample(diff.second.begin(), diff.second.end());

            if (m_DiffVec.find(diff.first) != m_DiffVec.end())
            {
                continue;
            }

            if (new_sample.empty())
            {
                continue;
            }

            NStr::ReplaceInPlace(field, "-", "_");
            add_value[sample_id][field] = new_sample;
            ids.insert(sample_id);
        }
        int response_timeout = CGuiRegistry::GetInstance().GetInt("GBENCH.System.SeqConfigTimeout", 40);
        //string sUrl = "https://dev-api-int.ncbi.nlm.nih.gov/biosample/update/"; // DEBUG
        string sUrl = "https://api-int.ncbi.nlm.nih.gov/biosample/update/";
        string sContentType = "application/json; charset=utf-8";
      
        auto sample_id_it = ids.begin();
        while (sample_id_it != ids.end())
        {
            int count = 0;
            CJson_Document req;
            CJson_Object top_obj = req.SetObject();
            CJson_Array biosample_array = top_obj.insert_array("update");
            for (; sample_id_it != ids.end() && count < chunk_size; ++sample_id_it)
            {
                auto sample_id = *sample_id_it;
                CJson_Object obj1 = biosample_array.push_back_object();
                obj1.insert("samples", sample_id);
                if (!delete_value[sample_id].empty())
                {
                    CJson_Array del_arr = obj1.insert_object("delete").insert_array("attribute");
                    for (const auto &kv : delete_value[sample_id])
                    {
                        const string &field = kv.first;
                        for (const auto &v : kv.second)
                        {
                            CJson_Object obj2 = del_arr.push_back_object();
                            obj2.insert("name", field);
                            obj2.insert("old_value", v);
                            count++;
                        }
                    }
                }
                if ( !change_organism[sample_id].empty())
                {
                    CJson_Object chg_obj = obj1.insert_object("change");
                    CJson_Object chg_org = chg_obj.insert_object("organism");
                    chg_org.insert("new_value", change_organism[sample_id]);
                    count++;
                }
                if (!add_value[sample_id].empty())
                {
                    CJson_Array add_arr = obj1.insert_object("add").insert_array("attribute");
                    for (const auto &kv : add_value[sample_id])
                    {
                        const string &field = kv.first;
                        for (const auto &v : kv.second)
                        {
                            CJson_Object obj2 = add_arr.push_back_object();
                            obj2.insert("name", field);
                            obj2.insert("new_value", v);
                            count++;
                        }
                    }
                }
            }
            top_obj.insert_object("options").insert("attribute_synonyms", true);

            string sData = req.ToString();
            //cout << "REQUEST" << endl;
            //cout << sData << endl;
            CHttpSession session;
            session.Cookies().Add(m_cookie);
            CHttpResponse response = session.Post(sUrl, sData, sContentType, CTimeout(response_timeout), 0); // send biosample request
            if (response.GetStatusCode() != 200)
            {
                stringstream ss;
                NcbiStreamCopy(ss, response.ErrorStream());
                string err = ss.str();
                if (err.empty())
                    err = "Unable to connect - server down?";
                wxMessageBox(wxString(err), wxT("Error"), wxOK);
                break;
            }
            stringstream ss;
            NcbiStreamCopy(ss, response.ContentStream());
            msg << ReportBiosampleUpdate(ss.str());
        }
    }
    m_modified_sample = false;
    if (!msg.IsEmpty())
    {
	CGenericReportDlg *report = new CGenericReportDlg(this);  
	report->SetTitle(wxT("Biosample Update Report")); 
        report->SetText(msg);
        report->Show(true);
	report->Raise();
        wxYieldIfNeeded();
    }
    else
    {
	wxWindow *win = FindWindowByLabel(wxT("Biosample Update Report"), this);
	if (win)
	{
	    CGenericReportDlg *report = dynamic_cast<CGenericReportDlg *>(win);
	    if (report)
		report->Close();
	}
    }
    ReloadTable();

}

void CCompareWithBiosample::OnLogout(wxCommandEvent& event)
{
    if (m_cookie.Validate() && !m_cookie.IsExpired())
    {
	CHttpSession session;
	session.Cookies().Add(m_cookie);
	int response_timeout = CGuiRegistry::GetInstance().GetInt("GBENCH.System.SeqConfigTimeout", 40);
	session.Get(CUrl("https://www.ncbi.nlm.nih.gov/account/signout/"), CTimeout(response_timeout), 0);
    }
    m_cookie.Reset();
}

void CCompareWithBiosample::ReloadTable()
{
/*    int collapsed_col = m_GridPanel->GetCollapseColAndExpand();
    vector<pair<wxString, wxColour> >  redo_values;
    for (int row = 0; row < m_Grid->GetNumberRows(); row++)
        if (!m_Grid->IsReadOnly(row, m_source_col) && m_Grid->IsRowShown(row))
        {
            redo_values.push_back(make_pair(m_Grid->GetCellValue(row, m_source_col), m_Grid->GetCellTextColour(row, m_source_col)));
        }
*/
    
     
    LoadData();
    
    if (m_DiffVec.empty()) 
    {
        wxMessageBox(wxT("No differences found for resolved Biosample IDs"), wxT("Info"), wxOK);
        Close();
        return;
    }
    CRef<objects::CSeq_table> values_table = GetValuesTableFromSeqEntry();
    if (values_table->GetNum_rows() < 1)
    {
        wxMessageBox(wxT("No differences found for resolved Biosample IDs"), wxT("Info"), wxOK);
        Close();
        return;
    }
    AddTablePanel(values_table);
/*
    for (int row = 0; row < redo_values.size(); row++)
    {
        wxString val = redo_values[row].first;
        wxColour colour = redo_values[row].second;
        m_Grid->SetCellTextColour(row, m_source_col, colour);
        m_Grid->SetCellValue(row, m_source_col, val);
    }
    m_GridPanel->CollapseByCol(collapsed_col);
*/
}

void CCompareWithBiosample::OnRefresh(wxCommandEvent& event)
{
    wxWindow *win = FindWindowByLabel(wxT("Biosample Update Report"), this);
    if (win)
    {
	CGenericReportDlg *report = dynamic_cast<CGenericReportDlg *>(win);
	if (report)
	    report->Close();
    }

    if (m_timer.IsRunning())
	m_timer.Stop();
    m_RefreshButton->Disable();
    ReloadTable();
    if (!m_timer.IsRunning())
	m_timer.Start(3000);
}

wxString CCompareWithBiosample::ReportBiosampleUpdate(const string &str)
{
    //cout << "REPLY" <<endl;
    //cout << str << endl;
    wxString report;
    CJson_Document doc;
    doc.ParseString(str);
    
    if (!doc.ReadSucceeded() || !doc.IsObject())
        return report;
    
    CJson_ConstObject obj = doc.GetObject();
    
    CJson_ConstObject::const_iterator it = obj.find("messages");
    if (it == obj.end() || !it->value.IsArray() || it->value.GetArray().empty())
        return report;

    unordered_map<string, vector<string> > messages;
    for ( CJson_ConstArray::const_iterator it2 = it->value.GetArray().begin(); it2 != it->value.GetArray().end(); ++it2)
    {
        CJson_ConstObject obj2 = it2->GetObject();
	if (obj2.find("level") != obj2.end() && obj2.find("message") != obj2.end())
	{
	    string level = obj2["level"].GetValue().GetString();
	    string msg = obj2["message"].GetValue().GetString();      
	    messages[level].push_back(msg);
	}
    }
    

    it = obj.find("validation_report");
    if (it != obj.end() && it->value.IsObject() && !it->value.GetObject().empty())
    {
	map<string, map<string, string> > validation_error, validation_warning;
	for ( CJson_ConstObject::const_iterator it2 = it->value.GetObject().begin(); it2 != it->value.GetObject().end(); ++it2)
	{
	    string name(it2->name);
	    for ( CJson_ConstArray::const_iterator it3 = it2->value.GetArray().begin(); it3 != it2->value.GetArray().end(); ++it3)
	    {
		CJson_ConstObject obj2 = it3->GetObject();
		if (obj2.find("message") == obj2.end())
		{
		    continue;
		}
	
		string validation_msg;
		if (obj2.find("type") != obj2.end())
		{
		    string type = obj2["type"].GetValue().GetString();      
		    if (type == "attribute_error" && obj2.find("attribute_name") != obj2.end())
		    {
			string attribute_name = obj2["attribute_name"].GetValue().GetString();
			validation_msg += " " + attribute_name;	    
			if (obj2.find("attribute_value") != obj2.end())
			{
			    string attribute_value = obj2["attribute_value"].GetValue().GetString();
			    if (!attribute_value.empty())
				validation_msg += ":" + attribute_value;
			}
		    }
		}

		string msg = obj2["message"].GetValue().GetString();      
		bool is_warning = false;
		if (obj2.find("is_warning") != obj2.end())
		    is_warning = obj2["is_warning"].GetValue().GetBool();	
		if (obj2.find("name") != obj2.end())
		{
		    string short_name = obj2["name"].GetValue().GetString();
		    msg += " (" + short_name + ")";
		    if (short_name == "taxonomy_error_warning" || short_name == "tax_consult_warning" || short_name == "taxonomy_service_failure_warning")
		    {
			wxMessageBox(wxString(msg), wxT("Taxonomy Consult Warning"), wxOK);
		    }
		}
		if (is_warning)
		    validation_warning[msg][name] = validation_msg;
		else
		    validation_error[msg][name] = validation_msg;
	    }
	}
	for (const auto &m : validation_error)
	{
	    messages["error"].push_back(m.first);
	    for (const auto &name : m.second)
	    {
		string str = name.first;
		if (!name.second.empty())
		    str += " - " + name.second;
		messages["error"].push_back(str);
	    }
	}
	for (const auto &m : validation_warning)
	{
	    messages["warning"].push_back(m.first);
	    for (const auto &name : m.second)
	    {
		string str = name.first;
		if (!name.second.empty())
		    str += " - " + name.second;
		messages["warning"].push_back(str);
	    }
	}
    }

    if (messages.empty())
        return report;

    if (!messages["error"].empty())
    {
        report << "Error:\n"; 
        for (const auto &m : messages["error"])
            report << m << "\n";
        report << "\n";
    }
    if (!messages["warning"].empty())
    {
        report << "Warning:\n"; 
        for (const auto &m : messages["warning"])
            report << m << "\n";
        report << "\n";
    }
    if (!messages["info"].empty())
    {
        report << "Info:\n"; 
        for (const auto &m : messages["info"])
            report << m << "\n";
        report << "\n";
    }
    return report;
}

/*
 * CLoginDialog type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CLoginDialog, wxDialog )


/*
 * CLoginDialog event table definition
 */

BEGIN_EVENT_TABLE( CLoginDialog, wxDialog )

////@begin CLoginDialog event table entries
////@end CLoginDialog event table entries

END_EVENT_TABLE()


/*
 * CLoginDialog constructors
 */

CLoginDialog::CLoginDialog()
{
    Init();
}

CLoginDialog::CLoginDialog( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*
 * CLoginDialog creator
 */

bool CLoginDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CLoginDialog creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CLoginDialog creation
    return true;
}


/*
 * CLoginDialog destructor
 */

CLoginDialog::~CLoginDialog()
{
////@begin CLoginDialog destruction
////@end CLoginDialog destruction
}


/*
 * Member initialisation
 */

void CLoginDialog::Init()
{
////@begin CLoginDialog member initialisation
    m_Username = NULL;
    m_Password = NULL;
////@end CLoginDialog member initialisation
}


/*
 * Control creation for CLoginDialog
 */

void CLoginDialog::CreateControls()
{    
////@begin CLoginDialog content construction
    CLoginDialog* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Username"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Username = new wxTextCtrl( itemDialog1, ID_USERNAMETEXTCTRL, wxString(GetProcessUserName()), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(m_Username, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("Password"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemStaticText8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Password = new wxTextCtrl( itemDialog1, ID_PASSWORDTEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemBoxSizer7->Add(m_Password, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton11 = new wxButton( itemDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemButton11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton12 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemButton12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CLoginDialog content construction
}


/*
 * Should we show tooltips?
 */

bool CLoginDialog::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CLoginDialog::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CLoginDialog bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CLoginDialog bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CLoginDialog::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CLoginDialog icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CLoginDialog icon retrieval
}

//TODO: deal with fields which can appear multiple times within the same bioseq.

END_NCBI_SCOPE
