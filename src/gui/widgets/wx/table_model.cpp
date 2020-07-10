/*$Id: table_model.cpp 25496 2012-03-27 20:41:38Z kuznets $
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
 * Authors:  Yury Voronov, Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/table_model.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CwxTableModelNotice

CwxTableModelNotice::CwxTableModelNotice(IwxTableModel& source)
:   source(source),
    type(eAllDataChanged),
    firstRow(0),
    lastRow(0),
    column(ALL_COLUMNS)
{
}


CwxTableModelNotice::CwxTableModelNotice(IwxTableModel& source, int row)
:   source(source),
    type(eUpdate),
    firstRow(row),
    lastRow(row),
    column(ALL_COLUMNS)
{
}


CwxTableModelNotice::CwxTableModelNotice(IwxTableModel& source, int first_row, int last_row,
                                         int col, EType type)
:   source(source),
    type(type),
    firstRow(first_row),
    lastRow(last_row),
    column(col)
{
}


///////////////////////////////////////////////////////////////////////////////
/// CwxAbstractTableModel
CwxAbstractTableModel::CwxAbstractTableModel()
:   IwxDecoratedTableModel(),
    m_ListenerList()
{
}


wxString CwxAbstractTableModel::GetColumnName( int aColIx ) const {

    wxString name;
    for(; aColIx >= 0; aColIx = aColIx / 26 - 1 ){
        name = wxString::Format( wxT("%c"), (char)( (char)(aColIx % 26) + 'A' ) ) + name;
    }
    return name;
}


wxString CwxAbstractTableModel::GetColumnType( int aColIx ) const {

    return GetNumRows() > 0 ? GetValueAt( 0, aColIx ).GetType() : wxT("int");
}


void CwxAbstractTableModel::AddTMListener( IwxTableModelListener& aTMEar ){

    // we inserted listeners in reverse order
    // for easier notice propagation
    m_ListenerList.push_front( &aTMEar );
}


void CwxAbstractTableModel::RemoveTMListener( IwxTableModelListener& aTMEar ){

    m_ListenerList.remove( &aTMEar );
}


void CwxAbstractTableModel::x_FireDataChanged() {

    CwxTableModelNotice notice( *this );
    x_FireTableChanged( notice );
}


void CwxAbstractTableModel::x_FireStructureChanged() {

    CwxTableModelNotice notice(
        *this, CwxTableModelNotice::HEADER_ROW, CwxTableModelNotice::HEADER_ROW,
        CwxTableModelNotice::ALL_COLUMNS, CwxTableModelNotice::eStructureChanged
    );
    x_FireTableChanged( notice );
}


void CwxAbstractTableModel::x_FireRowsInserted( int aFirstRow, int aLastRow ){

    CwxTableModelNotice notice(
        *this, aFirstRow, aLastRow,
        CwxTableModelNotice::ALL_COLUMNS, CwxTableModelNotice::eInsert
    );
    x_FireTableChanged( notice );
}


void CwxAbstractTableModel::x_FireRowsUpdated( int aFirstRow, int aLastRow ){

    CwxTableModelNotice notice(
        *this, aFirstRow, aLastRow,
        CwxTableModelNotice::ALL_COLUMNS, CwxTableModelNotice::eUpdate
    );
    x_FireTableChanged( notice );
}


void CwxAbstractTableModel::x_FireRowsDeleted( int aFirstRow, int aLastRow ){

    CwxTableModelNotice notice(
        *this, aFirstRow, aLastRow,
        CwxTableModelNotice::ALL_COLUMNS, CwxTableModelNotice::eDelete
    );
    x_FireTableChanged( notice );
}


void CwxAbstractTableModel::x_FireCellUpdated( int aRow, int aCol ){

    CwxTableModelNotice notice( *this, aRow, aRow, aCol );
    x_FireTableChanged( notice );
}


void CwxAbstractTableModel::x_FireTableChanged( const CwxTableModelNotice& aNote ){

    NON_CONST_ITERATE( list<IwxTableModelListener*>, iter, m_ListenerList ){
        (*iter) -> TableChanged( aNote );
    }
}


///////////////////////////////////////////////////////////////////////////////
/// CwxTextTableModel
CTextTableModel::CTextTableModel(int n_cols, int n_rows)
{
    Init(n_cols, n_rows);
}


void CTextTableModel::Init(int n_cols, int n_rows)
{
    vector<wxString> names(n_cols, wxString() );
    Init(names, n_rows);
}

/*
void CTextTableModel::Init(const vector<string>& columns, int n_rows)
{
    ITERATE( vector<string>, col_it, columns ){
        m_ColumnNames.push_back( wxString::FromUTF8( col_it->c_str() ) );
    }
    
    m_Icons.clear();
    m_Columns.clear();
    m_Attachments.clear();

    size_t n_cols = m_ColumnNames.size();

    _ASSERT(n_cols > 0  &&  n_rows >= 0);

    m_Columns.resize(n_cols);

    SetNumRows( n_rows );
}
*/

void CTextTableModel::Init(const vector<wxString>& columns, int n_rows)
{
    m_ColumnNames.clear();
    ITERATE( vector<wxString>, col_it, columns ){
        m_ColumnNames.push_back( *col_it );
    }

    m_Icons.clear();
    m_Columns.clear();
    m_Attachments.clear();

    size_t n_cols = m_ColumnNames.size();

    _ASSERT(n_cols > 0  &&  n_rows >= 0);

    m_Columns.resize(n_cols);

    SetNumRows( n_rows );
}

void CTextTableModel::SetNumRows( int n_rows )
{
    m_Icons.resize(n_rows);
    m_Attachments.resize(n_rows);

    for( size_t i = 0; i < m_Columns.size();  i++ ) {
        m_Columns[i].resize( n_rows );
    }
}


/*
void CTextTableModel::SetIcon( int row, const string& icon_alias )
{
    _ASSERT(row >= 0  &&  row < (int) m_Icons.size());
    m_Icons[row] = wxString::FromUTF8( icon_alias.c_str() );
}
*/

void CTextTableModel::SetIcon( int row, const wxString& icon_alias )
{
    _ASSERT(row >= 0  &&  row < (int) m_Icons.size());
    m_Icons[row] = icon_alias;
}

/*
void CTextTableModel::SetStringValueAt( int row, int col, const string& value )
{
    _ASSERT(col >=0  &&  col < (int) m_Columns.size());

    TColumn& column = m_Columns[col];

    _ASSERT(row >= 0  &&  row < (int) column.size());

    column[row] = wxString::FromUTF8( value.c_str() );
}
*/

void CTextTableModel::SetStringValueAt( int row, int col, const wxString& value )
{
    _ASSERT(col >=0  &&  col < (int) m_Columns.size());

    TColumn& column = m_Columns[col];

    _ASSERT(row >= 0  &&  row < (int) column.size());

    column[row] = value;
}

void CTextTableModel::SetAttachment( int row, void* attachment )
{
    _ASSERT(row >= 0  &&  row < (int) m_Attachments.size());
    m_Attachments[row] = attachment;
}


int CTextTableModel::GetNumRows() const
{
    return (int) (m_Columns.empty() ? 0 : m_Columns[0].size());
}


int CTextTableModel::GetNumColumns() const
{
    return (int)m_Columns.size();
}


wxVariant CTextTableModel::GetValueAt( int row, int col ) const
{
    _ASSERT(col >=0  &&  col < (int) m_Columns.size());

    const TColumn& column = m_Columns[col];

    _ASSERT(row >= 0  &&  row < (int) column.size());

    return column[row];
}


wxString CTextTableModel::GetColumnName(int col) const
{
    _ASSERT(col >=0  &&  col < (int) m_ColumnNames.size());
    //return wxString::FromUTF8( m_ColumnNames[col].c_str() ); // ToWxString 
    return m_ColumnNames[col];
}


void* CTextTableModel::GetAttachment(int row)
{
    return m_Attachments[row];
}


void CTextTableModel::FireStructureChanged()
{
    x_FireStructureChanged();
}

void CTextTableModel::FireDataChanged()
{
    x_FireDataChanged();
}



void CTextTableModel::FireRowsUpdated( int first_row, int last_row )
{
    x_FireRowsUpdated( first_row, last_row );
}



wxString CTextTableModel::GetImageAliasAt( int row, int col ) const
{
    if( col == 0 ){
        _ASSERT(row >= 0  &&  row < (int) m_Icons.size());

        return m_Icons[row];
    } else {
        return wxString();
    }
}


END_NCBI_SCOPE
