#ifndef GUI_GUI___EXTENSION_IMPL__HPP
#define GUI_GUI___EXTENSION_IMPL__HPP

/*  $Id: extension_impl.hpp 23111 2011-02-07 19:20:32Z katargir $
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

/** @addtogroup GUI_UTILS
 *
 * @{
 */

#include <corelib/ncbi_safe_static.hpp>

#include <gui/gui_export.h>

#include <gui/utils/extension.hpp>

#include <corelib/ncbimtx.hpp>


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CExtensionPoint - standard implementation of IExtensionPoint.
/// see IExtensionPoint for more information.
class NCBI_GUIUTILS_EXPORT  CExtensionPoint :
    public CObject,
    public IExtensionPoint
{
public:
    CExtensionPoint(const string& id, const string& label);

    /// @name IExtensionPoint implementation
    /// @{
    virtual string  GetIdentifier() const;
    virtual string  GetLabel() const;
    virtual bool    AddExtension(IExtension& extension);
    virtual TExtRef GetExtension(const string& ext_id);
    virtual void    GetExtensions(TExtVec& extensions);
    virtual bool    RemoveExtension(IExtension& extension);
    virtual bool    RemoveExtension(const string& ext_id);
    /// @}

protected:
    friend class CExtensionRegistry;

    virtual void    SetLabel(const string& label);

protected:
    typedef map<string, TExtRef>    TIdToExtMap;

    mutable CMutex m_Mutex;

    string m_Id;
    string m_Label;

    TIdToExtMap m_IdToExt; // maps Extension ID -> IExtension
};


///////////////////////////////////////////////////////////////////////////////
/// CExtensionRegistry - standard implementation of IExtensionRegistry.
/// see IExtensionRegistry for more information.

class NCBI_GUIUTILS_EXPORT  CExtensionRegistry
    :   public CObject,
        public IExtensionRegistry
{
    friend class CSafeStaticRef<CExtensionRegistry>;
public:
    typedef CIRef<IExtension> TExtRef;
    typedef vector<TExtRef>   TExtVec;
    typedef CIRef<IExtensionPoint>  TExtPointRef;
    typedef vector<TExtPointRef> TExtPointVec;

public:
    /// provides access to the Singleton
    static CIRef<IExtensionRegistry>    GetInstance();

    /// @name IExtensionRegistry implementation
    /// @{
    virtual bool    AddExtensionPoint(IExtensionPoint& ext_point);
    virtual bool    AddExtensionPoint(const string& ext_point_id,
                                      const string& ext_point_label);
    virtual bool    AddExtension(const string& ext_point_id, IExtension& extension);
    virtual TExtRef GetExtension(const string& extension_id);
    virtual TExtRef GetExtension(const string& ext_point_id,
                                 const string& extension_id);
    virtual TExtPointRef    GetExtensionPoint(const string& ext_point_id);
    virtual bool    GetExtensions(const string& ext_point_id, TExtVec& extensions);

    virtual void    GetExtensionPoints(TExtPointVec& ext_points);

    virtual bool    RemoveExtension(IExtension& extension);
    virtual bool    RemoveExtension(const string& ext_id);
    virtual bool    RemoveExtension(const string& ext_point_id, IExtension& extension);
    virtual bool    RemoveExtension(const string& ext_point_id, const string& ext_id);

    virtual bool    RemoveExtensionPoint(IExtensionPoint& ext_point);
    /// @}

protected:
    CExtensionRegistry();

    CRef<CExtensionPoint> x_CreateExtensionPoint(const string& ext_point_id,
                                                 const string& ext_point_label);

protected:
    typedef map<string, TExtPointRef> TIdToPointMap;

    static CSafeStaticRef<CExtensionRegistry> sm_Instance;

    CMutex m_Mutex;
    TIdToPointMap   m_IdToPoint; // Point ID - > IExtensionPoint
};



///////////////////////////////////////////////////////////////////////////////
/// CExtensionPointDeclaration - static declaration helper.
/// To declare an Extension Point instantiate this class as a static variable
/// in your module.
class NCBI_GUIUTILS_EXPORT CExtensionPointDeclaration
{
public:
    CExtensionPointDeclaration(const string& id, const string& label);
    CExtensionPointDeclaration(IExtensionPoint& ext_point);
};


///////////////////////////////////////////////////////////////////////////////
/// CExtensionDeclaration - static declaration helper.
/// To declare an Extension instatiate this class as a static variable in
/// your module.
class NCBI_GUIUTILS_EXPORT CExtensionDeclaration
{
public:
    CExtensionDeclaration(const string& ext_point_id, IExtension* extension);
};


/// GetExtensionAsInterface() is a helper function that extracts all extensions
/// implementing the specified interface from the given extension point.
template<class I>
void GetExtensionAsInterface(const string& ext_point_id,
                             vector< CIRef<I> >& interfaces)
{
    CIRef<IExtensionRegistry> reg = CExtensionRegistry::GetInstance();
    CExtensionRegistry::TExtVec ext_vec;
    _VERIFY( reg->GetExtensions(ext_point_id, ext_vec) );

    for( size_t i = 0;  i < ext_vec.size();  i++)   {
        CIRef<IExtension> ext = ext_vec[i];
        I* type = dynamic_cast<I*>(ext.GetPointer());

        if(type)    {
            interfaces.push_back(CIRef<I>(type));
        } else {
            ERR_POST("Extension " << ext_point_id << "::" << ext->GetExtensionIdentifier() <<
                     " does not implement the required interface!");
            _ASSERT(false);
        }
    }
}


/// GetExtensionAsInterface() is a helper function that extracts all extensions
/// implementing the specified interface from the given extension point.
template<class I>
void GetExtensionAsInterface(const string& ext_point_id,
                             vector<I*>& interfaces)
{
    CIRef<IExtensionRegistry> reg = CExtensionRegistry::GetInstance();
    CExtensionRegistry::TExtVec ext_vec;
    _VERIFY( reg->GetExtensions(ext_point_id, ext_vec) );

    for( size_t i = 0;  i < ext_vec.size();  i++)   {
        CIRef<IExtension> ext = ext_vec[i];
        I* type = dynamic_cast<I*>(ext.GetPointer());

        if(type)    {
            interfaces.push_back(type);
        } else {
            ERR_POST("Extension " << ext_point_id << "::" << ext->GetExtensionIdentifier() <<
                     " does not implement the required interface!");
            _ASSERT(false);
        }
    }
}

END_NCBI_SCOPE

/* @} */

#endif // GUI_GUI___EXTENSION_IMPL__HPP
