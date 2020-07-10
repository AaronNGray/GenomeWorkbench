#ifndef GUI_PKG_SEQ_EDIT___REFERENCE_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___REFERENCE_PANEL__HPP

/*  $Id: reference_panel.hpp 43209 2019-05-29 14:01:26Z bollin $
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
#include <objects/biblio/Auth_list.hpp>
#include <objects/biblio/Affil.hpp>
#include <gui/widgets/edit/submission_page_interface.hpp>
#include <gui/utils/command_processor.hpp>
#include <objmgr/seq_entry_handle.hpp>

#include <wx/panel.h>
#include <wx/bookctrl.h>

class wxListbook;

BEGIN_NCBI_SCOPE

class CPubAuthorPanel;
class CAuthorNamesPanel;
class CPubStatusPanel;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CREFERENCEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CREFERENCEPANEL_TITLE _("Reference Info Panel")
#define SYMBOL_CREFERENCEPANEL_IDNAME ID_CREFERENCEPANEL
#define SYMBOL_CREFERENCEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CREFERENCEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CReferencePanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CReferencePanel: public wxPanel, public ISubmissionPage
{    
    DECLARE_DYNAMIC_CLASS( CReferencePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CReferencePanel();
    CReferencePanel( wxWindow* parent, 
                     ICommandProccessor* cmdproc,
                     objects::CSeq_entry_Handle seh,
                     wxWindowID id = SYMBOL_CREFERENCEPANEL_IDNAME, 
                     const wxPoint& pos = SYMBOL_CREFERENCEPANEL_POSITION, 
                     const wxSize& size = SYMBOL_CREFERENCEPANEL_SIZE, 
                     long style = SYMBOL_CREFERENCEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CREFERENCEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CREFERENCEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CREFERENCEPANEL_SIZE, 
        long style = SYMBOL_CREFERENCEPANEL_STYLE );

    /// Destructor
    ~CReferencePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CReferencePanel event handler declarations
    void OnPageChanged(wxBookCtrlEvent& event);
    void OnPageChanging(wxBookCtrlEvent& event);
////@end CReferencePanel event handler declarations

////@begin CReferencePanel member function declarations
    virtual bool StepForward();
    virtual bool StepBackward();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CReferencePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual void ApplySubmitBlock(objects::CSubmit_block& block);
    virtual void ApplyDescriptor(objects::CSeqdesc& desc);

    virtual bool TransferDataFromWindow();
    virtual bool TransferDataToWindow();

    bool MatchesCitSubAuthors(const objects::CAuth_list& authlist);
    bool CopyAuthors(objects::CAuth_list& authlist);
    void CopyAffil(objects::CAuth_list &auth_list);

    virtual void SetSeqEntryHandle(objects::CSeq_entry_Handle seh);
    virtual void ApplyCommand();
    virtual void ReportMissingFields(string &text);
    virtual wxString GetAnchor();

private:
    ICommandProccessor* m_CmdProcessor;
    objects::CSeq_entry_Handle m_Seh;
    CRef<objects::CSubmit_block> m_SubmitBlock;
    CRef<objects::CSeqdesc> m_Pub;

////@begin CReferencePanel member variables
    wxListbook* m_Listbook;
//    CPubAuthorPanel* m_AuthorPanel;
    CAuthorNamesPanel* m_AuthorsPanel;
    CPubStatusPanel* m_StatusPanel;
public:
    /// Control identifiers
    enum {
        ID_CREFERENCEPANEL = 6640,
        ID_REFERENCELISTBOOK = 6641
    };
////@end CReferencePanel member variables
};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___REFERENCE_PANEL__HPP
