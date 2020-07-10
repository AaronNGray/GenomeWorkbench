#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SEQUENCE_GOTO_DLG__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SEQUENCE_GOTO_DLG__HPP

/*  $Id: configure_tracks_dlg.hpp 39740 2017-10-31 19:49:07Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

#include <gui/widgets/wx/dialog.hpp>
#include <wx/imaglist.h>

#include "track_list_ctrl.hpp"


////@begin includes
#include "wx/splitter.h"
#include "wx/treectrl.h"
#include "wx/srchctrl.h"
#include "wx/listctrl.h"
#include "wx/statline.h"
////@end includes

#include <wx/treelist.h>

/*!
 * Forward declarations
 */

class wxStaticText;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CCONFIGURETRACKSDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CCONFIGURETRACKSDLG_TITLE _("Configure Tracks")
#define SYMBOL_CCONFIGURETRACKSDLG_IDNAME ID_CCONFIGURETRACKSDLG
#define SYMBOL_CCONFIGURETRACKSDLG_SIZE wxSize(400, 300)
#define SYMBOL_CCONFIGURETRACKSDLG_POSITION wxDefaultPosition
////@end control identifiers

class CFeaturePanel;

/*!
 * CConfigureTracksDlg class declaration
 */

class CConfigureTracksDlg: public CDialog
{    
    DECLARE_DYNAMIC_CLASS( CConfigureTracksDlg )
    DECLARE_EVENT_TABLE()

public:

    CConfigureTracksDlg();
    CConfigureTracksDlg(wxWindow* parent,
                        CFeaturePanel* featurePanel,
                        wxWindowID id = SYMBOL_CCONFIGURETRACKSDLG_IDNAME, const wxString& caption = SYMBOL_CCONFIGURETRACKSDLG_TITLE, const wxPoint& pos = SYMBOL_CCONFIGURETRACKSDLG_POSITION, const wxSize& size = SYMBOL_CCONFIGURETRACKSDLG_SIZE, long style = SYMBOL_CCONFIGURETRACKSDLG_STYLE);

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCONFIGURETRACKSDLG_IDNAME, const wxString& caption = SYMBOL_CCONFIGURETRACKSDLG_TITLE, const wxPoint& pos = SYMBOL_CCONFIGURETRACKSDLG_POSITION, const wxSize& size = SYMBOL_CCONFIGURETRACKSDLG_SIZE, long style = SYMBOL_CCONFIGURETRACKSDLG_STYLE );

    ~CConfigureTracksDlg();

    void Init();

    void CreateControls();

    void SetCategory(const string& category);

////@begin CConfigureTracksDlg event handler declarations

    void OnTreectrlSelChanged( wxTreeEvent& event );

    void OnSearchButtonClick( wxCommandEvent& event );

    void OnSearchCancelClick( wxCommandEvent& event );

    void OnSearchEnter( wxCommandEvent& event );

    void OnSearchCtrlUpdate( wxUpdateUIEvent& event );

    void OnOkClick( wxCommandEvent& event );

////@end CConfigureTracksDlg event handler declarations

////@begin CConfigureTracksDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CConfigureTracksDlg member function declarations

    static bool ShowToolTips();

////@begin CConfigureTracksDlg member variables
    wxSplitterWindow* m_Splitter;
    wxTreeCtrl* m_TreeCtrl;
    wxSearchCtrl* m_SearchCtrl;
    wxStaticText* m_TrackCountCtrl;
    CTrackListCtrl* m_TrackListCtrl;
    enum {
        ID_CCONFIGURETRACKSDLG = 10022,
        ID_SPLITTERWINDOW = 10005,
        ID_TREECTRL = 10006,
        ID_PANEL1 = 10012,
        ID_SEARCHCTRL = 10023,
        ID_LISTCTRL = 10015
    };
////@end CConfigureTracksDlg member variables

protected:
    virtual void    x_LoadSettings(const CRegistryReadView& view);
    virtual void    x_SaveSettings(CRegistryWriteView view) const;

private:
    void x_UpdateTrackCount();
    void x_OnSearch(const wxString& value);

    wxImageList m_ImageList;

    CFeaturePanel* m_FeaturePanel;
    CTrackListCtrl::TTracks m_AllTracks;

    string m_CurrentFilter;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___SEQUENCE_GOTO_DLG__HPP
