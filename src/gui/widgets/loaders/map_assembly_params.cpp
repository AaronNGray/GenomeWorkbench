/*  $Id: map_assembly_params.cpp 44756 2020-03-05 18:44:36Z shkeda $
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
 * Authors:
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/loaders/map_assembly_params.hpp>

////@begin includes
////@end includes

BEGIN_NCBI_SCOPE

CMapAssemblyParams::CMapAssemblyParams()
{
    Init();
}
CMapAssemblyParams::CMapAssemblyParams(const CMapAssemblyParams& data)
{
    Init();
    Copy(data);
}
CMapAssemblyParams::~CMapAssemblyParams()
{
}
void CMapAssemblyParams::operator=(const CMapAssemblyParams& data)
{
    Copy(data);
}
bool CMapAssemblyParams::operator==(const CMapAssemblyParams& data) const
{
////@begin CMapAssemblyParams equality operator
     if (!(m_AssemblyAcc == data.m_AssemblyAcc)) return false;
     if (!(m_AssemblyDesc == data.m_AssemblyDesc)) return false;
     if (!(m_AssemblyName == data.m_AssemblyName)) return false;
     if (!(m_SearchTerm == data.m_SearchTerm)) return false;
     if (!(m_UseMapping == data.m_UseMapping)) return false;
////@end CMapAssemblyParams equality operator
    return true;
}
void CMapAssemblyParams::Copy(const CMapAssemblyParams& data)
{
////@begin CMapAssemblyParams copy function
    m_AssemblyAcc = data.m_AssemblyAcc;
    m_AssemblyDesc = data.m_AssemblyDesc;
    m_AssemblyName = data.m_AssemblyName;
    m_SearchTerm = data.m_SearchTerm;
    m_UseMapping = data.m_UseMapping;
////@end CMapAssemblyParams copy function
}
void CMapAssemblyParams::Init()
{
////@begin CMapAssemblyParams member initialisation
    m_SearchTerm = "human";
    m_UseMapping = false;
////@end CMapAssemblyParams member initialisation
}

static const char* kUseMapping = "UseMapping";
static const char* kAssAcc = "AssemblyAcc";
static const char* kAssName = "AssemblyName";
static const char* kAssDesc = "AssemblyDesc";
static const char* kAssSearchTerm = "AssemblySearchTerm";

void CMapAssemblyParams::LoadAsn(objects::CUser_field& user_field)
{
    if (user_field.HasField(kUseMapping) && 
        user_field.GetField(kUseMapping).GetData().IsBool()) {
            m_UseMapping = user_field.
                GetField(kUseMapping).GetData().GetBool();
    }

    if (user_field.HasField(kAssAcc) && 
        user_field.GetField(kAssAcc).GetData().IsStr()) {
            m_AssemblyAcc = user_field.
                GetField(kAssAcc).GetData().GetStr();
    }
    if (user_field.HasField(kAssName) && 
        user_field.GetField(kAssName).GetData().IsStr()) {
            m_AssemblyName = user_field.
                GetField(kAssName).GetData().GetStr();
    }
    if (user_field.HasField(kAssDesc) && 
        user_field.GetField(kAssDesc).GetData().IsStr()) {
            m_AssemblyDesc = user_field.
                GetField(kAssDesc).GetData().GetStr();
    }
    if (user_field.HasField(kAssSearchTerm) && 
        user_field.GetField(kAssSearchTerm).GetData().IsStr()) {
            m_SearchTerm = user_field.
                GetField(kAssSearchTerm).GetData().GetStr();
    }
}

void CMapAssemblyParams::SaveAsn(objects::CUser_field& user_field) const
{
    user_field.AddField(kUseMapping, m_UseMapping);
    user_field.AddField(kAssAcc, m_AssemblyAcc);
    user_field.AddField(kAssName, m_AssemblyName);
    user_field.AddField(kAssDesc, m_AssemblyDesc);
    user_field.AddField(kAssSearchTerm, m_SearchTerm);
}

void CMapAssemblyParams::SaveSettings(CRegistryWriteView& view) const
{
    view.Set(kUseMapping, m_UseMapping);
    view.Set(kAssAcc, m_AssemblyAcc);
    view.Set(kAssName, m_AssemblyName);
    view.Set(kAssDesc, m_AssemblyDesc);
    view.Set(kAssSearchTerm, m_SearchTerm);
}

void CMapAssemblyParams::LoadSettings(CRegistryReadView& view)
{
    m_UseMapping = view.GetBool(kUseMapping, m_UseMapping);
    m_AssemblyAcc = view.GetString(kAssAcc, m_AssemblyAcc);
    m_AssemblyName = view.GetString(kAssName, m_AssemblyName);
    m_AssemblyDesc = view.GetString(kAssDesc, m_AssemblyDesc);
    m_SearchTerm = view.GetString(kAssSearchTerm, m_SearchTerm);
}

END_NCBI_SCOPE
