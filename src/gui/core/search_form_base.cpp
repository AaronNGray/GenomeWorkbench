/*  $Id: search_form_base.cpp 39745 2017-10-31 21:21:36Z katargir $
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

#include <ncbi_pch.hpp>

#include <gui/core/search_form_base.hpp>
#include <gui/core/data_mining_service.hpp>
#include <gui/core/seqloc_search_context.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/widgets/seq/feature_check_dialog.hpp>

#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>

#include <wx/sizer.h>
#include <wx/srchctrl.h>
#include <wx/choice.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CSearchFormBase

#define DICTIONARY_MAX_SIZE 50

CSearchControl::CSearchControl (wxWindow* parent, wxWindowID id, const wxString& value,
                                const wxPoint& pos, const wxSize& size,
                                long style, const wxValidator& validator,
                                const wxString& name)
    : CAutoCompleteCombo(parent, id, value, pos, size, wxArrayString(), style, validator, name)
    , m_MaxSize(DICTIONARY_MAX_SIZE)
    , m_bDoSelectAll(false)
    , m_bIgnoreNextTextEdit(false)
    , m_pDict(NULL)
{
}

CSearchControl::CSearchControl(wxWindow *parent, wxWindowID id,
                               const wxPoint& pos,
                               const wxSize& size,
                               long style,
                               const wxValidator& validator,
                               const wxString& name)
    : CAutoCompleteCombo(parent, id, wxT(""), pos, size, wxArrayString(), style, validator, name)
    , m_MaxSize(DICTIONARY_MAX_SIZE)
    , m_bDoSelectAll(false)
    , m_bIgnoreNextTextEdit(false)
    , m_pDict(NULL)
{
}

class StringCmp
{
public:
    string m_Str;

    StringCmp(string str) {m_Str = str;}

    bool operator()(string str)
    {
        return str.find(m_Str) == 0;
    }
};

void CSearchControl::SetDictionary(list<string> * dict)
{
    m_pDict = dict;
    
    wxArrayString items;
    ITERATE (list<string>, str, *m_pDict) {
        items.Add(ToWxString(*str));
    }    
    SetBaseItems(items);
}

void CSearchControl::Push()
{
    if (m_pDict) {
        while (m_pDict->size() >= m_MaxSize) {
            m_pDict->pop_back();
            if (GetCount()) Delete(0);
        }
        string value = ToStdString(GetValue());
        if (value.size() && (find(m_pDict->begin(), m_pDict->end(), value)==m_pDict->end())) {
            m_pDict->push_front(value);
            Append(GetValue());
        }
        //m_pDict->unique();        
    } 
}

///////////////////////////////////////////////////////////////////////////////
/// CSearchFormBase
CSearchFormBase::CSearchFormBase()
:   m_ActiveContext(NULL),
    m_Controller(NULL),
    m_Sizer(NULL),
    m_Text(NULL),
    m_bRange(false)
{

    const CFeatList* feat_list = CSeqFeatData::GetFeatList();
    ITERATE(CFeatList, ft_it, *feat_list) {
        const CFeatListItem& item = *ft_it;
        string desc = item.GetDescription();
        int feat_type = item.GetType();

        if (feat_type == CSeqFeatData::e_Rna  ||
            feat_type == CSeqFeatData::e_Gene ||
            feat_type == CSeqFeatData::e_Cdregion) {
            m_FeatTypesSet.insert(item);
        }
    }
}


CSearchFormBase::~CSearchFormBase()
{
}


void CSearchFormBase::SetController(IDMSearchFormController* controller)
{
    m_Controller = controller;
}

void CSearchFormBase::Push()
{
    if (m_Text) {
        m_Text->Push();
    }
}

void CSearchFormBase::SetRangeLimit(bool bLimit)
{
    m_bRange = bLimit;
}


wxSizer* CSearchFormBase::GetWidget(wxWindow * parent)
{
    return NULL;
}


CIRef<IDMSearchQuery> CSearchFormBase::ConstructQuery()
{
    CIRef<IDMSearchQuery> ref(new CSearchQueryNull());
    return ref;
}


/// updates m_ContextCombo
void CSearchFormBase::UpdateContexts()
{
    m_ActiveContext = 0;

    m_Contexts.clear();

    CDataMiningService* srv = m_Controller->GetDataMiningService();
    if(srv) {
        /// create a list of compatible contexts
        vector<string> context_names;

        /// Add "Active" Context
        IDataMiningContext* act_context = srv->GetLastActiveContext();
        bool ok = false;

        if (act_context) {
            ok = x_GetTool()->IsCompatible(act_context);
            if (ok) {
                m_ActiveContext = act_context;
            }
        }

        /// now add all the compatible contexts
        vector<IDataMiningContext*> all_contexts;
        srv->GetContexts(all_contexts);

        for( size_t i = 0;  i < all_contexts.size();  i++ )   {
            IDataMiningContext* ctx = all_contexts[i];
            ok = x_GetTool()->IsCompatible(ctx);
            if(ok)  {
                m_Contexts.push_back(ctx);
                //string name = ctx->GetDMContextName();
                //context_names.push_back(name);
            }
        }

        /// initilize the ComboBox with the list of context names
        // DDX_ComboBox(m_ContextCombo, context_names, eDDX_Set);
    }

    bool bEnable = false;

    ITERATE(TContexts, cont, m_Contexts) {
        ISeqLocSearchContext* seq_ctx =
            dynamic_cast<ISeqLocSearchContext*>(*cont);
        if (seq_ctx && !seq_ctx->GetSearchLoc().Empty()){
            bEnable=true;
            break;
        }
    }
    if (m_Controller) {
        m_Controller->OnSearchEnabled(bEnable);
    }
}

void CSearchFormBase::UpdateContextCombo(wxChoice* combo)
{
    combo->Clear();

    combo->Append(wxT("All Searchable Contexts (") +
                      ToWxString(NStr::SizetToString(m_Contexts.size())) +
                      wxT(" available)"));       
    combo->Select(0);

    for( size_t i = 0;  i < m_Contexts.size();  i++ ) {
        if (m_Contexts[i]) {
            string cname = m_Contexts[i]->GetDMContextName();
            if (cname.length() > 40) {
                cname = cname.substr(0, 40) + "...";
            }
            combo->Append(ToWxString(cname));

            if (m_ActiveContext && (m_ActiveContext->GetDMContextName() == m_Contexts[i]->GetDMContextName())) {
                combo->Select((int)(i+1));
            }
        }
    }
}

/// TODO this function should be here - it should be move the the form
/// that needs this dialog

/// popup a dialog for additional info
void CSearchFormBase::PopupADialog(TDialogType dt)
{
    switch (dt) {
    case IDMSearchForm::FEATURE_SEL_DIALOG:
        CFeatureCheckDialog dlg(NULL);
        dlg.SetRegistryPath(m_RegPath);
        dlg.SetSelected(m_FeatTypesSet); //default values
        dlg.ShowModal();
        dlg.GetSelected(m_FeatTypesSet);
        break;
    }
}


void CSearchFormBase::SetDictionary(list<string> * ddd)
{
    if (m_Text) {
        m_Text->SetDictionary(ddd);
    }
}


void CSearchFormBase::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}


void CSearchFormBase::LoadSettings()
{
    if (!m_RegPath.empty())
        x_LoadSettings(CGuiRegistry::GetInstance().GetReadView(m_RegPath));
}


void CSearchFormBase::SaveSettings() const
{
    if (!m_RegPath.empty())
        x_SaveSettings(CGuiRegistry::GetInstance().GetWriteView(m_RegPath));
}


string  CSearchFormBase::GetMainValue()
{
    return m_Text ? ToStdString(m_Text->GetValue()) : string("#empty#");
}

void    CSearchFormBase::SetMainValue(string val)
{
    if (m_Text) {
        wxString wVal = ToWxString(val);
        if (wVal != m_Text->GetValue())
            m_Text->SetValue(wVal);
    }
}


END_NCBI_SCOPE
