#ifndef GUI_WIDGETS_GL_WX_DEMO___GLPANEWIDGET_DEMO__HPP
#define GUI_WIDGETS_GL_WX_DEMO___GLPANEWIDGET_DEMO__HPP


/*  $Id: glpanewidget_demo.hpp 23394 2011-03-18 18:36:24Z falkrb $
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

#include <corelib/ncbistd.hpp>

#include <gui/opengl.h>
#include <wx/scrolbar.h>
#include <gui/widgets/gl/gl_widget_base.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CGlWidgetDemo
class CGlWidgetDemo : public CGlWidgetBase
{
    DECLARE_EVENT_TABLE();
public:
    /// Constructors
    CGlWidgetDemo(wxWindow* parent, wxWindowID id = wxID_ANY);
    virtual ~CGlWidgetDemo();

    virtual CGlPane&    GetPort();
    virtual const CGlPane&    GetPort() const;

    void OnContextMenu(wxContextMenuEvent& event);

    virtual void    SetScale(TModelUnit scale, const TModelPoint& point);

protected:
    // CGlWidgetBase overridables
    virtual void x_CreatePane();
    virtual void x_SetPortLimits();

protected:
    CGlPane  m_Port;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_GL_WX_DEMO___GLPANEWIDGET_DEMO__HPP
