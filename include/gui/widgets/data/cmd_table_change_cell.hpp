#ifndef GUI_OBJUTILS___CMD_TABLE_CHANGE_CELL__HPP
#define GUI_OBJUTILS___CMD_TABLE_CHANGE_CELL__HPP

/*  $Id: cmd_table_change_cell.hpp 31553 2014-10-22 16:28:19Z katargir $
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
#include <gui/gui_export.h>

#include <wx/variant.h>

#include <gui/utils/command_processor.hpp>
#include <gui/objutils/table_data.hpp>
#include <objmgr/seq_feat_handle.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_DATA_EXPORT CCmdTableChangeCell : 
    public CObject, public IEditCommand
{
public:
    CCmdTableChangeCell(ITableData& tableData, int row, int col, long value)
        : m_TableData(&tableData), m_Row(row), m_Col(col), m_Value(value) {}
    CCmdTableChangeCell(ITableData& tableData, int row, int col, double value)
        : m_TableData(&tableData), m_Row(row), m_Col(col), m_Value(value) {}
    CCmdTableChangeCell(ITableData& tableData, int row, int col, const wxString& value)
        : m_TableData(&tableData), m_Row(row), m_Col(col), m_Value(value) {}

    /// @name IEditCommand interface implementation
    /// @{
    virtual void Execute();
    virtual void Unexecute();
    virtual string GetLabel();
    /// @}

protected:
    CIRef<ITableData> m_TableData;
    int  m_Row;
    int  m_Col;
    wxVariant m_Value;
};

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___CMD_TABLE_CHANGE_CELL__HPP
