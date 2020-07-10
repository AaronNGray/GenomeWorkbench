#ifndef GUI_FRAMEWORK___SERVICE__HPP
#define GUI_FRAMEWORK___SERVICE__HPP

/*  $Id: service.hpp 31428 2014-10-02 17:42:02Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// IService – an abstraction that represents an application component
/// providing specific functional capabilities via a special interface.
/// The interface can be produced by applying dynamic_cast<> to IService pointer.
/// The main intention of IService is to serve as a polymorphic base class and
/// to provide basic lifetime management. Classes implementing IService must be
/// CRef-compatible.

class NCBI_GUIFRAMEWORK_EXPORT IService
{
public:
    virtual void    InitService() = 0;
    virtual void    ShutDownService() = 0;

    virtual ~IService() {};
};


///////////////////////////////////////////////////////////////////////////////
/// IServiceLocator - an abstract mechanism for locating services.
/// IServiceLocator locates a service by it’s name. In most cases the name of
/// a service class or interface should be used as the name of the service.

class IServiceLocator
{
public:
    /// returns true if the service exists within the scope of the locator
    virtual bool    HasService(const string& name) = 0;

    /// retrieves the service
    virtual CIRef<IService> GetService(const string& name) = 0;

    virtual ~IServiceLocator() {}

    /// retrieves a typed reference to a service, the name of C++ type
    /// is used as the name of the service.
    template<class T>   CIRef<T>    GetServiceByType();
};


/// helper function that uses C++ class name of the service as its name
/// the function can be used like this:
/// CIRef<TheService> srv = srv_locator->GetServiceByType<TheService>();
///
template<class T> inline CIRef<T> IServiceLocator::GetServiceByType()
{
    string t_name(typeid(T).name());
    CIRef<IService> srv = GetService(t_name);
    CIRef<T> t_srv(dynamic_cast<T*>(srv.GetPointer()));
    return t_srv;
}

///////////////////////////////////////////////////////////////////////////////
/// IServiceLocatorConsumer - classes that need IServiceLocator should
/// implement this interface.
class IServiceLocatorConsumer
{
public:
    virtual void    SetServiceLocator(IServiceLocator* locator) = 0;

    virtual ~IServiceLocatorConsumer() {}
};


template<class T> void Consumer_SetLocator(T* obj, IServiceLocator* locator)
{
    IServiceLocatorConsumer* consumer =
        dynamic_cast<IServiceLocatorConsumer*>(obj);
    if(consumer)    {
        consumer->SetServiceLocator(locator);
    }
}


END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___SERVICE__HPP

