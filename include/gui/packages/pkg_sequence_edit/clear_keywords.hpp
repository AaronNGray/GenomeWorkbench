/*  $Id: clear_keywords.hpp 39538 2017-10-06 14:50:51Z bollin $
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
 * Authors:  Igor Filippov
 */
#ifndef _CLEAR_KEYWORDS_H_
#define _CLEAR_KEYWORDS_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>
#include <wx/checkbox.h>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <gui/widgets/wx/wx_utils.hpp>
#include <wx/textctrl.h>
#include <wx/radiobut.h>
#include <wx/listbox.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE
using namespace objects;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CCLEAR_KEYWORDS wxID_ANY
#define SYMBOL_CCLEAR_KEYWORDS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CCLEAR_KEYWORDS_TITLE _("Clear Keywords")
#define SYMBOL_CCLEAR_KEYWORDS_IDNAME ID_CCLEAR_KEYWORDS
#define SYMBOL_CCLEAR_KEYWORDS_SIZE wxDefaultSize
#define SYMBOL_CCLEAR_KEYWORDS_POSITION wxDefaultPosition
////@end control identifiers


class CClearKeywords : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CClearKeywords )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CClearKeywords();
    CClearKeywords( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id = SYMBOL_CCLEAR_KEYWORDS_IDNAME, const wxString& caption = SYMBOL_CCLEAR_KEYWORDS_TITLE, const wxPoint& pos = SYMBOL_CCLEAR_KEYWORDS_POSITION, const wxSize& size = SYMBOL_CCLEAR_KEYWORDS_SIZE, long style = SYMBOL_CCLEAR_KEYWORDS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCLEAR_KEYWORDS_IDNAME, const wxString& caption = SYMBOL_CCLEAR_KEYWORDS_TITLE, const wxPoint& pos = SYMBOL_CCLEAR_KEYWORDS_POSITION, const wxSize& size = SYMBOL_CCLEAR_KEYWORDS_SIZE, long style = SYMBOL_CCLEAR_KEYWORDS_STYLE );

    /// Destructor
    ~CClearKeywords();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    CRef<CCmdComposite> GetCommand();
private:
    typedef  CChangeUnindexedObjectCommand<objects::CGB_block> CChangeGenbankBlockCommand;
    void ApplyToCSeq_entry(const CSeq_entry& se, CCmdComposite* composite, bool pass_genome);
    objects::CSeq_entry_Handle m_TopSeqEntry;
    wxChoice *m_Status;
    wxTextCtrl *m_Keyword;
    wxCheckBox *m_AllKeywords;
    wxRadioButton* m_Any;
    wxRadioButton* m_Nuc;
    wxRadioButton* m_DNA;
    wxRadioButton* m_RNA;
    wxRadioButton* m_Proteins;
    CStringConstraintPanel* m_StringConstraintPanel;    
    wxListBox *m_SourceLoc;

    void OnAllKeywordsSelected(wxCommandEvent& event);
};



END_NCBI_SCOPE

#endif
    // _CLEAR_KEYWORDS_H_
