#ifndef GUI_WIDGET_SEQ_GRAPHICS___RULER_PANEL__HPP
#define GUI_WIDGET_SEQ_GRAPHICS___RULER_PANEL__HPP

/*  $Id: ruler_panel.hpp 36670 2016-10-21 14:09:34Z evgeniev $
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
 * Authors:  Liangshou Wu
 *
 */

#include <gui/gui.hpp>
#include <gui/widgets/gl/ruler.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <gui/opengl/gltexturefont.hpp>

BEGIN_NCBI_SCOPE


class CGlPane;
class CSeqGraphicConfig;

///////////////////////////////////////////////////////////////////////////////
///   CRulerPanel -- the ruler panel class
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CRulerPanel
    : public CObject
{
public:
    CRulerPanel();
    virtual ~CRulerPanel();

    void SetSeqLength(TSeqPos len);
    void SetOrientation(bool horz, bool flip);
    void SetShown(bool f);
    bool IsShown() const { return m_Shown; };

    TVPPoint  GetSize(int max_num = 0) const;

    int  GetMainRulerLabelStep() const;

    void SetRulerSeqStart(TSeqPos pos);

    bool HitTest(const TModelPoint& p);

    void Render(CGlPane& pane);
    void GetHTMLActiveAreas(CSeqGlyph::TAreaVector* p_areas) const;

    void LoadSettings(const CSeqGraphicConfig& g_config);
    void SaveSettings() const;


private:
    void x_ReversePane(CGlPane& pane) const;

    void x_UpdateMappingRanges();
    void x_InitRulerSpecs();

private:
    CRuler              m_MainRuler;

    /// extra ruler.
    /// The extra ruler is available only when reseting the origin.
    /// It appears right under the main ruler, and count up on the right
    /// always regardless of the flip strand mode.
    auto_ptr<CRuler>    m_ExtraRuler;
    auto_ptr<CRuler>    m_ExtraNegRuler;
    
    /// the total sequence length in bases.
    TSeqPos             m_SeqLength;

    TSignedSeqPos       m_RulerStart;
    bool                m_Flipped;
    bool                m_Horz;
    bool                m_Shown;

    /// @name configuration
    /// @{
    CRgbaColor      m_BG;
    CRgbaColor      m_GridColor;
    CRgbaColor      m_LabelColor;
    CGlTextureFont  m_LabelFont;
    CGlTextureFont  m_SmallLabelFont;    
    int             m_MajorTickSize;
    int             m_MinorTickSize;
    int             m_LabelTickSize;
    int             m_OppMajorTickSize;
    int             m_OppMinorTickSize;
    int             m_OppLabelTickSize;
    CRuler::ELabelAlign  m_LabelAln;
    /// @}
};


///////////////////////////////////////////////////////////////////////////////
///   CRulerPanel inline methods
inline
void CRulerPanel::SetSeqLength(TSeqPos len)
{
    m_SeqLength = len;
    x_UpdateMappingRanges();
}

inline
int CRulerPanel::GetMainRulerLabelStep() const
{ return m_MainRuler.GetLabelStep(); }

inline
void CRulerPanel::SetShown(bool f)
{
    m_Shown = f;
}

END_NCBI_SCOPE


#endif  // GUI_WIDGET_SEQ_GRAPHICS___RULER_PANEL__HPP
