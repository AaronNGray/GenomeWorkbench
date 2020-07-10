#ifndef PKG_SEQUENCE___BAM_LOAD_OPTION_PANEL__HPP
#define PKG_SEQUENCE___BAM_LOAD_OPTION_PANEL__HPP

/*  $Id: bam_load_option_panel.hpp 43976 2019-10-01 16:28:09Z katargir $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/utils/job_future.hpp>

#include <gui/widgets/wx/richtextctrl.hpp>
#include <wx/panel.h>
#include <wx/dnd.h>
#include <wx/timer.h>
#include <set>

////@begin includes
////@end includes

////@begin forward declarations
class wxBoxSizer;
class CRichTextCtrl;
////@end forward declarations

////@begin control identifiers
#define SYMBOL_CBAMLOADOPTIONPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CBAMLOADOPTIONPANEL_TITLE _("BAM Load Option Panel")
#define SYMBOL_CBAMLOADOPTIONPANEL_IDNAME ID_CBAMLOADOPTIONPANEL
#define SYMBOL_CBAMLOADOPTIONPANEL_SIZE wxDefaultSize
#define SYMBOL_CBAMLOADOPTIONPANEL_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE


/** @addtogroup GUI_PKG_NCBI_INTERNAL
 *
 * @{
 */

///////////////////////////////////////////////////////////////////////////////
/// CBamLoadOptionPanel

class NCBI_GUIWIDGETS_LOADERS_EXPORT CBamLoadOptionPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CBamLoadOptionPanel )
    DECLARE_EVENT_TABLE()

public:
    typedef set<string> TBamFiles;

    CBamLoadOptionPanel();
    CBamLoadOptionPanel( wxWindow* parent, wxWindowID id = SYMBOL_CBAMLOADOPTIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CBAMLOADOPTIONPANEL_POSITION, const wxSize& size = SYMBOL_CBAMLOADOPTIONPANEL_SIZE, long style = SYMBOL_CBAMLOADOPTIONPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CBAMLOADOPTIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CBAMLOADOPTIONPANEL_POSITION, const wxSize& size = SYMBOL_CBAMLOADOPTIONPANEL_SIZE, long style = SYMBOL_CBAMLOADOPTIONPANEL_STYLE );

    ~CBamLoadOptionPanel();

    void Init();

    void CreateControls();

////@begin CBamLoadOptionPanel event handler declarations

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_BAM_INPUT
    void OnBamInputTextUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
    void OnButtonClick( wxCommandEvent& event );

////@end CBamLoadOptionPanel event handler declarations

////@begin CBamLoadOptionPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CBamLoadOptionPanel member function declarations

    static bool ShowToolTips();

////@begin CBamLoadOptionPanel member variables
    wxBoxSizer* m_MainSizer;
    CRichTextCtrl* m_BamInput;
    wxBoxSizer* m_ParseProgressSizer;
    /// Control identifiers
    enum {
        ID_CBAMLOADOPTIONPANEL = 10070,
        ID_BAM_INPUT = 10071,
        ID_BUTTON = 10072
    };
////@end CBamLoadOptionPanel member variables

    enum {
        ID_PARSING_TEXT = 10100,
        ID_PARSING_PROGRESS
    };

    void    OnTimer(wxTimerEvent& event);

public:
    string GetInput() const;
    void SetInput(const string& input);

    bool IsInputValid();
    bool HasBamFile() const;
    bool HasIndexFile() const;
    string GetBamFilePath() const;
    const TBamFiles& GetBamFiles() const;
    const TBamFiles& GetCSraFiles() const;
    const TBamFiles& GetSrzAccs() const;
    const TBamFiles& GetSraAccs() const;

    void    SetFilenames(const wxArrayString& filenames);
    void    AddFilenames(const wxArrayString& filenames);

protected:
    void    x_ValidateInput();

    class CDropTarget : public wxDropTarget
    {
    public:
        CDropTarget(CBamLoadOptionPanel& panel);
        virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def);
    protected:
        CBamLoadOptionPanel& m_Panel;
    };

private:
    enum EParseState
    {
        eParseComplete,
        eParseRunning,
        eParseCanceling
    };

    bool         m_UpdatingStyle = false;
    EParseState  m_State = eParseComplete;
    bool         m_TextChanged = false;

    wxTextAttr   m_DefStyle;
    wxTextAttr   m_ErrStyle;
    size_t       m_InputNum;  ///< total number of valid inputs
    size_t       m_ErrNum;    ///< number of token with errors
    TBamFiles    m_BamFiles;
    TBamFiles    m_CSraFiles;
    TBamFiles    m_SrzAccs;
    TBamFiles    m_SraAccs;

    wxTimer      m_Timer;

    std::unique_ptr<async_job> m_ValidateJob;
};


/* @} */

END_NCBI_SCOPE


#endif   // PKG_SEQUENCE___BAM_LOAD_OPTION_PANEL__HPP
