#ifndef GUI_WIDGETS_EDIT___SRCMOD_LIST_PANEL__HPP
#define GUI_WIDGETS_EDIT___SRCMOD_LIST_PANEL__HPP

/*  $Id: srcmod_list_panel.hpp 42655 2019-03-28 20:17:09Z asztalos $
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
 * Authors:  Vasuki Palanigobu
 */

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/OrgMod.hpp>
#include <wx/panel.h>
#include <wx/hyperlink.h>
#include <gui/widgets/wx/richtextctrl.hpp>
#include "pcr_primers_panel.hpp"
#include <gui/widgets/edit/srcmod_panel.hpp>
#include "wx/hyperlink.h"

class wxFlexGridSizer;
class wxCheckListBox;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSRCMODLISTPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSRCMODLISTPANEL_TITLE _("Source Modifiers")
#define SYMBOL_CSRCMODLISTPANEL_IDNAME ID_CSRCMODLISTPANEL
#define SYMBOL_CSRCMODLISTPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSRCMODLISTPANEL_POSITION wxDefaultPosition
////@end control identifiers
#define ID_PCR_CHECKBOX 10032


/*!
 * CSrcModListPanel class declaration
 */

class CSrcModListPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CSrcModListPanel )
    DECLARE_EVENT_TABLE()

public:

    using TVecModData = list<CSrcModPanel::SModData>;

    /// Constructors
    CSrcModListPanel();
    CSrcModListPanel( wxWindow* parent, objects::CBioSource& source, wxWindowID id = SYMBOL_CSRCMODLISTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSRCMODLISTPANEL_POSITION, const wxSize& size = SYMBOL_CSRCMODLISTPANEL_SIZE, long style = SYMBOL_CSRCMODLISTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSRCMODLISTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSRCMODLISTPANEL_POSITION, const wxSize& size = SYMBOL_CSRCMODLISTPANEL_SIZE, long style = SYMBOL_CSRCMODLISTPANEL_STYLE );

    /// Destructor
    ~CSrcModListPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSrcModListPanel event handler declarations

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_CLR_SRC_NOTE
    void OnClrSrcNoteHyperlinkClicked( wxHyperlinkEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_CLR_ORG_NOTE
    void OnClrOrgNoteHyperlinkClicked( wxHyperlinkEvent& event );

////@end CSrcModListPanel event handler declarations
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    void PopulateModifiers(objects::CBioSource& biosrc);
    void SetSource(const objects::CBioSource& biosrc);

    void AddLastSrcMod (wxWindow* link);

////@begin CSrcModListPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSrcModListPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSrcModListPanel member variables
    wxScrolledWindow* m_ScrolledWindow;
    wxFlexGridSizer* m_Sizer;
    CRichTextCtrl* m_SrcInfo;
    CRichTextCtrl* m_OrgNote;
    /// Control identifiers
    enum {
        ID_CSRCMODLISTPANEL = 10022,
        ID_SCROLLEDWINDOW1 = 10015,
        ID_TEXTCTRL7 = 10031,
        ID_CLR_SRC_NOTE = 10037,
        ID_TEXTCTRL4 = 10036,
        ID_CLR_ORG_NOTE = 10038
    };
////@end CSrcModListPanel member variables

    void OnDelete (wxHyperlinkEvent& event);
    void OnPCRCheckBox(wxCommandEvent& event);
    void OnAddPCRPrimer(wxCommandEvent& event);
    void OnChangedTaxname(void);

private:
	CRef<objects::CBioSource>	m_Source;
	TVecModData					m_Data;
    string                      m_SrcNoteText;
    string                      m_OrgNoteText;
    int							m_NumRows;
    int							m_TotalHeight;
    int							m_TotalWidth;
    int							m_ScrollRate;
    list<CRef<objects::COrgMod> > m_ReadOnlyQuals;

    //int							m_MaxRowsDisplayed;
    CPCRPrimersPanel* m_PCRPrimersPanel;
    wxBoxSizer* m_PCRPrimersBoxSizer1;
    wxBoxSizer* m_PCRPrimersBoxSizer2;
    wxBoxSizer* m_PCRPrimersBoxSizer3;
    wxCheckBox* m_PCRCheckbox;

    void x_FillDataList();
    void x_UpdateList();
    void x_SetSrcSubMod(int subtype, const string& value);
    void x_SetSrcOrgMod(int subtype, const string& value);
    void x_SetBioSourceModifier(const string& name, const string& value);
    void x_AddEmptyRow();
    void x_AddRow(wxString name, wxString value);
    wxSizerItemList::iterator   x_FindRow(wxWindow* wnd, wxSizerItemList& itemList);
    void x_CollectTextMods ();
    void x_AddTextSrcMods ();
    void x_ShowPCRPrimerItems(bool value);
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___SRCMOD_LIST_PANEL__HPP

