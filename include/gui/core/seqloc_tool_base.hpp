#ifndef GUI_SERVICES___SEQLOC_TOOL_TOOL_HPP
#define GUI_SERVICES___SEQLOC_TOOL_TOOL_HPP

/*  $Id: seqloc_tool_base.hpp 24658 2011-11-01 20:09:01Z katargir $
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

#include <gui/services/seqloc_search_context.hpp>
#include <gui/services/search_form_base.hpp>
#include <gui/services/search_tool_base.hpp>

#include <gui/utils/command.hpp>

#include <gui/objutils/app_job.hpp>
#include <gui/objutils/event_translator.hpp>

#include <util/regexp.hpp>

#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/Gene_ref.hpp>

class Fl_Choice;
class Fl_Box;

BEGIN_NCBI_SCOPE

class IDMSearchForm;
class CInput;
class CButton;
class CRoundButton;
class CCheckButton;
class CAppJobDispatcher;


///////////////////////////////////////////////////////////////////////////////
/// CSeqLocToolBase
class NCBI_GUISERVICES_EXPORT CSeqLocToolBase : public CSearchToolBase
{
public:
    CSeqLocToolBase(ESearchType type);

    /// @name IDMSearchTool implementation
    /// @{
    virtual CIREf<IDMSearchForm>  CreateSearchForm() = 0;
    virtual bool    IsCompatible(IDataMiningContext* context);
    /// @}
};


///////////////////////////////////////////////////////////////////////////////
/// CSeqLocFormBase
class CComboBox;

class NCBI_GUISERVICES_EXPORT CSeqLocFormBase : public CSearchFormBase
{
public:
    typedef CSeqLocToolQuery::TScopedLocs    TScopedLocs;

    CSeqLocFormBase(CSeqLocToolBase& tool);
    ~CSeqLocFormBase();

    /// @name IDMSearchForm implementation
    /// @{
    virtual void    Create();
    virtual void    Init();

    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

protected:
    DECLARE_EVENT_MAP();

    void    x_DDX_Context();

protected:
    typedef CSeqLocToolQuery::EPatternType TPatternType;
    typedef map<string, TPatternType>   TNameToType;

    vector<ISeqLocSearchContext*>   m_SeqLocContexts;

    TNameToType m_NameToType;
    CComboBox*  m_MatchTypeCombo;
    CSeqLocToolQuery::EPatternType m_PatternType;

    CCheckButton*   m_CaseBtn;
    bool    m_CaseSensitive;
};


///////////////////////////////////////////////////////////////////////////////
/// CFeatureSearchJob
///
class NCBI_GUISERVICES_EXPORT CFeatureSearchJob : public CSearchJobBase
{
public:
    CFeatureSearchJob(CSeqLocToolQuery& query);

protected:
    typedef CSeqLocToolQuery::TFeatTypeItem TFeatTypeItem;
    typedef CSeqLocToolQuery::TFeatTypeItemSet TFeatTypeItemSet;
    typedef CSeqLocToolQuery::SScopedLoc     TScopedLoc;
    typedef CSeqLocToolQuery::TScopedLocs    TScopedLocs;

    /// @name CSearchJobBase overridables
    /// @{
    virtual bool    x_ValidateParams();
    virtual void    x_DoSearch();
    /// @}

    void    x_SearchFeatures(TScopedLocs& scoped_locs,
                             const TFeatTypeItemSet& feat_types);
    void    x_SearchComponents(TScopedLocs& scoped_locs);

    bool    x_Match(const string& text);
    bool    x_Match(const objects::CGene_ref& gene_ref);
    void    x_AddToResults(CObject& obj, objects::CScope& scope,
                           const string& loc_name, const string& ctx_name);

protected:
    typedef CSeqLocToolQuery::EPatternType   TPatternType;

    CRef<CSeqLocToolQuery>   m_Query;
    TPatternType    m_PatternType;
    string      m_SearchStr; /// search patteern for "Exact Match" and "Wildcard" modes
    CRegexp*    m_Pattern;  /// regular expression to apply
    bool        m_CaseSensitive;
};


END_NCBI_SCOPE


/*
 * ===========================================================================
 * ===========================================================================
 */

#endif  // GUI_SERVICES___SEQLOC_TOOL_TOOL_HPP
