/*  $Id: object_list_widget.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Andrey Yazhuk, Yury Voronov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/object_list/object_list_widget.hpp>
#include <gui/widgets/wx/simple_clipboard.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/data/object_contrib.hpp>

#include <gui/utils/clipboard.hpp>

#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/seq/Bioseq.hpp>

#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/gui_object_info.hpp>
#include <gui/objutils/label.hpp>

#include <wx/artprov.h>
#include <wx/textbuf.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CObjectListTableModel

inline void CObjectListTableModel::x_AssertRowValid( int row ) const
{
    if( row < 0 || row >= GetNumRows() ){
        _ASSERT(false);
        NCBI_THROW( CException, eUnknown, "Invalid row index" );
    }
}

bool CObjectListTableModel::x_ColIsLabel( int col ) const
{
	_ASSERT(!m_ObjectList.IsNull());

	int num_labels = GetNumExtraColumns(); //m_ObjectList->GetNumObjectLabels();
	int num_columns = m_ObjectList->GetNumColumns();
	int col_delta = m_ObjectList->GetColStartIx();

	// !labels first!

	if( col_delta < 0 || col_delta >= num_labels ){
		return col < num_labels;
	}

	return( col-col_delta < 0 || col-col_delta >= num_columns );
}


int CObjectListTableModel::x_AdjustColToData( int col ) const
{
	_ASSERT(!m_ObjectList.IsNull());

	int num_labels = GetNumExtraColumns(); //m_ObjectList->GetNumObjectLabels();
	int num_columns = m_ObjectList->GetNumColumns();
	int col_delta = m_ObjectList->GetColStartIx();

	// !labels first!

	if( col_delta < 0 || col_delta >= num_labels ){
		// label : column
		return col < num_labels ? col : col - num_labels;
	}

	if( col-col_delta < 0 ){
		// label 
		return col;

	} else if( col-col_delta >= num_columns ){
		// label
		return col - num_columns;

	} else {
		// column
		return col-col_delta;
	}
}

CObjectListTableModel::~CObjectListTableModel()
{
}

void CObjectListTableModel::SetObjectList( CObjectList& list )
{
    m_ObjectList.Reset( &list );
    m_Cache.clear();

    x_FireStructureChanged();
}

void CObjectListTableModel::SetObjects( TConstScopedObjects& objects )
{
	m_ObjectList.Reset( new CObjectList() );
    m_ObjectList->Init( objects );
    m_Cache.clear();

    x_FireDataChanged();
}

void CObjectListTableModel::Clear()
{
    x_Clear();

    x_FireStructureChanged();
}

void CObjectListTableModel::x_Clear()
{
    m_ObjectList.Reset();
    m_Cache.clear();
}

// IwxTableModel virtual functions implementation

int CObjectListTableModel::GetNumRows() const
{
	if( m_ObjectList.IsNull() ) return 0;

	return m_ObjectList->GetNumRows();
}

int CObjectListTableModel::GetNumColumns() const
{
	if( m_ObjectList.IsNull() ) return 0;

	//return m_ObjectList->GetNumObjectLabels() + m_ObjectList->GetNumColumns();
	return GetNumExtraColumns() + m_ObjectList->GetNumColumns();
}

wxString CObjectListTableModel::GetColumnType( int col ) const
{
    if( !x_ColIsLabel( col ) ){
        int ol_col = x_AdjustColToData( col );

        CObjectList::EColumnType type = m_ObjectList->GetColumnType( ol_col );
        switch( type ){
        case CObjectList::eString:
            return wxT("string");
        case CObjectList::eInteger:
            return wxT("int");
        case CObjectList::eDouble:
            return wxT("double");
		default:
			_ASSERT(false);
			break;
        }
    }

	return wxT("string");
}

wxString CObjectListTableModel::GetColumnName( int col ) const
{
	int ol_col = x_AdjustColToData( col );

	return  
		x_ColIsLabel( col )
		? GetExtraColumnName/*m_ObjectList->GetObjectLabelName*/( ol_col ) 
		: ToWxString( m_ObjectList->GetColumnName( ol_col ) )
	;
}

wxVariant CObjectListTableModel::GetValueAt( int row, int col ) const
{
    x_AssertRowValid(row);

	int ol_col = x_AdjustColToData( col );

	if( x_ColIsLabel( col ) ){

		return GetExtraValueAt( row, ol_col );

		/*
		const CObject* obj = m_ObjectList->GetObject( row );
		CScope* scope = const_cast<CScope*>( m_ObjectList->GetScope( row ) );

		CLabel::ELabelType type = m_ObjectList->GetObjectLabelType( ol_col );

		string label;
		if( type == CLabel::eUserType || type == CLabel::eUserSubtype ){

			SConstScopedObject scoped_object(*obj, *scope);
			CIRef<IGuiObjectInfo> gui_info(
				CreateObjectInterface<IGuiObjectInfo>(scoped_object, NULL)
			);

			if( type == CLabel::eUserType ){
				label = gui_info->GetType();

			} else if( type == CLabel::eUserSubtype ){
				label = gui_info->GetSubtype();
			}
		}

		if( label.empty() ){
			CLabel::GetLabel( *obj, &label, type, scope );
		}

		return ToWxString( label );
		*/

	} else {
        CObjectList::EColumnType type = m_ObjectList->GetColumnType( ol_col );
        switch( type ){
        case CObjectList::eString:
            return ToWxString( m_ObjectList->GetString(ol_col, row) );
        case CObjectList::eInteger:
            return wxVariant( m_ObjectList->GetInteger(ol_col, row) );
        case CObjectList::eDouble:
            return wxVariant( m_ObjectList->GetDouble(ol_col, row) );

        default:
            _ASSERT(false);
            break;
        }
    }

    static wxString sError( wxT("Error") );
    return sError;
}

CIRef<IwxStringFormat> CObjectListTableModel::GetColumnFormat( int col ) const
{
	if( !x_ColIsLabel( col ) ){
		int ol_col = x_AdjustColToData( col );

        CObjectList::EColumnType type = m_ObjectList->GetColumnType( ol_col );
        switch( type ){
        case CObjectList::eString:
            return null;
        case CObjectList::eInteger:
            return CIRef<IwxStringFormat>( new CwxIntWithFlagsFormat( NStr::fWithCommas ) );
        case CObjectList::eDouble:
            return CIRef<IwxStringFormat>( new CwxDoublePrintfFormat( wxT("%f") ) );

        default:
            _ASSERT(false);
			break;
        }
    }

	return null;
}

wxString CObjectListTableModel::GetImageAliasAt( int row, int col ) const
{
    x_AssertRowValid( row );

    return col == 0 ? GetImageAlias( row ) : wxT("");
}

wxString CObjectListTableModel::GetImageAlias( int row ) const
{
    SConstScopedObject sco(
        m_ObjectList->GetObject(row),
        const_cast<CScope*>(m_ObjectList->GetScope(row)));

    CIRef<IGuiObjectInfo> gui_info(CreateObjectInterface<IGuiObjectInfo>(sco, NULL));

    return gui_info.IsNull() ? wxString() : ToWxString( gui_info->GetIcon() );
}


/////////// Extra columns definition //////////////////

int CObjectListTableModel::GetNumExtraColumns() const
{
	return m_ObjectList->GetNumObjectLabels();
}

wxString CObjectListTableModel::GetExtraColumnType( int /* col */ ) const
{
    return ToWxString("string");
}

wxString CObjectListTableModel::GetExtraColumnName( int col ) const
{
	return ToWxString( m_ObjectList->GetObjectLabelName( col ) );
}

wxVariant CObjectListTableModel::GetExtraValueAt( int row, int col ) const
{
    if( col < 0 || col >= GetNumExtraColumns() ){
        _ASSERT(false);
        NCBI_THROW( CException, eUnknown, "Invalid extra column index" );
    }

	string label;
    if (x_GetCachedLabel(row, col, label))
        return ToWxString( label );

    const CObject* obj = m_ObjectList->GetObject( row );
    CScope* scope = const_cast<CScope*>( m_ObjectList->GetScope( row ) );

	CLabel::ELabelType type = m_ObjectList->GetObjectLabelType( col );

    try {
        if (type == CLabel::eUserType || type == CLabel::eUserSubtype){

            SConstScopedObject scoped_object(obj, scope);
            CIRef<IGuiObjectInfo> gui_info(
                CreateObjectInterface<IGuiObjectInfo>(scoped_object, NULL)
                );

            if (!gui_info.IsNull()) {
                if (type == CLabel::eUserType){
                    label = gui_info->GetType();

                }
                else if (type == CLabel::eUserSubtype){
                    label = gui_info->GetSubtype();
                }
            }
        }
    }
    catch (const CException& err) {
        LOG_POST(Error << "Error in CObjectListTableModel::GetExtraValueAt: " << err);
    }

	if( label.empty() ){
        try {
		    CLabel::GetLabel( *obj, &label, type, scope );
        } catch (const exception&) {
            label = "N/A";
        }
	}

    x_SetCachedLabel(row, col, label);
    return ToWxString( label );
}

bool CObjectListTableModel::x_GetCachedLabel(int row, int col, string& value) const
{
    if ((size_t)GetNumRows() != m_Cache.size()) {
        m_Cache = vector<vector<CachedCell> >(GetNumRows(), vector<CachedCell>(GetNumExtraColumns()));
        return false;
    }

    if (m_Cache[row][col].m_Initialized) {
        value = m_Cache[row][col].m_Value;
        return true;
    }
    return false;
}

void CObjectListTableModel::x_SetCachedLabel(int row, int col, const string& value) const
{
    if ((size_t)GetNumRows() != m_Cache.size())
        m_Cache = vector<vector<CachedCell> >(GetNumRows(), vector<CachedCell>(GetNumExtraColumns()));

    m_Cache[row][col] = CachedCell(value);
}

CIRef<IwxStringFormat> CObjectListTableModel::GetExtraColumnFormat( int /* col */ ) const
{
    return null;
}


///////////////////////////////////////////////////////////////////////////////
/// CObjectListWidget
//IMPLEMENT_DYNAMIC_CLASS(CObjectListWidget,wxListCtrl)

/*
BEGIN_EVENT_TABLE(CObjectListWidget, CwxTableListCtrl)
    EVT_LIST_ITEM_SELECTED( wxID_ANY, CObjectListWidget::OnSelected )
    EVT_LIST_ITEM_DESELECTED( wxID_ANY, CObjectListWidget::OnDeselected )
END_EVENT_TABLE()
*/

CObjectListWidget::CObjectListWidget(
    wxWindow* parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style
)
    : CwxTableListCtrl( parent, id, pos, size, style )
{
    AssignModel( new CObjectListTableModel() );
}

void CObjectListWidget::SetObjects( TConstScopedObjects& objects )
{
	GetOLTModel()->SetObjects( objects );
}

void CObjectListWidget::SetObjectList( CObjectList& list )
{
    GetOLTModel()->SetObjectList( list );
}

void CObjectListWidget::Init( CObjectList& aList, CObjectListTableModel* aModel )
{
    if( aModel == NULL ){
        GetOLTModel()->SetObjectList( aList );

    } else {
        aModel->SetObjectList( aList );
        AssignModel( aModel );
    }
}

void CObjectListWidget::Clear()
{
    GetOLTModel()->Clear();
}

void CObjectListWidget::x_OnCopy()
{
    wxString clip_str;

    TConstScopedObjects objs;
    GetSelection( objs );

    bool first_row = true;
    ITERATE( TConstScopedObjects, obj, objs ){

        string label;
        CLabel::GetLabel(
            *obj->object, &label,
            CLabel::eDefault,
            const_cast<CScope*>( obj->scope.GetPointer() )
        );

        if( first_row ){
            first_row = false;
        } else {
            clip_str += wxTextBuffer::GetEOL();
        }
        clip_str += ToWxString( label );
    }

    if( !clip_str.IsEmpty() ){
        CSimpleClipboard::Instance().Set( clip_str );
    }
}


void CObjectListWidget::GetSelection( TConstScopedObjects& objects )
{
    wxArrayInt objIxs = GetDataRowsSelected();

    for( int i = 0; i < (int)objIxs.size(); ++i ){
        int ix = objIxs[i];
        CObject* obj = GetOLTModel()->GetObject(ix);
        CScope* sc = GetOLTModel()->GetScope(ix);

        objects.push_back(SConstScopedObject(obj, sc));
    }
}


void CObjectListWidget::DisableMultipleSelection(bool disabled)
{
    long style = GetWindowStyle();
    if (disabled) {
        style |= wxLC_SINGLE_SEL;
    }
    else {
        style &= ~wxLC_SINGLE_SEL;
    }
    SetWindowStyle(style);
}


void CObjectListWidget::SaveSettings(CRegistryWriteView& view)
{
    SaveTableSettings(view);
}


void CObjectListWidget::LoadSettings(CRegistryReadView& view)
{
    LoadTableSettings(view);
}

void CObjectListWidget::AppendMenuItems( wxMenu& aMenu )
{
    TConstScopedObjects sel_objects;
    GetSelection( sel_objects );

    if( sel_objects.empty() ){
        return;
    }

    aMenu.Append( wxID_SEPARATOR, wxT("Top Actions") );

    vector<wxEvtHandler*> handlers;
    AddCommandsForScopedObjects( 
        aMenu, 
        handlers, 
        EXT_POINT__SCOPED_OBJECTS__CMD_CONTRIBUTOR, 
        sel_objects 
    );

    CwxTableListCtrl::AppendMenuItems( aMenu );
}

void CObjectListWidget::OnSelected( wxListEvent& anEvt )
{
    //LOG_POST( Info << "Selected:" << anEvt.GetIndex() );
    anEvt.Skip();
}

void CObjectListWidget::OnDeselected( wxListEvent& anEvt )
{
    //LOG_POST( Info << "Deselected:" << anEvt.GetIndex() );
    anEvt.Skip();
}

END_NCBI_SCOPE
