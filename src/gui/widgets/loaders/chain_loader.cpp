/*  $Id: chain_loader.cpp 39192 2017-08-18 14:12:57Z evgeniev $
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
*  and reliability of the software and data,  the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties,  express or implied,  including
*  warranties of performance,  merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Authors:  Vladislav Evgeniev
*/


#include <ncbi_pch.hpp>

#include <gui/widgets/loaders/chain_loader.hpp>

#include <util/icanceled.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

IObjectLoader::TObjects& CChainLoader::GetObjects()
{
    if (m_Objects.empty()) {
        for (auto &loader : m_Loaders) {
            IObjectLoader::TObjects& objects = loader->GetObjects();
            m_Objects.insert(m_Objects.end(), objects.begin(), objects.end());
        }
    }
    return m_Objects;
}

string CChainLoader::GetDescription() const
{
    string description;
    bool first{ true };
    for (auto &loader : m_Loaders) {
        if (first)
            first = false;
        else
            description += " & ";
        description += loader->GetDescription();
    }
    return description;
}

bool CChainLoader::PreExecute()
{
    for (auto &unit : m_ExecuteUnits) {
        unit->PreExecute();
    }
    return true;
}

bool CChainLoader::Execute(ICanceled& canceled)
{
    for (auto &unit : m_ExecuteUnits) {
        
        if (canceled.IsCanceled()) 
            return false;
        
        unit->Execute(canceled);
    }
     
    return true;
}

bool CChainLoader::PostExecute()
{
    for (auto &unit : m_ExecuteUnits) {
        unit->PostExecute();
    }
    return true;
}

void CChainLoader::Add(IObjectLoader *loader)
{
    _ASSERT(loader);
    CIRef<IObjectLoader> ldr(loader);
    IExecuteUnit* unit = dynamic_cast<IExecuteUnit*>(loader);
    _ASSERT(unit);
    m_Loaders.push_back(ldr);
    m_ExecuteUnits.push_back(CIRef<IExecuteUnit>(unit));
}

END_NCBI_SCOPE
