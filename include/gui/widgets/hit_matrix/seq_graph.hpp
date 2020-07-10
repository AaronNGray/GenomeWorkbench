#ifndef __GUI_WIDGETS_HIT_MATRIX___SEQ_GRAPH__HPP
#define __GUI_WIDGETS_HIT_MATRIX___SEQ_GRAPH__HPP

/*  $Id: seq_graph.hpp 33149 2015-06-04 15:29:58Z falkrb $
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

#include <gui/utils/rgba_color.hpp>
#include <gui/opengl/glpane.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/widgets/gl/irenderable.hpp>

#include <objmgr/bioseq_handle.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CHitMatrixSeqGraph
class   CHitMatrixSeqGraph :
    public CObject,
    public CRenderableImpl
{
public:
    CHitMatrixSeqGraph(bool b_horz);
    void    SetFonts(CGlTextureFont* seq_font);

    void    SetBioseqHandle(objects::CBioseq_Handle& handle);

    void    ShowLabel(bool show);

    /// @name IRenderable implemenation
    /// @{
    virtual void        Render(CGlPane& pane);

    virtual TVPPoint    PreferredSize();

    virtual bool    NeedTooltip(CGlPane& pane, int vp_x, int vp_y);
    virtual string  GetTooltip();
    /// @}

protected:
    bool    m_bHorz;
    CRef<CGlTextureFont> m_pSeqFont;

    CRgbaColor    m_BackColor;
    CRgbaColor    m_TextColor;

    objects::CBioseq_Handle  m_Bioseq;
};


END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_HIT_MATRIX___SEQ_GRAPH__HPP
