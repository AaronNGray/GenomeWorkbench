/*  $Id: srceditdialog.cpp 43832 2019-09-09 13:50:37Z filippov $
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

////@begin includes
////@end includes

#include <objects/general/Object_id.hpp>
#include <objects/seqfeat/PCRReaction.hpp>
#include <objects/seqfeat/PCRReactionSet.hpp>
#include <objects/seqfeat/PCRPrimer.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objects/seqfeat/OrgMod.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <objtools/edit/source_edit.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <objmgr/util/sequence.hpp>
#include <gui/packages/pkg_sequence_edit/srceditdialog.hpp>
#include <gui/packages/pkg_sequence_edit/srcedit_util.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/widgets/grid_widget/grid_widget.hpp>
#include <gui/widgets/edit/biosource_autocomplete.hpp>
#include <gui/objutils/interface_registry.hpp>

#include <gui/widgets/loaders/open_objects_dlg.hpp>
#include <gui/widgets/loaders/file_load_wizard.hpp>
#include <gui/utils/object_loader.hpp>
#include <gui/utils/execute_unit.hpp>
#include <gui/widgets/wx/async_call.hpp>
#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/widgets/wx/csv_exporter.hpp>
#include <wx/grid.h>

#include <gui/widgets/wx/ui_command.hpp>


////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE



/*!
 * SrcEditDialog type definition
 */

IMPLEMENT_DYNAMIC_CLASS( SrcEditDialog, wxDialog )


/*!
 * SrcEditDialog event table definition
 */

BEGIN_EVENT_TABLE( SrcEditDialog, wxDialog )

////@begin SrcEditDialog event table entries
    EVT_CHOICE( ID_QUALCHOICE, SrcEditDialog::OnQualchoiceSelected )

    EVT_BUTTON( ID_ADD_QUAL_BUTTON, SrcEditDialog::OnAddQual )

    EVT_BUTTON( ID_LOADQUALS, SrcEditDialog::OnLoadqualsClick )

    EVT_BUTTON( ID_EXPORT_QUALS, SrcEditDialog::OnExportQualsClick )


    EVT_BUTTON( wxID_CANCEL, SrcEditDialog::OnClickCancel )

////@end SrcEditDialog event table entries

END_EVENT_TABLE()


/*!
 * SrcEditDialog constructors
 */

SrcEditDialog::SrcEditDialog()
{
    Init();
}

SrcEditDialog::SrcEditDialog( wxWindow* parent, objects::CSeq_entry_Handle seh,  
                             IWorkbench* workbench, CConstRef<objects::CSeq_submit> seqSubmit,
                             wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    :  m_SEH(seh),
       m_Copied (NULL)
{
    this->m_Workbench = workbench;
    this->m_SeqSubmit = seqSubmit;
    CBioseq_CI b_iter(seh, objects::CSeq_inst::eMol_na);
    for ( ; b_iter ; ++b_iter ) 
    {
        m_BioseqHandles.push_back(*b_iter);
    }

    Init();
    Create(parent, id, caption, pos, size, style);
}

SrcEditDialog::SrcEditDialog( wxWindow* parent, const vector<CBioseq_Handle>& bioseq_handles, IWorkbench* workbench,
                             wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    :  m_BioseqHandles(bioseq_handles),
       m_Copied (NULL)
{
    this->m_Workbench = workbench;
    if (!bioseq_handles.empty()) {
        m_SEH = bioseq_handles.front().GetTopLevelEntry();
    }
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * SrcEditDialog creator
 */

bool SrcEditDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin SrcEditDialog creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    SetLayoutAdaptationMode(wxDIALOG_ADAPTATION_MODE_ENABLED);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end SrcEditDialog creation
    return true;
}


/*!
 * SrcEditDialog destructor
 */

SrcEditDialog::~SrcEditDialog()
{
////@begin SrcEditDialog destruction
////@end SrcEditDialog destruction
}


/*!
 * Member initialisation
 */

void SrcEditDialog::Init()
{
////@begin SrcEditDialog member initialisation
    m_QualList = NULL;
    m_AddQualBtn = NULL;
////@end SrcEditDialog member initialisation
    m_SaveFileDir = wxEmptyString;
    m_SaveFileName = wxEmptyString;

}


/*!
 * Control creation for SrcEditDialog
 */

void SrcEditDialog::CreateControls()
{    
////@begin SrcEditDialog content construction
    SrcEditDialog* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString itemChoiceStrings, itemChoiceStringsWritable;

    if (m_BioseqHandles.empty()) 
    {
        NCBI_THROW( CException, eUnknown, "No Source records found" );
    }
    /* this is where we need to construct a CSeqTable object from the BioSources in the record */
    CRef<objects::CSeq_table> seqTable = GetSeqTableFromSeqEntry(m_SEH);
    if (seqTable->GetNum_rows() < 1)
    {
        wxMessageBox(wxT("No Source records found"), wxT("Error"), wxOK | wxICON_ERROR);
        NCBI_THROW( CException, eUnknown, "No Source records found" );
    }

    CRef<objects::CSeq_table> choices = x_GetSourceTableChoices(seqTable);
    int glyph_col =   GetCollapsible();
    m_GridPanel = new CSeqTableGridPanel(this, seqTable, choices,glyph_col);
    itemBoxSizer3->Add(m_GridPanel, 0, wxALL, 5);
    m_Grid = m_GridPanel->GetGrid();
    
    CSeqTableGrid *gridAdapter = new CSeqTableGrid(seqTable);
    m_Grid->SetTable(gridAdapter, true);
    
    m_Grid->AutoSizeColumns();

    int l_height = m_Grid->GetColLabelSize();
    m_Grid->SetColLabelSize( 2 * l_height );           
    m_GridPanel->SetColumnSizesAndChoices();

    int pos = 0;
    ITERATE (CSeq_table::TColumns, it, seqTable->GetColumns()) 
    {
        CRef< CSeqTable_column > new_col(new CSeqTable_column);
        new_col->Assign(**it);
        m_columns.push_back(new_col);
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
    
    if (glyph_col >= 0 && glyph_col+1 < m_Grid->GetNumberCols())
    {
        m_GridPanel->InitColumnCollapse(glyph_col+1);  
    }


    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    wxArrayString m_QualListStrings;
    m_QualList = new wxChoice( itemDialog1, ID_QUALCHOICE, wxDefaultPosition, wxDefaultSize, m_QualListStrings, 0 );
    itemBoxSizer4->Add(m_QualList, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    x_RepopulateAddQualList();

    for (unsigned int i=0; i<m_QualList->GetCount(); i++)
    {
        if (!m_QualList->GetString(i).IsEmpty())
            itemChoiceStringsWritable.Add(m_QualList->GetString(i));
    }

    m_AddQualBtn = new wxButton( itemDialog1, ID_ADD_QUAL_BUTTON, _("Add Qualifier"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AddQualBtn->Enable(false);
    itemBoxSizer4->Add(m_AddQualBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    wxButton* itemButton8 = new wxButton( itemDialog1, ID_LOADQUALS, _("Import Qualifier Table"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton9 = new wxButton( itemDialog1, ID_EXPORT_QUALS, _("Export Qualifier Table"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemButton9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);


    m_StringConstraintPanel = new CStringConstraintSelect( itemDialog1, m_GridPanel, itemChoiceStrings, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_StringConstraintPanel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxFIXED_MINSIZE, 0);

    m_AecrPanel = new CApplyEditconvertPanel(itemDialog1, m_GridPanel, itemChoiceStringsWritable, 0, true, wxID_ANY, wxDefaultPosition, wxSize(1190, 261));
    itemBoxSizer2->Add(m_AecrPanel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxFIXED_MINSIZE, 1);
    
    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3);

    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton15 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemButton15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end SrcEditDialog content construction

}


CRef<objects::CSeq_table> SrcEditDialog::x_GetSourceTableChoices (CRef<objects::CSeq_table> values)
{
    vector<bool> tf_positions;
    bool any_tf = false;

    ITERATE( objects::CSeq_table::TColumns, col_iter, values->GetColumns() ) {
        bool is_tf = false;
        if ((*col_iter)->IsSetHeader() && (*col_iter)->GetHeader().IsSetTitle()) {
            string label = (*col_iter)->GetHeader().GetTitle();
            size_t pos = NStr::Find(label, "\n");
            if (pos != string::npos) {
                label = label.substr(0, pos);
            }

            if (!NStr::IsBlank(label)) {
                // see if it's a SubSource subtype:
                try {
                    objects::CSubSource::TSubtype st = objects::CSubSource::GetSubtypeValue (label, objects::CSubSource::eVocabulary_insdc);
                    if (objects::CSubSource::NeedsNoText(st)) {
                        is_tf = true;
                        any_tf = true;
                    }
                } catch (exception &) {
                    // no true/false orgmods
                }
            }
        }
        tf_positions.push_back(is_tf);
    }
    if (!any_tf) {
        CRef<objects::CSeq_table> no_choices(NULL);
        return no_choices;
    }

    CRef<objects::CSeq_table> choices (new objects::CSeq_table());
    vector<string> tf_strings;
    tf_strings.push_back("true");
    tf_strings.push_back("");

    for (size_t i = 1; i < tf_positions.size(); i++) {
        CRef<objects::CSeqTable_column> col(new objects::CSeqTable_column());
        if (tf_positions[i]) {
            for (size_t j = 0; j < tf_strings.size(); j++) {
                col->SetData().SetString().push_back(tf_strings[j]);
            }
        }
        choices->SetColumns().push_back(col);
    }
    return choices;
}


int SrcEditDialog::x_FindColumn( const wxString& name )
{
    int i = 0;

    for (i = 0; i < m_Grid->GetTable()->GetColsCount(); i++) {
        wxString col_name = m_Grid->GetTable()->GetColLabelValue(i);
        if (NStr::EqualNocase(ToStdString (name), ToStdString (col_name))) {
            return i;
        }
    }
    return -1;
}


void SrcEditDialog::x_RepopulateAddQualList()
{
    wxArrayString srcModNameStrings;
    GetQualChoices(srcModNameStrings);

    m_QualList->Clear();
    ITERATE (wxArrayString, it, srcModNameStrings) {
        m_QualList->Append (*it);
    }
}


void SrcEditDialog::ChangeColumnName(int col, string qual_name)
{
    // update choices for new column
    bool is_tf = false;
    // see if it's a SubSource subtype:
    try {
        objects::CSubSource::TSubtype st = objects::CSubSource::GetSubtypeValue (qual_name, objects::CSubSource::eVocabulary_insdc);
        if (objects::CSubSource::NeedsNoText(st)) {
            is_tf = true;
        }
    } catch (exception &) {
        // no true/false orgmods
    }
    vector<string> new_choices;
    if (is_tf) {
        new_choices.push_back("true");
        new_choices.push_back("");
    }
    m_GridPanel->UpdateColumnChoices(col, new_choices);

    x_RepopulateAddQualList();    
}


/*!
 * Should we show tooltips?
 */

bool SrcEditDialog::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap SrcEditDialog::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin SrcEditDialog bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end SrcEditDialog bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon SrcEditDialog::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin SrcEditDialog icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end SrcEditDialog icon retrieval
}

CRef<objects::CSeq_table> SrcEditDialog::x_GetValuesTableFromGridPanel()
{
    CRef<objects::CSeq_table> null_table;
    if (!m_GridPanel)
        return null_table;
    return m_GridPanel->GetValuesTable();
}

CRef<CCmdComposite> SrcEditDialog::GetCommand()
{
    CRef<CCmdComposite> cmd;
    CRef<objects::CSeq_table> values_table = x_GetValuesTableFromGridPanel();
    cmd = ApplySrcTableToSeqEntry (values_table);

    return cmd;
}



/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
 */

void SrcEditDialog::OnClickCancel( wxCommandEvent& event )
{

    bool modified = m_GridPanel->GetModified() | m_AecrPanel->GetModified();
    if (modified)
    {
        wxMessageDialog dlg(this,_("Discard modifications?"), _("Attention"),wxOK|wxCANCEL|wxCENTRE);
        if (dlg.ShowModal() == wxID_OK)
        {
            event.Skip();
        }
    }
    else    
        event.Skip();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_ADD_QUAL_BUTTON
 */

void SrcEditDialog::OnAddQual( wxCommandEvent& event )
{
    wxString qual_name = m_QualList->GetStringSelection();
    if (NStr::IsBlank(ToStdString(qual_name))) {
        return;
    }
    int collapsed_col = m_GridPanel->GetCollapseColAndExpand();
    GetNewColumn(qual_name);
    m_GridPanel->CollapseByCol(collapsed_col);
    m_AddQualBtn->Enable(false);
}

void SrcEditDialog::ResetSubPanels()
{
    wxArrayString itemChoiceStrings, itemChoiceStringsWritable;
    for (int i = 0; i < m_Grid->GetTable()->GetColsCount(); i++) 
    {
        wxString title = m_Grid->GetTable()->GetColLabelValue(i);
        if (!title.IsEmpty())
        {
            itemChoiceStrings.Add(title);
            if (!IsReadOnlyColumn(title.ToStdString())) 
                itemChoiceStringsWritable.Add(title);
        }
    }
    for (unsigned int i=0; i<m_QualList->GetCount(); i++)
    {
        if (!m_QualList->GetString(i).IsEmpty())
            itemChoiceStringsWritable.Add(m_QualList->GetString(i));
    }
    m_StringConstraintPanel->SetColumns(itemChoiceStrings);
    m_AecrPanel->SetColumns(itemChoiceStringsWritable);
}

int SrcEditDialog::GetNewColumn(wxString new_col)
{
    int num_cols = m_Grid->GetTable()->GetNumberCols();
    m_Grid->GetTable()->InsertCols(num_cols - 1);
    m_Grid->GetTable()->SetColLabelValue(num_cols - 1, new_col.ToStdString());
    m_Grid->AutoSizeColumns();
    ChangeColumnName(num_cols - 1, ToStdString(new_col));
    ResetSubPanels();
    return(num_cols - 1);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CLEAR_QUALS_BTN
 */

void SrcEditDialog::OnClearQuals( wxCommandEvent& event )
{
    int num_cols = m_Grid->GetTable()->GetNumberCols();
    int num_rows = m_Grid->GetTable()->GetNumberRows();
    for (int col = 0; col < num_cols; col++) {
        for (int row = 0; row < num_rows; row++) {
            m_Grid->GetTable()->SetValue(row, col, "");
        }
    }
    m_Grid->ForceRefresh();
}



string SrcEditDialog::FindBadColumns (CRef<objects::CSeq_table> table)
{
    int col = 1;
    string error = "";
    vector<string> already_seen;
    NON_CONST_ITERATE (objects::CSeq_table::TColumns, cit, table->SetColumns()) {
        if (!(*cit)->GetData().IsId()) {
            string label = "";
            if ((*cit)->IsSetHeader() 
                && (*cit)->GetHeader().IsSetTitle()) {
                label = (*cit)->GetHeader().GetTitle();
            }
            if (NStr::IsBlank(label)) {
                error += "Column " + NStr::NumericToString(col) + " has no header; ";
            } else {
                string new_name;
                if( (*cit)->IsSetHeader() && (*cit)->GetHeader().IsSetTitle() ) 
                {
                    string sTitle = (*cit)->GetHeader().GetTitle();
                    new_name = GetLabelForTitle(sTitle);
                }

                if (new_name.empty()) {
                    error += "Column " + NStr::NumericToString(col) + " header '" + label + "' is not recognized; ";
                } else {
                    (*cit)->SetHeader().SetTitle(new_name);
                    int as_col = 1;
                    ITERATE (vector<string>, sit, already_seen) {
                        if (NStr::Equal(new_name, *sit)) {
                            error += "Column " + NStr::NumericToString(col) + " (" 
                                  + label + ") and column " + NStr::NumericToString(as_col) 
                                  + " refer to the same field (" + new_name + "); ";
                        }
                        as_col++;
                    }
                    already_seen.push_back(new_name);
                }
            }
        }
        col++;
    }
    if (!NStr::IsBlank(error)) {
        error = error.substr(0, error.length() - 2);
    }
    return error;
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_LOADQUALS
 */

void SrcEditDialog::OnLoadqualsClick( wxCommandEvent& event )
{
    CRef<CFileLoadWizard> fileManager(new CFileLoadWizard());
    vector<string> format_ids;
    format_ids.push_back("file_loader_table");
    fileManager->LoadFormats(format_ids);

    vector<CIRef<IOpenObjectsPanelClient> > loadManagers;
    loadManagers.push_back(CIRef<IOpenObjectsPanelClient>(fileManager.GetPointer()));

    COpenObjectsDlg dlg(NULL);
    dlg.SetSize(710, 480);

    dlg.SetRegistryPath("Dialogs.Edit.OpenTable");
    dlg.SetManagers(loadManagers);

    if (dlg.ShowModal() == wxID_OK) {
        CIRef<IObjectLoader> object_loader(dlg.GetObjectLoader());
        if (!object_loader) {
            wxMessageBox(wxT("Failed to get object loader"), wxT("Error"),
                         wxOK | wxICON_ERROR);
        }
        else {
            IExecuteUnit* execute_unit = dynamic_cast<IExecuteUnit*>(object_loader.GetPointer());
            if (execute_unit) {
                if (!execute_unit->PreExecute())
                    return;

                if (!GUI_AsyncExecUnit(*execute_unit, wxT("Reading file(s)...")))
                    return; // Canceled

                if (!execute_unit->PostExecute())
                    return;
            }
            const IObjectLoader::TObjects& objects = object_loader->GetObjects();
            ITERATE(IObjectLoader::TObjects, obj_it, objects) {
                const CObject& ptr = obj_it->GetObject();
                const objects::CSeq_annot* annot = dynamic_cast<const objects::CSeq_annot*>(&ptr);
                if (annot && annot->IsSeq_table()) {
                    CRef<objects::CSeq_table> input_table (new objects::CSeq_table());
                    input_table->Assign(annot->GetData().GetSeq_table());
                    // adjust for import
                    FixTableAfterImport (input_table);

                    // make sure qual names are all legit
                    string error = FindBadColumns(input_table);
                    if (!NStr::IsBlank(error)) {
                        wxMessageBox(ToWxString(error), wxT("Error"),
                                         wxOK | wxICON_ERROR, this);
                        return;
                    }

                    // get existing table
                  
                    CRef<objects::CSeq_table> values_table = x_GetValuesTableFromGridPanel();
                    if (!values_table) {
                        return;
                    }

                    error = FindBadRows (input_table, values_table);
                    if (!NStr::IsBlank(error)) {
                        wxMessageBox(ToWxString(error), wxT("Error"),
                                         wxOK | wxICON_ERROR, this);
                        return;
                    }

                    if (CountTableColumnConflicts(values_table, *input_table) > 0) {
                        wxMessageBox(wxT("New table conflicts with existing values"), wxT("Error"),
                                         wxOK | wxICON_ERROR, this);
                        return;
                    }


                    // merge input table with existing table
                    int num_new = CombineTables (values_table, *input_table);

                    m_GridPanel->SetValuesTable(values_table);
                }
            }
        }
    }

}



/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EXPORT_QUALS
 */

void SrcEditDialog::OnExportQualsClick( wxCommandEvent& event )
{
    
    CRef<objects::CSeq_table> grid_table = x_GetValuesTableFromGridPanel();
    if (!grid_table) {
        return;
    }

    CRef<objects::CSeq_table> values_table(new objects::CSeq_table());
    values_table->Assign(*grid_table);
    // fix headers
    NON_CONST_ITERATE(objects::CSeq_table::TColumns, it, values_table->SetColumns()) {
        if ((*it)->IsSetHeader() && (*it)->GetHeader().IsSetTitle()) {
            string title = (*it)->GetHeader().GetTitle();
            size_t pos = NStr::Find(title, "\n");
            if (pos != string::npos) {
                title = title.substr(0, pos);
            }
            (*it)->SetHeader().SetTitle(title);
        }
    }

    SaveTableFile (this, m_SaveFileDir, m_SaveFileName, values_table);
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_QUALCHOICE
 */

void SrcEditDialog::OnQualchoiceSelected( wxCommandEvent& event )
{
    wxString qual_name = m_QualList->GetStringSelection();
    if (NStr::IsBlank(ToStdString(qual_name))) {
        m_AddQualBtn->Enable(false);
    } else {
        m_AddQualBtn->Enable(true);
    }
}

void SrcEditDialog::GetQualChoices(wxArrayString& srcModNameStrings)
{
    size_t i;
    for (i = 0; i < objects::CSubSource::eSubtype_other; i++) {
        try {
            string qual_name = objects::CSubSource::GetSubtypeName (i);
            if (!NStr::IsBlank(qual_name) && x_FindColumn(qual_name) == -1 && !objects::CSubSource::IsDiscouraged(i)) {
                srcModNameStrings.push_back(ToWxString(qual_name));
            }
        } catch (exception &) {
        }
    }
    for (i = 0; i < objects::COrgMod::eSubtype_other; i++) {
        try {
            string qual_name = objects::COrgMod::GetSubtypeName (i);
            if (NStr::EqualNocase(qual_name, "nat-host")) {
                qual_name = "host";
            }
            if (!NStr::IsBlank(qual_name) && x_FindColumn(qual_name) == -1&& !objects::COrgMod::IsDiscouraged(i, true)) {
                srcModNameStrings.push_back(ToWxString(qual_name));
            }
        } catch (exception &) {
        }
    }

    const wxString s_SubSourceNoteName = _(kSubSourceNote);
    const wxString s_OrgModNoteName = _(kOrgModNote);
    const wxString s_DbXref = _(kDbXref);

    if (x_FindColumn(s_DbXref) == -1) {
        srcModNameStrings.push_back(s_DbXref);
    }

    srcModNameStrings.Sort();

    if (x_FindColumn(s_OrgModNoteName) == -1) {
        srcModNameStrings.push_back(s_OrgModNoteName);
    }
    if (x_FindColumn(s_SubSourceNoteName) == -1) {
        srcModNameStrings.push_back(s_SubSourceNoteName);
    }    
}


CRef<CCmdComposite> SrcEditDialog::ApplySrcTableToSeqEntry (CRef<objects::CSeq_table>values_table)
{
    CRef<CCmdComposite> cmd( new CCmdComposite("Bulk Source Edit") );

    CRef<objects::CSeqTable_column> id_col = FindSeqTableColumnByName (values_table, kSequenceIdColLabel);

    if (!id_col) {
        return cmd;
    }

    if( ! values_table->IsSetColumns() || values_table->GetColumns().empty() ) {
        return cmd;
    }

    const objects::CSeq_table::TColumns & columns = values_table->GetColumns();
    size_t num_cols = columns.size();

    // create an edit-command maker for every column.  This somewhat byzantine
    // system is here to speed things up to avoid re-parsing the meaning
    // of each column every time.
    vector< CRef<CSrcTableColumnBase> > vecColEditFactories;

    ITERATE( objects::CSeq_table::TColumns, col_iter, columns ) {
        vecColEditFactories.push_back( CSrcTableColumnBaseFactory::Create(**col_iter) );               // TODO
    }

    vector< CRef<CSrcTableColumnBase> > old_vecColEditFactories;

    ITERATE( objects::CSeq_table::TColumns, col_iter, m_columns ) {
        old_vecColEditFactories.push_back( CSrcTableColumnBaseFactory::Create(**col_iter) );           
    }


    for (int row = 0; row < values_table->GetNum_rows() && (size_t) row < id_col->GetData().GetSize(); row++) {
        objects::CBioseq_Handle bsh = GetBioseqHandle(row);
        if (!bsh)
            continue;

        CRef<objects::CSeqdesc> new_source_desc( new objects::CSeqdesc );
        objects::CSeqdesc_CI desc_ci( bsh, objects::CSeqdesc::e_Source);
        if (desc_ci) {
            new_source_desc->Assign(*desc_ci);
        }
        objects::CBioSource & bioSource = new_source_desc->SetSource();
        string old_taxname;
        if (bioSource.IsSetOrg() && bioSource.GetOrg().IsSetTaxname()) {
            old_taxname = bioSource.GetOrg().GetTaxname();
        }

        // iterate through the columns, skipping Seq-id column, to erase current values
        // need to do this in separate steps, in case of multiple qualifier columns in table
        for( size_t col = 1; col < m_columns.size(); ++col ) {
            if (old_vecColEditFactories[col])
                old_vecColEditFactories[col]->ClearInBioSource(bioSource);                               // TODO
            // TODO: detect if change occurred to avoid excessive changes
        }

        // iterate through the columns, skipping Seq-id column
        for( size_t col = 1; col < num_cols; ++col ) {
            if (vecColEditFactories[col] && columns[col]->GetData().GetSize() > (size_t) row) {
                vecColEditFactories[col]->AddToBioSource(                                           // TODO
                    bioSource, NStr::TruncateSpaces(*columns[col]->GetStringPtr(row)), edit::eExistingText_replace_old);
            }
            // TODO: detect if change occurred to avoid excessive changes
        }

        string new_taxname;
        if (bioSource.IsSetOrg() && bioSource.GetOrg().IsSetTaxname()) {
            new_taxname = bioSource.GetOrg().GetTaxname();
        }
        if (!NStr::IsBlank(old_taxname) && !NStr::Equal(old_taxname, new_taxname)) {
            edit::CleanupForTaxnameChange(bioSource);
        }

        // automatically populate taxon ID, div, genetic codes if available
        CBioSourceAutoComplete::AutoFill(bioSource.SetOrg());

        if (desc_ci) {
            CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(desc_ci.GetSeq_entry_Handle(), *desc_ci, *new_source_desc));
            cmd->AddCommand (*ecmd);
        } else {
            objects::CBioseq_set_Handle bssh = bsh.GetParentBioseq_set();
            if (bssh && bssh.IsSetClass() && bssh.GetClass() == objects::CBioseq_set::eClass_nuc_prot) {
                cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(bssh.GetParentEntry(), *new_source_desc)) );
            } else {
                cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(bsh.GetParentEntry(), *new_source_desc)) );
            }
        }
    }

    // send composite command
    return cmd;
}


bool SrcEditDialog::OneRowOk (CRef<objects::CSeq_id> id, CRef<objects::CSeqTable_column> id_col) 
{
    if (!id || !id_col) {
        return false;
    }
    CScope &scope = m_SEH.GetScope();
    size_t row = 0;
    bool found = false;
    while (row < id_col->GetData().GetSize() && !found) {
        CRef<objects::CSeq_id> row_id = id_col->GetData().GetId()[row];
        objects::CSeq_id::E_SIC compare = id->Compare(*row_id);
        if (compare == objects::CSeq_id::e_YES) {
            found = true;
        } else if (compare == objects::CSeq_id::e_DIFF) {
            if (scope.IsSameBioseq( CSeq_id_Handle::GetHandle(*id),  CSeq_id_Handle::GetHandle(*row_id), CScope::eGetBioseq_Resolved)) {
                found = true;
                id->Assign(*row_id);
            }
        }
        row++;
    }
    return found;           
}

string SrcEditDialog::FindBadRows (CRef<objects::CSeq_table> src, CRef<objects::CSeq_table> dst)
{
    CRef<objects::CSeqTable_column> src_col = FindSeqIDColumn(*src);
    CRef<objects::CSeqTable_column> dst_col = FindSeqIDColumn(*dst);
    
    if (!src_col) {
        return "No sequence ID column selected!";
    }

    string error = "";    
    size_t row = 0;
    vector<CRef<objects::CSeq_id> > already_seen;
    while (row < src_col->GetData().GetSize()) {
        if (!OneRowOk(src_col->GetData().GetId()[row], dst_col)) {
            string id_label = "";
            src_col->GetData().GetId()[row]->GetLabel(&id_label, objects::CSeq_id::eContent);
            error += "Unable to find " + id_label + " for row " + NStr::NumericToString(row + 1) + "; ";
        } else {
            int as_row = 1;
            ITERATE (vector<CRef<objects::CSeq_id> >, sit, already_seen) {
                if ((*sit)->Compare(*(src_col->GetData().GetId()[row])) == objects::CSeq_id::e_YES) {
                    error += "Row " + NStr::NumericToString(row + 1)
                          + " and row " + NStr::NumericToString(as_row) 
                          + " refer to the same sequence; ";
                }
                as_row++;
            }
            already_seen.push_back(src_col->GetData().GetId()[row]);
        }
        row++;
    }
    if (!NStr::IsBlank(error)) {
        error = error.substr(0, error.length() - 2);
    }
    return error;
}

CBioseq_Handle SrcEditDialog::GetBioseqHandle(int row)
{
    CBioseq_Handle bsh;
    int index = m_GridPanel->GetRowIndex(row);
    if (index >=0 && index < m_BioseqHandles.size())
        bsh = m_BioseqHandles[index];
    return bsh;
}

const CObject* SrcEditDialog::RowToScopedObjects(int row, TConstScopedObjects &objects,  CBioseq_Handle &bsh)
{
    objects.clear();
    const CObject* obj = NULL;
   
    if ((unsigned int) row <  m_BioseqHandles.size() )
    {
        bsh = m_BioseqHandles[row];
        if (bsh)
        {
            if (m_SeqSubmit)
                objects.push_back(SConstScopedObject(m_SeqSubmit,&bsh.GetScope()));
            else
                objects.push_back(SConstScopedObject(bsh.GetTopLevelEntry().GetCompleteSeq_entry(),&bsh.GetScope()));            
            obj = bsh.GetBioseqCore().GetPointer();
        }
    }
    return obj;
}

CRef<objects::CSeqTable_column> SrcEditDialog::FindSeqIDColumn(const objects::CSeq_table& table)
{
    ITERATE (objects::CSeq_table::TColumns, cit, table.GetColumns()) {
        if ((*cit)->IsSetData() && (*cit)->GetData().IsId()) {
            return *cit;
        }
    }
    CRef<objects::CSeqTable_column> no_col(NULL);
    return no_col;
}

void SrcEditDialog::CombineColumns (CRef<objects::CSeqTable_column> dst_id, CRef<objects::CSeqTable_column> dst_col,
                            CRef<objects::CSeqTable_column> src_id, CRef<objects::CSeqTable_column> src_col)
{
    size_t i = 0;

    for (i = 0; i < src_id->GetData().GetSize() && i < src_col->GetData().GetSize(); i++) {
        int row = FindRowForSeqId(dst_id, src_id->GetData().GetId()[i]);
        if (row > -1) {
            while (dst_col->SetData().SetString().size() < dst_id->GetData().GetSize()) {
                dst_col->SetData().SetString().push_back ("");
            }
            try {
                dst_col->SetData().SetString()[row] = src_col->GetData().GetString()[i];
            } catch (exception &) {
                // int x = 0; // compiler warning here, commented out unused "x"
            }
        }
    }
}


// return number of new columns
int SrcEditDialog::CombineTables (CRef<objects::CSeq_table> dst, const objects::CSeq_table& src)
{
    int num_new = 0;
    CRef<objects::CSeqTable_column> src_id = FindSeqIDColumn(src);
    CRef<objects::CSeqTable_column> dst_id = FindSeqIDColumn(*dst);

    // for every column in source, if in dst, combine columns, otherwise add column to dst
    ITERATE (objects::CSeq_table::TColumns, cit, src.GetColumns()) {
        if ((*cit)->IsSetData() && !(*cit)->GetData().IsId()) {
            CRef<objects::CSeqTable_column> dst_col = FindSeqTableColumnByName (dst, (*cit)->GetHeader().GetTitle());
            if (dst_col) {
                // combine columns
            } else {
                dst_col = new objects::CSeqTable_column();
                dst_col->SetHeader().Assign((*cit)->GetHeader());
                if (!dst_col->GetHeader().IsSetTitle() && dst_col->GetHeader().IsSetField_name()) {
                    dst_col->SetHeader().SetTitle(dst_col->GetHeader().GetField_name());
                }
                dst->SetColumns().push_back(dst_col);
                num_new++;
            }
            CombineColumns(dst_id, dst_col, src_id, *cit);
        }
    }
    return num_new;
}


bool SrcEditDialog::SaveTableFile (wxWindow *parent, wxString& save_file_dir, wxString& save_file_name, CRef<objects::CSeq_table> values_table)
{
    wxFileDialog table_save_file(parent, wxT("Select a file"), save_file_dir, save_file_name,
      CFileExtensions::GetDialogFilter(CFileExtensions::kTable) + wxT("|") +
                               CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                               wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    if (table_save_file.ShowModal() == wxID_OK) 
    {
        wxString path = table_save_file.GetPath();
        wxString name = table_save_file.GetFilename();

        name.Trim(false);
        name.Trim(true);

        if (name.IsEmpty())
        {
            wxMessageBox(wxT("Please, select file name"), wxT("Error"),
                         wxOK | wxICON_ERROR, parent);
            return false; 
        }
        ios::openmode mode = ios::out;

        CNcbiOfstream os(path.fn_str(), mode); 
        if (!os)
        {
            wxMessageBox(wxT("Cannot open file ")+name, wxT("Error"),
                         wxOK | wxICON_ERROR, parent);
            return false; 
        }
        
        CCSVExporter exporter(os, '\t', '"');
        ITERATE (objects::CSeq_table::TColumns, cit, values_table->GetColumns()) {
            exporter.Field((*cit)->GetHeader().GetTitle());
        }
        exporter.NewRow();
        for (int i = 0; i < values_table->GetNum_rows(); ++i) {
            ITERATE (objects::CSeq_table::TColumns, cit, values_table->GetColumns()) {
                if (i < (*cit)->GetData().GetSize()) {
                    if ((*cit)->GetData().IsId()) {
                        string label = "";
                        (*cit)->GetData().GetId()[i]->GetLabel(&label, objects::CSeq_id::eContent);
                        exporter.Field(label);
                    } else if ((*cit)->GetData().IsString()) {
                        exporter.Field((*cit)->GetData().GetString()[i]);
                    }
                } else {
                    string blank = "";
                    exporter.Field(blank);
                }
            }
            exporter.NewRow();
        }

        save_file_dir = table_save_file.GetDirectory();
        save_file_name = table_save_file.GetFilename();
        return true;
    } else {
        return false;
    }
}

int SrcEditDialog::FindRowForSeqId (CRef<objects::CSeqTable_column> id_col, CRef<objects::CSeq_id> id)
{
    if (!id_col || !id || !id_col->IsSetData() || !id_col->GetData().IsId()) {
        return -1;
    }
    size_t row = 0;
    while (row < id_col->GetData().GetSize()) {
        if (id->Compare(*(id_col->GetData().GetId()[row])) == objects::CSeq_id::e_YES) {
            return row;
        }
        row++;
    }
    return -1;
}

int SrcEditDialog::CountColumnRowConflicts (CRef<objects::CSeqTable_column> dst_id, CRef<objects::CSeqTable_column> dst_col,
                                   CRef<objects::CSeqTable_column> src_id, CRef<objects::CSeqTable_column> src_col)
{
    int count = 0;
    for (size_t i = 0; i < src_id->GetData().GetSize() && i < src_col->GetData().GetSize(); i++) {
        int row = FindRowForSeqId(dst_id, src_id->GetData().GetId()[i]);
        if (row > -1) {
            if (dst_col->GetData().GetString().size() > row
                && !NStr::IsBlank(dst_col->GetData().GetString()[row])
                && !NStr::Equal(dst_col->GetData().GetString()[row], src_col->GetData().GetString()[i])) {
                count++;
            }
        }
    }
    return count;
}


int SrcEditDialog::CountTableColumnConflicts (CRef<objects::CSeq_table> dst, const objects::CSeq_table& src)
{
    CRef<objects::CSeqTable_column> src_id = FindSeqIDColumn(src);
    CRef<objects::CSeqTable_column> dst_id = FindSeqIDColumn(*dst);
    int conflicting_columns = 0;

    // for every column in source, if in dst, combine columns, otherwise add column to dst
    ITERATE (objects::CSeq_table::TColumns, cit, src.GetColumns()) {
        if ((*cit)->IsSetData() && !(*cit)->GetData().IsId()) {
            CRef<objects::CSeqTable_column> dst_col = FindSeqTableColumnByName (dst, (*cit)->GetHeader().GetTitle());
            if (dst_col) {
                if (CountColumnRowConflicts (dst_id, dst_col, src_id, *cit) > 0) {
                    conflicting_columns++;
                }
            }
        }
    }
    return conflicting_columns;
}

void SrcEditDialog::FixTableAfterImport (CRef<objects::CSeq_table> input_table)
{
    if (!input_table || !input_table->IsSetColumns() || input_table->GetColumns().size() < 1) {
        return;
    }
   
    bool all_default = true;
    int  col_num = 1;
    ITERATE (objects::CSeq_table::TColumns, cit, input_table->SetColumns()) { 
        string expected_title = "Column " + NStr::NumericToString(col_num);
        if (!(*cit)->IsSetHeader() || !(*cit)->GetHeader().IsSetTitle()
            || !NStr::Equal(expected_title, (*cit)->GetHeader().GetTitle())) {
            all_default = false;
            break;
        }
        col_num++;
    }                   

    bool all_match = true;
    // make titles
    NON_CONST_ITERATE (objects::CSeq_table::TColumns, cit, input_table->SetColumns()) { 
        if (!(*cit)->IsSetData() || (*cit)->GetData().GetSize() == 0) {
            // no data, skip
        } else {
            // find first value
            string first_val = "";
            if ((*cit)->GetData().IsString()) {
                first_val = (*cit)->GetData().GetString()[0];
            } else if ((*cit)->GetData().IsId()) {
                (*cit)->GetData().GetId()[0]->GetLabel(&first_val, objects::CSeq_id::eContent);
            }
            
            if (all_default || !(*cit)->IsSetHeader() || !(*cit)->GetHeader().IsSetTitle() || NStr::IsBlank((*cit)->GetHeader().GetTitle())) {                
                // set header if missing
                (*cit)->SetHeader().SetTitle(first_val);
            } else if (!NStr::Equal((*cit)->GetHeader().GetTitle(), first_val)) {
                all_match = false;
            }
        } 
    }

    // if all titles match data, delete first row
    if (all_match) {
        DeleteTableRow(input_table, 0);
    }   

    CRef<objects::CSeqTable_column> id_col = FindSeqIDColumn (*input_table);
    if (!id_col) {
        // if an ID column wasn't specified, go look for it
        NON_CONST_ITERATE (objects::CSeq_table::TColumns, cit, input_table->SetColumns()) { 
            if (!(*cit)->IsSetData() || (*cit)->GetData().GetSize() == 0) {
                // no data, skip
            } else if ((*cit)->GetData().IsString()) {
                string title = (*cit)->GetHeader().GetTitle();
                NStr::ReplaceInPlace(title, " ", "");
                NStr::ReplaceInPlace(title, "-", "");
                NStr::ReplaceInPlace(title, "_", "");
                if (NStr::EqualNocase(title, "SeqId") || NStr::EqualNocase(title, "SequenceID")) {
                    // title looks right
                    vector<CRef<objects::CSeq_id> > new_ids;
                    for (size_t j = 0; j < (*cit)->GetData().GetString().size(); j++) {
                        string val = (*cit)->GetData().GetString()[j];
                        try {
                            CRef<objects::CSeq_id> id(new objects::CSeq_id(val));
                            new_ids.push_back(id);
                        } catch (exception &) {
                            CRef<objects::CSeq_id> id(new objects::CSeq_id());
                            id->SetLocal().SetStr(val);
                            new_ids.push_back(id);
                        }
                    }
                    for (size_t j = 0; j < new_ids.size(); j++) {
                        (*cit)->SetData().SetId().push_back(new_ids[j]);
                    }
                    // found the ID column, stop looking now
                    break;
                }
            }
        }
    }

    // change numbers to strings
    NON_CONST_ITERATE (objects::CSeq_table::TColumns, cit, input_table->SetColumns()) { 
        if (!(*cit)->IsSetData() || (*cit)->GetData().GetSize() == 0) {
            // skip - no data
        } else if ((*cit)->GetData().IsInt()) {
            vector<string> values;
            for (size_t j = 0; j < (*cit)->GetData().GetInt().size(); j++) {
                values.push_back(NStr::NumericToString((*cit)->GetData().GetInt()[j]));
            }
            for (size_t j = 0; j < values.size(); j++) {
                (*cit)->SetData().SetString().push_back(values[j]);
            }
        } else if ((*cit)->GetData().IsReal()) {
            vector<string> values;
            for (size_t j = 0; j < (*cit)->GetData().GetInt().size(); j++) {
                values.push_back(NStr::NumericToString((*cit)->GetData().GetReal()[j]));
            }
            for (size_t j = 0; j < values.size(); j++) {
                (*cit)->SetData().SetString().push_back(values[j]);
            }
        }
    }
}

void SrcEditDialog::DeleteTableRow (CRef<objects::CSeq_table> table, int row)
{
    NON_CONST_ITERATE (objects::CSeq_table::TColumns, cit, table->SetColumns()) {
        if (!(*cit)->IsSetData() || (*cit)->GetData().GetSize() == 0) {
            // no data, skip
        } else {
            int num_vals = (*cit)->GetData().GetSize();
            if (row >= num_vals) {
                continue;
            }
            if ((*cit)->GetData().IsString()) {
                for (int j = row; j < num_vals - 1; j++) {
                    (*cit)->SetData().SetString()[j] = (*cit)->GetData().GetString()[j + 1];
                }
                (*cit)->SetData().SetString().pop_back();
            } else if ((*cit)->GetData().IsId()) {
                for (int j = row; j < num_vals - 1; j++) {
                    (*cit)->SetData().SetId()[j]->Assign(*((*cit)->GetData().GetId()[j + 1]));
                }
                (*cit)->SetData().SetId().pop_back();
            } else if ((*cit)->GetData().IsLoc()) {
                for (int j = row; j < num_vals - 1; j++) {
                    (*cit)->SetData().SetLoc()[j]->Assign(*((*cit)->GetData().GetLoc()[j + 1]));
                }
                (*cit)->SetData().SetLoc().pop_back();
            } else if ((*cit)->GetData().IsInt()) {
                for (int j = row; j < num_vals - 1; j++) {
                    (*cit)->SetData().SetInt()[j] = (*cit)->GetData().GetInt()[j + 1];
                }
                (*cit)->SetData().SetInt().pop_back();
            } else if ((*cit)->GetData().IsReal()) {
                for (int j = row; j < num_vals - 1; j++) {
                    (*cit)->SetData().SetReal()[j] = (*cit)->GetData().GetReal()[j + 1];
                }
                (*cit)->SetData().SetReal().pop_back();
            } else if ((*cit)->GetData().IsBit()) {
                for (int j = row; j < num_vals - 1; j++) {
                    (*cit)->SetData().SetBit()[j] = (*cit)->GetData().GetBit()[j + 1];
                }
                (*cit)->SetData().SetBit().pop_back();
            } else if ((*cit)->GetData().IsBytes()) {
                for (int j = row; j < num_vals - 1; j++) {
                    (*cit)->SetData().SetBytes()[j] = (*cit)->GetData().GetBytes()[j + 1];
                }
                (*cit)->SetData().SetBytes().pop_back();
            }
        }
    }
}

string SrcEditDialog::GetLabelForTitle(string sTitle)
{
    SIZE_TYPE nl_pos = NStr::Find(sTitle, "\n");
    if ( nl_pos != NPOS ) {
        sTitle = sTitle.substr(0, nl_pos);
    }

    SIZE_TYPE desc_pos = NStr::FindNoCase(sTitle, " descriptor");
    SIZE_TYPE feat_pos = NStr::FindNoCase(sTitle, " feature");
    if (desc_pos != NPOS) {
        sTitle = sTitle.substr(0, desc_pos);
    } if (feat_pos != NPOS) {
        sTitle = sTitle.substr(0, feat_pos);
    }

    if (sTitle.empty()) {
        return kEmptyStr;
    }

    if (sTitle == kSequenceIdColLabel) {
        return kEmptyStr;
    }

    if (NStr::EqualNocase(sTitle, kHost)) {
        sTitle = kNatHost;
    }

    if( NStr::EqualNocase(sTitle, "Organism Name") || NStr::EqualNocase(sTitle, "org") || NStr::EqualNocase(sTitle, "taxname") ) {
        return "Organism Name";
    } else if( NStr::EqualNocase(sTitle, "Taxname after Binomial")) {
        return "Taxname after binomial";
    } else if (!NStr::Equal(sTitle, kGenomeProjectID, NStr::eNocase) && NStr::StartsWith( sTitle, "genome", NStr::eNocase )) {
        string organelle = sTitle.substr(6, NPOS);
        NStr::TruncateSpacesInPlace(organelle);
        return "Genome";
    } else if (NStr::EqualNocase( sTitle, "origin" )) {
        return "Origin";
    } else if (NStr::EqualNocase(sTitle, kSubSourceNote) 
           || NStr::EqualNocase(sTitle, "subsource-note")
           || NStr::EqualNocase(sTitle, "subsrc-note")
           || NStr::EqualNocase(sTitle, "note-subsrc")) {
        return objects::CSubSource::GetSubtypeName(CSubSource::eSubtype_other);
    } else if (NStr::EqualNocase(sTitle, kOrgModNote)
           || NStr::EqualNocase(sTitle, "orgmod-note")) {
        return objects::COrgMod::GetSubtypeName(COrgMod::eSubtype_other);
    } else if ( NStr::EqualNocase(sTitle, "fwd-primer-name") || NStr::EqualNocase(sTitle, "fwd primer name")
                || NStr::EqualNocase(sTitle, "fwd-name") || NStr::EqualNocase(sTitle, "fwd name") ) {
        return "fwd-primer-name";
    } else if (NStr::EqualNocase(sTitle, "fwd-primer-seq") || NStr::EqualNocase(sTitle, "fwd primer seq")
               || NStr::EqualNocase(sTitle, "fwd-seq") || NStr::EqualNocase(sTitle, "fwd seq") ) {
        return "fwd-primer-seq";
    } else if (NStr::EqualNocase(sTitle, "rev-primer-name") || NStr::EqualNocase(sTitle, "rev primer name")
               || NStr::EqualNocase(sTitle, "rev-name") || NStr::EqualNocase(sTitle, "rev name") ) { 
        return "rev-primer-name";
    } else if (NStr::EqualNocase(sTitle, "rev-primer-seq") || NStr::EqualNocase(sTitle, "rev primer seq")
               || NStr::EqualNocase(sTitle, "rev-seq") || NStr::EqualNocase(sTitle, "rev seq") ) {
        return "rev-primer-seq";
    } else if (NStr::EqualNocase(sTitle, "common-name") || NStr::EqualNocase(sTitle, "common name")) {
        return kEmptyStr; // TODO ???
    } else if (NStr::EqualNocase(sTitle, "lineage")) {
        return kEmptyStr; // TODO ???
    } else if (NStr::EqualNocase(sTitle, "division")) {
        return kEmptyStr; // TODO ???
    } else if (NStr::StartsWith(sTitle, "dbxref", NStr::eNocase)) {
//        string dbname = sTitle.substr(6, NPOS);
//        NStr::TruncateSpacesInPlace(dbname);
        return kDbXref; 
    } else if (NStr::EqualNocase(sTitle, "taxid")) {
        return "Tax ID";
    } else if (NStr::EqualNocase(sTitle, "all-notes") || NStr::EqualNocase(sTitle, kAllNotes)) {
        return kEmptyStr; // TODO ???
    } else if (NStr::EqualNocase(sTitle, kAllPrimers)) {
        return kEmptyStr; // TODO ???
    }
    // see if it is a structured voucher - an orgmod qualifier
    else if (NStr::EndsWith(sTitle, "-inst") || NStr::EndsWith(sTitle, "-coll") || NStr::EndsWith(sTitle, "-specid")) {
        SIZE_TYPE pos = NStr::Find(sTitle, "-", NStr::eCase, NStr::eReverseSearch);
        if ( pos != NPOS) {
            string subtype = sTitle.substr(0, pos);
            bool isorgmod = COrgMod::IsValidSubtypeName(subtype, COrgMod::eVocabulary_insdc);
            if (isorgmod) {
                COrgMod::TSubtype st = COrgMod::GetSubtypeValue (subtype, COrgMod::eVocabulary_insdc);
                string epart = sTitle.substr(pos+1, NPOS);
                if (!(epart == "coll" || epart == "inst" || epart == "specid"))
                    epart.clear();
                return objects::COrgMod::GetSubtypeName(st) + epart;
            }
        }
    }
    else if (CSubSource::IsValidSubtypeName(sTitle, CSubSource::eVocabulary_insdc)) {
        CSubSource::TSubtype st = CSubSource::GetSubtypeValue (sTitle, CSubSource::eVocabulary_insdc);
        return objects::CSubSource::GetSubtypeName(st);
    } else if (COrgMod::IsValidSubtypeName(sTitle, COrgMod::eVocabulary_insdc)) {
        COrgMod::TSubtype st = COrgMod::GetSubtypeValue (sTitle, COrgMod::eVocabulary_insdc);
        return objects::COrgMod::GetSubtypeName(st);
    }

    return kEmptyStr;
}

END_NCBI_SCOPE
