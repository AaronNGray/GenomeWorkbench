#ifndef GUI_UTILS___TABLE_DATA_SEQ_TABLE__HPP
#define GUI_UTILS___TABLE_DATA_SEQ_TABLE__HPP

/*  $Id: table_data_seq_table.hpp 32240 2015-01-22 21:53:03Z katargir $
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
#include <objmgr/scope.hpp>

#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/table_data_base.hpp>

/** @addtogroup GUI_UTILS
 *
 * @{
 */

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CSeqTable_column;
END_SCOPE(objects)

class CTableDataSeq_table : public CObject, public CTableDataBase
{
public:
    static CTableDataSeq_table* CreateObject(SConstScopedObject& object, ICreateParams* params);

    virtual void       LoadData() {}
    virtual ColumnType GetColumnType(size_t col) const;
    virtual string     GetColumnLabel(size_t col) const;
    virtual string     GetRowLabel(size_t row) const;

    virtual vector<string> GetColumnCommonStrings(size_t col) const;

    virtual size_t GetRowsCount() const;
    virtual size_t GetColsCount() const;

    void GetStringValue(size_t row, size_t col, string& value) const;

    virtual long   GetIntValue(size_t row, size_t col) const;
    virtual double GetRealValue(size_t row, size_t col) const;
    virtual SConstScopedObject GetObjectValue(size_t row, size_t col) const;

    virtual void SetStringValue(size_t row, size_t col, const string& value);
    virtual void SetIntValue(size_t row, size_t col, long value);
    virtual void SetRealValue(size_t row, size_t col, double value);

    virtual bool AllowEdit(size_t col);

    CRef<objects::CScope> GetScope() { return m_Scope; }

private:
    const objects::CSeqTable_column& x_GetColumn(size_t col) const;
    objects::CSeqTable_column& x_GetColumn(size_t col);
    void Init();

    CConstRef<CObject> m_Object;
    mutable CRef<objects::CScope> m_Scope;

    struct SLocLabel {
        SLocLabel() : m_Col(-1) {}
        SLocLabel(int col) : m_Col(col) {}

        bool operator<(const SLocLabel& rhs) const { return m_Col < rhs.m_Col; }
        bool operator==(const SLocLabel& rhs) const { return m_Col == rhs.m_Col; }

        int m_Col;
        vector<string> m_Labels;
    };

    mutable vector<SLocLabel> m_LocLabels;

    vector<size_t> m_ColMap; // Some columns are hidden (i.e. disabled rows)
    set<size_t>    m_InvalidRows; // disabled rows
};

END_NCBI_SCOPE


/* @} */

#endif  // GUI_UTILS___TABLE_DATA_SEQ_TABLE__HPP
