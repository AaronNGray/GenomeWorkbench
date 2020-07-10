#ifndef __GUI_WIDGETS_GL___IEVENT_HANDLER__HPP
#define __GUI_WIDGETS_GL___IEVENT_HANDLER__HPP

/*  $Id: ievent_handler.hpp 30858 2014-07-31 14:05:43Z ucko $
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

#include <corelib/ncbistl.hpp>
#include <corelib/ncbistd.hpp>

#include <gui/opengl/glpane.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/event.h>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// IGlEventHandler
class NCBI_GUIWIDGETS_GL_EXPORT IGlEventHandler
{
public:
    virtual ~IGlEventHandler()    {};

    virtual void    SetPane(CGlPane* /*pane*/) { _ASSERT(false); };
    virtual wxEvtHandler*    GetEvtHandler() = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// IGenericHandlerHost
class NCBI_GUIWIDGETS_GL_EXPORT IGenericHandlerHost
{
public:
    /// redraws the Host and the handler
    virtual void    GHH_Redraw() = 0;

    /// changes the cursor in the hosting window
    virtual void    GHH_SetCursor(const wxCursor& cursor) = 0;

    /// captures mouse events in the hosting window for D&D
    virtual void    GHH_CaptureMouse() = 0;

    /// releases captured mouse
    virtual void    GHH_ReleaseMouse() = 0;

    virtual ~IGenericHandlerHost()   {};
};


END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_GL___IEVENT_HANDLER__HPP
