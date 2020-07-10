#ifndef GUI_PKG_SEQ_EDIT___GENERAL_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___GENERAL_PANEL__HPP

/*  $Id: general_panel.hpp 43209 2019-05-29 14:01:26Z bollin $
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
#include <gui/utils/command_processor.hpp>
#include <objmgr/seq_entry_handle.hpp>

#include <wx/panel.h>


class wxDatePickerCtrl;
class wxRadioButton;
class wxTextCtrl;
class wxStaticText;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CGENERALPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CGENERALPANEL_TITLE _("General Panel")
#define SYMBOL_CGENERALPANEL_IDNAME ID_CGENERALPANEL
#define SYMBOL_CGENERALPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CGENERALPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CGeneralPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CGeneralPanel: public wxPanel, public ISubmissionPage
{    
    DECLARE_DYNAMIC_CLASS( CGeneralPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CGeneralPanel();
    CGeneralPanel( wxWindow* parent, 
                   ICommandProccessor* cmdproc,
                   objects::CSeq_entry_Handle seh,
                   wxWindowID id = SYMBOL_CGENERALPANEL_IDNAME, 
                   const wxPoint& pos = SYMBOL_CGENERALPANEL_POSITION, 
                   const wxSize& size = SYMBOL_CGENERALPANEL_SIZE, 
                   long style = SYMBOL_CGENERALPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CGENERALPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CGENERALPANEL_POSITION, 
        const wxSize& size = SYMBOL_CGENERALPANEL_SIZE, 
        long style = SYMBOL_CGENERALPANEL_STYLE );

    /// Destructor
    ~CGeneralPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

////@begin CGeneralPanel event handler declarations

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_SUBMITTER_IMMEDIATE
    void OnSubmitterImmediateSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_SUBMITTER_HUP
    void OnSubmitterHupSelected( wxCommandEvent& event );

////@end CGeneralPanel event handler declarations

////@begin CGeneralPanel member function declarations

   /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CGeneralPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual void ApplySubmitBlock(objects::CSubmit_block& block);
    virtual void ApplyDescriptor(objects::CSeqdesc& desc);
    virtual void SetSeqEntryHandle(objects::CSeq_entry_Handle seh) {m_Seh = seh;}
    virtual void ApplyCommand();
    void ApplyDescriptorCommand();
    virtual wxString GetAnchor() { return _("general"); }

private:
////@begin CGeneralPanel member variables
    wxTextCtrl* m_Bioproject;
    wxTextCtrl* m_Biosample;
    wxRadioButton* m_ImmediateRelease;
    wxRadioButton* m_HUP;
    wxStaticText* m_Label;
    wxDatePickerCtrl* m_Date;
public:
    /// Control identifiers
    enum {
        ID_CGENERALPANEL = 6532,
        ID_SUBMITTER_BIOPROJECT = 6533,
        ID_SUBMITTER_BIOSAMPLE = 6534,
        ID_SUBMITTER_IMMEDIATE = 6535,
        ID_SUBMITTER_HUP = 6536,
        ID_DATEPICKERCTRL = 6537
    };
////@end CGeneralPanel member variables

private:
    ICommandProccessor* m_CmdProcessor;
    objects::CSeq_entry_Handle m_Seh;
    CRef< objects::CSubmit_block> m_Block;
    CRef< objects::CSeqdesc> m_Dblink;

    void x_Reset();
    void x_UpdateBlockControls();
    void x_UpdateDescControls();

};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___GENERAL_PANEL__HPP
