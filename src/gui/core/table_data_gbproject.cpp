/*  $Id: table_data_gbproject.cpp 32240 2015-01-22 21:53:03Z katargir $
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

#include <serial/iterator.hpp>

#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/table_data_base.hpp>

#include <objects/gbproj/GBProject_ver2.hpp>
#include <objects/gbproj/ProjectFolder.hpp>
#include <objects/gbproj/FolderInfo.hpp>
#include <objects/seq/Annotdesc.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CTableDataGBProject : public CObject, public CTableDataBase
{
public:
    static CTableDataGBProject* CreateObject(SConstScopedObject& object, ICreateParams* params);

    virtual void       LoadData() {}
    virtual ColumnType GetColumnType(size_t col) const;
    virtual string     GetColumnLabel(size_t col) const;

    virtual size_t GetRowsCount() const;
    virtual size_t GetColsCount() const;

    virtual void GetStringValue(size_t row, size_t col, string& value) const;

    virtual SConstScopedObject GetRowObject(size_t row) const;

    CRef<CScope> GetScope() { return m_Scope; }

private:
    void x_Init();

    vector<CConstRef<CProjectItem> > m_Items;

    CConstRef<CObject> m_Object;
    mutable CRef<CScope> m_Scope;
};

void initCTableGBProject()
{
    CInterfaceRegistry::RegisterFactory(
            typeid(ITableData).name(),
            CGBProject_ver2::GetTypeInfo(),
            new CObjectInterfaceFactory<CTableDataGBProject>());
}

CTableDataGBProject* CTableDataGBProject::CreateObject(SConstScopedObject& object, ICreateParams*)
{
    CTableDataGBProject* table_data = new CTableDataGBProject();
    table_data->m_Object = object.object;
    table_data->m_Scope  = object.scope;
    table_data->x_Init();
    return table_data;
}

void CTableDataGBProject::x_Init()
{
    const CGBProject_ver2& project = dynamic_cast<const CGBProject_ver2&>(*m_Object);
    for (CTypeConstIterator<CProjectItem> it(project.GetData()); it; ++it) {
        m_Items.push_back(CConstRef<CProjectItem>(&*it));
    }
}

size_t CTableDataGBProject::GetRowsCount() const
{
    return m_Items.size();
}

size_t CTableDataGBProject::GetColsCount() const
{
    return 3;
}

CTableDataGBProject::ColumnType CTableDataGBProject::GetColumnType(size_t col) const
{
    if (col == 0 || col == 1 || col == 2)
        return kString;

    return kNone;
}

string CTableDataGBProject::GetColumnLabel(size_t col) const
{
    if (col == 0)
        return "Item Name";
    else if (col == 1)
        return "Item Description";
    else if (col == 2)
        return "Folder";

    return "";
}

void CTableDataGBProject::GetStringValue(size_t row, size_t col, string& value) const
{
    value.resize(0);
    if (row >= m_Items.size())
        return;

    if (col == 0) {
        value = m_Items[row]->GetLabel();
        return;
    }
    else if (col == 1) {
        const list< CRef<CAnnotdesc> >& descr_list = m_Items[row]->GetDescr();
        CRef<CAnnotdesc> descr;
        ITERATE( list< CRef<CAnnotdesc> >, descr_itr, descr_list ){
            if( (**descr_itr).IsComment() ){
                descr = *descr_itr;
            }
        }
        value = descr.IsNull() ? "" : descr->SetComment();
        return;
    }
    else if (col == 2) {
        const CGBProject_ver2& project = dynamic_cast<const CGBProject_ver2&>(*m_Object);
        const CProjectFolder* folder = project.GetData().FindProjectItemFolder(m_Items[row]->GetId());
        if (folder) {
            const CFolderInfo& info = folder->GetInfo();
            value = info.GetTitle();
            return;
        }
    }
}

SConstScopedObject CTableDataGBProject::GetRowObject(size_t row) const
{
    SConstScopedObject value;

    if (row >= m_Items.size())
        return value;

    value.object.Reset(m_Items[row]->GetObject());
    value.scope = m_Scope;
    return value;
}

END_NCBI_SCOPE

