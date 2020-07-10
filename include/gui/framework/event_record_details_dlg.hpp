#ifndef GUI_FRAMEWORK___EVENT_RECORD_DETAILS_DLG__HPP
#define GUI_FRAMEWORK___EVENT_RECORD_DETAILS_DLG__HPP

/*  $Id: event_record_details_dlg.hpp 31805 2014-11-17 16:56:57Z katargir $
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

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>
#include <gui/framework/event_log_service.hpp>

#include <wx/dialog.h>

////@begin includes
#include "wx/html/htmlwin.h"
////@end includes

////@begin forward declarations
class wxHtmlWindow;
////@end forward declarations


////@begin control identifiers
#define SYMBOL_CEVENTRECORDDETAILSDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CEVENTRECORDDETAILSDLG_TITLE wxT("Event Details")
#define SYMBOL_CEVENTRECORDDETAILSDLG_IDNAME ID_CEVENTRECORDDETAILS
#define SYMBOL_CEVENTRECORDDETAILSDLG_SIZE wxSize(400, 300)
#define SYMBOL_CEVENTRECORDDETAILSDLG_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CEventRecordDetails

class NCBI_GUIFRAMEWORK_EXPORT CEventRecordDetailsDlg :
    public wxDialog
{
    DECLARE_DYNAMIC_CLASS(CEventRecordDetailsDlg)
    DECLARE_EVENT_TABLE()

public:
    CEventRecordDetailsDlg();
    CEventRecordDetailsDlg( wxWindow* parent, wxWindowID id = SYMBOL_CEVENTRECORDDETAILSDLG_IDNAME, const wxString& caption = SYMBOL_CEVENTRECORDDETAILSDLG_TITLE, const wxPoint& pos = SYMBOL_CEVENTRECORDDETAILSDLG_POSITION, const wxSize& size = SYMBOL_CEVENTRECORDDETAILSDLG_SIZE, long style = SYMBOL_CEVENTRECORDDETAILSDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CEVENTRECORDDETAILSDLG_IDNAME, const wxString& caption = SYMBOL_CEVENTRECORDDETAILSDLG_TITLE, const wxPoint& pos = SYMBOL_CEVENTRECORDDETAILSDLG_POSITION, const wxSize& size = SYMBOL_CEVENTRECORDDETAILSDLG_SIZE, long style = SYMBOL_CEVENTRECORDDETAILSDLG_STYLE );

    ~CEventRecordDetailsDlg();

    void Init();

    void CreateControls();

    void    SetRecord(const IEventRecord& record);

////@begin CEventRecordDetailsDlg event handler declarations

////@end CEventRecordDetailsDlg event handler declarations

////@begin CEventRecordDetailsDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CEventRecordDetailsDlg member function declarations

    static bool ShowToolTips();

protected:
////@begin CEventRecordDetailsDlg member variables
    wxHtmlWindow* m_HTMLWindow;
    enum {
        ID_CEVENTRECORDDETAILS = 10000,
        ID_HTMLWINDOW1 = 10001
    };
////@end CEventRecordDetailsDlg member variables

    CConstIRef<IEventRecord> m_Record;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___EVENT_RECORD_DETAILS_DLG__HPP
