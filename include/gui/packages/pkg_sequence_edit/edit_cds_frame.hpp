/*  $Id: edit_cds_frame.hpp 38638 2017-06-05 19:26:02Z asztalos $
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
 * Authors: Igor Filippov
 */
#ifndef _EDIT_CDS_FRAME_H_
#define _EDIT_CDS_FRAME_H_

#include <corelib/ncbistd.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <wx/dialog.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/sizer.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_EDIT_CDS_FRAME wxID_ANY
#define SYMBOL_CEDIT_CDS_FRAME_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CEDIT_CDS_FRAME_TITLE _("Edit CDS Frame")
#define SYMBOL_CEDIT_CDS_FRAME_IDNAME ID_EDIT_CDS_FRAME
#define SYMBOL_CEDIT_CDS_FRAME_SIZE wxSize(200,100)
#define SYMBOL_CEDIT_CDS_FRAME_POSITION wxDefaultPosition
////@end control identifiers



class CConstraintPanel;

class CEditCdsFrame : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CEditCdsFrame )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CEditCdsFrame();
    CEditCdsFrame( wxWindow* parent, 
                   CSeq_entry_Handle seh,
                   wxWindowID id = SYMBOL_CEDIT_CDS_FRAME_IDNAME, 
                   const wxString& caption = SYMBOL_CEDIT_CDS_FRAME_TITLE, 
                   const wxPoint& pos = SYMBOL_CEDIT_CDS_FRAME_POSITION, 
                   const wxSize& size = SYMBOL_CEDIT_CDS_FRAME_SIZE, 
                   long style = SYMBOL_CEDIT_CDS_FRAME_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
		 wxWindowID id = SYMBOL_CEDIT_CDS_FRAME_IDNAME, 
		 const wxString& caption = SYMBOL_CEDIT_CDS_FRAME_TITLE, 
		 const wxPoint& pos = SYMBOL_CEDIT_CDS_FRAME_POSITION, 
		 const wxSize& size = SYMBOL_CEDIT_CDS_FRAME_SIZE, 
		 long style = SYMBOL_CEDIT_CDS_FRAME_STYLE );
    
    /// Destructor
    ~CEditCdsFrame();

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

    CRef<CCmdComposite> GetCommand(void);

private:
    CSeq_entry_Handle m_TopSeqEntry;
    wxChoice* m_From;
    wxChoice* m_To;
    wxCheckBox* m_Retranslate;
    CConstraintPanel* m_Constraint;
};

END_NCBI_SCOPE

#endif
    // _EDIT_CDS_FRAME_H_
