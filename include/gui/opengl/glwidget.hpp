#ifndef GUI_OPENGL___GL_WIDGET__HPP
#define GUI_OPENGL___GL_WIDGET__HPP

/*  $Id: glwidget.hpp 14562 2007-05-18 11:48:21Z dicuccio $
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
 * Authors:  Peter Meric
 *
 * File Description:
 *     CGlWidget
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui.hpp>
#include <gui/opengl/glevent.hpp>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE


class NCBI_GUIOPENGL_EXPORT CGlWidget : public CObject
{
public:
    CGlWidget(bool visible = true);
    virtual ~CGlWidget();

    virtual void Draw(void) = 0;
    virtual void ProcessEvent(CGlEvent& event) = 0;

    virtual void SetVisible(bool visible = true);
    virtual bool IsVisible(void) const;


private:
    bool m_Visible;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GL_WIDGET__HPP
