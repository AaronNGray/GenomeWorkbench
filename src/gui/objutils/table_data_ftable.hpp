#ifndef GUI_UTILS___TABLE_DATA_FTABLE__HPP
#define GUI_UTILS___TABLE_DATA_FTABLE__HPP

/*  $Id: table_data_ftable.hpp 32240 2015-01-22 21:53:03Z katargir $
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
#include <objects/seqfeat/Seq_feat.hpp>

/** @addtogroup GUI_UTILS
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class CTableDataFTable : public CObject, public CTableDataBase
{
public:
    static CTableDataFTable* CreateObject(SConstScopedObject& object, ICreateParams* params);

    virtual ColumnType GetColumnType(size_t col) const;
    virtual string     GetColumnLabel(size_t col) const;

    virtual void       LoadData() {}
    virtual size_t GetRowsCount() const;
    virtual size_t GetColsCount() const;

    virtual void GetStringValue(size_t row, size_t col, string& value) const;
    virtual long   GetIntValue(size_t row, size_t col) const;
    virtual double GetRealValue(size_t row, size_t col) const;
    virtual SConstScopedObject GetObjectValue(size_t row, size_t col) const;
    virtual SConstScopedObject GetRowObject(size_t row) const;

    CRef<objects::CScope> GetScope() { return m_Scope; }

private:
    void Init();

    CConstRef<CObject> m_Object;
    mutable CRef<objects::CScope> m_Scope;
    vector<CRef<objects::CSeq_feat> > m_FTable;
};

END_NCBI_SCOPE


/* @} */

#endif  // GUI_UTILS___TABLE_DATA_FTABLE__HPP
