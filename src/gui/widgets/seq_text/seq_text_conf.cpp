/*  $Id: seq_text_conf.cpp 39602 2017-10-13 19:04:41Z katargir $
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
 * Authors:  Colleen Bollin, based on a file by Vlad Lebedev
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/widgets/seq_text/seq_text_conf.hpp>
#include <objmgr/annot_selector.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>

BEGIN_NCBI_SCOPE

static const string kShowKey("show");
static const string kColorsKey("color");

CSeqTextConfig::CSeqTextConfig(CGuiRegistry* config_cache)
    : CSettingsSet("SequenceTextView", config_cache)
    , m_PaneConfig(config_cache)
{
}

CSeqTextConfig::~CSeqTextConfig()
{
}


void CSeqTextConfig::Reload()
{
    m_PaneConfig.Reload();
    m_FeatColors.clear();
    Uncache();
}


bool CSeqTextConfig::GetShow(int subtype) const
{
    return GetBool(GetFeatReadKey(subtype, kEmptyStr, kShowKey));
}


CRgbaColor* CSeqTextConfig::GetColor(int subtype) const
{
    TFeatColorMap::iterator iter = m_FeatColors.find(subtype);
    if (iter == m_FeatColors.end()) {
        CRgbaColor color =
            CSettingsSet::GetColor(GetFeatReadKey(subtype, kEmptyStr, kColorsKey));
        color.Lighten(.5);

        m_FeatColors[subtype] = CRgbaColor(color);
        iter = m_FeatColors.find(subtype);
        _ASSERT(iter != m_FeatColors.end());
    }

    return &(iter->second);
}


void CSeqTextConfig::SetShow(objects::CSeqFeatData::ESubtype subtype, bool b)
{
    Set(CSettingsSet::GetFeatWriteKey(subtype, kEmptyStr) +
        CSettingsSet::kDefaultDelim + kShowKey, b);
}


void CSeqTextConfig::SetColor(objects::CSeqFeatData::ESubtype subtype, CRgbaColor color)
{
    Set(CSettingsSet::GetFeatWriteKey(subtype, kEmptyStr) +
        CSettingsSet::kDefaultDelim + kColorsKey, color);
}


void CSeqTextConfig::SetCaseFeature(objects::CSeqFeatData::ESubtype subtype)
{
    m_PaneConfig.SetCaseFeature(subtype);
}


int CSeqTextConfig::GetCaseFeatureSubtype()
{
    return m_PaneConfig.GetCaseFeatureSubtype();

}


objects::SAnnotSelector *CSeqTextConfig::GetCaseFeature ()
{
    return m_PaneConfig.GetCaseFeature();
}

void CSeqTextConfig::SetFontSize(int font_size)
{
    m_PaneConfig.SetFontSize(font_size);
}


int CSeqTextConfig::GetFontSize()
{
    return m_PaneConfig.GetFontSize();
}


void CSeqTextConfig::SetShowAbsolutePosition (bool show_abs)
{
    m_PaneConfig.SetShowAbsolutePosition (show_abs);
}


bool CSeqTextConfig::GetShowAbsolutePosition()
{
    return m_PaneConfig.GetShowAbsolutePosition();
}


bool CSeqTextConfig::GetShowFeatAsLower()
{
    return m_PaneConfig.GetShowFeatAsLower();
}


void CSeqTextConfig::SetShowFeatAsLower(bool show_lower)
{
    m_PaneConfig.SetShowFeatAsLower(show_lower);
}


CSeqTextPaneConfig::EFeatureDisplayType CSeqTextConfig::GetFeatureColorationChoice()
{
    return m_PaneConfig.GetFeatureColorationChoice();
}


void CSeqTextConfig::SetFeatureColorationChoice(CSeqTextPaneConfig::EFeatureDisplayType ftype)
{
    m_PaneConfig.SetFeatureColorationChoice(ftype);
}


CSeqTextPaneConfig::EFeatureDisplayType CSeqTextConfig::GetCodonDrawChoice()
{
    return m_PaneConfig.GetCodonDrawChoice();
}


void CSeqTextConfig::SetCodonDrawChoice(CSeqTextPaneConfig::EFeatureDisplayType ftype)
{
    m_PaneConfig.SetCodonDrawChoice(ftype);
}


END_NCBI_SCOPE
