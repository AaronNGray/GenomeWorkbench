#ifndef GUI_FRAMEWORK___EVENT_LOG_SERVICE_IMPL__HPP
#define GUI_FRAMEWORK___EVENT_LOG_SERVICE_IMPL__HPP

/*  $Id: event_log_service_impl.hpp 35258 2016-04-18 01:35:40Z whlavina $
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

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>
#include <gui/framework/event_log_service.hpp>
#include <gui/framework/service.hpp>

#include <gui/utils/event_handler.hpp>

#include <deque>

BEGIN_NCBI_SCOPE

class CLogServiceEvent : public CEvent
{
public:
    enum EEventId {
        eNewRecord
    };

    CLogServiceEvent() : CEvent(eNewRecord) {}
};


///////////////////////////////////////////////////////////////////////////////
/// CEventLogService - the standard implementation of IEventLogService.
/// see IEventLogService interface for information.
class  NCBI_GUIFRAMEWORK_EXPORT  CEventLogService :
    public CObjectEx,
    public IEventLogService,
    public IService,
    public CEventHandler
{
public:
    CEventLogService();

    /// @name IService interface implementation
    /// @{
    virtual void    InitService();
    virtual void    ShutDownService();
    /// @}

    /// @name IEventLogService interface implementation
    /// @{
    virtual void    AddRecord(IEventRecord* record);

    virtual size_t     GetRecordsCount() const;
    virtual const IEventRecord*   GetRecord(size_t index) const;
    /// @}

protected:
    typedef CIRef<IEventRecord>   TIRef;
    typedef deque<TIRef>    TRecords;

    TRecords    m_Records;
};


///////////////////////////////////////////////////////////////////////////////
/// CEventRecord - standard implementation of IEventRecord.
/// CEventRecord must be subclassed to implement GetAction().
class  NCBI_GUIFRAMEWORK_EXPORT  CEventRecord :
    public CObject,
    public IEventRecord
{
public:
    CEventRecord(EType type,
                 const string& title = kEmptyStr,
                 const string& descr = kEmptyStr,
                 const TTime& time = 0);

    /// @name IEventRecord implementation
    /// @{
    virtual EType   GetType() const;
    virtual string  GetTitle() const;
    virtual string  GetDescription() const;
    virtual const TTime&    GetTime() const;
    /// @}

    virtual void    SetType(EType type);
    virtual void    SetTitle(const string& title);
    virtual void    SetDescription(const string& descr);
    virtual void    SetTime(const TTime& time);

protected:
    EType   m_Type;
    string  m_Title;
    string  m_Description;
    TTime   m_Time;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___EVENT_LOG_SERVICE_IMPL__HPP

