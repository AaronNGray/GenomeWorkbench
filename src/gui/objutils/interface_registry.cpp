/*  $Id: interface_registry.cpp 40531 2018-03-02 21:27:20Z katargir $
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
#include <corelib/ncbimtx.hpp>
#include <gui/objutils/interface_registry.hpp>

BEGIN_NCBI_SCOPE

DEFINE_STATIC_MUTEX(s_ObjFactoryMutex);

void CInterfaceRegistry::RegisterFactory(const string& interface_name, IInterfaceFactory* factory)
{
    RegisterFactory(interface_name, NULL, factory);
}

void CInterfaceRegistry::RegisterFactory(const string& interface_name, IObjectInterfaceFactory* factory)
{
    RegisterFactory(interface_name, NULL, factory);
}

void CInterfaceRegistry::RegisterFactory(const string& interface_name,
                                                TTypeInfo info, 
                                                IInterfaceFactory* factory)
{
    CMutexGuard LOCK(s_ObjFactoryMutex);
    x_GetInstance().m_Interfaces[info][interface_name] = CRef<IInterfaceFactory>(factory);
}

void CInterfaceRegistry::RegisterFactory(const string& interface_name,
                                                TTypeInfo info,
                                                IObjectInterfaceFactory* factory)
{
    CMutexGuard LOCK(s_ObjFactoryMutex);
    x_GetInstance().m_ObjectInterfaces[info][interface_name] = CRef<IObjectInterfaceFactory>(factory);
}

CObject* CInterfaceRegistry::CreateInterface(
            const string& interface_name,
            TTypeInfo info,
            ICreateParams* params)
{
    CMutexGuard LOCK(s_ObjFactoryMutex);

    CInterfaceRegistry& factory = x_GetInstance();

    TTypeIfFactoryMap::const_iterator it = factory.m_Interfaces.find(info);
    if (it == factory.m_Interfaces.end())  // Search default factory
        it = factory.m_Interfaces.find(NULL);
    if (it == factory.m_Interfaces.end())
        return NULL;

    TIfFactoryMap::const_iterator it2 = it->second.find(interface_name);
    if (it2 != it->second.end())
        return it2->second->CreateInterface(params);

    return NULL;
}

CObject* CInterfaceRegistry::CreateInterface(
            const string& interface_name,
            SConstScopedObject& object,
            ICreateParams* params)
{
    CMutexGuard LOCK(s_ObjFactoryMutex);

    const CSerialObject* cso = dynamic_cast<const CSerialObject*>(object.object.GetPointer());
    if (!cso)
        return nullptr;

    try {
        TTypeInfo info = cso->GetThisTypeInfo();

        CInterfaceRegistry& factory = x_GetInstance();

        TTypeIfObjectFactoryMap::const_iterator it = factory.m_ObjectInterfaces.find(info);
        if (it != factory.m_ObjectInterfaces.end()) {
            TIfObjectFactoryMap::const_iterator it2 = it->second.find(interface_name);
            if (it2 != it->second.end())
                return it2->second->CreateInterface(object, params);
        }
        it = factory.m_ObjectInterfaces.find(NULL);
        if (it != factory.m_ObjectInterfaces.end()) {
            TIfObjectFactoryMap::const_iterator it2 = it->second.find(interface_name);
            if (it2 != it->second.end())
                return it2->second->CreateInterface(object, params);
        }

        return CreateInterface(interface_name, info, params);
    }
    catch (const exception&)
    {
        return nullptr;
    }
}

bool CInterfaceRegistry::ObjectHasInterface(const string& interface_name, SConstScopedObject& object)
{
    CMutexGuard LOCK(s_ObjFactoryMutex);

    const CSerialObject* cso = dynamic_cast<const CSerialObject*>(object.object.GetPointer());
    if (!cso)
        return false;

    TTypeInfo info = cso->GetThisTypeInfo();

    CInterfaceRegistry& factory = x_GetInstance();

    TTypeIfObjectFactoryMap::const_iterator it = factory.m_ObjectInterfaces.find(info);
    if (it != factory.m_ObjectInterfaces.end()) {
        TIfObjectFactoryMap::const_iterator it2 = it->second.find(interface_name);
        if (it2 != it->second.end())
            return true;
    }
    it = factory.m_ObjectInterfaces.find(NULL);
    if (it != factory.m_ObjectInterfaces.end()) {
        TIfObjectFactoryMap::const_iterator it2 = it->second.find(interface_name);
        if (it2 != it->second.end())
            return true;
    }

    return false;
}

CInterfaceRegistry& CInterfaceRegistry::x_GetInstance()
{
    static CInterfaceRegistry inst_;
    return inst_;
}

END_NCBI_SCOPE
