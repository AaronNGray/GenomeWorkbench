#ifndef PKG_SEQUENCE_EDIT___AUTODEF_PARAMS__HPP
#define PKG_SEQUENCE_EDIT___AUTODEF_PARAMS__HPP

/*  $Id: autodef_params.hpp 37956 2017-03-08 13:58:01Z bollin $
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
 * Authors:  Colleen Bollin, based on a file by Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>

#include <gui/packages/pkg_sequence_edit/autodef_params.hpp>

#include <gui/core/algo_tool_manager_base.hpp>

#include <gui/objutils/objects.hpp>

#include <gui/objutils/reg_settings.hpp>

#include <objtools/edit/autodef_with_tax.hpp>


BEGIN_NCBI_SCOPE
///////////////////////////////////////////////////////////////////////////////
///

class SAutodefParams : public IRegSettings
{
public:
    SAutodefParams();

    void Init();

    /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath(const string& reg_path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

public:
    string m_RegPath;

    TConstScopedObjects     m_SeqEntries;
    bool    m_UseLabels;
    bool    m_LeaveParenthetical;
    bool    m_DoNotApplyToSp;
    bool    m_DoNotApplyToNr;
    bool    m_DoNotApplyToCf;
    bool    m_DoNotApplyToAff;
    bool    m_IncludeCountryText;
    bool    m_KeepAfterSemicolon;
    unsigned int m_MaxMods;
    unsigned int m_HIVRule;
    bool         m_NeedHIVRule;

    unsigned int m_FeatureListType;
    unsigned int m_MiscFeatRule;
    unsigned int m_ProductFlag;
    unsigned int m_NuclearCopyFlag;
    bool m_SpecifyNuclearProduct;
    bool m_AltSpliceFlag;
    bool m_SuppressLocusTags;
    bool m_GeneClusterOppStrand;
    bool m_SuppressFeatureAltSplice;
    bool m_SuppressMobileElementSubfeatures;
    bool m_SuppressMiscFeatSubfeatures;
    bool m_KeepExons;
    bool m_KeepIntrons;
    bool m_UseFakePromoters;
    bool m_KeepRegulatoryFeatures;
    bool m_KeepLTRs;
    bool m_Keep3UTRs;
    bool m_Keep5UTRs;
    bool m_UseNcRNAComment;
    bool m_KeepMiscRecomb;
    bool m_KeepRepeatRegion;

    set<objects::CFeatListItem> m_SuppressedFeatures;

    objects::CAutoDefSourceDescription::TAvailableModifierVector m_ModifierList;

    string m_CustomFeatureClause;
};


END_NCBI_SCOPE

#endif // PKG_SEQUENCE_EDIT___AUTODEF_PARAMS__HPP

