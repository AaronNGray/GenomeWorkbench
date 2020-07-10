#ifndef GUI_WIDGETS_EDIT___AUTHORNAMES_PANEL__HPP
#define GUI_WIDGETS_EDIT___AUTHORNAMES_PANEL__HPP

/*  $Id: authornames_panel.hpp 43221 2019-05-29 19:53:05Z filippov $
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
#include <objects/biblio/Author.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/utils/command_processor.hpp>
#include <gui/widgets/edit/author_names_container.hpp>
#include <gui/widgets/edit/import_export_access.hpp>
#include <gui/widgets/edit/submission_page_interface.hpp>

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <gui/widgets/wx/wx_utils.hpp>
#include <wx/panel.h>
#include <wx/hyperlink.h>

/*!
 * Includes
 */

////@begin includes
#include "wx/hyperlink.h"
#include "wx/statline.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

class wxScrolledWindow;

BEGIN_NCBI_SCOPE

class CSerialObject;

/*!
 * Control identifiers
 */


////@begin control identifiers
#define ID_DELETELINK 7000
#define ID_REPLACE_ALL_AUTHORS 7001
#define ID_REPLACE_SAME_AUTHORS 7002
#define SYMBOL_CAUTHORNAMESPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CAUTHORNAMESPANEL_TITLE _("AuthorNames")
#define SYMBOL_CAUTHORNAMESPANEL_IDNAME ID_AUTHORNAMES
#define SYMBOL_CAUTHORNAMESPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CAUTHORNAMESPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CAuthorNamesPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CAuthorNamesPanel: public CAuthorNamesContainer, public CImportExportAccess, public ISubmissionPage
{    
    DECLARE_DYNAMIC_CLASS( CAuthorNamesPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAuthorNamesPanel();
    CAuthorNamesPanel( wxWindow* parent, CSerialObject& object, bool show_replace_buttons,
                       wxWindowID id = SYMBOL_CAUTHORNAMESPANEL_IDNAME, const wxPoint& pos = SYMBOL_CAUTHORNAMESPANEL_POSITION, const wxSize& size = SYMBOL_CAUTHORNAMESPANEL_SIZE, long style = SYMBOL_CAUTHORNAMESPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CAUTHORNAMESPANEL_IDNAME, const wxPoint& pos = SYMBOL_CAUTHORNAMESPANEL_POSITION, const wxSize& size = SYMBOL_CAUTHORNAMESPANEL_SIZE, long style = SYMBOL_CAUTHORNAMESPANEL_STYLE );

    /// Destructor
    ~CAuthorNamesPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    bool x_GetAuthors(bool keep_blank = false);
    void PopulateAuthors (objects::CAuth_list& auth_list);

    /// @name CAuthorNamesContainer implementation
    /// @{
    virtual void AddLastAuthor (wxWindow* link);
    virtual void InsertAuthorBefore (wxWindow* link);
    virtual void InsertAuthorAfter (wxWindow* link);
    virtual void SetAuthors(const objects::CAuth_list& auth_list);
    /// @}

////@begin CAuthorNamesPanel event handler declarations

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_ADDAUTH
    void OnAddauthHyperlinkClicked( wxHyperlinkEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_ADDCONSORTIUM
    void OnAddconsortiumHyperlinkClicked( wxHyperlinkEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_IMPORT_AUTHORS
    void OnImportAuthorsHyperlinkClicked( wxHyperlinkEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_IMPORT_AUTHORS_AFFILIATION
    void OnImportAuthorsAffiliationHyperlinkClicked( wxHyperlinkEvent& event );

    void OnDelete(wxHyperlinkEvent& event);

    void OnReplaceAllAuthors( wxCommandEvent& event );
    void OnReplaceSameAuthors( wxCommandEvent& event );

////@end CAuthorNamesPanel event handler declarations

////@begin CAuthorNamesPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAuthorNamesPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual bool IsImportEnabled(void);
    virtual bool IsExportEnabled(void);
    virtual CRef<CSerialObject> OnExport(void);
    virtual void OnImport( CNcbiIfstream &istr);

////@begin CAuthorNamesPanel member variables

    /// Control identifiers
    enum {
        ID_AUTHORNAMES = 10022,
        ID_ADDAUTH = 10023,
        ID_ADDCONSORTIUM = 10024,
        ID_IMPORT_AUTHORS = 10025,
        ID_IMPORT_AUTHORS_AFFILIATION = 10026,
        ID_PANEL = 10027
    };
////@end CAuthorNamesPanel member variables

    static bool IsAuthorEmpty (const objects::CAuthor& auth);

    virtual void ApplySubmitBlock(objects::CSubmit_block& block);
    virtual void ApplyCommand();
	virtual wxString GetAnchor() {return _("reference-sequence-authors");}
private:
    /// @name CAuthorNamesContainer implementation
    /// @{
    virtual wxSizerItem* x_FindSingleAuthor(wxWindow* wnd, wxSizerItemList& itemList);
    virtual void x_AddRowToWindow(wxWindow* row);
    virtual void x_InsertRowtoWindow(wxWindow* row, size_t index);
    virtual void x_MoveRowUp(int row_num);
    virtual int x_FindRow(wxWindow* wnd, wxSizerItemList& itemList);
    /// @}

    void x_AddNewEmptyAuthor(void);
    void x_SetAffiliation();
    ICommandProccessor* GetUndoManager();
    objects::CSeq_entry_Handle GetSeqEntryHandle();
    void x_AdjustScroll(wxWindow* row, bool scroll_to_end = true);
    
    CSerialObject* m_Object;
    CRef<CSerialObject> m_EditedAuthList;
    int m_RowHeight;
    bool m_show_replace_buttons;

    wxWindow* m_Consortium;
    CRef<objects::CSubmit_block> m_SubmitBlock;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___AUTHORNAMES_PANEL__HPP
