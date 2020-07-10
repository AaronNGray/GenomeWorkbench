#ifndef GUI_WIDGETS_SEQ_GRAPHIC___GENE_MODEL_CONFIG__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___GENE_MODEL_CONFIG__HPP

/*  $Id: gene_model_config.hpp 43606 2019-08-07 22:40:09Z evgeniev $
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
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/widgets/seq_graphic/cds_glyph.hpp>
#include <gui/widgets/seq_graphic/config_utils.hpp>


BEGIN_NCBI_SCOPE

class CCdsConfig;
class CSeqGraphicConfig;
class CBoundaryParams;

class CGeneModelConfig : public CObject
{
public:
    /// style on how to merge RNAs and CDSs.
    enum EMergeStyle {
        eMerge_No,     ///< show all feature individually
        eMerge_Pairs,  ///< merge each transcript and CDS pair
        eMerge_All,    ///< merge all transcripts and CDSs
        eMerge_OneLine ///< merge into one line
        /// if there are transcripts and/or CDSs, show only
        /// transcripts and CDSs, and merge them into one bar (prefix
        /// the gene label to RNA and CDS label. If there is no RNA or
        /// CDS feature, show exon features only. If there is no exon
        /// feature, show gene bar.
    };


    static const string& StyleValueToStr(CGeneModelConfig::EMergeStyle style);
    static EMergeStyle StyleStrToValue(const string& style);


    /// rendering options on how to render features overall.
    /// each option corresponds to one or more settings.
    enum ERenderingOption {
        eOption_GeneOnly,    ///< show gene bar only
        eOption_MergeAll,    ///< merge all RNAs and CDSs, show exons, no gene bar
        eOption_MergePairs,  ///< merge each RNA and CDS pair, show exons, no gene bar
        eOption_ShowAllButGenes,  ///< show all RNAs, CDSs and exons, no gene bar
        eOption_ShowAll,     ///< show all features individually
        eOption_SingleLine,  ///< merge all RNAs and CDSs into one line
        eOption_Default = eOption_MergeAll        
    };

    static const string& OptionValueToStr(CGeneModelConfig::ERenderingOption option);
    static ERenderingOption OptionStrToValue(const string& option);

    CGeneModelConfig() 
        : m_ShowLMGeneLabel(true)
        , m_ShowGenes(true)
        , m_ShowRNAs(true)
        , m_ShowCDSs(true)
        , m_ShowExons(true)
        , m_ShowVDJCs(true)
        , m_ShowGeneProducts(false)
        , m_ShowCDSProductFeats(false)
        , m_ShowNtRuler(true)
        , m_ShowAaRuler(true)
        , m_ShowHistogram(true)
        , m_MergeStyle(eMerge_No)
        , m_LandmarkFeatLimit(200)
        , m_OverviewFeatCutoff(30)
        , m_HighlightMode(0)
    {}

    void LoadSettings(CConstRef<CSeqGraphicConfig> g_conf,
        const string& reg_path, const string& profile);
    // For gene model track settings we actually don't need the
    // CSeqGraphicConfig, and it's a burden to initialize it and
    // pass through here. So we introduce lightweight load. SV-2343
    void LoadSettingsPartial(const string& reg_path, const string& profile);
    void SaveSettings(CConstRef<CSeqGraphicConfig> g_conf,
        const string& reg_path, const string& profile) const;

    void UpdateSettings(const CSGConfigUtils::TKeyValuePairs& settings);
    string SaveSettingsAsString(const string& preset_style) const;


    /// @name layout settings.
    /// @{
    bool    m_ShowLMGeneLabel;  ///< landmark gene labels.
    bool    m_ShowGenes;
    bool    m_ShowRNAs;
    bool    m_ShowCDSs;
    bool    m_ShowExons;
    bool    m_ShowVDJCs;            ///< show VDJ_segments and C_region 
    bool    m_ShowGeneProducts;     ///< feature histogram on a gene.
    bool    m_ShowCDSProductFeats;  ///< CDS associated protein features.
    bool    m_ShowNtRuler;          ///< Show nucleotide ruler
    bool    m_ShowAaRuler;          ///< Show protein ruler
    bool    m_ShowHistogram;        ///< show gene distribution histogram.
    bool    m_ShowLabel = true;     ///< Show labels
    string  m_CDSProfile;           ///< cds setting profile
    EMergeStyle m_MergeStyle;       ///< the way on how to merge CDSs and RNAs.
    /// @}

    int     m_LandmarkFeatLimit;
    int     m_OverviewFeatCutoff;
    int     m_HighlightMode;

    CRef<CCdsConfig>        m_CdsConfig;
    CRef<CBoundaryParams>   m_GeneBoxConfig;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___GENE_MODEL_CONFIG__HPP
