/*  $Id: item.cpp 547688 2017-10-03 13:21:50Z gouriano $
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
* Author: Eugene Vasilchenko
*
* File Description:
*   !!! PUT YOUR DESCRIPTION HERE !!!
*/

#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>
#include <serial/impl/item.hpp>

BEGIN_NCBI_SCOPE

CItemInfo::CItemInfo(const CMemberId& id,
                     TPointerOffsetType offset, TTypeInfo type)
    : m_Id(id), m_Index(kInvalidMember), m_Offset(offset), m_Type(type),
      m_NonEmpty(false), m_Optional(false), m_Restrict(nullptr)

{
}

CItemInfo::CItemInfo(const CMemberId& id,
                     TPointerOffsetType offset, const CTypeRef& type)
    : m_Id(id), m_Index(kInvalidMember), m_Offset(offset), m_Type(type),
      m_NonEmpty(false), m_Optional(false), m_Restrict(nullptr)
{
}

CItemInfo::CItemInfo(const char* id,
                     TPointerOffsetType offset, TTypeInfo type)
    : m_Id(id), m_Index(kInvalidMember), m_Offset(offset), m_Type(type),
      m_NonEmpty(false), m_Optional(false), m_Restrict(nullptr)
{
}

CItemInfo::CItemInfo(const char* id,
                     TPointerOffsetType offset, const CTypeRef& type)
    : m_Id(id), m_Index(kInvalidMember), m_Offset(offset), m_Type(type),
      m_NonEmpty(false), m_Optional(false), m_Restrict(nullptr)
{
}

CItemInfo::~CItemInfo(void)
{
    if (m_Restrict) {
        delete m_Restrict;
    }
}

CItemInfo* CItemInfo::SetNonEmpty(void)
{
    m_NonEmpty = true;
    return this;
}

END_NCBI_SCOPE
