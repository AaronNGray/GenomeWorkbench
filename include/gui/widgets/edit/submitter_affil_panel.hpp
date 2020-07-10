#ifndef GUI_PKG_SEQ_EDIT___SUBMITTER_AFFIL_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___SUBMITTER_AFFIL_PANEL__HPP

/*  $Id: submitter_affil_panel.hpp 43209 2019-05-29 14:01:26Z bollin $
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
class wxComboBox;

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CSubmit_block;
    class CAuth_list;
END_SCOPE(objects);

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSUBMITTERAFFILPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSUBMITTERAFFILPANEL_TITLE _("Submitter Affiliation Panel")
#define SYMBOL_CSUBMITTERAFFILPANEL_IDNAME ID_SUBMITTERAFFILPANEL
#define SYMBOL_CSUBMITTERAFFILPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSUBMITTERAFFILPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSubmitterAffilPanel class declaration
 */

class CSubmitterAffilPanel: public wxPanel, public ISubmissionPage
{    
    DECLARE_DYNAMIC_CLASS( CSubmitterAffilPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSubmitterAffilPanel();
    CSubmitterAffilPanel( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSUBMITTERAFFILPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSUBMITTERAFFILPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSUBMITTERAFFILPANEL_SIZE, 
        long style = SYMBOL_CSUBMITTERAFFILPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSUBMITTERAFFILPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSUBMITTERAFFILPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSUBMITTERAFFILPANEL_SIZE, 
        long style = SYMBOL_CSUBMITTERAFFILPANEL_STYLE );

    /// Destructor
    ~CSubmitterAffilPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

////@begin CSubmitterAffilPanel event handler declarations

    /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_SUB_AFFIL_COUNTRY
    void OnAffilCountrySelected( wxCommandEvent& event );

////@end CSubmitterAffilPanel event handler declarations

////@begin CSubmitterAffilPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSubmitterAffilPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void ApplySubmitBlock(objects::CSubmit_block& block);
    virtual void ApplyCommand();
    virtual void ReportMissingFields(string &text);
    virtual wxString GetAnchor() { return _("submitter-affiliation"); }


    /// Control identifiers
    enum {
        ID_SUBMITTERAFFILPANEL = 6520,
        ID_SUB_AFFIL_INST = 6521,
        ID_SUB_AFFIL_DEPT = 6522,
        ID_SUB_AFFIL_ADDRESS = 6523,
        ID_SUB_AFFIL_CITY = 6524,
        ID_SUB_AFFIL_STATE = 6525,
        ID_SUB_AFFIL_CODE = 6526,
        ID_SUB_AFFIL_COUNTRY = 6527,
        ID_SUB_AFFIL_PHONE
    };
private:

////@begin CSubmitterAffilPanel member variables
    wxTextCtrl* m_AffilCtrl;
    wxTextCtrl* m_DepartmentCtrl;
    wxTextCtrl* m_StreetCtrl;
    wxTextCtrl* m_CityCtrl;
    wxTextCtrl* m_SubCtrl;
    wxTextCtrl* m_ZipCtrl;
    wxComboBox* m_CountryCtrl;
    wxTextCtrl* m_PhoneCtrl;

////@end CSubmitterAffilPanel member variables

    CRef<objects::CSubmit_block> m_SubmitBlock;
    void x_Reset();
    void x_CopyAffil(objects::CAuth_list &auth_list);
};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___SUBMITTER_AFFIL_PANEL__HPP
