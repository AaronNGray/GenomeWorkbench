/*  $Id: validate_frame.hpp 43609 2019-08-08 16:12:53Z filippov $
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
 * Authors:  Igor Filippov
 *
 */

#ifndef _VALIDATE_FRAME_H_
#define _VALIDATE_FRAME_H_

#include <corelib/ncbistd.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/core/selection_client.hpp>
#include <gui/utils/event_handler.hpp>
#include <gui/utils/job_adapter.hpp>
#include <objects/valerr/ValidError.hpp>
#include <objects/valerr/ValidErrItem.hpp>
#include <objects/general/User_object.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <gui/widgets/seq/flat_file_ctrl.hpp>
#include <gui/widgets/edit/table_data_validate.hpp>
#include <gui/widgets/edit/table_data_validate_params.hpp>

/*!
 * Includes
 */

////@begin includes
#include "wx/frame.h"
#include <wx/listctrl.h>
#include <wx/stattext.h>
////@end includes


BEGIN_NCBI_SCOPE


/*!
 * Forward declarations
 */

////@begin forward declarations
class CValidatePanel;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_VALIDATE_FRAME 17000
#define ID_VALIDATE_FRAME_REFRESH 17004
#define ID_VALIDATE_FRAME_REPORT_BTN 17006
#define ID_VALIDATE_FRAME_DISMISS 17007
#define ID_VALIDATE_FRAME_SEQUESTER_BTN 17008
#define ID_VALIDATE_FRAME_SEGREGATE_BTN 17009
#define ID_VALIDATE_FRAME_DONE_BTN 17010
#define SYMBOL_VALIDATE_FRAME_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxMINIMIZE_BOX|wxMAXIMIZE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_VALIDATE_FRAME_TITLE _("Validation Errors")
#define SYMBOL_VALIDATE_FRAME_IDNAME ID_VALIDATE_FRAME
#define SYMBOL_VALIDATE_FRAME_SIZE wxSize(900, 600)
#define SYMBOL_VALIDATE_FRAME_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CValidateFrame class declaration
 */

class CValidateFrame : public wxFrame
{    
    DECLARE_CLASS( CValidateFrame )
    DECLARE_EVENT_TABLE()


public:
    static void GetInstance( wxWindow* parent, IWorkbench* workbench, CRef<objects::CUser_object> params, wxWindowID id = SYMBOL_VALIDATE_FRAME_IDNAME, const wxString& caption = SYMBOL_VALIDATE_FRAME_TITLE, const wxPoint& pos = SYMBOL_VALIDATE_FRAME_POSITION, const wxSize& size = SYMBOL_VALIDATE_FRAME_SIZE, long style = SYMBOL_VALIDATE_FRAME_STYLE );

    /// Constructors
    CValidateFrame();
    CValidateFrame( wxWindow* parent, IWorkbench* wb, CRef<objects::CUser_object> params, wxWindowID id = SYMBOL_VALIDATE_FRAME_IDNAME, const wxString& caption = SYMBOL_VALIDATE_FRAME_TITLE, const wxPoint& pos = SYMBOL_VALIDATE_FRAME_POSITION, const wxSize& size = SYMBOL_VALIDATE_FRAME_SIZE, long style = SYMBOL_VALIDATE_FRAME_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_VALIDATE_FRAME_IDNAME, const wxString& caption = SYMBOL_VALIDATE_FRAME_TITLE, const wxPoint& pos = SYMBOL_VALIDATE_FRAME_POSITION, const wxSize& size = SYMBOL_VALIDATE_FRAME_SIZE, long style = SYMBOL_VALIDATE_FRAME_STYLE );

    /// Destructor
    virtual ~CValidateFrame();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();


////@begin CValidateFrame event handler declarations
    void OnRefresh(wxCommandEvent& event );
    void OnDismiss(wxCommandEvent& event );
    void OnSequester( wxCommandEvent& event );
    void OnSegregate( wxCommandEvent& event );
    void OnReport( wxCommandEvent& event );
    void OnDone( wxCommandEvent& event );
    void OnUpdateDoneBtn(wxUpdateUIEvent& event);
////@end CValidateFrame event handler declarations


////@begin CValidateFrame member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CValidateFrame member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CValidateFrame member variables
////@end CValidateFrame member variables

private:
    virtual void SetRegistryPath(const string& reg_path);  
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    bool x_AllowDoneButton();
    void LoadData(void);

    IWorkbench*     m_Workbench;
    CRef<objects::CUser_object> m_params;
    CValidatePanel* m_Panel;
    static CValidateFrame *m_Instance;
    string m_RegPath;
    wxButton *m_DoneButton;
};


END_NCBI_SCOPE

#endif
    // _VALIDATE_FRAME_H_
