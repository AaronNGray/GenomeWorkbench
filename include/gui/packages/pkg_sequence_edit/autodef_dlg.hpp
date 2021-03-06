/*  $Id: autodef_dlg.hpp 38632 2017-06-05 17:16:13Z asztalos $
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
// Generated by DialogBlocks (unregistered), 21/06/2013 13:22:54

#ifndef _AUTODEF_DLG_H_
#define _AUTODEF_DLG_H_

#include <corelib/ncbistd.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/packages/pkg_sequence_edit/autodef_org_panel.hpp>
#include <gui/packages/pkg_sequence_edit/autodef_feat_clause_panel.hpp>
#include <gui/packages/pkg_sequence_edit/autodef_params.hpp>

#include <wx/dialog.h>
#include <wx/panel.h>
#include <wx/button.h>

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class CAutodefOrgPanel;
class CAutodefFeatClausePanel;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CAUTODEFDLG 10189
#define ID_AD_WINDOW7 10206
#define ID_AD_WINDOW8 10207
#define ID_CHECKBOX14 10425
#define SYMBOL_CAUTODEFDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CAUTODEFDLG_TITLE _("Automatic Definition Line")
#define SYMBOL_CAUTODEFDLG_IDNAME ID_CAUTODEFDLG
#define SYMBOL_CAUTODEFDLG_SIZE wxSize(400, 300)
#define SYMBOL_CAUTODEFDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CAutodefDlg class declaration
 */

class CAutodefDlg : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CAutodefDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAutodefDlg();
    CAutodefDlg( wxWindow* parent, wxWindowID id = SYMBOL_CAUTODEFDLG_IDNAME, const wxString& caption = SYMBOL_CAUTODEFDLG_TITLE, const wxPoint& pos = SYMBOL_CAUTODEFDLG_POSITION, const wxSize& size = SYMBOL_CAUTODEFDLG_SIZE, long style = SYMBOL_CAUTODEFDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CAUTODEFDLG_IDNAME, const wxString& caption = SYMBOL_CAUTODEFDLG_TITLE, const wxPoint& pos = SYMBOL_CAUTODEFDLG_POSITION, const wxSize& size = SYMBOL_CAUTODEFDLG_SIZE, long style = SYMBOL_CAUTODEFDLG_STYLE );

    /// Destructor
    ~CAutodefDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAutodefDlg event handler declarations

////@end CAutodefDlg event handler declarations

////@begin CAutodefDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAutodefDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAutodefDlg member variables
    CAutodefOrgPanel* m_OrgPanel;
    CAutodefFeatClausePanel* m_FeatClausePanel;
    wxCheckBox* m_Targeted;
////@end CAutodefDlg member variables
    void SetParams(SAutodefParams* params);
    void PopulateParams(SAutodefParams& params);

    void Clear();
    void ShowSourceOptions(bool show);
    bool OnlyTargeted() { if (m_Targeted) return m_Targeted->GetValue(); else return false; };
};

END_NCBI_SCOPE

#endif
    // _AUTODEF_DLG_H_
