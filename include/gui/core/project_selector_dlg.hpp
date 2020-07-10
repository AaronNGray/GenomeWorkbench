#ifndef GUI_CORE___PROJECT_SELECTOR_DLG__HPP
#define GUI_CORE___PROJECT_SELECTOR_DLG__HPP

/*  $Id: project_selector_dlg.hpp 39737 2017-10-31 17:03:07Z katargir $
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

#include <corelib/ncbistl.hpp>

#include <gui/widgets/wx/dialog.hpp>

#include <gui/core/project_selector_panel.hpp>


////@begin includes
#include "wx/listctrl.h"
////@end includes


////@begin forward declarations
class wxBoxSizer;
class CObjectListWidget;
class CProjectSelectorPanel;
////@end forward declarations

////@begin control identifiers
#define SYMBOL_CPROJECTSELECTORDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CPROJECTSELECTORDLG_TITLE _("Project Selector Dialog")
#define SYMBOL_CPROJECTSELECTORDLG_IDNAME ID_CPROJECTSELECTORDLG
#define SYMBOL_CPROJECTSELECTORDLG_SIZE wxDefaultSize
#define SYMBOL_CPROJECTSELECTORDLG_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

class CObjectListWidget;

class CProjectSelectorDlg: public CDialog
{
    DECLARE_DYNAMIC_CLASS( CProjectSelectorDlg )
    DECLARE_EVENT_TABLE()

public:
    CProjectSelectorDlg();
    CProjectSelectorDlg( wxWindow* parent, wxWindowID id = SYMBOL_CPROJECTSELECTORDLG_IDNAME, const wxString& caption = SYMBOL_CPROJECTSELECTORDLG_TITLE, const wxPoint& pos = SYMBOL_CPROJECTSELECTORDLG_POSITION, const wxSize& size = SYMBOL_CPROJECTSELECTORDLG_SIZE, long style = SYMBOL_CPROJECTSELECTORDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CPROJECTSELECTORDLG_IDNAME, const wxString& caption = SYMBOL_CPROJECTSELECTORDLG_TITLE, const wxPoint& pos = SYMBOL_CPROJECTSELECTORDLG_POSITION, const wxSize& size = SYMBOL_CPROJECTSELECTORDLG_SIZE, long style = SYMBOL_CPROJECTSELECTORDLG_STYLE );

    ~CProjectSelectorDlg();

    void Init();

    void CreateControls();

    bool TransferDataToWindow();

    bool TransferDataFromWindow();

////@begin CProjectSelectorDlg event handler declarations

////@end CProjectSelectorDlg event handler declarations

////@begin CProjectSelectorDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CProjectSelectorDlg member function declarations

    static bool ShowToolTips();

////@begin CProjectSelectorDlg member variables
    wxBoxSizer* m_Sizer;
    CObjectListWidget* m_List;
    CProjectSelectorPanel* m_ProjectSelPanel;
    enum {
        ID_CPROJECTSELECTORDLG = 10011,
        ID_COBJECTLISTWIDGET = 10000,
        ID_PANEL1 = 10013
    };
////@end CProjectSelectorDlg member variables

public:
    void    SetProjectService(CProjectService* prj_srv);
    void    SetObjects(TConstScopedObjects& objects);

    //void    SetParams(const SProjectSelectorParams& params);
    void    GetParams(SProjectSelectorParams& params) const;

protected:
    virtual void    x_LoadSettings(const CRegistryReadView& view);
    virtual void    x_SaveSettings(CRegistryWriteView view) const;

protected:
    CProjectService*     m_ProjectService;
    TConstScopedObjects* m_Objects;
    SProjectSelectorParams  m_Params;    
    
};


END_NCBI_SCOPE

#endif
    // GUI_CORE___PROJECT_SELECTOR_DLG__HPP
