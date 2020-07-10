#ifndef GUI_WIDGETS_SEQ___SEQ_TEXT_PANEL__HPP
#define GUI_WIDGETS_SEQ___SEQ_TEXT_PANEL__HPP

/*  $Id: seq_text_panel.hpp 36516 2016-10-03 19:16:16Z katargir $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors: Roman Katargin
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include <gui/utils/job_adapter.hpp>
#include <gui/widgets/seq_text/seq_text_ds.hpp>
#include <gui/widgets/seq_text/seq_text_widget.hpp>

#include <wx/panel.h>
#include <wx/dialog.h>

class wxStaticText;
class wxButton;

BEGIN_NCBI_SCOPE

////@begin forward declarations
class CSeqTextWidget;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSEQTEXTPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSEQTEXTPANEL_TITLE _("Sequence Text Panel")
#define SYMBOL_CSEQTEXTPANEL_IDNAME ID_CSEQTEXTPANEL
#define SYMBOL_CSEQTEXTPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSEQTEXTPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSeqTextPanel class declaration
 */

class NCBI_GUIWIDGETS_SEQ_EXPORT CSeqTextPanel: 
    public wxPanel, 
    public ISeqTextWidgetHost,
    public CJobAdapter::IJobCallback
{
    DECLARE_DYNAMIC_CLASS( CSeqTextPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSeqTextPanel();
    CSeqTextPanel( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSEQTEXTPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSEQTEXTPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSEQTEXTPANEL_SIZE, 
        long style = SYMBOL_CSEQTEXTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSEQTEXTPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSEQTEXTPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSEQTEXTPANEL_SIZE, 
        long style = SYMBOL_CSEQTEXTPANEL_STYLE );

    /// Destructor
    ~CSeqTextPanel();

    /// Initialises member variables
    void Init();
    bool InitObject(SConstScopedObject& object);

    /// Creates the controls and sizers
    void CreateControls();

    CSeqTextWidget* GetWidget() const { return m_SeqTextWidget; }
    const CSeqTextDataSource* GetDataSource() const { return m_DataSource.GetPointer(); }

////@begin CSeqTextPanel event handler declarations
    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
    void OnFindBwdClick(wxCommandEvent& event);

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
    void OnFindFwdClick(wxCommandEvent& event);

    void OnStopClick(wxCommandEvent& event);

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE1
    void OnFeatureTypeSelected(wxCommandEvent& event);

////@end CSeqTextPanel event handler declarations

////@begin CSeqTextPanel member function declarations

    /// @name CJobAdapter::IJobCallback interface implementation
    /// @{
    virtual void OnJobResult(CObject* result, CJobAdapter& adapter);
    virtual void OnJobFailed(const string&, CJobAdapter& adapter);
    /// @}

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSeqTextPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSeqTextPanel member variables
    wxButton* m_BwdButton;
    wxButton* m_FwdButton;
    wxButton* m_StopButton;
    wxStaticText* m_MousePos;
    CSeqTextWidget* m_SeqTextWidget;
    /// Control identifiers
    enum {
        ID_CSEQTEXTPANEL = 10000,
        ID_TEXTCTRL1 = 10001,
        ID_BUTTON = 10002,
        ID_BUTTON1 = 10003,
        ID_BUTTON2 = 10004,
        ID_CHOICE1 = 10005,
        ID_WINDOW = 10006
    };
////@end CSeqTextPanel member variables

// ISeqTextWidgetHost implemenatation
    virtual void        STWH_ChangeConfig() {}
    virtual void        STWH_ReportMouseOverPos(TSeqPos pos);

private:
    void x_StartSearch(bool forward);
    void x_RestoreButtonStates();

    CRef<CSeqTextDataSource> m_DataSource;
    CRef<CJobAdapter> m_JobAdapter;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___SEQ_TEXT_PANEL__HPP
