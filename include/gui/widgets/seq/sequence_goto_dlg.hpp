
#ifndef GUI_WIDGETS_SEQ___SEQUENCE_GOTO_DLG__HPP
#define GUI_WIDGETS_SEQ___SEQUENCE_GOTO_DLG__HPP

/*  $Id: sequence_goto_dlg.hpp 30858 2014-07-31 14:05:43Z ucko $
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

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

/*!
 * Includes
 */


// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/dialog.h>

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

class wxTextCtrl;

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSEQUENCEGOTODLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CSEQUENCEGOTODLG_TITLE _("Go to sequence")
#define SYMBOL_CSEQUENCEGOTODLG_IDNAME ID_CSEQUENCEGOTODLG
#define SYMBOL_CSEQUENCEGOTODLG_SIZE wxSize(400, 300)
#define SYMBOL_CSEQUENCEGOTODLG_POSITION wxDefaultPosition
////@end control identifiers

class CSequenceGotoData;
class CSequenceGotoEvent;

/*!
 * CSequenceGotoDlg class declaration
 */

class NCBI_GUIWIDGETS_SEQ_EXPORT CSequenceGotoDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CSequenceGotoDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSequenceGotoDlg();
    CSequenceGotoDlg( wxWindow* parent,
                      CSequenceGotoData *data,
                      wxEvtHandler* eventHandler = 0,
                      wxWindowID id = SYMBOL_CSEQUENCEGOTODLG_IDNAME,
                      const wxString& caption = SYMBOL_CSEQUENCEGOTODLG_TITLE,
                      const wxPoint& pos = SYMBOL_CSEQUENCEGOTODLG_POSITION,
                      const wxSize& size = SYMBOL_CSEQUENCEGOTODLG_SIZE,
                      long style = SYMBOL_CSEQUENCEGOTODLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSEQUENCEGOTODLG_IDNAME, const wxString& caption = SYMBOL_CSEQUENCEGOTODLG_TITLE, const wxPoint& pos = SYMBOL_CSEQUENCEGOTODLG_POSITION, const wxSize& size = SYMBOL_CSEQUENCEGOTODLG_SIZE, long style = SYMBOL_CSEQUENCEGOTODLG_STYLE );

    // implementation only, don't use
    void Send(CSequenceGotoEvent& event);

    /// Destructor
    ~CSequenceGotoDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSequenceGotoDlg event handler declarations

    /// wxEVT_CLOSE_WINDOW event handler for ID_CSEQUENCEGOTODLG
    void OnCloseWindow( wxCloseEvent& event );

    /// wxEVT_COMMAND_TEXT_ENTER event handler for ID_TEXTCTRL1
    void OnTextctrl1Enter( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOkClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
    void OnCancelClick( wxCommandEvent& event );

////@end CSequenceGotoDlg event handler declarations

////@begin CSequenceGotoDlg member function declarations

    wxString GetSeqPos() const { return m_SeqPos ; }
    void SetSeqPos(wxString value) { m_SeqPos = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSequenceGotoDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void SetFocusToInputBox();
    virtual bool Show(bool show = true);

////@begin CSequenceGotoDlg member variables
    wxTextCtrl* m_InputBox;
private:
    wxString m_SeqPos;
    /// Control identifiers
    enum {
        ID_CSEQUENCEGOTODLG = 10002,
        ID_TEXTCTRL1 = 10003
    };
////@end CSequenceGotoDlg member variables
    CSequenceGotoData* m_Data;
    wxEvtHandler* m_EventHandler;
};


class NCBI_GUIWIDGETS_SEQ_EXPORT CSequenceGotoData: public wxObject
{
public:
    const wxString& GetPositionString() { return m_PositionStr; }
    bool GetRange(long& from, long& to) const;

private:
    friend class CSequenceGotoDlg;

    void SetPositionString(const wxString& str) { m_PositionStr = str; }
    wxString m_PositionStr;
};



class CSequenceGotoEvent : public wxCommandEvent
{
public:
    CSequenceGotoEvent(wxEventType commandType = wxEVT_NULL, int id = 0)
        : wxCommandEvent(commandType, id) { }

    wxString GetPositionString() const { return m_PositionStr; }

    CSequenceGotoDlg* GetDialog() const
        { return wxStaticCast(GetEventObject(), CSequenceGotoDlg); }

private:
    wxString m_PositionStr;

    DECLARE_DYNAMIC_CLASS_NO_COPY(CSequenceGotoEvent)

    // implementation only
    void SetPositionString(const wxString& str) { m_PositionStr = str; }

    friend class CSequenceGotoDlg;
};


BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EXPORTED_EVENT_TYPE(NCBI_GUIWIDGETS_SEQ_EXPORT, wxEVT_COMMAND_SEQ_GOTO, 510)
    DECLARE_EXPORTED_EVENT_TYPE(NCBI_GUIWIDGETS_SEQ_EXPORT, wxEVT_COMMAND_SEQ_GOTO_CLOSE, 511)
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*CSequenceGotoEventFunction)(CSequenceGotoEvent&);

#define CSequenceGotoEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(CSequenceGotoEventFunction, &func)

#define EVT_SEQ_GOTO(id, fn) \
    wx__DECLARE_EVT1(wxEVT_COMMAND_SEQ_GOTO, id, CSequenceGotoEventHandler(fn))

#define EVT_SEQ_GOTO_CLOSE(id, fn) \
    wx__DECLARE_EVT1(wxEVT_COMMAND_SEQ_GOTO_CLOSE, id, CSequenceGotoEventHandler(fn))


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___SEQUENCE_GOTO_DLG__HPP
