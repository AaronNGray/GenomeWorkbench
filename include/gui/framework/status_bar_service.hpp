#ifndef GUI_FRAMEWORK___STATUS_BAR_SERVICE__HPP
#define GUI_FRAMEWORK___STATUS_BAR_SERVICE__HPP

/*  $Id: status_bar_service.hpp 23934 2011-06-22 18:14:13Z wuliangs $
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
 *      IStatusBarService - Status Bar Service interface declaration.
 */

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbistd.hpp>

class wxWindow;
class wxString;

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// IStatusBarService

/// Status Bar Service defines several standard fields:
///
/// 1. Message field – main text fields for showing text messages about
/// running operations, application events, menu bar hints etc.
/// 2. Task field – shows pictogram and brief information about background
/// application tasks.

/// Message field can display several kinds of messages:
/// - Status Message – text describing current application state (such as the
///   name of the task being executed)
/// - Event Message – information about an event that happened recently (task
///   finished). Event Message is displayed for a short period of time, it
///   overrides Status Messages.
/// - Hint – help string for the menu item or toolbar button being selected.
///   A Hint is shown only as long as mouse hovers over the item, overrides
////  Status Messages and Event Messages.

class IEventRecord;

class IStatusBarService
{
public:
    virtual ~IStatusBarService() {}

    /// message guard - helper class for resetting messages
    class CMsgGuard {
        CMsgGuard(const string& /*message*/)    {}
    };

    virtual void    SetStatusMessage(const string& msg) = 0;

    virtual void    AddEventMessage(const IEventRecord& record) = 0;

    virtual void    ShowHintMessage(const string& msg) = 0;
    virtual void    HideHintMessage() = 0;

    virtual void    ShowStatusProgress(int value, int range) = 0;
    virtual void    ShowStatusProgress() = 0;
    virtual void    HideStatusProgress() = 0;

    virtual void    InsertSlot(int index, wxWindow* slot, int width = 50) = 0;
    virtual wxWindow*   RemoveSlot(int index) = 0;
    virtual void    SetStatusText(const wxString& text, int index) = 0;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___STATUS_BAR_SERVICE__HPP

