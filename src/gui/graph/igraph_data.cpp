/*  $Id: igraph_data.cpp 23997 2011-07-05 15:20:35Z wuliangs $
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
#include <gui/graph/igraph_data.hpp>

BEGIN_NCBI_SCOPE

#define ASSERT_ARRAY_INDEX(iArray) _ASSERT((iArray) < m_vpArrays.size())

////////////////////////////////////////////////////////////////////////////////
/// IDataArray

IDataArray::~IDataArray()
{
};

////////////////////////////////////////////////////////////////////////////////
/// class CSeriesBase
CSeriesBase::CSeriesBase()
: m_Length(0)
{
}

CSeriesBase::CSeriesBase(int Length)
: m_Length(Length)
{
}

CSeriesBase::~CSeriesBase()
{
    RemoveAllArrays();
}


int CSeriesBase::GetLength() const
{
    return m_Length;
}
size_t CSeriesBase::GetArraysCount() const
{
    return m_vpArrays.size();
}

CSeriesBase::EDataType   CSeriesBase::GetArrayType(size_t iArray) const
{
    ASSERT_ARRAY_INDEX(iArray);
    return m_vpArrays[iArray]->GetType();
}

//*string  GetArrayName(int iArray) const

IDataArray* CSeriesBase::GetArray(size_t iArray) const
{
    ASSERT_ARRAY_INDEX(iArray);
    return m_vpArrays[iArray];
}

//int     GetIndexByName(const string& Name);

void    CSeriesBase::CreateArrays()
{
    RemoveAllArrays();
}

void    CSeriesBase::CreateArrays(int Length)
{
    m_Length = Length;
    CreateArrays();
}

void    CSeriesBase::AddArray(/*const string& Name,*/ IDataArray* pArray)
{
    //###_ASSERT(pArray && pArray->GetSize()==m_Length);
    m_vpArrays.push_back(pArray);
}

void    CSeriesBase::InsertArray(size_t iArray,/* const string& Name,*/ IDataArray* pArray)
{
    _ASSERT(pArray && pArray->GetSize() == (size_t)m_Length);
    ASSERT_ARRAY_INDEX(iArray);
    m_vpArrays.insert(m_vpArrays.begin() + iArray, pArray);
}

void    CSeriesBase::RemoveAllArrays()
{
    NON_CONST_ITERATE(vector<IDataArray*>, it, m_vpArrays)    {
        delete *it;
    }
}

// type-safe functions returning specialized arrays
INumericArray*  CSeriesBase::x_GetNumericArray(size_t iArray)
{
    ASSERT_ARRAY_INDEX(iArray);
    IDataArray* pAr = m_vpArrays[iArray];
    return dynamic_cast<INumericArray*>(pAr);
}

IStringArray*   CSeriesBase::x_GetStringArray(size_t iArray)
{
    ASSERT_ARRAY_INDEX(iArray);
    IDataArray* pAr = m_vpArrays[iArray];
    return dynamic_cast<IStringArray*>(pAr);
}

IColorArray*    CSeriesBase::x_GetColorArray(size_t iArray)
{
    ASSERT_ARRAY_INDEX(iArray);
    IDataArray* pAr = m_vpArrays[iArray];
    return dynamic_cast<IColorArray*>(pAr);
}

IPointerArray*  CSeriesBase::x_GetPointerArray(size_t iArray)
{
    ASSERT_ARRAY_INDEX(iArray);
    IDataArray* pAr = m_vpArrays[iArray];
    return dynamic_cast<IPointerArray*>(pAr);
}

END_NCBI_SCOPE
