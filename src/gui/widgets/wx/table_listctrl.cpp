/*  $Id: table_listctrl.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors:  Yury Voronov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/table_listctrl.hpp>
#include <gui/widgets/wx/table_cols_more_dlg.hpp>

#include <gui/widgets/wx/simple_clipboard.hpp>
#include <gui/widgets/wx/ui_command.hpp>

#include <gui/widgets/wx/csv_export_dlg.hpp>
#include <gui/widgets/wx/csv_exporter.hpp>

#include <wx/image.h>
#include <wx/menu.h>
#include <wx/log.h>
#include <wx/artprov.h>
#include <wx/textbuf.h>
#include <wx/renderer.h>
#include <algorithm>

#define MAX_COLNAMES_SHOWED_IN_POPUP 12
#define wxID_HEADER_CONTEXT_MENU 2000
#define HEADER_CONTEXT_MORE 0
#define HEADER_CONTEXT_SORT 1
#define wxID_HEADER_CONTEXT_MORE (wxID_HEADER_CONTEXT_MENU+MAX_COLNAMES_SHOWED_IN_POPUP+HEADER_CONTEXT_MORE)
#define wxID_HEADER_CONTEXT_SORT (wxID_HEADER_CONTEXT_MENU+MAX_COLNAMES_SHOWED_IN_POPUP+HEADER_CONTEXT_SORT)


BEGIN_NCBI_SCOPE

class CwxTableRow : public IwxRowModel
{
/****** PRIVATE DATA SECTION ******/
private:

    const IwxTableModel& m_Data;
    const int m_Row;

/****** PUBLIC DEFINITION SECTION ******/
public:

    CwxTableRow( const IwxTableModel& aModel, int aRow )
        : m_Data( aModel )
        , m_Row( aRow )
    {
    }

    virtual int GetNumColumns() const { return m_Data.GetNumColumns(); }

    virtual wxVariant GetValueAtColumn( int aCol ) const
    {
        return m_Data.GetValueAt( m_Row, aCol );
    }
};

class CIndexSorter {

    const IwxTableModel& m_Data;
    const IRowSorter& m_Sorter;

public:
    CIndexSorter( const IwxTableModel& aModel, const IRowSorter& aSorter )
        : m_Data( aModel )
        , m_Sorter( aSorter )
    {
    }

    bool operator()( int aRowIx, int bRowIx ) const {

        return m_Sorter( CwxTableRow( m_Data, aRowIx ), CwxTableRow( m_Data, bRowIx ) );
    }
};

//wxImageList CwxTableListCtrl::sm_ImageList( 16, 16 );

#define INIT_MEMBERS_CwxTableListCtrl \
    , m_ImageList( 16, 16 )           \
    , m_DataModel()                   \
    , mf_OwnDataModel( false )        \
    , m_ModelImageShift( 0 )          \
    , m_2ndBackColor()                \
    , m_NumColumnsVisible( 0 )        \
    , mf_Propagate( false )           \
    , m_Filter( NULL )                \
    , m_FilterInvalid( false )        \
    , m_Sorter( NULL )                \
    , m_Sorted( eNoSort )             \
    , m_SortInvalid( false )          \
    , m_PrepareSortHint( false )      \
    , m_SortedCols()                  \
    , m_CurrentCol(-1)                \
    , m_CurrentRow(-1)                \
    , m_Timer(this)

// , m_SortedCol( -1 )               

static const long kDefStyle = wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES | wxLC_VRULES;

CwxTableListCtrl::CwxTableListCtrl(
    wxWindow* parent,
    wxWindowID id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxValidator& validator,
    const wxString& name
)
    : wxListView(
        parent, id, pos, size,
        (style & ~wxLC_MASK_TYPE) | kDefStyle, validator, name
    )
    INIT_MEMBERS_CwxTableListCtrl
{
    x_Init();
}


CwxTableListCtrl::CwxTableListCtrl(
    wxWindow* parent,
    wxWindowID id,
    IwxTableModel* aData
)
    : wxListView( parent, id, wxDefaultPosition, wxDefaultSize, kDefStyle )
    INIT_MEMBERS_CwxTableListCtrl
{
    x_Init();

    SetModel( aData );
}


CwxTableListCtrl::~CwxTableListCtrl()
{
    m_Timer.Stop();

    x_ShowTooltip(false);

    RemoveModel();

    ITERATE( TSorterMap, iter, m_DefaultSorters ){
        delete iter->second;
    }
    m_DefaultSorters.clear();
}


/* XPM */
static const char * checked_xpm[] = {
"16 16 4 1",
"   c None",
".  c #808080",
"X  c Black",
"o  c #c0c0c0",
"                ",
"                ",
"  ............  ",
"  .XXXXXXXXXXo  ",
"  .X         o  ",
"  .X       X o  ",
"  .X      XX o  ",
"  .X X   XXX o  ",
"  .X XX XXX  o  ",
"  .X XXXXX   o  ",
"  .X  XXX    o  ",
"  .X   X     o  ",
"  .X         o  ",
"  .ooooooooooo  ",
"                ",
"                "};
/* XPM */
static const char * unchecked_xpm[] = {
"16 16 4 1",
"   c None",
".  c #808080",
"X  c Black",
"o  c #c0c0c0",
"                ",
"                ",
"  ............  ",
"  .XXXXXXXXXXo  ",
"  .X         o  ",
"  .X         o  ",
"  .X         o  ",
"  .X         o  ",
"  .X         o  ",
"  .X         o  ",
"  .X         o  ",
"  .X         o  ",
"  .X         o  ",
"  .ooooooooooo  ",
"                ",
"                "};

static const char * up_xpm[] = {
"16 16 2 1",
"   c None",
"X  c Gray25",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"        X       ",
"       XXX      ",
"      XXXXX     ",
"     XXXXXXX    ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                "};


static const char * down_xpm[] = {
"16 16 2 1",
"   c None",
"X  c Gray25",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"     XXXXXXX    ",
"      XXXXX     ",
"       XXX      ",
"        X       ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                "};

static const char * up2_xpm[] = {
"16 16 2 1",
"   c None",
"X  c Gray25",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"    XX     XX   ",
"   XXXX   XXXX  ",
"  XXXXXX XXXXXX ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                "};

static const char * down2_xpm[] = {
"16 16 2 1",
"   c None",
"X  c Gray25",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"  XXXXXX XXXXXX ",
"   XXXX   XXXX  ",
"    XX     XX   ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                "};

#define NO_IMAGE_ID -1
#define UNCHECKED_ICON_ID 0
#define CHECKED_ICON_ID 1
#define UP_ARROW_ID 2
#define DOWN_ARROW_ID 3
#define UP2_ARROW_ID 4
#define DOWN2_ARROW_ID 5
                       
void CwxTableListCtrl::x_Init()
{
    if( m_ImageList.GetImageCount() == 0 ){
        m_ImageList.Add( wxBitmap( unchecked_xpm ) );
        m_ImageList.Add( wxBitmap( checked_xpm ) );
        m_ImageList.Add( wxBitmap( up_xpm ) );
        m_ImageList.Add( wxBitmap( down_xpm ) );
        m_ImageList.Add( wxBitmap( up2_xpm ) );
        m_ImageList.Add( wxBitmap( down2_xpm ) );

        m_AliasToIndex[wxT("tlc_unchecked")] = 0;
        m_AliasToIndex[wxT("tlc_checked")] = 1;
    }
    mf_ImageListInstalled = false;
    //SetImageList( &m_ImageList, wxIMAGE_LIST_SMALL );

    // zebra coloring is easily confused with selection - disable it
    m_2ndBackColor = GetBackgroundColour();

    CreateDefaultSorters();

    m_FixGenericListCtrl.ConnectToControl(*this);  
}

// wxWidgets RTTI information
IMPLEMENT_DYNAMIC_CLASS( CwxTableListCtrl, wxListCtrl )

BEGIN_EVENT_TABLE( CwxTableListCtrl, wxListCtrl )
    EVT_LIST_ITEM_SELECTED( wxID_ANY, CwxTableListCtrl::OnSelectionChanged )
    EVT_LIST_ITEM_DESELECTED( wxID_ANY, CwxTableListCtrl::OnSelectionChanged )
    EVT_LIST_COL_CLICK( wxID_ANY, CwxTableListCtrl::OnColumnClick )
    EVT_LIST_COL_RIGHT_CLICK( wxID_ANY, CwxTableListCtrl::OnColumnRightClick )
    EVT_LIST_COL_BEGIN_DRAG( wxID_ANY, CwxTableListCtrl::OnColumnBeginDrag )
    EVT_LIST_COL_END_DRAG( wxID_ANY, CwxTableListCtrl::OnColumnEndDrag )
    // EVT_LIST_BEGIN_DRAG( wxID_ANY, CwxTableListCtrl::OnDebugEvent)
    // EVT_LIST_BEGIN_RDRAG( wxID_ANY, CwxTableListCtrl::OnDebugEvent)
    EVT_MOTION( CwxTableListCtrl::OnMouseMove)
    EVT_LEFT_DOWN( CwxTableListCtrl::OnLeftDown)
    EVT_TIMER(-1, CwxTableListCtrl::OnTimer)
    
    // EVT_LEFT_UP( CwxTableListCtrl::OnMouseEvent)
    // EVT_LEFT_DCLICK( CwxTableListCtrl::OnMouseEvent)
    EVT_LIST_ITEM_FOCUSED( wxID_ANY, CwxTableListCtrl::OnFocusChanged )
    EVT_MENU( wxID_SELECTALL, CwxTableListCtrl::OnSelectAll )
    EVT_KEY_DOWN( CwxTableListCtrl::OnKeyDown )
    //EVT_KEY_UP( CwxTableListCtrl::OnKeyUp )

    EVT_CONTEXT_MENU( CwxTableListCtrl::OnContextMenu )
    EVT_MENU( wxID_COPY, CwxTableListCtrl::OnCopy )
    EVT_UPDATE_UI( wxID_COPY, CwxTableListCtrl::OnCopyUpdate )

	EVT_PAINT( CwxTableListCtrl::OnPaint )

END_EVENT_TABLE()


IwxTableModel* CwxTableListCtrl::GetModel() const
{
    return m_DataModel;
}


void CwxTableListCtrl::AssignModel( IwxTableModel* data_model )
{
    SetModel( data_model, true );
}


void CwxTableListCtrl::RemoveModel()
{
    SetModel( (IwxTableModel*) NULL );
}

void CwxTableListCtrl::SetModel( IwxTableModel* data_model, bool own )
{
    if( data_model == m_DataModel ){

        mf_OwnDataModel = own;
        return;
    }

    RemoveSorter();
    RemoveFilter();

    //ClearSelection();

    if( m_DataModel != NULL ){

        m_DataModel->RemoveTMListener( *this );

        IwxDecoratedTableModel* decor_model = dynamic_cast<IwxDecoratedTableModel*>(m_DataModel);
        if( decor_model != NULL ){

            while( m_ImageList.GetImageCount() > m_ModelImageShift ){
                m_ImageList.Remove( m_ModelImageShift );
            }
            m_AliasToIndex.clear();
        }

        if( mf_OwnDataModel ){
            delete m_DataModel;
        }

        //! RemoveAllRows();
        RemoveAllColumns();
    }

    m_DataModel = data_model;
    mf_OwnDataModel = own;

    if( m_DataModel != NULL ){

        IwxDecoratedTableModel* decor_model = dynamic_cast<IwxDecoratedTableModel*>(m_DataModel);
        if( decor_model != NULL /*&& decor_model->GetImageList() != NULL*/ ){

            m_ModelImageShift = m_ImageList.GetImageCount();
        }

        m_DataModel->AddTMListener( *this );

        TableChanged(
            CwxTableModelNotice( *m_DataModel, CwxTableModelNotice::HEADER_ROW )
        );
    } else {
        // FireFocusCellChanged();
    }
}

wxVariant CwxTableListCtrl::GetValueVisibleAt( int row, int col ) const
{
    return GetModel()->GetValueAt(
        RowVisibleToData(row), ColumnVisibleToModel(col)
    );
}

const wxString CwxTableListCtrl::GetTypeVisibleAt( int row, int col ) const
{
    return GetModel()->GetTypeAt(
        RowVisibleToData( row ), ColumnVisibleToModel( col )
    );
}


void CwxTableListCtrl::EnableHeader(bool flag)
{
    SetSingleStyle(wxLC_NO_HEADER, flag);
}


#ifndef  wxHAS_LISTCTRL_COLUMN_ORDER
// Gets the column order from its index or index from its order
int CwxTableListCtrl::GetColumnOrder(int col) const
{
    if(col >= 0  &&  col < GetColumnCount() ){
        return col;
    }
    return -1;
}


int CwxTableListCtrl::GetColumnIndexFromOrder( int anOrder ) const
{
    if( anOrder >= 0 && anOrder < GetColumnCount() ){
        return anOrder;
    }

    return -1;
}

// Gets the column order for all columns
wxArrayInt CwxTableListCtrl::GetColumnsOrder() const
{
    wxArrayInt rv;

    for( int i = 0; i < GetColumnCount(); i++ ){
        rv.Add( i );
    }

    return rv;
}

// Sets the column order for all columns
bool CwxTableListCtrl::SetColumnsOrder( const wxArrayInt& orders )
{
    return false;
}
#endif

bool CwxTableListCtrl::SetColumnOrder( int col, int vis_col )
{
    int numCols = GetColumnCount();
    wxASSERT_MSG( col >= 0 && col < numCols, wxT("Col index out of bounds") );
    wxASSERT_MSG( vis_col >= 0 && vis_col < numCols, wxT("Visual col index out of bounds") );

    bool rv = false;
    wxArrayInt orderArray = GetColumnsOrder();
    if( orderArray.Count() > 0 ){

        int old_vis_col = orderArray[col];
        if( old_vis_col < vis_col ){
            for( int i = 0; i < numCols; i++ ){
                if(
                    orderArray[i] > old_vis_col
                    && orderArray[i] <= vis_col
                ){
                    orderArray[i]--;
                }
            }
            orderArray[col] = vis_col;

        } else if( vis_col < old_vis_col ){
            for( int i = 0; i < numCols; i++ ){
                if(
                    orderArray[i] >= vis_col
                    && orderArray[i] < old_vis_col
                ){
                    orderArray[i]++;
                }
            }
            orderArray[col] = vis_col;
        }
        rv = true;

        SetColumnsOrder( orderArray );
    }

    return rv;
}

int CwxTableListCtrl::GetNumColumnsVisible() const
{
    return m_NumColumnsVisible;
}

int CwxTableListCtrl::GetNumColumnsHidden() const
{
    return GetColumnCount() - GetNumColumnsVisible();
}

int CwxTableListCtrl::ColumnVisibleToIndex( int aCol ) const
{
    if( aCol >= 0 && aCol < GetNumColumnsVisible() ){

        wxArrayInt order = GetColumnsOrder();
        wxArrayInt index;
        index.Add( -1, GetColumnCount() );

        for( int i = 0; i < GetColumnCount(); i++ ){
            index[order[i]] = i;
        }

        for( int i = 0; i <= aCol; i++ ){
            if( !m_Columns[index[i]].m_Visible ){
                aCol++;
            }
        }

        return index[aCol];
    }

    return -1;
}
int CwxTableListCtrl::ColumnIndexToVisible( int aColIx ) const
{
    if( aColIx >= 0 && aColIx < GetColumnCount() ){
        if( !m_Columns[aColIx].m_Visible ){
            return -1;
            // we may return minus-hidden index
        }

        wxArrayInt order = GetColumnsOrder();

        int hidden = 0;
        for( int i = 0; i < GetColumnCount(); i++ ){
            if(
                !m_Columns[i].m_Visible
                && order[i] < order[aColIx]
            ){
                hidden++;
            }
        }

        return order[aColIx] - hidden;
    }

    return -1;
}

int CwxTableListCtrl::ColumnHiddenToIndex( int aCol ) const
{
    if( aCol >= 0 && aCol < GetNumColumnsHidden() ){

        wxArrayInt order = GetColumnsOrder();
        wxArrayInt index;
        index.Add( -1, GetColumnCount() );

        for( int i = 0; i < GetColumnCount(); i++ ){
            index[order[i]] = i;
        }

        for( int i = 0; i <= aCol; i++ ){
            if( m_Columns[index[i]].m_Visible ){
                aCol++;
            }
        }

        return index[aCol];
    }

    return -1;
}

int CwxTableListCtrl::ColumnIndexToHidden( int aColIx ) const
{
    if( aColIx >= 0 && aColIx < GetColumnCount() ){
        if( m_Columns[aColIx].m_Visible ){
            return -1;
            // we may return minus-hidden index
        }

        wxArrayInt order = GetColumnsOrder();

        int visible = 0;
        for( int i = 0; i < GetColumnCount(); i++ ){
            if(
                m_Columns[i].m_Visible
                && order[i] < order[aColIx]
            ){
                visible++;
            }
        }

        return order[aColIx] - visible;
    }

    return -1;
}

int CwxTableListCtrl::ColumnIndexToModel( int aCol ) const
{
    if( aCol >= 0 && aCol < GetColumnCount() ){
        return m_Columns[aCol].m_ModelIx;
    }

    return -1;
}

int CwxTableListCtrl::ColumnModelToIndex( int aCol ) const
{
    if( aCol >= 0 && aCol < GetColumnCount() ){
        if( m_Columns[aCol].m_ModelIx == aCol ){
            return aCol;
        }

        int col = 0;
        ITERATE( vector<CTableListColumn>, iter, m_Columns ){
            if( iter->m_ModelIx == aCol ){
                return col;
            }
            col++;
        }
    }

    return -1;
}

int CwxTableListCtrl::ColumnVisibleToModel( int aCol ) const
{
    return ColumnIndexToModel( ColumnVisibleToIndex( aCol ) );
}

int CwxTableListCtrl::ColumnModelToVisible( int aCol ) const
{
    return ColumnIndexToVisible( ColumnModelToIndex( aCol ) );
}

bool CwxTableListCtrl::IsColumnVisibleByIndex( int aColIx ) const
{
    return m_Columns[aColIx].m_Visible;
}

wxString CwxTableListCtrl::GetColumnNameByModel( int aColIx ) const
{
    return GetModel()->GetColumnName( aColIx );
}

wxString CwxTableListCtrl::GetColumnNameByIndex( int aColIx ) const
{
    return GetColumnNameByModel( ColumnIndexToModel( aColIx ) );
}

wxString CwxTableListCtrl::GetColumnNameVisible( int aCol ) const
{
    return GetColumnNameByIndex( ColumnVisibleToIndex( aCol ) );
}

wxString CwxTableListCtrl::GetColumnNameHidden( int aCol ) const
{
    return GetColumnNameByIndex( ColumnHiddenToIndex( aCol ) );
}

wxString CwxTableListCtrl::GetColumnTitleByModel( int aCol ) const
{
    //! for now
    return GetColumnNameByModel( aCol );
}

wxString CwxTableListCtrl::GetColumnTitleByIndex( int aColIx ) const
{
    //! for now
    return GetColumnNameByIndex( aColIx );
}

wxString CwxTableListCtrl::GetColumnTitleVisible( int aCol ) const
{
    return GetColumnTitleByIndex( ColumnVisibleToIndex( aCol ) );
}

wxString CwxTableListCtrl::GetColumnTitleHidden( int aCol ) const
{
    return GetColumnTitleByIndex( ColumnHiddenToIndex( aCol ) );
}

int CwxTableListCtrl::GetModelIndexByName( wxString aName ) const
{
    for( int i = 0; i < GetModel()->GetNumColumns(); i++ ){
        if( aName == GetModel()->GetColumnName( i ) ){
            return i;
        }
    }

    return -1;
}

int CwxTableListCtrl::GetColumnIndexByName( wxString aName ) const
{
    for( int i = 0; i < GetColumnCount(); i++ ){
        if( aName == GetColumnNameByIndex( i ) ){
            return i;
        }
    }

    return -1;
}

int CwxTableListCtrl::GetColumnVisibleByName( wxString name ) const
{
    return ColumnIndexToVisible( GetColumnIndexByName( name ) );
}

int CwxTableListCtrl::GetColumnHiddenByName( wxString name ) const
{
    return ColumnIndexToHidden( GetColumnIndexByName( name ) );
}

wxListColumnFormat CwxTableListCtrl::GetColumnAlign( int aColIx ) const
{
    wxListItem item;
    item.SetMask( wxLIST_MASK_FORMAT );
    if( wxListView::GetColumn( aColIx, item ) ){
        return item.GetAlign();
    }
    return wxLIST_FORMAT_CENTRE;
}

void CwxTableListCtrl::SetColumnAlign( int aColIx, wxListColumnFormat aAlign )
{
    wxListItem item;
    item.SetAlign( aAlign );
    if( wxListView::SetColumn( aColIx, item ) ){
        m_Columns[aColIx].m_Item.SetAlign( aAlign );
    }
}

//! SetColumn()

void CwxTableListCtrl::AddColumn( CTableListColumn& aColumn )
{
    if( aColumn.m_Item.GetMask() & wxLIST_MASK_WIDTH ){
        aColumn.m_Width = aColumn.m_Item.GetWidth();
    }

    if( !aColumn.m_Visible ){
        aColumn.m_Item.SetWidth( 0 );
    }


    if( (aColumn.m_Item.GetMask() & wxLIST_MASK_TEXT) == 0 ){
        aColumn.m_Item.SetText( GetColumnTitleByModel( aColumn.m_ModelIx ) );
    }

    if( wxListView::InsertColumn( GetColumnCount(), aColumn.m_Item ) >= 0 ){
        m_Columns.push_back( aColumn );

        if( aColumn.m_Width < 0 ){
            aColumn.m_Width = GetColumnWidth( GetColumnCount() -1 );
        }

        if( aColumn.m_Visible ){
            m_NumColumnsVisible++;
        }
    }

}

void CwxTableListCtrl::AddColumn( int aModelIx, bool aHidden )
{
    CTableListColumn colData( aModelIx );
    colData.m_Visible = !aHidden;
    colData.m_Item.SetAlign( GetAlignByType( GetModel()->GetColumnType( aModelIx ) ) );

    IwxDecoratedTableModel* decor_model = dynamic_cast<IwxDecoratedTableModel*>(GetModel());
    if( decor_model != NULL ){
        colData.m_Formatter = decor_model->GetColumnFormat( aModelIx );
    }

    AddColumn( colData );
}

void CwxTableListCtrl::RemoveColumn( int aColIx, bool aHide )
{
    if( aColIx >= 0 && aColIx < GetColumnCount() ){
        bool was_visible = false;
        bool was_erased = false;

        if( aHide ){
            if( m_Columns[aColIx].m_Visible ){
                m_Columns[aColIx].m_Width = GetColumnWidth( aColIx );
                if( m_Columns[aColIx].m_Width == 0 ){
                    m_Columns[aColIx].m_Width = 80;
                }
                m_Columns[aColIx].m_Visible = false;
                SetColumnWidth( aColIx, 0 );

                was_visible = true;
            }
        } else {
            if( wxListView::DeleteColumn( aColIx ) ){
                if( m_Columns[aColIx].m_Visible ){
                    was_visible = true;
                }

                m_Columns.erase( m_Columns.begin() + aColIx );
                was_erased = true;
            }
        }

        if( was_visible ){
            m_NumColumnsVisible--;

            for( size_t ix = 0; ix < m_SortedCols.size(); ix++ ){
                if( m_SortedCols[ix].first == aColIx ){
                    m_SortedCols.erase( m_SortedCols.begin() + ix );

                    SetSorterByColumns();
                    ApplySorting();

                    break;
                }
            }
        }

        if( was_erased ){
            for( size_t ix = 0; ix < m_SortedCols.size(); ix++ ){
                if( m_SortedCols[ix].first > aColIx ){
                    m_SortedCols[ix].first--;
                }
            }
        }
    }
}

void CwxTableListCtrl::MoveColumn( int aColIx, int aTargetColIx )
{
    _ASSERT( aColIx >= 0 );
    _ASSERT( aTargetColIx >= 0 );

    if( aColIx >= GetColumnCount() ){
        return;
    }
    if( aTargetColIx >= GetColumnCount() ){
        return;
    }

    if( aColIx == aTargetColIx ){
        return;
    }

    wxListView::GetColumn( aColIx, m_Columns[aColIx].m_Item );
    CTableListColumn column = m_Columns[aColIx];

    DeleteColumn( aColIx );
    m_Columns.erase( m_Columns.begin() + aColIx );

    InsertColumn( aTargetColIx, column.m_Item );
    m_Columns.insert( m_Columns.begin() + aTargetColIx, column );


    for( int ix = 0; ix < (int)m_SortedCols.size(); ix++ ){
        if( m_SortedCols[ix].first == aColIx ){
            m_SortedCols[ix].first = aTargetColIx;

        } else if( m_SortedCols[ix].first > aColIx ){
            if( aTargetColIx >= m_SortedCols[ix].first ){
                m_SortedCols[ix].first--;
            }

        } else if( m_SortedCols[ix].first < aColIx ){
            m_SortedCols[ix].first++;
        }
    }
    
/*
    if( m_SortedCol == aColIx ){
        m_SortedCol = aTargetColIx;

    } else if( m_SortedCol > aColIx ){
        if( aTargetColIx >= m_SortedCol ){
            m_SortedCol--;
        }

    } else if( m_SortedCol < aColIx ){
        if( aTargetColIx <= m_SortedCol ){
            m_SortedCol++;
        }
    }
*/

    Refresh();
}

void CwxTableListCtrl::ShowColumn( int aColIx )
{
    if( aColIx >= 0 && aColIx < GetColumnCount() ){
        if( !m_Columns[aColIx].m_Visible ){
            SetColumnWidth( aColIx, m_Columns[aColIx].m_Width );
            m_Columns[aColIx].m_Visible = true;
            m_NumColumnsVisible++;
        }
    }
}

void CwxTableListCtrl::RemoveAllColumns()
{
    ClearAll();

    m_Columns.clear();
    m_NumColumnsVisible = 0;
}

void CwxTableListCtrl::CreateAllColumnsFromModel()
{
    if( GetModel() == NULL ){
        return;
    }

    for( int col = 0; col < GetModel()->GetNumColumns(); col++ ){
        CTableListColumn column( col );
        column.m_Item.SetColumn( col );
        column.m_Item.SetAlign( GetAlignByType( GetModel()->GetColumnType( col ) ) );

        IwxDecoratedTableModel* decor_model = dynamic_cast<IwxDecoratedTableModel*>(GetModel());
        if( decor_model != NULL ){
            column.m_Formatter = decor_model->GetColumnFormat( col );
        }

#if !defined(__WIN32__) || defined(__WXUNIVERSAL__)
        column.m_Item.SetWidth(
            GetModel()->GetColumnType( col ).Cmp( wxT("string") ) == 0
            ? 100
            : 40
        );
#endif

        AddColumn( column );
    }
}


/// /////////// Row staff /////////////////

int CwxTableListCtrl::RowVisibleToData( int aRow ) const
{
    if( aRow < 0 || aRow >= GetNumRowsVisible() ){
        return -1;
    }

    return m_VisibleRows[aRow];
}


int CwxTableListCtrl::RowDataToVisible( int aRow ) const
{
    if( aRow >= 0 && aRow < GetNumRowsVisible() ){
        if( m_VisibleRows[aRow] == aRow ){
            return aRow;
        }

        int row = 0;
        ITERATE( vector<int>, iter, m_VisibleRows ){
            if( *iter == aRow ){
                return row;
            }
            row++;
        }
    }

    return -1;
}

int CwxTableListCtrl::GetFocusRow() const
{
    return GetFocusedItem();
}


void CwxTableListCtrl::SetFocusRow(int row)
{
    Focus(row);
}


int CwxTableListCtrl::GetFocusColumn() const
{
    return 0;
}


void CwxTableListCtrl::OnFocusChanged( wxListEvent& event )
{
    long itemId = event.GetItem();
    if( itemId >= 0 && itemId < GetItemCount() && wxGetKeyState( WXK_SHIFT ) ){
        wxListEvent sel_evt( event );
        sel_evt.SetEventType(
            IsSelected( itemId )
            ? wxEVT_COMMAND_LIST_ITEM_SELECTED
            : wxEVT_COMMAND_LIST_ITEM_DESELECTED
        );

        GetEventHandler()->ProcessEvent(sel_evt);
    }

    event.Skip();
}


long CwxTableListCtrl::GetLastSelected() const
{
    return GetNextItem( GetItemCount(), wxLIST_NEXT_ABOVE, wxLIST_STATE_SELECTED );
}


void CwxTableListCtrl::SelectAll()
{
    int num_items = GetItemCount();

    for( int i = 0;  i < num_items; i++ )    {
        SetItemState( i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
    }
}


void CwxTableListCtrl::ClearSelection()
{
    for(  int item = -1;
        (item = GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED )) != -1;) {
        SetItemState( item, 0, wxLIST_STATE_SELECTED );
    }
}


wxArrayInt CwxTableListCtrl::GetDataRowsSelected() const
{
    wxArrayInt rv;

    for(  int item = -1;
        (item = GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED )) != -1;) {
        rv.Add( RowVisibleToData( item ) );
    }
    return rv;
}


void CwxTableListCtrl::GetDataRowsSelected(vector<int>& rows) const
{
    for(  int item = -1;
        (item = GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED )) != -1;) {
        int r = RowVisibleToData(item);
        rows.push_back(r);
    }
}


void CwxTableListCtrl::SetDataRowsSelected(const vector<int>& rows)
{
    ClearSelection();

    for(  size_t i = 0; i < rows.size();  i++ ) {
        long vis = RowDataToVisible(rows[i]);
        Select(vis, true);
    }
}


const IRowFilter* CwxTableListCtrl::GetFilter() const
{
    return m_Filter;
}


void CwxTableListCtrl::SetFilter( IRowFilter* filter )
{
    if( m_Filter != filter ){
        m_Filter = filter;
        m_FilterInvalid = true;
    }
}


IRowFilter* CwxTableListCtrl::RemoveFilter()
{
    IRowFilter* rv = m_Filter;
    SetFilter( (IRowFilter*)NULL );
    return rv;
}


void CwxTableListCtrl::ApplyFiltering( bool preserve_state )
{
    if( !m_FilterInvalid ){
        return;
    }

    if( GetModel() == NULL ){
        return;
    }
    IwxTableModel& model = *GetModel();

    SViewState table_state;
    if( preserve_state ){
        // remember focus
        x_SaveViewState( &table_state );
    }

    //?  remember selection
    /*
    _ASSERT( GetSelectionModel() != NULL );
    ISelectionModel& selModel = *GetSelectionModel();

    vector<bool> selected( model.GetNumRows() );

    bool beenSelected = !selModel.IsSelectionEmpty();
    if( beenSelected y){
        for( size_t vRow = 0; vRow < m_VisibleRows.size(); vRow++ ){
            if( selModel.IsSelectedIndex( vRow ) ){
                selected[m_VisibleRows[vRow]] = true;
            }
        }
        selModel.ClearSelection();
    }
    */

    m_VisibleRows.clear();
    m_VisibleRows.reserve( min( 100, model.GetNumRows() ) );

    if( GetFilter() == NULL ){
        for( int mRow = 0; mRow < model.GetNumRows(); mRow++ ){
            m_VisibleRows.push_back( mRow );
        }
    } else {
        const IRowFilter& filter = *GetFilter();

        for( int mRow = 0; mRow < model.GetNumRows(); mRow++ ){
            if( filter( CwxTableRow( model, mRow ) ) ){
                m_VisibleRows.push_back( mRow );
            }
        }
    }

    SetItemCount( (int)m_VisibleRows.size() );
    Refresh();

    //? restore selection
    /*
    if( beenSelected ){
        for( size_t vRow = 0; vRow < m_VisibleRows.size(); vRow++ ){
            if( selected[m_VisibleRows[vRow]] ){
                selModel.AddSelectionSpan( vRow, vRow );
            }
        }
    }
    */

    if( preserve_state ){
        // restore focus
        x_RestoreViewState( &table_state );
    }

    m_FilterInvalid = false;
}


CIRef<IRowSorter> CwxTableListCtrl::GetSorter() const
{
    return m_Sorter;
}


void CwxTableListCtrl::SetSorter( CIRef<IRowSorter> sorter )
{
    if( m_Sorter != sorter ){
        ClearSortIcons();

        m_Sorter = sorter;
        m_Sorted = eAscSort;

        InvalidateSorting();

        //! m_SortedCols are not cleared!
    }
}

CIRef<IRowSorter> CwxTableListCtrl::RemoveSorter()
{
    CIRef<IRowSorter> rv = m_Sorter;

    SetSorter( null );
    m_SortedCols.clear();

    return rv;
}


CIRef<IRowSorter> CwxTableListCtrl::GetColumnSorterByIndex( int col_ix ) const
{
    if( m_Columns[col_ix].m_Sortable ){

        CIRef<IRowSorter> colSorter = m_Columns[col_ix].m_Sorter;
        if( colSorter != null ){
            return colSorter;
        }

        int model_ix = ColumnIndexToModel( col_ix );
        IwxVariantSorter* defSorter = GetDefaultSorter(
            GetModel()->GetColumnType( model_ix )
        );

        if( defSorter != NULL ){
            return CIRef<IRowSorter>( new CSorterByColumn( defSorter, model_ix ) );
        }
    }

    return null;
}


void CwxTableListCtrl::SetSorterByIndex( int col )
{
    m_SortedCols.clear();
    m_SortedCols.push_back( TSortedCol( col, eAscSort ) );

    SetSorterByColumns();
}

void CwxTableListCtrl::SetSorterByColumns()
{
    CIRef<IRowSorter> sorter;

    ITERATE( vector<TSortedCol>, sort_itr, m_SortedCols ){
        int col = sort_itr->first;
        ESortedState state = sort_itr->second;

        if( state == eNoSort ){
            continue;
        }

        CIRef<IRowSorter> col_sorter = GetColumnSorterByIndex( col );
        if( state == eDescSort ){
            col_sorter = new CReverseSorter( col_sorter );
        } 

        if( sorter.IsNull() ){
            sorter = col_sorter;
        } else {
            sorter = new CTandemSorter( sorter, col_sorter );
        }
    }

    SetSorter( sorter ); 

    UpdateSortIcons();
}

void CwxTableListCtrl::ClearSortIcons()
{
    //ITERATE( vector<TSortedCol>, sort_itr, m_SortedCols ){
    //    int col = sort_itr->first;

    for( int col = 0; col < GetColumnCount(); col++ ){
        wxListItem item;
        //? Why is align here? At least, it should not be by model ix.
        item.SetAlign( 
            GetAlignByType( GetModel()->GetColumnType( ColumnIndexToModel( col ) ) ) 
        );
        item.SetImage( NO_IMAGE_ID );
        SetColumn( col, item );
    }
}

void CwxTableListCtrl::UpdateSortIcons()
{
    ClearSortIcons();
    if( m_Sorted == eNoSort ) return;
    
    int ix = 0;
    ITERATE( vector<TSortedCol>, sort_itr, m_SortedCols ){
        int sorted_col = sort_itr->first;
        ESortedState sorted_state = sort_itr->second;

        wxListItem item;
        //? Why is align here? At least, it should not be by model ix.
        item.SetAlign( 
            GetAlignByType( GetModel()->GetColumnType( ColumnIndexToModel( sorted_col ) ) ) 
        );

        if( sorted_state == eNoSort ){
            item.SetImage( NO_IMAGE_ID );

        } else {
            int up_id = ix == 0 ? UP_ARROW_ID :( ix == 1 ? UP2_ARROW_ID : NO_IMAGE_ID ); 
            int down_id = ix == 0 ? DOWN_ARROW_ID :( ix == 1 ? DOWN2_ARROW_ID : NO_IMAGE_ID ); 
            ix++;

            if( sorted_state == eAscSort ){
                item.SetImage( m_Sorted == eAscSort ? up_id : down_id );
            } else {
                item.SetImage( m_Sorted == eAscSort ? down_id : up_id );
            }
        }

        if( !mf_ImageListInstalled ){
            SetImageList( &m_ImageList, wxIMAGE_LIST_SMALL );
            mf_ImageListInstalled = true;
        }

        SetColumn( sorted_col, item );
    }
}


void CwxTableListCtrl::ApplySorting( bool preserve_state )
{
    if( !m_SortInvalid ){
        return;
    }

    if( GetModel() == NULL || GetModel()->GetNumRows() == 0 ){
        return;
    }

    SViewState table_state;
    if( preserve_state ){
        // remember focus
        x_SaveViewState( &table_state );
    }

    //? remember selection
    /*
    _ASSERT( GetSelectionModel() != NULL );
    ISelectionModel& selModel = *GetSelectionModel();

    vector<bool> selected( GetModel()->GetNumRows() );

    bool beenSelected = !selModel.IsSelectionEmpty();
    if( beenSelected ){
        for( size_t row = 0; row < m_VisibleRows.size(); row++ ){
            if( selModel.IsSelectedIndex( row ) ){
                selected[m_VisibleRows[row]] = true;
            }
        }
        selModel.ClearSelection();
    }
    */

    if( GetSorter() == NULL ){
        m_Sorted = eNoSort;
    }

    switch( m_Sorted ){
    case eNoSort:
        std::stable_sort( m_VisibleRows.begin(), m_VisibleRows.end() );
        break;

    case eAscSort:
        if( m_PrepareSortHint ){
            std::stable_sort( m_VisibleRows.begin(), m_VisibleRows.end() );
            m_PrepareSortHint = false;
        }
        std::stable_sort(
            m_VisibleRows.begin(), m_VisibleRows.end(),
            CIndexSorter( *GetModel(), *GetSorter() )
        );
        break;

    case eDescSort:
        if( m_PrepareSortHint ){
            std::stable_sort( m_VisibleRows.begin(), m_VisibleRows.end() );
            m_PrepareSortHint = false;
        }
        std::stable_sort(
            m_VisibleRows.rbegin(), m_VisibleRows.rend(),
            CIndexSorter( *GetModel(), *GetSorter() )
        );
        break;
    }

    //? restore selection
    /*
    if( beenSelected ){
        for( size_t row = 0; row < m_VisibleRows.size(); row++ ){
            if( selected[m_VisibleRows[row]] ){
                selModel.AddSelectionSpan( row, row );
            }
        }
    }
    */

    /*
        Sort icons update goes to SetSorterByColumns() method
    */

    if( preserve_state ){
        // restore focus
        x_RestoreViewState( &table_state );
    }

    m_SortInvalid = false;
}

void CwxTableListCtrl::SetSortedState( ESortedState sorted )
{
    m_Sorted = sorted;

    InvalidateSorting();
}


void CwxTableListCtrl::ShiftSorting( int col )
{
    if( col < 0 ){
        if( GetSorter() ){
            switch( m_Sorted ){
            case eNoSort:
                m_Sorted = eAscSort;
                break;
            case eAscSort:
                m_Sorted = eDescSort;
                break;
            case eDescSort:
                m_Sorted = eNoSort;
                break;
            }

            InvalidateSorting();
        }
    } else {
        size_t found = 0;
        ITERATE( vector<TSortedCol>, found_itr, m_SortedCols ){
            if( found_itr->first == col ){
                break;
            }
            found++;
        }

        if( found < m_SortedCols.size() ){
            switch( m_SortedCols[found].second ){
            case eNoSort:
                m_SortedCols[found].second = eAscSort;
                break;
            case eAscSort:
                m_SortedCols[found].second = eDescSort;
                break;
            case eDescSort:
                m_SortedCols[found].second = eNoSort;
                m_PrepareSortHint = true;
                break;
            }
        } else {
            m_SortedCols.push_back( TSortedCol( col, eAscSort ) );
        }

        SetSorterByColumns();
    }
}

void CwxTableListCtrl::OnSelectionChanged( wxListEvent& event )
{
    x_OnSelectionChanged( 
        event.GetEventType() == wxEVT_COMMAND_LIST_ITEM_SELECTED,
        event.GetIndex()
    );

    event.Skip();
}

void CwxTableListCtrl::x_OnSelectionChanged( bool on, int start_row, int end_row )
{
    // Default implementation does nothing, effectively skipping the event.
}


void CwxTableListCtrl::OnColumnClick( wxListEvent &anEvt )
{
    int click_col = anEvt.GetColumn();

    if( !m_Columns[click_col].m_Sortable ) return;


#   if defined(NCBI_OS_DARWIN)
#       define CTRL_MODIFIER WXK_COMMAND
#   else
#       define CTRL_MODIFIER WXK_CONTROL
#   endif

    if( ::wxGetKeyState( CTRL_MODIFIER ) ){
        // Secondary sort
        if( m_SortedCols.empty() ){
            SetSorterByIndex( click_col );

        } else if( m_SortedCols[0].first == click_col ){
            ShiftSorting( click_col );

        } else if( m_SortedCols.size() > 1 ){
            if( m_SortedCols[1].first == click_col ){
                ShiftSorting( click_col );

            } else {
                m_SortedCols[1] = TSortedCol( click_col, eAscSort );

                SetSorterByColumns();
            }
        } else {
            m_SortedCols.push_back( TSortedCol( click_col, eAscSort ) );

            SetSorterByColumns();
        }

    } else {
        // Primary sort
        if( m_SortedCols.empty() || m_SortedCols[0].first != click_col ){
            SetSorterByIndex( click_col );

        } else {
            ShiftSorting( click_col );
        }
    }

    ApplySorting();

    Refresh();
}

void CwxTableListCtrl::OnColumnRightClick( wxListEvent &anEvt )
{
    if( GetModel() == NULL ){
        return;
    }

    wxPoint point = anEvt.GetPoint();
    //point = ScreenToClient( point );

    int col = ShowHeaderContextMenu( point );
    if( col < 0 ){
        return;
    }

    if( col < MAX_COLNAMES_SHOWED_IN_POPUP ){
        if( IsColumnVisibleByIndex( col ) ){
            HideColumn( col );
        } else {
            ShowColumn( col );
        }

        Refresh();

    } else {
        col -= MAX_COLNAMES_SHOWED_IN_POPUP;

        if( col == HEADER_CONTEXT_MORE ){
            // ShowMoreVisibleColumnsDlg();
            CMoreTableColsDlg dlg( this );

            dlg.ShowModal();

        } else if( col == HEADER_CONTEXT_SORT ){
            // ShowCustomSortDlg();
        }
    }
}

void CwxTableListCtrl::OnColumnBeginDrag( wxListEvent &anEvt )
{
#if defined(__WXGTK__) || defined(__WXMAC__)
    // With wxGTK, columns are handled differently - we can safely ignore this
    // event
    anEvt.Skip();
#else
    if( anEvt.GetItem().GetWidth() == 0 ){
        anEvt.Veto();
    } else {
        anEvt.Skip();
    }
#endif
}

void CwxTableListCtrl::OnColumnEndDrag( wxListEvent &anEvt )
{
    int drag_col = anEvt.GetColumn();

#if defined(__WIN32__) && !defined(__WXUNIVERSAL__)

    if( anEvt.GetItem().GetWidth() == 0 ){
        HideColumn( drag_col );
    }
#endif

    if( !IsColumnVisibleByIndex( drag_col ) && GetColumnWidth( drag_col ) > 0 ){
        m_Columns[drag_col].m_Width = GetColumnWidth( drag_col );
        ShowColumn( drag_col );
    }


    Refresh();
    anEvt.Skip();
}

void CwxTableListCtrl::OnContextMenu( wxContextMenuEvent& anEvent )
{
    m_Timer.Stop();

    wxMenu* menu;

    void* data = anEvent.GetClientData();
    menu = data ? (wxMenu*)data : new wxMenu();

    AppendMenuItems( *menu );

    if( ShouldPropagateContextMenu() ){
        anEvent.SetClientData( menu );
        anEvent.Skip();
        return;
    }

    anEvent.SetClientData( NULL );

    CleanupSeparators( *menu );
    PopupMenu( menu );

    delete menu;
}

void CwxTableListCtrl::AppendMenuItems( wxMenu& aMenu )
{
    aMenu.Append( wxID_SEPARATOR, wxT("Edit") );
    CUICommandRegistry::GetInstance().AppendMenuItem( aMenu, wxID_COPY );
}

// this is used to pass the id of the selected item from the menu event handler
// to the main function itself
//
// it's ok to use a global here as there can be at most one popup menu shown at
// any time
static int gs_popupMenuSelection = wxID_NONE;

void CwxTableListCtrl::InternalOnPopupMenu(wxCommandEvent& event)
{
    // store the id in a global variable where we'll retrieve it from later
    gs_popupMenuSelection = event.GetId();
}

int CwxTableListCtrl::DoGetPopupMenuSelectionFromUser(wxMenu& menu, const wxPoint& pos )
{
    gs_popupMenuSelection = wxID_NONE;

    Connect(wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(CwxTableListCtrl::InternalOnPopupMenu),
            NULL,
            this);

    PopupMenu(&menu, pos);

    Disconnect(wxEVT_COMMAND_MENU_SELECTED,
               wxCommandEventHandler(CwxTableListCtrl::InternalOnPopupMenu),
               NULL,
               this);

    return gs_popupMenuSelection;
}

int CwxTableListCtrl::ShowHeaderContextMenu( const wxPoint& pos )
{
    if( GetModel() == NULL ){
        return wxID_NONE;
    }

    wxMenu menu;

    // menu.SetTitle( "Choose columns:" );
    for( int col = 0; col < GetModel()->GetNumColumns() && col < MAX_COLNAMES_SHOWED_IN_POPUP; col++ ){
        wxString col_name = GetColumnNameByIndex( col );
        menu.AppendCheckItem( wxID_HEADER_CONTEXT_MENU + col, col_name );

        if( IsColumnVisibleByIndex( col ) ){
            menu.Check( wxID_HEADER_CONTEXT_MENU + col, true );
        }
    }

    menu.AppendSeparator();

    menu.Append( wxID_HEADER_CONTEXT_MORE, wxT("Column Options...") );

    const int rc = DoGetPopupMenuSelectionFromUser( menu, pos );
    if( rc != wxID_NONE ){
        //wxLogMessage(wxT("You have selected column \"%i\""), rc - wxID_HEADER_CONTEXT_MENU );

        return rc - wxID_HEADER_CONTEXT_MENU;
    }

    return rc;
}

void CwxTableListCtrl::x_SaveViewState( SViewState* state )
{
    if( state ){
        for ( long item = -1;; ) {
            item = GetNextItem(item,
                               wxLIST_NEXT_ALL,
                               wxLIST_STATE_SELECTED);
            if ( item == -1 )
                break;
            state->m_Selected.push_back(RowVisibleToData((int)item));
        }

        state->m_FocusModelRow = RowVisibleToData( GetFocusRow() );

        int topVisibleItem = GetTopItem();
        state->m_TopModelRow = topVisibleItem >= 0 ? RowVisibleToData( topVisibleItem ) : -1;
    }
}

void CwxTableListCtrl::x_RestoreViewState( SViewState* state )
{
    if( state ){
        ClearSelection();

        for (size_t i = 0; i < state->m_Selected.size(); ++i) {
            int selectedItem = RowDataToVisible( state->m_Selected[i] );
            if (selectedItem >= 0) {
                SetItemState( selectedItem, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
            }
        }

        int indexFocusRow = RowDataToVisible( state->m_FocusModelRow );

        if( indexFocusRow != -1 ){
            SetFocusRow( indexFocusRow );
        }

        if( state->m_TopModelRow >= 0 ){
            int topVisibleItem = RowDataToVisible( state->m_TopModelRow );

            EnsureVisible( topVisibleItem );
        }
    }
}


IwxVariantSorter* CwxTableListCtrl::GetDefaultSorter( wxString aType ) const
{
    TSorterMap::iterator it = m_DefaultSorters.find( aType );
    if( it != m_DefaultSorters.end() ){
        return it->second;
    }

    return NULL;
}

void CwxTableListCtrl::SetDefaultSorter(  wxString aType, IwxVariantSorter* aSorter ){

    if( aSorter != NULL ){
        m_DefaultSorters[aType] = aSorter;
    } else {
        m_DefaultSorters.erase( aType );
    }
}

class CwxBoolSorter : public IwxVariantSorter
{
/****** PUBLIC DEFINITION SECTION ******/
public:

    virtual bool operator()( const wxVariant& aAny, const wxVariant& bAny ) const
    {
        return !aAny.GetBool() && bAny.GetBool();
    }
};

class CwxCharSorter : public IwxVariantSorter
{
/****** PUBLIC DEFINITION SECTION ******/
public:

    virtual bool operator()( const wxVariant& aAny, const wxVariant& bAny ) const
    {
        return aAny.GetChar() < bAny.GetChar();
    }
};

class CwxLongSorter : public IwxVariantSorter
{
/****** PUBLIC DEFINITION SECTION ******/
public:

    virtual bool operator()( const wxVariant& aAny, const wxVariant& bAny ) const
    {
        return aAny.GetLong() < bAny.GetLong();
    }
};

class CwxDoubleSorter : public IwxVariantSorter
{
/****** PUBLIC DEFINITION SECTION ******/
public:

    virtual bool operator()( const wxVariant& aAny, const wxVariant& bAny ) const
    {
        return aAny.GetDouble() < bAny.GetDouble();
    }
};

class CwxStringSorter : public IwxVariantSorter
{
/****** PUBLIC DEFINITION SECTION ******/
public:

    virtual bool operator()( const wxVariant& aAny, const wxVariant& bAny ) const
    {
        return aAny.GetString().Cmp( bAny.GetString() ) < 0;
    }
};

void CwxTableListCtrl::CreateDefaultSorters()
{
    // Default wxVariant values are:
    // bool, char, long, double, string, date, time, list, stringlist, void*.

    SetDefaultSorter( wxT("bool"), new CwxBoolSorter() );
    SetDefaultSorter( wxT("char"), new CwxCharSorter() );
    SetDefaultSorter( wxT("int"), new CwxLongSorter() );
    SetDefaultSorter( wxT("long"), new CwxLongSorter() );
    SetDefaultSorter( wxT("float"), new CwxDoubleSorter() );
    SetDefaultSorter( wxT("double"), new CwxDoubleSorter() );
    SetDefaultSorter( wxT("string"), new CwxStringSorter() );
}


void CwxTableListCtrl::TableChanged( const CwxTableModelNotice& aNotice )
{
    if( & aNotice.GetSource() != &*GetModel() ){
        return;
    }

    bool state_saved = false;
    SViewState table_state;

    if( aNotice.GetFirstRow() == CwxTableModelNotice::HEADER_ROW ){
        // It means that the whole model structure is changed, including data

        //! RemoveAllRows();
        RemoveAllColumns();

        //if( m_AutoCreateVisible ){
        // create default columns from data
        CreateAllColumnsFromModel();

        m_VisibleRows.clear();
        m_VisibleRows.reserve( GetModel()->GetNumRows() );
        for( int row = 0; row < GetModel()->GetNumRows(); row++ ){
            m_VisibleRows.push_back( row );
        }

        SetItemCount( (int)m_VisibleRows.size() );
        Refresh();

        // FireFocusCellChanged();

    } else if( aNotice.GetType() == CwxTableModelNotice::eAllDataChanged ){

        DeleteAllItems();

        m_VisibleRows.clear();
        m_VisibleRows.reserve( GetModel()->GetNumRows() );
        for( int row = 0; row < GetModel()->GetNumRows(); row++ ){
            m_VisibleRows.push_back( row );
        }

        SetItemCount( (int)m_VisibleRows.size() );
        Refresh();

        // FireFocusCellChanged();

    } else if( aNotice.GetType() == CwxTableModelNotice::eInsert ){

        x_SaveViewState( &table_state );
        state_saved = true;

        int mStart = aNotice.GetFirstRow();
        int mEnd = aNotice.GetLastRow();
        if( mEnd < 0 ){
            mEnd = mStart;
        }

        int mInserted = mEnd - mStart + 1;

        SetItemCount( GetItemCount() + mInserted );
        if( GetItemCount() ){
            RefreshItems( mStart, GetItemCount()-1 );
        }

        m_FilterInvalid = true;
        //! m_VisibleRows will be refreshed in ApplyFiltering()

        // FireCellsResized();

    } else if( aNotice.GetType() == CwxTableModelNotice::eDelete ){

        x_SaveViewState( &table_state );
        state_saved = true;

        int mStart = aNotice.GetFirstRow();
        int mEnd = aNotice.GetLastRow();
        if( mEnd < 0 ){
            mEnd = mStart;
        }

        int mDeleted = mEnd - mStart + 1;

        SetItemCount( GetItemCount() - mDeleted );
        if( GetItemCount() ){
            RefreshItems( mStart, GetItemCount()-1 );
        }

        m_FilterInvalid = true;
        //! m_VisibleRows will be refreshed in ApplyFiltering()

        // FireFocusCellChanged();

    } else if( aNotice.GetType() == CwxTableModelNotice::eUpdate ){

        int mStart = aNotice.GetFirstRow();
        int mEnd = aNotice.GetLastRow();

        if( mEnd < 0 ){
            RefreshItem( mStart );
        } else {
            RefreshItems( mStart, mEnd );
        }

        m_FilterInvalid = true;
        //! m_VisibleRows will be refreshed in ApplyFiltering()
    }

    // Now aNotice.GetType() == ITableModel::CTMNotice::eUpdate

    InvalidateSorting();

    ApplyFiltering( !state_saved );
    ApplySorting( !state_saved );

    if( state_saved ){
        x_RestoreViewState( &table_state );
    }
}

wxListColumnFormat CwxTableListCtrl::GetAlignByType( wxString aType )
{
    if(
        aType == wxT("short")
        || aType == wxT("int")
        || aType == wxT("long")
        || aType == wxT("unsigned char")
        || aType == wxT("float")
        || aType == wxT("double")
    ){
        return wxLIST_FORMAT_RIGHT;

    } else if(
        aType == wxT("char")
    ){
        return wxLIST_FORMAT_CENTER;

    } else if(
        aType == wxT("bool")
    ){
        return wxLIST_FORMAT_CENTER;

    } else {
        return wxLIST_FORMAT_LEFT;
    }
}

void CwxTableListCtrl::OnKeyDown( wxKeyEvent &anEvt )
{
    if( anEvt.GetKeyCode() == 'A' && anEvt.ControlDown() ){
        SelectAll();
    } else {
        anEvt.Skip();
    }
}


void CwxTableListCtrl::OnSelectAll( wxCommandEvent &anEvt )
{
    SelectAll();
}

void CwxTableListCtrl::OnCopy( wxCommandEvent &anEvt )
{
    x_OnCopy();
}

void CwxTableListCtrl::OnCopyUpdate( wxUpdateUIEvent &anEvt )
{
    anEvt.Enable( GetFirstSelected() != -1 );
}

void CwxTableListCtrl::x_OnCopy()
{
    wxString clip_str;

    int row = -1;
    bool first_row = true;
    while( (row = GetNextSelected( row )) != -1 ){

        wxString row_str;
        for( int col = 0; col < GetNumColumnsVisible(); col++ ){
            if( col ){
                row_str += ' ';
            }
            row_str += GetValueVisibleAt( row, col ).GetString();
        }

        if( first_row ){
            first_row = false;
        } else {
            clip_str += wxTextBuffer::GetEOL();
        }
        clip_str += row_str;
    }

    if( !clip_str.IsEmpty() ){
        CSimpleClipboard::Instance().Set( clip_str );
    }
}

void CwxTableListCtrl::OnDebugEvent( wxListEvent &anEvt )
{
    string name = "Debug";
    if( anEvt.GetKeyCode() == wxEVT_COMMAND_LIST_BEGIN_DRAG ){
        name = "BEGIN-DRAG";
    } else if( anEvt.GetKeyCode() == wxEVT_COMMAND_LIST_BEGIN_RDRAG ){
    } else if( anEvt.GetKeyCode() == wxEVT_COMMAND_LIST_BEGIN_LABEL_EDIT ){
    } else if( anEvt.GetKeyCode() == wxEVT_COMMAND_LIST_END_LABEL_EDIT ){
    } else if( anEvt.GetKeyCode() == wxEVT_COMMAND_LIST_DELETE_ITEM ){
    } else if( anEvt.GetKeyCode() == wxEVT_COMMAND_LIST_DELETE_ALL_ITEMS ){
    } else if( anEvt.GetKeyCode() == wxEVT_COMMAND_LIST_ITEM_SELECTED ){
    } else if( anEvt.GetKeyCode() == wxEVT_COMMAND_LIST_ITEM_DESELECTED ){
    } else if( anEvt.GetKeyCode() == wxEVT_COMMAND_LIST_KEY_DOWN ){
    } else if( anEvt.GetKeyCode() == wxEVT_COMMAND_LIST_INSERT_ITEM ){
    } else if( anEvt.GetKeyCode() == wxEVT_COMMAND_LIST_COL_CLICK ){
    } else if( anEvt.GetKeyCode() == wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK ){
    } else if( anEvt.GetKeyCode() == wxEVT_COMMAND_LIST_ITEM_MIDDLE_CLICK ){
    } else if( anEvt.GetKeyCode() == wxEVT_COMMAND_LIST_ITEM_ACTIVATED ){
    } else if( anEvt.GetKeyCode() == wxEVT_COMMAND_LIST_CACHE_HINT ){
    } else if( anEvt.GetKeyCode() == wxEVT_COMMAND_LIST_COL_RIGHT_CLICK ){
    } else if( anEvt.GetKeyCode() == wxEVT_COMMAND_LIST_COL_BEGIN_DRAG ){
        name = "COL-BEGIN-DRAG";
    } else if( anEvt.GetKeyCode() == wxEVT_COMMAND_LIST_COL_DRAGGING ){
    } else if( anEvt.GetKeyCode() == wxEVT_COMMAND_LIST_COL_END_DRAG ){
        name = "COL-END-DRAG";
    } else if( anEvt.GetKeyCode() == wxEVT_COMMAND_LIST_ITEM_FOCUSED ){
    }


    /*LOG_POST(
        name << ": pt(" << anEvt.GetPoint().x << ", " << anEvt.GetPoint().y
        << ", rc(" << anEvt.GetIndex() << ", " << anEvt.GetColumn()
        << ")"
    );*/

    anEvt.Skip();
}

void CwxTableListCtrl::OnLeftDown( wxMouseEvent &anEvt )
{
    anEvt.Skip();
    m_CurrentCol = -1;    
    m_CurrentRow = -1;
    return;
}

int CwxTableListCtrl::x_FindRow(const wxPoint &pt) const
{
    long low = 0;
    int high = GetItemCount() - 1;

    while (low <= high) {
        int mid = (low + high) / 2;

        wxRect rect;
        GetItemRect(mid, rect);

        if (rect.Contains(pt))
            return mid;

        if (pt.y < rect.y)
            high = mid - 1;
        else
            low = mid + 1;
    }
    return wxNOT_FOUND;
}

int CwxTableListCtrl::x_FindColumn(const wxPoint &pt) const
{
    int total_width = 0;
    m_TipStartX = 0;
    m_TipEndX = 0;
    // Get index of column mouse pointer is in:
    for (int i = 0; i < GetColumnCount(); ++i) {
        total_width += this->GetColumnWidth(i);
        m_TipStartX = m_TipEndX;
        m_TipEndX = total_width;
        if (pt.x < total_width) {
            return i;
        }
    }
    return wxNOT_FOUND;
}

void CwxTableListCtrl::OnMouseMove(wxMouseEvent &event)
{
// #if defined(__WXGTK__) || defined(__WXMAC__)  -- Disable for mac until probelm with disappearing text is solved (GB-6680)
#if defined(__WXGTK__) 
    if (event.Dragging()) {
        event.Skip();
        return;
    }

    // Get mouse position in window coords
    wxPoint pt = ScreenToClient(wxGetMousePosition());

    m_CurrentCol = x_FindColumn(pt);
    m_CurrentRow = x_FindRow(pt);

    x_ShowTooltip(false);

    m_Timer.Stop();

    if (wxNOT_FOUND != m_CurrentCol  && wxNOT_FOUND != m_CurrentRow) 
        m_Timer.Start(500, wxTIMER_ONE_SHOT);
#endif    

    event.Skip();
}

void CwxTableListCtrl::OnTimer(wxTimerEvent& WXUNUSED(event))
{
    if (m_Timer.IsRunning())
        return;

    x_ShowTooltip(true);
}

void CwxTableListCtrl::x_ShowTooltip(bool show)
{
    static wxTipWindow* s_tipWindow = nullptr;

    if (s_tipWindow)
    {
        s_tipWindow->SetTipWindowPtr(nullptr);
        s_tipWindow->Close();
        s_tipWindow = nullptr;
    }

    if (!show)
        return;

    if (m_CurrentCol < 0 || m_CurrentRow < 0)
        return;

    wxPoint pt = wxGetMousePosition();
    if (!GetScreenRect().Contains(pt))
        return;

    m_ToolTipText = this->GetItemText(m_CurrentRow, ColumnIndexToVisible(m_CurrentCol));
    if (m_ToolTipText.IsEmpty())
        return;

    wxClientDC dc(this);
    int w = dc.GetTextExtent(m_ToolTipText).GetWidth();

    wxRect r;
    GetItemRect(m_CurrentRow, r);
    // Convert from window to screen coordinates
    r.SetPosition(ClientToScreen(r.GetPosition()));

#if defined(__WXGTK__)
    s_tipWindow = new wxTipWindow(this, m_ToolTipText, w, &s_tipWindow, &r);
#elif defined(__WXMAC__)
    // The nullptr prevents double destruction (wxWidgets issue on MacOS)
    s_tipWindow = new wxTipWindow(nullptr, m_ToolTipText, w, &s_tipWindow, &r);
#endif
}

void CwxTableListCtrl::OnPaint( wxPaintEvent& anEvt )
{
#   ifdef __WXMSW__	

	/*
	if( 
		InReportView() && HasFlag( wxLC_HRULES ) && GetItemCount() > 0
		&& GetTopItem() >= GetItemCount() 
	){
		long style = GetWindowStyle();
		wxControl::SetWindowStyle( style ^ wxLC_HRULES );

		wxListCtrl::OnPaint( anEvt );

		wxControl::SetWindowStyle( style );
		ERR_POST( Warning << "ListView_GetTopIndex() bug workaround is in action!!" );

	} else {
		anEvt.Skip();
	}
	return;
	*/

	// Here is a direct copy from wxListCtrl::OnPaint() with additional 
	// check of the result of GetTopItem() (see ERR_POST around)
	

	const int itemCount = GetItemCount();
	const bool drawHRules = HasFlag(wxLC_HRULES);

	const bool drawVRules = HasFlag(wxLC_VRULES);

	if (!InReportView() || !(drawHRules || drawVRules) || !itemCount)
	{
		anEvt.Skip();
		return;
	}

	wxPaintDC dc(this);

	wxControl::OnPaint(anEvt);

	// Reset the device origin since it may have been set
	dc.SetDeviceOrigin(0, 0);

	wxPen pen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DLIGHT));
	dc.SetPen(pen);
	dc.SetBrush(* wxTRANSPARENT_BRUSH);

	wxSize clientSize = GetClientSize();
	wxRect itemRect;

	if (drawHRules)
	{
		long top = GetTopItem();
		if( top >= itemCount ){
			top = itemCount -1;
			ERR_POST( Warning << "ListView_GetTopIndex() bug workaround is in action!" );
		}
		for ( int i = top; i < top + GetCountPerPage() + 1; i++ )
		{
			if (GetItemRect(i, itemRect))
			{
				int cy = itemRect.GetTop();
				if (i != 0) // Don't draw the first one
				{
					dc.DrawLine(0, cy, clientSize.x, cy);
				}
				// Draw last line
				if (i == itemCount - 1)
				{
					cy = itemRect.GetBottom();
					dc.DrawLine(0, cy, clientSize.x, cy);
					break;
				}
			}
		}
	}

	if (drawVRules)
	{
		wxRect firstItemRect;
		GetItemRect(0, firstItemRect);

		if (GetItemRect(itemCount - 1, itemRect))
		{
			// this is a fix for bug 673394: erase the pixels which we would
			// otherwise leave on the screen
			static const int gap = 2;
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.SetBrush(wxBrush(GetBackgroundColour()));
			dc.DrawRectangle(0, firstItemRect.GetY() - gap,
				clientSize.GetWidth(), gap);

			dc.SetPen(pen);
			dc.SetBrush(*wxTRANSPARENT_BRUSH);

			const int numCols = GetColumnCount();
			wxVector<int> indexArray(numCols);
			if ( !ListView_GetColumnOrderArray(GetHwnd(),
				numCols,
				&indexArray[0]) )
			{
				wxFAIL_MSG( wxT("invalid column index array in OnPaint()") );
				return;
			}

			int x = itemRect.GetX();
			for (int col = 0; col < numCols; col++)
			{
				int colWidth = GetColumnWidth(indexArray[col]);
				x += colWidth ;
				dc.DrawLine(x-1, firstItemRect.GetY() - gap,
					x-1, itemRect.GetBottom());
			}
		}
	}




#	else
	anEvt.Skip();
#	endif
}


/****** Virtual mode implementation ****************/

wxString CwxTableListCtrl::OnGetItemText( long aRow, long aCol ) const
{
    if( !m_Columns[aCol].m_Visible ){
        return wxString();
    }

    int row = RowVisibleToData( aRow );
    int col = ColumnIndexToModel( aCol );

    wxVariant val = GetModel()->GetValueAt( row, col );

    if( val.GetType() == wxT("bool") ){
        return wxString();
    }


    const IwxStringFormat* formatter = m_Columns[aCol].m_Formatter;
    if (formatter)
        return formatter->Format(val);

    return val.MakeString();
}

int CwxTableListCtrl::OnGetItemImage( long WXUNUSED(aRow) ) const
{
    return -1;
}

int CwxTableListCtrl::OnGetItemColumnImage( long aRow, long aCol ) const
{
    if( !m_Columns[aCol].m_Visible ){
        return -1;
    }

    int row = RowVisibleToData( aRow );
    int col = ColumnIndexToModel( aCol );

    wxVariant val =  GetModel()->GetValueAt( row, col );

    int ix = -1;

    if( val.GetType() == wxT("bool") ){
        ix = val.GetBool() ? CHECKED_ICON_ID : UNCHECKED_ICON_ID;

    } else {

        IwxDecoratedTableModel* decor_model =
            dynamic_cast<IwxDecoratedTableModel*>(GetModel());
        if( decor_model != NULL /*&& decor_model->GetImageList() != NULL*/ ){

            wxString alias = decor_model->GetImageAliasAt( row, col );
            if( !alias.empty() ){
                TAliasMap::iterator iter = m_AliasToIndex.find( alias );
                if( iter != m_AliasToIndex.end() ){
                    ix = iter->second;

                } else {
                    wxBitmap bmp = wxArtProvider::GetBitmap( alias );
                    if( bmp.IsOk() ){
                        ix = m_ImageList.Add( bmp );
                        m_AliasToIndex[alias] = ix;
                    }
                }
            }
        }
    }

    if( ix != -1 ){
        if( !mf_ImageListInstalled ){
            CwxTableListCtrl* nonconst_this = const_cast<CwxTableListCtrl*>( this );
            nonconst_this->SetImageList( &m_ImageList, wxIMAGE_LIST_SMALL );

            mf_ImageListInstalled = true;
        }
    }

    return ix;
}

wxListItemAttr* CwxTableListCtrl::OnGetItemAttr( long aRow ) const
{
    static wxListItemAttr s_attr;

    IwxDecoratedTableModel* decor_model =
        dynamic_cast<IwxDecoratedTableModel*>(GetModel());
    if( decor_model != NULL ){

        int row = RowVisibleToData( aRow );
        wxColor color = decor_model->GetBackgroundColor( row );
        if( color.IsOk() ){
            s_attr.SetBackgroundColour( color );
            return &s_attr;
        }
    }

    if( !m_2ndBackColor.IsOk() ){
        wxColor fore = GetTextColour();
        wxColor back = const_cast<CwxTableListCtrl*>(this)->GetBackgroundColour();

        float weight = 0.95f;

        #define uch unsigned char

        uch r = (uch)(weight*back.Red() + (1-weight)*fore.Red());
        uch g = (uch)(weight*back.Green() + (1-weight)*fore.Green());
        uch b = (uch)(weight*back.Blue() + (1-weight)*fore.Blue());

        #undef uch

        const_cast<wxColor&>(m_2ndBackColor).Set( r, g, b );
    }

    if( aRow%2 != 0 ){
        s_attr.SetBackgroundColour( m_2ndBackColor );
        return &s_attr;

    } else {
        return NULL;
    }
}

bool CwxTableListCtrl::IsCellSelectedAt( int row, int WXUNUSED(col) ) const
{
    return IsSelected( row );
}

bool CwxTableListCtrl::ShowHorzLines() const
{
    return (GetWindowStyleFlag() & wxLC_HRULES) != 0;
}

void CwxTableListCtrl::SetShowHorzLines( bool show )
{
    SetSingleStyle( wxLC_HRULES, show );
}

// We save the following information
/// - a set of visible and invisible columns
/// - columns widths
/// - sort order

static const char* kColNumTag = "Count";
static const char* kColNumVisTag = "Vis-Count";
static const char* kColNumHidTag = "Hid-Count";
static const char* kColNumModTag = "Mod-Count";
static const char* kColumnTag = "Column";
static const char* kColModelName = "Name";
static const char* kColVisOrder = "Vis-Order";
static const char* kColModIndex = "Mod-Index";
static const char* kColumnWidth = "Width";
static const char* kColumnShown = "Shown";
static const char* kSortedGlobal = "Sorted-Global";
static const char* kSortedColumn = "Sorted-Col";
static const char* kSortedOrder = "Sorted-Asc";


static string GetColumnSectionName(int ix)
{
    return kColumnTag + string("_") + NStr::IntToString(ix);
}

void CwxTableListCtrl::SaveTableSettings(CRegistryWriteView& view, bool saveSorting) const
{
    string error;
    try {
        IwxTableModel* model = GetModel();
        if (model == NULL){
            //_ASSERT( false );

            NCBI_THROW(CException, eUnknown, "Table model is not present");
        }

        int total_num = GetColumnCount();
        view.Set(kColNumTag, total_num);

        int num_vis = GetNumColumnsVisible();
        int num_hid = GetNumColumnsHidden();
        int num_mod = GetModel()->GetNumColumns();

        _ASSERT(total_num == num_vis + num_hid);

        view.Set(kColNumVisTag, num_vis);
        view.Set(kColNumHidTag, num_hid);
        view.Set(kColNumModTag, num_mod);

        if (saveSorting && !GetSortedCols().empty()){
            CwxTableListCtrl::ESortedState sorted_global = GetSortedState();
            if (sorted_global != CwxTableListCtrl::eNoSort){
                view.Set(kSortedGlobal, sorted_global == CwxTableListCtrl::eAscSort);
            }
            else {
                view.DeleteField(kSortedGlobal);
            }

            vector<int> sorted_cols;
            vector<int> sorted_orders;

            ITERATE(vector<CwxTableListCtrl::TSortedCol>, coltr, GetSortedCols()){
                sorted_cols.push_back(coltr->first);
                sorted_orders.push_back((int)coltr->second);
            }

            view.Set(kSortedColumn, sorted_cols);
            view.Set(kSortedOrder, sorted_orders);

        }
        else {
            view.DeleteField(kSortedGlobal);
            view.DeleteField(kSortedColumn);
            view.DeleteField(kSortedOrder);
        }

        if (GetColumnCount() > 0) {
            wxArrayInt order_array = GetColumnsOrder();

            for (int ix = 0; ix < total_num; ix++){
                const int mod_ix = ColumnIndexToModel(ix);
                const bool shown = IsColumnVisibleByIndex(ix);

                string name_str = ToStdString(GetColumnNameByModel(mod_ix));
                _ASSERT(!name_str.empty());

                int width = shown ? GetColumnWidth(ix) : m_Columns[ix].m_Width;
                _ASSERT(width > 0);

                string section = GetColumnSectionName(ix);

                view.Set(CGuiRegistryUtil::MakeKey(section, kColModelName), name_str);
                view.Set(CGuiRegistryUtil::MakeKey(section, kColModIndex), mod_ix);
                view.Set(CGuiRegistryUtil::MakeKey(section, kColVisOrder), order_array[ix]);
                view.Set(CGuiRegistryUtil::MakeKey(section, kColumnShown), shown);
                view.Set(CGuiRegistryUtil::MakeKey(section, kColumnWidth), width);
            }
        }

        return;

    }
    catch (CException& e){
        error = e.GetMsg();

    }
    catch (std::exception& e){
        error = e.what();

    }

    if (!error.empty()){
        ERR_POST("Error saving table settings " << error);
    }
}


void CwxTableListCtrl::LoadTableSettings(const CRegistryReadView& view, bool byName)
{
    string error;
    try {
        IwxTableModel* model = GetModel();
        if (model == NULL){
            // _ASSERT( false );
            NCBI_THROW(CException, eUnknown, "Table model is not present");
        }

        // read configuration
        int total_num = view.GetInt(kColNumTag, -1);
        if (total_num == -1){
            //NCBI_THROW( CException, eUnknown, "No settings for the table" );
            return;

        }
        else if (total_num == 0){
            NCBI_THROW(CException, eUnknown, "Wrong settings for the table");
        }

        int num_vis = view.GetInt(kColNumVisTag, -1);
        int num_hid = view.GetInt(kColNumHidTag, -1);
        //int num_mod = view.GetInt( kColNumModTag, -1 );

        if (total_num != num_vis + num_hid){
            // file is inconsistent
            NCBI_THROW(CException, eUnknown, "Inconsistent info - column counts are wrong");
        }



        vector<bool> mod_vec(GetModel()->GetNumColumns(), false);

        wxArrayInt order_array;
        order_array.SetCount(total_num, -1);

        for (int ix = 0; ix < total_num; ix++){
            string section = GetColumnSectionName(ix);

            string name_str = view.GetString(CGuiRegistryUtil::MakeKey(section, kColModelName));
            int mod_ix = view.GetInt(CGuiRegistryUtil::MakeKey(section, kColModIndex));
            int ord_ix = view.GetInt(CGuiRegistryUtil::MakeKey(section, kColVisOrder));
            bool shown = view.GetBool(CGuiRegistryUtil::MakeKey(section, string(kColumnShown)));
            int width = view.GetInt(CGuiRegistryUtil::MakeKey(section, kColumnWidth));

            // Match columns by name first, then by index
            // Find column in visibles first, then in hidden, and vice versa

            int new_mod = GetModelIndexByName(ToWxString(name_str));
            if (new_mod == -1){
                LOG_POST(Warning << "No model column with such a name " << name_str);

                if (byName
                    || mod_ix >= GetModel()->GetNumColumns()
                    ){
                    continue;
                }

            }
            else if (new_mod != mod_ix){
                LOG_POST(Warning << "Model index is different for the name " << name_str);

                mod_ix = new_mod;
            }
            mod_vec[mod_ix] = true;

            int cur_ix = ColumnModelToIndex(mod_ix);
            if (cur_ix == -1){
                cur_ix = GetColumnCount();
                AddColumn(mod_ix);
            }

            m_Columns[cur_ix].m_Visible = true;
            SetColumnWidth(cur_ix, width);

            if (!shown){
                RemoveColumn(cur_ix, true);
            }

            if (!byName){
                if (ix < GetColumnCount()){
                    if (cur_ix != ix){
                        MoveColumn(cur_ix, ix);
                        cur_ix = ix;
                    }
                }

                //??? that's strange
                order_array.SetCount(cur_ix + 1, -1);
                order_array[cur_ix] = ord_ix;
            }
        }

        if (!byName){
            for (int mod_ix = 0; mod_ix < GetModel()->GetNumColumns(); mod_ix++){
                if (mod_vec[mod_ix]){ continue; }

                int real_cur_ix = ColumnModelToIndex(mod_ix);
                if (real_cur_ix == -1){ continue; }

                int cur_ix;

                if (mod_ix == 0){
                    cur_ix = 0;

                }
                else {
                    cur_ix = ColumnModelToIndex(mod_ix - 1) + 1;
                }

                if (cur_ix == real_cur_ix){ continue; }

                MoveColumn(real_cur_ix, cur_ix);

                //??? that's strange
                //order_array.SetCount( cur_ix+1, -1 );
                //order_array[cur_ix] = ord_ix;
            }
        }

        if (!byName && GetColumnCount() == order_array.GetCount()){
            SetColumnsOrder(order_array);
        }

        if (view.HasField(kSortedGlobal)){
            ResetSorting();

            bool sorted_global = view.GetBool(kSortedGlobal, true);
            SetSortedState(
                sorted_global ? CwxTableListCtrl::eAscSort : CwxTableListCtrl::eDescSort
                );

            vector<int> sorted_cols;
            vector<int> sorted_orders;

            view.GetIntVec(kSortedColumn, sorted_cols);
            view.GetIntVec(kSortedOrder, sorted_orders);

            size_t cols_num = sorted_cols.size();
            if (sorted_orders.size() < cols_num){
                cols_num = sorted_orders.size();
            }

            for (size_t ix = 0; ix < cols_num; ix++){
                int sort_ix = sorted_cols[ix];
                if (sort_ix >= 0 && sort_ix < GetColumnCount()){
                    AddSorterIndex(
                        sort_ix, (CwxTableListCtrl::ESortedState)sorted_orders[ix]
                        );
                }
            }

            SetSorterByColumns();

            ApplySorting(false);
        }

        Refresh();

    }
    catch (CException& e){
        error = e.GetMsg();

    }
    catch (std::exception& e){
        error = e.what();

    }

    if (!error.empty()){
        ERR_POST("Error loading table settings: " << error);
    }
}

void CwxTableListCtrl::ExportTableControlToCSV()
{
    CwxCSVExportDlg dlg(this);
    dlg.SetRegistryPath("Dialogs.ExportTableControlToCSV");

    vector<wxString> selectedColumns;
    const int colNum = GetNumColumnsVisible();
    for (int i = 0; i < colNum; ++i) {
        selectedColumns.push_back(GetColumnNameVisible(i));
    }
    dlg.SetColumnsList(selectedColumns);
    dlg.Layout();
    dlg.Fit();
    dlg.Refresh();

    if (dlg.ShowModal() == wxID_OK) {
        wxString fileName = dlg.GetFileName();
        if (fileName.empty())
            return;

        auto_ptr<CNcbiOstream> os;
        os.reset(new CNcbiOfstream(fileName.fn_str(), IOS_BASE::out));

        if (os.get() == NULL){
            NCBI_THROW(
                CException, eUnknown,
                "File is not accessible"
                );
        }

        bool with_headers = dlg.GetWithHeaders();
        bool selected_only = dlg.GetSelectedOnly();
        vector<int> cols;
        dlg.GetSelectedColumns(cols);

        CCSVExporter exporter(*os, ',', '"');

        if (with_headers){
            if (cols.size() != GetNumColumnsVisible()) {
                for (size_t i = 0; i < cols.size(); ++i){
                    wxString value = GetColumnNameVisible(cols[i]);
                    exporter.Field(value);
                }
            }
            else {
                for (int i = 0; i < GetNumColumnsVisible(); ++i){
                    wxString value = GetColumnNameVisible(i);
                    exporter.Field(value);
                }
            }
            exporter.NewRow();
        }

        int flags = selected_only ? wxLIST_STATE_SELECTED : wxLIST_STATE_DONTCARE;
        int item = -1;
        while ((item = GetNextItem(item, wxLIST_NEXT_ALL, flags)) != -1){
            if (cols.size() != GetNumColumnsVisible()) {
                for (size_t i = 0; i < cols.size(); ++i){
                    wxString value;
                    GetValueVisibleAt(item, cols[i]).Convert(&value);
                    exporter.Field(value);
                }
            }
            else {
                for (int i = 0; i < GetNumColumnsVisible(); ++i){
                    wxString value;
                    GetValueVisibleAt(item, i).Convert(&value);
                    exporter.Field(value);
                }
            }
            exporter.NewRow();
        }
    }
}

CTableListColumn::CTableListColumn( int aModelIndex ) 
    : m_ModelIx( aModelIndex )
    , m_Width( -1 )
    , m_Visible( true )
    , m_HidingAllowed( true )
    , m_Resizable( true )
    , m_Sortable( true )
{

}

END_NCBI_SCOPE

