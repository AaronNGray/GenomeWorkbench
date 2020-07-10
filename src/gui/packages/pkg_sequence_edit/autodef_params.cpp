/*  $Id: autodef_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors: Colleen Bollin, based on a file by Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_sequence_edit/autodef_params.hpp>
#include <objtools/edit/autodef_with_tax.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// SAutodefParams

SAutodefParams::SAutodefParams()
{
    Init();
}

void SAutodefParams::Init()
{
    m_UseLabels = false;
    m_LeaveParenthetical = true;
    m_DoNotApplyToSp = false;
    m_DoNotApplyToNr = false;
    m_DoNotApplyToCf = false;
    m_DoNotApplyToAff = false;
    m_IncludeCountryText= false;
    m_KeepAfterSemicolon= false;
    m_MaxMods = 0;
    m_HIVRule = CAutoDefOptions::ePreferClone;
    m_NeedHIVRule = false;
    m_FeatureListType = CAutoDefOptions::eListAllFeatures;
    m_MiscFeatRule = CAutoDefOptions::eDelete;
    m_ProductFlag = 0;
    m_NuclearCopyFlag = 0;
    m_SpecifyNuclearProduct = true;
    m_AltSpliceFlag = false;
    m_SuppressLocusTags= false;
    m_GeneClusterOppStrand= false;
    m_SuppressFeatureAltSplice= false;
    m_SuppressMobileElementSubfeatures= false;
    m_SuppressMiscFeatSubfeatures = false;
    m_KeepExons= false;
    m_KeepIntrons= false;
    m_KeepRegulatoryFeatures= false;
    m_UseFakePromoters= false;
    m_KeepLTRs= false;
    m_Keep3UTRs= false;
    m_Keep5UTRs= false;
    m_UseNcRNAComment= false;
    m_KeepMiscRecomb = false;
    m_KeepRepeatRegion = false;
    m_SuppressedFeatures.clear();
    m_CustomFeatureClause = kEmptyStr;
}

void SAutodefParams::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
}

static const char* kUseLabelsTag = "Use Labels";
static const char* kLeaveParentheticalTag = "Leave Parenthetical";
static const char* kDoNotApplyToSp = "Do Not Apply To Sp";
static const char* kIncludeCountryTextTag = "Include Country Text";


void SAutodefParams::SaveSettings() const
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kUseLabelsTag, m_UseLabels);
        view.Set(kLeaveParentheticalTag, m_LeaveParenthetical);
        view.Set(kDoNotApplyToSp, m_DoNotApplyToSp);
        view.Set(kIncludeCountryTextTag, m_IncludeCountryText);
    }
}


void SAutodefParams::LoadSettings()
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_UseLabels = view.GetBool(kUseLabelsTag, m_UseLabels);
        m_LeaveParenthetical = view.GetBool(kLeaveParentheticalTag, m_LeaveParenthetical);
        m_DoNotApplyToSp = view.GetBool(kDoNotApplyToSp, m_DoNotApplyToSp);
        m_IncludeCountryText = view.GetBool(kIncludeCountryTextTag, m_IncludeCountryText);
    }
}

END_NCBI_SCOPE
