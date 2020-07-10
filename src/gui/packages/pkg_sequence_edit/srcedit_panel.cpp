/*  $Id: srcedit_panel.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/seqtable/Seq_table.hpp>
#include <objects/seqtable/SeqTable_column.hpp>
#include <objects/seqtable/SeqTable_multi_data.hpp>
#include <objects/seqtable/SeqTable_column_info.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objects/seqfeat/OrgMod.hpp>
#include <objects/seqfeat/SubSource.hpp>

////@begin includes
////@end includes

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/grid_widget/grid_widget.hpp>
#include <gui/packages/pkg_sequence_edit/srcedit_panel.hpp>
#include <gui/packages/pkg_sequence_edit/srcedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_util.hpp>
#include <gui/widgets/object_list/object_list_widget.hpp>
#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/table_data.hpp>

#include <gui/packages/pkg_sequence_edit/seq_table_grid.hpp>

#include <wx/grid.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CSrcEditPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSrcEditPanel, CAlgoToolManagerParamsPanel )


/*!
 * CSrcEditPanel event table definition
 */

BEGIN_EVENT_TABLE( CSrcEditPanel, CAlgoToolManagerParamsPanel )

////@begin CSrcEditPanel event table entries
////@end CSrcEditPanel event table entries

END_EVENT_TABLE()


/*!
 * CSrcEditPanel constructors
 */

CSrcEditPanel::CSrcEditPanel()
{
    Init();
}

CSrcEditPanel::CSrcEditPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CSrcEditPanel creator
 */

bool CSrcEditPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSrcEditPanel creation
    CAlgoToolManagerParamsPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSrcEditPanel creation
    return true;
}


/*!
 * CSrcEditPanel destructor
 */

CSrcEditPanel::~CSrcEditPanel()
{
////@begin CSrcEditPanel destruction
////@end CSrcEditPanel destruction
}


/*!
 * Member initialisation
 */

void CSrcEditPanel::Init()
{
////@begin CSrcEditPanel member initialisation
    m_ObjectList = NULL;
////@end CSrcEditPanel member initialisation
}


/*!
 * Control creation for CSrcEditPanel
 */

void CSrcEditPanel::CreateControls()
{    
////@begin CSrcEditPanel content construction
    CSrcEditPanel* itemCAlgoToolManagerParamsPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCAlgoToolManagerParamsPanel1->SetSizer(itemBoxSizer2);

    wxGrid* itemGrid3 = new wxGrid( itemCAlgoToolManagerParamsPanel1, ID_GRID, wxDefaultPosition, wxSize(200, 150), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    itemGrid3->SetDefaultColSize(50);
    itemGrid3->SetDefaultRowSize(25);
    itemGrid3->SetColLabelSize(25);
    itemGrid3->SetRowLabelSize(50);
    itemGrid3->CreateGrid(5, 5, wxGrid::wxGridSelectCells);
    itemBoxSizer2->Add(itemGrid3, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(itemCAlgoToolManagerParamsPanel1, wxID_ANY, _("Sequences to Edit"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_ObjectList = new CObjectListWidget( itemCAlgoToolManagerParamsPanel1, ID_FOREIGN, wxDefaultPosition, wxSize(300, 150), wxSIMPLE_BORDER );
    itemStaticBoxSizer4->Add(m_ObjectList, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CSrcEditPanel content construction

    m_Grid = itemGrid3;
}




/*!
 * Transfer data to the window
 */

bool CSrcEditPanel::TransferDataToWindow()
{
    /// CSeq_entry, extracted from ptr somehow
    objects::CSeq_entry_Handle seh = GetSeqEntryHandle();
    if (seh) {
        /* this is where we need to construct a CSeqTable object from the BioSources in the record */
        CRef<objects::CSeq_table> seqTable = GetSeqTableFromSeqEntry(seh);
        SConstScopedObject  so(seqTable, &seh.GetScope());
        CIRef<ITableData> table_data(CreateObjectInterface<ITableData>(so, NULL));
        CSeqTableGrid *gridAdapter = new CSeqTableGrid(seqTable);
        m_Grid->SetTable(gridAdapter, true);
        /*                CCmdApplySrcTable *cmdProcessor = new CCmdApplySrcTable(seh);
        CwxGridTableAdapter* gridAdapter = new CwxGridTableAdapter(*table_data, cmdProcessor); */
        //                m_Grid->SetTable(*table_data, true);
    }

    // get undo manager

    return wxPanel::TransferDataToWindow();
}


/*!
 * Transfer data from the window
 */

bool CSrcEditPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    m_ObjectList->GetSelection(GetData().m_Objects);
    if (GetData().m_Objects.empty()) {
        wxMessageBox(wxT("Please select objects to add to the submission."), wxT("Error"),
                     wxOK | wxICON_ERROR, this);
        m_ObjectList->SetFocus();
        return false;
    }

    objects::CSeq_entry_Handle seh = GetSeqEntryHandle();
    if( seh ) {
        CSeqTableGrid *gridAdapter = dynamic_cast<CSeqTableGrid*>(m_Grid->GetTable());
        if( gridAdapter ) {
            CRef<objects::CSeq_table> values_table = gridAdapter->GetValuesTable();
            CRef<CCmdComposite> cmd = ApplySrcTableToSeqEntry (values_table, seh);
        }
    }

    return true;
}


static const char *kTableTag = "Table";

void CSrcEditPanel::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTableTag);
        CRegistryWriteView table_view = gui_reg.GetWriteView(reg_path);
        m_ObjectList->SaveTableSettings(table_view);
    }
}


void CSrcEditPanel::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path;
        CRegistryReadView table_view;

        reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTableTag);
        table_view = gui_reg.GetReadView(reg_path);
        m_ObjectList->LoadTableSettings(table_view);
    }
}


objects::CSeq_entry_Handle CSrcEditPanel::GetSeqEntryHandle(void)
{
    objects::CSeq_entry_Handle seh;

    if (m_InputObjects) {
        m_ObjectList->SetObjects(*m_InputObjects);
        m_ObjectList->SelectAll();
        NON_CONST_ITERATE (TConstScopedObjects, it, *m_InputObjects) 
        {
            const CObject* ptr = it->object.GetPointer();
            const objects::CSeq_entry* seqEntry = dynamic_cast<const objects::CSeq_entry*>(ptr);
            if (seqEntry) {
                seh = it->scope->GetSeq_entryHandle (*seqEntry);
            } else {
                const objects::CSeq_submit* seqsubmit = dynamic_cast<const objects::CSeq_submit*>(ptr);
                if (seqsubmit) {
                    if (seqsubmit->IsEntrys()) {
                        seh = it->scope->GetSeq_entryHandle(*(seqsubmit->GetData().GetEntrys().front()));
                    }
                } else {
                    const objects::CSeq_id* seqid = dynamic_cast<const objects::CSeq_id*>(ptr);
                    if( seqid ) {
                        objects::CBioseq_Handle bsh = it->scope->GetBioseqHandle(*seqid);
                        if( bsh ) {
                            seh = bsh.GetParentEntry();
                        }
                    }
                }
            }
            if( seh ) {
                return seh;
            }
        }
    }

    return seh;
}

/*!
 * Should we show tooltips?
 */

bool CSrcEditPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSrcEditPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSrcEditPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSrcEditPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSrcEditPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSrcEditPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSrcEditPanel icon retrieval
}

void CSrcEditPanel::RestoreDefaults()
{
    GetData().Init();
    TransferDataToWindow();
}

END_NCBI_SCOPE
