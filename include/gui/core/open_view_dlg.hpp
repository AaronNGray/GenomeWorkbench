#ifndef GUI_CORE___OPEN_VIEW_DLG__HPP
#define GUI_CORE___OPEN_VIEW_DLG__HPP

/*  $Id: open_view_dlg.hpp 39737 2017-10-31 17:03:07Z katargir $
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

#include <gui/gui_export.h>

#include <gui/widgets/wx/dialog.hpp>
#include <gui/core/open_view_manager_impl.hpp>
#include <gui/core/project_view_factory.hpp>

#include <wx/listbase.h>

////@begin includes
#include "wx/statline.h"
#include "wx/hyperlink.h"
////@end includes

class wxButton;

/*!
 * Control identifiers
 */
#define ID_ONLY 10050

////@begin control identifiers
#define SYMBOL_COPENVIEWDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxMAXIMIZE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_COPENVIEWDLG_TITLE _("Open View")
#define SYMBOL_COPENVIEWDLG_IDNAME ID_COPENVIEWDLG
#define SYMBOL_COPENVIEWDLG_SIZE wxDefaultSize
#define SYMBOL_COPENVIEWDLG_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

class CItemSelectionPanel;


class NCBI_GUICORE_EXPORT COpenViewDlg: public CDialog
{
    DECLARE_DYNAMIC_CLASS( COpenViewDlg )
    DECLARE_EVENT_TABLE()

public:
    COpenViewDlg();
    COpenViewDlg( wxWindow* parent, wxWindowID id = SYMBOL_COPENVIEWDLG_IDNAME, const wxString& caption = SYMBOL_COPENVIEWDLG_TITLE, const wxPoint& pos = SYMBOL_COPENVIEWDLG_POSITION, const wxSize& size = SYMBOL_COPENVIEWDLG_SIZE, long style = SYMBOL_COPENVIEWDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_COPENVIEWDLG_IDNAME, const wxString& caption = SYMBOL_COPENVIEWDLG_TITLE, const wxPoint& pos = SYMBOL_COPENVIEWDLG_POSITION, const wxSize& size = SYMBOL_COPENVIEWDLG_SIZE, long style = SYMBOL_COPENVIEWDLG_STYLE );

    ~COpenViewDlg();

    void Init();

    void CreateControls();

////@begin COpenViewDlg event handler declarations

    void OnInitDialog( wxInitDialogEvent& event );

    void OnBackwardClick( wxCommandEvent& event );

    void OnForwardClick( wxCommandEvent& event );

////@end COpenViewDlg event handler declarations

////@begin COpenViewDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end COpenViewDlg member function declarations

    static bool ShowToolTips();

////@begin COpenViewDlg member variables
    wxPanel* m_Panel;
    wxButton* m_BackBtn;
    wxButton* m_NextBtn;
    enum {
        ID_COPENVIEWDLG = 10029,
        ID_OPENVIEWPANEL = 10033,
        ID_HELPLINK = 10050
    };
////@end COpenViewDlg member variables

    typedef CIRef<IProjectViewFactory>  TFactoryRef;
    typedef vector<TFactoryRef>  TFactories;
    typedef CIRef<IOpenViewManager> TManagerRef;

    void    SetFactories(TFactories& factories);
    void    SetInputObjects(const TConstScopedObjects& input_objects);
    const   TConstScopedObjects& GetOutputObjects() const
    {
        return m_OutputObjects;
    }
    const   vector<int> GetOutputIndices() const
    {
        return m_OutputIndices;
    }

    void    OnOnlyClick( wxCommandEvent& event );
    void    OnViewSelected(wxCommandEvent& event);
    void    OnListItemActivate(wxListEvent &event);
    void    OnSelectAll( wxCommandEvent &anEvt );

    TFactoryRef GetSelectedFactory();

    virtual void    SetRegistryPath(const string& path);

    static string GetDefaultFactoryLabel(const string& reg_path, TConstScopedObjects& objects);

public:
    // overriding CDialog::EndModal()
    virtual void EndModal(int ret_code);

protected:
    virtual void x_LoadSettings(const CRegistryReadView& view);
    virtual void x_SaveSettings(CRegistryWriteView view) const;

    void    x_UpdateViewPanel();
    void    x_SetCurrentPanel(wxPanel* panel, const wxString& caption = SYMBOL_COPENVIEWDLG_TITLE);
    void    x_UpdateButtons();
    void    x_DoTransition(IOpenViewManager::EAction action);

protected:
    CItemSelectionPanel* m_ItemPanel;
    wxPanel*    m_ViewListPanel;
    wxPanel*    m_CurrPanel;


    TFactories  m_Factories;
    TFactories  m_CompatibleFactories;
    bool        m_CompatibleEmpty; // m_CompatibleFactories has not been built yet

    TFactoryRef m_CurrFactory;
    TManagerRef m_CurrManager;

    CRef<COpenViewManager> m_DefaultManager;

    /// The original input objects that serve as input arguments
    /// for object converter.
    TConstScopedObjects m_InputObjects;

    /// The output objects after view-specific object conversion 
    /// and possible object aggregation. These objects will be fed 
    /// into views according to flag mf_OutputCombineRequested.
    TConstScopedObjects m_OutputObjects;
    vector<int> m_OutputIndices;

    bool    m_CompatibleOnly;
    string  m_InputCategory;
};


END_NCBI_SCOPE


#endif
    // GUI_CORE___OPEN_VIEW_DLG__HPP
