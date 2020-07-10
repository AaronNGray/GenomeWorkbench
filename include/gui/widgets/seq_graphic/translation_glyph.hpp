#ifndef GUI_OBJUTILS___TRANSLATION_GLYPH__HPP
#define GUI_OBJUTILS___TRANSLATION_GLYPH__HPP

/*  $Id: translation_glyph.hpp 42821 2019-04-18 19:32:56Z joukovv $
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
 * File Description:
 */


#include <corelib/ncbiobj.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>


BEGIN_NCBI_SCOPE

class CTranslationConfig;

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CTranslationGlyph : public CSeqGlyph
{
public:
    enum EFrame {
        ePositive_One = 0,
        ePositive_Two,
        ePositive_Three,
        eNegative_One,
        eNegative_Two,
        eNegative_Three,
        eFrame_Max
    };

    typedef vector<char> TTransSeq;
    typedef vector<TSeqRange> TORFs;
    typedef vector<TSeqPos>   TCodons;

    // consturctor
    CTranslationGlyph(EFrame frame);

    CTranslationGlyph(TSeqPos start, TSeqPos stop, EFrame frame);

    /// @name CSeqGlyph virtual methods.
    /// @{
    virtual bool NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetHTMLActiveAreas(TAreaVector* p_areas) const;
    virtual bool IsClickable() const;
    ///@}

    void SetConfig(CTranslationConfig* config);

    TTransSeq& GetTranslation();
    const TTransSeq& GetTranslation() const;

    TORFs& GetOrfs();
    const TORFs& GetOrfs() const;

    TCodons& GetStartCodons();
    const TCodons& GetStartCodons() const;

    TCodons& GetStopCodons();
    const TCodons& GetStopCodons() const;

    void SetStart(TSeqPos start);
    void SetStop(TSeqPos stop);

    EFrame GetFrame() const;
    bool NegativeStrand() const;

protected:
    /// @name CSeqGlyph virtual methods.
    /// @{
    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();
    /// @}

private:
    void x_DrawSequence(TSeqPos start, TSeqPos stop, TSeqPos vis_from,
        TSeqPos vis_to, TModelUnit center_y, bool negative) const;
    
    bool x_Intersecting(TSeqPos f1, TSeqPos t1, TSeqPos f2, TSeqPos t2) const;
    string x_GetFrameLabel() const;

private:
    TTransSeq                   m_TransSeq; ///< translation sequence
    TORFs                       m_Orfs;     ///< Open reading frame list
    TCodons                     m_StartCodons;
    TCodons                     m_StopCodons;
    EFrame                      m_Frame;    ///< which frame
    CRef<CTranslationConfig>    m_Config;   ///< configuration for rendering
};


class CTranslationConfig : public CObject
{
public:
    CTranslationConfig() 
        : m_BarHeight(10)
        , m_OrfThreshold(20)
        , m_HighlightCodons(true)
        , m_ShowLabel(true)
    {}

    void SetStartCodonColor(const CRgbaColor& color);
    CRgbaColor& GetStartCodonColor();
    const CRgbaColor& GetStartCodonColor() const;

    void SetStopCodonColor(const CRgbaColor& color);
    CRgbaColor& GetStopCodonColor();
    const CRgbaColor& GetStopCodonColor() const;

    void SetOrfHighlightColor(const CRgbaColor& color);
    CRgbaColor& GetOrfHighlightColor();
    const CRgbaColor& GetOrfHighlightColor() const;

    void SetSeqColor(const CRgbaColor& color);
    CRgbaColor& GetSeqColor();
    const CRgbaColor& GetSeqColor() const;

    void SetOrfSeqColor(const CRgbaColor& color);
    CRgbaColor& GetOrfSeqColor();
    const CRgbaColor& GetOrfSeqColor() const;

    void SetLabelBgColor(const CRgbaColor& color);
    CRgbaColor& GetLabelBgColor();
    const CRgbaColor& GetLabelBgColor() const;

    void SetCommentColor(const CRgbaColor& color);
    CRgbaColor& GetCommentColor();
    const CRgbaColor& GetCommentColor() const;

    void SetSeqFont(CGlTextureFont::EFontFace font, unsigned int font_size);
    CGlTextureFont& GetSeqFont();
    const CGlTextureFont& GetSeqFont() const;

    void SetStrandFont(CGlTextureFont::EFontFace font, unsigned int font_size);
    CGlTextureFont& GetStrandFont();
    const CGlTextureFont& GetStrandFont() const;

    void SetBarHeight(int h);
    int GetBarHeight() const;

    void SetOrfThreshold(int val);
    int GetOrfThreshold() const;

    void SetHighlightCodons(bool f);
    bool GetHighlightCodons() const;

    void SetShowLabel(bool f);
    bool ShowLabel() const;

private:
    CRgbaColor      m_StartCodonColor;
    CRgbaColor      m_StopCodonColor;
    CRgbaColor      m_OrfHighlightColor;
    CRgbaColor      m_SeqColor;
    CRgbaColor      m_OrfSeqColor;
    CRgbaColor      m_LabelBgColor;
    CRgbaColor      m_CommentColor;
    CGlTextureFont  m_SeqFont;
    CGlTextureFont  m_StrandFont;
    int             m_BarHeight;
    int             m_OrfThreshold;
    bool            m_HighlightCodons;
    bool            m_ShowLabel;
};


///////////////////////////////////////////////////////////////////////////////
/// CTranslationGlyph inline methods
///
inline
void CTranslationGlyph::SetConfig(CTranslationConfig* config)
{ m_Config.Reset(config); }

inline
CTranslationGlyph::TORFs& CTranslationGlyph::GetOrfs()
{ return m_Orfs; }

inline
const CTranslationGlyph::TORFs& CTranslationGlyph::GetOrfs() const
{ return m_Orfs; }

inline
CTranslationGlyph::TCodons& CTranslationGlyph::GetStartCodons()
{ return m_StartCodons; }

inline
const CTranslationGlyph::TCodons& CTranslationGlyph::GetStartCodons() const
{ return m_StartCodons; }

inline
CTranslationGlyph::TCodons& CTranslationGlyph::GetStopCodons()
{ return m_StopCodons; }

inline
const CTranslationGlyph::TCodons& CTranslationGlyph::GetStopCodons() const
{ return m_StopCodons; }

inline
CTranslationGlyph::TTransSeq& CTranslationGlyph::GetTranslation()
{ return m_TransSeq; }

inline
const CTranslationGlyph::TTransSeq& CTranslationGlyph::GetTranslation() const
{ return m_TransSeq; }

inline
void CTranslationGlyph::SetStart(TSeqPos start)
{ SetLeft(start); }

inline
void CTranslationGlyph::SetStop(TSeqPos stop)
{
    if (stop < GetLeft()) {
        SetWidth(GetLeft() - stop);
        SetStart(stop);
    } else {
        SetWidth(stop - GetLeft());
    }
}

inline
CTranslationGlyph::EFrame CTranslationGlyph::GetFrame() const
{ return m_Frame; }

inline
bool CTranslationGlyph::NegativeStrand() const
{ return m_Frame / 3 == 1; }

///////////////////////////////////////////////////////////////////////////////
/// CTranslationConfig inline methods
///
inline
void CTranslationConfig::SetStartCodonColor(const CRgbaColor& color)
{ m_StartCodonColor = color; }

inline
CRgbaColor& CTranslationConfig::GetStartCodonColor()
{ return m_StartCodonColor; }

inline
const CRgbaColor& CTranslationConfig::GetStartCodonColor() const
{ return m_StartCodonColor; }

inline
void CTranslationConfig::SetStopCodonColor(const CRgbaColor& color)
{ m_StopCodonColor = color; }

inline
CRgbaColor& CTranslationConfig::GetStopCodonColor()
{ return m_StopCodonColor; }

inline
const CRgbaColor& CTranslationConfig::GetStopCodonColor() const
{ return m_StopCodonColor; }

inline
void CTranslationConfig::SetOrfHighlightColor(const CRgbaColor& color)
{ m_OrfHighlightColor = color; }

inline
CRgbaColor& CTranslationConfig::GetOrfHighlightColor()
{ return m_OrfHighlightColor; }

inline
const CRgbaColor& CTranslationConfig::GetOrfHighlightColor() const
{ return m_OrfHighlightColor; }

inline
void CTranslationConfig::SetSeqColor(const CRgbaColor& color)
{ m_SeqColor = color; }

inline
CRgbaColor& CTranslationConfig::GetSeqColor()
{ return m_SeqColor; }

inline
const CRgbaColor& CTranslationConfig::GetSeqColor() const
{ return m_SeqColor; }

inline
void CTranslationConfig::SetOrfSeqColor(const CRgbaColor& color)
{ m_OrfSeqColor = color; }

inline
CRgbaColor& CTranslationConfig::GetOrfSeqColor()
{ return m_OrfSeqColor; }

inline
const CRgbaColor& CTranslationConfig::GetOrfSeqColor() const
{ return m_OrfSeqColor; }

inline
void CTranslationConfig::SetLabelBgColor(const CRgbaColor& color)
{ m_LabelBgColor = color; }

inline
CRgbaColor& CTranslationConfig::GetLabelBgColor()
{ return m_LabelBgColor; }

inline
const CRgbaColor& CTranslationConfig::GetLabelBgColor() const
{ return m_LabelBgColor; }

inline
void CTranslationConfig::SetCommentColor(const CRgbaColor& color)
{ m_CommentColor = color; }

inline
CRgbaColor& CTranslationConfig::GetCommentColor()
{ return m_CommentColor; }

inline
const CRgbaColor& CTranslationConfig::GetCommentColor() const
{ return m_CommentColor; }

inline
void CTranslationConfig::SetSeqFont(CGlTextureFont::EFontFace font, unsigned int font_size)
{ m_SeqFont.SetFontFace(font); m_SeqFont.SetFontSize(font_size); }

inline
CGlTextureFont& CTranslationConfig::GetSeqFont()
{ return m_SeqFont; }

inline
const CGlTextureFont& CTranslationConfig::GetSeqFont() const
{ return m_SeqFont; }

inline
void CTranslationConfig::SetStrandFont(CGlTextureFont::EFontFace font, unsigned int font_size)
{ m_StrandFont.SetFontFace(font); m_StrandFont.SetFontSize(font_size); }

inline
CGlTextureFont& CTranslationConfig::GetStrandFont()
{ return m_StrandFont; }

inline
const CGlTextureFont& CTranslationConfig::GetStrandFont() const
{ return m_StrandFont; }

inline
void CTranslationConfig::SetBarHeight(int h)
{ m_BarHeight = h; }

inline
int CTranslationConfig::GetBarHeight() const
{ return m_BarHeight; }

inline
void CTranslationConfig::SetOrfThreshold(int val)
{ m_OrfThreshold = val; }

inline
int CTranslationConfig::GetOrfThreshold() const
{ return m_OrfThreshold; }

inline
void CTranslationConfig::SetHighlightCodons(bool f)
{ m_HighlightCodons = f; }

inline
bool CTranslationConfig::GetHighlightCodons() const
{ return m_HighlightCodons; }

inline
void CTranslationConfig::SetShowLabel(bool f)
{ m_ShowLabel = f; }

inline
bool CTranslationConfig:: ShowLabel() const
{ return m_ShowLabel; }

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___TRANSLATION_GLYPH__HPP
