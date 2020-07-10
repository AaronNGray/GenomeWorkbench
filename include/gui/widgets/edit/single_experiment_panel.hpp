/*  $Id: single_experiment_panel.hpp 37154 2016-12-08 19:21:13Z asztalos $
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
#ifndef _SINGLE_EXPERIMENT_PANEL_H_
#define _SINGLE_EXPERIMENT_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include "formattedqual_panel.hpp"

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSINGLEEXPERIMENTPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSINGLEEXPERIMENTPANEL_TITLE _("SingleExperimentPanel")
#define SYMBOL_CSINGLEEXPERIMENTPANEL_IDNAME ID_CSINGLEEXPERIMENTPANEL
#define SYMBOL_CSINGLEEXPERIMENTPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSINGLEEXPERIMENTPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSingleExperimentPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CSingleExperimentPanel : public CFormattedQualPanel
{    
    DECLARE_DYNAMIC_CLASS( CSingleExperimentPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSingleExperimentPanel();
    CSingleExperimentPanel( wxWindow* parent, wxWindowID id = SYMBOL_CSINGLEEXPERIMENTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSINGLEEXPERIMENTPANEL_POSITION, const wxSize& size = SYMBOL_CSINGLEEXPERIMENTPANEL_SIZE, long style = SYMBOL_CSINGLEEXPERIMENTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSINGLEEXPERIMENTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSINGLEEXPERIMENTPANEL_POSITION, const wxSize& size = SYMBOL_CSINGLEEXPERIMENTPANEL_SIZE, long style = SYMBOL_CSINGLEEXPERIMENTPANEL_STYLE );

    /// Destructor
    ~CSingleExperimentPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual void SetValue(string val);
    virtual string GetValue();
    virtual void GetDimensions(int *width, int *height);

////@begin CSingleExperimentPanel event handler declarations

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_EXPERIMENT_TXT
    void OnExperimentTxtTextUpdated( wxCommandEvent& event );

////@end CSingleExperimentPanel event handler declarations

////@begin CSingleExperimentPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSingleExperimentPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSingleExperimentPanel member variables
    wxChoice* m_Category;
    wxTextCtrl* m_Experiment;
    wxTextCtrl* m_PMID;
    /// Control identifiers
    enum {
        ID_CSINGLEEXPERIMENTPANEL = 10179,
        ID_CHOICE18 = 10180,
        ID_EXPERIMENT_TXT = 10181,
        ID_TEXTCTRL2 = 10182
    };
////@end CSingleExperimentPanel member variables
};

END_NCBI_SCOPE

#endif
    // _SINGLE_EXPERIMENT_PANEL_H_
