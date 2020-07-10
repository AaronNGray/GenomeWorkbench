/*  $Id: user_object_util.cpp 45095 2020-05-29 13:13:36Z bollin $
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
 * Authors:  Colleen Bollin
 */

#include <ncbi_pch.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqtable/Seq_table.hpp>
#include <objects/seqtable/SeqTable_column.hpp>
#include <objects/seqtable/SeqTable_column_info.hpp>
#include <objects/seqtable/SeqTable_multi_data.hpp>
#include <gui/widgets/edit/user_object_util.hpp>
#include <gui/widgets/edit/single_ref_gene_track_loc_panel.hpp>
#include <gui/widgets/edit/single_assembly_tracking_panel.hpp>
#include <gui/widgets/wx/csv_exporter.hpp>
#include <gui/widgets/loaders/table_import_data_source.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CSingleUserFieldPanel::x_OnChange()
{
    wxWindow* parent = this->GetParent();

    CUserFieldListPanel * listpanel = dynamic_cast<CUserFieldListPanel *>(parent);

    while (parent && !listpanel) {
        parent = parent->GetParent();
        listpanel = dynamic_cast<CUserFieldListPanel*>(parent);
    }

    if (!listpanel) {
        return;
    }
    listpanel->AddLastField((wxWindow*)this);
}


CSingleUserFieldPanel * CRefGeneTrackFieldManager::MakeEditor(wxWindow* parent, objects::CUser_field& field)
{
    return new CSingleRefGeneTrackLocPanel(parent, field);
}


bool CRefGeneTrackFieldManager::x_IsTypeString(const string& str)
{
    if (NStr::EqualNocase(str, "Assembly") 
        || NStr::EqualNocase(str, "IdenticalTo")) {
        return true;
    } else {
        return false;
    }
}


bool CRefGeneTrackFieldManager::IsEligible(const objects::CUser_field& field)
{
    if (field.IsSetLabel() && (field.GetLabel().IsStr() && x_IsTypeString(field.GetLabel().GetStr()))) {
        return true;
    } else {
        return false;
    }
}


bool CRefGeneTrackFieldManager::x_HasPopulatedFields(const objects::CUser_field& field)
{
    if (!field.IsSetData()) {
        return false;
    } 
    if (field.GetData().IsFields()) {
        for (auto& it : field.GetData().GetFields()) {
            if (it->IsSetData() && it->GetData().IsStr() &&
                !NStr::IsBlank(it->GetData().GetStr())) {
                return true;
            }
        }
    } else if (field.GetData().IsStr() && !NStr::IsBlank(field.GetData().GetStr())) {
        return true;
    }
    return false;
}


bool CRefGeneTrackFieldManager::IsEmpty(const objects::CUser_field& field)
{
    if (!field.IsSetData() || !field.GetData().IsFields()) {
        return true;
    }
    for (auto& it : field.GetData().GetFields()) {
        if (x_HasPopulatedFields(*it)) {
            return false;
        }
    }
    return true;
}


CRef<CUser_field> CRefGeneTrackFieldManager::GetUserField(wxWindow* ctrl)
{
    CSingleRefGeneTrackLocPanel* single_panel = dynamic_cast<CSingleRefGeneTrackLocPanel*> (ctrl);
    if (single_panel) {
        return single_panel->GetUser_field();
    } else {
        CRef<CUser_field> empty(NULL);
        return empty;
    }
}


CSingleUserFieldPanel * CAssemblyTrackFieldManager::MakeEditor(wxWindow* parent, objects::CUser_field& field)
{
    return new CSingleAssemblyTrackingPanel(parent, field);
}


bool CAssemblyTrackFieldManager::IsEligible(const objects::CUser_field& field)
{
    if (field.IsSetData() && field.GetData().IsFields()) {
        return true;
    } else {
        return false;
    }
}


bool CAssemblyTrackFieldManager::IsEmpty(const objects::CUser_field& field)
{
    if (field.IsSetData() && field.GetData().IsFields() && !field.GetData().GetFields().empty()) {
        return false;
    } else {
        return true;
    }
}


CRef<CUser_field> CAssemblyTrackFieldManager::GetUserField(wxWindow* ctrl)
{
    CSingleAssemblyTrackingPanel* single_panel = dynamic_cast<CSingleAssemblyTrackingPanel*> (ctrl);
    if (single_panel) {
        return single_panel->GetUser_field();
    } else {
        CRef<CUser_field> empty(NULL);
        return empty;
    }
}


void CUserFieldListPanel::SetUser_object(CRef<objects::CUser_object> user)
{
    if (user) {
        m_UserObj->Assign(*user);
    } else {
        m_UserObj = new CUser_object();
    }
    TransferDataToWindow();
}


void CUserFieldListPanel::PopulateFields(CUser_object& user_obj)
{  
    CUser_object::TData::iterator it = user_obj.SetData().begin();
    while (it != user_obj.SetData().end()) {
        if (m_FieldManager->IsEligible(**it)) {
            it = user_obj.SetData().erase(it);
        } else {
            ++it;
        }
    }

    for (CUser_object::TData::iterator x = m_Fields.begin();
         x != m_Fields.end();
         x++) {
        if (!m_FieldManager->IsEmpty(**x)) {
            CRef<CUser_field> new_field(new CUser_field());
            new_field->Assign(**x);
            user_obj.SetData().push_back(new_field);
        }
    }
}


bool CUserFieldListPanel::x_IsTPA() const
{
    if (m_UserObj && m_UserObj->IsSetType() && m_UserObj->GetType().IsStr() &&
        NStr::EqualNocase(m_UserObj->GetType().GetStr(), "TpaAssembly")) {
        return true;
    } else {
        return false;
    }

}


bool CUserFieldListPanel::TransferDataToWindow()
{
    Freeze();

    // remove existing subitems
    while (m_Sizer->GetItemCount() > 0) {
        size_t pos = 0;
        m_Sizer->GetItem(pos)->DeleteWindows();
        m_Sizer->Remove(pos);
    }

    m_TotalHeight = 0;
    m_TotalWidth = 0;
    m_NumRows = 0;
    m_ScrollRate = 0;

    x_UpdateFields();
    vector< CRef< CUser_field > >::iterator it = m_Fields.begin();
    vector<wxWindow *> new_rows;
    bool is_tpa = x_IsTPA();
    while (it != m_Fields.end()) {
        if (!is_tpa && (*it)->IsSetData() && (*it)->GetData().IsFields()) {
            for (auto& fit : (*it)->GetData().GetFields()) {
                new_rows.push_back(x_AddRow(fit, m_ScrolledWindow, m_Sizer));
            }
        } else {
            new_rows.push_back(x_AddRow(*it, m_ScrolledWindow, m_Sizer));
        }
        ++it;
    }
    if (new_rows.size() == 0 || m_NeedsEmptyLastRow) {
        CRef<CUser_field> last_field(new CUser_field());
        x_AddRow(last_field, m_ScrolledWindow, m_Sizer);
    }
    m_ScrolledWindow->SetVirtualSize(m_TotalWidth + 10, m_TotalHeight);
    m_ScrolledWindow->SetScrollRate(0, m_ScrollRate);
    m_ScrolledWindow->FitInside();
    Layout();
    Thaw();
    ITERATE(vector<wxWindow *>, it, new_rows) {
        (*it)->TransferDataToWindow();
    }

    if (!wxPanel::TransferDataToWindow())
        return false;
    return true;
}


bool CUserFieldListPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    m_Fields.clear();

    wxSizerItemList::iterator node = m_Sizer->GetChildren().begin();
    for (; node != m_Sizer->GetChildren().end(); ++node) {
        wxWindow *w = (*node)->GetWindow();
        if (!w) {
            continue;
        }
        w->TransferDataFromWindow();
        CRef<CUser_field> edited_field = m_FieldManager->GetUserField(w);
        if (edited_field) {
            m_Fields.push_back(edited_field);
        }
    }
    return true;
}


void CUserFieldListPanel::AddLastField (wxWindow* link)
{
    if (!m_NeedsEmptyLastRow) {
        return;
    }
    // find the row that holds the author to be deleted
    wxSizerItemList& itemList = m_Sizer->GetChildren();

    if (IsLastRow(link, itemList)) {
        link->TransferDataFromWindow();
        CRef<CUser_field> curr_field = m_FieldManager->GetUserField(link);
        if (curr_field && !m_FieldManager->IsEmpty(*curr_field)) {
            CRef<CUser_field> last_field(new CUser_field());
            m_Fields.push_back(last_field);
            x_AddRow (last_field, m_ScrolledWindow, m_Sizer);
            m_ScrolledWindow->SetVirtualSize(m_TotalWidth + 10, m_TotalHeight);
            m_ScrolledWindow->SetScrollRate(0, m_ScrollRate);
            m_ScrolledWindow->FitInside();
        }
    }
}


wxWindow* CUserFieldListPanel::x_AddRow(CRef<CUser_field> field, wxScrolledWindow* scrolled_window, wxBoxSizer* sizer)
{
    wxWindow* row = m_FieldManager->MakeEditor(scrolled_window, *field);
    sizer->Add(row, 0, wxALIGN_LEFT|wxALL, 0);
    int row_width;
    int row_height;
    row->GetSize(&row_width, &row_height);
    if (row_width > m_TotalWidth) {
        m_TotalWidth = row_width;
    }
    m_TotalHeight += row_height;
    m_ScrollRate = row_height;
    m_NumRows++;
    return row;
}


void CUserFieldListPanel::x_UpdateFields()
{
    //m_Object = dynamic_cast<CSeq_feat*>(&object);
    m_Fields.clear();
    if (m_UserObj->IsSetData()) {
        ITERATE(CUser_object::TData, it, m_UserObj->GetData()) {
            if (m_FieldManager->IsEligible(**it)) {
                CRef< CUser_field > field(new CUser_field());
                field->Assign(**it);
                m_Fields.push_back(field);
            }
        }
    }
}


bool IsLastRow(wxWindow* wnd, wxSizerItemList& itemList)
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
                is_last = false;
                return is_last;
            }
        }
    }

    return is_last;
}


CRef<CSeq_table> ReadTabDelimitedTable(const wxString& filename, bool read_header)
{
    CTableImportDataSource ds;
    if (!ds.LoadTable(filename)) {
        CRef<CSeq_table> empty(NULL);
        return empty;
    }
    ds.SetTableType(CTableImportDataSource::eDelimitedTable);
    if (read_header) {
        ds.SetColumnHeaderRow(0);
        ds.SetFirstImportRow(1);
    }
    
    // All delimiter options can be updated through the CTableDelimiterRules
    // class.  The separators part of that is a vector of 'char'.
    vector<char> delims;
    delims.push_back('\t');
    ds.GetDelimiterRules().SetDelimiters(delims);

    // Recompute the table (break it up into fields) using the new
    // delimiters and header row specs
    ds.RecomputeFields(true);

    // Set column information which will be used to set the seq-table
    // column properties
    for (size_t i=0; i<ds.GetColumns().size(); ++i) {
        // set names for columns if no header
        if (!read_header) {
            ds.GetColumns()[i].SetName("This is column: " + NStr::Int8ToString(i));
        }
        ds.GetColumns()[i].SetType(CTableImportColumn::eTextColumn); // skipped is a type too
        ds.GetColumns()[i].SetDataType(CTableImportColumn::eUnspecifiedText);
    }

    // Convert table to a seq-table (inside of a seq-annot)
    CRef<CSeq_annot> annot_container;
    annot_container.Reset(new CSeq_annot());
    ds.ConvertToSeqAnnot(annot_container);

    CRef<CSeq_table> table(new CSeq_table());
    table->Assign(annot_container->GetData().GetSeq_table());
    return table;
}


bool SaveTableFile (const string& save_file_name, CRef<objects::CSeq_table> values_table, bool include_header)
{
    ios::openmode mode = ios::out;

    CNcbiOfstream os(save_file_name.c_str(), mode); 
    if (!os)
    {
        return false; 
    }
        
    CCSVExporter exporter(os, '\t', '"', false);
    if (include_header) {
        ITERATE (objects::CSeq_table::TColumns, cit, values_table->GetColumns()) {
            exporter.Field((*cit)->GetHeader().GetTitle());
        }
        exporter.NewRow();
    }
    for (int i = 0; i < values_table->GetNum_rows(); ++i) {
        ITERATE (objects::CSeq_table::TColumns, cit, values_table->GetColumns()) {
            if (i < (*cit)->GetData().GetSize()) {
                if ((*cit)->GetData().IsId()) {
                    string label = "";
                    (*cit)->GetData().GetId()[i]->GetLabel(&label, objects::CSeq_id::eContent);
                    exporter.Field(label);
                } else if ((*cit)->GetData().IsString()) {
                    exporter.Field((*cit)->GetData().GetString()[i]);
                } else if ((*cit)->GetData().IsInt()) {
                    string val = NStr::NumericToString((*cit)->GetData().GetInt()[i]);
                    exporter.Field(val);
                }
            } else {
                string blank = "";
                exporter.Field(blank);
            }
        }
        exporter.NewRow();
    }
    return true;
}


bool SaveTableRowAsListFile (const string& save_file_name, CRef<objects::CSeq_table> values_table, int col)
{
    if (!values_table || !values_table->IsSetColumns() || col >= values_table->GetColumns().size()) {
        return false;
    }
    CRef<CSeqTable_column> src = values_table->GetColumns()[col];
    if (!src->IsSetData() || src->GetData().GetSize() == 0) {
        return false;
    }
    ios::openmode mode = ios::out;

    CNcbiOfstream os(save_file_name.c_str(), mode); 
    if (!os)
    {
        return false; 
    }
        
    CCSVExporter exporter(os, ',', '"', false);
    for (size_t i = 0; i < values_table->GetColumns()[col]->GetData().GetSize(); ++i) {
        if (i < src->GetData().GetSize()) {
            if (src->GetData().IsId()) {
                string label = "";
                src->GetData().GetId()[i]->GetLabel(&label, objects::CSeq_id::eContent);
                exporter.Field(label);
            } else if (src->GetData().IsString()) {
                exporter.Field(src->GetData().GetString()[i]);
            } else if (src->GetData().IsInt()) {
                string val = NStr::NumericToString(src->GetData().GetInt()[i]);
                exporter.Field(val);
            }
        }
    }
    exporter.NewRow();
    return true;
}


void AddAssemblyUserFieldsFromSeqTable (CRef<CUser_object> obj, CRef<CSeq_table> table)
{
    if (!table || !table->IsSetColumns() || table->GetColumns().size() < 1
        || !table->GetColumns()[0]->IsSetData()
        || !table->GetColumns()[0]->GetData().IsString()) {
        return;
    }
    CRef<CSeqTable_column> acc_col = table->GetColumns()[0];
    size_t num_rows;
    if (table->IsSetNum_rows()) {
        num_rows = table->GetNum_rows();
    } else {
        num_rows = acc_col->GetData().GetSize();
    }

    CRef<CSeqTable_column> from_col(NULL);
    CRef<CSeqTable_column> to_col(NULL);
    if (table->GetColumns().size() > 2) {
        from_col = table->GetColumns()[1];
        to_col = table->GetColumns()[2];
    }
    
    for (size_t row = 0; row < num_rows; row++) {
        string accession = table->GetColumns()[0]->GetData().GetString()[row];
        if (!NStr::IsBlank(accession)) {
            CRef<CUser_field> new_field(new CUser_field());
            new_field->SetLabel().SetId(0);
            CRef<CUser_field> acc_field(new CUser_field());
            acc_field->SetLabel().SetStr("accession");
            acc_field->SetData().SetStr(accession);
            new_field->SetData().SetFields().push_back(acc_field);
            if (from_col && to_col 
                && row < from_col->GetData().GetSize() 
                && row < to_col->GetData().GetSize()
                && !NStr::IsBlank(from_col->GetData().GetString()[row])
                && !NStr::IsBlank(to_col->GetData().GetString()[row])) {
                try {
                    int from_val = NStr::StringToInt(from_col->GetData().GetString()[row]);
                    int to_val = NStr::StringToInt(to_col->GetData().GetString()[row]);
                    CRef<CUser_field> from_field(new CUser_field());
                    from_field->SetLabel().SetStr("from");
                    from_field->SetData().SetInt(from_val - 1);
                    new_field->SetData().SetFields().push_back(from_field);
                    CRef<CUser_field> to_field(new CUser_field());
                    to_field->SetLabel().SetStr("to");
                    to_field->SetData().SetInt(to_val - 1);
                    new_field->SetData().SetFields().push_back(to_field);
                } catch(CException &e) {} catch (exception &e) {}                
            }
            obj->SetData().push_back(new_field);
        }
    }
}

CRef<CSeq_table> SeqTableFromAssemblyFields (const CUser_object& obj)
{
    if (!obj.IsSetData() || obj.GetData().size() == 0) {
        CRef<CSeq_table> empty(NULL);
        return empty;
    }
    CRef<CSeq_table> table(new CSeq_table());
    CRef<CSeqTable_column> acc_col(new CSeqTable_column());
    table->SetColumns().push_back(acc_col);
    CRef<CSeqTable_column> from_col(new CSeqTable_column());
    table->SetColumns().push_back(from_col);
    CRef<CSeqTable_column> to_col(new CSeqTable_column());
    table->SetColumns().push_back(to_col);

    ITERATE(CUser_object::TData, it, obj.GetData()) {
        if ((*it)->IsSetData() && ((*it)->GetData().IsFields())) {
            string accession = "";
            int from_val = -1;
            int to_val = -1;
            ITERATE(CUser_field::TData::TFields, fit, (*it)->GetData().GetFields()) {
                if ((*fit)->IsSetLabel() && (*fit)->GetLabel().IsStr() && (*fit)->IsSetData()) {
                    string label = (*fit)->GetLabel().GetStr();
                    if (NStr::EqualNocase(label, "accession")) {
                        if ((*fit)->GetData().IsStr()) {
                            accession = (*fit)->GetData().GetStr();
                        }
                    } else if (NStr::EqualNocase(label, "from")) {
                        if ((*fit)->GetData().IsInt()) {
                            from_val = (*fit)->GetData().GetInt();
                        }
                    } else if (NStr::EqualNocase(label, "to")) {
                        if ((*fit)->GetData().IsInt()) {
                            to_val = (*fit)->GetData().GetInt();
                        }
                    }
                }
            }
            if (!NStr::IsBlank(accession)) {
                acc_col->SetData().SetString().push_back(accession);
                string from = "";
                string to = "";
                if (from_val > -1 && to_val > -1) {
                    from = NStr::NumericToString(from_val + 1);
                    to = NStr::NumericToString(to_val + 1);
                }
                from_col->SetData().SetString().push_back(from);
                to_col->SetData().SetString().push_back(to);
            }
        }
    }

    if (acc_col->IsSetData()) {
        table->SetNum_rows(acc_col->GetData().GetSize());
    } else {
        table.Reset(NULL);
    }
    return table;
}


END_NCBI_SCOPE

