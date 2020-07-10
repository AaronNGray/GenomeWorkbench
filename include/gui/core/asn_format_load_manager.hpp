#ifndef GUI_CORE___UI_ASN_LOAD_MANAGER__HPP
#define GUI_CORE___UI_ASN_LOAD_MANAGER__HPP

/*  $Id: asn_format_load_manager.hpp 38477 2017-05-15 21:10:59Z evgeniev $
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

#include <gui/core/ui_file_load_manager.hpp>

#include <gui/widgets/loaders/file_load_panel_client.hpp>
#include <gui/widgets/loaders/tool_wizard.hpp>

#include <gui/utils/extension.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CASNFormatLoadManager
///
/// CASNFormatLoadManager - a Load Manager for ASN format. This class implements
/// IFileFormatLoaderManager interface in order to work with generic Format Load
/// Manager.
class  NCBI_GUICORE_EXPORT  CASNFormatLoadManager :
    public CObject,
    public IFileFormatLoaderManager,
	public IFileLoadPanelClient,
	public IToolWizard,
    public IExtension
{
public:
    CASNFormatLoadManager();

    /// @name IFileLoadPanelClient interface implementation
    /// @{
	virtual string   GetLabel() const;
    virtual wxString GetFormatWildcard() const;
	virtual string   GetFileLoaderId() const { return Id(); }
    /// @}

    /// @name IUILoadManager interface implementation
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
    virtual IWizardPage*   GetFirstPage() { return 0; }
    virtual IAppTask*      GetTask();
    virtual IExecuteUnit*  GetExecuteUnit();
    /// @}

    /// @{ IFileFormatLoaderManager - additional members
    /// @{
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

	static string Id() { return "file_loader_asn"; }
	static string Label() { return "NCBI ASN.1"; }

protected:
    CUIObject m_Descr;
    IServiceLocator* m_SrvLocator;
    wxWindow*   m_ParentWindow;
    vector<wxString> m_FileNames;
};

END_NCBI_SCOPE


#endif  // GUI_CORE___UI_ASN_LOAD_MANAGER__HPP
