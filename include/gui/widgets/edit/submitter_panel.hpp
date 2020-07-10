#ifndef GUI_PKG_SEQ_EDIT___SUBMITTER_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___SUBMITTER_PANEL__HPP

/*  $Id: submitter_panel.hpp 43206 2019-05-28 21:41:23Z asztalos $
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
#include <gui/gui_export.h>
#include <gui/widgets/edit/submission_page_interface.hpp>
#include <wx/panel.h>
#include <wx/bookctrl.h>

class wxListbook;

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CContact_info;
END_SCOPE(objects);

class CSubmitterNamePanel;
class CSubmitterAffilPanel;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSUBMITTERPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSUBMITTERPANEL_TITLE _("Submitter Panel")
#define SYMBOL_CSUBMITTERPANEL_IDNAME ID_CSUBMITTERPANEL
#define SYMBOL_CSUBMITTERPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSUBMITTERPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSubmitterPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CSubmitterPanel: public wxPanel, public ISubmissionPage
{    
    DECLARE_DYNAMIC_CLASS(CSubmitterPanel)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSubmitterPanel();
    CSubmitterPanel(
        wxWindow* parent,
        objects::CContact_info& contact_info,
        wxWindowID id = SYMBOL_CSUBMITTERPANEL_IDNAME,
        const wxPoint& pos = SYMBOL_CSUBMITTERPANEL_POSITION,
        const wxSize& size = SYMBOL_CSUBMITTERPANEL_SIZE,
        long style = SYMBOL_CSUBMITTERPANEL_STYLE);

    /// Creation
    bool Create( 
        wxWindow* parent, 
        wxWindowID id = SYMBOL_CSUBMITTERPANEL_IDNAME,
        const wxPoint& pos = SYMBOL_CSUBMITTERPANEL_POSITION,
        const wxSize& size = SYMBOL_CSUBMITTERPANEL_SIZE,
        long style = SYMBOL_CSUBMITTERPANEL_STYLE);

    /// Destructor
    ~CSubmitterPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    //virtual bool TransferDataFromWindow();

////@begin CSubmitterPanel event handler declarations
    void OnPageChanged(wxBookCtrlEvent& event);
    void OnPageChanging(wxBookCtrlEvent& event);
////@end CSubmitterPanel event handler declarations

////@begin CSubmitterPanel member function declarations

    virtual bool StepForward();
    virtual bool StepBackward();
    virtual bool IsFirstPage() const;

    virtual void ApplySubmitBlock(objects::CSubmit_block& block);
    virtual void ApplyCommand();
    virtual void ReportMissingFields(string &text);
    virtual wxString GetAnchor();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSubmitterPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    enum {
        ID_CSUBMITTERPANEL = 6500,
        ID_CSUBMITTERNOTEBOOK
    };
private:
    wxListbook* m_Listbook;
    CSubmitterNamePanel* m_NamePanel;
    CSubmitterAffilPanel* m_AffilPanel;
    CRef<objects::CContact_info> m_Contact;
};

END_NCBI_SCOPE

#endif  // GUI_PKG_SEQ_EDIT___SUBMITTER_PANEL__HPP
