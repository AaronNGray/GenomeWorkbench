#ifndef GUI_PKG_SEQ_EDIT___ORGADVANCED_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___ORGADVANCED_PANEL__HPP
/*  $Id: orgadvanced_panel.hpp 43228 2019-05-30 17:19:13Z bollin $
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
 * Authors:  Andrea Asztalos
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include <gui/widgets/edit/submission_page_interface.hpp>
#include <wx/panel.h>
#include <wx/hyperlink.h>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <objects/seqfeat/OrgMod.hpp>
#include <gui/utils/command_processor.hpp>
#include <objmgr/seq_entry_handle.hpp>

class wxScrolledWindow;
class wxFlexGridSizer;


BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CORGADVANCEDPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CORGADVANCEDPANEL_TITLE _("Organism Advanced Panel")
#define SYMBOL_CORGADVANCEDPANEL_IDNAME ID_CORGADVANCEDPANEL
#define SYMBOL_CORGADVANCEDPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CORGADVANCEDPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * COrgAdvancedPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT COrgAdvancedPanel: public wxPanel, public ISubmissionPage
{    
    DECLARE_DYNAMIC_CLASS( COrgAdvancedPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    COrgAdvancedPanel();
    COrgAdvancedPanel( wxWindow* parent, 
        ICommandProccessor* cmdproc,
        objects::CSeq_entry_Handle seh,
        wxWindowID id = SYMBOL_CORGADVANCEDPANEL_IDNAME,
        const wxPoint& pos = SYMBOL_CORGADVANCEDPANEL_POSITION, 
        const wxSize& size = SYMBOL_CORGADVANCEDPANEL_SIZE, 
        long style = SYMBOL_CORGADVANCEDPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CORGADVANCEDPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CORGADVANCEDPANEL_POSITION, 
        const wxSize& size = SYMBOL_CORGADVANCEDPANEL_SIZE, 
        long style = SYMBOL_CORGADVANCEDPANEL_STYLE );

    /// Destructor
    ~COrgAdvancedPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    void ApplyBioSource(objects::CBioSource& src);
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    void ApplyCommand();

////@begin COrgAdvancedPanel event handler declarations

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_ORGADVHYPERLINK
    void OnAddAttributeHyperlinkClicked( wxHyperlinkEvent& event );

    void OnDeleteQualifier(wxHyperlinkEvent& event);

////@end COrgAdvancedPanel event handler declarations

////@begin COrgAdvancedPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end COrgAdvancedPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual void SetSeqEntryHandle(objects::CSeq_entry_Handle seh) { m_Seh = seh; }
    virtual wxString GetAnchor() { return _("organism-info-additional-qualifiers"); }

private:
    void x_AddEmptyRow();
    void x_AddRowToWindow(wxWindow* row);

    void x_Reset();
    void x_SetBioSourceModifier(objects::CBioSource& src, const string& name, const string& value);
    void x_SetBioSourceModifier(const string& name, const string& value);
    bool x_ApplyChangesToSource(objects::CBioSource& src);

    size_t m_Rows{ 2 };
    int m_RowHeight{ 0 };
#ifndef __WXMSW__
    int m_TotalHeight{ 0 };
    int	m_TotalWidth{ 0 };
    int	m_ScrollRate{ 0 };
    int m_MaxRows{ 4 };
#endif
    CRef<objects::CBioSource> m_Source;
    ICommandProccessor* m_CmdProcessor;
    objects::CSeq_entry_Handle m_Seh;

////@begin COrgAdvancedPanel member variables
    wxScrolledWindow* m_ScrolledWindow;
    wxFlexGridSizer* m_Sizer;
public:
    /// Control identifiers
    enum {
        ID_CORGADVANCEDPANEL = 6592,
        ID_ORGADVSCROLLEDWND = 6593,
        ID_ORGADVHYPERLINK = 6594,
        ID_ORGADVDELETE = 6595
    };
////@end COrgAdvancedPanel member variables
};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___ORGADVANCED_PANEL__HPP
