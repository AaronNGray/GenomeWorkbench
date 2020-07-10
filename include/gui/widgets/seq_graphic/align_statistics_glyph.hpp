#ifndef GUI_WIDGETS_SEQ_GRAPHIC___ALIGN_STATISTICS_GLYPH__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___ALIGN_STATISTICS_GLYPH__HPP

/*  $Id: align_statistics_glyph.hpp 42508 2019-03-15 18:24:51Z shkeda $
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
 *    CAlnStatGlyph -- utility class for having
 *                      feature density histograms his in graphical layouts.
 *
 */


#include <corelib/ncbiobj.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/objutils/density_map.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <gui/widgets/seq_graphic/histogram_conf.hpp>
#include <array>

BEGIN_NCBI_SCOPE


class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CAlnStatConfig : public CObject
{
public:
    CAlnStatConfig()
        : m_StatZoomLevel(1)
        , m_Display(fDefault)
        , m_ShowLabel(true)
    {
        m_Colors = {
            CRgbaColor("green3"), 
            CRgbaColor("0 114 178"),
            CRgbaColor("204 33 00"),
            CRgbaColor("grey50"),
            CRgbaColor("0 0 0"),
            CRgbaColor("grey50"),
            CRgbaColor("grey80"),
            CRgbaColor("255 0 0"),
            CRgbaColor("0 0 255"),
            CRgbaColor("grey50") };
    }

    /// statistics enum.
    /// some stats can be derived from others. E.g. eStat_Match is 
    /// of of A, G, T and C that matches reference sequence, and eStat_Mismatch
    /// is the sum of all other three, and eStat_Total is the sum of 
    /// gaps, matches, and mismatches.
    enum EStatType {
        eStat_A = 0,
        eStat_G,
        eStat_T,
        eStat_C,
        eStat_Gap,
        eStat_Intron,   ///< intron (for mRNA-to-genome alignments)
        eStat_Match,    ///< matches
        eStat_Mismatch, ///< mismatches (A+G+T+C - matches)
        eStat_Ambig,    ///< ambiguous consensus (for MSA pileup)
        eStat_Total     ///< total alignment count at this base (A+G+T+C+Gap)
    };

    enum FDisplay {
        fBarGraph  = 1 << 0,    ///< otherwise, shown as density table
        fShowCount = 1 << 1,    ///< otherwise, shown percentage
        fShowMismatch = 1 << 2, ///< otherwise, shown individual count
        fShowTotal = 1 << 3, 
        fDefault = fBarGraph | fShowCount | fShowMismatch | fShowTotal
    };

    typedef int TDisplay;

    static int GetRefSeqIdex(char base)
    {
        switch (base) {
        case 'A':
            return eStat_A;
        case 'G':
            return eStat_G;
        case 'T':
            return eStat_T;
        case 'C':
            return eStat_C;
        case 'N':
            return eStat_Intron;
        case '-':
            return eStat_Gap;
        default:
            break;
        }
        return eStat_Gap;
    }

    static string GetStatLabel(int stat)
    {
        EStatType stat_t = (EStatType)stat;
        string label = "";
        switch (stat_t) {
            case CAlnStatConfig::eStat_A:
                label = "A";
                break;
            case CAlnStatConfig::eStat_G:
                label = "G";
                break;
            case CAlnStatConfig::eStat_T:
                label = "T";
                break;
            case CAlnStatConfig::eStat_C:
                label = "C";
                break;
            case CAlnStatConfig::eStat_Gap:
                label = "Gap";
                break;
            case CAlnStatConfig::eStat_Match:
                label = "Match";
                break;
            case CAlnStatConfig::eStat_Mismatch:
                label = "Mismatch";
                break;
            case CAlnStatConfig::eStat_Intron:
                label = "Intron";
                break;
            case CAlnStatConfig::eStat_Total:
                label = "Total";
                break;
            default:
                break;
        }
        return label;
    }

    bool IsBarGraph() const;
    bool ShowCount() const;
    bool ShowAGTC() const;
    bool ShowTotal() const;
    bool ShowLabel() const;
    void SetShowLabel(bool flag);

    void SetDisplayFlag(FDisplay bit, bool f);
    array<CRgbaColor, eStat_Total + 1> m_Colors; ///< color settings
    int         m_StatZoomLevel; ///< at what zoom level to turn on statistics
    TDisplay    m_Display;
    bool        m_ShowLabel;
    bool        m_HasAmbigBases = false; // MSA Viewer uses it 
};


class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CAlnStatGlyph : public CSeqGlyph
{
public:
    struct SStatStruct {
        SStatStruct()
        {
            for (size_t i = 0; i < CAlnStatConfig::eStat_Total + 1; ++i) {
                m_Data[i] = 0;
            }
        }

        int m_Data[CAlnStatConfig::eStat_Total + 1];
    };

    typedef vector<SStatStruct> TStatVec;
    /// @name ctors
    /// @{
    CAlnStatGlyph(TSeqPos start, TModelUnit sacle);
    /// @}

    /// @name CSeqGlyph virtual methods.
    /// @{
    virtual bool OnLeftDblClick(const TModelPoint& p);
    virtual bool NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetHTMLActiveAreas(TAreaVector* p_areas) const;
    virtual bool IsClickable() const;
    ///@}

    void SetConfig(CRef<CAlnStatConfig> conf, CRef<CHistParams> params);
    const TStatVec& GetStatVec() const;
    TStatVec& GetStatVec();

    void SetCgiMode(bool cgi_mode = true);

    void SetDialogHost(IGlyphDialogHost* host);

    void ShowIntrons(bool show = true) { m_ShowIntrons = show; }

    static CRef<objects::CChoice> CreateDisplayOptions(const string& option_name, int display_flag);
protected:
    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();

private:
    void x_DrawBarGraph() const;
    void x_DrawDensityTable() const;
    int x_GetMaxTotal() const;
    vector<int> x_GetShowList() const;

private:
    IGlyphDialogHost*     m_DlgHost;
    CRef<CAlnStatConfig>  m_Config;
    CRef<CHistParams>     m_GraphParams;
    TStatVec              m_StatVec;
    TSeqPos               m_StartPos;

    /// zoom scale when this statistics are computed.
    /// needed for rendering.
    TModelUnit            m_ZoomScale;
    bool                  m_CgiMode = false;
    bool                  m_ShowIntrons = false;
};



///////////////////////////////////////////////////////////////////////////////
/// CAlnStatConfig inline method implementation.
inline
bool CAlnStatConfig::IsBarGraph() const
{
    return m_Display & CAlnStatConfig::fBarGraph;
}

inline
bool CAlnStatConfig::ShowCount() const
{
    return m_Display & CAlnStatConfig::fShowCount;
}

inline
bool CAlnStatConfig::ShowAGTC() const
{
    return !(m_Display & CAlnStatConfig::fShowMismatch);
}

inline
bool CAlnStatConfig::ShowTotal() const
{
    return m_Display & CAlnStatConfig::fShowTotal;
}

inline
bool CAlnStatConfig::ShowLabel() const
{
    return m_ShowLabel;
}

inline
void CAlnStatConfig::SetShowLabel(bool flag)
{
    m_ShowLabel = flag;
}

inline
void CAlnStatConfig::SetDisplayFlag(FDisplay bit, bool f)
{
    if (f) {
        m_Display |= bit;
    } else {
        m_Display &= ~bit;
    }
}


///////////////////////////////////////////////////////////////////////////////
/// CAlnStatGlyph inline method implementation.
inline
void CAlnStatGlyph::SetConfig(CRef<CAlnStatConfig> conf, CRef<CHistParams> params)
{ 
    m_Config = conf; 
    m_GraphParams = params;
}

inline
const CAlnStatGlyph::TStatVec& CAlnStatGlyph::GetStatVec() const
{ return m_StatVec; }

inline
CAlnStatGlyph::TStatVec& CAlnStatGlyph::GetStatVec()
{ return m_StatVec; }

inline
void CAlnStatGlyph::SetDialogHost(IGlyphDialogHost* host)
{ m_DlgHost = host; }

inline
void CAlnStatGlyph::SetCgiMode(bool cgi_mode)
{
    m_CgiMode = cgi_mode;
}
END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___ALIGN_STATISTICS_GLYPH__HPP
