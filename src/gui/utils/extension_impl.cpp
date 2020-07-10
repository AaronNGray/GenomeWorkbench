/*  $Id: extension_impl.cpp 18080 2008-10-14 22:33:27Z yazhuk $
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

#include <ncbi_pch.hpp>

#include <gui/utils/extension_impl.hpp>

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
///  CExtensionPoint - default implementation of IExtensionPoint
CExtensionPoint::CExtensionPoint(const string& id, const string& label)
:   m_Id(id),
    m_Label(label)
{
}


string CExtensionPoint::GetIdentifier() const
{
    CMutexGuard guard(m_Mutex);
    return m_Id;
}


string CExtensionPoint::GetLabel() const
{
    CMutexGuard guard(m_Mutex);
    return m_Label;
}


bool CExtensionPoint::AddExtension(IExtension& extension)
{
    CMutexGuard guard(m_Mutex);

    string id = extension.GetExtensionIdentifier();
    TIdToExtMap::iterator it = m_IdToExt.find(id);
    if(it == m_IdToExt.end())   {
        m_IdToExt[id] = TExtRef(&extension);
        return true;
    }
    return false;
}


CIRef<IExtension> CExtensionPoint::GetExtension(const string& ext_id)
{
    CMutexGuard guard(m_Mutex);

    TIdToExtMap::iterator it = m_IdToExt.find(ext_id);
    if(it != m_IdToExt.end())   {
        return it->second;
    }
    return TExtRef();
}


void CExtensionPoint::GetExtensions(TExtVec& extensions)
{
    CMutexGuard guard(m_Mutex);

    NON_CONST_ITERATE(TIdToExtMap, it, m_IdToExt)   {
        extensions.push_back(it->second);
    }
}


bool CExtensionPoint::RemoveExtension(IExtension& extension)
{
    return RemoveExtension(extension.GetExtensionIdentifier());
}


bool CExtensionPoint::RemoveExtension(const string& ext_id)
{
    CMutexGuard guard(m_Mutex);

    NON_CONST_ITERATE(TIdToExtMap, it, m_IdToExt)   {
        if(it->first == ext_id)   {
            m_IdToExt.erase(it);
            return true;
        }
    }
    return false;
}


void CExtensionPoint::SetLabel(const string& label)
{
    CMutexGuard guard(m_Mutex);
    if(m_Label.empty()) {
        m_Label = label;
    }
}


///////////////////////////////////////////////////////////////////////////////
/// CExtensionRegistry

CSafeStaticRef<CExtensionRegistry> CExtensionRegistry::sm_Instance;

// static
CIRef<IExtensionRegistry> CExtensionRegistry::GetInstance()
{
    return CIRef<IExtensionRegistry>(&sm_Instance.Get());
}


CExtensionRegistry::CExtensionRegistry()
{
}

bool CExtensionRegistry::AddExtensionPoint(IExtensionPoint& ext_point)
{
    CMutexGuard guard(m_Mutex);

    string ext_point_id = ext_point.GetIdentifier();

    TExtPointRef point_ref = GetExtensionPoint(ext_point_id);
    if( point_ref)   {
        ERR_POST("CExtensionRegistry::AddExtensionPoint() - point \"" << ext_point_id
            << "\" already registered.");
        return false;
    }

    m_IdToPoint[ext_point_id] = CIRef<IExtensionPoint>(&ext_point);
    return true;
}



bool CExtensionRegistry::AddExtensionPoint(const string& ext_point_id, const string& ext_point_label)
{
    CMutexGuard guard(m_Mutex);

    TExtPointRef point_ref = GetExtensionPoint(ext_point_id);
    if( ! point_ref)   {
        x_CreateExtensionPoint(ext_point_id, ext_point_label);
        return true;
    } else {
        // see if the point was created automatically
        if(point_ref->GetLabel().empty())   {
            CExtensionPoint* the_point =
                dynamic_cast<CExtensionPoint*>(point_ref.GetPointer());
            _ASSERT(the_point);

            the_point->SetLabel(ext_point_label); // only assign label
            return true;
        } else {
            ERR_POST("CExtensionRegistry::AddExtensionPoint() - point \"" << ext_point_id
                     << "\" already registered.");
        }
    }
    return false;
}


bool CExtensionRegistry::AddExtension(const string& ext_point_id, IExtension& extension)
{
    CMutexGuard guard(m_Mutex);

    TExtPointRef point = GetExtensionPoint(ext_point_id);
    if( ! point)   {
        // create the point automatically
        point = x_CreateExtensionPoint(ext_point_id, "");
    }
    _ASSERT(point);
    return point->AddExtension(extension);
}


CRef<CExtensionPoint>
    CExtensionRegistry::x_CreateExtensionPoint(const string& ext_point_id,
                                                const string& ext_point_label)
{
    _ASSERT( ! ext_point_id.empty());

    CRef<CExtensionPoint> point(new CExtensionPoint(ext_point_id, ext_point_label));
    m_IdToPoint[ext_point_id] = point;
    return point;
}


CExtensionRegistry::TExtRef
    CExtensionRegistry::GetExtension(const string& extension_id)
{
    CMutexGuard guard(m_Mutex);

    TExtRef extension; // empty

    NON_CONST_ITERATE(TIdToPointMap, it, m_IdToPoint)  {
        IExtensionPoint& point = *(it->second);
        extension = point.GetExtension(extension_id);
        if(extension)  {
            break;
        }
    }
    return extension;
}


CExtensionRegistry::TExtRef
    CExtensionRegistry::GetExtension(const string& ext_point_id,
                                     const string& extension_id)
{
    CMutexGuard guard(m_Mutex);

    TExtRef extension;
    TExtPointRef point = GetExtensionPoint(ext_point_id);
    if(point)   {
        extension = point->GetExtension(extension_id);
    }
    return extension;
}


CExtensionRegistry::TExtPointRef
    CExtensionRegistry::GetExtensionPoint(const string& ext_point_id)
{
    CMutexGuard guard(m_Mutex);

    TExtPointRef point;
    TIdToPointMap::iterator it = m_IdToPoint.find(ext_point_id);
    if(it != m_IdToPoint.end())   {
        point = it->second;
    }
    return point;
}


bool CExtensionRegistry::GetExtensions(const string& ext_point_id,
                                       TExtVec& extensions)
{
    CMutexGuard guard(m_Mutex);

    TExtPointRef point;
    TIdToPointMap::iterator it = m_IdToPoint.find(ext_point_id);
    if(it != m_IdToPoint.end())   {
        point = it->second;
        point->GetExtensions(extensions);
        return true;
    }
    return false;
}


void CExtensionRegistry::GetExtensionPoints(TExtPointVec& ext_points)
{
    CMutexGuard guard(m_Mutex);

    NON_CONST_ITERATE(TIdToPointMap, it, m_IdToPoint)   {
        ext_points.push_back(it->second);
    }
}


bool CExtensionRegistry::RemoveExtension(IExtension& extension)
{
    return RemoveExtension(extension.GetExtensionIdentifier());
}


bool CExtensionRegistry::RemoveExtension(const string& ext_id)
{
    CMutexGuard guard(m_Mutex);

    bool ok = false;
    NON_CONST_ITERATE(TIdToPointMap, it, m_IdToPoint)   {
        IExtensionPoint& point = *it->second;
        bool res = point.RemoveExtension(ext_id);
        ok |= res;
    }
    return ok;
}


bool CExtensionRegistry::RemoveExtension(const string& ext_point_id, IExtension& extension)
{
    TExtPointRef point = GetExtensionPoint(ext_point_id);
    if(point)   {
        return point->RemoveExtension(extension);
    }
    return false;
}


bool CExtensionRegistry::RemoveExtension(const string& ext_point_id, const string& ext_id)
{
    TExtPointRef point = GetExtensionPoint(ext_point_id);
    if(point)   {
        return point->RemoveExtension(ext_id);
    }
    return false;
}


bool CExtensionRegistry::RemoveExtensionPoint(IExtensionPoint& ext_point)
{
    CMutexGuard guard(m_Mutex);

    NON_CONST_ITERATE(TIdToPointMap, it, m_IdToPoint)   {
        if(it->second.GetPointer() == &ext_point)   {
            m_IdToPoint.erase(it);
            return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
/// CExtensionPointDeclaration
CExtensionPointDeclaration::CExtensionPointDeclaration(const string& id,
                                                       const string& label)
{
    CIRef<IExtensionRegistry> reg = CExtensionRegistry::GetInstance();
    reg->AddExtensionPoint(id, label);
}

CExtensionPointDeclaration::CExtensionPointDeclaration(IExtensionPoint& ext_point)
{
    CIRef<IExtensionRegistry> reg = CExtensionRegistry::GetInstance();
    reg->AddExtensionPoint(ext_point);
}

///////////////////////////////////////////////////////////////////////////////
/// CExtensionDeclaration
CExtensionDeclaration::CExtensionDeclaration(const string& ext_point_id,
                                             IExtension* extension)
{
    CIRef<IExtensionRegistry> reg = CExtensionRegistry::GetInstance();
    reg->AddExtension(ext_point_id, *extension);
}


END_NCBI_SCOPE
