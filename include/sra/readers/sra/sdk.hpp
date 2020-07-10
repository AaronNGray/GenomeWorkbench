#ifndef SRA__READER__SRA__SDK__HPP
#define SRA__READER__SRA__SDK__HPP
/*  $Id: sdk.hpp 519854 2016-11-18 18:25:23Z vasilche $
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
 * Authors:  Eugene Vasilchenko
 *
 * File Description:
 *   Access to SRA SDK
 *
 */

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiexpt.hpp>
#include <sra/readers/sra/exception.hpp>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)

class CSeq_entry;

/////////////////////////////////////////////////////////////////////////////
//  CSraRcFormatter
/////////////////////////////////////////////////////////////////////////////

class CSraRcFormatter
{
public:
    explicit CSraRcFormatter(rc_t rc) : m_RC(rc) {}

    rc_t GetRC(void) const
        {
            return m_RC;
        }
private:
    rc_t m_RC;
};
NCBI_SRAREAD_EXPORT
ostream& operator<<(ostream& out, const CSraRcFormatter& f);

template<class Object>
struct CSraRefTraits
{
};

#define DECLARE_SRA_REF_TRAITS(T, Const)                                \
    template<>                                                          \
    struct CSraRefTraits<Const T>                                       \
    {                                                                   \
        static rc_t x_Release(const T* t);                              \
        static rc_t x_AddRef (const T* t);                              \
    }
#define DEFINE_SRA_REF_TRAITS(T, Const)                                 \
    rc_t CSraRefTraits<Const T>::x_Release(const T* t)                  \
    { return T##Release(t); }                                           \
    rc_t CSraRefTraits<Const T>::x_AddRef (const T* t)                  \
    { return T##AddRef(t); }


template<class Object>
class CSraRef
{
protected:
    typedef CSraRef<Object> TSelf;
    typedef CSraRefTraits<Object> TTraits;
public:
    typedef Object TObject;
    typedef TObject* TPointer;

    CSraRef(void)
        : m_Object(0)
        {
        }
    CSraRef(const TSelf& ref)
        : m_Object(s_AddRef(ref))
        {
        }
    /// Copy constructor from an existing CRef object, 
    CSraRef(TSelf&& ref)
        {
            m_Object = ref.m_Object;
            ref.m_Object = 0;
        }

    TSelf& operator=(const TSelf& ref)
        {
            if ( this != &ref ) {
                Release();
                m_Object = s_AddRef(ref);
            }
            return *this;
        }
    TSelf& operator=(TSelf&& ref)
        {
#ifdef NCBI_COMPILER_MSVC
            // extra check on MSVC that doesn't conform to C++ Standard
            if ( this == &ref ) {
                // no-op
                return *this;
            }
#endif
            Release();
            m_Object = ref.m_Object;
            ref.m_Object = 0;
            return *this;
        }

    ~CSraRef(void)
        {
            Release();
        }

    void Release(void)
        {
            if ( m_Object ) {
                if ( rc_t rc = TTraits::x_Release(m_Object) ) {
                    CSraException::ReportError("Cannot release ref", rc);
                }
                m_Object = 0;
            }
        }
    
    TObject* GetPointer(void) const
        {
            if ( !m_Object ) {
                NCBI_THROW(CSraException, eNullPtr,
                           "Null SRA pointer");
            }
            return m_Object;
        }

    operator TObject*(void) const
        {
            return m_Object;
        }
    TObject* operator->(void) const
        {
            return GetPointer();
        }
    TObject& operator*(void) const
        {
            return *GetPointer();
        }

protected:
    explicit
    CSraRef(TPointer ptr)
        : m_Object(ptr)
        {
        }

    TObject** x_InitPtr(void)
        {
            Release();
            return &m_Object;
        }

protected:
    static TObject* s_AddRef(const TSelf& ref)
        {
            TObject* obj = ref.m_Object;
            if ( obj ) {
                if ( rc_t rc = TTraits::x_AddRef(obj) ) {
                    NCBI_THROW2(CSraException, eAddRefFailed,
                                "Cannot add ref", rc);
                }
            }
            return obj;
        }

private:
    TObject* m_Object;
};


END_SCOPE(objects)
END_NCBI_SCOPE

#endif // SRA__READER__SRA__SDK__HPP
