#ifndef GUI_WIDGETS_EDIT___SUBMITBLOCKPANEL__HPP
#define GUI_WIDGETS_EDIT___SUBMITBLOCKPANEL__HPP

/*  $Id: submitblockpanel.hpp 42249 2019-01-17 21:38:18Z asztalos $
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

#include <corelib/ncbiobj.hpp>
#include <serial/serialbase.hpp>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <gui/gui_export.h>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CSubmit_block;
END_SCOPE(objects);

class CFlexibleDatePanel;


/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CSUBMITBLOCKPANEL 10077
#define ID_RBTN_SUBPNL1 10082
#define ID_RBTN_SUBPNL2 10084
#define ID_TEXTCTRL10 10086
#define SYMBOL_CSUBMITBLOCKPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSUBMITBLOCKPANEL_TITLE _("SubmitBlockPanel")
#define SYMBOL_CSUBMITBLOCKPANEL_IDNAME ID_CSUBMITBLOCKPANEL
#define SYMBOL_CSUBMITBLOCKPANEL_SIZE wxSize(400, 400)
#define SYMBOL_CSUBMITBLOCKPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSubmitBlockPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CSubmitBlockPanel : public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CSubmitBlockPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSubmitBlockPanel();
    CSubmitBlockPanel( 
        wxWindow* parent, 
        CRef<objects::CSubmit_block> submit_block,
        bool create_submit_block = false,
        wxWindowID id = SYMBOL_CSUBMITBLOCKPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSUBMITBLOCKPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSUBMITBLOCKPANEL_SIZE, 
        long style = SYMBOL_CSUBMITBLOCKPANEL_STYLE );

    /// Creation
    bool Create( 
        wxWindow* parent, 
        wxWindowID id = SYMBOL_CSUBMITBLOCKPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSUBMITBLOCKPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSUBMITBLOCKPANEL_SIZE, 
        long style = SYMBOL_CSUBMITBLOCKPANEL_STYLE );

    /// Destructor
    ~CSubmitBlockPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    void SetData(const objects::CSubmit_block& submit_block);

    ////@begin CSubmitBlockPanel event handler declarations

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RBTN_SUBPNL1
    void OnImmediateReleaseSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RBTN_SUBPNL2
    void OnHUPSelected( wxCommandEvent& event );

////@end CSubmitBlockPanel event handler declarations

////@begin CSubmitBlockPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSubmitBlockPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

private:
////@begin CSubmitBlockPanel member variables
    wxRadioButton* m_ImmediateRelease;
    wxRadioButton* m_HUP;
    wxTextCtrl* m_Title;
////@end CSubmitBlockPanel member variables
    CFlexibleDatePanel* m_Reldate;
    CFlexibleDatePanel* m_Submitdate;
private:
    CRef<objects::CSubmit_block> m_SubBlock;
    bool m_CreateMode{ false };
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___SUBMITBLOCKPANEL__HPP
