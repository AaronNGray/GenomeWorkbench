#ifndef GUI_FRAMEWORK___STATUS_MESSAGE_SLOT_PANEL__HPP
#define GUI_FRAMEWORK___STATUS_MESSAGE_SLOT_PANEL__HPP

/*  $Id: status_message_slot_panel.hpp 31805 2014-11-17 16:56:57Z katargir $
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
 *  CStatusMessageSlotPanel - defines a panel displayed in a popup window
 *  on the Message slot in the Status Bar.
 */

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>


////@begin includes
#include "wx/valgen.h"
#include "wx/hyperlink.h"
////@end includes

#include <wx/panel.h>

////@begin forward declarations
class wxHyperlinkCtrl;
////@end forward declarations

class wxStaticText;

////@begin control identifiers
#define SYMBOL_CSTATUSMESSAGESLOTPANEL_STYLE 0
#define SYMBOL_CSTATUSMESSAGESLOTPANEL_TITLE _("Status Message Slot Panel")
#define SYMBOL_CSTATUSMESSAGESLOTPANEL_IDNAME ID_CSTATUSMESSAGESLOTPANEL
#define SYMBOL_CSTATUSMESSAGESLOTPANEL_SIZE wxSize(300, 100)
#define SYMBOL_CSTATUSMESSAGESLOTPANEL_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

class CStatusMessageSlotPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CStatusMessageSlotPanel )
    DECLARE_EVENT_TABLE()

public:
    CStatusMessageSlotPanel();
    CStatusMessageSlotPanel( wxWindow* parent,
                             wxWindowID id = SYMBOL_CSTATUSMESSAGESLOTPANEL_IDNAME,
                             const wxPoint& pos = SYMBOL_CSTATUSMESSAGESLOTPANEL_POSITION,
                             const wxSize& size = SYMBOL_CSTATUSMESSAGESLOTPANEL_SIZE,
                             long style = SYMBOL_CSTATUSMESSAGESLOTPANEL_STYLE );

    bool Create( wxWindow* parent,
                 wxWindowID id = SYMBOL_CSTATUSMESSAGESLOTPANEL_IDNAME,
                 const wxPoint& pos = SYMBOL_CSTATUSMESSAGESLOTPANEL_POSITION,
                 const wxSize& size = SYMBOL_CSTATUSMESSAGESLOTPANEL_SIZE,
                 long style = SYMBOL_CSTATUSMESSAGESLOTPANEL_STYLE );

    ~CStatusMessageSlotPanel();

    void Init();

    void CreateControls();

    bool TransferDataToWindow();

////@begin CStatusMessageSlotPanel event handler declarations

    void OnExtraLinkHyperlinkClicked( wxHyperlinkEvent& event );

////@end CStatusMessageSlotPanel event handler declarations

////@begin CStatusMessageSlotPanel member function declarations

    wxString GetTitleText() const { return m_Title ; }
    void SetTitleText(wxString value) { m_Title = value ; }

    wxString GetDescrText() const { return m_Descr ; }
    void SetDescrText(wxString value) { m_Descr = value ; }

    wxString GetTimeText() const { return m_Time ; }
    void SetTimeText(wxString value) { m_Time = value ; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CStatusMessageSlotPanel member function declarations

    static bool ShowToolTips();

////@begin CStatusMessageSlotPanel member variables
    wxStaticText* m_TitleStatic;
    wxStaticText* m_TimeStatic;
    wxStaticText* m_DescrStatic;
    wxHyperlinkCtrl* m_ExtraLink;
protected:
    wxString m_Title;
    wxString m_Descr;
    wxString m_Time;
    enum {
        ID_CSTATUSMESSAGESLOTPANEL = 10009,
        ID_EXTRA_LINK = 10017
    };
////@end CStatusMessageSlotPanel member variables

public:
    void    SetExtraLink(const string & label);
    void    WrapText();

protected:
    int m_BestWidth;
};


END_NCBI_SCOPE

/* @} */

#endif // GUI_FRAMEWORK___STATUS_MESSAGE_SLOT_PANEL__HPP
