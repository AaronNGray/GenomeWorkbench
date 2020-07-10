#ifndef GUI_OBJUTILS___FEATURE_CONFIG__HPP
#define GUI_OBJUTILS___FEATURE_CONFIG__HPP

/* $Id: feature_conf.hpp 34709 2016-02-03 16:53:15Z evgeniev $
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
 * Author:  Liangshou Wu
 *
 */

 /**
 * File Description:
 * configuration class for everything needed to render a single feature
 */


#include <gui/gui.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/utils/rgba_color.hpp>

BEGIN_NCBI_SCOPE

//TModelUnit GetTotalHeight(bool show_product, int ruler,
//    bool is_selected, bool label_override, bool can_label_fit = true);


///////////////////////////////////////////////////////////////////////////////
/// CCustomFeatureColor:
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CCustomFeatureColor : public CObject
{
public:
    typedef vector<CRgbaColor>  TColorCode;

    enum EColorIdex {
        eDefault = 0,  /// feature's default color based on feature type
        eLowIdentity,
        eMidIdentity,
        eHighIdentity,
        eCNV,
        eInversion,
        eLoss,
        eInsertion,
        eGain,
        eComplex,
        eUnknown,
        eEverted,
        eTranschr,
        eDeletionInsertion,
        eMax
    };

    CCustomFeatureColor()
    {
        m_Colors.resize(eMax);
    }

    void SetColor(int idx, const CRgbaColor& color)
    {
        if (idx >= 0  &&  (unsigned)idx < m_Colors.size()) {
            m_Colors[idx] = color;
        }
    }

    const TColorCode& GetColorCode() const
    {
        return m_Colors;
    }

private:
    TColorCode m_Colors;
};


///////////////////////////////////////////////////////////////////////////////
/// CFeatureParams: the data structure holding feature rendering parameters.
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CFeatureParams : public CObject
{
public:
    enum EDecoElements {
        eLabelPosition = 0,
        eHeadStyle,
        eTailStyle,
        eLineStyle,
        eConnection
    };

    /// strand indicator at the beginning.
    enum EHeadStyle {
        eHead_No,
        eHead_Triangle,
        eHead_NeckedTriangle,
        eHead_Arrow,
        eHead_TickMark
    };

    enum ETailStyle {
        eTail_No,
        eTail_Square,
        eTail_Circle,
        eTail_Fletched
    };

    enum ELabelPosition {
        ePos_Above,   ///< above the rendered bar
        ePos_Inside,  ///< inside the rendered bar
        ePos_Side,    ///< always on 5' side
        ePos_NoLabel  ///< no label
    };

    /// how feature border and exons connections are drawn.
    enum ELineStyle {
        eLine_Solid,
        eLine_Dashed,
        eLine_Dotted,
        eLine_DashDotted,
        eLine_ShortDashed
    };

    /// type of exon connection lines.
    enum EConnection {
        eStraightLine,
        eCantedLine,
        eFilledBox,
        eBox
    };

	/// how the box of the feature is drawn
    enum EBoxStyle {
        eBox_Filled,
        eBox_Hollow,
        eBox_Line,            ///< No box - just a line

        /// additional decoration for the box
        eBox_Insertion,       ///< add insertion triangles in the midst of the box
        eBox_Deletion         ///< add an inverted triangle to mark a deletion
    };

public:
    CFeatureParams()
        : m_CgiMode(false)
        , m_ShowNtRuler(false)
        , m_ShowAaRuler(false)
        , m_ShowStrandIndicator(true)
        , m_LabelPos(ePos_Side)
        , m_HeadStyle(eHead_No)
        , m_TailStyle(eTail_No)
        , m_LineStyle(eLine_Solid)
        , m_Connections(eStraightLine)
		, m_BoxStyle(eBox_Filled)
        , m_BarHeight(7.0)
        , m_OverviewFactor(0.5)
        , m_HeadHeight(9.0)
        , m_TailHeight(1.0)
        , m_LineWidth(2.0)
    {}


    static int DecoStrToValue(const string& s);
    static const string& DecoValueToStr(EDecoElements e, int v);

    TModelUnit GetBarHeight(bool overview) const;

public:

    bool m_CgiMode;

    /// @name content flags.
    /// @{
    bool    m_ShowNtRuler;
    bool    m_ShowAaRuler;
    bool    m_ShowStrandIndicator;
    /// @}

    /// @name style elements.
    /// @{
    ELabelPosition  m_LabelPos;
    EHeadStyle      m_HeadStyle;
    ETailStyle      m_TailStyle;
    ELineStyle      m_LineStyle;
    EConnection     m_Connections;
    /// @}

	EBoxStyle		m_BoxStyle;

    /// @name Colors.
    /// @{
    CRgbaColor m_fgColor;
    CRgbaColor m_bgColor;
    CRgbaColor m_LabelColor;
    // For introns only (SV-2124)
    CRgbaColor m_NonConsensus;
    /// @}

    /// @name Sizes.
    /// @{
    TModelUnit m_BarHeight;     ///< absolute size (in pixel)
    TModelUnit m_OverviewFactor;///< ratio to bar height
    TModelUnit m_HeadHeight;    ///< ratio to bar height
    TModelUnit m_TailHeight;    ///< ratio to bar height
    TModelUnit m_LineWidth;     ///< absolute value (in pixel)
    /// @}

    CGlTextureFont   m_LabelFont;

    /// custom color code for features with a special attribute.
    CRef<CCustomFeatureColor> m_CustomColors;
};


///////////////////////////////////////////////////////////////////////////////
/// CClonePlacementParams:
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CClonePlacementParams : public CObject
{
public:
    typedef CFeatureParams::ELabelPosition TLabelPosition;
    CClonePlacementParams()
        : m_LabelPos(CFeatureParams::ePos_Side)
        , m_BarHeight(6.0)
        , m_HeadHeight(8.0)
        , m_OverviewFactor(0.5)
    {}

    TLabelPosition  m_LabelPos;
    CGlTextureFont   m_LabelFont;

    /// @name Colors.
    /// @{
    CRgbaColor m_ConcordantColor;
    CRgbaColor m_DiscordantColor;
    CRgbaColor m_UnknownConcordancyColor;
    CRgbaColor m_HighlightColor;
    CRgbaColor m_CombinedPlacementColor;
    CRgbaColor m_FakeEndColor;
    CRgbaColor m_LabelColor;
    /// @}

    /// @name Sizes.
    /// @{
    TModelUnit m_BarHeight;     ///< absolute size (in pixel)
    TModelUnit m_HeadHeight;    ///< absolute size (in pixel)
    TModelUnit m_OverviewFactor;///< ratio to bar height
    /// @}
};



END_NCBI_SCOPE

#endif // GUI_OBJUTILS___FEATURE_CONFIG__HPP
