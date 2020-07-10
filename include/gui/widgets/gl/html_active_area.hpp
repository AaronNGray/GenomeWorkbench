#ifndef __GUI_WIDGETS_GL___HTML_ACTIVE_AREA__HPP
#define __GUI_WIDGETS_GL___HTML_ACTIVE_AREA__HPP

/*  $Id: html_active_area.hpp 40099 2017-12-20 15:59:30Z shkeda $
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

#include <gui/opengl/glpane.hpp>
#include <gui/utils/track_info.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/objects/TrackConfig.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_GL_EXPORT CHTMLActiveArea
{
public:
    /// we will need to deprecate EHtmlAreaType eventually.
    enum EHtmlAreaType {
        eInvalid = -1,
        eHtmlArea_Link = 0,
        eHtmlArea_CheckBox,
        eHtmlArea_Feature,
        eHtmlArea_Gene,
        eHtmlArea_Cdregion,
        eHtmlArea_Protein,
        eHtmlArea_Trace,
        eHtmlArea_SNP,
        eHtmlArea_STS,
        eHtmlArea_SeqComponent,
        eHtmlArea_Alignment,
        eHtmlArea_SeqGraph,
        eHtmlArea_Comment,
        eHtmlArea_Histogram,
        eHtmlArea_Track,
        eHtmlArea_Nonselectable
        // add additional types at the bottom always
    };

    /// html active area flags.
    /// An area may have multiple flags set.
    enum FAreaType {
        fLink            = 1 << 0, ///< a direct link stored in m_Action
        fCheckBox        = 1 << 1, ///< a toggle button/icon
        fNoSelection     = 1 << 2, ///< the object can't be selected
        fClientSelection = 1 << 3, ///< the selection can be done on client
        fNoHighlight     = 1 << 4, ///< no highlighting on mouse over
        fNoTooltip       = 1 << 5, ///< do not request and show tooltip
        fTooltipEmbedded = 1 << 6, ///< tooltip embedded
        fTrack           = 1 << 7, ///< track title bar
        fRuler           = 1 << 8, ///< ruler bar
        fEditable        = 1 << 9, ///< can be modified (isca browser feature editing only)
        fNoPin           = 1 << 10,///< tooltip is not pinnable
        fIgnorable       = 1 << 11,///< feature can be ignored (isca browser feature editing only)
        fSequence        = 1 << 12,///< sequence track flag
        fComment         = 1 << 13,///< render a label/comment on client side
        fDrawBackground  = 1 << 14,///< highlight background for this area
        //Dirt             = 1 << 16, used internally by sviewer.js
        fNoNavigation    = 1 << 17,///< track title won't show navigation controls
        fLegendItem      = 1 << 18,
        fNoCaching       = 1 << 19, ///< The tooltip for this feature should not be cached
        fZoomOnDblClick  = 1 << 20  ///< Zoom on Double click
      };

    TSeqRange m_SeqRange;
    TVPRect m_Bounds;
    int     m_Type;
    int     m_Flags;  ///< area flags, will need to replace m_Type
    bool    m_PositiveStrand; ///< the default is true
    string  m_ID;     ///< area identifier
    string  m_Descr;  ///< description that can be used as label or tooltip
    string  m_Color; ///< optional area color (used for legen items)
    string  m_Action; ///< identifies action to be taken
    string  m_DB_Name;
    string  m_DB_ID;
    string  m_ParentId; ///< parent entity idendifier, such as track_id

    string  m_Signature;  // AAAAAAAA-BBBBBBBB-CCCCCCCC-DDEE-FFFFFFFF-GGGGGGGG
    // where:
    // AAAAAAAA = sequence id of sequence on which object lives
    // BBBBBBBB-CCCCCCCC = total bounded range (from-to) of object
    // DD       = type code for object type (alignment, feature, graph, subsequence, etc.)
    // EE       = subtype code for object (i.e., feature subtype)
    // FFFFFFFF = object fingerprint
    // GGGGGGGG = hash code for the data source that the object is associated. (optional)

    // track rendering result information is optional and is set only if
    // m_Flags & fTrack == true
    TTrackErrorStatus m_TrackErrorStatus;

    // Number of merged features. Optional, set only if m_Flags & fMergedFeats == true
    unsigned m_MergedFeatsCount;

    CRef<objects::CTrackConfig>  m_Config; ///< Area specific configuration settings

    CHTMLActiveArea();
    CHTMLActiveArea(int type, const TVPRect& bounds, const string& id,
                    const string& descr, const string& action);
};


/// Simple class to represent hairlines drawn on top of seqgraphic images. This is only
/// needed for CGI rendering which may choose to render tracks separately and compoiste
/// them on the client.  This requires explicit drawing instructions for hairlines (or 
/// anything else drawn over multiple tracks).
class NCBI_GUIWIDGETS_GL_EXPORT CSeqGraphicHairline {
public:
    CSeqGraphicHairline() 
        : m_HairlinePos(0.0f)
        , m_ShowDownwardHairline(true)
    {}

    void SetPos(float pos) { m_HairlinePos = pos; }
    void SetTopColor(const CRgbaColor& c) { m_TopColor=c; }
    void ShowDownwardHairline(bool b) { m_ShowDownwardHairline = b; }
    void SetBottomColor(const CRgbaColor& c) { m_BottomColor=c; }

    float m_HairlinePos;
    CRgbaColor m_TopColor;
    bool m_ShowDownwardHairline;
    CRgbaColor m_BottomColor;     
};

class CSeqGraphicImageInfo {
public:
    void Clear() { m_ActiveAreas.clear(); m_Hairlines.clear(); }

    typedef vector<CHTMLActiveArea> TAreaVector;
    typedef vector<CSeqGraphicHairline> THairlines;

    TAreaVector m_ActiveAreas;
    THairlines  m_Hairlines;
};

END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_GL___HTML_ACTIVE_AREA__HPP
