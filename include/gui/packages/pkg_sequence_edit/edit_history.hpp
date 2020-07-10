/*  $Id: edit_history.hpp 38635 2017-06-05 18:51:04Z asztalos $
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
#ifndef EDIT_HISTORY_HPP
#define EDIT_HISTORY_HPP

#include <corelib/ncbistd.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/widgets/edit/flexibledate_panel.hpp>

/*!
 * Includes
 */

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/hyperlink.h>

////@begin includes
////@end includes



class CGrowableListCtrl;

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)


/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CEDITHISTORY wxID_ANY
#define SYMBOL_CEDITHISTORY_STYLE  wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CEDITHISTORY_TITLE _("Edit History")
#define SYMBOL_CEDITHISTORY_IDNAME ID_CEDITHISTORY
#define SYMBOL_CEDITHISTORY_SIZE wxDefaultSize
#define SYMBOL_CEDITHISTORY_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CEditHistory class declaration
 */

class CEditHistory : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CEditHistory )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CEditHistory();
    CEditHistory( wxWindow* parent, CBioseq_Handle bsh, 
                  wxWindowID id = SYMBOL_CEDITHISTORY_IDNAME, const wxString& caption = SYMBOL_CEDITHISTORY_TITLE, const wxPoint& pos = SYMBOL_CEDITHISTORY_POSITION, const wxSize& size = SYMBOL_CEDITHISTORY_SIZE, long style = SYMBOL_CEDITHISTORY_STYLE );
    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CEDITHISTORY_IDNAME, const wxString& caption = SYMBOL_CEDITHISTORY_TITLE, const wxPoint& pos = SYMBOL_CEDITHISTORY_POSITION, const wxSize& size = SYMBOL_CEDITHISTORY_SIZE, long style = SYMBOL_CEDITHISTORY_STYLE );

    /// Destructor
    ~CEditHistory();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CEditHistory event handler declarations

////@end CEditHistory event handler declarations

////@begin CEditHistory member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CEditHistory member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CEditHistory member variables
////@end CEditHistory member variables
    CRef<CCmdComposite> GetCommand();

private:
    CBioseq_Handle m_Bioseq;
    CGrowableListCtrl* m_IdListReplaces;
    CGrowableListCtrl* m_IdListReplacedBy;
    wxCheckBox *m_Deleted;
    CFlexibleDatePanel *m_DeletedDate;
    CFlexibleDatePanel *m_ReplacesDate;
    CFlexibleDatePanel *m_ReplacedByDate;
};



END_SCOPE(objects)
END_NCBI_SCOPE

#endif  // EDIT_HISTORY_HPP
