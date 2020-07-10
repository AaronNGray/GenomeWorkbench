/*  $Id: constraint_panel.hpp 45100 2020-05-29 20:38:10Z asztalos $
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
 * Authors:  Igor Filippov
 *
 * File Description:
 *
 */

#ifndef CONSTRAINT_PANEL__HPP
#define CONSTRAINT_PANEL__HPP


#include <corelib/ncbistd.hpp>
#include <wx/listbook.h>
#include <wx/choicebk.h>
#include <wx/checklst.h>

#include <objects/macro/String_constraint.hpp>
#include <gui/widgets/edit/cds_gene_prot_field_name_panel.hpp>
#include <gui/widgets/edit/rna_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/parse_text_options_dlg.hpp>
#include <gui/widgets/edit/edit_text_options_panel.hpp>
#include <gui/packages/pkg_sequence_edit/remove_text_outside_string_panel.hpp>
#include <gui/packages/pkg_sequence_edit/cap_change_panel.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>


class wxListbook;
class wxComboBox;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */
#define ID_CONSTRAINT_CHOICEBOOK 14002
#define ID_CONSTRAINT_ADD 14003
#define ID_CONSTRAINT_REMOVE 14004
#define ID_CONSTRAINT_LIST 14005

class CConstraintPanelBase
{
public:
    virtual ~CConstraintPanelBase() {}
    virtual  CRef<CEditingActionConstraint> GetConstraint(const string &field, CFieldNamePanel::EFieldType field_type, int subtype) = 0;
};

class CConstraintPanel: public wxPanel, public CFieldNamePanelParent, public CConstraintPanelBase
{    
    DECLARE_DYNAMIC_CLASS( CConstraintPanel )
    DECLARE_EVENT_TABLE()

public:
    CConstraintPanel();
    CConstraintPanel( wxWindow* parent, objects::CSeq_entry_Handle seh, vector<const objects::CFeatListItem *> *featlist = nullptr, 
                      wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL|wxSIMPLE_BORDER );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    ~CConstraintPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    void SetSelection(int page);
    CFieldNamePanel* GetFieldNamePanel(void);
    CRef<edit::CStringConstraint> GetStringConstraint(void);
    CStringConstraintPanel*  GetStringConstraintPanel() {return m_StringConstraintPanel;}
    string GetFieldType(void) const;
    bool SetFieldName(const string& field);
    void ClearValues(void);

    // inherited from CFieldNamePanelParent
    virtual void UpdateEditor(void);

    void OnPageChanged(wxBookCtrlEvent& event);
    void ListPresentFeaturesFirst(objects::CSeq_entry_Handle seh) {m_feat_field_panel->ListPresentFeaturesFirst(seh);}
    virtual CRef<CEditingActionConstraint> GetConstraint(const string &field, CFieldNamePanel::EFieldType field_type, int subtype);

private:
    wxChoicebook* m_Notebook;
    CStringConstraintPanel*  m_StringConstraintPanel;
    CFeatureFieldNamePanel *m_feat_field_panel;
    objects::CSeq_entry_Handle m_TopSeqEntry;
    vector<const objects::CFeatListItem *> *m_featlist;
};

struct SFieldTypeAndMatcher
{
    SFieldTypeAndMatcher() : field_type(CFieldNamePanel::eFieldType_Unknown), subtype(-1), enabled(false), subtype2(-1), enabled2(false), top_level(-1), second_level(-1) {}
    string field; 
    CFieldNamePanel::EFieldType field_type; 
    int subtype;
    CRef<CConstraintMatcher> matcher;
    bool enabled;
    string field2; 
    CFieldNamePanel::EFieldType field_type2; 
    int subtype2;
    CRef<CConstraintMatcher> matcher2;
    bool enabled2;
    int top_level;
    int second_level;
    string full_field;
    string full_field2;
    CRef<CString_constraint> string_constraint;
    CRef<objects::CLocation_constraint> location_constraint;
};

class CCompoundConstraintPanel: public wxPanel, public CConstraintPanelBase
{    
    DECLARE_DYNAMIC_CLASS( CCompoundConstraintPanel )
    DECLARE_EVENT_TABLE()

public:
    CCompoundConstraintPanel();
    CCompoundConstraintPanel( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    ~CCompoundConstraintPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    void SetSelection(int page);  
    void ClearValues(void);

    virtual CRef<CEditingActionConstraint> GetConstraint(const string &field, CFieldNamePanel::EFieldType field_type, int subtype);
    void OnAddConstraint( wxCommandEvent& event );
    void OnRemoveConstraint( wxCommandEvent& event );
    void OnEditConstraint( wxCommandEvent& event );
    void AddConstraint(pair<string, SFieldTypeAndMatcher > item, int id);
private:
    wxCheckListBox *m_CheckListBox;
    objects::CSeq_entry_Handle m_TopSeqEntry;
    map<string, SFieldTypeAndMatcher > m_label_to_constraint;
    int m_page;
};

class CAdvancedConstraintPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CAdvancedConstraintPanel )
    DECLARE_EVENT_TABLE()

public:
    CAdvancedConstraintPanel();
    CAdvancedConstraintPanel( wxWindow* parent, objects::CSeq_entry_Handle seh, vector<const objects::CFeatListItem *> *featlist = nullptr, 
                              wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    ~CAdvancedConstraintPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    void SetSelection(int page);  
    void ClearValues(void);

    CRef<CEditingActionConstraint> GetConstraint(const string &field, CFieldNamePanel::EFieldType field_type, int subtype);
private:
    wxNotebook* m_Notebook;
    CConstraintPanel*  m_BasicConstraintPanel;
    CCompoundConstraintPanel* m_CompoundConstraintPanel;
    objects::CSeq_entry_Handle m_TopSeqEntry;
    vector<const objects::CFeatListItem *> *m_featlist;
};


END_NCBI_SCOPE

#endif  // CONSTRAINT_PANEL__HPP
