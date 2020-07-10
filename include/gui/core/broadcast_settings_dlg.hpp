/*  $Id: broadcast_settings_dlg.hpp 33354 2015-07-09 21:06:35Z evgeniev $
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
 * Authors:  Yury Voronov
 *
 * File Description:
 *
 */

#ifndef _BROADCAST_SETTINGS_DLG_H_
#define _BROADCAST_SETTINGS_DLG_H_

#include <corelib/ncbistd.hpp>
/*!
 * Includes
 */

#include <wx/dialog.h>

////@begin includes
#include "wx/notebook.h"
#include "wx/valgen.h"
#include "wx/hyperlink.h"
#include "wx/statline.h"
////@end includes

#include <wx/editlbox.h>
#include <wx/checkbox.h>
#include <gui/gui_export.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxEditableListBox;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CBROADCASTDIALOG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CBROADCASTDIALOG_TITLE _("Broadcast Options")
#define SYMBOL_CBROADCASTDIALOG_IDNAME ID_CBROADCASTDIALOG
#define SYMBOL_CBROADCASTDIALOG_SIZE wxSize(400, 300)
#define SYMBOL_CBROADCASTDIALOG_POSITION wxDefaultPosition
////@end control identifiers

struct SBroadcastParams
{
    SBroadcastParams() :
        m_BioseqMatching(),
        m_SeqIDMatching(),
        m_MatchAlnLocs(true),
        m_MatchByProduct(true),
        m_BroadcastBetweenProjects(true),
        m_AutoSelBroadcast(true),
        m_AutoVRBroadcast(true),
        m_VRBPolicy(),
        m_TreeBroadcastOneToOne(true)
        {
        }

    int m_BioseqMatching;
    int m_SeqIDMatching;
    bool m_MatchAlnLocs;
    bool m_MatchByProduct;
    bool m_BroadcastBetweenProjects;
    bool m_AutoSelBroadcast;
    bool m_AutoVRBroadcast;
    int m_VRBPolicy;
    /// node properties used to compare trees when broadcasting
    vector<string> m_TreeBroadcastProperties;

    /// if true, when views of the same tree are opened, we broadcast between
    /// trees simply based on the node id - we use one-to-one matching. If false,
    /// one-to-one matching may not occur - selection will be defined by broadcast
    /// properties
    bool m_TreeBroadcastOneToOne;
};

/*!
 * CBroadcastDialog class declaration
 */

class NCBI_GUICORE_EXPORT CBroadcastDialog: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CBroadcastDialog )
    DECLARE_EVENT_TABLE()

public:
    CBroadcastDialog();
    CBroadcastDialog( wxWindow* parent, wxWindowID id = SYMBOL_CBROADCASTDIALOG_IDNAME, const wxString& caption = SYMBOL_CBROADCASTDIALOG_TITLE, const wxPoint& pos = SYMBOL_CBROADCASTDIALOG_POSITION, const wxSize& size = SYMBOL_CBROADCASTDIALOG_SIZE, long style = SYMBOL_CBROADCASTDIALOG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CBROADCASTDIALOG_IDNAME, const wxString& caption = SYMBOL_CBROADCASTDIALOG_TITLE, const wxPoint& pos = SYMBOL_CBROADCASTDIALOG_POSITION, const wxSize& size = SYMBOL_CBROADCASTDIALOG_SIZE, long style = SYMBOL_CBROADCASTDIALOG_STYLE );

    ~CBroadcastDialog();

    void Init();

    void CreateControls();

    void SetData( const SBroadcastParams& params );
    SBroadcastParams GetData() const;

////@begin CBroadcastDialog event handler declarations

    void OnApplyClick( wxCommandEvent& event );

    void OnSetDefaultsBtnClick( wxCommandEvent& event );

////@end CBroadcastDialog event handler declarations

////@begin CBroadcastDialog member function declarations

    bool GetAutoSelBroadcast() const { return m_AutoSelBroadcast ; }
    void SetAutoSelBroadcast(bool value) { m_AutoSelBroadcast = value ; }

    bool GetAutoVRBroadcast() const { return m_AutoVRBroadcast ; }
    void SetAutoVRBroadcast(bool value) { m_AutoVRBroadcast = value ; }

    bool GetBroadcastBetweenProjects() const { return m_BroadcastBetweenProjects ; }
    void SetBroadcastBetweenProjects(bool value) { m_BroadcastBetweenProjects = value ; }

    bool GetMatchAlnLocs() const { return m_MatchAlnLocs ; }
    void SetMatchAlnLocs(bool value) { m_MatchAlnLocs = value ; }

    bool GetMatchByProduct() const { return m_MatchByProduct ; }
    void SetMatchByProduct(bool value) { m_MatchByProduct = value ; }

    int GetMatchRadios() const { return m_MatchRadios ; }
    void SetMatchRadios(int value) { m_MatchRadios = value ; }

    int GetPolicy() const { return m_Policy ; }
    void SetPolicy(int value) { m_Policy = value ; }

    int GetVRBPolicy() const { return m_VRBPolicy ; }
    void SetVRBPolicy(int value) { m_VRBPolicy = value ; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CBroadcastDialog member function declarations

    static bool ShowToolTips();

////@begin CBroadcastDialog member variables
    wxEditableListBox* m_BroadcastList;
    wxCheckBox* m_SelectOneToOne;
private:
    bool m_AutoSelBroadcast;
    bool m_AutoVRBroadcast;
    bool m_BroadcastBetweenProjects;
    bool m_MatchAlnLocs;
    bool m_MatchByProduct;
    int m_MatchRadios;
    int m_Policy;
    int m_VRBPolicy;
    enum {
        ID_CBROADCASTDIALOG = 10001,
        ID_NOTEBOOK = 10000,
        ID_GENERAL_PANEL = 10002,
        ID_POLICY_CHOICE = 10020,
        ID_SEQUENCE_MATCHING_RADIOBOX = 10021,
        ID_USE_LOCATIONS_CHECKBOX = 10022,
        ID_MATCH_BY_PRODUCTS_CHECKBOX = 10023,
        ID_PROJECTS_BROADCAST_CHECKBOX = 10024,
        ID_AUTO_BROADCAST_SEL_CHECKBOX = 10026,
        ID_VISIBLE_RANGE_RADIOBOX = 10027,
        ID_AUTO_BROADCAST_RANGE_CHECKBOX = 10028,
        ID_BIOTREE_CONTAINER_PANEL = 10003,
        ID_BROADCAST_LISTBOX = 10030,
        ID_ONE_TO_ONE_SELECT_CHECKBOX = 10031,
        ID_SET_DEFAULTS_BTN = 10032,
        ID_HYPERLINKCTRL = 10033
    };
////@end CBroadcastDialog member variables
};

END_NCBI_SCOPE

#endif // _BROADCAST_SETTINGS_DLG_H_
