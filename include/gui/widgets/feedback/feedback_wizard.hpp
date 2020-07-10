#ifndef GUI_WIDGETS_FEEDBACK___FEEDBACK_WIZARD__HPP
#define GUI_WIDGETS_FEEDBACK___FEEDBACK_WIZARD__HPP

/*  $Id: feedback_wizard.hpp 39434 2017-09-22 15:41:37Z katargir $
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
 * Authors:  Roman Katargin
 */

#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

/*!
 * Includes
 */

////@begin includes
#include "wx/wizard.h"
#include "wx/valgen.h"
#include "wx/valtext.h"
////@end includes

class wxCheckBox;
class wxTextCtrl;

BEGIN_NCBI_SCOPE

/*!
 * Forward declarations
 */

////@begin forward declarations
class WizardPage;
class WizardPage1;
class WizardPage2;
class WizardPage3;
class WizardPage4;
////@end forward declarations

///////////////////////////////////////////////////////////////////////////////
/// class CFeedbackReport serves as a data model for all pages and coordinates
/// the submission of a report
class NCBI_GUIWIDGETS_FEEDBACK_EXPORT CFeedbackReport : public CObject
{
public:
    void SetEmailAddr(const string& str) { m_EmailAddr = str; }
    const string& GetEmailAddr() const { return m_EmailAddr; }

    void SetDescr(const string& str) { m_Descr = str; }
    const string& GetDescr() const { return m_Descr; }

    void SetSysInfo(const string& str) { m_SysInfo = str; }
    const string& GetSysInfo() const { return m_SysInfo; }

    void SetIncludeLog(bool b) { m_IncludeLog = b; }
    bool GetIncludeLog() const { return m_IncludeLog; }

    void SetSubscribe(bool b) { m_Subscribe = b; }
    bool GetSubscribe() const { return m_Subscribe; }

    void SendFeedback();

    void Format(CNcbiOstream& ostr) const;

private:
    string m_EmailAddr;
    string m_Descr;
    string m_SysInfo;
    bool m_IncludeLog;
    bool m_Subscribe;
};

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CFEEDBACKWIZARD_IDNAME ID_CFEEDBACKWIZARD
////@end control identifiers

/*!
 * CFeedbackWizard class declaration
 */

class NCBI_GUIWIDGETS_FEEDBACK_EXPORT CFeedbackWizard: public wxWizard
{
    DECLARE_DYNAMIC_CLASS( CFeedbackWizard )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CFeedbackWizard();
    CFeedbackWizard( wxWindow* parent, bool opt_out );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CFEEDBACKWIZARD_IDNAME, const wxPoint& pos = wxDefaultPosition );

    /// Destructor
    ~CFeedbackWizard();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CFeedbackWizard event handler declarations

////@end CFeedbackWizard event handler declarations

////@begin CFeedbackWizard member function declarations

    /// Runs the wizard
    bool Run();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CFeedbackWizard member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CFeedbackWizard member variables
    /// Control identifiers
    enum {
        ID_CFEEDBACKWIZARD = 10000
    };
////@end CFeedbackWizard member variables

public:
    CFeedbackReport GetReport() const;

    bool GetOptOut() const { return m_OptOut; }

    void SetProblemDescription(const char* description);

private:
    bool m_OptOut;
};

/*!
 * WizardPage class declaration
 */

class NCBI_GUIWIDGETS_FEEDBACK_EXPORT WizardPage: public wxWizardPageSimple
{
    DECLARE_DYNAMIC_CLASS( WizardPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WizardPage();

    WizardPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Destructor
    ~WizardPage();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin WizardPage event handler declarations

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX4
    void OnOptOutClick( wxCommandEvent& event );

////@end WizardPage event handler declarations

////@begin WizardPage member function declarations

    bool GetOptOut() const { return m_OptOut ; }
    void SetOptOut(bool value) { m_OptOut = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end WizardPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin WizardPage member variables
private:
    bool m_OptOut;
    /// Control identifiers
    enum {
        ID_WIZARDPAGE1 = 10001,
        ID_CHECKBOX4 = 10014
    };
////@end WizardPage member variables
};

/*!
 * WizardPage1 class declaration
 */

class NCBI_GUIWIDGETS_FEEDBACK_EXPORT WizardPage1: public wxWizardPageSimple
{
    DECLARE_DYNAMIC_CLASS( WizardPage1 )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WizardPage1();

    WizardPage1( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Destructor
    ~WizardPage1();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin WizardPage1 event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WIZARDPAGE2
    void OnWizardpagePolicyChanging( wxWizardEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_ACKNOWLEDGE
    void OnAcknowledgePolicy( wxCommandEvent& event );

////@end WizardPage1 event handler declarations

////@begin WizardPage1 member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end WizardPage1 member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin WizardPage1 member variables
    wxCheckBox* m_AcceptPolicy;
    /// Control identifiers
    enum {
        ID_WIZARDPAGE2 = 10002,
        ID_TEXTCTRL1 = 10003,
        ID_ACKNOWLEDGE = 10004
    };
////@end WizardPage1 member variables
};

/*!
 * WizardPage2 class declaration
 */

class NCBI_GUIWIDGETS_FEEDBACK_EXPORT WizardPage2: public wxWizardPageSimple
{
    DECLARE_DYNAMIC_CLASS( WizardPage2 )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WizardPage2();

    WizardPage2( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Destructor
    ~WizardPage2();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin WizardPage2 event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WIZARDPAGE3
    void OnProblemPageChanging( wxWizardEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_EMAILCTRL
    void OnEmailCtrlTextUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX
    void OnSendAnonymously( wxCommandEvent& event );

////@end WizardPage2 event handler declarations

////@begin WizardPage2 member function declarations

    wxString GetEMail() const { return m_EMail ; }
    void SetEMail(wxString value) { m_EMail = value ; }

    bool GetIncludeLog() const { return m_IncludeLog ; }
    void SetIncludeLog(bool value) { m_IncludeLog = value ; }

    wxString GetProblemDescr() const { return m_ProblemDescr ; }
    void SetProblemDescr(wxString value) { m_ProblemDescr = value ; }

    bool GetSendAnonymously() const { return m_SendAnonymously ; }
    void SetSendAnonymously(bool value) { m_SendAnonymously = value ; }

    bool GetSubscribe() const { return m_Subscribe ; }
    void SetSubscribe(bool value) { m_Subscribe = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end WizardPage2 member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin WizardPage2 member variables
    wxTextCtrl* m_EMailCtrl;
    wxCheckBox* m_SendAnonymouslyCtrl;
    wxCheckBox* m_SubChbx;
private:
    wxString m_EMail;
    bool m_IncludeLog;
    wxString m_ProblemDescr;
    bool m_SendAnonymously;
    bool m_Subscribe;
    /// Control identifiers
    enum {
        ID_WIZARDPAGE3 = 10005,
        ID_EMAILCTRL = 10006,
        ID_CHECKBOX = 10027,
        ID_DESCRCTRL = 10007,
        ID_CHECK_SENDLOG = 10008,
        ID_CHECK_SUBSCRIBE = 10019
    };
////@end WizardPage2 member variables
};

/*!
 * WizardPage3 class declaration
 */

class NCBI_GUIWIDGETS_FEEDBACK_EXPORT WizardPage3: public wxWizardPageSimple
{
    DECLARE_DYNAMIC_CLASS( WizardPage3 )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WizardPage3();

    WizardPage3( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Destructor
    ~WizardPage3();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    void OnShow(wxShowEvent& event);

////@begin WizardPage3 event handler declarations

////@end WizardPage3 event handler declarations

////@begin WizardPage3 member function declarations

    bool GetIncludeSysInfo() const { return m_IncludeSysInfo ; }
    void SetIncludeSysInfo(bool value) { m_IncludeSysInfo = value ; }

    wxString GetSysInfo() const { return m_SysInfo ; }
    void SetSysInfo(wxString value) { m_SysInfo = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end WizardPage3 member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin WizardPage3 member variables
private:
    bool m_IncludeSysInfo;
    wxString m_SysInfo;
    /// Control identifiers
    enum {
        ID_WIZARDPAGE4 = 10009,
        ID_CHECKBOX3 = 10010,
        ID_TEXTCTRL4 = 10011
    };
////@end WizardPage3 member variables
};

/*!
 * WizardPage4 class declaration
 */

class NCBI_GUIWIDGETS_FEEDBACK_EXPORT WizardPage4: public wxWizardPageSimple
{
    DECLARE_DYNAMIC_CLASS( WizardPage4 )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WizardPage4();

    WizardPage4( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Destructor
    ~WizardPage4();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin WizardPage4 event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_WIZARDPAGE5
    void OnWizardpageSubmitChanged( wxWizardEvent& event );

////@end WizardPage4 event handler declarations

////@begin WizardPage4 member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end WizardPage4 member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin WizardPage4 member variables
    /// Control identifiers
    enum {
        ID_WIZARDPAGE5 = 10012,
        ID_TEXTCTRL5 = 10013
    };
////@end WizardPage4 member variables
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_FEEDBACK___FEEDBACK_WIZARD__HPP
