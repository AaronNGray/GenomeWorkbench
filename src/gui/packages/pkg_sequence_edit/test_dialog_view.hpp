#ifndef PKG_SEQUENCE_EDIT___TEST_DIALOG_VIEW__HPP
#define PKG_SEQUENCE_EDIT___TEST_DIALOG_VIEW__HPP

/*  $Id: test_dialog_view.hpp 30382 2014-05-08 19:02:17Z katargir $
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
 * Authors:  Roman Katargin
 */

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

#include <gui/widgets/wx/gui_widget_dlg.hpp>
#include <gui/widgets/text_widget/text_widget_host.hpp>

#include <wx/dialog.h>

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CTESTDIALOGVIEW 10356
#define ID_PANEL1 10362
#define SYMBOL_CTESTDIALOGVIEW_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CTESTDIALOGVIEW_TITLE _("Test Dialog View")
#define SYMBOL_CTESTDIALOGVIEW_IDNAME ID_CTESTDIALOGVIEW
#define SYMBOL_CTESTDIALOGVIEW_SIZE wxSize(400, 300)
#define SYMBOL_CTESTDIALOGVIEW_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

class CTextPanel;

/*!
 * CTestDialogView class declaration
 */

class CTestDialogView: public CGuiWidgetDlg
    , public ITextWidgetHost
{    
    DECLARE_DYNAMIC_CLASS( CTestDialogView )
    DECLARE_EVENT_TABLE()

public:
    static wxWindow* CreateDialog(wxWindow* parent);

    /// Constructors
    CTestDialogView();
    CTestDialogView( wxWindow* parent, wxWindowID id = SYMBOL_CTESTDIALOGVIEW_IDNAME, const wxString& caption = SYMBOL_CTESTDIALOGVIEW_TITLE, const wxPoint& pos = SYMBOL_CTESTDIALOGVIEW_POSITION, const wxSize& size = SYMBOL_CTESTDIALOGVIEW_SIZE, long style = SYMBOL_CTESTDIALOGVIEW_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CTESTDIALOGVIEW_IDNAME, const wxString& caption = SYMBOL_CTESTDIALOGVIEW_TITLE, const wxPoint& pos = SYMBOL_CTESTDIALOGVIEW_POSITION, const wxSize& size = SYMBOL_CTESTDIALOGVIEW_SIZE, long style = SYMBOL_CTESTDIALOGVIEW_STYLE );

    /// Destructor
    ~CTestDialogView();

    /// @name ITextWidgetHost implementation
    /// @{
    virtual void WidgetSelectionChanged();

    virtual void OnTextGotFocus() {}
    virtual void OnTextLostFocus() {}
    virtual void OnTextPositionChanged(int, int) {}
    /// @}


    /// @name IGuiWidget implementation
    /// @{
    virtual void SetHost(IGuiWidgetHost* host);
    virtual bool InitWidget(TConstScopedObjects& objects);
    virtual const CObject* GetOrigObject() const;

    // Selection
    virtual void GetSelectedObjects (TConstObjects& objects) const;
    virtual void SetSelectedObjects (const TConstObjects& objects);

    // Data
    virtual void SetUndoManager(ICommandProccessor* cmdProccessor);
    virtual void DataChanging();
    virtual void DataChanged();

    // Objects for action
    virtual void GetActiveObjects(vector<TConstScopedObjects>& objects);
    /// @}

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CTestDialogView event handler declarations

////@end CTestDialogView event handler declarations

////@begin CTestDialogView member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CTestDialogView member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CTestDialogView member variables
    CTextPanel* m_TextPanel;
////@end CTestDialogView member variables
};

END_NCBI_SCOPE

#endif // PKG_SEQUENCE_EDIT___TEST_DIALOG_VIEW__HPP
