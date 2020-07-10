#ifndef GUI_WIDGETS_EDIT___PUBLICATIONTYPE_PANEL__HPP
#define GUI_WIDGETS_EDIT___PUBLICATIONTYPE_PANEL__HPP

/*  $Id: publicationtype_panel.hpp 44569 2020-01-22 20:33:17Z filippov $
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
 * Authors:  Colleen Bollin
 */

#include <corelib/ncbistd.hpp>

#include <objects/seq/Pubdesc.hpp>
#include <objects/biblio/Cit_gen.hpp>
#include <objects/biblio/Cit_art.hpp>
#include <objects/biblio/Cit_book.hpp>
#include <objects/biblio/Cit_pat.hpp>
#include <objects/biblio/Cit_let.hpp>
#include <objects/biblio/Imprint.hpp>
#include <objects/pub/Pub.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/utilities.hpp>
#include <gui/widgets/edit/import_export_access.hpp>
#include <gui/widgets/edit/unpublished_panel.hpp>

/*!
 * Includes
 */

////@begin includes
#include "wx/notebook.h"
////@end includes

#include <wx/radiobox.h>
#include <wx/checkbox.h>
#include <wx/panel.h>
#include <wx/frame.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxNotebook;
////@end forward declarations

class CPublisherPanel;
class CJournalPanel;
class CPatentPanel;
class CCitSubPanel;

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
class CCit_gen;
class CCit_art;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_PUBLICATIONTYPE 10018
#define ID_UNPUB_INPRESS_PUB 10019
#define ID_PUBCLASS 10020
#define ID_TREEBOOK 10017
#define	ID_DOI_LOOKUP 10021
#define ID_DOI_PMID 10022

#define SYMBOL_CPUBLICATIONTYPEPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CPUBLICATIONTYPEPANEL_TITLE _("PublicationType")
#define SYMBOL_CPUBLICATIONTYPEPANEL_IDNAME ID_PUBLICATIONTYPE
#define SYMBOL_CPUBLICATIONTYPEPANEL_SIZE wxSize(400, 400)
#define SYMBOL_CPUBLICATIONTYPEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CPublicationTypePanel class declaration
 */

class CPublicationTypePanel: public wxPanel, public IDescEditorPanel, public CImportExportAccess
{    
    DECLARE_DYNAMIC_CLASS( CPublicationTypePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CPublicationTypePanel();
    CPublicationTypePanel( wxWindow* parent, CRef<objects::CPubdesc> pubdesc, 
        bool create_mode, 
        wxWindowID id = SYMBOL_CPUBLICATIONTYPEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CPUBLICATIONTYPEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CPUBLICATIONTYPEPANEL_SIZE, 
        long style = SYMBOL_CPUBLICATIONTYPEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CPUBLICATIONTYPEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CPUBLICATIONTYPEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CPUBLICATIONTYPEPANEL_SIZE, 
        long style = SYMBOL_CPUBLICATIONTYPEPANEL_STYLE );

    /// Destructor
    ~CPublicationTypePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    CRef<objects::CPubdesc> GetPubdesc() const;

////@begin CPublicationTypePanel event handler declarations

    /// wxEVT_COMMAND_RADIOBOX_SELECTED event handler for ID_UNPUB_INPRESS_PUB
    void OnUnpubInpressPubSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBOX_SELECTED event handler for ID_PUBCLASS
    void OnPubclassSelected( wxCommandEvent& event );

    void OnDoiLookup( wxCommandEvent& event );

////@end CPublicationTypePanel event handler declarations

////@begin CPublicationTypePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CPublicationTypePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CPublicationTypePanel member variables
    wxRadioBox* m_Status;
    wxRadioBox* m_PubClass;
    wxNotebook* m_PubdescDetails;
    wxTextCtrl* m_PmidCtrl;
    wxButton* m_LookupButton;
////@end CPublicationTypePanel member variables

    void SetPub(const objects::CPubdesc& pubdesc);

    // IDescEditorPanel implementation
    virtual void ChangeSeqdesc(const objects::CSeqdesc& desc);
    virtual void UpdateSeqdesc(objects::CSeqdesc& desc);
    virtual bool IsImportEnabled(void);
    virtual bool IsExportEnabled(void);
    virtual CRef<CSerialObject> OnExport(void);
    virtual void OnImport( CNcbiIfstream &istr);
    void SetFinalValidation(void);

private:
    CRef<objects::CPubdesc> m_Pubdesc;
    int m_SerialNumber;
    bool m_CreateMode;

    enum EPubType   {
        ePubTypeJournalArticle = 0,
        ePubTypeBookChapter,
        ePubTypeBook,
        ePubTypeThesis,
        ePubTypeProcChapter,
        ePubTypeProc,
        ePubTypePatent,
        ePubTypeSub
    };

    int m_CurrentStatus;
    int m_CurrentType;
    long m_Pmid;
    long m_Muid;

    CPublisherPanel* m_PublisherPanel;
    CJournalPanel* m_JournalPanel;
    CPatentPanel* m_PatentPanel;
    CCitSubPanel* m_CitSubPanel;
    CUnpublishedPanel *m_Unpublished;

    void x_CreatePubControls(CRef<objects::CPub> pub);
    void x_CreateUnpubControls(objects::CCit_gen& cit_gen);
    void x_CreateJournalArticleControls(objects::CCit_art& art);
    void x_CreateBookChapterControls(objects::CCit_art& art);
    void x_CreateBookControls(objects::CCit_book& book, bool is_chapter);
    void x_CreateProcChapterControls(objects::CCit_art& art);
    void x_CreateProcControls(objects::CCit_proc& proc);
    void x_CreateSubControls(objects::CCit_sub& sub);
    void x_CreatePatentControls(objects::CCit_pat &pat);
    void x_CreateThesisControls(objects::CCit_let &let);
    void x_SetStatus (const objects::CImprint& imprint);
    void x_SetStatusAndClass (CRef<objects::CPub> pub);
    bool x_CollectAuthors (void);
    CRef<objects::CPub> x_GetMainPub(void);
    bool x_CollectDetails (void);
    CRef<CPub> x_PutMainPubFirst(int pubtype, int status);
};

END_SCOPE(objects)
END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___PUBLICATIONTYPE_PANEL__HPP
