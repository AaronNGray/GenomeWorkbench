#ifndef GUI_GRAPH___IGRAPH_DATA__HPP
#define GUI_GRAPH___IGRAPH_DATA__HPP

/*  $Id: igraph_data.hpp 23960 2011-06-24 15:46:45Z wuliangs $
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

#include <corelib/ncbistl.hpp>

#include <gui/graph/igraph.hpp>

/** @addtogroup GUI_GRAPH
 *
 * @{
 */

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// interface IDataArray

class NCBI_GUIGRAPH_EXPORT IDataArray
{
public:
    enum EDataType {
        eNumeric,
        eString,
        eColor,
        ePointer
    };

    virtual ~IDataArray();
    virtual EDataType   GetType() = 0;
    virtual size_t     GetSize() = 0;
};

///////////////////////////////////////////////////////////////////////////////
/// class STypeSelector
// this class is used to define TValueType based on EDataType

template <IDataArray::EDataType Type>
struct STypeSelector   {
};

template<> struct STypeSelector<IDataArray::eNumeric>   {
    typedef TModelUnit  TValueType;
};
template<> struct STypeSelector<IDataArray::eString>    {
    typedef string  TValueType;
};
template<> struct STypeSelector<IDataArray::eColor>    {
    typedef CRgbaColor  TValueType;
};
template<> struct STypeSelector<IDataArray::ePointer>    {
    typedef void*  TValueType;
};

///////////////////////////////////////////////////////////////////////////////
/// class ITypedDataArray<Type>

template <IDataArray::EDataType Type>
class ITypedDataArray : public IDataArray
{
public:
    typedef typename STypeSelector<Type>::TValueType TValueType;

    virtual EDataType   GetType()   {   return Type;    }
    virtual size_t      GetSize() = 0;
    virtual TValueType    GetElem(size_t i) = 0; // returns value of specialized type
};

// specializations for various EDataTypes

typedef ITypedDataArray<IDataArray::eNumeric>   INumericArray;
typedef ITypedDataArray<IDataArray::eString>    IStringArray;
typedef ITypedDataArray<IDataArray::eColor>     IColorArray;
typedef ITypedDataArray<IDataArray::ePointer>   IPointerArray;

///////////////////////////////////////////////////////////////////////////////
/// class CTypedArrayAdapter<Type, TBase>

template <IDataArray::EDataType Type,
    typename TBase =
#ifndef NCBI_COMPILER_MSVC
    typename
#endif
    ITypedDataArray<Type>::TValueType >
    class CTypedArrayAdapter : public ITypedDataArray<Type>
{
public:
    typedef typename ITypedDataArray<Type>::TValueType TValueType;
    typedef vector<TBase> TCont;

    CTypedArrayAdapter()    {}
    CTypedArrayAdapter(int Length) {    m_vValues.resize(Length);   }

    // ITypedDataArray<Type> implementation
    virtual size_t     GetSize()    {   return m_vValues.size();   }
    virtual TValueType  GetElem(size_t i)
    {
        _ASSERT(i < m_vValues.size());
        return (TValueType) m_vValues[i];
    };

    TCont&    GetContainer()    {   return m_vValues;  }
    const TCont&    GetContainer()  const    {   return m_vValues;  }

protected:
    TCont m_vValues;
};

///////////////////////////////////////////////////////////////////////////////
/// class CSeriesBase
/// CSeries represents a set of colinear IDataArrays and provides basic access and management.
/// All arrays must have the same length.
class NCBI_GUIGRAPH_EXPORT CSeriesBase
{
public:
    typedef IDataArray::EDataType   EDataType;

    virtual ~CSeriesBase();

    // user interface for accessing data
    int GetLength() const;
    size_t GetArraysCount() const;

    EDataType   GetArrayType(size_t iArray) const;
    IDataArray* GetArray(size_t iArray) const;

    //string      GetArrayName(int iArray) const;
    //int     GetIndexByName(const string& Name);
protected:
    // DataArray management interface provided for derived classes
    CSeriesBase();
    CSeriesBase(int Length);

    virtual void    CreateArrays();
    virtual void    CreateArrays(int Length);

    // CSeriesBase owns IDataArray-s added to series, destructor will destroy them
    void    AddArray(/*const string& Name,*/ IDataArray* pArray);
    void    InsertArray(size_t iArray,/*const string& Name,*/ IDataArray* pArray);
    //void    RemoveArray(int iArray);
    void    RemoveAllArrays();

    // type-safe functions returning specialized arrays
    INumericArray*  x_GetNumericArray(size_t iArray);
    IStringArray*   x_GetStringArray(size_t iArray);
    IColorArray*    x_GetColorArray(size_t iArray);
    IPointerArray*  x_GetPointerArray(size_t iArray);

protected:
    vector<IDataArray*>    m_vpArrays;
    map<string, int>    m_mpNameToIndex;
    int m_Length;
};


END_NCBI_SCOPE

#endif
