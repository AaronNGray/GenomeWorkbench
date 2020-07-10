#ifndef GUI_SERVICES___FEATURE_SEARCH_TOOL_HPP
#define GUI_SERVICES___FEATURE_SEARCH_TOOL_HPP

/*  $Id: feature_search_tool.hpp 39744 2017-10-31 21:12:13Z katargir $
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

#include <gui/core/seqloc_search_context.hpp>
#include <gui/core/search_form_base.hpp>

#include <util/xregexp/regexp.hpp>

#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/Gene_ref.hpp>

#include <wx/hyperlink.h>

class wxTextCtrl;
class wxChoice;
class wxSearchCtrl;
class wxHyperlink;


BEGIN_NCBI_SCOPE


/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */

///////////////////////////////////////////////////////////////////////////////
/// CFeatureSearchTool
class CFeatureSearchTool : public CSearchToolBase
{
public:
    CFeatureSearchTool();

    /// @name IUITool implementation
    /// @{
    virtual IUITool* Clone() const;
    virtual string  GetName() const;
    virtual string  GetDescription() const;
    /// @}

    /// @name IDMSearchTool implementation
    /// @{
    virtual CIRef<IDMSearchForm>  CreateSearchForm();
    virtual bool    IsCompatible(IDataMiningContext* context);
    /// @}

    /// @name IExtension implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

protected:
    /// implementing CSearchToolBase pure virtual function
    virtual CRef<CSearchJobBase> x_CreateJob(IDMSearchQuery& query);
};


///////////////////////////////////////////////////////////////////////////////
/// CFeatureSearchTool
class CFeatureSearchQuery :
    public CSearchQueryBase
{
public:
    typedef CFeatureSearchTool::EPatternType    TPatternType;

    typedef objects::CFeatListItem    TFeatTypeItem;
    typedef set<TFeatTypeItem>      TFeatTypeItemSet;

    CFeatureSearchQuery(TScopedLocs& locs,
                        const string& pattern,
                        bool case_sensitive,
                        TPatternType pt_type,
                        const TFeatTypeItemSet& feat_types);

    string      GetPattern()        {   return m_Pattern;   }
    TPatternType    GetPatternType(){   return m_PatternType;   }
    bool        GetCaseSensitive()  {   return m_CaseSensitive; }
    const TFeatTypeItemSet& GetFeatTypes()   {   return m_FeatTypesSet;  }

    virtual string ToString() const;
protected:
    string              m_Pattern;
    TPatternType        m_PatternType;
    bool                m_CaseSensitive;
    TFeatTypeItemSet    m_FeatTypesSet; /// Feature types to search
};


///////////////////////////////////////////////////////////////////////////////
/// CFeatureSearchForm
class CComboBox;

class CFeatureSearchForm : public CSearchFormBase
{
public:
    typedef CFeatureSearchQuery::TScopedLocs    TScopedLocs;

    CFeatureSearchForm(CFeatureSearchTool& tool);
    ~CFeatureSearchForm();

    /// @name IDMSearchForm implementation
    /// @{
    virtual void    Create();
    virtual void    Init();
    virtual void    Update();
    /// @}

    virtual CIRef<IDMSearchQuery>   ConstructQuery();
    virtual wxSizer *  GetWidget(wxWindow * parent);
protected:

    /// @name CSearchFormBase overridables
    /// @{
    //virtual string  x_GetRunningText();
    virtual void    x_LoadSettings(const CRegistryReadView& view);
    virtual void    x_SaveSettings(CRegistryWriteView view) const;

    virtual IDMSearchTool*  x_GetTool();
    virtual void    x_CreateWidgets();

    virtual void UpdateContexts()
    {
        CSearchFormBase::UpdateContexts();
    }
    
protected:
    typedef CFeatureSearchQuery::TFeatTypeItem  TFeatTypeItem;
    typedef CFeatureSearchQuery::TFeatTypeItemSet  TFeatTypeItemSet;

    CRef<CFeatureSearchTool>    m_Tool;

    wxChoice      * m_DbCombo;
    wxChoice      * m_TypeCombo;
    wxHyperlinkCtrl * m_HyperLink;

    vector<ISeqLocSearchContext*>   m_SeqLocContexts;
    bool    m_CaseSensitive;


    typedef CSearchToolBase::EPatternType   TPatternType;
    TPatternType    m_PatternType;

    // serializable
    string          m_SearchPattern;
    list<string>    m_FeatureTypes;
};


///////////////////////////////////////////////////////////////////////////////
/// CFeatureSearchJob
class CFeatureSearchJob : public CSearchJobBase
{
public:
    CFeatureSearchJob(CFeatureSearchQuery& query);

	string SeqLocToString(const objects::CSeq_loc& loc);

protected:
    typedef CFeatureSearchQuery::TFeatTypeItem TFeatTypeItem;
    typedef CFeatureSearchQuery::TFeatTypeItemSet TFeatTypeItemSet;
    typedef CFeatureSearchQuery::SScopedLoc     TScopedLoc;
    typedef CFeatureSearchQuery::TScopedLocs    TScopedLocs;

    /// @name CSearchJobBase overridables
    /// @{
    virtual bool        x_ValidateParams();
    virtual EJobState   x_DoSearch();
    virtual void    x_SetupColumns(CObjectList& obj_list);
    /// @}

    void    x_SearchFeatures(TScopedLocs& scoped_locs,
                             const TFeatTypeItemSet& feat_types);

    bool    x_Match(const string& text);
    bool    x_Match(const objects::CGene_ref& gene_ref);
    void    x_AddToResults(CObject& obj, objects::CScope& scope,
                           const string& loc_name,
						   const string& strand,
						   const string& acc_name,
						   const string& ctx_name);

protected:
    typedef CSearchToolBase::EPatternType   TPatternType;

    CRef<CFeatureSearchQuery>   m_Query;
    TPatternType    m_PatternType;
    string      m_SearchStr; /// search pattern for "Exact Match" and "Wildcard" modes
    CRegexp*    m_Pattern;  /// regular expression to apply
    bool        m_CaseSensitive;
};


/* @} */

END_NCBI_SCOPE

#endif  // GUI_SERVICES___FEATURE_SEARCH_TOOL_HPP
