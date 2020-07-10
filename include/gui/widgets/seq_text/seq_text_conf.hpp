#ifndef GUI_WIDGETS_SEQ_TEXT___SEQTEXT_CONF__HPP
#define GUI_WIDGETS_SEQ_TEXT___SEQTEXT_CONF__HPP

/*  $Id: seq_text_conf.hpp 39680 2017-10-26 16:03:20Z katargir $
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
 *   Configuration file for Sequence Text Widget
 *
 */


#include <gui/config/settings_set.hpp>
#include <gui/utils/rgba_color.hpp>

#include <objmgr/seq_vector.hpp>
#include <objmgr/annot_selector.hpp>

#include <objects/seqfeat/SeqFeatData.hpp>
class CSeqTextPaneConfig;
#include <gui/widgets/seq_text/seq_text_pane_conf.hpp>

BEGIN_NCBI_SCOPE


class NCBI_GUIWIDGETS_SEQTEXT_EXPORT CSeqTextConfig : public CSettingsSet
{
public:

    CSeqTextConfig(CGuiRegistry* config_cache = 0);

    ~CSeqTextConfig();

    /// uncache any cached data
    void Reload();

    bool GetShow(int subtype) const;

    CRgbaColor*   GetColor(int subtype) const;

    CSettingsSet&     SetContentConf();   // so a detailed dialog can change this.

    void SetShow(objects::CSeqFeatData::ESubtype subtype, bool show);
    void SetColor(objects::CSeqFeatData::ESubtype subtype, CRgbaColor color);

    void SetCaseFeature(objects::CSeqFeatData::ESubtype subtype);
    int  GetCaseFeatureSubtype ();
    objects::SAnnotSelector *GetCaseFeature ();
    bool GetShowFeatAsLower ();
    void SetShowFeatAsLower(bool show_lower);

    void SetFontSize(int font_size);
    int  GetFontSize();

    void SetShowAbsolutePosition (bool show_abs);
    bool GetShowAbsolutePosition();

    CSeqTextPaneConfig::EFeatureDisplayType GetFeatureColorationChoice();
    void SetFeatureColorationChoice(CSeqTextPaneConfig::EFeatureDisplayType ftype);
    CSeqTextPaneConfig::EFeatureDisplayType GetCodonDrawChoice();
    void SetCodonDrawChoice(CSeqTextPaneConfig::EFeatureDisplayType ftype);

private:

    // forbidden
    CSeqTextConfig(const CSeqTextConfig&);
    CSeqTextConfig& operator=(const CSeqTextConfig&);

    CSeqTextPaneConfig m_PaneConfig;

    /// cached colors by feature subtype
    typedef map<TFeatSubtype, CRgbaColor> TFeatColorMap;
    mutable TFeatColorMap m_FeatColors;
};


END_NCBI_SCOPE

#endif  /* GUI_WIDGETS_SEQ_TEXT___SEQTEXT_CONF__HPP */
