#ifndef PKG_ALIGNMENT___ALIGN_TAB_EXPORT_PAGE2__HPP
#define PKG_ALIGNMENT___ALIGN_TAB_EXPORT_PAGE2__HPP

/*  $I$
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

#include <wx/panel.h>
#include <gui/packages/pkg_alignment/align_tab_export_params.hpp>
#include <gui/utils/job_future.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

class wxListBox;
class wxCheckListBox;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CALIGNTABEXPORTPAGE2_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CALIGNTABEXPORTPAGE2_TITLE _("Align Tab Export Page 2")
#define SYMBOL_CALIGNTABEXPORTPAGE2_IDNAME ID_CALIGNTABEXPORTPAGE2
#define SYMBOL_CALIGNTABEXPORTPAGE2_SIZE wxSize(400, 300)
#define SYMBOL_CALIGNTABEXPORTPAGE2_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE


/*!
 * CAlignTabExportPage2 class declaration
 */

class CAlignTabExportPage2: public wxPanel
    , public IRegSettings
{    
    DECLARE_DYNAMIC_CLASS( CAlignTabExportPage2 )
    DECLARE_EVENT_TABLE()

public:
    CAlignTabExportPage2();
    CAlignTabExportPage2( wxWindow* parent, wxWindowID id = SYMBOL_CALIGNTABEXPORTPAGE2_IDNAME, const wxPoint& pos = SYMBOL_CALIGNTABEXPORTPAGE2_POSITION, const wxSize& size = SYMBOL_CALIGNTABEXPORTPAGE2_SIZE, long style = SYMBOL_CALIGNTABEXPORTPAGE2_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CALIGNTABEXPORTPAGE2_IDNAME, const wxPoint& pos = SYMBOL_CALIGNTABEXPORTPAGE2_POSITION, const wxSize& size = SYMBOL_CALIGNTABEXPORTPAGE2_SIZE, long style = SYMBOL_CALIGNTABEXPORTPAGE2_STYLE );

    ~CAlignTabExportPage2();

    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path);
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    void CreateControls();

    virtual bool TransferDataToWindow();

    virtual bool TransferDataFromWindow();

    CAlignTabExportParams& GetData() { return m_data; }
    const CAlignTabExportParams& GetData() const { return m_data; }
    void SetData(const CAlignTabExportParams& data) { m_data = data; }

////@begin CAlignTabExportPage2 event handler declarations

////@end CAlignTabExportPage2 event handler declarations

    void OnIdle(wxIdleEvent& event);

////@begin CAlignTabExportPage2 member function declarations

    bool GetNoAlignments() const { return m_NoAlignments ; }
    void SetNoAlignments(bool value) { m_NoAlignments = value ; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CAlignTabExportPage2 member function declarations

    static bool ShowToolTips();

////@begin CAlignTabExportPage2 member variables
    wxBoxSizer* m_Sizer;
    wxListBox* m_AlignList;
    wxCheckListBox* m_FieldList;
    wxBoxSizer* m_LoadingSizer;
private:
    bool m_NoAlignments;
    enum {
        ID_CALIGNTABEXPORTPAGE2 = 10080,
        ID_LISTBOX = 10081,
        ID_CHECKLISTBOX = 10082
    };
////@end CAlignTabExportPage2 member variables

    enum {
        ID_LOADING_ERROR = 10100,
        ID_LOADING_TEXT,
        ID_LOADING_PROGRESS
    };
   
private:
    CAlignTabExportParams m_data;
    string m_RegPath;
    wxColour m_SaveTextClr;

    void x_StartAlignNamesJob();
    void x_StartNAAlignNamesJob();
    void x_FinishLoading();
    void x_ReportError(const string errMsg);

    map<string, string> m_TitleToAccession;

    job_future<vector<string> > m_FutureNames;
    job_future<vector<tuple<string, string> > > m_FutureNANames;
};

END_NCBI_SCOPE

#endif // PKG_ALIGNMENT___ALIGN_TAB_EXPORT_PAGE__HPP
