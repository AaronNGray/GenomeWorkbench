#ifndef GUI_FRAMEWORK___EVENT_LOG_SERVICE__HPP
#define GUI_FRAMEWORK___EVENT_LOG_SERVICE__HPP

/*  $Id: event_log_service.hpp 31805 2014-11-17 16:56:57Z katargir $
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


BEGIN_NCBI_SCOPE

class IEventRecord;

///////////////////////////////////////////////////////////////////////////////
/// IEventLogService - records application events.
class IEventLogService
{
public:
    /// Records are managed by CIRef inside the service
    virtual void    AddRecord(IEventRecord* record) = 0;

    /// returns the total number of records
    virtual size_t GetRecordsCount() const = 0;

    /// returns the record with the specified index
    virtual const IEventRecord*   GetRecord(size_t index) const = 0;

    virtual ~IEventLogService() {}
};



///////////////////////////////////////////////////////////////////////////////
/// IEventRecord
///
/// IEventRecord - an interface representing an abstract record in Event Log.
/// IEventRecord provides descriptive information for an Event and, optionally,
/// an action that activates objects associated with the event.
///
/// Classes implementing IEventRecord are managed using CIRef and so should be
/// derived from CObject.

class IEventRecord
{
public:
    typedef time_t  TTime;

    enum    EType   {
        eInfo = 0,
        eError,
        eWarning,
        eLast
    };

    virtual ~IEventRecord()   {};

    virtual EType   GetType() const = 0;

    // returns a short (one line) textual description of the event
    virtual string  GetTitle() const = 0;

    // returns long (~ one paragraph) textual description of the event
    virtual string  GetDescription() const = 0;

    // get event time
    virtual const TTime&    GetTime() const = 0;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___EVENT_LOG_SERVICE__HPP

