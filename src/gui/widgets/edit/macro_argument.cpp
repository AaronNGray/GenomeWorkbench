/*  $Id: macro_argument.cpp 44581 2020-01-24 20:57:23Z asztalos $
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


#include <ncbi_pch.hpp>
#include <gui/widgets/edit/macro_argument.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(macro);

CArgument::CArgument(const SArgMetaData &arg, bool enabled, bool shown) 
    : m_Arg(arg)
{
    m_Value.Attach(this);
    m_Enabled = enabled;
    m_Shown = shown;
}

void CArgument::Attach(IArgumentSubscriber* subscriber) const
{
    assert(subscriber);
    m_Subscribers.push_back(subscriber);
}

void CArgument::Detach(IArgumentSubscriber* subscriber) const
{
    assert(subscriber);
    TSubscribersVector::iterator itToBeDetached;
    for (itToBeDetached = m_Subscribers.begin(); itToBeDetached != m_Subscribers.end(); ++itToBeDetached) {
        if (subscriber != *itToBeDetached)
            continue;

        m_Subscribers.erase(itToBeDetached);
        break;
    }
}

void CArgument::Notify()
{
    for (auto subscriber : m_Subscribers)
        subscriber->Update(*this);
}

// CArgumentList
void CArgumentList::Add(const SArgMetaData& arg, bool enabled, bool shown)
{
    CRef<CArgument> argument(new CArgument(arg, enabled, shown));
    m_Args.push_back(argument);
    m_ArgsNew.emplace(arg.m_Name, argument);
    argument->Attach(this);
}

CArgument& CArgumentList::operator[](const string &arg_name)
{
    auto it = m_ArgsNew.find(arg_name);
    if ( it != m_ArgsNew.end()) {
        return *(it->second);
    }
    NCBI_THROW(CException, eUnknown, "Argument '" + arg_name + "' not found!");
}

const CArgument& CArgumentList::operator[](const string &arg_name) const
{
    const auto it = m_ArgsNew.find(arg_name);
    if (it != m_ArgsNew.end()) {
        return *(it->second);
    }
    NCBI_THROW(CException, eUnknown, "Argument '" + arg_name + "' not found!");
}

size_t CArgumentList::count(const string& arg_name) const
{
    return count_if(m_Args.begin(), m_Args.end(), [&arg_name](CRef<CArgument> arg) { return (arg->GetName() == arg_name); });
}

void CArgumentList::Attach(FOnArgumentChanged subscriber) const
{
    assert(subscriber);
    m_Subscribers.push_back(subscriber);
}

void CArgumentList::Detach(FOnArgumentChanged subscriber) const
{
    assert(subscriber);
    TSubscribersVector::iterator itToBeDetached;
    for (itToBeDetached = m_Subscribers.begin(); itToBeDetached != m_Subscribers.end(); ++itToBeDetached) {
        if (subscriber != *itToBeDetached)
            continue;

        m_Subscribers.erase(itToBeDetached);
        break;
    }
}

void CArgumentList::Notify(CArgument& arg)
{
    for (auto subscriber : m_Subscribers)
        (*subscriber)(*this, arg);
}

CArgumentList::~CArgumentList()
{
}

END_NCBI_SCOPE
