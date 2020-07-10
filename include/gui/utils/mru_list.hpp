#ifndef GUI_WIDGETS_FL___MRU_LIST__HPP
#define GUI_WIDGETS_FL___MRU_LIST__HPP

/*  $Id: mru_list.hpp 31318 2014-09-18 17:48:09Z katargir $
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

#include <corelib/ncbistd.hpp>
#include <corelib/ncbistl.hpp>

#include <algorithm>
#include <map>


BEGIN_NCBI_SCOPE


////////////////////////////////////////////////////////////////////////////////
/// CMRUList
///
/// CMRUList is an adapter around a standard STL container implementing MRU list
/// behavior.
template<class T>   class CMRUList
{
public:
    typedef list<T> TItems;

    CMRUList(size_t max_size = 7)
    {
        SetMaxSize((int)max_size);
    }

    void    AddItem(T item)
    {
        typename TItems::iterator it
            = std::find(m_Items.begin(), m_Items.end(), item);

        if (it != m_Items.begin()  ||  m_Items.empty()) {
            if (it != m_Items.end())
                m_Items.erase(it);
            m_Items.push_front(item);
            if(m_Items.size() > m_MaxSize)
                m_Items.pop_back();
        }
    }

    void    RemoveItem(T item)
    {
        typename TItems::iterator it
            = std::find(m_Items.begin(), m_Items.end(), item);

        if (it != m_Items.end())
            m_Items.erase(it);
    }

    void    SetMaxSize(int max_size)
    {
        if(max_size >= 0)  {
            m_MaxSize = max_size;
            int excess = (int)(m_Items.size() - m_MaxSize);
            while(0 < excess--)   {
                m_Items.pop_back();
            }
        }
    }

    size_t GetMaxSize() const { return m_MaxSize; }

    const TItems&   GetItems() const
    {
        return m_Items;
    }
    void    GetItems(TItems& items)
    {
        items = m_Items;
    }
    void    GetItems(vector<T>& items) const
    {
        ITERATE(typename TItems, it, m_Items)    {
            items.push_back(*it);
        }
    }
    void    SetItems(const TItems& items)
    {
        m_Items = items;
    }
    void    SetItems(const vector<T>& items)
    {
        m_Items.clear();
        ITERATE(typename vector<T>, it, items)    {
            m_Items.push_back(*it);
        }
    }
    TItems& SetItems()
    {
        return m_Items;
    }

private:
    TItems  m_Items;
    size_t  m_MaxSize;
};


////////////////////////////////////////////////////////////////////////////////
/// CTimeMRUList - a MRU list where "recent" items are determined based on
/// provided time, not on insertion order
template<class T>  class  CTimeMRUList
{
public:
    typedef multimap<time_t, T> TTimeToTMap;
    typedef T value_type;

    CTimeMRUList(int max_size = 10) :   m_MaxSize(max_size) {}

    size_t  GetMaxSize() const  {   return m_MaxSize;   }
    void    SetMaxSize(size_t max_size);

    void    Clear();
    void    Add(T elem, time_t time = 0);

    const TTimeToTMap&   GetMap() const   {   return  m_TimeToT;   }

protected:
    int m_MaxSize;
    TTimeToTMap  m_TimeToT;
};


template<class T> void CTimeMRUList<T>::SetMaxSize(size_t max_size)
{
    if(m_TimeToT.size() > max_size)    {
        // delete some of the elements from the end
        size_t n = m_TimeToT.size() - max_size;
        for( size_t i = 0;  i < n;  i++ )  {
            m_TimeToT.erase(m_TimeToT.begin());
        }
    }
    m_MaxSize = (int)max_size;
}


template<class T> void CTimeMRUList<T>::Clear()
{
    m_TimeToT.clear();
}


template<class T> void CTimeMRUList<T>::Add(T elem, time_t tm)
{
    if(tm == 0)   {
        tm = time(NULL); //now
    }
    // elem may be already in the list, we need to erase it
    for( typename TTimeToTMap::iterator it = m_TimeToT.begin();
                                    it != m_TimeToT.end();  it++ )  {
        if(it->second == elem)  {
            m_TimeToT.erase(it);
            break;
        }
    }
    m_TimeToT.insert(typename TTimeToTMap::value_type(tm, elem));
    SetMaxSize(m_MaxSize);
}


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_FL___MRU_LIST__HPP
