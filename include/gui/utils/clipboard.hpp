#ifndef GUI_UTILS___CLIPBOARD__HPP
#define GUI_UTILS___CLIPBOARD__HPP

/*  $Id: clipboard.hpp 23824 2011-06-09 16:06:43Z kuznets $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

/** @addtogroup GUI_UTILS
*
* @{
*/


#include <corelib/ncbiobj.hpp>
#include <corelib/ncbimtx.hpp>

#include <gui/gui.hpp>

#include <set>
#include <vector>


BEGIN_NCBI_SCOPE

// transferrable data interface
class ITransferable
{
public:
    ITransferable(){}
    virtual ~ITransferable(){}
    virtual const std::type_info & GetType() const = 0;
    virtual ITransferable * Clone() const = 0;
};


// templated transferrable data
template <class T>
class CTransferable : public ITransferable
{
public:
    CTransferable(const T & val) : m_Value(val){}
    virtual ~CTransferable(){}
    const T & GetValue() const { return m_Value; }
    virtual const std::type_info & GetType() const { return typeid(T); }
    virtual ITransferable * Clone() const { return new CTransferable(m_Value);}
private:
    T m_Value;
};


// clipboard class, providing functionality to store
// multiple objects in multiple flavors each
//
//  CClipboard & clipboard = CClipboard::GetInstance();
//  int index = clipboard.Add(string("ten"))
//  clipboard.AddFlavor(index, 10);
//  clipboard.AddFlavor(index, 10.0);
//  clipboard.Add("Second Object");
//  string text    = clipboard.Get<string>();
//  int    integer = clipboard.Get<int>();
//  string text2   = clipboard.GetAt(1)<string>();
//  next line will throw exception, as there is no integer flavor for second element
//  int    integer2 = clipboard.GetAt(1)<int>();
//
class NCBI_GUIUTILS_EXPORT CClipboard
{
public:
    //
    typedef ITransferable    TFlavor;
    typedef set<TFlavor*>    TItem;
    typedef vector<TItem>    TContents;

public:
    // singleton access
    static CClipboard & Instance();

    // basic operations, Set() will call Clear() then Add()
    void                            Clear(void);
    size_t                          Size(void);
    template <typename T> int       Add(const T & val);
    template <typename InIt> void   Add(InIt first, InIt last);
    template <typename T> void      Set(const T & val);
    template <typename InIt> void   Set(InIt first, InIt last);
    bool                            IsEmpty(void);
    template <typename T> void      AddFlavor(const T & val);
    template <typename T> void      AddFlavorAt(int idx, const T & val);

    // these functions returns first clipboard object
    // Get() will throw an exception on type cast errors
    // GetPtr() will return NULL on invalid type casts
    template<typename T> const T & Get() const;
    template<typename T> const T * GetPtr() const;

    // accessors to any indexed object
    template<typename T> const T & GetAt(int idx) const;
    template<typename T> const T * GetPtrAt(int idx) const;

    // find item
    template <typename T> bool     Contains(const T & val);

    // to string - "\n" separated string values
    string                      ToString();
protected:

    // mutex to guard our access
    DECLARE_CLASS_STATIC_MUTEX(sm_ClipboardMutex);

private:
    // forbidden
    CClipboard(){};

protected:
    // no mutex in these functions
    // basic types
    template <typename T> size_t       x_AddValue(const T & val);
    template <typename T> size_t       x_AddValueEx(const T & val);
    template <typename T> const T * x_GetValueAt(int idx) const;
    template <typename T> void         x_AddFlavor(int idx, const T & val);
    void                            x_Clear(void);
private:
    TContents                   m_Contents;
    static auto_ptr<CClipboard> m_Clipboard;
};

// locating
template <typename T>
bool CClipboard::Contains(const T & val)
{
    for (TContents::size_type i=0; i<m_Contents.size(); i++) {
        const T * valuePtr = x_GetValueAt<T>(i);
        if (valuePtr && (*valuePtr == val)) {
            return true;
        }
    }
    return false;
}

// Adding an object to clipboard
template <typename T>
inline int CClipboard::Add(const T & val)
{
    CMutexGuard LOCK(CClipboard::sm_ClipboardMutex);
    return x_AddValue(val);
}

// Adding array of objects (one flavor of each object)
template <typename InIt>
inline void CClipboard::Add(InIt first, InIt last)
{
    CMutexGuard LOCK(CClipboard::sm_ClipboardMutex);
    for (; first != last; ++first){
        x_AddValue(*first);
    }
}

// Setting clipboard with one object
template <typename T>
inline void CClipboard::Set(const T & val)
{
    CMutexGuard LOCK(CClipboard::sm_ClipboardMutex);
    m_Contents.clear();
    x_AddValue(val);
}

// Adding array of objects (one flavor of each object)
template <typename InIt>
inline void CClipboard::Set(InIt first, InIt last)
{
    CMutexGuard LOCK(CClipboard::sm_ClipboardMutex);
    m_Contents.clear();
    for (; first != last; ++first){
        x_AddValue(*first);
    }
}

// executable adder - no specializations
template <typename T>
inline size_t CClipboard::x_AddValueEx(const T & val)
{
    TFlavor * new_flavor = new CTransferable<T>(val);
    TItem   * new_item   = new TItem;
    new_item->insert(new_flavor);
    m_Contents.push_back(*new_item);
    return (m_Contents.size() - 1);
}

// generic clipboard adder
template <typename T>
inline size_t CClipboard::x_AddValue(const T & val)
{
    return x_AddValueEx(val);
}


// adder specialization for strings
template <>
inline size_t CClipboard::x_AddValue<string>(const string & val)
{
    string procText = "";
#ifdef NCBI_OS_MSWIN
    /// adjust bare 'cr' for 'cr/lf'
    string s(val);
    string::size_type pos = 0;
    while ( (pos = s.find_first_of("\n", pos)) != string::npos) {
        if (pos == 0  ||  s[pos - 1] != '\r') {
            s.insert(s.begin() + pos, '\r');
            ++pos;
        }
        ++pos;
    }
    procText.append(s);
#else
    procText.append(val);
#endif
    return x_AddValueEx(procText);
}

template<typename T>
inline const T & CClipboard::GetAt(int idx) const
{
    CMutexGuard LOCK(CClipboard::sm_ClipboardMutex);

    if (m_Contents.empty()) {
        NCBI_THROW(CException, eUnknown,
                   "CClipboard::Get() - Clipboard Is Empty");
    }

    const T * ret_value = x_GetValueAt<T>(idx);

    if (!ret_value) {
        NCBI_THROW(CException, eUnknown,
                "CClipboard::Get() - No compatible flavor in clipboard");
    }

    return *ret_value;
}

// indexed pointer getter
template<typename T>
inline const T * CClipboard::GetPtrAt(int idx) const
{
    CMutexGuard LOCK(CClipboard::sm_ClipboardMutex);
    return x_GetValueAt<T>(idx);
}

// first value getter
template<typename T>
inline const T & CClipboard::Get() const
{
    return GetAt<T>(0);
}

// pointer-to-first-item getter
template<typename T>
inline const T * CClipboard::GetPtr() const
{
    return GetPtrAt<T>(0);
}



// function getting flavor of correct type
// returns NULL if it fails to find it
template <typename T>
inline const T * CClipboard::x_GetValueAt(int idx) const
{
    if (m_Contents.size() > (unsigned)idx){
        ITERATE(TItem, flavor, m_Contents[idx]) {
            if (typeid(T) == (*flavor)->GetType()) {
                return &(static_cast<const CTransferable<T>&>(**flavor)).GetValue();
            }
        }
    }
    return NULL;
}

template <typename T>
inline void CClipboard::x_AddFlavor(int idx, const T & val)
{
    if (m_Contents.size() > (unsigned)idx) {
        NON_CONST_ITERATE(TItem, flavor, m_Contents[idx]) {
            // drop an old same-typed flavor
            if (typeid(T) == (*flavor)->GetType()) {
                m_Contents[idx].erase(flavor);
                break;
            }
        }
        m_Contents[idx].insert(new CTransferable<T>(val));
    }
    else if (m_Contents.empty()){
        x_AddValueEx(val);
    }
}

template <typename T>
inline void CClipboard::AddFlavor(const T & val)
{
    CMutexGuard LOCK(CClipboard::sm_ClipboardMutex);
    x_AddFlavor(0, val);
}

template <typename T>
inline void CClipboard::AddFlavorAt(int idx, const T & val)
{
    CMutexGuard LOCK(CClipboard::sm_ClipboardMutex);
    x_AddFlavor(idx, val);
}

END_NCBI_SCOPE

/* @} */

#endif  // GUI_UTILS___CLIPBOARD__HPP
