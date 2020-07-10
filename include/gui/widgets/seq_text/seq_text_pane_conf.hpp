#ifndef GUI_WIDGETS_SEQ_TEXT___SEQTEXT_PANE_CONF__HPP
#define GUI_WIDGETS_SEQ_TEXT___SEQTEXT_PANE_CONF__HPP

/*  $Id: seq_text_pane_conf.hpp 39680 2017-10-26 16:03:20Z katargir $
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

#include <objmgr/seq_vector.hpp>
#include <objmgr/annot_selector.hpp>

#include <objects/seqfeat/SeqFeatData.hpp>

BEGIN_NCBI_SCOPE

class CSpacingConf;


class NCBI_GUIWIDGETS_SEQTEXT_EXPORT CSeqTextPaneConfig : public CSettingsSet
{
public:
    /// we use int instead of CSeqFeatData::ESubtype for two reasons:
    ///  - avoids unnecessary includes
    ///  - gives transparent warning-free compiles with generic serialized
    ///    code (i.e., when we serialize a subtype, it's as an int not as an
    ///    enum)
    typedef CSettingsSet::TFeatSubtype TFeatSubtype;

    enum EFeatureDisplayType {
        eNone,
        eAll,
        eSelected,
        eMouseOver
    };


    CSeqTextPaneConfig(CGuiRegistry* config_cache = 0);

    ~CSeqTextPaneConfig();

    /// uncache any cached data
    void Reload();

    void SetCaseFeature(TFeatSubtype subtype);
    objects::SAnnotSelector *GetCaseFeature ();
    int  GetCaseFeatureSubtype();
    bool GetShowFeatAsLower ();
    void SetShowFeatAsLower(bool show_lower);

    void SetFontSize(int font_size);
    int  GetFontSize();

    void SetShowAbsolutePosition (bool show_abs);
    bool GetShowAbsolutePosition();

    EFeatureDisplayType GetFeatureColorationChoice();
    void SetFeatureColorationChoice(EFeatureDisplayType ftype);
    EFeatureDisplayType GetCodonDrawChoice();
    void SetCodonDrawChoice(EFeatureDisplayType ftype);

private:

    // forbidden
    CSeqTextPaneConfig(const CSeqTextPaneConfig&);
    CSeqTextPaneConfig& operator=(const CSeqTextPaneConfig&);


    EFeatureDisplayType ConfigStringToFeatureDisplayType (string str);
    const string FeatureDisplayTypeToConfigString (EFeatureDisplayType disp);

    objects::SAnnotSelector m_CaseFeatureSel;

};



END_NCBI_SCOPE

#endif  /* GUI_WIDGETS_SEQ_TEXT___SEQTEXT_CONF__HPP */
