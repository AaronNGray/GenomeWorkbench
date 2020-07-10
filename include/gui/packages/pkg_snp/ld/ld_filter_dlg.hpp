/*  $Id: ld_filter_dlg.hpp 23953 2011-06-23 19:29:53Z wuliangs $
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
 * Authors:  Melvin Quintos
 */
#ifndef _LD_FILTER_DLG_H_
#define _LD_FILTER_DLG_H_

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

#include <wx/dialog.h>

////@begin includes
#include "ld_filter_params.hpp"
#include "wx/valgen.h"
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
#define SYMBOL_CLDFILTERDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CLDFILTERDIALOG_TITLE _("LD BLock Filter")
#define SYMBOL_CLDFILTERDIALOG_IDNAME ID_LDBLOCKFILTER
#define SYMBOL_CLDFILTERDIALOG_SIZE wxSize(400, 300)
#define SYMBOL_CLDFILTERDIALOG_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

/*!
 * CLDFilterDialog class declaration
 */

class CLDFilterDialog: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CLDFilterDialog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CLDFilterDialog();
    CLDFilterDialog( wxWindow* parent, wxWindowID id = SYMBOL_CLDFILTERDIALOG_IDNAME, const wxString& caption = SYMBOL_CLDFILTERDIALOG_TITLE, const wxPoint& pos = SYMBOL_CLDFILTERDIALOG_POSITION, const wxSize& size = SYMBOL_CLDFILTERDIALOG_SIZE, long style = SYMBOL_CLDFILTERDIALOG_STYLE );

    /// Creation
    bool CreateX( wxWindow* parent, wxWindowID id = SYMBOL_CLDFILTERDIALOG_IDNAME, const wxString& caption = SYMBOL_CLDFILTERDIALOG_TITLE, const wxPoint& pos = SYMBOL_CLDFILTERDIALOG_POSITION, const wxSize& size = SYMBOL_CLDFILTERDIALOG_SIZE, long style = SYMBOL_CLDFILTERDIALOG_STYLE );

    /// Destructor
    ~CLDFilterDialog();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Transfer data to the window
    virtual bool TransferDataToWindow();
    
////@begin CLDFilterDialog event handler declarations

    /// wxEVT_COMMAND_SLIDER_UPDATED event handler for ID_SLIDER
    void OnScoreSliderUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_SLIDER_UPDATED event handler for ID_SLIDER1
    void OnLengthSliderUpdated( wxCommandEvent& event );

////@end CLDFilterDialog event handler declarations

////@begin CLDFilterDialog member function declarations

    /// Data access
    CLDFilterParams& GetData() { return m_data; }
    const CLDFilterParams& GetData() const { return m_data; }
    void SetData(const CLDFilterParams& data) { m_data = data; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CLDFilterDialog member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CLDFilterDialog member variables
    wxSlider* m_ctrlScore;
    wxStaticText* m_txtScore;
    wxSlider* m_ctrlLength;
    wxStaticText* m_txtLength;
    /// The data edited by this window
    CLDFilterParams m_data;
    /// Control identifiers
    enum {
        ID_LDBLOCKFILTER = 10003,
        ID_SLIDER = 10004,
        ID_SLIDER1 = 10005
    };
////@end CLDFilterDialog member variables
};

END_NCBI_SCOPE

#endif // _LD_FILTER_DLG_H_
