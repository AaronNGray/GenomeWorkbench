#ifndef GUI_WIDGETS_SEQ___SELECT_SEQUENCE__HPP
#define GUI_WIDGETS_SEQ___SELECT_SEQUENCE__HPP

/*  $Id: select_sequence_dlg.hpp 35537 2016-05-18 17:18:09Z katargir $
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

/*!
 * Includes
 */

#include <corelib/ncbistd.hpp>
#include <gui/widgets/wx/dialog.hpp>

////@begin includes
#include "wx/listctrl.h"
////@end includes


#include <gui/widgets/seq/text_panel.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
class CListCtrlSeqs;
////@end forward declarations

BEGIN_NCBI_SCOPE

class CCompositeTextItem;
class CListCtrlSeqs;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSELECTSEQUENCEDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CSELECTSEQUENCEDLG_TITLE _("Select Sequence to View")
#define SYMBOL_CSELECTSEQUENCEDLG_IDNAME ID_CSELECTSEQUENCEDLG
#define SYMBOL_CSELECTSEQUENCEDLG_SIZE wxSize(266, 184)
#define SYMBOL_CSELECTSEQUENCEDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSelectSequenceDlg class declaration
 */

class CSelectSequenceDlg: public CDialog
{    
    DECLARE_DYNAMIC_CLASS( CSelectSequenceDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSelectSequenceDlg();
    CSelectSequenceDlg(wxWindow* parent, vector<CFlatFileSeq>& seqs, const wxString& select);

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSELECTSEQUENCEDLG_IDNAME, const wxString& caption = SYMBOL_CSELECTSEQUENCEDLG_TITLE, const wxPoint& pos = SYMBOL_CSELECTSEQUENCEDLG_POSITION, const wxSize& size = SYMBOL_CSELECTSEQUENCEDLG_SIZE, long style = SYMBOL_CSELECTSEQUENCEDLG_STYLE );

    /// Destructor
    ~CSelectSequenceDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataFromWindow();

////@begin CSelectSequenceDlg event handler declarations

    /// wxEVT_COMMAND_LIST_ITEM_ACTIVATED event handler for ID_LISTCTRL
    void OnListctrlItemActivated( wxListEvent& event );

////@end CSelectSequenceDlg event handler declarations

////@begin CSelectSequenceDlg member function declarations

    string GetSequenceId() const { return m_SequenceId ; }
    void SetSequenceId(string value) { m_SequenceId = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSelectSequenceDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSelectSequenceDlg member variables
    CListCtrlSeqs* m_SeqList;
private:
    string m_SequenceId;
    /// Control identifiers
    enum {
        ID_CSELECTSEQUENCEDLG = 10007,
        ID_LISTCTRL = 10013
    };
////@end CSelectSequenceDlg member variables

    vector<CFlatFileSeq>* m_Seqs;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_SEQ___SELECT_SEQUENCE__HPP
