#ifndef GUI_WIDGETS_CONTROLS___ROW_MODEL__HPP
#define GUI_WIDGETS_CONTROLS___ROW_MODEL__HPP

/*  $Id: row_model.hpp 43775 2019-08-29 18:56:30Z katargir $
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

#include <corelib/ncbistd.hpp>
#include <wx/variant.h>

//#include <gui/widgets/controls/anytype.hpp>
//#include <gui/widgets/controls/anyref.hpp>

BEGIN_NCBI_SCOPE


class IwxRowModel
{
/****** PUBLIC DEFINITION SECTION ******/
public:
    virtual ~IwxRowModel() {}

    virtual int GetNumColumns() const = 0;

    virtual wxVariant GetValueAtColumn( int col ) const = 0;
};
typedef IwxRowModel IRowModel;

class IRowSorter
{
/****** PUBLIC DEFINITION SECTION ******/
public:
    virtual ~IRowSorter() {}

    virtual bool operator()( const IwxRowModel& aRow, const IwxRowModel& bRow ) const = 0;
};

class NCBI_GUIWIDGETS_WX_EXPORT CReverseSorter
    : public CObject, public IRowSorter
{
    /****** PRIVATE DATA SECTION ******/
private:

    CIRef<IRowSorter> m_Sorter;

    /****** PUBLIC DEFINITION SECTION ******/
public:

    CReverseSorter( CIRef<IRowSorter> aSorter )
        : m_Sorter( aSorter )
    {
    }

    virtual bool operator()( const IwxRowModel& aRow, const IwxRowModel& bRow ) const
    {
        return (*m_Sorter)( bRow, aRow );
    }
};

class NCBI_GUIWIDGETS_WX_EXPORT CTandemSorter
    : public CObject, public IRowSorter
{
/****** PRIVATE DATA SECTION ******/
private:

    CIRef<IRowSorter> m_SorterOne;
    CIRef<IRowSorter> m_SorterTwo;

/****** PUBLIC DEFINITION SECTION ******/
public:

    CTandemSorter( CIRef<IRowSorter> aSorter, CIRef<IRowSorter> bSorter )
        : m_SorterOne( aSorter )
        , m_SorterTwo( bSorter )
    {
    }

    virtual bool operator()( const IwxRowModel& aRow, const IwxRowModel& bRow ) const
    {
        bool rv =(*m_SorterOne)( aRow, bRow );
        if( rv ){
            return rv;
        }

        rv = (*m_SorterOne)( bRow, aRow );
        if( rv ){
            return !rv;
        }

        return (*m_SorterTwo)( aRow, bRow );
    }
};

class IwxVariantSorter
{
/****** PUBLIC DEFINITION SECTION ******/
public:
    virtual ~IwxVariantSorter() {}

    virtual bool operator()( const wxVariant& aAny, const wxVariant& bAny ) const = 0;
};


class NCBI_GUIWIDGETS_WX_EXPORT CSorterByColumn
    : public CObject, public IRowSorter
{
    IwxVariantSorter* m_Sorter;
    int m_Column;

/****** PUBLIC DEFINITION SECTION ******/
public:

    CSorterByColumn( IwxVariantSorter* aSorter, int aColumn )
        : m_Sorter( aSorter )
        , m_Column( aColumn )
    {
    }

    void SetColumn( int aColumn ){ m_Column = aColumn; }

    virtual bool operator()(const IwxRowModel& aRow, const IwxRowModel& bRow) const
    {
        return (*m_Sorter)(aRow.GetValueAtColumn(m_Column), bRow.GetValueAtColumn(m_Column));
    }
};

class NCBI_GUIWIDGETS_WX_EXPORT CSorterByMultiCols
    : public CObject, public IRowSorter
{
    vector<CSorterByColumn*> m_ColSorters;

    /****** PUBLIC DEFINITION SECTION ******/
public:

    CSorterByMultiCols( vector<CSorterByColumn*> aSorters )
        : m_ColSorters( aSorters )
    {
    }

    virtual bool operator()( const IwxRowModel& aRow, const IwxRowModel& bRow ) const
    {
        ITERATE( vector<CSorterByColumn*>, sort_itr, m_ColSorters ){
            CSorterByColumn* sorter = *sort_itr;

            int cmp = (*sorter)( aRow, bRow );

            if( cmp != 0 ){
                return cmp;
            }
        }

        return 0;
    }
};

class IRowFilter
{
/****** PUBLIC DEFINITION SECTION ******/
public:
    virtual ~IRowFilter() {}

    virtual bool operator()( const IwxRowModel& aRow ) const = 0;
};


END_NCBI_SCOPE

#endif // GUI_WIDGETS_CONTROLS___ROW_MODEL__HPP
