/*  $Id: edit_pub_dlg_std.hpp 42454 2019-02-28 18:47:55Z asztalos $
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
 * Authors:  Colleen Bollin, Igor Filippov
 */
#ifndef _EDIT_PUB_DLG_STD_H_
#define _EDIT_PUB_DLG_STD_H_

#include <corelib/ncbistd.hpp>
#include <objects/biblio/Author.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>
#include <gui/widgets/edit/pub_field_name_panel.hpp>
#include <gui/widgets/edit/authornames_panel.hpp>

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/radiobut.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include <wx/choice.h>
#include <wx/notebook.h>
#include <wx/combobox.h>
#include <wx/radiobox.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
class CFieldHandlerNamePanel;
class CStringConstraintPanel;
class COkCancelPanel;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */


////@begin control identifiers
#define ID_CEDITPUBDLG wxID_ANY
#define ID_PUB_STATUS_CONSTRAINT 10296
#define SYMBOL_CEDITPUBDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CEDITPUBDLG_TITLE _("Edit Publications")
#define SYMBOL_CEDITPUBDLG_IDNAME ID_CEDITPUBDLG
#define SYMBOL_CEDITPUBDLG_SIZE  wxSize(800, 300)
#define SYMBOL_CEDITPUBDLG_POSITION wxDefaultPosition
////@end control identifiers
#define ID_IMPORT_PUB_CONSTRAINT 10292
#define ID_IMPORT_PUB_SELECTED 10293
#define ID_CLEAR_AUTHORS 10294

////@end control identifiers

class CEditPubDlg: public CBulkCmdDlg , public CFieldNamePanelParent
{    
    DECLARE_DYNAMIC_CLASS( CEditPubDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CEditPubDlg();
    CEditPubDlg( wxWindow* parent, IWorkbench* wb, 
        wxWindowID id = SYMBOL_CEDITPUBDLG_IDNAME, 
        const wxString& caption = SYMBOL_CEDITPUBDLG_TITLE, 
        const wxPoint& pos = SYMBOL_CEDITPUBDLG_POSITION, 
        const wxSize& size = SYMBOL_CEDITPUBDLG_SIZE, 
        long style = SYMBOL_CEDITPUBDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CEDITPUBDLG_IDNAME, 
        const wxString& caption = SYMBOL_CEDITPUBDLG_TITLE, 
        const wxPoint& pos = SYMBOL_CEDITPUBDLG_POSITION,
        const wxSize& size = SYMBOL_CEDITPUBDLG_SIZE, 
        long style = SYMBOL_CEDITPUBDLG_STYLE );

    /// Destructor
    ~CEditPubDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();



    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CBulkEditFeatDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();


    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();

    
    // for CFieldNameParent, so that we can update the size after changing field constraint
    virtual void UpdateEditor() {}

    CConstRef<CObject> GetSelectedPub();
    void AppendToErrorMsg(const string &msg);
    string GetStatusConstraint() {return ToStdString(m_PubStatusConstraint->GetStringSelection());}
    string GetFieldName(const bool subfield) {return m_Fieldhandler->GetFieldName(subfield);}
    CRef<edit::CStringConstraint> GetStringConstraint() {return  m_StrConstraintPanel->GetStringConstraint();}
    CScope& GetScope() {return m_TopSeqEntry.GetScope();}
    CSeq_entry_Handle GetTopSeqEntry() {return m_TopSeqEntry;}
    CConstRef<objects::CSeq_submit> GetSeqSubmit() {return m_SeqSubmit;}
private:

    virtual void SetRegistryPath(const string& reg_path);  
    virtual void LoadSettings();
    virtual void SaveSettings() const;

    wxNotebook *m_Notebook;
    string m_ErrorMessage;
    string m_RegPath;
    CFieldHandlerNamePanel* m_Fieldhandler;
    CStringConstraintPanel* m_StrConstraintPanel;
    wxChoice* m_PubStatusConstraint;
    COkCancelPanel* m_OkCancel;
};

class CEditPubCmdPanel
{
public:
    virtual ~CEditPubCmdPanel() {}
    virtual  CRef<CCmdComposite> GetCommand(const vector<CConstRef<CObject>>& objs, const vector<CSeq_entry_Handle>& descr_ctxt) = 0;
};

class CEditPubSubpanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CEditPubSubpanel )
    DECLARE_EVENT_TABLE()

public:
    CEditPubSubpanel() : wxPanel() {  Init(); }
    CEditPubSubpanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) 
        { 
          Init(); 
          Create(parent, id, pos, size, style);
        }
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL )
        {
            wxPanel::Create( parent, id, pos, size, style );
            CreateControls();
            if (GetSizer())
            {
                GetSizer()->SetSizeHints(this);
            }
            Centre();
            return true;
        }
    virtual ~CEditPubSubpanel() {}
    bool ShowToolTips()  { return true; }
    wxBitmap GetBitmapResource( const wxString& name ) {   wxUnusedVar(name);   return wxNullBitmap; }
    wxIcon GetIconResource( const wxString& name ) {  wxUnusedVar(name);   return wxNullIcon; }
    virtual void Init() {}
    virtual void CreateControls() {}
};

class CReplaceSectionPanel : public CEditPubSubpanel, public CEditPubCmdPanel
{    
    DECLARE_DYNAMIC_CLASS( CReplaceSectionPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CReplaceSectionPanel(){  Init(); }
    CReplaceSectionPanel( wxWindow* parent,  CEditPubDlg* dlg, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL )  
        :    m_dlg(dlg)
        {  
            Init(); 
            Create(parent, id, pos, size, style);
        }
    virtual ~CReplaceSectionPanel() {}
    /// Creates the controls and sizers
    virtual void CreateControls();
    virtual  CRef<CCmdComposite> GetCommand(const vector<CConstRef<CObject>>& objs, const vector<CSeq_entry_Handle>& descr_ctxt);
private:
    wxCheckBox* m_AuthorListSection;
    wxCheckBox* m_TitleSection;
    wxCheckBox* m_AffiliationSection;
    CEditPubDlg* m_dlg;
};

class CReplaceEntirePubPanel : public CEditPubSubpanel, public CEditPubCmdPanel
{    
    DECLARE_DYNAMIC_CLASS( CReplaceEntirePubPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CReplaceEntirePubPanel(){  Init(); }
    CReplaceEntirePubPanel( wxWindow* parent,  CEditPubDlg* dlg, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL )  
        :    m_dlg(dlg)
        {  
            Init(); 
            Create(parent, id, pos, size, style);
        }
    virtual ~CReplaceEntirePubPanel() {}
    /// Creates the controls and sizers
    virtual void CreateControls();
    virtual  CRef<CCmdComposite> GetCommand(const vector<CConstRef<CObject>>& objs, const vector<CSeq_entry_Handle>& descr_ctxt);
private:
    CEditPubDlg* m_dlg;
};

class CReplaceSingleFieldPanel : public CEditPubSubpanel, public CEditPubCmdPanel
{    
    DECLARE_DYNAMIC_CLASS( CReplaceSingleFieldPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CReplaceSingleFieldPanel(){  Init(); }
    CReplaceSingleFieldPanel( wxWindow* parent,  CEditPubDlg* dlg, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL )  
        :    m_dlg(dlg)
        {  
            Init(); 
            Create(parent, id, pos, size, style);
        }
    virtual ~CReplaceSingleFieldPanel() {}
    /// Creates the controls and sizers
    virtual void CreateControls();
    virtual  CRef<CCmdComposite> GetCommand(const vector<CConstRef<CObject>>& objs, const vector<CSeq_entry_Handle>& descr_ctxt);

private:
    CFieldHandlerNamePanel* m_PubField;
    wxTextCtrl* m_FieldText;
    CEditPubDlg* m_dlg;
};


class CAuthorReplacePanel : public CEditPubSubpanel, public CEditPubCmdPanel
{    
    DECLARE_DYNAMIC_CLASS( CAuthorReplacePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAuthorReplacePanel(){  Init(); }
    CAuthorReplacePanel( wxWindow* parent,  CEditPubDlg* dlg, CRef<CAuth_list> authors, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL )  
        : m_dlg(dlg), m_Authors(authors)
        {  
            Init(); 
            Create(parent, id, pos, size, style);
        }
    virtual ~CAuthorReplacePanel() {}
    /// Creates the controls and sizers
    virtual void CreateControls();
    virtual  CRef<CCmdComposite> GetCommand(const vector<CConstRef<CObject>>& objs, const vector<CSeq_entry_Handle>& descr_ctxt);

private:
    CAuthorNamesPanel* m_AuthorsPanel;
    CRef<CAuth_list> m_Authors;
    CEditPubDlg* m_dlg;
};

END_NCBI_SCOPE

#endif
    // _EDIT_PUB_DLG_STD_H_
