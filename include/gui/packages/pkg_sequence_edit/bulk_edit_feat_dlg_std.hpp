/*  $Id: bulk_edit_feat_dlg_std.hpp 39649 2017-10-24 15:22:12Z asztalos $
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
 * Authors:  Colleen Bollin
 */
#ifndef _BULK_EDIT_FEAT_DLG_H_
#define _BULK_EDIT_FEAT_DLG_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/widgets/edit/feature_type_panel.hpp>
#include <gui/packages/pkg_sequence_edit/field_constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/edit_feat_loc_panel.hpp>
#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>
#include <gui/widgets/edit/single_experiment_panel.hpp>
#include <gui/widgets/edit/inference_panel.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/radiobut.h>
#include <wx/choice.h>
#include <wx/notebook.h>
#include <wx/combobox.h>
#include <wx/radiobox.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class CFeatureTypePanel;
class wxBoxSizer;
class CFieldConstraintPanel;
class COkCancelPanel;
class CConstraintPanel;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CBULKEDITFEATDLG 10268
#define ID_EFL_FEATURETYPE 10269
#define ID_EFL_CONSTRAINT 10272
#define ID_EFL_OKCANCEL 10275
#define SYMBOL_CBULKEDITFEATDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CBULKEDITFEATDLG_TITLE _("BulkEditFeatDlg")
#define SYMBOL_CBULKEDITFEATDLG_IDNAME ID_CBULKEDITFEATDLG
#define SYMBOL_CBULKEDITFEATDLG_SIZE wxSize(800, 300)
#define SYMBOL_CBULKEDITFEATDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CBulkEditFeatDlg class declaration
 */

class CBulkEditFeatDlg: public CBulkCmdDlg , public CFieldNamePanelParent
{    
    DECLARE_DYNAMIC_CLASS( CBulkEditFeatDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CBulkEditFeatDlg();
    CBulkEditFeatDlg( wxWindow* parent, IWorkbench* wb, wxWindowID id = SYMBOL_CBULKEDITFEATDLG_IDNAME, const wxString& caption = SYMBOL_CBULKEDITFEATDLG_TITLE, const wxPoint& pos = SYMBOL_CBULKEDITFEATDLG_POSITION, const wxSize& size = SYMBOL_CBULKEDITFEATDLG_SIZE, long style = SYMBOL_CBULKEDITFEATDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, IWorkbench* wb, wxWindowID id = SYMBOL_CBULKEDITFEATDLG_IDNAME, const wxString& caption = SYMBOL_CBULKEDITFEATDLG_TITLE, const wxPoint& pos = SYMBOL_CBULKEDITFEATDLG_POSITION, const wxSize& size = SYMBOL_CBULKEDITFEATDLG_SIZE, long style = SYMBOL_CBULKEDITFEATDLG_STYLE );

    /// Destructor
    ~CBulkEditFeatDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CBulkEditFeatDlg event handler declarations

    void ProcessUpdateFeatEvent( wxCommandEvent& event );

////@end CBulkEditFeatDlg event handler declarations
    void UpdateChildrenFeaturePanels( wxWindow* win );

////@begin CBulkEditFeatDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CBulkEditFeatDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CBulkEditFeatDlg member variables
    CFeatureTypePanel* m_FeatureType;
    CConstraintPanel* m_Constraint;
    COkCancelPanel* m_OkCancel;
////@end CBulkEditFeatDlg member variables

    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();

    enum EEditingType {
        eEvidence = 0,
        eLocation,
        eStrand,
        eRevInt,
        eResynch,
        ePseudo,
        eException,
        eExperiment,
        eInference
    };

    void SetEditingType(EEditingType editing_type);

    // for CFieldNameParent, so that we can update the size after changing field constraint
    virtual void UpdateEditor() {}
private:
    bool x_IsRefSeq();

    virtual void SetRegistryPath(const string& reg_path);  
    virtual void LoadSettings();
    virtual void SaveSettings() const;

    wxNotebook *m_Notebook;
    string m_ErrorMessage;
    string m_RegPath;
};

class CBulkFeatEditCmdPanel
{
public:
    virtual ~CBulkFeatEditCmdPanel() {}
    virtual bool AddCommand(vector<CConstRef<CObject> > &objs, CRef<CCmdComposite> cmd, CScope &scope) { return false;}
};

class CEvidencePanel: public wxPanel, public CBulkFeatEditCmdPanel
{    
    DECLARE_DYNAMIC_CLASS( CEvidencePanel )
    DECLARE_EVENT_TABLE()

public:
    CEvidencePanel();
    CEvidencePanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    ~CEvidencePanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    virtual bool AddCommand(vector<CConstRef<CObject> > &objs, CRef<CCmdComposite> cmd, CScope &scope);
};

class CBlkEdtFeatLocationPanel: public wxPanel, public CBulkFeatEditCmdPanel
{    
    DECLARE_DYNAMIC_CLASS( CBlkEdtFeatLocationPanel )
    DECLARE_EVENT_TABLE()

public:
    CBlkEdtFeatLocationPanel();
    CBlkEdtFeatLocationPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    ~CBlkEdtFeatLocationPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    virtual bool AddCommand(vector<CConstRef<CObject> > &objs, CRef<CCmdComposite> cmd, CScope &scope);
private:
    CEditFeatLocPanel *m_EditLoc;
};

class CStrandPanel: public wxPanel, public CBulkFeatEditCmdPanel
{    
    DECLARE_DYNAMIC_CLASS( CStrandPanel )
    DECLARE_EVENT_TABLE()

public:
    CStrandPanel();
    CStrandPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    ~CStrandPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    virtual bool AddCommand(vector<CConstRef<CObject> > &objs, CRef<CCmdComposite> cmd, CScope &scope);
private:
    wxChoice* m_StrandFrom;
    wxChoice* m_StrandTo;
};

class CRevIntPanel: public wxPanel, public CBulkFeatEditCmdPanel
{    
    DECLARE_DYNAMIC_CLASS( CRevIntPanel )
    DECLARE_EVENT_TABLE()

public:
    CRevIntPanel();
    CRevIntPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    ~CRevIntPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    virtual bool AddCommand(vector<CConstRef<CObject> > &objs, CRef<CCmdComposite> cmd, CScope &scope);
};

class CResyncPanel: public wxPanel, public CBulkFeatEditCmdPanel
{    
    DECLARE_DYNAMIC_CLASS( CResyncPanel )
    DECLARE_EVENT_TABLE()

public:
    CResyncPanel();
    CResyncPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    ~CResyncPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    virtual bool AddCommand(vector<CConstRef<CObject> > &objs, CRef<CCmdComposite> cmd, CScope &scope);
};

class CPseudoPanel: public wxPanel, public CBulkFeatEditCmdPanel
{    
    DECLARE_DYNAMIC_CLASS( CPseudoPanel )
    DECLARE_EVENT_TABLE()

public:
    CPseudoPanel();
    CPseudoPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    ~CPseudoPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    virtual bool AddCommand(vector<CConstRef<CObject> > &objs, CRef<CCmdComposite> cmd, CScope &scope);
private:
    wxChoice* m_PseudogeneChoice;
};

class CExceptionPanel: public wxPanel, public CBulkFeatEditCmdPanel
{    
    DECLARE_DYNAMIC_CLASS( CExceptionPanel )
    DECLARE_EVENT_TABLE()

public:
    CExceptionPanel();
    CExceptionPanel( wxWindow* parent, bool is_refseq, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    ~CExceptionPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    virtual bool AddCommand(vector<CConstRef<CObject> > &objs, CRef<CCmdComposite> cmd, CScope &scope);
private:
    wxComboBox* m_Exception;
    wxRadioBox* m_FeatureProduct;
    wxCheckBox* m_MoveToComment;
    bool m_is_refseq;
};

class CExperimentPanel: public wxPanel, public CBulkFeatEditCmdPanel
{    
    DECLARE_DYNAMIC_CLASS( CExperimentPanel )
    DECLARE_EVENT_TABLE()

public:
    CExperimentPanel();
    CExperimentPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    ~CExperimentPanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    virtual bool AddCommand(vector<CConstRef<CObject> > &objs, CRef<CCmdComposite> cmd, CScope &scope);
    void OnCreateOrDelete(wxCommandEvent& event);
private:
    wxRadioBox* m_CreateOrDelete;
    CSingleExperimentPanel *m_SingleExperiment;
};


class CEditInferencePanel: public wxPanel, public CBulkFeatEditCmdPanel
{    
    DECLARE_DYNAMIC_CLASS( CEditInferencePanel )
    DECLARE_EVENT_TABLE()

public:
    CEditInferencePanel();
    CEditInferencePanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
    ~CEditInferencePanel();
    void Init();
    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
    virtual bool AddCommand(vector<CConstRef<CObject> > &objs, CRef<CCmdComposite> cmd, CScope &scope);
    void OnCreateOrDelete(wxCommandEvent& event);
private:
    wxRadioBox* m_CreateOrDelete;
    CInferencePanel *m_SingleInference;
};

END_NCBI_SCOPE

#endif
    // _BULK_EDIT_FEAT_DLG_H_
