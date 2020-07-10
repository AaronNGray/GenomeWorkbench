/*  $Id: object_list.cpp 39100 2017-07-28 16:42:32Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/object_list.hpp>


template<class T> class CObjectVector
    : public ncbi::CObject, public std::vector<T>
{
public:
    typedef std::vector<T> TVectorBase;

    CObjectVector() {}

    CObjectVector( int _Count ) : TVectorBase( _Count ) {}

    CObjectVector( int _Count, const T& _Val ) : TVectorBase( _Count, _Val ) {}
};

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

const vector<int>& CObjectList::_CColumn::IntVector() const
{
	_ASSERT( m_Type == eInteger );

	return *dynamic_cast<const CObjectVector<int>*>( m_ValuesRef.GetPointer() );
}

vector<int>& CObjectList::_CColumn::IntVector()
{
    _ASSERT( m_Type == eInteger );

    return *dynamic_cast<CObjectVector<int>*>( m_ValuesRef.GetPointer() );
}

void CObjectList::_CColumn::IntVector( int size )
{
	_ASSERT( m_Type == eInteger );

	if( m_ValuesRef.IsNull() ){
		m_ValuesRef.Reset( new CObjectVector<int>( size ) );
	} else {
		dynamic_cast<CObjectVector<int>*>( m_ValuesRef.GetPointer() )->resize( size );
	}
}

const vector<double>& CObjectList::_CColumn::DoubleVector() const
{
	_ASSERT( m_Type == eDouble );

	return *dynamic_cast<const CObjectVector<double>*>( m_ValuesRef.GetPointer() );
}

vector<double>& CObjectList::_CColumn::DoubleVector()
{
	_ASSERT( m_Type == eDouble );

	return *dynamic_cast<CObjectVector<double>*>( m_ValuesRef.GetPointer() );
}

void CObjectList::_CColumn::DoubleVector( int size )
{
	_ASSERT( m_Type == eDouble );

	if( m_ValuesRef.IsNull() ){
		m_ValuesRef.Reset( new CObjectVector<double>( size ) );
	} else {
		dynamic_cast<CObjectVector<double>*>( m_ValuesRef.GetPointer() )->resize( size );
	}
}

const vector<string>& CObjectList::_CColumn::StringVector() const
{
	_ASSERT( m_Type == eString );

	return *dynamic_cast<const CObjectVector<string>*>( m_ValuesRef.GetPointer() );
}

vector<string>& CObjectList::_CColumn::StringVector()
{
	_ASSERT( m_Type == eString );

	return *dynamic_cast<CObjectVector<string>*>( m_ValuesRef.GetPointer() );
}

void CObjectList::_CColumn::StringVector( int size )
{
	_ASSERT( m_Type == eString );

	if( m_ValuesRef.IsNull() ){
		m_ValuesRef.Reset( new CObjectVector<string>( size ) );
	} else {
		dynamic_cast<CObjectVector<string>*>( m_ValuesRef.GetPointer() )->resize( size );
	}
}

/*
vector< CRef<CObject> >& CObjectList::_CColumn::GetVectorObject()
{
	_ASSERT( m_Type == eObject );

	return *dynamic_cast<CObjectVector< CRef<CObject> >*>( m_ValuesRef.GetPointer() );
}

const vector< CRef<CObject> >& CObjectList::_CColumn::GetVectorObject() const
{
	_ASSERT( m_Type == eObject );

	return *dynamic_cast<const CObjectVector< CRef<CObject> >*>( m_ValuesRef.GetPointer() );
}
*/

///////////////////////////////////////////////////////////////////////////////
/// CObjectList

void CObjectList::x_AssertValidLabel( int col ) const
{
	if( col < 0 || col >= GetNumObjectLabels() ){
		_ASSERT(false);
		NCBI_THROW( CObjectListException, eUnknown, "Invalid column index" );
	}
}

void CObjectList::x_AssertValidColumn( int col ) const
{
    if( col < 0 || col >= GetNumColumns() ){
        _ASSERT(false);
        NCBI_THROW( CObjectListException, eUnknown, "Invalid column index" );
    }
}

void CObjectList::x_AssertValidRow( int row ) const
{
    if( row < 0 || row >= GetNumRows() ){
        _ASSERT(false);
        NCBI_THROW( CObjectListException, eUnknown, "Invalid row index" );
    }
}

void CObjectList::x_AssertValid( int col, int row ) const
{
    x_AssertValidColumn( col );
    x_AssertValidRow( row );
}

CObjectList::CObjectList()
	: m_StartIx( -1 )
{
	x_Init();
}


CObjectList::CObjectList(const CObjectList& ob_list )
	: m_Objects( ob_list.m_Objects )
	, m_Scopes( ob_list.m_Scopes )
	, m_ExtraColName( ob_list.m_ExtraColName )
	, m_ExtraColType( ob_list.m_ExtraColType )
	, m_StartIx( ob_list.m_StartIx )
{
	// columns are deep copied

    ITERATE( TColumns, it, ob_list.m_Columns ){
        _CColumn new_col(it->m_Type, it->m_Name);
		switch (it->m_Type) {
		case eString:
            new_col.StringVector((int)it->StringVector().size());
			new_col.StringVector() = it->StringVector();
			break;

		case eInteger:
            new_col.IntVector((int)it->IntVector().size());
			new_col.IntVector() = it->IntVector();
			break;

		case eDouble:
            new_col.DoubleVector((int)it->DoubleVector().size());
			new_col.DoubleVector() = it->DoubleVector();
			break;

		default:
			_ASSERT(false);
			NCBI_THROW( CObjectListException, eUnknown, "Invalid column type" );
		}
        m_Columns.push_back( new_col );
    }
}

CObjectList::~CObjectList()
{
    //x_Clear();
}

void CObjectList::x_Clear()
{
	ClearColumns();
	ClearRows();

	ClearObjectLabels();
	ClearColStartIx();
}


/// initializers
void CObjectList::x_Init()
{
    x_Clear();

	AddObjectLabel( "Label", CLabel::eContent );
	AddObjectLabel( "Type", CLabel::eUserType );
	AddObjectLabel( "Subtype", CLabel::eUserSubtype );
	AddObjectLabel( "NCBI Type", CLabel::eType );
	AddObjectLabel( "Description", CLabel::eDescription );
}

void CObjectList::Init( TConstScopedObjects& objects )
{
    x_Init();

    size_t num_objects = objects.size();
    m_Objects.resize( num_objects );
    m_Scopes.resize( num_objects );

    for( size_t i = 0; i < num_objects; i++ ) {
        SConstScopedObject& sc = objects[i];
        m_Objects[i].Reset( const_cast<CObject*>(sc.object.GetPointer()) );
        m_Scopes[i].Reset( const_cast<CScope*>(sc.scope.GetPointer()) );
    }
}

void CObjectList::Clear()
{
    x_Clear();
}

void CObjectList::ClearRows()
{
    m_Scopes.clear();
    m_Objects.clear();

	NON_CONST_ITERATE( TColumns, it, m_Columns ){
		_CColumn& column = *it;

		switch( column.m_Type ){
		case eString:
			column.StringVector().clear();
			break;

		case eInteger:
			column.IntVector().clear();
			break;

		case eDouble:
			column.DoubleVector().clear();
			break;

		default:
			_ASSERT(false);
			NCBI_THROW( CObjectListException, eUnknown, "Invalid column type" );
		}
	}
	
}

template<class C> void AppendContainer( C& c1, const C& c2 )
{
    c1.insert( c1.end(), c2.begin(), c2.end() );
}


void CObjectList::Append( const CObjectList& list )
{
    _ASSERT( m_Columns.size() == list.m_Columns.size() );

    AppendContainer( m_Objects, list.m_Objects );
    AppendContainer( m_Scopes, list.m_Scopes );

    for(  size_t i = 0;  i < m_Columns.size(); i++) {
        _CColumn& col_1 = m_Columns[i];
        const _CColumn& col_2 = list.m_Columns[i];

        _ASSERT( col_1.m_Type == col_2.m_Type );

        switch( col_1.m_Type ){
        case eString:
            AppendContainer( col_1.StringVector(), col_2.StringVector() );
            break;
        case eInteger:
            AppendContainer( col_1.IntVector(), col_2.IntVector() );
            break;
        case eDouble:
            AppendContainer( col_1.DoubleVector(), col_2.DoubleVector() );
            break;
        default:
            _ASSERT( false ); // not supported
        }
    }
}

int CObjectList::AddColumn( EColumnType type, const string& name, int col )
{
	if( col != -1 ){
		x_AssertValidColumn( col );
    }

    int size = GetNumRows();

    _CColumn column( type, name );
    
    switch( type ){
    case eString:
		column.StringVector( size );
        break;

    case eInteger:
		column.IntVector( size );
        break;

    case eDouble:
        column.DoubleVector( size );
        break;

    default:
        _ASSERT(false);
        NCBI_THROW( CObjectListException, eUnknown, "Invalid column type" );
    }

    if( col == -1 ){
        m_Columns.push_back( column );
        return (int)m_Columns.size() - 1;

	} else {
        m_Columns.insert( m_Columns.begin() + col, column );
        return col;
    }
}


void CObjectList::DeleteColumn( int i_col )
{
    //bool valid_index = i_col > 1  &&  i_col < GetColumnsCount();
    //_VERIFY(valid_index);
    _ASSERT(false); // not implemented
}


int CObjectList::AddRow( CObject* obj, CScope* scope )
{
    m_Objects.emplace_back(obj);
    m_Scopes.emplace_back(scope);

    int num_rows = (int)m_Objects.size();
    NON_CONST_ITERATE( TColumns, it, m_Columns ){
        _CColumn& column = *it;

        switch( column.m_Type ){
        case eString:
            column.StringVector( num_rows );
            break;

        case eInteger:
            column.IntVector( num_rows );
            break;
        
        case eDouble:
            column.DoubleVector( num_rows );
            break;

        default:
            _ASSERT(false);
            NCBI_THROW( CObjectListException, eUnknown, "Invalid column type" );
        }
    }

    return num_rows - 1;
}


int CObjectList::GetNumColumns() const
{
    return (int)m_Columns.size();
}


CObjectList::EColumnType CObjectList::GetColumnType( int col ) const
{
    x_AssertValidColumn( col );

    const _CColumn& column = m_Columns[col];
    switch( column.m_Type ){
    case eString:
    case eInteger:
    case eDouble:
        return column.m_Type;

    default:
        break;
    }
    
    _ASSERT(false);
    return eInvalid;
}


const string& CObjectList::GetColumnName( int col ) const
{
    x_AssertValidColumn( col );

    return m_Columns[col].m_Name;
}


int CObjectList::GetNumRows() const
{
    return (int)m_Objects.size();
}


CObject* CObjectList::GetObject( int row )
{
    x_AssertValidRow( row );

    return m_Objects[row].GetPointer();
}


void CObjectList::SetObject( int row, CObject& obj )
{
    x_AssertValidRow( row );

    m_Objects[row].Reset( &obj );
}


const CObject* CObjectList::GetObject( int row ) const
{
    x_AssertValidRow( row );

    return m_Objects[row].GetPointer();
}


CScope* CObjectList::GetScope( int row )
{
    x_AssertValidRow( row );

    return m_Scopes[row].GetPointer();
}


const CScope* CObjectList::GetScope( int row ) const
{
    x_AssertValidRow( row );

    return m_Scopes[row].GetPointer();
}


void CObjectList::SetScope( int row, CScope& sc )
{
    x_AssertValidRow( row );

    m_Scopes[row].Reset( &sc );
}


const string& CObjectList::GetString( int col, int row ) const
{
    x_AssertValid( col, row );

    return m_Columns[col].StringVector()[row];
}


void CObjectList::SetString( int col, int row, const string& val )
{
    x_AssertValid( col, row );

    m_Columns[col].StringVector()[row] = val;
}


int CObjectList::GetInteger( int col, int row ) const
{
    x_AssertValid( col, row );

    return m_Columns[col].IntVector()[row];
}


void CObjectList::SetInteger( int col, int row, int val )
{
    x_AssertValid( col, row );

    m_Columns[col].IntVector()[row] = val;
}


double CObjectList::GetDouble(int col, int row) const
{
    x_AssertValid( col, row );

    return m_Columns[col].DoubleVector()[row];
}


void CObjectList::SetDouble(int col, int row, double val)
{
    x_AssertValid( col, row );

    m_Columns[col].DoubleVector()[row] = val;
}

string CObjectList::GetObjectLabelName( int col ) const 
{ 
	x_AssertValidLabel( col );

	return m_ExtraColName[col]; 
}

CLabel::ELabelType CObjectList::GetObjectLabelType( int col ) const 
{ 
	x_AssertValidLabel( col );

	return m_ExtraColType[col]; 
}


void CObjectList::SetObjectLabel( int col, const string& name, CLabel::ELabelType type )
{
	if( col < 0 ){
		col = GetNumObjectLabels();
	}

	m_ExtraColName.insert( m_ExtraColName.begin() + col, name );
	m_ExtraColType.insert( m_ExtraColType.begin() + col, type );
}

void CObjectList::RemoveObjectLabel( int col )
{
	if( col < 0 ){
		col = GetNumObjectLabels() -1;
	}

	m_ExtraColName.erase( m_ExtraColName.begin() + col );
	m_ExtraColType.erase( m_ExtraColType.begin() + col );
}

END_NCBI_SCOPE
