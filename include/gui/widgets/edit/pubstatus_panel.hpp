#ifndef GUI_PKG_SEQ_EDIT___PUBSTATUS_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___PUBSTATUS_PANEL__HPP
/*  $Id: pubstatus_panel.hpp 43228 2019-05-30 17:19:13Z bollin $
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

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/submission_page_interface.hpp>
#include <wx/panel.h>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <gui/widgets/edit/authornames_panel.hpp>
#include <gui/utils/command_processor.hpp>
#include <objmgr/seq_entry_handle.hpp>

class wxFlexGridSizer;
class wxSimplebook;


BEGIN_NCBI_SCOPE

class CPubAuthorPanel;
class CUnpublishedRefPanel;
class CPublishedRefPanel;
class CReferencePanel;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CPUBSTATUSPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CPUBSTATUSPANEL_TITLE _("Reference Pub Status Panel")
#define SYMBOL_CPUBSTATUSPANEL_IDNAME ID_CPUBSTATUSPANEL
#define SYMBOL_CPUBSTATUSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CPUBSTATUSPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CPubStatusPanel class declaration
 */

class CPubStatusPanel: public wxPanel, public ISubmissionPage
{    
    DECLARE_DYNAMIC_CLASS( CPubStatusPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CPubStatusPanel();
    CPubStatusPanel( wxWindow* parent, 
                     ICommandProccessor* cmdproc,
                     objects::CSeq_entry_Handle seh,
                     wxWindowID id = SYMBOL_CPUBSTATUSPANEL_IDNAME, 
                     const wxPoint& pos = SYMBOL_CPUBSTATUSPANEL_POSITION, 
                     const wxSize& size = SYMBOL_CPUBSTATUSPANEL_SIZE, 
                     long style = SYMBOL_CPUBSTATUSPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CPUBSTATUSPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CPUBSTATUSPANEL_POSITION, 
        const wxSize& size = SYMBOL_CPUBSTATUSPANEL_SIZE, 
        long style = SYMBOL_CPUBSTATUSPANEL_STYLE );

    /// Destructor
    ~CPubStatusPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CPubStatusPanel event handler declarations

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_UNPUBLISHEDBTN
    void OnUnpublishedSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_INPRESSBTN
    void OnInpressSelected(wxCommandEvent& event);

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_PUBLISHEDBTN
    void OnPublishedSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_REFSEQAUTHORSBTN
    void OnSameAuthorsSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_NEWAUTHORSBTN
    void OnNewAuthorsSelected( wxCommandEvent& event );

////@end CPubStatusPanel event handler declarations

////@begin CPubStatusPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CPubStatusPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    void ApplyCitSub(objects::CCit_sub& sub);
    void ApplyPub(objects::CPubdesc& pubdesc);

    virtual void SetSeqEntryHandle(objects::CSeq_entry_Handle seh) { m_Seh = seh;}
    virtual void ApplyCommand();
    virtual void ReportMissingFields(string &text);
    virtual wxString GetAnchor() { return _("reference-publication"); }


private:
    CReferencePanel* x_GetReferencePanel();

    CAuthorNamesPanel* m_AuthorsPanel;
////@begin CPubStatusPanel member variables
//    CPubAuthorPanel* m_AuthorPanel;
    wxSimplebook* m_Notebook;
    CUnpublishedRefPanel* m_UnpublishedRef;
    CPublishedRefPanel* m_InpressRef;
    CPublishedRefPanel* m_PublishedRef;
public:
    /// Control identifiers
    enum {
        ID_CPUBSTATUSPANEL = 6652,
        ID_UNPUBLISHEDBTN = 6653,
        ID_INPRESSBTN = 6654,
        ID_PUBLISHEDBTN = 6655,
        ID_REFSEQAUTHORSBTN = 6656,
        ID_NEWAUTHORSBTN = 6657
    };
////@end CPubStatusPanel member variables

private:
    ICommandProccessor* m_CmdProcessor;
    objects::CSeq_entry_Handle m_Seh;
    CRef<objects::CCit_sub> m_Sub;
    CRef<objects::CPubdesc> m_Pubdesc;
    wxRadioButton* m_Unpublished;
    wxRadioButton* m_InPress;
    wxRadioButton* m_Published;
    wxRadioButton* m_SameAuthors;
    wxRadioButton* m_NewAuthors;
};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___PUBSTATUS_PANEL__HPP
