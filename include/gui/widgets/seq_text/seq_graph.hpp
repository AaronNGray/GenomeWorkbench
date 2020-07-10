#ifndef __GUI_WIDGETS_SEQ_TEXT___SEQ_GRAPH__HPP
#define __GUI_WIDGETS_SEQ_TEXT___SEQ_GRAPH__HPP

/*  $Id: seq_graph.hpp 31714 2014-11-07 18:44:50Z falkrb $
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
 * Authors:  Colleen Bollin (adapted from a file by Andrey Yazhuk)
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbistd.hpp>

#include <util/range_coll.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/opengl/glpane.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/widgets/gl/irenderable.hpp>
#include <gui/widgets/seq_text/seq_text_defs.hpp>
#include <gui/widgets/seq_text/seq_text_ds.hpp>
#include <gui/widgets/seq_text/seq_text_conf.hpp>
#include <gui/widgets/seq_text/seq_text_geometry.hpp>

#include <objmgr/bioseq_handle.hpp>
#include <objmgr/seq_vector.hpp>

BEGIN_NCBI_SCOPE

class ISeqTextGeometry;
class ISequenceTextGraphHost;
class CSeqTextDataSource;

///////////////////////////////////////////////////////////////////////////////
/// CSequenceTextGraph
class   CSequenceTextGraph : public CRenderableImpl
{
public:

    typedef CRangeCollection<TSeqPos>   TRangeColl;
    typedef vector<CRgbaColor*> TColorVector;

    CSequenceTextGraph();
    void    SetFont(CGlTextureFont* seq_font);
    TModelUnit  GetTextHeight() { return m_TextHeight; };
    TModelUnit  GetTextOffset() { return m_TextOffset; };

    virtual void SetDataSource(CSeqTextDataSource* p_ds);
    void         SetConfig(CSeqTextConfig *p_cfg);
    void         SetHost(ISequenceTextGraphHost* pHost);
    void         SetGeometry(ISeqTextGeometry* pGeometry);

    void    ShowLabel(bool show);

    /// @name IRenderable implemenation
    /// @{
    virtual void        Render(CGlPane& pane);

    virtual TModelRect  GetModelRect() const;
    virtual void        SetModelRect(const TModelRect& rc);

    virtual TVPPoint    PreferredSize();

    virtual bool    NeedTooltip(CGlPane& pane, int vp_x, int vp_y);
    virtual string  GetTooltip();
    /// @}

protected:

    void x_BoxCharacters (ncbi::CSeqTextDefs::TSpliceSiteVector splice_sites,
                          TSeqPos seq_start, TSeqPos num_chars_per_row,
                          TModelUnit offset_X, TModelUnit offset_Y);
    void x_GetColors (CSeqTextDefs::TSubtypeVector &subtypes, TColorVector &colors, TSeqPos seq_start, TSeqPos num_chars_per_row);
    void x_DrawIntervalBreaks (TModelUnit x, TModelUnit y, CSeqTextDefs::TSeqPosVector &breaks, TSeqPos seq_start, TSeqPos num_chars_per_row);
    void x_DrawVariations (TSeqPos seq_start, TSeqPos num_chars_per_row, float scale_x, float scale_y, CSeqTextDefs::TVariationGraphVector &variations);
    void x_RenderSequence(CGlPane& pane);

    CRef<CGlTextureFont> m_pSeqFont;

    CRgbaColor    m_BackColor;
    CRgbaColor    m_TextColor;
    CRgbaColor    m_FeatColor;
    CRgbaColor    m_SpliceBoxColor;

    TModelUnit  m_TextHeight;
    TModelUnit  m_TextOffset;

    CRef<CSeqTextDataSource> m_DataSource;
    CRef<CSeqTextConfig> m_Config;
    ISequenceTextGraphHost* m_pHost;
    ISeqTextGeometry*  m_pGeometry;
};

END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_SEQ_TEXT___SEQ_GRAPH__HPP
