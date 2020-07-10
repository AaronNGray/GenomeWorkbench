#ifndef GUI_CORE___SEARCH_FORM_BASE_HPP
#define GUI_CORE___SEARCH_FORM_BASE_HPP

/*  $Id: search_form_base.hpp 39744 2017-10-31 21:12:13Z katargir $
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

#include <gui/core/search_tool_base.hpp>
#include <gui/core/data_mining_context.hpp>

#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/Gene_ref.hpp>

#include <gui/utils/command.hpp>
#include <gui/utils/mru_list.hpp>

#include <gui/objutils/registry.hpp>

#include <wx/srchctrl.h>

#include <gui/widgets/wx/auto_complete_combo.hpp>

class wxChoice;


BEGIN_NCBI_SCOPE

class CSearchControl;

///////////////////////////////////////////////////////////////////////////////
/// CSearchFormBase
class NCBI_GUICORE_EXPORT CSearchFormBase :
        public CObjectEx,
        public IDMSearchForm
{
public:
    typedef vector<IDataMiningContext*>   TContexts;
    typedef CMRUList<string>              TMRUInputs;

    typedef objects::CFeatListItem    TFeatTypeItem;
    typedef set<TFeatTypeItem>      TFeatTypeItemSet;

    typedef CSearchToolBase::EPatternType TPatternType;
    typedef map<string, TPatternType>   TNameToType;

public:
    CSearchFormBase();
    virtual ~CSearchFormBase();

    virtual void    SetController(IDMSearchFormController* controller);
    
    virtual IDMSearchTool*  x_GetTool() = 0;

    virtual wxSizer*  GetWidget(wxWindow * parent);

    virtual CIRef<IDMSearchQuery> ConstructQuery();
    virtual void UpdateContexts();

    void UpdateContextCombo(wxChoice* combo);

    virtual void PopupADialog(TDialogType dt);    
    virtual void SetDictionary(list<string> * ddd);
    virtual void Push();
    virtual void SetRangeLimit(bool bLimit);

    virtual string  GetMainValue();
    virtual void    SetMainValue(string val);
    
    virtual void    SetRegistryPath(const string& reg_path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    
protected:
    virtual void    x_LoadSettings(const CRegistryReadView& /*view*/) = 0;
    virtual void    x_SaveSettings(CRegistryWriteView /*view*/) const = 0;

protected:
    TContexts                   m_Contexts;
    IDataMiningContext*         m_ActiveContext;
    TFeatTypeItemSet            m_FeatTypesSet;
    IDMSearchFormController*    m_Controller;   
    wxSizer*                    m_Sizer;
    CSearchControl*             m_Text; 
    string                      m_RegPath;

    string  m_Input;
    bool    m_bRange;

private:
    CSearchFormBase(const CSearchFormBase&);
    CSearchFormBase& operator=(const CSearchFormBase&);
};

///////////////////////////////////////////////////////////////////////////////
/// CSearchControl
// helpful autocomplete control
class NCBI_GUICORE_EXPORT CSearchControl : public CAutoCompleteCombo
{
public:
    CSearchControl(wxWindow *parent, wxWindowID id,
               const wxString& value = wxEmptyString,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = 0,
               const wxValidator& validator = wxDefaultValidator,
               const wxString& name = wxSearchCtrlNameStr);

    /// another constructor.
    /// This is just for serving the code created using DialogBlock.
    CSearchControl(wxWindow *parent, wxWindowID id,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = 0,
               const wxValidator& validator = wxDefaultValidator,
               const wxString& name = wxSearchCtrlNameStr);
    
    void SetDictionary(list<string> * dict);
    void Push();

protected:

    size_t m_MaxSize;
    bool m_bDoSelectAll;
    bool m_bIgnoreNextTextEdit;
    list<string> * m_pDict;
};


END_NCBI_SCOPE


#endif  // GUI_CORE___SEARCH_FORM_BASE_HPP
