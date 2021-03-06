/* $Id: PluginObject.cpp 30280 2014-04-23 20:28:26Z katargir $
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
 * Author:  .......
 *
 * File Description:
 *   .......
 *
 * Remark:
 *   This code was originally generated by application DATATOOL
 *   using specifications from the data definition file
 *   'plugin.asn'.
 */

// standard includes

// generated includes
#include <ncbi_pch.hpp>
#include <gui/objects/PluginObject.hpp>
#include <gui/objects/GBProjectHandle.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

// destructor
CPluginObject::~CPluginObject(void)
{
}


//
//
void CPluginObject::SetProject(const CGBProjectHandle* project)
{
    m_Project.Reset(project);
    m_Object.Reset(project);

    SetObjaddr(NStr::PtrToString(m_Object.GetPointer()));
    SetSubtype(CGBProjectHandle::GetTypeInfo()->GetName());
}


void CPluginObject::SetObject(const CObject& obj, const CGBProjectHandle* project, const CDataHandle * dh)
{
    m_Project.Reset(project);
    m_Object.Reset(&obj);

    SetObjaddr(NStr::PtrToString(m_Object.GetPointer()));

    // subtype determination
    SetSubtype("Object");
    {{
        const CSerialObject* ptr =
            dynamic_cast<const CSerialObject*>(m_Object.GetPointer());
        if (ptr) {
            SetSubtype(ptr->GetThisTypeInfo()->GetName());
        }
    }}
}


void CPluginObject::SetObject(const CTypeInfo* info)
{
    if (info) {
        SetObject(info->GetName());
    } else {
        SetObject("Object");
    }
}


void CPluginObject::SetObject(const string& type)
{
    m_Project.Reset();
    m_Object.Reset();

    SetObjaddr(NStr::PtrToString(NULL));
    SetSubtype(type);
}


//
// overloaded Assign() because we have some derived information to copy as well
//
void CPluginObject::UserOp_Assign(const CSerialUserOp& obj)
{
    const CPluginObject& o = dynamic_cast<const CPluginObject&> (obj);

    // copy the address - this is the base class info
    SetObjaddr(o.GetObjaddr());
    SetSubtype(o.GetSubtype());

    // copy our derived info as well
    m_Object = o.m_Object;
    m_Project  = o.m_Project;
}


bool CPluginObject::UserOp_Equals(const CSerialUserOp& obj) const
{
    const CPluginObject& o = dynamic_cast<const CPluginObject&> (obj);

    return (GetObjaddr()  == o.GetObjaddr()  &&
            GetSubtype()  == o.GetSubtype()  &&
            GetProject() == o.GetProject()  &&
            GetObject()   == o.GetObject());
}



// Reset()
// this is overloaded from CPluginObject_Base and allows us to reset our derived
// type information
void CPluginObject::Reset(void)
{
    // call the base class
    Tparent::Reset();

    // reset our specific information
    m_Object.Reset();
    m_Project.Reset();
}


//
// PostRead()
// this function is responsible for parsing any state information from our
// address string
//
void CPluginObject::PostRead()
{
    m_Object.Reset
        (static_cast<const CObject*>(NStr::StringToPtr(GetObjaddr())));

    /**
    //
    // store our derived object type information
    //
    SetSubtype("Object");

    // generic handling of CSerialObject-derived classes
    {{
        const CSerialObject* obj =
            dynamic_cast<const CSerialObject*>(m_Object.GetPointer());
        if (obj) {
            SetSubtype(obj->GetThisTypeInfo()->GetName());
        }
    }}

    // generic handling of CGBProjectHandles
    {{
        const CGBProjectHandle* obj =
            dynamic_cast<const CGBProjectHandle*>(m_Object.GetPointer());
        if (obj) {
            SetSubtype("Project");
        }
    }}
    **/
}


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE

/* Original file checksum: lines: 64, chars: 1886, CRC32: f857b507 */
