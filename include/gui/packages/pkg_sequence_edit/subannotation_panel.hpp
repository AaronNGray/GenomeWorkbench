#ifndef GUI_PKG_SEQ_EDIT___SUBANNOTATION_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___SUBANNOTATION_PANEL__HPP

/*  $Id: subannotation_panel.hpp 43291 2019-06-07 16:14:30Z filippov $
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

#include <corelib/ncbiobj.hpp>
#include <gui/widgets/edit/generic_panel.hpp>
#include <gui/widgets/edit/submission_page_interface.hpp>
#include <gui/utils/job_adapter.hpp>

#include <wx/panel.h>

BEGIN_NCBI_SCOPE


/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSUBANNOTATIONPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSUBANNOTATIONPANEL_TITLE _("Submission Annotation Panel")
#define SYMBOL_CSUBANNOTATIONPANEL_IDNAME ID_CSUBANNOTATIONPANEL
#define SYMBOL_CSUBANNOTATIONPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSUBANNOTATIONPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSubAnnotationPanel class declaration
 */

class CSubAnnotationPanel: public wxPanel, public ISubmissionPage, public CJobAdapter::IJobCallback
{    
    DECLARE_DYNAMIC_CLASS( CSubAnnotationPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSubAnnotationPanel();
    CSubAnnotationPanel( wxWindow* parent,  ICommandProccessor* proc, const wxString &dir, objects::CSeq_entry_Handle seh,
        wxWindowID id = SYMBOL_CSUBANNOTATIONPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSUBANNOTATIONPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSUBANNOTATIONPANEL_SIZE, 
        long style = SYMBOL_CSUBANNOTATIONPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSUBANNOTATIONPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSUBANNOTATIONPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSUBANNOTATIONPANEL_SIZE, 
        long style = SYMBOL_CSUBANNOTATIONPANEL_STYLE );

    /// Destructor
    ~CSubAnnotationPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSubAnnotationPanel event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_ORGINFOBTN
    void OnImportFeatTableClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CONTEXT_HELP
    void OnContextHelpClick( wxCommandEvent& event );

////@end CSubAnnotationPanel event handler declarations

////@begin CSubAnnotationPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSubAnnotationPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// @name CJobAdapter::IJobCallback interface implementation
/// @{
    virtual void OnJobResult(CObject* result, CJobAdapter& adapter);
    virtual void OnJobFailed(const string&, CJobAdapter& adapter);
    /// @}

    virtual void SetSeqEntryHandle(objects::CSeq_entry_Handle seh) { m_Seh = seh;}
    virtual wxString GetAnchor() { return _("annotation"); }

private:
    void x_SetFontAsTextPanel();
////@begin CSubAnnotationPanel member variables
    CGenericPanel* m_FeatureTable;
public:
    /// Control identifiers
    enum {
        ID_CSUBANNOTATIONPANEL = 6636,
        ID_IMPORT_FTABLE_BTN = 6637,
        ID_SUBANNOTFEATTBL = 6638
    };
////@end CSubAnnotationPanel member variables

    virtual bool TransferDataToWindow();

private:
    CRef<CJobAdapter> m_JobAdapter;
    ICommandProccessor* m_CmdProcessor;
    wxString m_WorkDir;
    objects::CSeq_entry_Handle m_Seh;

public:
    struct SFeatTableParams {
        objects::CSeq_entry_Handle seh;

        bool showProteinId;
        bool showSrcFeature;

        SFeatTableParams(bool protein_id, bool src_feat)
            : showProteinId(protein_id),
            showSrcFeature(src_feat) {}
    };

    static bool Create5ColFeatTable(SFeatTableParams& input, string& output, string& error, ICanceled& canceled);
};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___SUBANNOTATION_PANEL__HPP
