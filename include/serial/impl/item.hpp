#ifndef ITEM__HPP
#define ITEM__HPP

/*  $Id: item.hpp 547822 2017-10-04 15:45:27Z gouriano $
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

#include <corelib/ncbistd.hpp>
#include <serial/serialutil.hpp>
#include <serial/impl/typeref.hpp>
#include <serial/impl/memberid.hpp>
#include <serial/impl/objstack.hpp>


/** @addtogroup FieldsComplex
 *
 * @{
 */


BEGIN_NCBI_SCOPE

class CItemsInfo;
class CSerialFacet;

class NCBI_XSERIAL_EXPORT CItemInfo
{
public:
    enum {
        eNoOffset = -1
    };

    CItemInfo(const CMemberId& id, TPointerOffsetType offset,
              TTypeInfo type);
    CItemInfo(const CMemberId& id, TPointerOffsetType offset,
              const CTypeRef& type);
    CItemInfo(const char* id, TPointerOffsetType offset,
              TTypeInfo type);
    CItemInfo(const char* id, TPointerOffsetType offset,
              const CTypeRef& type);
    virtual ~CItemInfo(void);

    const CMemberId& GetId(void) const;
    CMemberId& GetId(void);

    TMemberIndex GetIndex(void) const;

    TPointerOffsetType GetOffset(void) const;

    TTypeInfo GetTypeInfo(void) const;

    virtual void UpdateDelayedBuffer(CObjectIStream& in,
                                     TObjectPtr classPtr) const = 0;

    TObjectPtr GetItemPtr(TObjectPtr object) const;
    TConstObjectPtr GetItemPtr(TConstObjectPtr object) const;

    bool NonEmpty(void) const;
    CItemInfo* SetNonEmpty(void);

    bool Optional(void) const;

    void Validate(TConstObjectPtr classPtr, const CObjectStack& stk) const;

    CItemInfo* Restrict( ESerialFacet type, const string& pattern);
    CItemInfo* Restrict( ESerialFacet type, Uint8 value);
    CItemInfo* RestrictI(ESerialFacet type, Int8 value);
    CItemInfo* RestrictD(ESerialFacet type, double value);

    template<typename T>
    CItemInfo* RestrictV(ESerialFacet type, T value) {
        static_assert(is_integral<T>::value || is_floating_point<T>::value, "Wrong data type");
        if (std::is_integral<T>::value && std::is_signed<T>::value) {
            return RestrictI(type, (Int8)value);
        }
        else if (std::is_integral<T>::value && std::is_unsigned<T>::value) {
            return Restrict(type, (Uint8)value);
        }
        else if (std::is_floating_point<T>::value) {
            return RestrictD(type, (double)value);
        }
        _ASSERT(0);
        return nullptr;
    }

private:
    friend class CItemsInfo;
    virtual void UpdateFunctions(void) = 0;

    // member ID
    CMemberId m_Id;
    // member index
    TMemberIndex m_Index;
    // offset of member inside object
    TPointerOffsetType m_Offset;
    // type of member
    CTypeRef m_Type;

    bool m_NonEmpty;
protected:
    bool m_Optional;
    const CSerialFacet* m_Restrict;
};

class CConstObjectInfo;
class NCBI_XSERIAL_EXPORT CSerialFacet
{
public:
    CSerialFacet(void);
    virtual ~CSerialFacet(void);
    void Validate(TTypeInfo info, TConstObjectPtr object, const CObjectStack& stk) const;
    virtual void Validate(const CConstObjectInfo& oi, const CObjectStack& stk) const = 0;
};

/* @} */


#include <serial/impl/item.inl>

END_NCBI_SCOPE

#endif  /* ITEM__HPP */
