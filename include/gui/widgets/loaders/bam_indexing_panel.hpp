/*  $Id: bam_indexing_panel.hpp 43788 2019-08-30 16:03:23Z katargir $
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
// Generated by DialogBlocks (unregistered), 11/03/2011 15:58:40

#ifndef _BAM_INDEXING_PANEL_H_
#define _BAM_INDEXING_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <corelib/ncbistd.hpp>
#include <wx/panel.h>

#include <gui/gui_export.h>

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations
class wxTextCtrl;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CBAMINDEXINGPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CBAMINDEXINGPANEL_TITLE _("Bam Indexing")
#define SYMBOL_CBAMINDEXINGPANEL_IDNAME ID_CBAMINDEXINGPANEL
#define SYMBOL_CBAMINDEXINGPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CBAMINDEXINGPANEL_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/*!
 * CBamIndexingPanel class declaration
 */

class NCBI_GUIWIDGETS_LOADERS_EXPORT CBamIndexingPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CBamIndexingPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CBamIndexingPanel();
    CBamIndexingPanel( wxWindow* parent, wxWindowID id = SYMBOL_CBAMINDEXINGPANEL_IDNAME, const wxString& caption = SYMBOL_CBAMINDEXINGPANEL_TITLE, const wxPoint& pos = SYMBOL_CBAMINDEXINGPANEL_POSITION, const wxSize& size = SYMBOL_CBAMINDEXINGPANEL_SIZE, long style = SYMBOL_CBAMINDEXINGPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CBAMINDEXINGPANEL_IDNAME, const wxString& caption = SYMBOL_CBAMINDEXINGPANEL_TITLE, const wxPoint& pos = SYMBOL_CBAMINDEXINGPANEL_POSITION, const wxSize& size = SYMBOL_CBAMINDEXINGPANEL_SIZE, long style = SYMBOL_CBAMINDEXINGPANEL_STYLE );

    /// Destructor
    ~CBamIndexingPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    void SetBamFilePath(const string& file);
    void SetSamtoolsPath(const string& path);
    string GetSamtoolsPath() const;
    bool IsInputValid();

////@begin CBamIndexingPanel event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON6
    void OnButton6Click( wxCommandEvent& event );

////@end CBamIndexingPanel event handler declarations

////@begin CBamIndexingPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CBamIndexingPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CBamIndexingPanel member variables
    wxTextCtrl* m_SamtoolsPath;
    /// Control identifiers
    enum {
        ID_CBAMINDEXINGPANEL = 10083,
        ID_TEXTCTRL9 = 10084,
        ID_BUTTON6 = 10085
    };
////@end CBamIndexingPanel member variables

private:
    wxString  m_BamFilePath;
};

END_NCBI_SCOPE


#endif
    // _BAM_INDEXING_PANEL_H_
