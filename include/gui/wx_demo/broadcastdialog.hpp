/*  $Id: broadcastdialog.hpp 25635 2012-04-13 20:53:49Z katargir $
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

#ifndef _BROADCASTDIALOG_H_
#define _BROADCASTDIALOG_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "broadcastdialog.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/valgen.h"
#include "wx/statline.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CBROADCASTDIALOG 10001
#define ID_CHOICE1 10022
#define ID_RADIOBOX1 10032
#define ID_CHECKBOX1 10018
#define ID_CHECKBOX2 10019
#define ID_CHECKBOX3 10024
#define ID_RADIOBOX2 10017
#define ID_CHECKBOX5 10026
#define ID_CHECKBOX4 10025
#define ID_STATICLINE2 10028
#define SYMBOL_CBROADCASTDIALOG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CBROADCASTDIALOG_TITLE _("Broadcast Options")
#define SYMBOL_CBROADCASTDIALOG_IDNAME ID_CBROADCASTDIALOG
#define SYMBOL_CBROADCASTDIALOG_SIZE wxSize(400, 300)
#define SYMBOL_CBROADCASTDIALOG_POSITION wxDefaultPosition
////@end control identifiers

struct SBroadcastDlgParams
{
    SBroadcastDlgParams() :
        m_BioseqMatching(),
        m_SeqIDMatching(),
        m_MatchAlnLocs(true),
        m_MatchByProduct(true),
        m_BroadcastBetweenProjects(true),
        m_AutoSelBroadcast(true),
        m_AutoVRBroadcast(true),
        m_VRPolicy() {}

    int m_BioseqMatching;
    int m_SeqIDMatching;
    bool m_MatchAlnLocs;
    bool m_MatchByProduct;
    bool m_BroadcastBetweenProjects;
    bool m_AutoSelBroadcast;
    bool m_AutoVRBroadcast;
    int m_VRPolicy;
};

/*!
 * CBroadcastDialog class declaration
 */

class CBroadcastDialog: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CBroadcastDialog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CBroadcastDialog();
    CBroadcastDialog( const SBroadcastDlgParams& params,
                      wxWindow* parent, wxWindowID id = SYMBOL_CBROADCASTDIALOG_IDNAME, const wxString& caption = SYMBOL_CBROADCASTDIALOG_TITLE, const wxPoint& pos = SYMBOL_CBROADCASTDIALOG_POSITION, const wxSize& size = SYMBOL_CBROADCASTDIALOG_SIZE, long style = SYMBOL_CBROADCASTDIALOG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CBROADCASTDIALOG_IDNAME, const wxString& caption = SYMBOL_CBROADCASTDIALOG_TITLE, const wxPoint& pos = SYMBOL_CBROADCASTDIALOG_POSITION, const wxSize& size = SYMBOL_CBROADCASTDIALOG_SIZE, long style = SYMBOL_CBROADCASTDIALOG_STYLE );

    /// Destructor
    ~CBroadcastDialog();

    /// Initialises member variables
    void Init( const SBroadcastDlgParams& params );

    /// Creates the controls and sizers
    void CreateControls();

    /// REturns dialog results
    SBroadcastDlgParams GetData() const;

////@begin CBroadcastDialog event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_APPLY
    void OnApplyClick( wxCommandEvent& event );

////@end CBroadcastDialog event handler declarations

////@begin CBroadcastDialog member function declarations

    int GetVRPolicy() const { return m_VRPolicy ; }
    void SetVRPolicy(int value) { m_VRPolicy = value ; }

    bool GetMatchAlnLocs() const { return m_MatchAlnLocs ; }
    void SetMatchAlnLocs(bool value) { m_MatchAlnLocs = value ; }

    bool GetMatchByProduct() const { return m_MatchByProduct ; }
    void SetMatchByProduct(bool value) { m_MatchByProduct = value ; }

    bool GetAutoSelBroadcast() const { return m_AutoSelBroadcast ; }
    void SetAutoSelBroadcast(bool value) { m_AutoSelBroadcast = value ; }

    bool GetAutoVRBroadcast() const { return m_AutoVRBroadcast ; }
    void SetAutoVRBroadcast(bool value) { m_AutoVRBroadcast = value ; }

    bool GetBroadcastBetweenProjects() const { return m_BroadcastBetweenProjects ; }
    void SetBroadcastBetweenProjects(bool value) { m_BroadcastBetweenProjects = value ; }

    int GetMatchRadios() const { return m_MatchRadios ; }
    void SetMatchRadios(int value) { m_MatchRadios = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CBroadcastDialog member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CBroadcastDialog member variables
private:
    int m_VRPolicy;
    bool m_MatchAlnLocs;
    bool m_MatchByProduct;
    bool m_AutoSelBroadcast;
    bool m_AutoVRBroadcast;
    bool m_BroadcastBetweenProjects;
    int m_MatchRadios;
////@end CBroadcastDialog member variables
};

#endif
    // _BROADCASTDIALOG_H_
