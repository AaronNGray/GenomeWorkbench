/*  $Id: vectortrimdlg.hpp 38632 2017-06-05 17:16:13Z asztalos $
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
 * Authors:  Yoon Choi
 */
#ifndef _VECTORTRIMDLG_H_
#define _VECTORTRIMDLG_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/widgets/edit/vectortrimpanel.hpp>
#include <gui/widgets/data/report_dialog.hpp>

#include <wx/dialog.h>

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

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_VECTORTRIMDLG wxID_ANY
#define SYMBOL_CVECTORTRIMDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CVECTORTRIMDLG_TITLE _("Vector Trim Dialog")
#define SYMBOL_CVECTORTRIMDLG_IDNAME ID_VECTORTRIMDLG
#define SYMBOL_CVECTORTRIMDLG_SIZE wxDefaultSize
#define SYMBOL_CVECTORTRIMDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CVectorTrimDlg class declaration
 */

class CVectorTrimDlg : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CVectorTrimDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CVectorTrimDlg();
    CVectorTrimDlg( wxWindow* parent, 
		    objects::CSeq_entry_Handle seh, 
                    ICommandProccessor* cmdProcessor,
		    wxWindowID id = SYMBOL_CVECTORTRIMDLG_IDNAME, 
		    const wxString& caption = SYMBOL_CVECTORTRIMDLG_TITLE, 
		    const wxPoint& pos = SYMBOL_CVECTORTRIMDLG_POSITION, 
		    const wxSize& size = SYMBOL_CVECTORTRIMDLG_SIZE, 
		    long style = SYMBOL_CVECTORTRIMDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
		 wxWindowID id = SYMBOL_CVECTORTRIMDLG_IDNAME, 
		 const wxString& caption = SYMBOL_CVECTORTRIMDLG_TITLE, 
		 const wxPoint& pos = SYMBOL_CVECTORTRIMDLG_POSITION, 
		 const wxSize& size = SYMBOL_CVECTORTRIMDLG_SIZE, 
		 long style = SYMBOL_CVECTORTRIMDLG_STYLE );

    /// Destructor
    ~CVectorTrimDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CVectorTrimDlg event handler declarations

////@end CVectorTrimDlg event handler declarations

////@begin CVectorTrimDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CVectorTrimDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CVectorTrimDlg member variables
    wxBoxSizer* m_PanelSizer;
////@end CVectorTrimDlg member variables

private:
    objects::CSeq_entry_Handle m_TopSeqEntry;
    ICommandProccessor* m_CmdProcessor;
    CVectorTrimPanel* m_Panel;
};

END_NCBI_SCOPE

#endif
    // _VECTORTRIMDLG_H_
