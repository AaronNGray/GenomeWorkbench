#ifndef GUI_FRAMEWORK___TASK_DETAILS__HPP
#define GUI_FRAMEWORK___TASK_DETAILS__HPP

/*  $Id: task_details_dlg.hpp 22112 2010-09-22 21:14:41Z falkrb $
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

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <gui/gui_export.h>

#include <corelib/ncbistl.hpp>

#include <gui/widgets/wx/dialog.hpp>

////@begin includes
#include "wx/valtext.h"
////@end includes

////@begin forward declarations
////@end forward declarations

////@begin control identifiers
#define SYMBOL_CTASKDETAILSDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CTASKDETAILSDLG_TITLE _("Task Details")
#define SYMBOL_CTASKDETAILSDLG_IDNAME ID_CTASKDETAILSDLG
#define SYMBOL_CTASKDETAILSDLG_SIZE wxSize(400, 300)
#define SYMBOL_CTASKDETAILSDLG_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

class CTaskDetailsDlg: public CDialog
{
    DECLARE_DYNAMIC_CLASS( CTaskDetailsDlg )
    DECLARE_EVENT_TABLE()

public:
    CTaskDetailsDlg();
    CTaskDetailsDlg( wxWindow* parent,
                     wxWindowID id = SYMBOL_CTASKDETAILSDLG_IDNAME,
                     const wxString& caption = SYMBOL_CTASKDETAILSDLG_TITLE,
                     const wxPoint& pos = SYMBOL_CTASKDETAILSDLG_POSITION,
                     const wxSize& size = SYMBOL_CTASKDETAILSDLG_SIZE,
                     long style = SYMBOL_CTASKDETAILSDLG_STYLE );

    bool Create( wxWindow* parent,
                 wxWindowID id = SYMBOL_CTASKDETAILSDLG_IDNAME,
                 const wxString& caption = SYMBOL_CTASKDETAILSDLG_TITLE,
                 const wxPoint& pos = SYMBOL_CTASKDETAILSDLG_POSITION,
                 const wxSize& size = SYMBOL_CTASKDETAILSDLG_SIZE,
                 long style = SYMBOL_CTASKDETAILSDLG_STYLE );

    ~CTaskDetailsDlg();

    void Init();

    void CreateControls();

////@begin CTaskDetailsDlg event handler declarations

    void OnCancelTaskClick( wxCommandEvent& event );

////@end CTaskDetailsDlg event handler declarations

////@begin CTaskDetailsDlg member function declarations

    wxString GetTaskTitle() const { return m_Title ; }
    void SetTaskTitle(wxString value) { m_Title = value ; }

    wxString GetStatus() const { return m_Status ; }
    void SetStatus(wxString value) { m_Status = value ; }

    wxString GetState() const { return m_State ; }
    void SetState(wxString value) { m_State = value ; }

    wxString GetTime() const { return m_Time ; }
    void SetTime(wxString value) { m_Time = value ; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CTaskDetailsDlg member function declarations

    static bool ShowToolTips();

////@begin CTaskDetailsDlg member variables
protected:
    wxString m_Title;
    wxString m_Status;
    wxString m_State;
    wxString m_Time;
    enum {
        ID_CTASKDETAILSDLG = 10011,
        ID_TEXTCTRL = 10012,
        ID_TEXTCTRL3 = 10014,
        ID_TEXTCTRL2 = 10013,
        ID_TEXTCTRL4 = 10015,
        ID_CANCEL_TASK = 10016
    };
////@end CTaskDetailsDlg member variables
};


END_NCBI_SCOPE

/* @} */

#endif // _TASK_DETAILS_DLG_H_
