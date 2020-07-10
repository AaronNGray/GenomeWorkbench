#ifndef GUI_WIDGETS_DATA___QUERY_PANEL_EVENT__HPP
#define GUI_WIDGETS_DATA___QUERY_PANEL_EVENT__HPP

/*  $Id: query_panel_event.hpp 40258 2018-01-18 19:45:07Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>

#include <gui/utils/event.hpp>

BEGIN_NCBI_SCOPE

class CQueryPanelEvent : public CEvent
{
public:
    enum EEventType {
        eStatusChange
    };

    CQueryPanelEvent(const string& status)
        : CEvent(eStatusChange), m_Status(status) {}

    const string& GetStatus() const { return m_Status; }

private:
    string m_Status;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_DATA___QUERY_PANEL_EVENT__HPP
