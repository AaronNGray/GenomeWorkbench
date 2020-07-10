#ifndef GUI_SERVICES___SEQUENCE_SEARCH_TOOL_HPP
#define GUI_SERVICES___SEQUENCE_SEARCH_TOOL_HPP

/*  $Id: sequence_search_tool.hpp 39744 2017-10-31 21:12:13Z katargir $
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
#include <util/range_coll.hpp>

#include <objmgr/seq_vector.hpp>



class wxTextCtrl;
class wxChoice;
class wxSearchCtrl;
class wxFlexGridSizer;


BEGIN_NCBI_SCOPE


/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */


///////////////////////////////////////////////////////////////////////////////
/// CSequenceSearchTool
class CSequenceSearchTool : public CSearchToolBase
{
public:
    CSequenceSearchTool();

    /// @name IUITool implementation
    /// @{
    virtual IUITool* Clone() const;
    virtual string    GetName() const;
    virtual string    GetDescription() const;
    /// @}

    /// @name IDMSearchTool implementation
    /// @{
    virtual CIRef<IDMSearchForm>  CreateSearchForm();
    virtual bool    IsCompatible(IDataMiningContext* context);  
    virtual IDMSearchTool::TUIToolFlags GetFlags(void);
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
/// CSequenceSearchTool
class CSequenceSearchQuery :
    public CSearchQueryBase
{
public:
    typedef CSequenceSearchTool::EPatternType    TPatternType;

    CSequenceSearchQuery(TScopedLocs& locs,
                        const string& pattern,
                        TPatternType pt_type,
                        const string& file);

    string      GetPattern()        {   return m_Pattern;   }
    string      GetPatternFile()    {   return m_PatternFile;   }
    TPatternType    GetPatternType(){   return m_PatternType;   }

    virtual string ToString() const;

protected:
    string              m_Pattern;
    TPatternType        m_PatternType;
    string              m_PatternFile;
};


///////////////////////////////////////////////////////////////////////////////
/// CSequenceSearchForm
class CComboBox;

class CSequenceSearchForm : public CSearchFormBase
{
public:
    typedef CSequenceSearchQuery::TScopedLocs    TScopedLocs;

    typedef pair<string, string>        TDescPattern;
    typedef list<TDescPattern>          TDescPatList;
    typedef pair<wxString, TDescPatList>  TPattern;
    typedef map<string, TPattern>       TFileList;


    CSequenceSearchForm(CSequenceSearchTool& tool);
    ~CSequenceSearchForm();

    /// @name IDMSearchForm implementation
    /// @{
    virtual void    Create();
    virtual void    Init();
    virtual void    Update();
    /// @}

    virtual CIRef<IDMSearchQuery>   ConstructQuery();
    virtual wxSizer *  GetWidget(wxWindow * parent);

    static TFileList m_FileList;    

protected:
    virtual void    x_LoadSettings(const CRegistryReadView& view);
    virtual void    x_SaveSettings(CRegistryWriteView view) const;

    virtual IDMSearchTool*  x_GetTool();

    virtual void UpdateContexts();

    // update our map of names of pattern sets to file names,
    // based on files in certain directories
    void x_UpdateFileList(void);

protected:
    CRef<CSequenceSearchTool>    m_Tool;
    wxChoice * m_DbCombo;
    wxChoice * m_TypeCombo;
    wxChoice * m_PatternCombo;
    vector<ISeqLocSearchContext*>   m_SeqLocContexts;

    typedef CSearchToolBase::EPatternType   TPatternType;
    TPatternType    m_PatternType;
    string          m_SearchPattern;

    wxFlexGridSizer * m_pSearchSizer;
    wxFlexGridSizer * m_pPatternSizer;

};



///////////////////////////////////////////////////////////////////////////////
/// CSequenceSearchJob
///
class CSequenceSearchJob : public CSearchJobBase
{
public:
    CSequenceSearchJob(CSequenceSearchQuery& query);

protected:
    typedef CSequenceSearchQuery::SScopedLoc     TScopedLoc;
    typedef CSequenceSearchQuery::TScopedLocs    TScopedLocs;
    typedef set<pair<TSeqPos, TSeqPos> >         TRangeCollection;

    /// @name CSearchJobBase overridables
    /// @{
    virtual bool    x_ValidateParams();
    virtual IAppJob::EJobState    x_DoSearch();
    virtual void    x_SetupColumns(CObjectList& obj_list);

    virtual CObjectListTableModel* x_GetNewOLTModel() const;
    /// @}

    IAppJob::EJobState    x_SearchSequence(TScopedLocs& scoped_locs);

    void    x_GetMatches(const string& data, int start, TRangeCollection& coll, bool reverse);
    bool    x_Match(const string& text);

    void    x_GetSequence(objects::CSeqVector& vec, const CRange<TSeqPos>& range, string* s);
    void    x_AddToResults(CObject& obj, objects::CScope& scope,
                           const string& sequence,
                           const string& loc_name,
                           const string& strand,
                           const string& ctx_name);
        

protected:
    typedef CSearchToolBase::EPatternType   TPatternType;

    CRef<CSequenceSearchQuery>   m_Query;
    TPatternType    m_PatternType;
    string      m_SearchStr; /// search patteern for "Exact Match" and "Wildcard" modes
    CRegexp*    m_Pattern;  /// regular expression to apply 
    string      m_PatternFile;
};


/* @} */

END_NCBI_SCOPE

#endif  // GUI_SERVICES___SEQUENCE_SEARCH_TOOL_HPP
