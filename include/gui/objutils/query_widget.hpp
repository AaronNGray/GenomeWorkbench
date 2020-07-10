#ifndef GUI_OBJUTILS___QUERY_WIDGET__HPP
#define GUI_OBJUTILS___QUERY_WIDGET__HPP

/*  $Id: query_widget.hpp 37201 2016-12-14 21:00:47Z falkrb $
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
 * Authors:  Robert Falk
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/string.h>

BEGIN_NCBI_SCOPE

class CMacroQueryExec;

///////////////////////////////////////////////////////////////////////////////
/// IQueryWidget
///
/// This are the interface requirements for a widget that wants to support the
/// query interface currently defined by CQueryParsePanel.  Interface allows
/// query toolbar to tell widget to show/hide/iterate from the query selection
/// set, and also to adjust the UI for the period while the query is running.
class IQueryWidget {
public:
    IQueryWidget() {}
    virtual ~IQueryWidget() {}

    /// Return web-address for the help page that disusses help for the 
    /// specific widget.  The default ("") will hide the  help icon.
    virtual string GetSearchHelpAddr() const 
    { return string(""); }

    /// Disable any widgets and/or set cursor/scroll as needed when query starts
    virtual void QueryStart() {}
    /// Re-enable widgets and/or set cursor/scroll as needed when query ends
    virtual void QueryEnd(CMacroQueryExec* exec) {}

    /// go to next(1), previous(-1) or first(0) element from most recent query
    virtual void IterateSelection(int /* dir */) {}
    /// set to yes to show all queried elements as selected
    virtual void SetSelectAll(bool /* b */) {}
    /// set to true to hide/obscure elements not selected by most recent query
    virtual void SetHideUnselected(bool /* b */) {}

    /// let widget send fake resize to fix windows opengl bug. Subclass this
    /// if widget/pane want to ignore the associated resize event.
    virtual void DlgOverlayFix(wxWindow* win) { DlgGLWinOverlayFix(win); }
};


END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___QUERY_WIDGET__HPP
