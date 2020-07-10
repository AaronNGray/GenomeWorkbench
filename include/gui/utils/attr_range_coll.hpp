#ifndef GUI_WIDGETS_DATA___ATTR_RANGE_COLL__HPP
#define GUI_WIDGETS_DATA___ATTR_RANGE_COLL__HPP

/*  $Id: attr_range_coll.hpp 23970 2011-06-27 18:15:09Z kuznets $
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
 */

/** @addtogroup GUI_UTILS
*
* @{
*/

#include <corelib/ncbistd.hpp>
#include <corelib/ncbistl.hpp>

#include <algorithm>
#include <iterator>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// TAttrRangeCollection represents a sequence of continious attributed intervals.
/// Intervals are neither intersecting nor separated by gaps.

template<class Attr, class Position>
    struct SAttrRangeCollectionElement
{
    // This structure is moved out of owner template CAttrRangeCollection
    // because of MSVS fails to resolve necessary reverse operator< by
    // template's inner class.
    Position  m_Start;
    Attr      m_Attr;

    bool operator<(const Position& pos) const
    {
        return m_Start < pos;
    }
#  ifdef NCBI_COMPILER_MSVC
#    if _MSC_VER >= 1400
    // Additional compare operator for MSVC 2005 only
    bool operator<(const SAttrRangeCollectionElement& elem) const
    {
        return m_Start < elem.m_Start;
    }
#    endif
#  endif
};

#ifdef NCBI_COMPILER_MSVC
#    if _MSC_VER >= 1400
    // Additional compare operator for MSVC 2005 only
    template<class Attr, class Position>
    inline
    bool operator<(const Position& pos,
        const SAttrRangeCollectionElement<Attr, Position>& elem)
    {
        return pos < elem.m_Start;
    }
#    endif
#endif


template<class Attr, class Position>
    class CAttrRangeCollection
{
public:
    typedef Attr        attr_type;
    typedef Position    position_type;
    typedef CAttrRangeCollection<attr_type, position_type>  TThisType;

protected:
    typedef SAttrRangeCollectionElement<Attr, Position> SElement;
    typedef vector<SElement>    TElementVector;
    TElementVector m_vElems;
    /// contains one extra element that is used to represent open end pos
public:
    typedef typename TElementVector::size_type   size_type;
    typedef typename TElementVector::const_iterator vec_const_iterator;

    class const_iterator;
    class element_proxy;
    class const_pos_iterator;

    CAttrRangeCollection()
    {
        SetFrom(0);
    }
    CAttrRangeCollection(TSeqPos start)
    {
        SetFrom(start);
    }
    void swap(CAttrRangeCollection& c)
    {
        m_vElems.swap(c.m_vElems);
    }
    void    SetFrom(TSeqPos start)
    {
        if(m_vElems.size() == 0)    {
            m_vElems.push_back(SElement());
        }
        else if(m_vElems.size() > 1) {
            clear();
        }
        m_vElems.back().m_Start = start;
    }
    void    push_back(const attr_type& attr)
    {
        push_back(attr, 1);
    }
    void    push_back(const attr_type& attr, TSeqPos len)
    {
        int N = (int)m_vElems.size();
        if((N > 1)  && (m_vElems[N-2].m_Attr == attr)) {
            m_vElems[N-1].m_Start += len; //extend
        } else {
            m_vElems[N-1].m_Attr = attr;
            m_vElems.push_back(SElement());
            m_vElems.back().m_Start = m_vElems[N-1].m_Start + len;
        }
    }

    const_iterator  begin() const
    {
        return const_iterator(m_vElems.begin());
    }

    const_iterator  end() const
    {
        // return the iterator to the last element (open end position)
        return const_iterator(m_vElems.end() - 1);
    }
    size_type size() const
    {
        return m_vElems.size() - 1;
    }
    bool    empty() const
    {
        return m_vElems.size() == 1;
    }
    const element_proxy operator[](size_type pos)   const
    {
        return element_proxy(m_vElems.begin() + pos);
    }
    void    clear()
    {
        m_vElems.begin()->m_Start = m_vElems.back().m_Start;
        m_vElems.resize(1);
    }
    const_iterator  find(position_type pos)   const
    {
        // use "m_vElems.end() - 1" because it represents the collection end()
        vec_const_iterator it = x_Find(m_vElems.begin(), m_vElems.end() - 1, pos);
        return const_iterator(it);
    }
    const_iterator  find(const_iterator first, const_iterator last, position_type pos)   const
    {
        vec_const_iterator it = x_Find(*first, *last, pos);
        return const_iterator(it);
    }

    position_type   GetFrom() const
    {
        return m_vElems.begin()->m_Start;
    }
    position_type   GetToOpen() const
    {
        return m_vElems.back().m_Start;
    }
    position_type   GetTo() const
    {
        return m_vElems.back().m_Start - 1;
    }
    position_type   GetLength (void) const
    {
       return m_vElems.back().m_Start - m_vElems.begin()->m_Start;
    }

    const_pos_iterator  begin_pos() const
    {
        return const_pos_iterator(begin());
    }
    const_pos_iterator  end_pos() const
    {
        return const_pos_iterator(end());
    }

protected:
    vec_const_iterator x_Find(vec_const_iterator first, vec_const_iterator last,
                              position_type pos) const
    {
        if(first != last)   {
            // pos is inside the collection
            vec_const_iterator it = lower_bound(first, last, pos);
            if(it != m_vElems.end())    {
                if(it->m_Start > pos)   {
                    if(it > m_vElems.begin()) {
                        return --it;
                    }
                } else return it;
            }
        }
        return  last; // return iterator to the last element
    }

public:
    // represents virtual element
    class element_proxy
    {
    public:
        element_proxy(vec_const_iterator it)
        : m_itEl(it)
        {
        }
        element_proxy(const element_proxy& ep)
        : m_itEl(ep.m_itEl)
        {
        }
        TSeqPos GetFrom() const
        {
            return m_itEl->m_Start;
        }
        TSeqPos GetTo()   const
        {
            return (m_itEl + 1)->m_Start - 1;
        }
        TSeqPos GetToOpen()   const
        {
            return (m_itEl + 1)->m_Start;
        }
        TSeqPos GetLength()   const
        {
            return (m_itEl + 1)->m_Start - m_itEl->m_Start;
        }
        const attr_type& GetAttr() const
        {
            return m_itEl->m_Attr;
        }
        bool    RefEquals(const element_proxy& ep) const
        {
            return m_itEl == ep.m_itEl;
        }
        operator vec_const_iterator ()    const
        {
            return m_itEl;
        }
        void    Inc()
        {
            ++m_itEl;
        }
        void    Dec()
        {
            --m_itEl;
        }
    protected:
        vec_const_iterator m_itEl;
    };

    class const_iterator
    {
        friend class CAttrRangeCollection;
    public:
        const_iterator(vec_const_iterator itEl)
            : m_proxy(itEl)
        {
        }
        const_iterator(const const_iterator& it)
            : m_proxy(it.m_proxy)
        {
        }
        const element_proxy& operator*()  const
        {
            return m_proxy;
        }
        const element_proxy* operator->()  const
        {
            return &m_proxy;
        }
        const_iterator& operator++()
        {
            Inc();
            return (*this);
        }
        const_iterator operator++(int)
        {
            const_iterator Tmp = *this;
            Inc();
            return Tmp;
        }
        const_iterator& operator--()
        {
            Dec();
            return (*this);
        }
        const_iterator operator--(int)
        {
            const_iterator Tmp = *this;
            Dec();
            return Tmp;
        }
        bool operator==(const const_iterator& it) const
        {
            return m_proxy.RefEquals(it.m_proxy);
        }
        bool operator!=(const const_iterator& it) const
        {
            return ! (*this==it);
        }
    protected:
        void    Inc()
        {
            m_proxy.Inc();
        }
        void    Dec()
        {
            m_proxy.Dec();
        }
    protected:
        element_proxy m_proxy;
    };

    // iterate as though this was a vector of attribute values,
    // not a vector of ranges of equal attribute values.
    class const_pos_iterator
    {
    public:
        typedef random_access_iterator_tag iterator_category;
        typedef const attr_type value_type;
        typedef const attr_type& reference;
        typedef const attr_type* pointer;
        typedef TSignedSeqPos difference_type;

        const_pos_iterator(const_iterator itR)
            : m_position(itR->GetFrom()), m_range_it(itR)
        {
        }
        const_pos_iterator(const const_pos_iterator& it)
            : m_position(it.m_position), m_range_it(it.m_range_it)
        {
        }
        const attr_type& operator*()  const
        {
            return m_range_it->GetAttr();
        }
        const attr_type* operator->()  const
        {
            return &(m_range_it->GetAttr());
        }
        const_pos_iterator& operator++()
        {
            Inc();
            return (*this);
        }
        const_pos_iterator operator++(int)
        {
            const_pos_iterator Tmp = *this;
            Inc();
            return Tmp;
        }
        const_pos_iterator& operator--()
        {
            Dec();
            return (*this);
        }
        const_pos_iterator operator--(int)
        {
            const_pos_iterator Tmp = *this;
            Dec();
            return Tmp;
        }
        const_pos_iterator& operator+=(int n) {
            m_position += n;
            if (n > 0) {
                while (m_range_it->GetTo() < m_position) {
                    ++m_range_it;
                }
            } else if (n < 0) {
                while (m_position < m_range_it->GetFrom()) {
                    --m_range_it;
                }
            }
            return (*this);
        }
        const_pos_iterator operator+(int n) const
        {
            const_pos_iterator Tmp = *this;
            return Tmp += n;
        }
        const_pos_iterator& operator-=(int n)
        {
            return (*this += -n);
        }
        const_pos_iterator operator-(int n) const
        {
            return this + -n;
        }
        TSignedSeqPos operator-(const const_pos_iterator& it) const
        {
            return m_position - it.m_position;
        }
        bool operator==(const const_pos_iterator& it) const
        {
            return m_position == it.m_position;
        }
        bool operator!=(const const_pos_iterator& it) const
        {
            return ! (*this==it);
        }
        bool operator<(const const_pos_iterator& it) const
        {
            return m_position < it.m_position;
        }
        bool operator>(const const_pos_iterator& it) const
        {
            return m_position > it.m_position;
        }
        bool operator<=(const const_pos_iterator& it) const
        {
            return m_position <= it.m_position;
        }
        bool operator>=(const const_pos_iterator& it) const
        {
            return m_position >= it.m_position;
        }
    protected:
        void    Inc()
        {
            ++m_position;
            if (m_range_it->GetTo() < m_position) {
                ++m_range_it;
            }
        }
        void    Dec()
        {
            --m_position;
            if (m_position < m_range_it->GetFrom()) {
                --m_range_it;
            }
        }
    protected:
        TSeqPos         m_position;
        const_iterator  m_range_it;
    };

protected:

};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_DATA___ATTR_RANGE_COLL__HPP
