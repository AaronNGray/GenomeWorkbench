/*  $Id: segregate_sets.hpp 43855 2019-09-10 18:05:26Z filippov $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Igor Filippov
 */
#ifndef _SEGREGATE_SETS_H_
#define _SEGREGATE_SETS_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/field_choice_panel.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
#include <gui/widgets/edit/set_class_panel.hpp>

#include <wx/dialog.h>
#include <wx/notebook.h>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/frame.h>
#include <wx/sizer.h>
#include <wx/listctrl.h>
#include <wx/treectrl.h>
#include <wx/bmpbuttn.h>
#include <gui/widgets/wx/wx_utils.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSEGREGATE_SETS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CSEGREGATE_SETS_TITLE _("Segregate Sets")
#define SYMBOL_CSEGREGATE_SETS_IDNAME ID_CSEGREGATE_SETS
#define SYMBOL_CSEGREGATE_SETS_SIZE wxDefaultSize
#define SYMBOL_CSEGREGATE_SETS_POSITION wxDefaultPosition
////@end control identifiers


class CSegregateSetsBase 
{
public:
    virtual ~CSegregateSetsBase() {}
    objects::CSeq_entry_Handle GetSeqEntry(unsigned int i) {return m_SetSeq[i].entry;}
    size_t GetSetSize() {return m_SetSeq.size();}
    objects::CSeq_entry_Handle GetTopSeqEntry(){ return m_TopSeqEntry; }

protected:
    objects::CSeq_entry_Handle m_TopSeqEntry;
    struct SSetSeqRecord
    {
        objects::CBioseq_set_Handle grandparent;
        objects::CBioseq_set_Handle parent;
        objects::CSeq_entry_Handle entry;
    };
    vector < SSetSeqRecord > m_SetSeq;
    virtual void PopulateSet(objects::CBioseq_set& new_set, const vector<int> &subset, CBioseq_set::EClass top_class);
    void ReadBioseq(const objects::CSeq_entry& se);
    void TakeFromSet(objects::CBioseq_set& changed_set, CRef<objects::CSeq_entry> se);
    void MoveDescrToEntries(objects::CBioseq_set& se);
    void AddUserObject(objects::CBioseq_set& se);
};

class CSegregateSets : public CSegregateSetsBase,  public wxFrame
{    
    DECLARE_DYNAMIC_CLASS( CSegregateSets )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSegregateSets();
    CSegregateSets( wxWindow* parent, objects::CSeq_entry_Handle seh, ICommandProccessor* cmdProcessor, wxWindowID id = SYMBOL_CSEGREGATE_SETS_IDNAME, const wxString& caption = SYMBOL_CSEGREGATE_SETS_TITLE, const wxPoint& pos = SYMBOL_CSEGREGATE_SETS_POSITION, const wxSize& size = SYMBOL_CSEGREGATE_SETS_SIZE, long style = SYMBOL_CSEGREGATE_SETS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSEGREGATE_SETS_IDNAME, const wxString& caption = SYMBOL_CSEGREGATE_SETS_TITLE, const wxPoint& pos = SYMBOL_CSEGREGATE_SETS_POSITION, const wxSize& size = SYMBOL_CSEGREGATE_SETS_SIZE, long style = SYMBOL_CSEGREGATE_SETS_STYLE );

    /// Destructor
    virtual ~CSegregateSets();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual CRef<CCmdComposite> GetCommand();
    vector< vector<int> > & GetSubsets();
    void SetSubsets(const set<objects::CBioseq_Handle> &selected);

    void OnCancel( wxCommandEvent& event );
    void OnAccept( wxCommandEvent& event );

    enum {
        ID_CSEGREGATE_SETS = 11200,
        ID_CSEGREGATE_NOTEBOOK,
        ID_CSEGREGATE_IDTAB,
        ID_CSEGREGATE_TEXTTAB,
        ID_CSEGREGATE_NUMSLENTAB,
        ID_CSEGREGATE_NUMSETSTAB,
        ID_CSEGREGATE_FNAMETAB,
        ID_CSEGREGATE_FIELDTAB,
        ID_CSEGREGATE_FEATTYPETAB,
        ID_CSEGREGATE_DESCTYPETAB,
        ID_CSEGREGATE_MOLINFOTAB,
        ID_CSEGREGATE_STRCOMMENTTAB
    };
private:
    wxNotebook *m_Notebook;
    CSetClassPanel *m_SetClassPanel;
    ICommandProccessor* m_CmdProcessor;
    wxCheckBox* m_LeaveUp;
};


class CRBSubpanel: public wxPanel
{
 DECLARE_DYNAMIC_CLASS( CRBSubpanel )
 DECLARE_EVENT_TABLE()

public:
    CRBSubpanel() {  Init(); }
    CRBSubpanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) { Init(); Create(parent, id, pos, size, style); }
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );       
    virtual ~CRBSubpanel() {}
    bool ShowToolTips()  { return true; }
    wxBitmap GetBitmapResource( const wxString& name ) {   wxUnusedVar(name);   return wxNullBitmap; }
    wxIcon GetIconResource( const wxString& name ) {  wxUnusedVar(name);   return wxNullIcon; }
    void CreateControlsHeader();
    void CreateControlsFooter();
    virtual void CreateControlsMiddle() {}
    virtual void Init();
    void CreateControls();    
    void UpdateList();
    void OnButtonMoveLeft( wxCommandEvent& event );
    void OnButtonMoveRight( wxCommandEvent& event );
    virtual void OnSelectButton( CSegregateSetsBase *win ) {}
    void OnSelect( wxCommandEvent& event );
    void OnSelectAll( wxCommandEvent& event );
    void OnUnselectAll( wxCommandEvent& event );
    vector< vector<int> > & GetSubsets() {return this->m_Subsets;}
    virtual void SetClusters() {}
    CSegregateSetsBase *GetBaseFrame();
    string GetLabel(objects::CSeq_entry_Handle seh);
    bool MatchConstraint(CRef<objects::edit::CStringConstraint> constraint, objects::CSeq_entry_Handle seh);
    CRBSubpanel * GetEventCaller(wxCommandEvent& event);
    void SetSubsets(const set<objects::CBioseq_Handle> &selected);
protected:    
    wxTreeCtrl* m_TreeCtrl;
    wxListCtrl* m_ListCtrl2;
    vector< vector<int> > m_Subsets;
    vector<int> m_Available;
    map<string, vector<pair<int,string> > > m_Clusters;
    class ItemData : public wxTreeItemData 
    {
    public:
        ItemData(int data) : m_data(data) {}
        int GetData() {return m_data;}
    private:
        int m_data;
    };
private:
    enum {
        ID_RBSUB_TOLEFT = 8000,
        ID_RBSUB_TORIGHT
    };
    void CombineLabels(const objects::CSeq_id &id, vector<string> &labels);
    void CollectLabels(objects::CSeq_entry_Handle seh, list<string> &strs);   
    void CombineLabelsAll(const CSeq_id &id, set<string> &labels);
    void CollectLabelsAll(CSeq_entry_Handle seh, set<string> &strs);
};

class CLengthSubpanel: public CRBSubpanel
{
 DECLARE_DYNAMIC_CLASS( CLengthSubpanel )
 DECLARE_EVENT_TABLE()

public:
    CLengthSubpanel() {  Init(); }
    CLengthSubpanel( wxWindow* parent,  wxWindowID id, const wxPoint& pos, const wxSize& size, long style)   { Init();  Create(parent, id, pos, size, style);}   
    virtual void CreateControlsMiddle();
    virtual void OnSelectButton( CSegregateSetsBase *win );
    virtual void SetClusters();
private:
    wxTextCtrl *m_Min;
    wxTextCtrl *m_Max;

    enum {
        ID_LENGTH_MINTEXT = 8010,
        ID_LENGTH_MAXTEXT
    };
};

class CIDSubpanel: public CRBSubpanel
{
 DECLARE_DYNAMIC_CLASS( CIDSubpanel )
 DECLARE_EVENT_TABLE()

public:
    CIDSubpanel() {  Init(); }
    CIDSubpanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)  { Init();  Create(parent, id, pos, size, style);}   
    virtual void CreateControlsMiddle();
    virtual void OnSelectButton( CSegregateSetsBase *win );
    virtual void SetClusters();
private:   
    CStringConstraintPanel* m_StringConstraintPanel;
};

class CConstraintPanel;

class CTextSubpanel: public CRBSubpanel
{
 DECLARE_DYNAMIC_CLASS( CTextSubpanel )
 DECLARE_EVENT_TABLE()

public:
    CTextSubpanel() {  Init(); }
    CTextSubpanel( wxWindow* parent,  wxWindowID id, const wxPoint& pos, const wxSize& size, long style)  { Init();  Create(parent, id, pos, size, style);}   
    virtual void CreateControlsMiddle();
    virtual void OnSelectButton( CSegregateSetsBase *win );
    void ProcessUpdateFeatEvent( wxCommandEvent& event );
    virtual void SetClusters();
    void OnChoiceChanged( wxCommandEvent& event );
    void OnPageChanged(wxBookCtrlEvent& event);
private: 
    CConstraintPanel* m_Constraint;
    void UpdateChildrenFeaturePanels( wxSizer* sizer );
};


class CNumSubpanel: public CRBSubpanel
{
 DECLARE_DYNAMIC_CLASS( CNumSubpanel )
 DECLARE_EVENT_TABLE()

public:
    CNumSubpanel() {  Init(); }
    CNumSubpanel( wxWindow* parent,  wxWindowID id, const wxPoint& pos, const wxSize& size, long style)  { Init();  Create(parent, id, pos, size, style);}   
    virtual void CreateControlsMiddle();
    virtual void OnSelectButton( CSegregateSetsBase *win );
private:
    wxTextCtrl *m_Num; 

    enum {
        ID_NUM_TEXTCTRL = 8015
    };
};

class CFileSubpanel: public CRBSubpanel
{
 DECLARE_DYNAMIC_CLASS( CFileSubpanel )
 DECLARE_EVENT_TABLE()

public:
    CFileSubpanel() {  Init(); }
    CFileSubpanel( wxWindow* parent,  wxWindowID id, const wxPoint& pos, const wxSize& size, long style)  { Init();  Create(parent, id, pos, size, style);}   
    virtual void CreateControlsMiddle();
    virtual void OnSelectButton( CSegregateSetsBase *win );
    virtual void SetClusters();
private:
    string GetFilename(const objects::CObject_id &tag);
};

class CFieldSubpanel: public CRBSubpanel
{
 DECLARE_DYNAMIC_CLASS( CFieldSubpanel )
 DECLARE_EVENT_TABLE()

public:
    CFieldSubpanel() {  Init(); }
    CFieldSubpanel( wxWindow* parent,  wxWindowID id, const wxPoint& pos, const wxSize& size, long style)  { Init();  Create(parent, id, pos, size, style);}   
    virtual void CreateControlsMiddle();
    virtual void OnSelectButton( CSegregateSetsBase *win );
    void ProcessUpdateFeatEvent( wxCommandEvent& event );
    virtual void SetClusters();
    void OnChoiceChanged( wxCommandEvent& event );
private: 
    CFieldChoicePanel* m_FieldType;
    void UpdateChildrenFeaturePanels( wxSizer* sizer );
};

class CFeatureTypeSubpanel: public CRBSubpanel
{
 DECLARE_DYNAMIC_CLASS( CFeatureTypeSubpanel )
 DECLARE_EVENT_TABLE()

public:
    CFeatureTypeSubpanel() {  Init(); }
    CFeatureTypeSubpanel( wxWindow* parent,  wxWindowID id, const wxPoint& pos, const wxSize& size, long style)  { Init();  Create(parent, id, pos, size, style);}   
    virtual void CreateControlsMiddle();
    virtual void OnSelectButton( CSegregateSetsBase *win );
    virtual void SetClusters();
    void OnChoiceChanged( wxCommandEvent& event );
private: 
    CFeatureTypePanel* m_FeatureType;
};

class CDescTypeSubpanel: public CRBSubpanel
{
 DECLARE_DYNAMIC_CLASS( CDescTypeSubpanel )
 DECLARE_EVENT_TABLE()

public:
    CDescTypeSubpanel() {  Init(); }
    CDescTypeSubpanel( wxWindow* parent,  wxWindowID id, const wxPoint& pos, const wxSize& size, long style)  { Init();  Create(parent, id, pos, size, style);}   
    virtual void CreateControlsMiddle();
    virtual void OnSelectButton( CSegregateSetsBase *win );
    virtual void SetClusters();
    void OnChoiceChanged( wxCommandEvent& event );
private: 
    wxChoice* m_DescType;

    enum {
        ID_DESC_CHOICE = 8020
    };
};

class CMolInfoSubpanel: public CRBSubpanel
{
    DECLARE_DYNAMIC_CLASS( CMolInfoSubpanel )
    DECLARE_EVENT_TABLE()
    
    public:
    CMolInfoSubpanel() {  Init(); }
    CMolInfoSubpanel( wxWindow* parent,  wxWindowID id, const wxPoint& pos, const wxSize& size, long style)  { Init();  Create(parent, id, pos, size, style);}   
    virtual void CreateControlsMiddle();
    virtual void OnSelectButton( CSegregateSetsBase *win );
    virtual void SetClusters();
    void OnChoiceChanged( wxCommandEvent& event );
private: 
    wxCheckBox *m_Class;
    wxCheckBox *m_Type;
    wxCheckBox *m_Topology;

    enum {
        ID_MOLINFO_CHKBX1 = 8030,
        ID_MOLINFO_CHKBX2,
        ID_MOLINFO_CHKBX3
    };
};


class CStructCommentSubpanel: public CRBSubpanel
{
 DECLARE_DYNAMIC_CLASS( CStructCommentSubpanel )
 DECLARE_EVENT_TABLE()

public:
    CStructCommentSubpanel() {  Init(); }
    CStructCommentSubpanel( wxWindow* parent,  wxWindowID id, const wxPoint& pos, const wxSize& size, long style)  { Init();  Create(parent, id, pos, size, style);}   
    virtual void CreateControlsMiddle();
    virtual void OnSelectButton( CSegregateSetsBase *win );
    virtual void SetClusters();
    void OnChoiceChanged( wxCommandEvent& event );
private: 
    wxChoice* m_Label;
    void GetAllLabels(wxArrayString &choices);
    void GetLabelsFromSeq(objects::CSeq_entry_Handle seh, set<string> &labels);

    enum {
        ID_STRCOMMENT_CHOICE = 8040
    };
};

END_NCBI_SCOPE

#endif
    // _SEGREGATE_SETS_H_
