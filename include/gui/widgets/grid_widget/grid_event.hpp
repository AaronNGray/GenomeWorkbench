#ifndef GUI_WIDGETS_GRID_WIDGET__GRID_EVENT__HPP
#define GUI_WIDGETS_GRID_WIDGET__GRID_EVENT__HPP

/*  $Id: grid_event.hpp 40262 2018-01-18 20:27:56Z katargir $
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
#include <gui/objutils/table_data.hpp>

BEGIN_NCBI_SCOPE

class CGridWidgetEvent : public CEvent
{
public:
    enum EEventType {
        eUrlHover,
        eRowClick,
        eRowDClick,
        eRefreshData,

        eUnknownEvent = -1
    };

    CGridWidgetEvent(TEventID eid)
        : CEvent(eid)
    {
    }

    CGridWidgetEvent(TEventID eid, const wxString& url)
        : CEvent(eid), m_URL(url)
    {
    }

    int GetModifiers() { return m_Modifiers; }
    void SetModifiers(int modifiers) { m_Modifiers = modifiers; }

    int GetSelectedRow() { return m_SelectedRow; }
    void SetSelectedRow(int row) { m_SelectedRow = row; }

    const wxString& GetURL() const { return m_URL; }

private:
    // bit mask of all pressed modifier keys, used for eRowClick in Validate View
    int m_Modifiers;
    // used for eRowClick and eRowDClick
    int m_SelectedRow;

    wxString m_URL;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_GRID_WIDGET__GRID_EVENT__HPP
