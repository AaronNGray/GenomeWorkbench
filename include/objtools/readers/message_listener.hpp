/*  $Id: message_listener.hpp 608381 2020-05-15 12:43:35Z ivanov $
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
 * Author: Frank Ludwig
 *
 * File Description:
 *   Classes for listening to errors, progress, etc.
 *
 */

#ifndef OBJTOOLS_READERS___MESSAGELISTENER__HPP
#define OBJTOOLS_READERS___MESSAGELISTENER__HPP

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <corelib/ncbi_message.hpp>
#include <objtools/readers/line_error.hpp>
#include <objtools/logging/listener.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects) // namespace ncbi::objects::

//  ============================================================================
class ILineErrorListener : public CObject, public IObjtoolsListener
//  ============================================================================
{
public:
    virtual ~ILineErrorListener() {}

    // IListener::Post() implementation
    NCBI_STD_DEPRECATED("This method is no longer functional and will be removed in SC-25.") 
    virtual void Post(const IMessage& /*message*/)
    {
        // Remove in SC-25
        return;
    }

    NCBI_STD_DEPRECATED("This method is redundant and will be removed in SC-25.") 
    virtual void Push(const IObjtoolsMessage& message) 
    {
        // Remove in SC-25
        PutMessage(message);
    }
    /// Store error in the container, and 
    /// return true if error was stored fine, and
    /// return false if the caller should terminate all further processing.
    ///
    virtual bool
    PutError(
        const ILineError& ) = 0;

    virtual bool 
    PutMessage(const IObjtoolsMessage& message) {
        const ILineError* le = dynamic_cast<const ILineError*>(&message);
        if (!le) return true;
        return PutError(*le);
    }
    
    // IListener::Get() implementation
    virtual const ILineError& Get(size_t index) const
    { return this->GetError(index); }

    /// 0-based error retrieval.
    virtual const ILineError&
    GetError(size_t ) const =0;

    virtual size_t Count(void) const = 0;

    /// Returns the number of errors seen so far at the given severity.
    virtual size_t
    LevelCount(
        EDiagSev )=0;

    /// Clear all accumulated messages.
    virtual void
    ClearAll() =0;

    // IListener::Progress() implementation
    virtual void Progress(const string& message,
                          Uint8         current,
                          Uint8         total) { PutProgress(message, current, total); }

    /// This is used for processing progress messages.
    virtual void
    PutProgress(
        const string & sMessage,
        const Uint8 iNumDone = 0,
        const Uint8 iNumTotal = 0 ) = 0;

    virtual const ILineError& GetMessage(size_t index) const
    { return Get(index); }

    virtual void Clear(void)
    { ClearAll(); }
};


// Compatibility declaration, avoid using objects::IMessageListener -
// use ncbi::IMessageListener or objects::ILineErrorListener instead.
class IMessageListener : public ILineErrorListener
{
public:
    virtual ~IMessageListener(void) {}
};


//  ============================================================================
class NCBI_XOBJREAD_EXPORT CMessageListenerBase : public objects::IMessageListener
//  ============================================================================
{
public:
    CMessageListenerBase() : m_pProgressOstrm(0) {};
    virtual ~CMessageListenerBase() {};
    
public:
    size_t
    Count() const { return m_Errors.size(); };
    
    virtual size_t
    LevelCount(
        EDiagSev eSev ) {
        
        size_t uCount( 0 );
        for ( size_t u=0; u < Count(); ++u ) {
            if ( m_Errors[u]->GetSeverity() == eSev ) ++uCount;
        }
        return uCount;
    };
    
    void
    ClearAll() { m_Errors.clear(); };
    
    const ILineError&
    GetError(
        size_t uPos ) const { 
            return *dynamic_cast<ILineError*>(m_Errors[ uPos ].get()); }
    
    virtual void Dump()
    {
        if (m_pProgressOstrm)
            Dump(*m_pProgressOstrm);
    }

    virtual void Dump(
        std::ostream& out )
    {
        if ( m_Errors.size() ) {
            TLineErrVec::iterator it;
            for ( it = m_Errors.begin(); it != m_Errors.end(); ++it ) {
                (*it)->Dump( out );
                out << endl;
            }
        }
        else {
            out << "(( no errors ))" << endl;
        }
    };

    virtual void DumpAsXML(
        std::ostream& out )
    {
        if ( m_Errors.size() ) {
            TLineErrVec::iterator it;
            for ( it = m_Errors.begin(); it != m_Errors.end(); ++it ) {
                (*it)->DumpAsXML( out );
                out << endl;
            }
        }
        else {
            out << "(( no errors ))" << endl;
        }
    };


    virtual void
    PutProgress(
        const string & sMessage,
        const Uint8 iNumDone,
        const Uint8 iNumTotal );

    /// This sets the stream to which progress messages are written.
    ///
    /// @param pProgressOstrm
    ///   The output stream for progress messages.  Set this to NULL to
    ///   stop writing progress messages.
    /// @param eNcbiOwnership
    ///   Indicates whether this CMessageListenerBase should own
    ///   pProgressOstrm.
    virtual void 
    SetProgressOstream(
        CNcbiOstream * pProgressOstrm,
        ENcbiOwnership eNcbiOwnership = eNoOwnership )
    {
        m_pProgressOstrm = pProgressOstrm;
        if( eNcbiOwnership == eTakeOwnership && pProgressOstrm ) {
            m_progressOstrmDestroyer.reset( pProgressOstrm );
        } else {
            m_progressOstrmDestroyer.reset();
        }
    }

private:
    // private so later we can change the structure if
    // necessary (e.g. to have indexing and such to speed up
    // level-counting)
    // typedef std::vector< AutoPtr<ILineError> > TLineErrVec;

    using TLineErrVec = vector<AutoPtr<IObjtoolsMessage>>;
    TLineErrVec m_Errors;


    // The stream to which progress messages are written.
    // If NULL, progress messages are not written.
    CNcbiOstream * m_pProgressOstrm;

    // do not read this pointer.  It's just used to make
    // sure m_pProgressOstrm is destroyed if we own it.
    AutoPtr<CNcbiOstream> m_progressOstrmDestroyer;

protected:

    // Child classes should use this to store errors
    // into m_Errors
    void StoreError(const ILineError& err)
    {
        m_Errors.emplace_back(err.Clone());
    }

    void StoreMessage(const IObjtoolsMessage& message)
    {
        m_Errors.emplace_back(dynamic_cast<IObjtoolsMessage*>(message.Clone()));
    }
};

//  ============================================================================
class CMessageListenerLenient:
//
//  Accept everything.
//  ============================================================================
    public CMessageListenerBase
{
public:
    CMessageListenerLenient() {};
    ~CMessageListenerLenient() {};

    bool
    PutMessage(
        const IObjtoolsMessage& message)
    {
        StoreMessage(message);
        return true;
    }
    
    bool
    PutError(
        const ILineError& err ) 
    {
        return PutMessage(err);
    };
};        

//  ============================================================================
class CMessageListenerStrict:
//
//  Don't accept any errors, at all.
//  ============================================================================
    public CMessageListenerBase
{
public:
    CMessageListenerStrict() {};
    ~CMessageListenerStrict() {};
    
    bool 
    PutMessage(
        const IObjtoolsMessage& message)
    {
        StoreMessage(message);
        return false;
    }

    bool
    PutError(
        const ILineError& err ) 
    {
        return PutMessage(err);
    };
};        

//  ===========================================================================
class CMessageListenerCount:
//
//  Accept up to <<count>> errors, any level.
//  ===========================================================================
    public CMessageListenerBase
{
public:
    CMessageListenerCount(
        size_t uMaxCount ): m_uMaxCount( uMaxCount ) {};
    ~CMessageListenerCount() {};
   
    bool PutMessage(
        const IObjtoolsMessage& message)
    {
        StoreMessage(message);
        return (Count() < m_uMaxCount);
    }


    bool
    PutError(
        const ILineError& err ) 
    {
        return PutMessage(err);
    };    
protected:
    size_t m_uMaxCount;
};

//  ===========================================================================
class CMessageListenerLevel:
//
//  Accept evrything up to a certain level.
//  ===========================================================================
    public CMessageListenerBase
{
public:
    CMessageListenerLevel(
        int iLevel ): m_iAcceptLevel( iLevel ) {};
    ~CMessageListenerLevel() {};
   
    bool 
    PutMessage(
        const IObjtoolsMessage& message) 
    {
        StoreMessage(message);
        return (message.GetSeverity() <= m_iAcceptLevel);
    }

    bool
    PutError(
        const ILineError& err ) 
    {
        return PutMessage(err);
    };    
    protected:
        int m_iAcceptLevel;
    };

    //  ===========================================================================
    class CMessageListenerWithLog:
    //
    //  Accept everything, and besides storing all errors, post them.
//  ===========================================================================
    public CMessageListenerBase
{
public:
    CMessageListenerWithLog(const CDiagCompileInfo& info)
        : m_Info(info) {};
    ~CMessageListenerWithLog() {};

    bool
    PutError(
        const ILineError& err )
    {
        CNcbiDiag(m_Info, err.Severity(),
                  eDPF_Log | eDPF_IsMessage).GetRef()
           << err.Message() << Endm;

        StoreError(err);
        return true;
    };

private:
    const CDiagCompileInfo m_Info;
};


//  ===========================================================================
class NCBI_XOBJREAD_EXPORT CGPipeMessageListener :
//  ===========================================================================
    public CMessageListenerBase 
{
public:
    CGPipeMessageListener(bool ignoreBadModValue=false);

    bool PutError(const ILineError& err) override final;
private:
    bool m_IgnoreBadModValue;
};

END_SCOPE(objects)

END_NCBI_SCOPE

#endif // OBJTOOLS_READERS___MESSAGELISTENER__HPP

