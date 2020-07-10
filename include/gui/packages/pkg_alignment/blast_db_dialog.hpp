#ifndef PKG_ALIGNMENT___BLAST_DB_DIALOG__HPP
#define PKG_ALIGNMENT___BLAST_DB_DIALOG__HPP

/*  $Id: blast_db_dialog.hpp 40033 2017-12-12 20:06:39Z katargir $
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
 *      A dialog for browsing BLAST Databases.
 */

#include <corelib/ncbistd.hpp>

#include <gui/widgets/wx/dialog.hpp>
#include <gui/objutils/blast_databases.hpp>

#include <objects/blast/Blast4_get_databases_reply.hpp>
#include <objects/blast/Blast4_database_info.hpp>

#include <gui/widgets/wx/async_call.hpp>

////@begin includes
#include "wx/treectrl.h"
////@end includes

#include <wx/dialog.h>

class wxTextCtrl;
class wxStaticText;
class wxTreeCtrl;
class wxButton;

////@begin control identifiers
#define SYMBOL_CBLAST_DB_DIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CBLAST_DB_DIALOG_TITLE wxEmptyString
#define SYMBOL_CBLAST_DB_DIALOG_IDNAME ID_CBLAST_DB_DIALOG
#define SYMBOL_CBLAST_DB_DIALOG_SIZE wxSize(400, 300)
#define SYMBOL_CBLAST_DB_DIALOG_POSITION wxDefaultPosition
////@end control identifiers

#include <deque>

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

class CBLAST_Dlg_Item : public CObject
{
public:
    typedef map<wxString, CRef<CBLAST_Dlg_Item> > TNameToItemMap;

    CBLAST_Dlg_Item();
    CBLAST_Dlg_Item(const string& label, const string& path,
                    bool is_database = false);
    ~CBLAST_Dlg_Item();

    wxString GetLabel() const;
    const string&   GetPath() const    {   return m_Path; }

    bool IsDatabase() const { return m_IsDatabase; }

    bool    IsVisible() const { return m_Visible;   }
    void    SetVisible(bool visible)    {   m_Visible = visible;    }

    bool    IsExpanded() const {    return m_Expanded;  }
    void    Expand(bool expand) {   m_Expanded = expand;    }

    CBLAST_Dlg_Item*  GetParent() {   return m_Parent;    }

    TNameToItemMap*  GetChildItems() {   return m_ChildItems;    }
    CBLAST_Dlg_Item*    GetChildByLabel(const string& label);

    void    AddChild(CBLAST_Dlg_Item& item);

    const wxTreeItemId& GetId() const   {   return m_Id;    }
    void    SetId(const wxTreeItemId& id)   {   m_Id = id;  }

protected:
    size_t m_LabelPos;
    size_t m_LabelLength;
//    string  m_Label; // short label
    string  m_Path; // full path - categories separated by "/" + label

    bool m_IsDatabase;
    bool    m_Visible;
    bool    m_Expanded;

    CBLAST_Dlg_Item*    m_Parent;
    TNameToItemMap*     m_ChildItems;
    wxTreeItemId        m_Id;
};


///////////////////////////////////////////////////////////////////////////////
/// CBLAST_DB_Dialog
class CBLAST_DB_Dialog: public CDialog
{
    DECLARE_DYNAMIC_CLASS( CBLAST_DB_Dialog )
    DECLARE_EVENT_TABLE()

public:
    CBLAST_DB_Dialog();
    CBLAST_DB_Dialog( wxWindow* parent, wxWindowID id = SYMBOL_CBLAST_DB_DIALOG_IDNAME, const wxString& caption = SYMBOL_CBLAST_DB_DIALOG_TITLE, const wxPoint& pos = SYMBOL_CBLAST_DB_DIALOG_POSITION, const wxSize& size = SYMBOL_CBLAST_DB_DIALOG_SIZE, long style = SYMBOL_CBLAST_DB_DIALOG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CBLAST_DB_DIALOG_IDNAME, const wxString& caption = SYMBOL_CBLAST_DB_DIALOG_TITLE, const wxPoint& pos = SYMBOL_CBLAST_DB_DIALOG_POSITION, const wxSize& size = SYMBOL_CBLAST_DB_DIALOG_SIZE, long style = SYMBOL_CBLAST_DB_DIALOG_STYLE );

    ~CBLAST_DB_Dialog();

    void Init();

    void CreateControls();

////@begin CBLAST_DB_Dialog event handler declarations

    void OnInitDialog( wxInitDialogEvent& event );

    void OnSearchTextUpdated( wxCommandEvent& event );

    void OnResetBtnClick( wxCommandEvent& event );

    void OnTreectrlSelChanged( wxTreeEvent& event );

    void OnTreectrlItemActivated( wxTreeEvent& event );

    void OnTreectrlItemCollapsed( wxTreeEvent& event );

    void OnTreectrlItemExpanded( wxTreeEvent& event );

    void OnOkClick( wxCommandEvent& event );

////@end CBLAST_DB_Dialog event handler declarations

////@begin CBLAST_DB_Dialog member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CBLAST_DB_Dialog member function declarations

    static bool ShowToolTips();

////@begin CBLAST_DB_Dialog member variables
    wxTextCtrl* m_SearchCtrl;
    wxStaticText* m_StatusText;
    wxTreeCtrl* m_TreeCtrl;
    wxButton* m_OKBtn;
    enum {
        ID_CBLAST_DB_DIALOG = 10012,
        ID_SEARCH = 10015,
        ID_RESET_BTN = 10016,
        wxID_STATUS = 10017,
        ID_TREECTRL = 10014
    };
////@end CBLAST_DB_Dialog member variables

public:
    typedef CBLASTDatabases::TDbMap TDbMap;

    void    SetToolName(const string& tool);
    void    SetDBMap(const TDbMap& map); //stores a refererence to the given DB
    void    SelectDatabases(vector<string>& databases);

    void    GetSelectedDatabases(vector<string>& databases);

protected:

    void    x_CreateItems();
    void    x_BuildTreeItems(CBLAST_Dlg_Item& item);
    void    x_CreateTreeItems();
    void    x_FilterItems(const string& query);
    void    x_UpdateCollapsedState(wxTreeItemId id);
    void    x_UpdateFilterStatusText(bool reset, int n);
    CBLAST_Dlg_Item*    x_GetSelectedDBItem();

    void    x_OnDatabaseSelected();

protected:
    typedef deque<CBLAST_Dlg_Item*> TItemDeque;

    const TDbMap*   m_DbMap;

    CRef<CBLAST_Dlg_Item>   m_Root;

    // helper lists to simplify filtering
    TItemDeque  m_CatItems; // all category items
    TItemDeque  m_DbItems;  // all database items

    string  m_SelDatabase;
};


/* @} */

END_NCBI_SCOPE

#endif // PKG_ALIGNMENT___BLAST_DB_DIALOG__HPP

