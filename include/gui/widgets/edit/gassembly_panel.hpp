#ifndef GUI_PKG_SEQ_EDIT___GASSEMBLY_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___GASSEMBLY_PANEL__HPP
/*  $Id: gassembly_panel.hpp 44733 2020-03-03 17:13:24Z asztalos $
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
#include <gui/widgets/edit/submission_page_interface.hpp>
#include <wx/panel.h>
#include <wx/hyperlink.h>
#include <objects/general/User_object.hpp>
#include <gui/widgets/edit/flexibledate_panel.hpp>
#include <gui/utils/command_processor.hpp>
#include <objmgr/seq_entry_handle.hpp>

class wxDatePickerCtrl;
class wxTextCtrl;
class wxScrolledWindow;
class wxFlexGridSizer;

BEGIN_NCBI_SCOPE
/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CGASSEMBLYPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CGASSEMBLYPANEL_TITLE _("Genome Assembly Panel")
#define SYMBOL_CGASSEMBLYPANEL_IDNAME ID_CGASSEMBLYPANEL
#define SYMBOL_CGASSEMBLYPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CGASSEMBLYPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CGAssemblyPanel class declaration
 */

class CGAssemblyPanel: public wxPanel, public ISubmissionPage
{    
    DECLARE_DYNAMIC_CLASS( CGAssemblyPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CGAssemblyPanel();
    CGAssemblyPanel( wxWindow* parent, 
                     ICommandProccessor* cmdproc,
                     objects::CSeq_entry_Handle seh,
                     wxWindowID id = SYMBOL_CGASSEMBLYPANEL_IDNAME, 
                     const wxPoint& pos = SYMBOL_CGASSEMBLYPANEL_POSITION, 
                     const wxSize& size = SYMBOL_CGASSEMBLYPANEL_SIZE, 
                     long style = SYMBOL_CGASSEMBLYPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CGASSEMBLYPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CGASSEMBLYPANEL_POSITION, 
        const wxSize& size = SYMBOL_CGASSEMBLYPANEL_SIZE, 
        long style = SYMBOL_CGASSEMBLYPANEL_STYLE );

    /// Destructor
    ~CGAssemblyPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    void ApplyUser(objects::CUser_object& user);
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();


////@begin CGAssemblyPanel event handler declarations

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_GAASSEMBLYHYPERLINK
    void OnAddNewAssemblyMethod( wxHyperlinkEvent& event );
    void OnDeleteAssembly(wxHyperlinkEvent& event);

////@end CGAssemblyPanel event handler declarations

////@begin CGAssemblyPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CGAssemblyPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual void ApplyCommand();
    virtual void SetSeqEntryHandle(objects::CSeq_entry_Handle seh) {m_Seh = seh;}
    virtual wxString GetAnchor() { return _("genome-info-assembly"); }

    /// Control identifiers
    enum {
        ID_CGASSEMBLYPANEL = 6540,
        ID_GAASSEMBLYDATE = 6541,
        ID_GAASSEMBLYNAME = 6542,
        ID_GASCROLLEDWND = 6543,
        ID_GAASSEMBLYHYPERLINK = 6544,
        ID_GAASSEMBLYDELETE = 6545
    };
private:
    void x_AddEmptyRow();
    void x_AddRowToWindow(wxWindow* row);
    void x_Reset();

    size_t m_Rows{ 2 };
    int m_RowHeight{ 0 };
#ifndef __WXMSW__
    int m_TotalHeight{ 0 };
    int	m_TotalWidth{ 0 };
    int	m_ScrollRate{ 0 };
    int m_MaxRows{ 4 };
#endif

    CRef<objects::CUser_object> m_User;
    ICommandProccessor* m_CmdProcessor;
    objects::CSeq_entry_Handle m_Seh;

////@begin CGAssemblyPanel member variables
    CFlexibleDatePanel* m_AssemblyDate;
    wxTextCtrl* m_AssemblyName;
    wxScrolledWindow* m_ScrolledWindow; 
    wxFlexGridSizer* m_Sizer;
////@end CGAssemblyPanel member variables
};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___GASSEMBLY_PANEL__HPP
