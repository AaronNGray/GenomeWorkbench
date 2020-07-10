#ifndef GUI_CORE___OPEN_VIEW_MANAGER_IMPL__HPP
#define GUI_CORE___OPEN_VIEW_MANAGER_IMPL__HPP

/*  $Id: open_view_manager_impl.hpp 43435 2019-06-28 16:15:08Z katargir $
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

#include <util/icanceled.hpp>

#include <gui/core/open_view_manager.hpp>
#include <gui/core/project_view_factory.hpp>
#include <gui/objutils/reg_settings.hpp>
#include <gui/widgets/object_list/object_list_widget_sel.hpp>

#include <wx/panel.h>

class wxCheckBox;

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CObjectListWidgetSel;
class CwxTableListCtrl;
class CParamsPanel;

///////////////////////////////////////////////////////////////////////////////
// Default implementation for some of the IOpenViewManager interfaces.

class NCBI_GUICORE_EXPORT COpenViewManager
    : public CObject
    , public IOpenViewManager
    , public IRegSettings
{
public:
    class COVMException : public CException
    {
    public:
        /// Errors are classified into one of two types.
        enum EErrCode {
            /// Group creation error.
            eGroupErr
        };

        /// Get a message describing the situation leading to the throw.
        virtual const char* GetErrCodeString() const override
        {
            switch ( GetErrCode() ) {
            case eGroupErr: return "eGroupErr";
            default: return CException::GetErrCodeString();
            }
        }

        /// Include standard NCBI exception behavior.
        NCBI_EXCEPTION_DEFAULT(COVMException, CException);
    };

public:
    COpenViewManager();
    COpenViewManager( IProjectViewFactory& factory );

    void SetProjectViewFactory( IProjectViewFactory* factory );

    /// @name IOpenViewManager interface implementation
    /// @{
    virtual void SetParentWindow( wxWindow* parent );

    virtual void InitUI();
    virtual void CleanUI();

    virtual void SetInputObjects( const TConstScopedObjects& input );
    virtual const TConstScopedObjects& GetOutputObjects() const;
    virtual const vector<int> GetOutputIndices() const;

    virtual wxPanel* GetCurrentPanel();

    virtual bool AtFinalStep();
    virtual bool IsCompletedState();
    virtual bool CanDo( EAction action );
    virtual bool DoTransition( EAction action );
    //virtual IAppTask*   GetTask();
    /// @}

    /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath(const string& path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

protected:
    virtual bool NeedParamsPanel( const TObjectListMap& objects );
    virtual CParamsPanel* CreateParamsPanel();
    virtual void FinalizeParams();

private:
    void x_DoConversion(TObjectListMap& objects);
    void x_LoadParamPanelSettings();

public:
    void DoAsyncConversion( TObjectListMap& objects, ICanceled& cancel );

protected:
    enum EState {
        eInvalid = -1,
        eParams = 0,
        eCompleted = 1,

        eMaxState = 1
    };

protected:
    IProjectViewFactory* m_Factory;
    const CProjectViewTypeDescriptor* m_Descriptor;

    /// objects that serve as input arguments for the object converter
    TConstScopedObjects m_InputObjects;

    /// The output objects after view-specific object conversion and possible object
    /// aggregation. These objects will be feed into views with one object for a
    /// separate view.
    TConstScopedObjects m_OutputObjects;
    mutable vector<int> m_OutputIndices;

    bool mf_IsCombineRequested;

    /// The GUI panel listing the parameters guiding the object conversion and
    /// the conversion results.
    CParamsPanel* m_ParamPanel;

    wxWindow* m_ParentWindow;

    int m_State;
    string m_RegPath;
};


///////////////////////////////////////////////////////////////////////////////
/// CParamsPanel

////@begin control identifiers
#define SYMBOL_CPARAMSPANEL_STYLE wxNO_BORDER|wxTAB_TRAVERSAL
#define SYMBOL_CPARAMSPANEL_TITLE _("Parameters Panel")
#define SYMBOL_CPARAMSPANEL_IDNAME ID_CPARAMSPANEL
#define SYMBOL_CPARAMSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CPARAMSPANEL_POSITION wxDefaultPosition
////@end control identifiers


class NCBI_GUICORE_EXPORT CParamsPanel : public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CParamsPanel )
    DECLARE_EVENT_TABLE()

public:
    CParamsPanel();

    virtual bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CPARAMSPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CPARAMSPANEL_POSITION, 
        const wxSize& size = SYMBOL_CPARAMSPANEL_SIZE, 
        long style = SYMBOL_CPARAMSPANEL_STYLE 
    );

    ////@begin CConversionResultPanel event handler declarations

    void OnOneViewPerObjectClick( wxCommandEvent& event );

    ////@end CConversionResultPanel event handler declarations

    ////@begin CConversionResultPanel member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
    ////@end CConversionResultPanel member function declarations

    static bool ShowToolTips();

    enum {
        ID_CPARAMSPANEL = 10034,
        ID_ONEVIEWPEROBJECT = 10035
    };
    ////@end CConversionResultPanel member variables

    /// add your methods here
    virtual void SetObjects(const TObjectListMap& objs );
    virtual void SelectAll() {}

    /// Return the selected item
    virtual bool GetSelection( TConstScopedObjects& buf )
    {
        buf.clear();
        return false; 
    }

    virtual bool GetSelectedObjects( TConstScopedObjects& buf )
    {
        return GetSelection( buf ); 
    }

    virtual bool GetSelectedIndices( vector<int>& buf )
    {
        buf.clear();
        return false; 
    }

    virtual bool IsCombineRequested() const;

    /// Force OneViewPerObject true, and lock it
    virtual void EnforceOneViewPerObject();

    /// @name IRegSettings implementation
    /// @{
    virtual void SetRegistryPath( const string& path );
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

protected:
    bool                m_OneViewPerObject;
    string              m_RegPath;
    TObjectListMap      m_ObjectMaps;
};


///////////////////////////////////////////////////////////////////////////////
/// CConversionResultPanel

////@begin control identifiers
#define SYM_CONVRESPANEL_STYLE wxNO_BORDER|wxTAB_TRAVERSAL
#define SYM_CONVRESPANEL_TITLE _("Parameters Panel")
#define SYM_CONVRESPANEL_IDNAME ID_CCONVERSIONRESULTPANEL
#define SYM_CONVRESPANEL_SIZE wxSize(400, 300)
#define SYM_CONVRESPANEL_POSITION wxDefaultPosition
////@end control identifiers

class NCBI_GUICORE_EXPORT CConversionResultPanel: public CParamsPanel
{
    DECLARE_DYNAMIC_CLASS( CConversionResultPanel )
    DECLARE_EVENT_TABLE()

public:

    CConversionResultPanel();
    CConversionResultPanel( wxWindow* parent, 
        wxWindowID id = SYM_CONVRESPANEL_IDNAME, 
        const wxPoint& pos = SYM_CONVRESPANEL_POSITION, 
        const wxSize& size = SYM_CONVRESPANEL_SIZE, 
        long style = SYM_CONVRESPANEL_STYLE 
    );

    virtual bool Create( wxWindow* parent, 
        wxWindowID id = SYM_CONVRESPANEL_IDNAME, 
        const wxPoint& pos = SYM_CONVRESPANEL_POSITION, 
        const wxSize& size = SYM_CONVRESPANEL_SIZE, 
        long style = SYM_CONVRESPANEL_STYLE 
    );

    void Init();

    virtual void CreateControls();

    enum {
        ID_CCONVERSIONRESULTPANEL = 10034,
        ID_ONEVIEWPEROBJECT = 10035,
        ID_GROUPVIEW = 10036
    };

    void SetProjectViewFactory( IProjectViewFactory* factory ){ 
        m_PVFactory = factory; 
    }

    void OnGroupViewClick( wxCommandEvent& event );

    /// add your methods here
    virtual void SetObjects(const TObjectListMap& objs );
    virtual void SelectAll();

    /// Return the selected item
    virtual bool GetSelection( TConstScopedObjects& buf );
    virtual bool GetSelectedIndices( vector<int>& buf );

    /// Force OneViewPerObject true, and lock it
    virtual void EnforceOneViewPerObject();

    /// @name IRegSettings implementation
    /// @{
    virtual void SetRegistryPath(const string& path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

private:
    ////@begin CConversionResultPanel member variables
    IProjectViewFactory* m_PVFactory;
    CObjectListWidgetSel* m_ListPanel;
    CwxTableListCtrl* m_GroupCtrl;
    wxCheckBox* m_GroupCheckBox;
    wxCheckBox* m_OneViewPerObjectCheckBox;

    vector<int> m_Groups;
    vector<int> m_GroupSize;
    vector<CSeq_id_Handle> m_LocHandles;
    vector< set<CSeq_id_Handle> > m_AlnHandles;
    ////@end CConversionResultPanel member variables
};

END_NCBI_SCOPE

#endif  // GUI_CORE___OPEN_VIEW_MANAGER_IMPL__HPP
