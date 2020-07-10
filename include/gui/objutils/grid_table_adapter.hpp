#ifndef GUI_WIDGETS_GRID_WIDGET___TABLE_DATA_QUERY__HPP
#define GUI_WIDGETS_GRID_WIDGET___TABLE_DATA_QUERY__HPP

/*  $Id: grid_table_adapter.hpp 26028 2012-06-28 21:22:02Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>

#include <gui/objutils/registry.hpp>
#include <gui/objutils/table_data.hpp>

#include <limits>

BEGIN_NCBI_SCOPE

#define kInvalidColumn (size_t)-1

class IGridTableAdapter
{
public:
    enum ESort
    {
        kAscending,
        Descending
    };

    typedef pair<size_t, ESort> TSortColumn;

    IGridTableAdapter(ITableData& tableData) 
        : m_TableData(&tableData), m_HideUnselected(false) {}
    virtual ~IGridTableAdapter() {}

    ITableData& GetTable() { return *m_TableData; }

    virtual vector<size_t> GetVisibleColumns() const { return m_VisibleColumns; }

    virtual void ShowColumn(size_t col, bool show) = 0;
    virtual bool IsColumnShown(size_t col) const = 0;

    virtual size_t GetOriginalRow(size_t queryRow) const = 0;
    virtual size_t GetCurrentRow(size_t queryRow) const = 0;

    virtual void SortByColumn(TSortColumn sortColumn) = 0;
    virtual TSortColumn GetSortColumn() const = 0;

    virtual void SetSelection(const vector<size_t>& query_sel);
    virtual size_t GetCurrentSelection() const { return m_CurrentSelection; }
    virtual void HideUnselected(bool b) { m_HideUnselected = b; }
    virtual bool GetHideUnselected() const { return m_HideUnselected; }

    virtual void IterateSelection(int /* dir */) {}

protected:
    CIRef<ITableData> m_TableData;
    vector<size_t>    m_VisibleColumns;

    vector<size_t>    m_Selected;
    size_t            m_CurrentSelection;
    bool              m_HideUnselected;
};

inline void IGridTableAdapter::SetSelection(const vector<size_t>& query_sel) 
{ 
    m_Selected = query_sel;
    std::sort(m_Selected.begin(), m_Selected.end());    
}

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_GRID_WIDGET___TABLE_DATA_QUERY__HPP
