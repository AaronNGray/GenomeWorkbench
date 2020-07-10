/*  $Id: seq_text_pane_conf.cpp 39602 2017-10-13 19:04:41Z katargir $
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
#include <gui/widgets/seq_text/seq_text_pane_conf.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <objmgr/annot_selector.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <corelib/ncbistr.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

const string kCaseFeature("case_feature");
const string kCaseLower("show_feat_as_lower");
const string kFontSize("font_size");
const string kShowAbsolute("show_absolute_position");
const string kFeatureColoration("feature_coloration");
const string kCodonDraw("codon_draw");

// string representations of EFeatureDisplayType
const string kDisplayAll("all");
const string kDisplayNone("none");
const string kDisplaySelected("selected");
const string kDisplayMouseOver("mouseover");


CSeqTextPaneConfig::CSeqTextPaneConfig(CGuiRegistry* config_cache)
    : CSettingsSet("SequenceTextView", config_cache)
{
}


CSeqTextPaneConfig::~CSeqTextPaneConfig()
{
}


void CSeqTextPaneConfig::Reload()
{
    /// cached colors by feature subtype
    Uncache();
}


void CSeqTextPaneConfig::SetCaseFeature(TFeatSubtype subtype)
{
    CFeatListItem config_item;
    const CFeatList* cfg_list = CSeqFeatData::GetFeatList();

    cfg_list->GetItemBySubtype (subtype, config_item);
    string desc = config_item.GetDescription();

    Set(kCaseFeature, desc);
    objects::SAnnotSelector sel = CSeqUtils::GetAnnotSelector();
    sel.SetFeatSubtype(static_cast<objects::CSeqFeatData::ESubtype>(subtype));
    m_CaseFeatureSel = sel;
}


objects::SAnnotSelector *CSeqTextPaneConfig::GetCaseFeature ()
{
    int type=0, subtype=0;
    string desc = GetString(kCaseFeature);
    CSeqFeatData::GetFeatList()->GetTypeSubType(desc, type, subtype);
    objects::SAnnotSelector sel = CSeqUtils::GetAnnotSelector();
    sel.SetFeatSubtype(static_cast<objects::CSeqFeatData::ESubtype>(subtype));
    m_CaseFeatureSel = sel;
    return &m_CaseFeatureSel;
}


int CSeqTextPaneConfig::GetCaseFeatureSubtype()
{
    int type=0, subtype=0;
    string desc = GetString(kCaseFeature);
    CSeqFeatData::GetFeatList()->GetTypeSubType(desc, type, subtype);
    return subtype;
}


void CSeqTextPaneConfig::SetFontSize(int font_size)
{
    Set (kFontSize, font_size);
}


int CSeqTextPaneConfig::GetFontSize()
{
    return GetInt(kFontSize);
}


void CSeqTextPaneConfig::SetShowAbsolutePosition (bool show_abs)
{
    Set (kShowAbsolute, show_abs);
}


bool CSeqTextPaneConfig::GetShowAbsolutePosition()
{
    return GetBool(kShowAbsolute);
}


bool CSeqTextPaneConfig::GetShowFeatAsLower()
{
    return GetBool(kCaseLower);
}


void CSeqTextPaneConfig::SetShowFeatAsLower(bool show_lower)
{
    Set(kCaseLower, show_lower);
}


CSeqTextPaneConfig::EFeatureDisplayType CSeqTextPaneConfig::ConfigStringToFeatureDisplayType (string str)
{
    if (NStr::CompareNocase (str, kDisplayAll) == 0) {
        return eAll;
    } else if (NStr::CompareNocase (str, kDisplaySelected) == 0) {
        return eSelected;
    } else if (NStr::CompareNocase (str, kDisplayMouseOver) == 0) {
        return eMouseOver;
    } else {
        return eNone;
    }
}


const string CSeqTextPaneConfig::FeatureDisplayTypeToConfigString (CSeqTextPaneConfig::EFeatureDisplayType disp)
{
    if (disp == eAll) {
        return kDisplayAll;
    } else if (disp == eSelected) {
        return kDisplaySelected;
    } else if (disp == eMouseOver) {
        return kDisplayMouseOver;
    } else {
        return kDisplayNone;
    }
}


CSeqTextPaneConfig::EFeatureDisplayType CSeqTextPaneConfig::GetFeatureColorationChoice()
{
    string desc = GetString (kFeatureColoration);
    return ConfigStringToFeatureDisplayType (desc);
}


void CSeqTextPaneConfig::SetFeatureColorationChoice(CSeqTextPaneConfig::EFeatureDisplayType ftype)
{
    Set(kFeatureColoration, FeatureDisplayTypeToConfigString (ftype));
}


CSeqTextPaneConfig::EFeatureDisplayType CSeqTextPaneConfig::GetCodonDrawChoice()
{
    string desc = GetString (kCodonDraw);
    return ConfigStringToFeatureDisplayType (desc);
}


void CSeqTextPaneConfig::SetCodonDrawChoice(CSeqTextPaneConfig::EFeatureDisplayType ftype)
{
    Set(kCodonDraw, FeatureDisplayTypeToConfigString (ftype));
}


END_NCBI_SCOPE
