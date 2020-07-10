#ifndef __GUI_WIDGETS_GL___PROGRESS_PANEL__HPP
#define __GUI_WIDGETS_GL___PROGRESS_PANEL__HPP

/*  $Id: progress_panel.hpp 31681 2014-11-05 14:32:56Z falkrb $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely avaislable
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

#include <gui/widgets/gl/irenderable.hpp>

#include <gui/opengl/gltexturefont.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
///
class  NCBI_GUIWIDGETS_GL_EXPORT  CProgressPanel
    : public CRenderableImpl
{
public:
    CProgressPanel();
    virtual ~CProgressPanel();

    virtual void    Update(float progress, const string& msg);

    /// @name IRenderable implementation
    /// @{
    virtual void Render(CGlPane& pane);

    virtual TVPPoint    PreferredSize();
    /// @}

protected:
    CGlTextureFont   m_Font;

    float   m_Progress;
    string  m_Message;
};


END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_GL___PROGRESS_PANEL__HPP
