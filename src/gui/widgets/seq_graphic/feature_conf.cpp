/*  $Id: feature_conf.cpp 35245 2016-04-14 19:01:03Z vasilche $
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

/// @file
///

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/feature_conf.hpp>
#include <util/static_map.hpp>
#include <math.h>

BEGIN_NCBI_SCOPE


/* unused ???
static const char* const sm_DecoKeyStrs[] = {
    "LabelPos",
    "HeadStyle",
    "TailStyle",
    "LineStyle",
    "Connection"
};
*/

typedef SStaticPair<CFeatureParams::EDecoElements, int> TDecoValue;
typedef SStaticPair<const char*, TDecoValue> TDecoStyleStr;
static const TDecoStyleStr s_DecoStyleStrs[] = {
    { "above",              { CFeatureParams::eLabelPosition, CFeatureParams::ePos_Above } },
    { "arrow with neck",    { CFeatureParams::eHeadStyle, CFeatureParams::eHead_Arrow } },
    { "box",                { CFeatureParams::eConnection, CFeatureParams::eBox } },
    { "box filled",         { CFeatureParams::eHeadStyle, CFeatureParams::eBox_Filled } },
    { "box hollow",         { CFeatureParams::eHeadStyle, CFeatureParams::eBox_Hollow } },
    { "box line",           { CFeatureParams::eHeadStyle, CFeatureParams::eBox_Line } },
    { "canted line",        { CFeatureParams::eConnection, CFeatureParams::eCantedLine } },
    { "circle",             { CFeatureParams::eTailStyle, CFeatureParams::eTail_Circle } },
    { "dashdotted",         { CFeatureParams::eLineStyle, CFeatureParams::eLine_DashDotted } },
    { "dashed",             { CFeatureParams::eLineStyle, CFeatureParams::eLine_Dashed } },
    { "dotted",             { CFeatureParams::eLineStyle, CFeatureParams::eLine_Dotted } },
    { "filled box",         { CFeatureParams::eConnection, CFeatureParams::eFilledBox } },
    { "fletched",           { CFeatureParams::eTailStyle, CFeatureParams::eTail_Fletched } },
    { "head tick",          { CFeatureParams::eHeadStyle, CFeatureParams::eHead_TickMark } },
    { "inside",             { CFeatureParams::eLabelPosition, CFeatureParams::ePos_Inside } },
    { "no head",            { CFeatureParams::eHeadStyle, CFeatureParams::eHead_No } },
    { "no label",           { CFeatureParams::eLabelPosition, CFeatureParams::ePos_NoLabel } },
    { "no tail",            { CFeatureParams::eTailStyle, CFeatureParams::eTail_No } },
    { "short dashed",       { CFeatureParams::eLineStyle, CFeatureParams::eLine_ShortDashed } },
    { "side",               { CFeatureParams::eLabelPosition, CFeatureParams::ePos_Side } },
    { "solid",              { CFeatureParams::eLineStyle, CFeatureParams::eLine_Solid } },
    { "solid triangle",     { CFeatureParams::eHeadStyle, CFeatureParams::eHead_Triangle } },
    { "square",             { CFeatureParams::eTailStyle, CFeatureParams::eTail_Square } },
    { "straight line",      { CFeatureParams::eConnection, CFeatureParams::eStraightLine } },
    { "triangle with neck", { CFeatureParams::eHeadStyle, CFeatureParams::eHead_NeckedTriangle } },
};

typedef CStaticArrayMap<string, TDecoValue> TDecoMap;
DEFINE_STATIC_ARRAY_MAP(TDecoMap, sm_DecoMap, s_DecoStyleStrs);

int CFeatureParams::DecoStrToValue(const string& s)
{
    TDecoMap::const_iterator dv_it = sm_DecoMap.find(s);
    if (dv_it != sm_DecoMap.end()) {
        return dv_it->second.second;
    }
    NCBI_THROW(CException, eInvalid, "Invalid settings: " + s);
}

const string& CFeatureParams::DecoValueToStr(CFeatureParams::EDecoElements e, int v)
{
    TDecoMap::const_iterator dv_it;
    for (dv_it = sm_DecoMap.begin();
         dv_it != sm_DecoMap.end();
         ++dv_it) {
        if (dv_it->second.first == e && dv_it->second.second == v) {
            return dv_it->first;
        }
    }
    return kEmptyStr;
}


TModelUnit CFeatureParams::GetBarHeight(bool overview) const
{
    TModelUnit head_height = m_BarHeight * m_HeadHeight;
    TModelUnit tail_height = m_BarHeight * m_TailHeight;

    TModelUnit height = max(m_BarHeight,
        max(m_HeadStyle == eHead_No ? 0 : head_height * 2,
        (m_TailStyle == eTail_No ||
        m_TailStyle == eTail_Fletched) ? 0 :tail_height));

    if (overview) {
        height = floor(height * m_OverviewFactor);
    }
    return height;  
}




END_NCBI_SCOPE
