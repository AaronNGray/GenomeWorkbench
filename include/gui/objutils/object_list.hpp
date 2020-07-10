#ifndef GUI_WIDGETS_OBJECT_LIST___OBJECT_LIST__HPP
#define GUI_WIDGETS_OBJECT_LIST___OBJECT_LIST__HPP

/*  $Id: object_list.hpp 29715 2014-02-03 14:08:58Z voronov $
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
 * Authors: Andrey Yazhuk, Yury Voronov
 *
 * File Description:
 *
 */

#include <gui/gui_export.h>
#include <corelib/ncbiobj.hpp>

#include <objmgr/scope.hpp>

#include <gui/objutils/objects.hpp>
#include <gui/objutils/label.hpp>

BEGIN_NCBI_SCOPE

/////////////////////////////////////////////////////////////////
/// CObjectListException
///
class NCBI_GUIOBJUTILS_EXPORT CObjectListException
    : EXCEPTION_VIRTUAL_BASE public CException
{
public:
    NCBI_EXCEPTION_DEFAULT(CObjectListException, CException);
};


/////////////////////////////////////////////////////////////////
/// CObjectList
/// Data structure representing a list of CObjects with associated
/// Scopes and other optional attributes (strings and numbers)
///
class NCBI_GUIOBJUTILS_EXPORT CObjectList
    : public CObject
{
public:
    enum EColumnType {
        eInvalid = -1,
        eObject,
        eScope,
        eString,
        eInteger,
        eDouble
    };
    struct _CColumn {

		_CColumn( EColumnType type, const string& name )
			: m_Name( name ), m_Type( type )
		{}

		//-------

        string m_Name;
        /// actually column can be of types eString, eInteger, eDouble
        EColumnType m_Type;

        /// actually it is a reference to wxArrayString, wxArrayInt, or wxArrayDouble
        CRef<CObject> m_ValuesRef;

        const vector<int>& IntVector() const;
		vector<int>& IntVector();
		void IntVector( int size );
		
		const vector<double>& DoubleVector() const;
		vector<double>& DoubleVector();
		void DoubleVector( int size );

		const vector<string>& StringVector() const;
        vector<string>& StringVector();
		void StringVector( int size );

		/*
		vector< CRef<CObject> >& GetVectorObject();
		const vector< CRef<CObject> >& GetVectorObject() const;
		*/
	};


public:
	CObjectList();
    CObjectList( const CObjectList& ob_list );

    virtual ~CObjectList();

    /// initializers
    void Init( TConstScopedObjects& objects );

    /// adds rows from the given list, the lists must have identical sets of columns
    void Append( const CObjectList& list );
    int AddRow( CObject* obj, objects::CScope* scope );

    int AddColumn( EColumnType type, const string& name, int col = -1 );
    void DeleteColumn( int col );

    /// access to columns (attribute, indexes)
    int GetNumColumns() const;
    EColumnType GetColumnType( int col ) const;
    const string& GetColumnName( int col ) const;
	
	int GetColStartIx() const { return m_StartIx; }
	void SetColStartIx( int ix ){ m_StartIx = ix; }
	void ClearColStartIx(){ m_StartIx = -1; }

    int GetNumRows() const;

    /// access to values (row, column)
    CObject* GetObject( int row );
    const CObject* GetObject( int row ) const;
    void SetObject( int row, CObject& obj );

    objects::CScope* GetScope( int row );
    const objects::CScope* GetScope(int row ) const;
    void SetScope( int row, objects::CScope& sc );

    const string& GetString( int col, int row ) const;
    void SetString( int col, int row, const string& val );

    int GetInteger( int col, int row ) const;
    void SetInteger( int col, int row, int val );

    double GetDouble( int col, int row ) const;
    void SetDouble( int col, int row, double val );

	/// access to a set of extra columns
	int GetNumObjectLabels() const { return (int)m_ExtraColName.size(); }
	string GetObjectLabelName( int col ) const;
	CLabel::ELabelType GetObjectLabelType( int col ) const;
	void SetObjectLabel( int col, const string& name, CLabel::ELabelType type );
	void AddObjectLabel( const string& name, CLabel::ELabelType type ){ 
		SetObjectLabel( -1, name, type ); 
	}
	void RemoveObjectLabel( int col );


	void ClearRows(); /// removes all data rows (i.e. objects)
	void ClearColumns() { m_Columns.clear(); } // remove all extra columns
	void ClearObjectLabels() {
		m_ExtraColName.clear();
		m_ExtraColType.clear();
	}
	void Clear(); /// removes all rows and non-standard columns

protected:
	void x_Init();
    void x_Clear();

	inline void x_AssertValidLabel( int col ) const;
    inline void x_AssertValidColumn( int col ) const;
    inline void x_AssertValidRow( int row ) const;
    inline void x_AssertValid( int col, int row ) const;

protected:
    /// all columns must be of the same size
    vector< CRef<CObject> > m_Objects; /// Objects column
    vector< CRef<objects::CScope> > m_Scopes;   /// Scopes column, may contain NULLs

    typedef vector<_CColumn> TColumns;
    TColumns m_Columns; /// all columns

protected:
	vector<string> m_ExtraColName;
	vector<CLabel::ELabelType> m_ExtraColType;
	int m_StartIx;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_OBJECT_LIST___OBJECT_LIST__HPP
