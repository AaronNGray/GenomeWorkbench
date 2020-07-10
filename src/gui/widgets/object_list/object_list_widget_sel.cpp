/*  $Id: object_list_widget_sel.cpp 43755 2019-08-28 16:37:16Z katargir $
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

////@begin includes
#include "wx/imaglist.h"
////@end includes

#include <wx/sizer.h>
#include <wx/choice.h>
#include <wx/statbox.h>
#include <wx/button.h>
#include <wx/radiobut.h>
#include <wx/msgdlg.h>
#include <wx/bitmap.h>
#include <wx/icon.h>

#include <objmgr/object_manager.hpp>

#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/gui_object_info.hpp>
#include <gui/objutils/label.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/async_call.hpp>

#include <gui/widgets/object_list/object_list_widget.hpp>
#include <gui/widgets/object_list/object_list_widget_sel.hpp>


BEGIN_NCBI_SCOPE

static const string skColumnNames[] = {	"Label", "Type", "Description" };
static const int skNumColumns = sizeof(skColumnNames)/sizeof(string);
static const CLabel::ELabelType skColumnTypes[] = {
    CLabel::eContent,
    CLabel::eUserType,
    CLabel::eDescription
};

static const wxChar* kAllObjects = wxT("All");
static const wxChar* kBlankGroup = wxT("Other");
static const wxChar* kSelection = wxT("Selection");

class CObjectListSelTM : public CObjectListTableModel
{
public:
    virtual int GetNumExtraColumns() const;
    virtual wxString GetExtraColumnName( int col ) const;
    virtual wxVariant GetExtraValueAt( int row, int col ) const;
};

int CObjectListSelTM::GetNumExtraColumns() const
{
    return skNumColumns;
}

wxString CObjectListSelTM::GetExtraColumnName( int col ) const
{
    if( col < 0 || col >= GetNumExtraColumns() ){
        _ASSERT(false);
        NCBI_THROW(CException, eUnknown, "Invalid extra column index");
    }

    return ToWxString(skColumnNames[col]);
}

wxVariant CObjectListSelTM::GetExtraValueAt( int row, int col ) const
{
    if( col < 0 || col >= GetNumExtraColumns() ){
        _ASSERT(false);
        NCBI_THROW( CException, eUnknown, "Invalid extra column index" );
    }

    string label;
    if (x_GetCachedLabel(row, col, label))
        return ToWxString( label );

    const CObject* obj = m_ObjectList->GetObject( row );
    objects::CScope* scope = const_cast<objects::CScope*>( m_ObjectList->GetScope( row ) );

    SConstScopedObject scoped_object(obj, scope);
    CIRef<IGuiObjectInfo> gui_info(CreateObjectInterface<IGuiObjectInfo>(scoped_object, NULL));

    CLabel::ELabelType type = skColumnTypes[col];

    if( !gui_info.IsNull() ){
        if( type == CLabel::eUserType ){
            label = gui_info->GetType();

        } else if( type == CLabel::eUserSubtype ){
            label = gui_info->GetSubtype();
        }
    }

    if( label.empty() ){
        CLabel::GetLabel( *obj, &label, type, scope );
    }

    x_SetCachedLabel(row, col, label);
    return ToWxString( label );
}

class CGroupTableModel : public CwxAbstractTableModel
{
public:
    CGroupTableModel();

    void SetNumRows( int n_rows );
    void Clear() { SetNumRows( 0 ); }

    virtual void SetIcon( int row, const wxString& icon_alias );
    virtual void SetStringValueAt( int row, int col, const wxString& value );
    virtual void SetIntValueAt( int row, int col, const int& value );
    //virtual void SetAttachment( int row, void* attachment );
    //virtual void* GetAttachment( int row );

    /// @name CwxAbstractTableModel pure virtual functions
    /// @{
    virtual wxString GetColumnType( int col ) const;
    virtual wxString GetColumnName( int col ) const;
    virtual int GetNumRows() const;
    virtual int GetNumColumns() const;
    virtual wxVariant GetValueAt( int row, int col ) const;
    /// @}

    virtual void FireDataChanged();

    /// @name IwxDecoratedTableModel virtual functions
    /// @{
    virtual wxString GetImageAliasAt( int row, int col ) const;
    /// @}

protected:
    typedef struct {
        wxString name;
        int number;
    } TRow;

    vector<wxString> m_ColumnNames;
    vector<TRow> m_Rows;
    vector<wxString> m_Icons;
    //vector<void*> m_Attachments;
};

CGroupTableModel::CGroupTableModel()
{
    m_ColumnNames.push_back( wxT("Object group") );
    m_ColumnNames.push_back( wxT("Number of Objects") );
}

void CGroupTableModel::SetNumRows( int n_rows )
{
    m_Rows.resize( n_rows );
    m_Icons.resize( n_rows );
    //m_Attachments.resize( n_rows );
}

void CGroupTableModel::SetIcon( int row, const wxString& icon_alias )
{
    _ASSERT(row >= 0 && row < (int) m_Icons.size());
    m_Icons[row] = icon_alias;
}

void CGroupTableModel::SetStringValueAt( int row, int col, const wxString& value )
{

    _ASSERT(col >=0 && col < (int) m_ColumnNames.size());
    _ASSERT(GetColumnType(col) == wxT("string"));

    _ASSERT(row >= 0 && row < (int) m_Rows.size());

    m_Rows[row].name = value;
}

void CGroupTableModel::SetIntValueAt( int row, int col, const int& value )
{

    _ASSERT(col >=0 && col < (int) m_ColumnNames.size());
    _ASSERT(GetColumnType(col) == wxT("int"));

    _ASSERT(row >= 0 && row < (int) m_Rows.size());

    m_Rows[row].number = value;
}

int CGroupTableModel::GetNumRows() const
{
    return m_Rows.size();
}

int CGroupTableModel::GetNumColumns() const
{
    return m_ColumnNames.size();
}

wxVariant CGroupTableModel::GetValueAt( int row, int col ) const
{
    _ASSERT(col >= 0 && col < (int)m_ColumnNames.size());
    _ASSERT(row >= 0 && row < (int)m_Rows.size());

    switch( col ){
    case 0:
        return m_Rows[row].name;
    case 1:
        return m_Rows[row].number;
    }
    
    return false;
}

wxString CGroupTableModel::GetColumnType( int col ) const {

    switch( col ){
    case 0:
        return wxT("string");
    case 1:
        return wxT("int");
    }

    return wxT("bool");
}


wxString CGroupTableModel::GetColumnName( int col ) const
{
    _ASSERT(col >=0 && col < (int)m_ColumnNames.size());
    //return wxString::FromUTF8( m_ColumnNames[col].c_str() ); // ToWxString 
    return m_ColumnNames[col];
}

void CGroupTableModel::FireDataChanged()
{
    x_FireDataChanged();
}

wxString CGroupTableModel::GetImageAliasAt( int row, int col ) const
{
    if( col == 0 ){
        _ASSERT(row >= 0 && row < (int)m_Icons.size());

        return m_Icons[row];
    } else {
        return wxString();
    }
}

/*!
 * CObjectListWidgetSel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CObjectListWidgetSel, wxPanel )


/*!
 * CObjectListWidgetSel event table definition
 */

BEGIN_EVENT_TABLE( CObjectListWidgetSel, wxPanel )

////@begin CObjectListWidgetSel event table entries
    EVT_TEXT( ID_ADDACC_CTRL, CObjectListWidgetSel::OnAccessionUpdated )
    EVT_BUTTON( ID_ADDACC_BTN, CObjectListWidgetSel::OnAddAccessionClick )
    EVT_TEXT( ID_FILTER, CObjectListWidgetSel::OnFilterUpdated )
    EVT_RADIOBUTTON( ID_NUC_RDBTN, CObjectListWidgetSel::OnNucleotidesSelected )
    EVT_RADIOBUTTON( ID_PROT_RDBTN, CObjectListWidgetSel::OnProteinsSelected )
    EVT_BUTTON( ID_BUTTON_SELECT_ALL, CObjectListWidgetSel::OnSelectAll )
    EVT_TOGGLEBUTTON( ID_TOGGLEGRP, CObjectListWidgetSel::OnGroupClick )
    EVT_CHOICE( ID_CATCHOICE, CObjectListWidgetSel::OnCategorySelected )
    EVT_LIST_ITEM_SELECTED( ID_OBJTABLE, CObjectListWidgetSel::OnSelectionUpdate )
    EVT_LIST_ITEM_DESELECTED( ID_OBJTABLE, CObjectListWidgetSel::OnSelectionUpdate )
    EVT_LIST_ITEM_SELECTED( ID_GRPTABLE, CObjectListWidgetSel::OnSelectionUpdate )
    EVT_LIST_ITEM_DESELECTED( ID_GRPTABLE, CObjectListWidgetSel::OnSelectionUpdate )
////@end CObjectListWidgetSel event table entries

END_EVENT_TABLE()


/*!
 * CObjectListWidgetSel constructors
 */

CObjectListWidgetSel::CObjectListWidgetSel() 
    : m_SingleSel(false)
    , m_FilterDuplicates(true)
    , m_AccValidator(nullptr)
{
    Init();
}

CObjectListWidgetSel::CObjectListWidgetSel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_ObjectsPacked(false)
    , m_GroupsPacked(false)
    , m_SingleSel(false)
    , m_FilterDuplicates(true)
    , m_AccValidator(nullptr)
{
    Init();
    Create( parent, id, pos, size, style );
}


/*!
 * CObjectListWidgetSel creator
 */

bool CObjectListWidgetSel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    m_SingleSel = (style&wxLC_SINGLE_SEL) != 0;
    style &= ~wxLC_SINGLE_SEL;

////@begin CObjectListWidgetSel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    Centre();
////@end CObjectListWidgetSel creation
    return true;
}


/*!
 * CObjectListWidgetSel destructor
 */

CObjectListWidgetSel::~CObjectListWidgetSel()
{
////@begin CObjectListWidgetSel destruction
////@end CObjectListWidgetSel destruction
}


/*!
 * Member initialization
 */

void CObjectListWidgetSel::Init()
{
////@begin CObjectListWidgetSel member initialization
    m_DoSelectAll = false;
    m_Combinator = NULL;
    m_GrpTableModel = new CGroupTableModel();
    m_Accession = NULL;
    m_AddBtn = NULL;
    m_ToggleGroups = NULL;
    m_CatChoice = NULL;
    m_ObjTable = NULL;
    m_GrpTable = NULL;
    m_NucRadio = NULL;
    m_ProtRadio = NULL;
////@end CObjectListWidgetSel member initialization

    CRef<objects::CObjectManager> obj_mgr = objects::CObjectManager::GetInstance();
    m_Scope.Reset( new objects::CScope(*obj_mgr) );
    m_Scope->AddDefaults();

    m_DefaultSelection = kSelection;
}


/*!
 * Control creation for CObjectListWidgetSel
 */

void CObjectListWidgetSel::CreateControls()
{
////@begin CObjectListWidgetSel content construction
    // Generated by DialogBlocks, 19/03/2019 14:52:27 (unregistered)

    CObjectListWidgetSel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemPanel1, ID_STATICBOX1, _("Static"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 1, wxGROW|wxALL, wxDLG_UNIT(itemPanel1, wxSize(5, -1)).x);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer3->Add(itemBoxSizer4, 0, wxGROW|wxALL, 0);

    m_Accession = new wxTextCtrl( itemPanel1, ID_ADDACC_CTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(m_Accession, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemPanel1, wxSize(5, -1)).x);

    m_AddBtn = new wxButton( itemPanel1, ID_ADDACC_BTN, _("Add Accession"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(m_AddBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemPanel1, wxSize(5, -1)).x);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("Filter:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemPanel1, wxSize(5, -1)).x);

    m_FilterText = new wxTextCtrl( itemPanel1, ID_FILTER, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(m_FilterText, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemPanel1, wxSize(5, -1)).x);

    m_NucRadio = new wxRadioButton( itemPanel1, ID_NUC_RDBTN, _("Nucleotides"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_NucRadio->SetValue(false);
    m_NucRadio->Show(false);
    itemBoxSizer4->Add(m_NucRadio, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemPanel1, wxSize(5, -1)).x);

    m_ProtRadio = new wxRadioButton( itemPanel1, ID_PROT_RDBTN, _("Proteins"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ProtRadio->SetValue(false);
    m_ProtRadio->Show(false);
    itemBoxSizer4->Add(m_ProtRadio, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemPanel1, wxSize(5, -1)).x);

    itemBoxSizer4->Add(wxDLG_UNIT(itemPanel1, wxSize(5, -1)).x, wxDLG_UNIT(itemPanel1, wxSize(-1, 1)).y, 1, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemPanel1, wxSize(5, -1)).x);

    wxButton* itemButton12 = new wxButton( itemPanel1, ID_BUTTON_SELECT_ALL, _("Select All"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton12, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemPanel1, wxSize(5, -1)).x);

    m_ToggleGroups = new wxToggleButton( itemPanel1, ID_TOGGLEGRP, _("Groups"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
    m_ToggleGroups->SetValue(false);
    if (CObjectListWidgetSel::ShowToolTips())
        m_ToggleGroups->SetToolTip(_("Show objects in groups"));
    itemBoxSizer4->Add(m_ToggleGroups, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemPanel1, wxSize(5, -1)).x);

    wxArrayString m_CatChoiceStrings;
    m_CatChoice = new wxChoice( itemPanel1, ID_CATCHOICE, wxDefaultPosition, wxDefaultSize, m_CatChoiceStrings, 0 );
    itemBoxSizer4->Add(m_CatChoice, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemPanel1, wxSize(2, -1)).x);

    m_ObjTable = new CObjectListWidget( itemPanel1, ID_OBJTABLE, wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer3->Add(m_ObjTable, 1, wxGROW|wxALL, wxDLG_UNIT(itemPanel1, wxSize(2, -1)).x);

    m_GrpTable = new CwxTableListCtrl( itemPanel1, ID_GRPTABLE, wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer3->Add(m_GrpTable, 1, wxGROW|wxALL, wxDLG_UNIT(itemPanel1, wxSize(2, -1)).x);

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer3->Add(itemBoxSizer1, 0, wxALIGN_LEFT, wxDLG_UNIT(itemPanel1, wxSize(3, -1)).x);

    wxStaticText* itemStaticText2 = new wxStaticText( itemPanel1, wxID_STATIC, _("Total Rows Selected: "), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer1->Add(itemStaticText2, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxBOTTOM, wxDLG_UNIT(itemPanel1, wxSize(3, -1)).x);

    m_TotalRows = new wxStaticText( itemPanel1, wxID_STATIC, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer1->Add(m_TotalRows, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, wxDLG_UNIT(itemPanel1, wxSize(3, -1)).x);

    long style = m_ObjTable->GetWindowStyle();
    if (m_SingleSel)
        style |= wxLC_SINGLE_SEL;
    else
        style &= ~wxLC_SINGLE_SEL;
    m_ObjTable->SetWindowStyle(style);

    if (m_SingleSel) {
        wxWindow* wnd = FindWindow(ID_BUTTON_SELECT_ALL);
        if (wnd) wnd->Show(false);
    }

    m_ObjTable->AssignModel( new CObjectListSelTM() );

    SetTitle(wxT("Select Object"));

    m_GrpTable->AssignModel( m_GrpTableModel );
    m_GrpTable->Hide();

    x_UpdateUI();
}

void CObjectListWidgetSel::SetAccessionValidator( IObjectListAccValidator* validator )
{
    m_AccValidator = validator;

    x_UpdateUI();
}

void CObjectListWidgetSel::ShowNuclProtButtons()
{
    m_NucRadio->Show();
    m_ProtRadio->Show();

    Layout();
    Refresh();
}


void CObjectListWidgetSel::EnableGroups(bool enable) 
{
    if (enable) {
        m_ToggleGroups->Show();
    } else {
        m_ToggleGroups->Hide();
        m_ToggleGroups->SetValue(false);
    } 
    Layout();
    Refresh();
}

void CObjectListWidgetSel::x_UpdateUI()
{
    if( m_AccValidator ){
        m_Accession->Show();
        m_AddBtn->Show();
        
        x_UpdateAddBtn();
    } else {
        m_Accession->Hide();
        m_AddBtn->Hide();
    }
        
    m_CatChoice->Enable( m_ObjectListMap.size() > 1 );
        
    //if( m_GrpTableModel->GetNumRows() > 0 ){
    //m_ToggleGroups->Show();
    wxString s;
    size_t num_rows = m_ToggleGroups->GetValue() ? m_GrpTable->GetDataRowsSelected().size() : m_ObjTable->GetDataRowsSelected().size();
    s << num_rows;
    m_TotalRows->SetLabelText(s);
    //} else {
    if( false ){
        m_ObjTable->Show();
        m_GrpTable->Hide();

        m_ToggleGroups->SetValue( false );
        m_ToggleGroups->Hide();
    }

    Layout();
    Refresh();
}

void CObjectListWidgetSel::x_UpdateAddBtn()
{
    wxString acc = m_Accession->GetValue();
    acc.Trim( true ).Trim( false );

    m_AddBtn->Enable( !acc.IsEmpty() );
}

void CObjectListWidgetSel::SetTitle(const wxString& value)
{
    wxStaticBox* staticBox = (wxStaticBox*)FindWindow(ID_STATICBOX1);
    if (staticBox)
        staticBox->SetLabel(value);
}


void CObjectListWidgetSel::OnCategorySelected( wxCommandEvent& event )
{
    x_FillWithObjects( event.GetString() );
    x_UpdateUI();
}

void CObjectListWidgetSel::SetObjects( const TObjectsMap* value )
{
    m_ObjectListMap.clear();
    m_NucObjects.clear();
    m_ProtObjects.clear();

    if (!value) {
        return;
    }

    ITERATE( TObjectsMap, iter, *value ) {
        CRef<CObjectList>& obj_list = m_ObjectListMap[iter->first];
        obj_list.Reset( new CObjectList() );

        CRef<CObjectList>& nucl_obj_list = m_NucObjects[iter->first];
        nucl_obj_list.Reset( new CObjectList() );

        CRef<CObjectList>& prot_obj_list = m_ProtObjects[iter->first];
        prot_obj_list.Reset(new CObjectList());

        ITERATE( TConstScopedObjects, o_iter, iter->second ){
            CObject* object = const_cast<CObject*>(o_iter->object.GetPointer());
            CRef<objects::CScope> scope = o_iter->scope;
            obj_list->AddRow( object, scope.GetPointer());

            CConstRef<objects::CSeq_loc> loc = ConstRef(dynamic_cast<const objects::CSeq_loc*>(object));
            if (loc) {
                objects::CBioseq_Handle bsh = scope->GetBioseqHandle(*loc);
                _ASSERT(bsh);
                if (bsh) {
                    if (bsh.IsNucleotide()) {
                        nucl_obj_list->AddRow(object, scope.GetPointer());
                    }
                    else {
                        prot_obj_list->AddRow(object, scope.GetPointer());
                    }
                }
            }
        }
    }
}


bool CObjectListWidgetSel::TransferDataToWindow()
{
    m_CatChoice->Clear();

    if( m_ObjectListMap.size() > 1 ){
        m_CatChoice->Append( kAllObjects );
        ITERATE( TObjectListMap, it, m_ObjectListMap ){
            m_CatChoice->Append( 
                !it->first.empty() 
                ? ToWxString( it->first ) 
                : wxString( kBlankGroup )
            );
        }

    } else if( !m_ObjectListMap.empty() ){
        m_CatChoice->Append( 
            !m_ObjectListMap.begin()->first.empty() 
            ? ToWxString( m_ObjectListMap.begin()->first ) 
            : wxString( kAllObjects )
        );
    } else {
        m_CatChoice->Append( wxT("None") );
    }

    if (m_NucRadio->IsShown()) {
        if (!m_NucObjects.empty() && (m_NucObjects.begin()->second->GetNumRows() > 0)) {
            m_NucRadio->SetValue(true);
        }
        else {
            if (!m_ProtObjects.empty() && (m_ProtObjects.begin()->second->GetNumRows() > 0)) {
                m_ProtRadio->SetValue(true);
            }
        }
    }

    int selection = m_CatChoice->FindString(m_DefaultSelection);
    if (selection != wxNOT_FOUND) {
        m_CatChoice->Select(selection);
        x_FillWithObjects(m_DefaultSelection);
    }
    else {
        m_CatChoice->Select(0);
        x_FillWithObjects(kAllObjects);
    }

    x_UpdateUI();

    return wxPanel::TransferDataToWindow();
}

/*!
* Transfer data from the window
*/

bool CObjectListWidgetSel::TransferDataFromWindow()
{
    if( !wxPanel::TransferDataFromWindow() ){
        return false;
    }

    if( m_ToggleGroups->GetValue() ){
        m_Selection.clear();
        wxArrayInt selected_rows = m_GrpTable->GetDataRowsSelected();

        for( size_t ix = 0; ix < selected_rows.size(); ix++ ){
            TConstScopedObjects& row_objects = m_GroupsMap[ToStdString( 
                m_GrpTableModel->GetValueAt( selected_rows[ix], 0 ).GetString() 
            )];
            
            m_Selection.insert( m_Selection.end(), row_objects.begin(), row_objects.end() );
        }
    } else {
        m_Selection.clear();
        m_ObjTable->GetSelection( m_Selection );
    }

    if( m_Selection.empty() ){
        wxMessageBox(
            wxT("Please, select object(s)."), wxT("Error"),
            wxOK | wxICON_ERROR, this
        );

        if( m_ToggleGroups->GetValue() ){
            m_ObjTable->SetFocus();
        } else {
            m_GrpTable->SetFocus();
        }
        return false;
    }

    return true;
}

void CObjectListWidgetSel::x_SetUpObjectList( CObjectList* obj_list )
{
    obj_list->ClearObjectLabels();
    obj_list->ClearColStartIx();

    obj_list->AddObjectLabel( "Label", CLabel::eContent );
    obj_list->AddObjectLabel( "Type", CLabel::eUserType );
    obj_list->AddObjectLabel( "Description", CLabel::eDescription );

    obj_list->SetColStartIx( 1 );
}

CRef<CObjectList> CObjectListWidgetSel::x_FilterObjectList( CObjectList* obj_list, string pattern )
{
    if( pattern.empty() ){
        return CRef<CObjectList>( obj_list );
    }

    CRef<CObjectList> filtered_list( new CObjectList( *obj_list ) );
    filtered_list->ClearRows();

    for( int row = 0, frow = 0; row < obj_list->GetNumRows(); row++ ){
        CObject* obj = const_cast<CObject*>( obj_list->GetObject( row ) );
        objects::CScope* scope = const_cast<objects::CScope*>( obj_list->GetScope( row ) );

        string label;
        CLabel::GetLabel( *obj, &label, CLabel::eContent, scope );

        if( NStr::FindNoCase( label, pattern ) != string::npos ){
            filtered_list->AddRow( obj, scope );

            for (int col = 0; col < obj_list->GetNumColumns(); ++col) {
                switch (obj_list->GetColumnType(col)) {
                case CObjectList::eString:
                    filtered_list->SetString(col, frow, obj_list->GetString(col, row));
                    break;
                case CObjectList::eInteger:
                    filtered_list->SetInteger(col, frow, obj_list->GetInteger(col, row));
                    break;
                case CObjectList::eDouble:
                    filtered_list->SetDouble(col, frow, obj_list->GetDouble(col, row));
                    break;
                default:
                    break;
                }
            }

            ++frow;
        }
    }

    return filtered_list;
}

void CObjectListWidgetSel::x_CopyUniqueObjects(const CObjectList &src_list, CObjectList &dst_list, CObjectListWidgetSel::TObjectLabelsMap &object_labels)
{
    _ASSERT(src_list.GetNumColumns() == dst_list.GetNumColumns());
    for (int row = 0; row < src_list.GetNumRows(); row++){
        CObject* obj = const_cast<CObject*>(src_list.GetObject(row));
        objects::CScope* scope = const_cast<objects::CScope*>(src_list.GetScope(row));

        string label;
        CLabel::GetLabel(*obj, &label, CLabel::eUserTypeAndContent, scope);

        auto it = object_labels.find(label);
        if (object_labels.end() != it) {
            // If an object with that label already exists, compare the two as ASN.1
            const CSerialObject* current = dynamic_cast<CSerialObject*>(obj);
            const CSerialObject* existing = dynamic_cast<const CSerialObject*>(it->second.object.GetPointer());
            if (current == existing)
                continue;

            if (!current || !existing)
                continue;

            if (current->Equals(*existing))
                continue;
        }

        object_labels[label] = SConstScopedObject(obj, scope);
        
        int frow = dst_list.AddRow(obj, scope);

        for (int col = 0; col < src_list.GetNumColumns(); ++col) {
            switch (src_list.GetColumnType(col)) {
            case CObjectList::eString:
                dst_list.SetString(col, frow, src_list.GetString(col, row));
                break;
            case CObjectList::eInteger:
                dst_list.SetInteger(col, frow, src_list.GetInteger(col, row));
                break;
            case CObjectList::eDouble:
                dst_list.SetDouble(col, frow, src_list.GetDouble(col, row));
                break;
            default:
                break;
            }
        }
    }
}

CRef<CObjectList> CObjectListWidgetSel::x_FilterDuplicates(TObjectListMap& obj_listmap)
{
    CRef<CObjectList> filtered_list;
    TObjectLabelsMap object_labels;
    for (auto &listmap : obj_listmap) {
        const CObjectList &obj_list = *listmap.second;
        if (filtered_list.Empty()) {
            filtered_list.Reset(new CObjectList(obj_list));
            filtered_list->ClearRows();
        }
        x_CopyUniqueObjects(obj_list, *filtered_list, object_labels);
    }
    return filtered_list;
}

CRef<CObjectList> CObjectListWidgetSel::x_GetObjectList(const wxString& category, TObjectListMap& obj_listmap)
{
    CRef<CObjectList> obj_list;
    if (category == kAllObjects) {
        bool first = true;
        if (m_FilterDuplicates) {
            return x_FilterDuplicates(obj_listmap);
        }
        else {
            NON_CONST_ITERATE(TObjectListMap, it, obj_listmap){
                if (first){
                    obj_list.Reset(new CObjectList(*it->second));
                    first = false;
                }
                else {
                    obj_list->Append(*it->second);
                }
            }
        }
    }
    else {
        obj_list =
            obj_listmap[category == kBlankGroup ? string() : ToStdString(category)];
    }
    return obj_list;
}

void CObjectListWidgetSel::x_FillWithObjects(const wxString& category)
{
    SaveSettings();

    m_ObjTable->Clear();
    CRef<CObjectList> obj_list;

    if (m_NucRadio->IsShown()) {
        TObjectListMap* obj_listmap = (m_NucRadio->GetValue()) ? &m_NucObjects : &m_ProtObjects;
        obj_list = x_GetObjectList(category, *obj_listmap);
    }
    else {
        obj_list = x_GetObjectList(category, m_ObjectListMap);
    }


    if( obj_list ){
        x_SetUpObjectList( obj_list );
        obj_list = x_FilterObjectList( obj_list, m_Pattern );
        x_PopulateObjects( obj_list );
    }

    m_GroupsMap.clear();
    m_GrpTableModel->Clear();
    m_GroupsPacked = false;

    //x_PopulateGroups();

    LoadSettings();
}

void CObjectListWidgetSel::x_PopulateObjects( CObjectList* obj_list )
{
    m_ObjTable->SetObjectList( *obj_list );

    if( m_ObjTable->GetItemCount() > 0 ){
        if( m_DoSelectAll){
            m_ObjTable->SelectAll();
        } else {
            m_ObjTable->Select( 0 );
        }
        m_ObjTable->Focus( 0 );
    }

    m_ObjectsPacked = true;
}

void CObjectListWidgetSel::x_PopulateGroups()
{
    CRef<CObjectList> obj_list( m_ObjTable->GetObjectList() );
    if (!obj_list)
        return;

    int num_objects = obj_list->GetNumRows();

    TConstScopedObjects objects;

    for( int i = 0; i < num_objects; ++i ){
        objects.push_back(
            SConstScopedObject( obj_list->GetObject(i), obj_list->GetScope(i) )
        );
    }

    if (num_objects > 0 && m_Combinator) {
        GUI_AsyncExec([this, &objects](ICanceled&) { (*m_Combinator)(objects, m_GroupsMap); },
            wxT("Grouping..."));
    }

    if( !m_GroupsMap.empty() /*&& m_GroupsMap.size() <= objects.size()*/ ){

        m_GrpTableModel->SetNumRows( m_GroupsMap.size() );

        int row = 0;
        ITERATE( TObjectsMap, itr, m_GroupsMap ){
            m_GrpTableModel->SetStringValueAt( row, 0, ToWxString(itr->first) );
            m_GrpTableModel->SetIntValueAt( row, 1, itr->second.size() );
            row++;
        }
    }

    m_GrpTableModel->FireDataChanged();

    if( m_GrpTable->GetItemCount() > 0 ){
        m_GrpTable->Select( 0 );
        m_GrpTable->Focus( 0 );
    }

    m_GroupsPacked = true;
}


void CObjectListWidgetSel::x_UpdateSequences()
{
    wxString cat = m_CatChoice->GetStringSelection();
    x_FillWithObjects(cat);

    Layout();
    Refresh();
}

static const char *kObjTag = "ObjectList";
static const char *kGrpTag = "GroupList";

void CObjectListWidgetSel::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kObjTag);
        CRegistryWriteView table_view = gui_reg.GetWriteView(reg_path);
        m_ObjTable->SaveTableSettings(table_view);

        reg_path = CGuiRegistryUtil::MakeKey( m_RegPath, kGrpTag );
        table_view = gui_reg.GetWriteView( reg_path );
        m_GrpTable->SaveTableSettings(table_view);
    }
}

void CObjectListWidgetSel::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path;
        CRegistryReadView table_view;

        reg_path = CGuiRegistryUtil::MakeKey( m_RegPath, kObjTag );
        table_view = gui_reg.GetReadView(reg_path);
        LOG_POST(Info << "load table setting for " << reg_path);
        m_ObjTable->LoadTableSettings(table_view, true);
        m_ObjTable->ResetSorting();

        reg_path = CGuiRegistryUtil::MakeKey( m_RegPath, kGrpTag );
        table_view = gui_reg.GetReadView( reg_path );
        m_GrpTable->LoadTableSettings(table_view);
    }
}

/*!
 * Should we show tooltips?
 */

bool CObjectListWidgetSel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CObjectListWidgetSel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CObjectListWidgetSel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CObjectListWidgetSel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CObjectListWidgetSel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CObjectListWidgetSel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CObjectListWidgetSel icon retrieval
}

void CObjectListWidgetSel::OnFilterUpdated( wxCommandEvent& WXUNUSED(event) )
{
    wxString pattern = m_FilterText->GetValue();
    pattern.Trim( true ).Trim( false );

    m_Pattern = ToStdString( pattern );
    
    wxString cat = m_CatChoice->GetStringSelection();
    x_FillWithObjects( cat );
    x_PopulateGroups();

    x_UpdateUI();
}

const string kAddedStr("Added");

void CObjectListWidgetSel::OnAddAccessionClick( wxCommandEvent& WXUNUSED(event) )
{
    static wxString caption(wxT("Cannot add accession"));

    if (m_AccValidator == 0)
        return;

    wxString acc = m_Accession->GetValue();
    acc.Trim( true ).Trim( false );

    CRef<objects::CSeq_id> id( new objects::CSeq_id() );

    try {
        id->Set( ToStdString(acc) );
    
    } catch( objects::CSeqIdException& ){
        wxString msg(wxT("Accession \""));
        msg += acc + wxT("\" not recognized as a valid accession.");
        wxMessageBox( msg, caption, wxOK | wxICON_ERROR );
        return;
    }

    wxString errMsg;
    CConstRef<CObject> obj = m_AccValidator->Validate(*id, *m_Scope, errMsg);
    if( !obj ){
        if( errMsg.empty() ){
            errMsg = wxT("Accession \"");
            errMsg += acc + wxT("\" doesn't not fit this list.");
        }
        wxMessageBox( errMsg, caption, wxOK | wxICON_ERROR );
        return;
    }

    wxString addedStr = ToWxString( kAddedStr );

    if( m_ObjectListMap.find( kAddedStr ) == m_ObjectListMap.end() ){
        if( m_ObjectListMap.size() == 1 ){
            m_CatChoice->Clear();
            m_CatChoice->Append( kAllObjects );
            ITERATE( TObjectListMap, it, m_ObjectListMap ){
                m_CatChoice->Append( ToWxString(it->first ) );
            }
        }
        m_CatChoice->Append(addedStr);
    }

    if( m_ObjectListMap[kAddedStr].IsNull() ){
        m_ObjectListMap[kAddedStr] = CRef<CObjectList>( new CObjectList() ); 
    }

    m_ObjectListMap[kAddedStr]->AddRow(
        const_cast<CObject*>(obj.GetPointer()), m_Scope.GetPointer());

    if (m_ObjectListMap.size() == 2) {
        x_UpdateUI();
        Refresh();
    }

    m_CatChoice->SetStringSelection(addedStr);
    x_FillWithObjects(addedStr);

    int count = m_ObjTable->GetItemCount();
    if (count > 1) {
        m_ObjTable->SetItemState(0, 0, wxLIST_STATE_SELECTED);
        m_ObjTable->SetItemState(count - 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_TOGGLEGRP
 */

void CObjectListWidgetSel::OnGroupClick( wxCommandEvent& event )
{
    if( !m_GroupsPacked ){
        x_PopulateGroups();
    }

    m_ObjTable->Show( !event.IsChecked() );
    m_GrpTable->Show( event.IsChecked() );

    Layout();
    Refresh();
}

void CObjectListWidgetSel::OnAccessionUpdated( wxCommandEvent& event )
{
    x_UpdateAddBtn();
}


void CObjectListWidgetSel::OnNucleotidesSelected( wxCommandEvent& event )
{
    x_UpdateSequences();
}

void CObjectListWidgetSel::OnProteinsSelected( wxCommandEvent& event )
{
    x_UpdateSequences();
}

void CObjectListWidgetSel::OnSelectAll( wxCommandEvent& event )
{
    m_ObjTable->SelectAll();
}


void CObjectListWidgetSel::x_UpdateTotalRows() 
{
    wxString s;
    size_t num_rows = m_ToggleGroups->GetValue() ? m_GrpTable->GetSelectedItemCount() : m_ObjTable->GetSelectedItemCount();
    s << num_rows;
    if (s != m_TotalRows->GetLabelText())
        m_TotalRows->SetLabelText(s);
}


void CObjectListWidgetSel::OnSelectionUpdate(wxListEvent& event) 
{
    event.Skip();
    x_UpdateTotalRows();
}







END_NCBI_SCOPE










