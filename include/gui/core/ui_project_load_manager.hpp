#ifndef GUI_CORE___UI_PROJECT_LOAD_MANAGER__HPP
#define GUI_CORE___UI_PROJECT_LOAD_MANAGER__HPP

/*  $Id: ui_project_load_manager.hpp 39528 2017-10-05 15:27:37Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/core/ui_tool_manager.hpp>
#include <gui/core/project_selector_panel.hpp>
#include <gui/core/ui_file_load_manager.hpp>

#include <gui/utils/extension_impl.hpp>


BEGIN_NCBI_SCOPE

class CProjectLoadOptionPanel;

///////////////////////////////////////////////////////////////////////////////
/// CProjectLoadManager -Load Manager that loads both Projects and Workspaces
class  NCBI_GUICORE_EXPORT  CProjectLoadManager :
    public CObject,
    public IUIToolManager
{
public:
    CProjectLoadManager();

    /// @name IUIToolManager interface implementation
    /// @{
    virtual void    SetServiceLocator(IServiceLocator* srv_locator);
    virtual void    SetParentWindow(wxWindow* parent);
    virtual const IUIObject&  GetDescriptor() const;
    virtual void    InitUI();
    virtual void    CleanUI();
    virtual wxPanel*    GetCurrentPanel();
    virtual bool    CanDo(EAction action);
    virtual bool    IsFinalState();
    virtual bool    IsCompletedState();
    virtual bool    DoTransition(EAction action);
    virtual IAppTask*      GetTask();
    /// @}

protected:
    enum EState {
        eInvalid = -1,
        eSelectFile,
        eCompleted
    };

protected:
    mutable CUIObject m_Descriptor;

    IServiceLocator*    m_SrvLocator;
    wxWindow*   m_ParentWindow;
    EState  m_State;

    CProjectLoadOptionPanel*    m_OptionPanel;
};


///////////////////////////////////////////////////////////////////////////////
/// CProjectFormatLoadManager
class  NCBI_GUICORE_EXPORT  CProjectFormatLoadManager :
    public CObject,
    public IFileFormatLoaderManager,
    public IExtension
{
public:
    CProjectFormatLoadManager();

    /// @name CUILoadManager interface implementation
    /// @{
    virtual void    SetServiceLocator(IServiceLocator* srv_locator);
    virtual void    SetParentWindow(wxWindow* parent);
    virtual const IUIObject&  GetDescriptor() const;
    virtual void    InitUI();
    virtual void    CleanUI();
    virtual wxPanel*    GetCurrentPanel();
    virtual bool    CanDo(EAction action);
    virtual bool    IsFinalState();
    virtual bool    IsCompletedState();
    virtual bool    DoTransition(EAction action);
    virtual IAppTask*      GetTask();
    /// @}

    /// @{ IFileFormatLoaderManager - additional members
    /// @{
    virtual string   GetFileLoaderId() const { return "file_loader_gbench_project"; }
    virtual wxString GetFormatWildcard();
    virtual bool     ValidateFilenames(const vector<wxString>& filenames);
    virtual void     SetFilenames(const vector<wxString>& filenames);
    virtual void     GetFilenames(vector<wxString>& filenames) const;
    virtual bool     IsInitialState();
    virtual bool     RecognizeFormat(const wxString& filename);
    virtual bool     RecognizeFormat(CFormatGuess::EFormat fmt);
    /// @}

    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

protected:
    CUIObject m_Descr;
    IServiceLocator*    m_SrvLocator;
    wxWindow*   m_ParentWindow;
    vector<wxString>  m_Filenames;

};


END_NCBI_SCOPE


#endif  // GUI_CORE___UI_PROJECT_LOAD_MANAGER__HPP

