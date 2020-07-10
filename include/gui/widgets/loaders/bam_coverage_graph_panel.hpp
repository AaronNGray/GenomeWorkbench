/*  $Id: bam_coverage_graph_panel.hpp 44756 2020-03-05 18:44:36Z shkeda $
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
 * Authors:  Liangshou Wu
 */
// Generated by DialogBlocks (unregistered), 02/03/2011 09:12:09

#ifndef _BAM_COVERAGE_GRAPH_PANEL_H_
#define _BAM_COVERAGE_GRAPH_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <wx/panel.h>
#include <set>

#include <gui/gui_export.h>

#include <gui/objutils/reg_settings.hpp>
#include <gui/utils/job_future.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class CAssemblySelPanel;
////@end forward declarations

class wxRadioButton;
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CBAMCOVERAGEGRAPHPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CBAMCOVERAGEGRAPHPANEL_TITLE _("Assembly Options")
#define SYMBOL_CBAMCOVERAGEGRAPHPANEL_IDNAME ID_CBAMCOVERAGEGRAPHPANEL
#define SYMBOL_CBAMCOVERAGEGRAPHPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CBAMCOVERAGEGRAPHPANEL_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

class CAssemblySelPanel;

/*!
 * CBamCoverageGraphPanel class declaration
 */

class NCBI_GUIWIDGETS_LOADERS_EXPORT CBamCoverageGraphPanel: public wxPanel
    , public IRegSettings
{    
    DECLARE_DYNAMIC_CLASS( CBamCoverageGraphPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CBamCoverageGraphPanel();
    CBamCoverageGraphPanel( wxWindow* parent, wxWindowID id = SYMBOL_CBAMCOVERAGEGRAPHPANEL_IDNAME, const wxString& caption = SYMBOL_CBAMCOVERAGEGRAPHPANEL_TITLE, const wxPoint& pos = SYMBOL_CBAMCOVERAGEGRAPHPANEL_POSITION, const wxSize& size = SYMBOL_CBAMCOVERAGEGRAPHPANEL_SIZE, long style = SYMBOL_CBAMCOVERAGEGRAPHPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CBAMCOVERAGEGRAPHPANEL_IDNAME, const wxString& caption = SYMBOL_CBAMCOVERAGEGRAPHPANEL_TITLE, const wxPoint& pos = SYMBOL_CBAMCOVERAGEGRAPHPANEL_POSITION, const wxSize& size = SYMBOL_CBAMCOVERAGEGRAPHPANEL_SIZE, long style = SYMBOL_CBAMCOVERAGEGRAPHPANEL_STYLE );

    /// Destructor
    ~CBamCoverageGraphPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    void SetBamFiles(const set<string>& bam_files);
    bool IsInputValid();

    /// @name IRegSettings interface implementation
    /// @{
    virtual void    SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

////@begin CBamCoverageGraphPanel event handler declarations

    /// wxEVT_IDLE event handler for ID_CBAMCOVERAGEGRAPHPANEL
    void OnIdle( wxIdleEvent& event );

////@end CBamCoverageGraphPanel event handler declarations

////@begin CBamCoverageGraphPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CBamCoverageGraphPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CBamCoverageGraphPanel member variables
    wxStaticText* m_BamAssembly;
    CAssemblySelPanel* m_AssemblyPanel;
    /// Control identifiers
    enum {
        ID_CBAMCOVERAGEGRAPHPANEL = 10074,
        ID_PANEL9 = 10014
    };
////@end CBamCoverageGraphPanel member variables

    set<string>     m_BamFiles;

    string m_RegPath;

    job_future<string> m_BamAssemblyFuture;
};


END_NCBI_SCOPE

#endif
    // _BAM_COVERAGE_GRAPH_PANEL_H_
