#ifndef GUI_OBJUTILS___TABLE_DATA_STATUS__HPP
#define GUI_OBJUTILS___TABLE_DATA_STATUS__HPP

/*  $Id: table_data_status.hpp 32240 2015-01-22 21:53:03Z katargir $
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

#include <gui/gui.hpp>

#include <gui/objutils/table_data_base.hpp>

BEGIN_NCBI_SCOPE

class CTableDataStatus : public CObject, public CTableDataBase
{
public:
    CTableDataStatus(const string& statusText) : m_StatusText(statusText) {}

    virtual ColumnType GetColumnType(size_t /*col*/) const { return kString; }
    virtual string     GetColumnLabel(size_t /*col*/) const { return "Status"; }

    virtual size_t GetRowsCount() const { return 1; }
    virtual size_t GetColsCount() const { return 1; }

    virtual void GetStringValue(size_t /*row*/, size_t /*col*/, string& value) const { value = m_StatusText; }

private:
    string m_StatusText;
};

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___TABLE_DATA_STATUS__HPP
