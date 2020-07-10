#ifndef GUI_WIDGETS_EDIT___PUBLISHER_SUBSCRIBER__HPP
#define GUI_WIDGETS_EDIT___PUBLISHER_SUBSCRIBER__HPP

/*  $Id: publisher_subscriber.hpp 40298 2018-01-23 21:34:09Z asztalos $
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
*  and reliability of the software and data,  the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties,  express or implied,  including
*  warranties of performance,  merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Authors:  Vladislav Evgeniev
*/

#include <vector>
#include <assert.h>

BEGIN_NCBI_SCOPE

template<typename T>
class ISubscriberT {
public:
    virtual void Update(const T& value) = 0;
};

template<typename T>
class CPublisherT
{
public:
    CPublisherT()
    {
    }

    CPublisherT(CPublisherT<T> const& publisher)
    {
        m_Subscribers = publisher.m_Subscribers;
        m_Value = publisher.m_Value;
        Notify();
    }

    CPublisherT<T>& operator=(T const& rhs)
    {
        m_Value = rhs;
        Notify();
        return (*this);
    }

    bool operator==(T const& rhs) const
    {
        return (m_Value == rhs);
    }

    bool operator!=(T const& rhs) const
    {
        return (m_Value != rhs);
    }

    operator T const&() const
    {
        return m_Value;
    }

    T const& get() const
    {
        return m_Value;
    }

    void Attach(ISubscriberT<T>* pSubscriber) const
    {
        assert(pSubscriber);
        m_Subscribers.push_back(pSubscriber);
    }

    void Detach(ISubscriberT<T>* pSubscriber) const
    {
        assert(pSubscriber);
        typename TSubscribersVector::iterator itToBeDetached;
        for (itToBeDetached = m_Subscribers.begin(); itToBeDetached != m_Subscribers.end(); ++itToBeDetached) {
            if (pSubscriber != *itToBeDetached)
                continue;

            m_Subscribers.erase(itToBeDetached);
            break;
        }
    }

    void Notify()
    {
        typename TSubscribersVector::iterator itSubscriber;
        for (itSubscriber = m_Subscribers.begin(); itSubscriber != m_Subscribers.end(); ++itSubscriber)
            (*itSubscriber)->Update(m_Value);
    }
private:
    typedef std::vector<ISubscriberT<T>*> TSubscribersVector;
    mutable TSubscribersVector m_Subscribers;
    T   m_Value;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_EDIT___PUBLISHER_SUBSCRIBER__HPP
