#ifndef GUI_WIDGETS_EDIT___MACRO_ARGUMENT__HPP
#define GUI_WIDGETS_EDIT___MACRO_ARGUMENT__HPP

/*  $Id: macro_argument.hpp 41535 2018-08-16 13:48:11Z asztalos $
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

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/publisher_subscriber.hpp>
#include <gui/objutils/macro_edit_fn_base.hpp>

BEGIN_NCBI_SCOPE

using TArgumentsVector = vector<macro::SArgMetaData>;

typedef CPublisherT<string>    TStdStringPublisher;
typedef CPublisherT<bool>      TBoolPublisher;

class CArgument;

class IArgumentSubscriber {
public:
    virtual void Update(CArgument&) = 0;
};

class CArgument
    : public CObject, public ISubscriberT<string>
{

public:
    CArgument(const macro::SArgMetaData& arg, bool enabled = true, bool shown = true);

    const TStdStringPublisher& GetValue() const { return m_Value; }
    const string&              GetName() const { return m_Arg.m_Name; }
    const macro::SArgMetaData& GetArgMetaData() const { return m_Arg; }
    const TBoolPublisher& GetEnabled() const { return m_Enabled; }
    const TBoolPublisher& GetShown() const { return m_Shown; }

    TStdStringPublisher& SetValue() { return m_Value; }
    TBoolPublisher&      SetEnabled()  { return m_Enabled; }
    TBoolPublisher&      SetShown() { return m_Shown; }

    void Attach(IArgumentSubscriber* subscriber) const;
    void Detach(IArgumentSubscriber* subscriber) const;

    bool IsTrue() const { return (m_Value.get() == "true"); }
    bool Empty() const { return m_Value.get().empty(); }

    virtual ~CArgument() {}
protected:

    void Notify();

    void Update(string const& value)
    {
        Notify();
    }

private:
    typedef vector<IArgumentSubscriber*> TSubscribersVector;
    mutable TSubscribersVector m_Subscribers;

private:
    const macro::SArgMetaData     m_Arg;
    TStdStringPublisher m_Value;
    TBoolPublisher      m_Enabled;
    TBoolPublisher      m_Shown;
};

class CArgumentList;

typedef void(*FOnArgumentChanged)(CArgumentList&, CArgument&);

class CArgumentList : public IArgumentSubscriber
{
public:
    using TArguments = vector<CRef<CArgument>>;

    void Add(const macro::SArgMetaData& arg, bool enabled, bool shown);
    void Attach(FOnArgumentChanged subscriber) const;
    void Detach(FOnArgumentChanged subscriber) const;

    const TArguments& GetArguments() const { return m_Args; }
    TArguments& SetArguments() { return m_Args; }

    CArgument&        operator[](const string &arg_name);
    const CArgument&  operator[](const string &arg_name) const;

    size_t count(const string& arg_name) const;

    virtual ~CArgumentList();

protected:

    void Update(CArgument& arg)
    {
        Notify(arg);
    }

    void Notify(CArgument& arg);

private:
    TArguments m_Args;
    map<string, CRef<CArgument>> m_ArgsNew;
    typedef vector<FOnArgumentChanged> TSubscribersVector;
    mutable TSubscribersVector m_Subscribers;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_EDIT___MACRO_ARGUMENT__HPP
