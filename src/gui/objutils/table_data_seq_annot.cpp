/*  $Id: table_data_seq_annot.cpp 36071 2016-08-03 18:14:45Z katargir $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/table_data.hpp>
#include <gui/objutils/table_data_aln_summary.hpp>

#include <objects/seqtable/Seq_table.hpp>

#include "table_data_ftable.hpp"
#include "table_data_seq_table.hpp"


#include <objects/seq/Seq_annot.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CTableDataSeq_annot : public CObject, public ITableData
{
public:
    static CObject* CreateObject(SConstScopedObject& object, ICreateParams* params);

    virtual ColumnType GetColumnType(size_t col) const { return kNone; }
    virtual string     GetColumnLabel(size_t col) const { return ""; }

    virtual size_t GetRowsCount() const { return 0; }
    virtual size_t GetColsCount() const { return 0; }
};

void initCTableDataSeq_annot()
{
    CInterfaceRegistry::RegisterFactory(
            typeid(ITableData).name(),
            CSeq_annot::GetTypeInfo(),
            new CObjectInterfaceFactory<CTableDataSeq_annot>());
}

CObject* CTableDataSeq_annot::CreateObject(SConstScopedObject& object, ICreateParams* params)
{
    const CSeq_annot& seq_annot = dynamic_cast<const CSeq_annot&>(*object.object);
    if (seq_annot.IsFtable()) {
        return CTableDataFTable::CreateObject(object, params);
    }
    else if (seq_annot.IsSeq_table()) {
        const CSeq_table& seq_table = seq_annot.GetData().GetSeq_table();
        SConstScopedObject new_object;
        new_object.object = CConstRef<CObject>(&seq_table);
        new_object.scope = object.scope;
        return CTableDataSeq_table::CreateObject(new_object, params);
    }
    else if (seq_annot.IsAlign()) {
        TConstScopedObjects objects;
        objects.push_back(object);
        return new CTableDataAlnSummary(objects);
    }
    return NULL;
}

END_NCBI_SCOPE
