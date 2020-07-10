#ifndef GUI_OBJUTILS___OBJECT_FACTORY__HPP
#define GUI_OBJUTILS___OBJECT_FACTORY__HPP

/*  $Id: interface_registry.hpp 32328 2015-02-05 19:03:30Z katargir $
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

#include <gui/gui_export.h>
#include <serial/typeinfo.hpp>
#include <serial/objectinfo.hpp>

#include <gui/objutils/objects.hpp>

BEGIN_NCBI_SCOPE

class ICreateParams
{
public:
    virtual ~ICreateParams() {}
};

class IInterfaceFactory : public CObject
{
public:
    virtual ~IInterfaceFactory() {}
    virtual CObject* CreateInterface(ICreateParams* params) const = 0;
};

class IObjectInterfaceFactory : public CObject
{
public:
    virtual ~IObjectInterfaceFactory() {}
    virtual CObject* CreateInterface(SConstScopedObject& object, ICreateParams* params) const = 0;
};

template <class T> class CTypeInterfaceFactory : public IInterfaceFactory
{
public:
    virtual CObject* CreateInterface(ICreateParams* /*params*/) const { return new T(); }
};

template <class T> class CObjectInterfaceFactory : public IObjectInterfaceFactory
{
public:
    virtual CObject* CreateInterface(SConstScopedObject& object, ICreateParams* params) const
    {
        return T::CreateObject(object, params);
    }
};

class NCBI_GUIOBJUTILS_EXPORT CInterfaceRegistry : public CObject
{
public:
    static void RegisterFactory(const string& interface_name, IInterfaceFactory* factory);
    static void RegisterFactory(const string& interface_name, IObjectInterfaceFactory* factory);
    static void RegisterFactory(const string& interface_name, TTypeInfo info, IInterfaceFactory* factory);
    static void RegisterFactory(const string& interface_name, TTypeInfo info, IObjectInterfaceFactory* factory);

    static CObject* CreateInterface(const string& interface_name,
                                    TTypeInfo info,
                                    ICreateParams* params);

    static CObject* CreateInterface(const string& interface_name,
                                    SConstScopedObject& object,
                                    ICreateParams* params);

    static bool ObjectHasInterface(const string& interface_name, SConstScopedObject& object);

private:
    typedef map<string, CRef<IInterfaceFactory> > TIfFactoryMap;
    typedef map<string, CRef<IObjectInterfaceFactory> > TIfObjectFactoryMap;

    typedef map<TTypeInfo, TIfFactoryMap>       TTypeIfFactoryMap;
    typedef map<TTypeInfo, TIfObjectFactoryMap> TTypeIfObjectFactoryMap;

    static CInterfaceRegistry& x_GetInstance();
    CInterfaceRegistry() {}

    TTypeIfFactoryMap       m_Interfaces;
    TTypeIfObjectFactoryMap m_ObjectInterfaces;
};

template <typename T> T* CreateInterface(TTypeInfo type_info, ICreateParams* params = NULL)
{
    return dynamic_cast<T*>
        (CInterfaceRegistry::CreateInterface(typeid(T).name(), type_info, params));
}

template <typename T> T* CreateObjectInterface(SConstScopedObject& object, ICreateParams* params)
{
    return dynamic_cast<T*>
        (CInterfaceRegistry::CreateInterface(typeid(T).name(), object, params));
}

template <typename T> bool ObjectHasInterface(SConstScopedObject& object)
{
    return CInterfaceRegistry::ObjectHasInterface(typeid(T).name(), object);
}

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___OBJECT_FACTORY__HPP
