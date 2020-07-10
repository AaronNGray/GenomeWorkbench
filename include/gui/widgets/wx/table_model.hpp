#ifndef GUI_WIDGETS_WX___TABLE_MODEL__HPP
#define GUI_WIDGETS_WX___TABLE_MODEL__HPP

/*  $Id: table_model.hpp 30889 2014-08-01 18:04:11Z ucko $
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
 * File Description: Data model interface for CTableControl widget.
 *
 */

#include <gui/widgets/wx/string_formatters.hpp>

#include <wx/colour.h>

BEGIN_NCBI_SCOPE


////////////////   wx part of definitions    ///////////////////////////

class IwxTableModelListener;

class IwxTableModel
{
/****** PUBLIC INTERFACE SECTION ******/
public:
    virtual ~IwxTableModel() {}

    /// Returns the number of rows in the model.  A CTableControl uses this method
    /// to determine how many rows it should display. This method should be quick,
    /// as it is called frequently during rendering.
    virtual int GetNumRows() const = 0;

    /// Returns the number of columns in the model.  A CTableControl uses this
    /// method to determine how many columns it should create and display
    /// by default.
    virtual int GetNumColumns() const = 0;

    /// Returns the name of the column at aColIx. This is used to initialize
    /// the table's column header name. This name does not need to be unique.
    virtual wxString GetColumnName(int col_idx) const = 0;

    ///
    virtual wxString GetColumnType(int col_idx) const = 0;

    virtual wxString GetTypeAt(int /*row*/, int col) const
    {
        return GetColumnType(col);
    }

    virtual wxVariant GetValueAt(int row, int col ) const = 0;
    virtual bool IsEditableAt(int /*i*/, int /*j*/ ) const { return false; };
    virtual void SetValueAt(int /*row*/, int /*col*/, const wxVariant& /*aValue*/ ) {}

    virtual void AddTMListener( IwxTableModelListener& aTMEar ) = 0;
    virtual void RemoveTMListener( IwxTableModelListener& aTMEar ) = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// CwxTableModelNotice
class NCBI_GUIWIDGETS_WX_EXPORT CwxTableModelNotice
{
public:
    static const int ALL_COLUMNS = -1;
    static const int HEADER_ROW = -1;

    enum EType {
        eInsert,
        eUpdate,
        eDelete,
        eAllDataChanged,
        eStructureChanged
    };

public:
    CwxTableModelNotice(IwxTableModel& source);
    CwxTableModelNotice(IwxTableModel& source, int row );
    CwxTableModelNotice(IwxTableModel& source, int first_row, int last_row,
                        int col = ALL_COLUMNS, EType type = eUpdate);

    IwxTableModel& GetSource() const { return source; }
    EType GetType() const   { return type; }
    int GetFirstRow() const { return firstRow; }
    int GetLastRow() const  { return lastRow; }
    int GetColumn() const   { return column; }

private:
    /// Table model should exist, notice is static and if stored,
    /// then on own storer risk
    IwxTableModel& source;
    EType type;
    int firstRow;
    int lastRow;
    int column;
};


///////////////////////////////////////////////////////////////////////////////
/// IwxTableModelListener
class IwxTableModelListener {
public:
    virtual ~IwxTableModelListener() {}

    virtual void TableChanged( const CwxTableModelNotice& aNotice ) = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// IwxDecoratedTableModel
class IwxDecoratedTableModel : public IwxTableModel
{
public:
    virtual ~IwxDecoratedTableModel() {}

    /// All the same methods of IwxTableModel, plus
    virtual wxString GetColumnTitle( int col_idx ) const
    {
        return GetColumnName( col_idx );
    }

    virtual CIRef<IwxStringFormat> GetColumnFormat( int /*col_idx*/ ) const
    {
        return CIRef<IwxStringFormat>();
    }

    virtual wxString GetImageAliasAt( int /*row*/, int /*col*/ ) const { return wxString(); }
    virtual wxString GetTooltipTextAt( int /*row*/, int /*col*/ ) const { return wxString(); }
    virtual wxColor GetBackgroundColor( int /*row*/ ) const { return wxColor(); }
};


///////////////////////////////////////////////////////////////////////////////
/// CwxAbstractTableModel - simple TableModel implementation.
/// In order to use it one should implement just three methods:
///     int GetNumRows();
///     int GetNumColumns();
///     wxVariant GetValueAt(int row, int col);
class NCBI_GUIWIDGETS_WX_EXPORT CwxAbstractTableModel : public IwxDecoratedTableModel
{
public:
    CwxAbstractTableModel();

    /// @name IwxTableModel interface implementation
    /// @{

    /// Returns the number of rows in the model.  A CTableControl uses this method
    /// to determine how many rows it should display. This method should be quick,
    /// as it is called frequently during rendering.
    virtual int GetNumRows() const = 0;

    /// Returns the number of columns in the model.  A CTableControl uses this
    /// method to determine how many columns it should create and display
    /// by default.
    virtual int GetNumColumns() const = 0;

    ///
    virtual wxVariant GetValueAt( int row, int col ) const = 0;

    /// Returns a default name for the column using spreadsheet
    /// conventions: A, B, C, ... Z, AA, AB, etc. If column cannot
    /// be found, returns an empty name. AnyType has a type
    /// of string.
    ///
    /// There is one static instance of string inside, so it is
    /// refilled every next call. It is enough for drawing purposes,
    /// but if someone needs more, (s)he should implement more.
    virtual wxString GetColumnName( int col_idx ) const;

    /// Tries to extract actual type from row 0 value if it exists.
    /// Otherwise returns typeid(int) [anyway it is of no use in abcence
    /// of data]
    virtual wxString GetColumnType( int col_idx ) const;

    virtual void AddTMListener( IwxTableModelListener& aTMEar );
    virtual void RemoveTMListener( IwxTableModelListener& aTMEar );
    /// @}

protected:
    virtual void x_FireDataChanged();
    virtual void x_FireStructureChanged();
    virtual void x_FireRowsInserted( int first_row, int last_row = -1 );
    virtual void x_FireRowsUpdated( int first_row, int last_row = -1 );
    virtual void x_FireRowsDeleted( int first_row, int last_row = -1 );
    virtual void x_FireCellUpdated( int row, int col );
    virtual void x_FireTableChanged( const CwxTableModelNotice& note );

protected:
    /// List of TM Listeners
    /// Listeners are just stored, no deletion upon removal
    list<IwxTableModelListener*>    m_ListenerList;
};


///////////////////////////////////////////////////////////////////////////////
/// CwxTextTableModel - very simple model for storing SMALL numbers fo strings.
///
/// DO NOT use this model in places where performance is critical.

class NCBI_GUIWIDGETS_WX_EXPORT CTextTableModel : public CwxAbstractTableModel
{
public:
    CTextTableModel( int n_cols = 1, int n_rows = 0 );

    virtual void Init( int n_cols, int n_rows );
    virtual void Init( const vector<wxString>& columns, int n_rows );
    //NCBI_DEPRECATED virtual void Init( const vector<string>& columns, int n_rows );

    void SetNumRows( int n_rows );

    virtual void SetIcon( int row, const wxString& icon_alias );
    virtual void SetStringValueAt( int row, int col, const wxString& value );
    //NCBI_DEPRECATED virtual void SetIcon( int row, const string& icon_alias );
    //NCBI_DEPRECATED virtual void SetStringValueAt( int row, int col, const string& value );
    virtual void SetAttachment( int row, void* attachment );
    virtual void* GetAttachment( int row );

    /// @name CwxAbstractTableModel pure virtual functions
    /// @{
    virtual wxString GetColumnName( int col ) const;
    virtual int GetNumRows() const;
    virtual int GetNumColumns() const;
    virtual wxVariant GetValueAt( int row, int col ) const;
    /// @}

    virtual void FireStructureChanged();
    virtual void FireDataChanged();
    virtual void FireRowsUpdated( int first_row, int last_row = -1 );

    virtual wxString GetImageAliasAt( int row, int col ) const;

protected:
    typedef vector<wxString> TColumn;

    vector<wxString> m_ColumnNames;
    vector<wxString> m_Icons;
    vector<TColumn> m_Columns;
    vector<void*> m_Attachments;
};


END_NCBI_SCOPE

#endif // GUI_WIDGETS_WX___TABLE_MODEL__HPP
