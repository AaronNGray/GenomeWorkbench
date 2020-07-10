#ifndef GUI_PKG_SEQ_EDIT___ORGGENERAL_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___ORGGENERAL_PANEL__HPP

/*  $Id: orggeneral_panel.hpp 44056 2019-10-17 13:54:12Z asztalos $
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
#include <gui/gui_export.h>
#include <gui/widgets/edit/submission_page_interface.hpp>
#include <wx/panel.h>
#include <objects/seqfeat/BioSource.hpp>
#include <gui/utils/command_processor.hpp>
#include <objmgr/seq_entry_handle.hpp>

class wxChoice;
class wxTextCtrl;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CORGGENERALPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CORGGENERALPANEL_TITLE _("Organism General Panel")
#define SYMBOL_CORGGENERALPANEL_IDNAME ID_CORGGENERALPANEL
#define SYMBOL_CORGGENERALPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CORGGENERALPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * COrgGeneralPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT COrgGeneralPanel: public wxPanel, public ISubmissionPage
{    
    DECLARE_DYNAMIC_CLASS( COrgGeneralPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    COrgGeneralPanel();
    COrgGeneralPanel( wxWindow* parent, 
        ICommandProccessor* cmdproc,
        objects::CSeq_entry_Handle seh,
        wxWindowID id = SYMBOL_CORGGENERALPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CORGGENERALPANEL_POSITION, 
        const wxSize& size = SYMBOL_CORGGENERALPANEL_SIZE, 
        long style = SYMBOL_CORGGENERALPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CORGGENERALPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CORGGENERALPANEL_POSITION, 
        const wxSize& size = SYMBOL_CORGGENERALPANEL_SIZE, 
        long style = SYMBOL_CORGGENERALPANEL_STYLE );

    /// Destructor
    ~COrgGeneralPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    void ApplyBioSource(objects::CBioSource& src);
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    virtual void ApplyCommand();
    virtual void ReportMissingFields(string &text);
    virtual wxString GetAnchor() { return _("organism-info-general"); }


////@begin COrgGeneralPanel event handler declarations

////@end COrgGeneralPanel event handler declarations

////@begin COrgGeneralPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end COrgGeneralPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void SetSeqEntryHandle(objects::CSeq_entry_Handle seh) { m_Seh = seh; }

private:
    CRef<objects::CBioSource> m_Source;
    ICommandProccessor* m_CmdProcessor;
    objects::CSeq_entry_Handle m_Seh;

    void x_Reset();
    bool x_ApplyChangesToSource(objects::CBioSource& src);
    bool x_HasTaxnameChanged(const objects::CBioSource& orig_src, const objects::CBioSource& edited_src);

////@begin COrgGeneralPanel member variables
    wxTextCtrl* m_Taxname;
    wxTextCtrl* m_Strain;
    wxTextCtrl* m_Isolate;
    wxTextCtrl* m_Cultivar;
    wxTextCtrl* m_Breed;
public:
    /// Control identifiers
    enum {
        ID_CORGGENERALPANEL = 6580,
        ID_ORGTAXNAME = 6581,
        ID_ORGSTRAIN = 6582,
        ID_ORGISOLATE = 6583,
        ID_ORGCULTIVAR = 6584,
        ID_ORGBREED = 6585
    };
////@end COrgGeneralPanel member variables
};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___ORGGENERAL_PANEL__HPP
