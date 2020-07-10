#ifndef GUI_PKG_SEQ_EDIT___GENOMEINFO_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___GENOMEINFO_PANEL__HPP
/*  $Id: genomeinfo_panel.hpp 43209 2019-05-29 14:01:26Z bollin $
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software / database is a "United States Government Work" under the
*  terms of the United States Copyright Act.It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.This software / database is freely available
*  to the public for use.The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data.The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Authors: Andrea Asztalos
*
*/

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>
#include <gui/widgets/edit/submission_page_interface.hpp>
#include <gui/utils/command_processor.hpp>
#include <objmgr/seq_entry_handle.hpp>

#include <wx/panel.h>
#include <wx/bookctrl.h>

class wxListbook;

BEGIN_NCBI_SCOPE

class CGAssemblyPanel;
class CGAssemblyOtherPanel;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CGENOMEINFOPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CGENOMEINFOPANEL_TITLE _("Genome Info Panel")
#define SYMBOL_CGENOMEINFOPANEL_IDNAME ID_CGENOMEINFOPANEL
#define SYMBOL_CGENOMEINFOPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CGENOMEINFOPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CGenomeInfoPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CGenomeInfoPanel: public wxPanel, public ISubmissionPage
{    
    DECLARE_DYNAMIC_CLASS( CGenomeInfoPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CGenomeInfoPanel();
    CGenomeInfoPanel( wxWindow* parent, 
                      ICommandProccessor* cmdproc,
                      objects::CSeq_entry_Handle seh,
                      wxWindowID id = SYMBOL_CGENOMEINFOPANEL_IDNAME, 
                      const wxPoint& pos = SYMBOL_CGENOMEINFOPANEL_POSITION, 
                      const wxSize& size = SYMBOL_CGENOMEINFOPANEL_SIZE, 
                      long style = SYMBOL_CGENOMEINFOPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CGENOMEINFOPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CGENOMEINFOPANEL_POSITION, 
        const wxSize& size = SYMBOL_CGENOMEINFOPANEL_SIZE, 
        long style = SYMBOL_CGENOMEINFOPANEL_STYLE );

    /// Destructor
    ~CGenomeInfoPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();
    virtual bool TransferDataToWindow();
    virtual void ApplyCommand();
    virtual void ReportMissingFields(string &text);
    virtual wxString GetAnchor();

////@begin CGenomeInfoPanel event handler declarations
    void OnPageChanged(wxBookCtrlEvent& event);
    void OnPageChanging(wxBookCtrlEvent& event);
////@end CGenomeInfoPanel event handler declarations

////@begin CGenomeInfoPanel member function declarations

    virtual bool StepForward();
    virtual bool StepBackward();

    virtual void ApplyDescriptor(objects::CSeqdesc& desc);
    virtual void SetSeqEntryHandle(objects::CSeq_entry_Handle seh);

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CGenomeInfoPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

private:
////@begin CGenomeInfoPanel member variables
    wxListbook* m_Listbook;
    CGAssemblyPanel* m_AssemblyPanel;
    CGAssemblyOtherPanel* m_OtherPanel;

    objects::CSeq_entry_Handle m_Seh;
    ICommandProccessor* m_CmdProcessor;

public:
    /// Control identifiers
    enum {
        ID_CGENOMEINFOPANEL = 6555,
        ID_GENOMENOTEBOOK = 6556
    };
////@end CGenomeInfoPanel member variables
};

END_NCBI_SCOPE

#endif
    // _GENOMEINFO_PANEL_H_
