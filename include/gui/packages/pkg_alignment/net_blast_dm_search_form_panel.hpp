#ifndef GUI_PKG_ALIGNMENT___NET_BLAST_DM_SEARCH_FORM_PANEL__HPP
#define GUI_PKG_ALIGNMENT___NET_BLAST_DM_SEARCH_FORM_PANEL__HPP

/*  $Id: net_blast_dm_search_form_panel.hpp 25478 2012-03-27 14:54:26Z kuznets $
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

#include <corelib/ncbistd.hpp>


////@begin includes
#include "wx/valgen.h"
////@end includes


////@begin forward declarations
////@end forward declarations


////@begin control identifiers
#define SYMBOL_CNETBLASTDMSEARCHFORMPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CNETBLASTDMSEARCHFORMPANEL_TITLE wxT("Dialog")
#define SYMBOL_CNETBLASTDMSEARCHFORMPANEL_IDNAME ID_CNETBLASTDMSEARCHFORM
#define SYMBOL_CNETBLASTDMSEARCHFORMPANEL_SIZE wxDefaultSize
#define SYMBOL_CNETBLASTDMSEARCHFORMPANEL_POSITION wxDefaultPosition
////@end control identifiers

#include <wx/panel.h>

class wxButton;
class wxTextCtrl;


BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

class CNetBlastDMSearchFormPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CNetBlastDMSearchFormPanel )
    DECLARE_EVENT_TABLE()

public:
    CNetBlastDMSearchFormPanel();
    CNetBlastDMSearchFormPanel( wxWindow* parent, wxWindowID id = SYMBOL_CNETBLASTDMSEARCHFORMPANEL_IDNAME, const wxPoint& pos = SYMBOL_CNETBLASTDMSEARCHFORMPANEL_POSITION, const wxSize& size = SYMBOL_CNETBLASTDMSEARCHFORMPANEL_SIZE, long style = SYMBOL_CNETBLASTDMSEARCHFORMPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CNETBLASTDMSEARCHFORMPANEL_IDNAME, const wxPoint& pos = SYMBOL_CNETBLASTDMSEARCHFORMPANEL_POSITION, const wxSize& size = SYMBOL_CNETBLASTDMSEARCHFORMPANEL_SIZE, long style = SYMBOL_CNETBLASTDMSEARCHFORMPANEL_STYLE );

    ~CNetBlastDMSearchFormPanel();

    void Init();

    void CreateControls();

    bool TransferDataToWindow();

////@begin CNetBlastDMSearchFormPanel event handler declarations

    void OnFilterInputTextUpdated( wxCommandEvent& event );

    void OnResetClick( wxCommandEvent& event );

    void OnSubmittedCheckClick( wxCommandEvent& event );

////@end CNetBlastDMSearchFormPanel event handler declarations

////@begin CNetBlastDMSearchFormPanel member function declarations

    bool GetCompleted() const { return m_Completed ; }
    void SetCompleted(bool value) { m_Completed = value ; }

    bool GetSubmitted() const { return m_Submitted ; }
    void SetSubmitted(bool value) { m_Submitted = value ; }

    bool GetFailed() const { return m_Failed ; }
    void SetFailed(bool value) { m_Failed = value ; }

    bool GetRetrieved() const { return m_Retrieved ; }
    void SetRetrieved(bool value) { m_Retrieved = value ; }

    bool GetExpired() const { return m_Expired ; }
    void SetExpired(bool value) { m_Expired = value ; }

    wxString GetSearchValue() const { return m_SearchValue ; }
    void SetSearchValue(wxString value) { m_SearchValue = value ; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CNetBlastDMSearchFormPanel member function declarations

    static bool ShowToolTips();

////@begin CNetBlastDMSearchFormPanel member variables
    wxTextCtrl* m_SearchCtrl;
    wxButton* m_ResetBtn;

public:
    void RestartSearch();

protected:
    bool m_Completed;
    bool m_Submitted;
    bool m_Failed;
    bool m_Retrieved;
    bool m_Expired;
    wxString m_SearchValue;
    bool m_BlockEvents;
    enum {
        ID_CNETBLASTDMSEARCHFORM = 10023,
        ID_FILTER_INPUT = 10025,
        ID_SUBMITTED_CHECK = 10027,
        ID_COMPLETED_CHECK = 10028,
        ID_FAILED_CHECK = 10029,
        ID_RETRIEVED_CHECK = 10030,
        ID_EXPIRED_CHECK = 10031
    };
////@end CNetBlastDMSearchFormPanel member variables
};

/* @} */

END_NCBI_SCOPE

#endif // GUI_PKG_ALIGNMENT___NET_BLAST_DM_SEARCH_FORM_PANEL__HPP

