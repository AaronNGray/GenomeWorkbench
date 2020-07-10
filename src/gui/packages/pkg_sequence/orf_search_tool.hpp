#ifndef GUI_SERVICES___ORF_SEARCH_TOOL_HPP
#define GUI_SERVICES___ORF_SEARCH_TOOL_HPP

/*  $Id: orf_search_tool.hpp 39744 2017-10-31 21:12:13Z katargir $
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
/// COrfSearchTool
class COrfSearchTool : public CSearchToolBase
{
public:
    COrfSearchTool();

    /// @name IUITool implementation
    /// @{
    virtual IUITool* Clone() const;
    virtual string    GetName() const;
    virtual string    GetDescription() const;
    /// @}

    /// @name IDMSearchTool implementation
    /// @{
    virtual CIRef<IDMSearchForm> CreateSearchForm();
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
/// COrfSearchTool
class COrfSearchQuery :
    public CSearchQueryBase
{
public:
    COrfSearchQuery(TScopedLocs& locs,
                        const string&  gc,
                        const string&  so,
                        const string&  mp);

    string          GetGencode()        {   return m_Gencode;    }
    string          GetStartOrf()       {   return m_StartOrf;   }
    string          GetMinPairs()       {   return m_MinPairs;   }

    virtual string ToString() const;

protected:      
    string  m_Gencode;
    string  m_StartOrf;
    string  m_MinPairs;
};


///////////////////////////////////////////////////////////////////////////////
/// COrfSearchForm
class CComboBox;

class COrfSearchForm : public CSearchFormBase
{
public:
    typedef COrfSearchQuery::TScopedLocs    TScopedLocs;    

    struct SKozakPattern {
        SKozakPattern(const string& name, const string& regexp, int codon_pos)
            : m_Name(name), m_Regexp(regexp), m_Codon_pos(codon_pos) {}
        string m_Name;
        string m_Regexp;
        int m_Codon_pos;
    };

    typedef struct SKozakPattern TKozakPattern;
    typedef list<TKozakPattern>  TKozakList;    

    COrfSearchForm(COrfSearchTool& tool);
    ~COrfSearchForm();

    /// @name IDMSearchForm implementation
    /// @{
    virtual void    Create();
    virtual void    Init();
    virtual void    Update();
    /// @}

    virtual CIRef<IDMSearchQuery>   ConstructQuery();
    virtual wxSizer *  GetWidget(wxWindow * parent);

    static TKozakList m_Kozak;
protected:
    virtual void    x_LoadSettings(const CRegistryReadView& view);
    virtual void    x_SaveSettings(CRegistryWriteView view) const;

    virtual IDMSearchTool*  x_GetTool();            

    void x_UpdateKozak();
protected:
    CRef<COrfSearchTool>    m_Tool;
    
    wxChoice * m_DbCombo;
    wxChoice * m_GencodeCombo;
    wxChoice * m_StartorfCombo;
    wxTextCtrl * m_BasepairsText;
    vector<ISeqLocSearchContext*>   m_SeqLocContexts;
    
    string  m_Gencode;
    string  m_StartOrf;
    string  m_MinPairs;
    
};



///////////////////////////////////////////////////////////////////////////////
/// COrfSearchJob
///
class COrfSearchJob : public CSearchJobBase
{
public:
    COrfSearchJob(COrfSearchQuery& query, const COrfSearchForm::TKozakList& kozakList);

protected:
    typedef COrfSearchQuery::SScopedLoc     TScopedLoc;
    typedef COrfSearchQuery::TScopedLocs    TScopedLocs;
    typedef CRangeCollection<TSeqPos>       TRangeCollection;

    /// @name CSearchJobBase overridables
    /// @{
    virtual bool    x_ValidateParams();
    virtual IAppJob::EJobState    x_DoSearch();
    virtual void    x_SetupColumns(CObjectList& obj_list);

    virtual CObjectListTableModel* x_GetNewOLTModel() const;
    /// @}

    int    x_DecodeGeneticCode(const string& s);

    void   x_AddToResults(CObject& obj, objects::CScope& scope, 
                           int start, 
                           int stop, 
                           const string& strand,
                           int length, 
                           int p_length,
                           const string& start_cdn,
                           const string& stop_cdn,
                           const string& translated_seq,
                           const string& nucleotide_seq,
                           const string& start_context,
                           const string& ctx_name,
                           const string& kozak,
                           int   k_loc,
                           int color);


protected:
    CRef<COrfSearchQuery> m_Query;
    int m_ColorCol;
    COrfSearchForm::TKozakList m_KozakList;
};


/* @} */

END_NCBI_SCOPE

#endif  // GUI_SERVICES___ORF_SEARCH_TOOL_HPP
