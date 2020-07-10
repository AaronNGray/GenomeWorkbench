#ifndef GUI_PKG_SEQ_EDIT___SUBMITTER_NAME_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___SUBMITTER_NAME_PANEL__HPP

/*  $Id: submitter_name_panel.hpp 43203 2019-05-28 18:33:53Z filippov $
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
 * Authors:  Andrea Asztalos
 */

#include <corelib/ncbiobj.hpp>
#include <gui/widgets/edit/submission_page_interface.hpp>
#include <wx/panel.h>

class wxTextCtrl;
class wxStaticText;

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CContact_info;
END_SCOPE(objects);

class CSingleAuthorPanel;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSUBMITTERNAMEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSUBMITTERNAMEPANEL_TITLE _("Submitter Name Panel")
#define SYMBOL_CSUBMITTERNAMEPANEL_IDNAME ID_CSUBMITTERNAMEPANEL
#define SYMBOL_CSUBMITTERNAMEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSUBMITTERNAMEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSubmitterNamePanel class declaration
 */

class CSubmitterNamePanel: public wxPanel, public ISubmissionPage
{    
    DECLARE_DYNAMIC_CLASS(CSubmitterNamePanel)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSubmitterNamePanel();
    CSubmitterNamePanel(
        wxWindow* parent,
        objects::CContact_info& contact_info,
        wxWindowID id = SYMBOL_CSUBMITTERNAMEPANEL_IDNAME,
        const wxPoint& pos = SYMBOL_CSUBMITTERNAMEPANEL_POSITION,
        const wxSize& size = SYMBOL_CSUBMITTERNAMEPANEL_SIZE,
        long style = SYMBOL_CSUBMITTERNAMEPANEL_STYLE);

    /// Creation
    bool Create( 
        wxWindow* parent, 
        wxWindowID id = SYMBOL_CSUBMITTERNAMEPANEL_IDNAME,
        const wxPoint& pos = SYMBOL_CSUBMITTERNAMEPANEL_POSITION,
        const wxSize& size = SYMBOL_CSUBMITTERNAMEPANEL_SIZE,
        long style = SYMBOL_CSUBMITTERNAMEPANEL_STYLE);

    /// Destructor
    ~CSubmitterNamePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    void ApplyContactInfo(objects::CContact_info& contact_info);
    virtual void ApplyCommand();
    virtual void ReportMissingFields(string &text);
    virtual wxString GetAnchor() {return _("submitter-name");}

////@begin CSubmitterNamePanel event handler declarations

    void OnPrimeEmailChanged( wxCommandEvent& event );
////@end CSubmitterNamePanel event handler declarations

////@begin CSubmitterNamePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSubmitterNamePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    enum {
        ID_CSUBMITTERNAMEPANEL = 6510,
        ID_SUBMITTERFIRSTNAME,
        ID_SUBMITTERMIDDLENAME,
        ID_SUBMITTERLASTNAME,
        ID_SUBMITTER_PRIMEMAIL,
        ID_SUBMITTER_SECEMAIL
    };
private:
    CSingleAuthorPanel* m_SingleAuthor;
    wxTextCtrl* m_EmailPrimary;
    wxTextCtrl* m_EmailSecondary;
    wxStaticText* m_LabelSecondary;
    CRef< objects::CContact_info> m_Contact;
};

END_NCBI_SCOPE

#endif  // GUI_PKG_SEQ_EDIT___SUBMITTER_NAME_PANEL__HPP
