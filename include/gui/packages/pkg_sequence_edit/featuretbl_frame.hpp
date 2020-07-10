/*  $Id: featuretbl_frame.hpp 43488 2019-07-16 19:55:22Z asztalos $
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
 *
 */

#ifndef _FEATURETBL_FRAME_H_
#define _FEATURETBL_FRAME_H_

#include <corelib/ncbistd.hpp>
#include <gui/utils/job_adapter.hpp>
#include <gui/objutils/reg_settings.hpp>
#include <wx/frame.h>

class wxCheckBox;

BEGIN_NCBI_SCOPE

class CGenericPanel;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CFEATURETBLFR_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CFEATURETBLFR_TITLE _("Feature Table")
#define SYMBOL_CFEATURETBLFR_IDNAME ID_CFEATURETBLFR
#define SYMBOL_CFEATURETBLFR_SIZE wxSize(700, 600)
#define SYMBOL_CFEATURETBLFR_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CFeatureTblFr class declaration
 */

class CFeatureTblFr : 
    public wxFrame,
    public IRegSettings,
    public CJobAdapter::IJobCallback
{    
    DECLARE_CLASS( CFeatureTblFr )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CFeatureTblFr();
    CFeatureTblFr(  wxWindow* parent, IWorkbench* workbench,
                    wxWindowID id = SYMBOL_CFEATURETBLFR_IDNAME, 
                    const wxString& caption = SYMBOL_CFEATURETBLFR_TITLE, 
                    const wxPoint& pos = SYMBOL_CFEATURETBLFR_POSITION, 
                    const wxSize& size = SYMBOL_CFEATURETBLFR_SIZE, 
                    long style = SYMBOL_CFEATURETBLFR_STYLE );

    bool Create(wxWindow* parent, 
                wxWindowID id = SYMBOL_CFEATURETBLFR_IDNAME, 
                const wxString& caption = SYMBOL_CFEATURETBLFR_TITLE, 
                const wxPoint& pos = SYMBOL_CFEATURETBLFR_POSITION, 
                const wxSize& size = SYMBOL_CFEATURETBLFR_SIZE, 
                long style = SYMBOL_CFEATURETBLFR_STYLE );

    /// Destructor
    ~CFeatureTblFr();

    /// @name CJobAdapter::IJobCallback interface implementation
    /// @{
    virtual void OnJobResult(CObject* result, CJobAdapter& adapter);
    virtual void OnJobFailed(const string&, CJobAdapter& adapter);
    /// @}

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CFeatureTblFr event handler declarations
    void OnCloseWindow(wxCloseEvent& event);

    void OnRefreshClick(wxCommandEvent& event);
////@end CFeatureTblFr event handler declarations

////@begin CFeatureTblFr member function declarations

    /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath(const string& reg_path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CFeatureTblFr member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void SetWorkDir(const wxString& workDir);

private:
    void x_RefreshTable(const string& path = kEmptyStr);


////@begin CFeatureTblFr member variables
    wxCheckBox* m_ProteinId;
    wxCheckBox* m_Source;
    wxCheckBox* m_ProteinFeats;
    CGenericPanel* m_GenericPanel;

    IWorkbench* m_Workbench;

    CRef<CJobAdapter> m_JobAdapter;
    string m_RegPath;
    bool m_ShowSources;
    bool m_ShowTranscriptId;
    bool m_ShowProteinFeats{ false };
    int m_FindMode;
    /// Control identifiers
    enum {
        ID_CFEATURETBLFR = 10454,
        ID_FTBL_CHCKBX1 = 10455,
        ID_FTBL_CHCKBX2 = 10456,
        ID_FTBL_CHCKBX3 = 10457,
        ID_FTBL_REFRESHBTN = 10458,
        ID_FTBL_GENPANEL = 10459
    };
////@end CFeatureTblFr member variables
};

END_NCBI_SCOPE

#endif
    // _FEATURETBL_FRAME_H_
